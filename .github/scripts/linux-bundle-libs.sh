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

# We can also exclude any dependencies that plugdata has:
libapparmor.so.1.8.2
libasound.so.2
libasound.so.2.0.0
libasound_module_pcm_pulse.so
libasyncns.so.0.3.1
libbrotlicommon.so.1
libbrotlicommon.so.1.0.9
libbrotlidec.so.1
libbrotlidec.so.1.0.9
libbsd.so.0.11.5
libbz2.so.1
libcap.so.2.44
libcom_err.so.2
libcom_err.so.2.1
libcrypto.so.3
libcurl.so.4
libcurl.so.4.7.0
libdbus-1.so.3.19.13
libffi.so.8.1.0
libFLAC.so.8.3.0
libfreetype.so.6.18.1
libgcrypt.so.20.3.4
libgmp.so.10.4.1
libgnutls.so.30.31.0
libgpg-error.so.0.32.1
libgraphite2.so.3
libgssapi_krb5.so.2
libgssapi_krb5.so.2.2
libharfbuzz.so.0
libhogweed.so.6.4
libidn2.so.0
libidn2.so.0.3.7
libk5crypto.so.3
libk5crypto.so.3.1
libkeyutils.so.1
libkeyutils.so.1.9
libkrb5.so.3
libkrb5.so.3.3
libkrb5support.so.0
libkrb5support.so.0.1
liblber-2.5.so.0.1.9
libldap-2.5.so.0.1.9
liblz4.so.1.9.3
liblzma.so.5.2.5
libmd.so.0.0.5
libmvec.so.1
libnettle.so.8.4
libnghttp2.so.14
libnghttp2.so.14.20.1
libogg.so.0.8.5
libopus.so.0.8.0
libp11-kit.so.0.3.0
libpcre2-8.so.0
libpng16.so.16
libpng16.so.16.37.0
libpsl.so.5.3.2
libpulse.so.0.24.1
libpulsecommon-15.99.so
libresolv.so.2
librtmp.so.1
libsasl2.so.2.0.25
libsndfile.so.1.0.31
libssh.so.4.8.7
libssl.so.3
libstdc++.so.6
libsystemd.so.0.32.0
libtasn1.so.6.6.2
libunistring.so.2
libunistring.so.2.2.0
libvorbis.so.0.4.9
libvorbisenc.so.2.0.12
libXau.so.6
libXau.so.6.0.0
libXcursor.so.1.0.2
libXdmcp.so.6.0.0
libXext.so.6.4.0
libXfixes.so.3.1.0
libXinerama.so.1.0.0
libXrandr.so.2.2.0
libXrender.so.1.3.0
libzstd.so.1.4.8
EOF

mkdir ./ofelia/libs/

copydeps ./ofelia/ofelia.* --exclude .exclude-list -d ./ofelia/libs/

# Copy all gstreamer plugins
# mkdir ./ofelia/libs/gstreamer-1.0
#cp -rf /usr/lib/x86_64-linux-gnu/gstreamer-1.0/*.so ./ofelia/libs/gstreamer-1.0/
#
## Loop over plugins, add dependencies into same dir as ofelia's dependencies
## This will ensure we don't get doubled dependencies
#for filename in ./ofelia/libs/gstreamer-1.0/*; do
#    copydeps $filename --exclude .exclude-list -d ./ofelia/libs/
#    patchelf --set-rpath "\$ORIGIN/.." $filename
#done

# Loop over all dependencies and set their own folder as rpath
for filename in ./ofelia/libs/*; do
    patchelf --set-rpath "\$ORIGIN" $filename
done

# Set rpath for ofelia itself
patchelf --set-rpath "\$ORIGIN/libs" "$1"
