@echo off
IF NOT DEFINED ALS_PATH_SETUP (
   SET ALS_PATH_SETUP=1
   
   REM - These quotes are required since path has a parentheses in it, which gets interpreted
   REM   as the end of the IF ... yeah batch is awful
   set "path=x:\lib;%path%"
)

call "C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\VC\\vcvarsall.bat" x64
x:

