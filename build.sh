#!/usr/bin/env bash

 # Prepare ofelia folder
rm -rf ./ofelia
cp -rf ./ofxOfelia/ofelia ./
mkdir -p ./ofelia/libs

# Build for macOS
if [ "$(uname)" == "Darwin" ]; then
      cd ./ofxOfelia/macOSExternal
      xcodebuild -configuration Release -project ./ofelia.xcodeproj
      # Copy fmod and external
      cp -f ./bin/libfmod.dylib    ../../ofelia/libs/libfmod.dylib
      cp -f ./bin/ofelia.pd_darwin ../../ofelia/ofelia.pd_darwin
      # Tell dynamic library about the new location of fmod
      install_name_tool -change @executable_path/../Frameworks/libfmod.dylib @loader_path/libs/libfmod.dylib ../../ofelia/ofelia.pd_darwin
# Build for Linux
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
      cd ./ofxOfelia/LinuxExternal
      CC=clang CXX=clang++ CFLAGS=-fPIC make
      # Copy output file with either .l_arm and .pd_linux extensions
      cp -f ./bin/ofelia.* ../../ofelia
      cd ../../
       # Script that copies all non-system dynamic libraries, and copies them into a dir that gets added as rpath
       # This should ensure that ofelia runs on any Linux distro, it's basically a flatpak
      ./bundle_libs.sh ./ofelia/ofelia.pd_linux
fi