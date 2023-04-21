cd .\ofxOfelia\WindowsExternal
msbuild ofelia.sln /p:configuration=release /p:PlatformToolset=v142 /p:platform=x64
DEL ".\bin\pd.dll"
DEL ".\bin\pthreadVC3.dll"
COPY  "..\ofelia" "%CD%\..\..\ofelia"
COPY  ".\bin\*.dll" "%CD%\..\..\ofelia\"
