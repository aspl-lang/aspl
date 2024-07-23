@echo off
setlocal enabledelayedexpansion

FOR /F "tokens=*" %%g IN ('git config --get remote.origin.url') do (SET remote=%%g)
echo %remote% | findstr /C:"github.com/aspl-lang/aspl" > nul && (
    git pull origin main
) || (
    git clone https://github.com/aspl-lang/aspl.git
    cd aspl
)

curl -L -o jq.exe https://github.com/stedolan/jq/releases/latest/download/jq-win64.exe

IF DEFINED GITHUB_TOKEN (
    SET AUTH_HEADER=Authorization: token %GITHUB_TOKEN%
) ELSE (
    SET AUTH_HEADER=
)

curl -s -H "%AUTH_HEADER%" https://api.github.com/repos/aspl-lang/cd/contents/latest.txt | jq -r .content > sha.txt
certutil -decode sha.txt sha_decoded.txt
SET /p SHA=<sha_decoded.txt
DEL sha.txt
DEL sha_decoded.txt

IF "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    SET EXECUTABLE=aspl_windows_x86_64.exe
) ELSE IF "%PROCESSOR_ARCHITECTURE%"=="IA64" (
    SET EXECUTABLE=aspl_windows_x86_64.exe
) ELSE IF "%PROCESSOR_ARCHITECTURE%"=="ARM64" (
    SET EXECUTABLE=aspl_windows_arm64.exe
) ELSE (
    echo Unsupported architecture
    exit /b 1
)

curl -H "%AUTH_HEADER%" https://api.github.com/repos/aspl-lang/cd/releases/tags/SHA-%SHA% -s | jq .assets | jq ".[] | select(.name == \"%EXECUTABLE%\").id" > id.txt
SET /p ASSET_ID=<id.txt
DEL id.txt
echo Downloading %EXECUTABLE% (%ASSET_ID%):
curl -H "%AUTH_HEADER%" -LJOH "Accept: application/octet-stream" https://api.github.com/repos/aspl-lang/cd/releases/assets/%ASSET_ID%
rename %EXECUTABLE% aspl.exe

echo Make the `aspl` command available everywhere (by adding it to the PATH?)? (y/n): 
set /p choice=y
if /i "%choice%"=="y" setx PATH "%PATH%;%CD%"

curl -H "%AUTH_HEADER%" https://api.github.com/repos/aspl-lang/cd/releases/tags/SHA-%SHA% -s | jq .assets | jq ".[] | select(.name == \"templates.zip\").id" > id.txt
SET /p ASSET_ID=<id.txt
DEL id.txt
echo Downloading templates.zip (%ASSET_ID%):
curl -H "%AUTH_HEADER%" -LJOH "Accept: application/octet-stream" https://api.github.com/repos/aspl-lang/cd/releases/assets/%ASSET_ID%
tar -xf templates.zip
DEL templates.zip

DEL jq.exe