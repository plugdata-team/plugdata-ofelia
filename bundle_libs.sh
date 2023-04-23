

patchelf --set-rpath "\$ORIGIN/libs" "$1"

for filename in /Data/*.txt; do
    patchelf --set-rpath "\$ORIGIN" $filename
done

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

EOF

mkdir -p ./ofelia/libs/
copydeps "$1" --exclude .exclude-list -d ./ofelia/libs/

