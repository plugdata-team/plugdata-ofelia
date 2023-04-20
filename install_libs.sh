#!/usr/bin/env bash

if [ "$(uname)" == "Darwin" ]; then
      ./openFrameworks/scripts/osx/download_libs.sh
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
      echo $(gcc -v)
      sudo ./openFrameworks/scripts/linux/ubuntu/install_dependencies.sh -y
      ./openFrameworks/scripts/linux/download_libs.sh -a 64gcc6
      rm -rf ./openFrameworks/libs/kiss
      ./kiss/kiss.sh
      #CC=clang CXX=clang++ CFLAGS=-fPIC ./openFrameworks/scripts/linux/compileOF.sh
fi