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
EOF

mkdir ./ofelia/libs/

copydeps "$1" --exclude .exclude-list -d ./ofelia/libs/

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
