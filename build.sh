#!/usr/bin/env bash

if [ "$(uname)" == "Darwin" ]; then
      cd ./ofxOfelia/macOSExternal
      make
      cp -rf ../ofelia ../../
      cp -f ./bin/ofelia.pd_darwin.app/Contents/MacOS/ofelia.pd_darwin ../../ofelia/ofelia.pd_darwin
      mkdir ../../ofelia/libs
      cp -f ./bin/ofelia.pd_darwin.app/Contents/Frameworks/libfmod.dylib ../../ofelia/libs/libfmod.dylib
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
      cd ./ofxOfelia/LinuxExternal
      CC=clang CXX=clang++ CFLAGS=-fPIC make
      cp -rf ../ofelia ../../
      cp -f ./bin/ofelia.pd_linux ../../ofelia/ofelia.pd_linux
      mkdir ../../ofelia/libs
      #cp -f ./bin/ofelia.pd_darwin.app/Contents/Frameworks/libfmodex.dylib ../../ofelia/ofelia.pd_darwin
fi