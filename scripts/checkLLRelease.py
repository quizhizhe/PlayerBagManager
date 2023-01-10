import requests
import base64
import git

BDSVersion = "0.0.0"
LLVersion = "0.0.0"
VERSION_PATH = "./LLCheckBag/Version.h"

versionFile = open(VERSION_PATH,'r+')
versionInfo = versionFile.readlines()
versionFile.close()

#获取LL协议，并存下此LL支持的BDS版本
def getLiteLoaderProtocol():
    global LLVersion
    global BDSVersion
    response = requests.get("https://api.github.com/repos/LiteLDev/LiteLoaderBDS/releases/latest")
    LLVersion = response.json()["tag_name"]
    bodyinfo = response.json()["body"]
    index = bodyinfo.find("ProtocolVersion")
    BDSindex = bodyinfo.find("Support BDS")
    BDSVersion = bodyinfo[BDSindex+12:index-3]
    return bodyinfo[index+16:index+19]

def getllcheckbagVersion():
    versionline = versionInfo[10]
    versionMajor = versionline[36:len(versionline)-1]

    versionline = versionInfo[11]
    versionMinor = versionline[36:len(versionline)-1]

    versionline = versionInfo[12]
    versionRevision = versionline[36:len(versionline)-1]

    return "v"+versionMajor+"."+versionMinor+"."+versionRevision

def getllcheckbagProtocol():
    protocolline = versionInfo[15]
    return protocolline[36:len(protocolline)-1]

def modifyVersionInfo(oldProtocol,newProtocol):
    print("Change version Info")
    #修改小版本号
    revisionline = versionInfo[12]
    versionRevision = int(revisionline[36:len(revisionline)-1])

    revisionline.replace(str(versionRevision),str(versionRevision+1))
    versionInfo[12] = revisionline

    #修改协议号
    protocolline = versionInfo[15]
    protocolline.replace(oldProtocol,newProtocol)

    print("Change version Info Success")

def modifyChangelog():
    print("Change Changelog")
    #以下变量需要在修改后获取，否则是旧版本号
    version = getllcheckbagVersion()
    protocol = getllcheckbagProtocol()
    changlogzh = "# "+version+ "（"+protocol+"协议）\n\n- 支持"+BDSVersion
    changlogen = "# "+version+ "("+protocol+"Protocol)\n\n- Support "+BDSVersion
    with open('CHANGELOG.md', "r+") as filezh:
        filezh.seek(0)
        filezh.truncate()
        filezh.write(changlogzh)
    filezh.close()
    with open('CHANGELOG_en.md', "r+") as fileen:
        fileen.seek(0)
        fileen.truncate()
        fileen.write(changlogzh)
    fileen.close()
    print("Change Changelog Success")

def modifyBDSLink():
    print("Change LINK.txt")
    header = {"accept":"application/vnd.github+json"}
    response = requests.get("https://api.github.com/repos/LiteLDev/LiteLoaderBDS/contents/scripts/LINK.txt",headers=header)
    linkinfo = response.json()["content"]
    link =  str(base64.b64decode(linkinfo),"utf8")
    with open('../LINK.txt', "r+") as file:
        file.seek(0)
        file.truncate()
        file.write(link)
    file.close()
    print("Change LINK.txt Success")

def commitChange():
    print("Commit Change")
    version = getllcheckbagVersion()
    repo = git.Repo("../")
    commitmsg = "Support "+BDSVersion
    modify_file_list = repo.index.diff(None)
    #print([m.a_path for m in modify_file_list])
    repo.index.add([m.a_path for m in modify_file_list])
    repo.index.commit(commitmsg)
    repo.create_tag(version)
    print("Commit Change Success")

if __name__ == '__main__':
    protocol = getllcheckbagProtocol()
    llprotocol = getLiteLoaderProtocol()
    if protocol == llprotocol:
        print("Don't need Update")
    else:
        print("Need Update\nNow Start Auto Update")
        modifyVersionInfo(protocol,llprotocol)

        with open(VERSION_PATH, "r+") as file:
            file.seek(0)
            file.truncate()
            file.write(versionInfo)
        file.close()

        modifyChangelog()
        modifyBDSLink()
        commitChange()
        print("Update Success")
