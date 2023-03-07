@echo off

if not exist %1\SDK-1.16.40\lib\bedrock_server_api.lib goto process
if not exist %1\SDK-1.16.40\lib\bedrock_server_var.lib goto process
goto end

:process
cd /d %1\SDK-1.16.40\tools\
LibraryBuilder.exe -o ..\lib\

:end