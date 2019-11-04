wget https://download.sysinternals.com/files/Handle.zip
7z x Handle.zip
for /F "tokens=3,6 delims=: " %%I IN ('handle.exe -accepteula osgeo4w') DO handle.exe -c %%J -y -p %%I
del handle.exe
del Handle.zip
