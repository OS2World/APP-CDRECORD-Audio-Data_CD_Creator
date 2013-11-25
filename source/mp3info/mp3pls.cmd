/* OS/2 REXX 
 *
 * Simple script for generation .PLS files used by WinAMP and 
 * also understands by PM123
 */

call rxfuncadd sysloadfuncs, rexxutil, sysloadfuncs
call sysloadfuncs

'@echo off'
'del album.pls 1>nul 2>nul'

call SysFileTree "*.mp3", "files", "O"
call lineout "album.pls", "[playlist]"
call stream  "album.pls", "c", "close"

do i = files.0 to 1 by -1 until flip_flop = 1
  flip_flop = 1
  do j = 2 to i
    m = j - 1
    if files.m >> files.j then
    do
      xchg    = files.m
      files.m = files.j
      files.j = xchg
      flip_flop = 0
    end /* if files.m ... */
  end /* do j = 2 ... */
end /* do i = files.0 ... */

do i = 1 to files.0
   call charout ,"."
   'mp3info -f "; %%a - %%l (%%y) - %%t (%%mm:%%ss)%%NFile'i'=%%B" "'files.i'" >> album.pls'
end

call lineout "album.pls", "NumberOfEntries="files.0
say

