@echo off

REM - Needed to append to variable inside for loop
setlocal enabledelayedexpansion

SET AllCppFiles=.
for %%f in (x:\code\*.cpp) do (
	SET AllCppFiles=!AllCppFiles! %%f
)

for %%f in (x:\code\stb\*.cpp) do (
	SET AllCppFiles=!AllCppFiles! %%f
)

REM - Remove first two characters
REM   First character was a dummy '.' because idk if we can have an empty variable in batch scripting
REM   Second character is a leading space when appending the '.' to the first filename
REM   This might cause error if there were 0 files (the length would only be 1, the '.'), but we should never have 0 files
SET AllCppFiles=%AllCppFiles:~2%

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM reenable warning 4244
SET CompilerFlags= /Zi /I x:\code /EHsc /WX /W4 /wd4100 /wd4201 /wd4244 /wd4267 /wd4305 /wd4189 /FC
SET LinkerFlags= x:\lib\glfw3dll.lib x:\lib\glew32.lib opengl32.lib /NOLOGO

call ..\code\shell_setup.bat

REM - kill the game if it is running so we can write the .exe
REM   pipes the output to null, so it isn't seen
taskkill /IM "game.exe" /f >nul 2>&1


cl %CompilerFlags% %AllCppFiles% /link %LinkerFlags% /out:game.exe

popd
