#!/bin/bash

# Sign plugins
/usr/bin/codesign --force -s "Developer ID Application: Timothy Schoen (7SV7JPRR2L)" ./ofelia.pd_darwin
/usr/bin/codesign --force -s "Developer ID Application: Timothy Schoen (7SV7JPRR2L)" ./libs/libfmod.dylib

zip -r ofelia.zip ofelia

# Notarise zip
xcrun notarytool store-credentials "notary_login" --apple-id ${AC_USERNAME} --password ${AC_PASSWORD} --team-id "7SV7JPRR2L"
xcrun notarytool submit ./ofelia.zip --keychain-profile "notary_login" --wait
#xcrun stapler staple "ofelia.zip"