#!/usr/bin/env bash
#
#[ -n "$1" ] || set -- a.out
#mkdir -p ./ofelia/libs/
## use ldd to resolve the libs and use `patchelf --print-needed to filter out
## "magic" libs kernel-interfacing libs such as linux-vdso.so, ld-linux-x86-65.so or libpthread
## which you probably should not relativize anyway
#join \
#    <(lddtree "$1" |awk '{if(substr($3,0,1)=="/") print $1,$3}' |sort) \
#    <(patchelf --print-needed "$1" |sort) |cut -d\  -f2 |
#
##copy the lib selection to ./libs
#xargs -d '\n' -I{} cp --copy-contents {} ./ofelia/libs/
##make the relative lib paths override the system lib path
#
#
## We can hopefully assume these libraries are already installed on the users' system
#rm ./ofelia/libs/libpthread.so.*
#rm ./ofelia/libs/libc.so.*
#rm ./ofelia/libs/libX11.so.*
#rm ./ofelia/libs/libstdc++.so.6
#rm ./ofelia/libs/libdl.so.*
#
## These have a lot of annoying sub-dependencies, so we better not include them
#rm ./ofelia/libs/libfreeimage.so.*
#rm ./ofelia/libs/libglib-2.0.so.*

patchelf --set-rpath "\$ORIGIN/libs" "$1"

mkdir -p ./ofelia/libs/
copydeps "$1" -d ./ofelia/libs/