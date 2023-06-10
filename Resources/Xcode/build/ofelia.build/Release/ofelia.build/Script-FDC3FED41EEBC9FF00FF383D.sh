#!/bin/sh
# copy libfmod.dylib and change @executable_path to @loader_path
mkdir -p "$SRCROOT/bin/libs"
cp -rf "$SRCROOT/../../Libraries/openFrameworks;/libs/fmod/lib/osx/libfmod.dylib" "$SRCROOT/bin/libs"
install_name_tool -change @executable_path/../Frameworks/libfmod.dylib @loader_path/libs/libfmod.dylib $BUILT_PRODUCTS_DIR/ofelia

