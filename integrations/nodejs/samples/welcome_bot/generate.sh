#!/bin/sh

if [ $# -ne 1 ]
  then
    echo "Usage: generate.sh <destination path>"
    exit 1
fi
mkdir -p temp/wickrio_addon
cp package.json.distribution temp/package.json
cp install.sh.distribution temp/install.sh
cp start.sh stop.sh configure.sh welcome_bot_node.js temp
cd ../../wickrio_addon
cp -r build/ install.sh ../samples/welcome_bot/temp/wickrio_addon
cp package.json.distribution ../samples/welcome_bot/temp/wickrio_addon/package.json
cd ../samples/welcome_bot/temp
tar czf $1/software.tar.gz *
cd ..
rm -r temp
