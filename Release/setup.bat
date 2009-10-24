@echo off

del "C:\Games\Warcraft 3\antihack.dll"
del "C:\Games\Warcraft 3\Loader.exe"

copy antihack.dll "C:\Games\Warcraft 3\"
copy Loader.exe "C:\Games\Warcraft 3\"

cd "C:\Games\Warcraft 3\"
Loader.exe