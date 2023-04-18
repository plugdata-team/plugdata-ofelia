#!/usr/bin/env bash

mkdir bin


if [ "$(uname)" == "Darwin" ]; then
      cd ./ofxOfelia/macOSExternal
      make
      cp -f ./bin/ofelia.pd_darwin.app/Contents/MacOS/ofelia.pd_darwin ../../bin/ofelia.pd_darwin
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
      cd ./ofxOfelia/LinuxExternal
      make
      cp -f ./bin/ofelia.pd_linux ../../bin/ofelia.pd_linux
fi