#!/bin/bash

ENTITLEMENTS="./ofxOfelia/macOSExternal/Entitlements.plist"
ID="Developer ID Application: Timothy Schoen (7SV7JPRR2L)"

# Sign dynamic libraries
/usr/bin/codesign --force -s $ID -entitlements $ENTITLEMENTS ./ofelia/ofelia.pd_darwin
/usr/bin/codesign --force -s $ID -entitlements $ENTITLEMENTS ./ofelia/libs/libfmod.dylib

zip -vr ofelia.zip ./ofelia -x "*.DS_Store"

# Notarise zip
xcrun notarytool store-credentials "notary_login" --apple-id $AC_USERNAME --password $AC_PASSWORD --team-id "7SV7JPRR2L"
xcrun notarytool submit ./ofelia.zip --keychain-profile "notary_login" --wait
#xcrun stapler staple "ofelia.zip"