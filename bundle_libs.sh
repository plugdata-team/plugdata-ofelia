# List of libraries not to copy
cat > .exclude-list << EOF

# These libs are required by LSB 5.0 Common - Core
librt.so.1
libdl.so.2
libgcc_s.so.1
libpthread.so.0
libz.so.1
libm.so.6
libc.so.6

# These libs are required by LSB 5.0 Common - Desktop
libX11.so.6
libXi.so.6
libICE.so.6
libSM.so.6
libxcb.so.1
libfreetype.so.6
libfontconfig.so.1
libXrender.so.1

### This one is not in LSB 5.0 Common - Desktop, but it's X11 & xcb, so it has to be there
libX11-xcb.so.1

## glib
libglib-2.0.so.0
libgmodule-2.0.so.0
libgobject-2.0.so.0
libgthread-2.0.so.0

## libGL
# Do not ship libGL, it depends on plugins and the user might have a different
# one, for example if he is using an NVidia card
libGL.so.1

## Skip libxml2, it is mandatory and shipping it means including extra versions of libicu
libxml2.so.2

#We can also exclude any dependencies that plugdata has:
libasound.so.2
libcurl.so.4
libstdc++.so.6
libbz2.so.1
libpng16.so.16
libharfbuzz.so.0
libbrotlidec.so.1
libnghttp2.so.14
libidn2.so.0
libssl.so.3
libcrypto.so.3
libgssapi_krb5.so.2
libgraphite2.so.3
libbrotlicommon.so.1
libunistring.so.2
libkrb5.so.3
libk5crypto.so.3
libcom_err.so.2
libkrb5support.so.0
libkeyutils.so.1
libresolv.so.2
libXau.so.6
libpcre2-8.so.0

EOF

mkdir -p ./ofelia/libs/
copydeps "$1" --exclude .exclude-list -d ./ofelia/libs/

patchelf --set-rpath "\$ORIGIN/libs" "$1"

for filename in ./ofelia/libs/*; do
    patchelf --set-rpath "\$ORIGIN" $filename
done

# Copy all gstreamer plugins
cp -rf /usr/lib/gstreamer-1.0 ./ofelia/libs

# Loop over plugins, add dependencies into same dir as ofelia's dependencies
# This will ensure we don't get doubled dependencies
for filename in ./ofelia/libs/gstreamer-1.0/*; do
    copydeps $filename --exclude .exclude-list -d ./ofelia/libs/
    patchelf --set-rpath "\$ORIGIN/.." $filename
done
