#!/bin/bash

#
# KissFFT
# "Keep It Simple, Stupid" Fast Fourier Transform
# http://sourceforge.net/projects/kissfft/
#
# has a Makefile
FORMULA_TYPES=( "linux" "linux64" "linuxarmv6l" "linuxarmv7l" "linuxaarch64" "msys2")

# define the version
VER=130

# tools for git use
GIT_URL=https://github.com/mborgerding/kissfft.git
GIT_TAG=v$VER

# download the source code and unpack it into LIB_NAME
function download() {
    echo "Running: git clone --branch ${GIT_TAG} ${GIT_URL}"
    git clone --branch ${GIT_TAG} ${GIT_URL}
    mv kissfft kiss
}

# prepare the build environment, executed inside the lib src dir
function prepare() {
	cp -f ./Makefile ./kiss/Makefile
}

# executed inside the lib src dir
function build() {
    make
}

OF_ROOT=$(realpath ./Libraries/openFrameworks/libs)
echo ${OF_ROOT}

# executed inside the lib src dir, first arg $1 is the dest libs dir root
function copy() {
	# headers
	mkdir -p ${OF_ROOT}/kiss/include
	cp -v kiss_fft.h ${OF_ROOT}/kiss/include
	cp -v tools/kiss_fftr.h ${OF_ROOT}/kiss/include

	mkdir -p ${OF_ROOT}/kiss/lib/linux64
	cp -v lib/libkiss.a ${OF_ROOT}/kiss/lib/linux64/libkiss.a

	# copy license file
	rm -rf ${OF_ROOT}/kiss/license # remove any older files if exists
	mkdir -p ${OF_ROOT}/kiss/license
	cp -v COPYING ${OF_ROOT}/kiss/license/
}

pushd Libraries/kiss

download
prepare

pushd kiss
build
copy
popd
popd