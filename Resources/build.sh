#!/usr/bin/env bash

 # Prepare ofelia folder
rm -rf ./ofelia
cp -rf ./Resources/ofelia ./ofelia
mkdir -p ./ofelia/libs

# Build for macOS
if [ "$(uname)" == "Darwin" ]; then
      cd ./Resources/Xcode
      xcodebuild -configuration Release -project ../../Libraries/openFrameworks/libs/openFrameworksCompiled/project/osx/openFrameworksLib.xcodeproj
      xcodebuild -configuration Release -project ./ofelia.xcodeproj
      #cp -f ./bin/libs/libfmod.dylib ../../ofelia/libs/libfmod.dylib
      cp -f ./bin/ofelia   ../../ofelia/ofelia
# Build for Linux
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then

      # Apply OF git patch for video playback
      pushd ./Libraries/openFrameworks
      if [ ! -f gstreamer-fix.patch ]; then
      wget  -O gstreamer-fix.patch "https://github.com/openFrameworks/openFrameworks/commit/bd4042344dc9670770754374607d2ca8190f9476.patch"
      git apply gstreamer-fix.patch
      fi
      popd

      cd ./Resources/Makefile
      CC=clang CXX=clang++ CFLAGS=-fPIC make
      # Copy output file with either .l_arm and .pd_linux extensions
      cp -f ./bin/ofelia.* ../../ofelia
fi
