#!/usr/bin/env bash

[ -n "$1" ] || set -- a.out
mkdir -p ./ofelia/libs/
# use ldd to resolve the libs and use `patchelf --print-needed to filter out
# "magic" libs kernel-interfacing libs such as linux-vdso.so, ld-linux-x86-65.so or libpthread
# which you probably should not relativize anyway
join \
    <(ldd "$1" |awk '{if(substr($3,0,1)=="/") print $1,$3}' |sort) \
    <(patchelf --print-needed "$1" |sort) |cut -d\  -f2 |

#copy the lib selection to ./lib
xargs -d '\n' -I{} cp --copy-contents {} ./ofelia/libs/
#make the relative lib paths override the system lib path
patchelf --set-rpath "\$ORIGIN/libs" "$1"
