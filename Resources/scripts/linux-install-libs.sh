#!/usr/bin/env bash

# Function to install packages on Ubuntu and Debian
install_ubuntu_packages() {
    apt-get update
    apt-get install -y build-essential ccache clang cmake curl git gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-ugly gstreamer1.0-vaapi libasound2-dev libassimp-dev libcairo-dev libcurl4-openssl-dev libfreetype6-dev libglu1-mesa-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-0 libjack-jackd2-dev libudev-dev libunwind-dev libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev lsb-release mesa-common-dev pax-utils pkg-config python3 python3-pip rsync wget patchelf libsndfile1-dev libxi-dev
}

# Function to install packages on Fedora
install_fedora_packages() {
    dnf install -y ccache clang cmake curl git gstreamer1-plugins-base gstreamer1-plugins-good gstreamer1-plugins-ugly gstreamer1-vaapi alsa-lib-devel libassimp-devel cairo-devel libcurl-devel freetype-devel mesa-libGLU-devel gstreamer1-devel gstreamer1 jack-audio-connection-kit-devel systemd-devel libunwind-devel libX11-devel libXcomposite-devel libXcursor-devel libXext-devel libXinerama-devel libXrandr-devel libXrender-devel redhat-lsb mesa-libGL-devel pax-utils pkg-config python3 python3-pip rsync wget patchelf libsndfile-devel libXi-devel
}

# Function to install packages on openSUSE
install_opensuse_packages() {
   zypper install -y ccache clang cmake curl git gstreamer-plugins-base gstreamer-plugins-good gstreamer-plugins-ugly gstreamer-vaapi libasound2-devel assimp-devel cairo-devel libcurl-devel freetype2-devel glu-devel gstreamer-devel gstreamer-plugins-base-devel libjack-devel libudev-devel libunwind-devel libX11-devel libXcomposite-devel libXcursor-devel libXext-devel libXinerama-devel libXrandr-devel libXrender-devel lsb-release Mesa-devel pax-utils pkg-config python3 python3-pip rsync wget patchelf libsndfile-devel libXi-devel
}

# Function to install packages on openSUSE
install_arch_packages() {
   pacman -S --needed ccache clang cmake curl git gstreamer gst-plugins-base gst-plugins-good gst-plugins-ugly gst-vaapi alsa-lib assimp cairo libcurl freetype2 glu gstreamer gst-plugins-base gst-plugins-good gst-plugins-ugly gst-plugins-base-libs libva libxcomposite libxcursor libxext libxinerama libxrandr libxrender lsb-release mesa pax-utils pkgconf python python-pip rsync wget libsndfile libxi
}

install_dependencies() {
    if [ -f /etc/os-release ]; then
        # shellcheck disable=SC1091
        source /etc/os-release
        if [[ -n $ID ]]; then
            echo "$ID"
            return
        fi
    fi

    if [ -f /etc/lsb-release ]; then
        # shellcheck disable=SC1091
        source /etc/lsb-release
        if [[ -n $DISTRIB_ID ]]; then
            echo "$DISTRIB_ID"
            return
        fi
    fi

    if [ -f /etc/fedora-release ]; then
        install_fedora_packages()
        return
    fi

    if [ -f /etc/SuSE-release ] || [ -f /etc/os-release ]; then
        # shellcheck disable=SC1091
        source /etc/os-release
        if [[ $ID = "opensuse" ]]; then
            install_opensuse_packages()
            return
        fi
    fi

    if [ -f /etc/arch-release ]; then
        install_arch_packages()
        return
    fi

    if [ -f /etc/debian_version ]; then
        install_ubuntu_packages()
        return
    fi

    install_ubuntu_packages()
}

install_dependencies()

# Manually build kissfft, because OF builds it without the fPIC flag
rm -rf ./openFrameworks/libs/kiss
./kiss/kiss.sh

# Install copydeps python package for shared library packaging
pip3 install copydeps
