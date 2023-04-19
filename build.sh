#!/usr/bin/env bash

if [ "$(uname)" == "Darwin" ]; then
      cd ./ofxOfelia/macOSExternal
      make
      cp -rf ../ofelia ../../ofelia
      cp -f ./bin/ofelia.pd_darwin.app/Contents/MacOS/ofelia.pd_darwin ../../ofelia/ofelia.pd_darwin
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
      cd ./ofxOfelia/LinuxExternal
      make
      cp -rf ../ofelia ../../ofelia
      cp -f ./bin/ofelia.pd_linux ../../ofelia/ofelia.pd_linux
fi