#!/usr/bin/env bash

if [ "$(uname)" == "Darwin" ]; then
      cd ./ofxOfelia/macOSExternal
      make
      cp -rf ../ofelia ../../
      cp -f ./bin/ofelia.pd_darwin.app/Contents/MacOS/ofelia.pd_darwin ../../ofelia/ofelia.pd_darwin
      cp -f ./bin/ofelia.pd_darwin.app/Contents/Frameworks/libfmod.dylib ../../ofelia/libfmod.dylib
      install_name_tool -change @executable_path/../Frameworks/libfmod.dylib @rpath/libfmod.dylib ../../ofelia/ofelia.pd_darwin
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
      cd ./ofxOfelia/LinuxExternal
      CC=clang CXX=clang++ CFLAGS=-fPIC make
      cp -rf ../ofelia ../../
      cp -f ./bin/ofelia.* ../../ofelia
      cp -rf ./deps-scripts ../../ofelia
      ./bundle_libs.sh ../../ofelia/ofelia.*
      #cp -f ./bin/ofelia.pd_darwin.app/Contents/Frameworks/libfmodex.dylib ../../ofelia/ofelia.pd_darwin
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
      cd ./ofxOfelia/WindowsExternal
fi