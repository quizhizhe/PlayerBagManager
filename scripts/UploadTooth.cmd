@echo off

cd %~dp0..
setlocal enabledelayedexpansion

set TOOTH_REMOTE_PATH=https://github.com/quizhizhe/Tooth-LLCheckBag.git


echo [INFO] Fetching LLCheckBag Tooth to GitHub ...
echo.


for /f "delims=-" %%i in ('git describe --tags --always') do set LLCHECKBAG_NOW_TAG=%%i


echo LLCHECKBAG_NOW_TAG %LLCHECKBAG_NOW_TAG%

echo [INFO] LLCheckBag Tooth Pulling from remote...
echo.
git clone %TOOTH_REMOTE_PATH%

cd Tooth-LLCheckBag
git fetch --all
cd ..

echo.
echo [INFO] Fetching LLCheckBag Tooth to GitHub finished
echo.

@REM remove all directory except .git in LiteLoaderSDK
@REM for /f "delims=" %%i in ('dir /b /ad LiteLoaderSDK') do (
@REM     if not "%%i"==".git" (
@REM         echo [INFO] Removing LiteLoaderSDK\%%i
@REM         rd /s /q LiteLoaderSDK\%%i
@REM     )
@REM )

@REM copy all from build/sdk to LiteLoaderSDK
xcopy /e /y /i /q output\* Tooth-LLCheckBag

set LLCHECKBAG_VERSION=[%1]

@REM Get LL Version for CHANGELOG.md file
set /a n=0
for /f "tokens=2 delims=v" %%i in (..\CHANGELOG.md) do (
    if !n!==1 (
        set LLVERSION=%%i
    )
    set /a n+=1
)

set vers=    "version": "%LLCHECKBAG_VERSION:~1%",
set str=                "%LLVERSION:~0,-1%x"

@REM modify tooth.json
set /a num=0
for /f "tokens=1 delims=" %%i in (Tooth-LLCheckBag\tooth.json) do (
    if !num!==3 (
        echo %vers%>>$
    ) ^
    else if !num!==7 (
        echo %str%>>$
    ) ^
    else (
        echo %%i>>$
    )

    set /a num+=1
)
move $ Tooth-LLCheckBag\tooth.json

cd Tooth-LLCheckBag
for /f "delims=" %%i in ('git status . -s') do set TOOTH_NOW_STATUS=%%i
if "%TOOTH_NOW_STATUS%" neq "" (
    echo [INFO] Modified files found.
    echo.
    git add .
    git commit -m "%LLCHECKBAG_NOW_TAG%"
    git tag %LLCHECKBAG_NOW_TAG%

    echo.
    echo [INFO] Pushing to origin...
    echo.

    git push origin main
    git push --tags origin main

    cd ..
    echo.
    echo [INFO] Upload finished.
    echo.
    goto Finish
) else (
    cd ..
    echo.
    echo.
    echo [INFO] No modified files found.
    echo [INFO] No need to Upgrade LiteLoaderSDK.
    goto Finish
)

:Finish
if [%2]==[action] goto End
timeout /t 3 >nul
:End