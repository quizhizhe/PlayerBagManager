import argparse
import logging
import os
from typing import Tuple
import requests
import json

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

class UpdateManager:
    def __init__(self, resource_id: int, token: str, new_version: str):
        self.resource_url = f"https://api.minebbs.com/api/openapi/v1/resources/{resource_id}"
        self.upload_url = f"https://api.minebbs.com/api/openapi/v1/upload/"
        self.update_url = f"https://api.minebbs.com/api/openapi/v1/resources/{resource_id}/update"
        self.file_headers = {
            "Authorization": f"Bearer {token}",
        }
        self.json_headers = {
            "Authorization": f"Bearer {token}",
            "Content-Type": "application/json",
        }
        # v1.2.3 -> 1.2.3
        self.new_version = new_version.replace("v", "").strip()
    
    def upload(self, file_path: str) -> str:
        with open(file_path, "rb") as f:
            data = f.read()
            multiple_files  = [
                ('upload[]', (os.path.basename(file_path), f))
            ]
            # send binary file data

            response = requests.post(self.upload_url, files=multiple_files, headers=self.file_headers)
            if response.status_code == 200 and response.json()["success"] == True:
                print(response.json())
                return response.json()["data"][0]
            else:
                message = response.json()["message"]
                raise Exception(f"Upload file failed: {response.status_code} - {message}")
    
    def updateFile(self, change_log: str, key: str) -> None:
        title, description = self.parseChangelog(change_log)
        data = {
            "title": title,
            "description": description,
            "new_version": f"v{self.new_version}",
            "file_key": key,
        }
        response = requests.post(self.update_url, data=json.dumps(data), headers=self.json_headers)
        if response.status_code == 200 and response.json()["success"] == True:
            return response.json()["data"]
        else:
            message = response.json()["message"]
            raise Exception(f"Update file failed: {response.status_code} - {message}")

    def updateUrl(self, change_log: str, url: str) -> None:
        title, description = self.parseChangelog(change_log)
        data = {
            "title": title,
            "description": description,
            "new_version": f"v{self.new_version}",
            "file_url": url,
        }
        response = requests.post(self.update_url, json={data}, headers=self.json_headers)
        if response.status_code == 200 and response.json()["success"] == True:
            return response.json()["data"]
        else:
            message = response.json()["message"]
            raise Exception(f"Upload url failed: {response.status_code} - {message}")

    def parseChangelog(self, file_path: str) -> Tuple[str, str]:
        with open(file_path, "r", encoding= "utf-8") as f:
            lines = f.readlines()
            title = lines[0].strip()
            title = title.replace("#", "").strip()
            description = "\n".join(lines[1:])
            description = description.strip()
            description = description.replace("\n\n", "\n")
            return title, description
    
    def getLatestVersion(self) -> str:
        response = requests.get(self.resource_url)
        if response.status_code == 200 and response.json()["success"] == True:
            version = response.json()["data"]["version"]
            return version.replace("v", "").strip()
        else:
            message = response.json()["message"]
            raise Exception(f"Upload file failed: {response.status_code} - {message}")
    


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Publish Minebbs update')
    parser.add_argument('-f', '--file', help='file to publish', required=True)
    parser.add_argument('-c', '--change', help='change log', required=True)
    parser.add_argument('-v', '--version', help='version', required=True)
    parser.add_argument('-t', '--token', help='MineBBS token', required=True)
    args = parser.parse_args()

    manager = UpdateManager(resource_id=3367, token=args.token, new_version=args.version)
    current_version = manager.getLatestVersion()
    file_key =  manager.upload(args.file)
    if file_key:
        manager.updateFile(args.change, file_key)
    else:
        raise Exception("Upload failed")
    if not manager.getLatestVersion() == manager.new_version:
        raise Exception(f"Publish failed, expected version: {manager.new_version}, actual version: {manager.getLatestVersion()}")
    print("Publish Success")
