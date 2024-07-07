@echo off

FOR /F "tokens=*" %%g IN ('git config --get remote.origin.url') do (SET remote=%%g)
echo %remote% | findstr /C:"github.com/aspl-lang/aspl" > nul && (
    git "pull" "origin" "main"
) || (
    git "clone" "https://github.com/aspl-lang/aspl.git"
    cd "aspl"
)

curl -L -o jq.exe https://github.com/stedolan/jq/releases/latest/download/jq-win64.exe

(curl -s https://api.github.com/repos/aspl-lang/cd/contents/latest.txt | jq -r .content) > sha.txt
certutil -decode sha.txt sha_decoded.txt
SET /p SHA=<sha_decoded.txt
DEL sha.txt
DEL sha_decoded.txt
SET CURL=curl https://api.github.com/repos/aspl-lang/cd/releases

if %PROCESSOR_ARCHITECTURE% == AMD64 (
    SET EXECUTABLE=aspl_windows_x86_64.exe
)
if %PROCESSOR_ARCHITECTURE% == IA64 (
    SET EXECUTABLE=aspl_windows_x86_64.exe
)
if %PROCESSOR_ARCHITECTURE% == ARM64 (
    SET EXECUTABLE=aspl_windows_arm64.exe
)
if not defined EXECUTABLE (
    echo Unsupported architecture
    exit /b 1
)

(%CURL%/tags/SHA-%SHA% | jq .assets | jq ".[] | select(.name == \"%EXECUTABLE%\").id") > id.txt
SET /p ASSET_ID=<id.txt
%CURL%/assets/%ASSET_ID% -LJOH "Accept: application/octet-stream"
DEL id.txt
rename %EXECUTABLE% aspl.exe
setx PATH "%PATH%;%CD%"

(%CURL%/tags/SHA-%SHA% | jq .assets | jq ".[] | select(.name == \"templates.zip\").id") > id.txt
SET /p ASSET_ID=<id.txt
DEL id.txt
%CURL%/assets/%ASSET_ID% -LJOH "Accept: application/octet-stream"
tar -xf "templates.zip"
DEL  "templates.zip"

DEL jq.exe