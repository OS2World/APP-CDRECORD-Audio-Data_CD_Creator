@echo off
if x%2x == xx goto setfile
set filename=%2
goto chk2
:setfile
set filename=cdboot.img

:chk2
if x%1x == xx goto setdl
set driveletter=%1
goto doit
:setdl
set driveletter=O:

:doit
echo on
savedskf %driveletter% %filename% /D /A
