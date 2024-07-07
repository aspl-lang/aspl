SHA=$(curl -s https://api.github.com/repos/aspl-lang/cd/contents/latest.txt | jq -r '.content' | base64 -d)
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
CURL="curl https://api.github.com/repos/aspl-lang/cd/releases"; ASSET_ID=$(eval "$CURL/tags/SHA-$SHA" | jq .assets | jq '.[] | select(.name == "'$EXECUTABLE'").id'); eval "$CURL/assets/$ASSET_ID -LJOH 'Accept: application/octet-stream'"
mv $EXECUTABLE aspl
chmod +x aspl
ln -s $PWD/aspl /usr/local/bin/aspl
CURL="curl https://api.github.com/repos/aspl-lang/cd/releases"; ASSET_ID=$(eval "$CURL/tags/SHA-$SHA" | jq .assets | jq '.[] | select(.name == "templates.zip").id'); eval "$CURL/assets/$ASSET_ID -LJOH 'Accept: application/octet-stream'"
unzip templates.zip
rm templates.zip