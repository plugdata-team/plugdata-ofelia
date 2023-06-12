
cd Libraries
git clone --recursive --shallow-submodules https://github.com/openframeworks/openFrameworks.git
cd openFrameworks
git reset --hard ac69b2f
C:\msys64\bin\bash.exe scripts\ci\vs\install.sh
cd ..\..
 
cd .\Resources\VisualStudio
msbuild ofelia.sln /p:configuration=release /p:PlatformToolset=v142 /p:platform=x64
MOVE  "%CD%\..\ofelia" "%CD%\..\..\ofelia"
COPY  "bin\*.dll" "%CD%\..\..\ofelia"
