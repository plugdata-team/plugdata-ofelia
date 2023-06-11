#!/bin/bash

# Function to install packages on Ubuntu and Debian
install_debian_packages() {
    export DEBIAN_FRONTEND=noninteractive
    export TZ="Europe/Amsterdam"
    apt-get update
    apt-get install -y gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-ugly gstreamer1.0-vaapi libgstreamer-plugins-base1.0-dev libgstreamer1.0-0 freeglut3-dev libasound2-dev libxmu-dev libxxf86vm-dev g++ libgl1-mesa-dev libglu1-mesa-dev libraw1394-dev libudev-dev libdrm-dev libglew-dev libopenal-dev libsndfile-dev libfreeimage-dev libcairo2-dev libfreetype6-dev libssl-dev libpulse-dev libusb-1.0-0-dev libgtk${GTK_VERSION}-dev libopencv-dev libegl1-mesa-dev libgles1 libgles2-mesa-dev libassimp-dev librtaudio-dev libboost-filesystem-dev libglfw3-dev liburiparser-dev libcurl4-openssl-dev libpugixml-dev libpoco-dev libgconf-2-4
    ./Libraries/openFrameworks/scripts/linux/debian/install_dependencies.sh
}

# Function to install packages on Fedora
install_fedora_packages() {
    dnf install -y gstreamer1-plugins-base gstreamer1-plugins-good gstreamer1-plugins-ugly gstreamer1-vaapi alsa-lib-devel libassimp-devel cairo-devel libcurl-devel freetype-devel mesa-libGLU-devel gstreamer1-devel gstreamer1 jack-audio-connection-kit-devel systemd-devel libunwind-devel libX11-devel libXcomposite-devel libXcursor-devel libXext-devel libXinerama-devel libXrandr-devel libXrender-devel redhat-lsb mesa-libGL-devel pax-utils pkg-config python3 python3-pip rsync wget patchelf libsndfile-devel libXi-devel
    ./Libraries/openFrameworks/scripts/linux/fedora/install_dependencies.sh
}

# Function to install packages on openSUSE
install_opensuse_packages() {
   DISTROVER=$(grep ^VERSION /etc/SuSE-release | tr -d ' ' | cut -d = -f 2)
   zypper addrepo obs://games/openSUSE_$DISTROVER games # for freeimage
   zypper refresh games
   zypper install -y freeglut-devel alsa-devel libXmu-devel libXxf86vm-devel gcc-c++ libraw1394-devel gstreamer-devel gstreamer-plugins-base-devel libudev-devel libtheora-devel libvorbis-devel openal-soft-devel libsndfile-devel python-lxml glew-devel flac-devel freeimage-devel cairo-devel libjack-devel portaudio-devel libpulse-devel
   zypper install -y ccache clang cmake curl git gstreamer-plugins-base gstreamer-plugins-good gstreamer-plugins-ugly gstreamer-vaapi libasound2-devel assimp-devel cairo-devel libcurl-devel freetype2-devel glu-devel gstreamer-devel gstreamer-plugins-base-devel libjack-devel libudev-devel libunwind-devel libX11-devel libXcomposite-devel libXcursor-devel libXext-devel libXinerama-devel libXrandr-devel libXrender-devel lsb-release Mesa-devel pax-utils pkg-config python3 python3-pip rsync wget patchelf libsndfile-devel libXi-devel
}

# Function to install packages on openSUSE
install_arch_packages() {
   ./Libraries/openFrameworks/scripts/linux/archlinux/install_dependencies.sh
}


if [ -f /etc/redhat-release ] ; then
    install_fedora_packages
elif [ -f /etc/SuSE-release ] ; then
    install_opensuse_packages
elif [ -f /etc/debian_version ] ; then	
    install_debian_packages
elif [ -f /etc/arch-release ] ; then
    install_arch_packages
fi

# Manually build kissfft, because OF builds it without the fPIC flag
rm -rf ./Libraries/openFrameworks/libs/kiss
./Libraries/kiss/kiss.sh

# Install copydeps python package for shared library packaging
pip3 install copydeps

