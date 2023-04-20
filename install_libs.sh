#!/usr/bin/env bash

if [ "$(uname)" == "Darwin" ]; then
      ./openFrameworks/scripts/osx/download_libs.sh
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
      sudo ./openFrameworks/scripts/linux/ubuntu/install_dependencies.sh -y
      ./openFrameworks/scripts/linux/download_libs.sh
      rm -rf ./openFrameworks/libs/kiss
      ./kiss/kiss.sh
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
     ./openFrameworks/scripts/vs/download_libs.sh
fi