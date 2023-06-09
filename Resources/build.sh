#!/bin/bash

 # Prepare ofelia folder
rm -rf ./ofelia
cp -rf ./Resources/ofelia ./ofelia
mkdir -p ./ofelia/libs

pushd Libraries
git clone --recursive --shallow-submodules https://github.com/openframeworks/openFrameworks.git
pushd openFrameworks
git reset --hard ac69b2f
popd
popd

# Build for macOS
if [ "$(uname)" == "Darwin" ]; then

      ./Libraries/openFrameworks/scripts/osx/download_libs.sh

      cd ./Resources/Xcode
      xcodebuild -configuration Release -project ../../Libraries/openFrameworks/libs/openFrameworksCompiled/project/osx/openFrameworksLib.xcodeproj
      xcodebuild -configuration Release -project ./ofelia.xcodeproj
      cp -f ./bin/libs/libfmod.dylib ../../ofelia/libs/libfmod.dylib
      cp -f ./bin/ofelia   ../../ofelia/ofelia

      cd ../..
# Build for Linux
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then

      # Apply OF git patch for video playback
      pushd ./Libraries/openFrameworks
      
      ./scripts/linux/download_libs.sh

      if [ ! -f gstreamer-fix.patch ]; then
      wget  -O gstreamer-fix.patch "https://github.com/openFrameworks/openFrameworks/commit/bd4042344dc9670770754374607d2ca8190f9476.patch"
      git apply gstreamer-fix.patch

      # Apply OF git patch for recend sndfile versions
      wget  -O sndfile-fix.patch "https://github.com/openFrameworks/openFrameworks/commit/290dbfc23930425024c9456b5c5519611fb1990d.patch"
      git apply sndfile-fix.patch

      fi
      popd

      cd ./Resources/Makefile
      CFLAGS=-fPIC CXXFLAGS=-std=c++14 make
      cp -f ./bin/ofelia ../../ofelia
fi
