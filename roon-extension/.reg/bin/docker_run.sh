#!/bin/sh
docker container inspect roon-extension-bridge > /dev/null 2>&1

if [ $? -eq 0 ]; then
    docker stop roon-extension-bridge
    docker rm roon-extension-bridge
fi

docker run -d --network host --restart unless-stopped --name roon-extension-bridge -v /Users/muness1/src/roon-extension-generator/out/knob/.reg/etc/config.json:/home/node/config.json muness/roon-extension-bridge:latest-arm64v8
