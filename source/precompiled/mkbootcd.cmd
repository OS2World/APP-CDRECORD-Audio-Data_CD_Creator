/* rexx */

/* create an ISO image of the current directory and make it bootable
   if this program is named mkbootcd.cmd (!) instead of mkcd.cmd
   */


/* configure */
cdbootimg = 'cdboot.img'
TracksDir = 'd:\Tracks'
parameters =  '-J -l -L -d -D -N -U -r'

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

'@echo off'

/* first, get the current directory */
curDir = directory()
/* if we are in the root directory, use the volume label as the
   CD-ROM's name
   */
if length(curDir)=3 then
   parse value SysDriveInfo(left(curDir,2)) with . . . cdname
else
   cdname = substr(curDir,lastpos('\',curDir)+1)

cdname = strip(cdname)
Trackfile = TracksDir'\'cdname'.trk'
if fileexist(Trackfile) then
 'del 'Trackfile

parse source . . me
myname = filespec('n',me)
if translate(myname) = 'MKBOOTCD.CMD' then
do
  bootString = '-b 'cdbootimg
  if \fileexist(cdbootimg) then
  do
    say '!!!No File "'cdbootimg'" found!!!'
    'pause'
    exit(1)
  end
end
else
  bootString = ''

'echo on'
'mkisofs.exe 'bootString' 'parameters' -volid 'cdname' -o 'Trackfile' -v .'
if \fileexist(Trackfile) then
do
  Say '!!!No File "'Trackfile'" found!!!'
  'pause'
  exit(2)
end
'cdrecord.exe speed=4 -eject 'Trackfile
'@echo off'
exit(0)

/*--------------------------------------------------------------------*/
fileexist:
parse arg file

rc = SysFileTree(file, 'data.','F')
if data.0 == 0 then
   return 0
else
   return 1


