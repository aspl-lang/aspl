#!/bin/bash

IFS=''

if $(git config --get remote.origin.url | grep -q 'github.com[:/]aspl-lang/aspl'); then
    git pull origin main
else
    git clone https://github.com/aspl-lang/aspl.git
    cd aspl
fi
SHA=$(curl -s https://api.github.com/repos/aspl-lang/cd/contents/latest.txt -s | jq -r '.content' | base64 -d)
if [ "$(uname)" = "Darwin" ]; then
    if [ "$(uname -m)" = "x86_64" ]; then
        EXECUTABLE=aspl_macos_x86_64
    elif [ "$(uname -m)" = "armv7l" ]; then
        EXECUTABLE=aspl_macos_arm32
    elif [ "$(uname -m)" = "aarch64" ]; then
        EXECUTABLE=aspl_macos_arm64
    else
        echo "Unsupported architecture"
        exit 1
    fi
elif [ "$(uname)" = "Linux" ]; then
    if [ "$(uname -m)" = "x86_64" ]; then
        EXECUTABLE=aspl_linux_x86_64
    elif [ "$(uname -m)" = "armv7l" ]; then
        EXECUTABLE=aspl_linux_arm32
    elif [ "$(uname -m)" = "aarch64" ]; then
        EXECUTABLE=aspl_linux_arm64
    else
        echo "Unsupported architecture"
        exit 1
    fi
else
    echo "Unsupported operating system"
    exit 1
fi

CURL="curl https://api.github.com/repos/aspl-lang/cd/releases"
ASSET_ID=$(eval "$CURL/tags/SHA-$SHA -s" | jq .assets | jq '.[] | select(.name == "'$EXECUTABLE'").id')
echo "Downloading $EXECUTABLE ($ASSET_ID):"
eval "$CURL/assets/$ASSET_ID -LJOH 'Accept: application/octet-stream' --progress-bar"

mv $EXECUTABLE aspl
chmod +x aspl

read -p "Create asml symlink in /usr/local/bin (Y/n)?" choice
case "$choice" in 
  ''|y|Y ) echo "Creating link"; sudo ln -fs $PWD/aspl /usr/local/bin/aspl;;
  * ) echo "Skiping...";;
esac

CURL="curl https://api.github.com/repos/aspl-lang/cd/releases"
ASSET_ID=$(eval "$CURL/tags/SHA-$SHA -s" | jq .assets | jq '.[] | select(.name == "templates.zip").id')
echo "Downloading templates.zip ($ASSET_ID):"
eval "$CURL/assets/$ASSET_ID -LJOH 'Accept: application/octet-stream' --progress-bar"
unzip -of templates.zip
rm templates.zip

