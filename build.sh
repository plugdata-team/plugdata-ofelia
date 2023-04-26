#!/usr/bin/env bash

 # Prepare ofelia folder
rm -rf ./ofelia
cp -rf ./ofxOfelia/ofelia ./
mkdir -p ./ofelia/libs

# Build for macOS
if [ "$(uname)" == "Darwin" ]; then
      cd ./ofxOfelia/macOSExternal
      xcodebuild -configuration Release -project ../../openFrameworks/libs/openFrameworksCompiled/project/osx/openFrameworksLib.xcodeproj
      xcodebuild -configuration Release -project ./ofelia.xcodeproj
      # Copy fmod and external
      cp -f ./bin/libs/libfmod.dylib ../../ofelia/libs/libfmod.dylib
      cp -f ./bin/ofelia.pd_darwin   ../../ofelia/ofelia.pd_darwin
# Build for Linux
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then

      # Apply OF git patch for video playback
      pushd ./openFrameworks
      if [[-f gstreamer-fix.patch]]; then
      else
      wget https://github.com/openFrameworks/openFrameworks/commit/bd4042344dc9670770754374607d2ca8190f9476.patch -o gstreamer-fix.patch
      git apply gstreamer-fix.patch
      fi
      popd

      cd ./ofxOfelia/LinuxExternal
      CC=clang CXX=clang++ CFLAGS=-fPIC make
      # Copy output file with either .l_arm and .pd_linux extensions
      cp -f ./bin/ofelia.* ../../ofelia
fi
