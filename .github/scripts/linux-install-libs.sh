#!/usr/bin/env bash

export DEBIAN_FRONTEND=noninteractive
export TZ="Europe/Amsterdam"

# Install basics first, this can help to prevent package conflicts
sudo apt install -y build-essential ccache clang cmake curl git gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-ugly gstreamer1.0-vaapi libasound2-dev libassimp-dev libcairo-dev libcurl4-openssl-dev libfreetype6-dev libglu1-mesa-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-0 libjack-jackd2-dev libudev-dev libunwind-dev libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev lsb-release mesa-common-dev pax-utils pkg-config python3 python3-pip rsync wget patchelf libsndfile1-dev

# Make sure all OF packages are installed
sudo DEBIAN_FRONTEND=noninteractive ./openFrameworks/scripts/linux/ubuntu/install_dependencies.sh -y

# Download OF libraries
./openFrameworks/scripts/linux/download_libs.sh

# Manually build kissfft, because OF builds it without the fPIC flag
rm -rf ./openFrameworks/libs/kiss
./kiss/kiss.sh

# Install copydeps python package for shared library packaging
pip3 install copydeps

# Try removing these plugins, we probably don't need them and it bloats out package a lot!
# Maybe later, we can delete plugins more selectively based on how essential vs large they are
sudo apt remove gstreamer1.0-plugins-bad