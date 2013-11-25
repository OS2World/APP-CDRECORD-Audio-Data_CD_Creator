/*                 Uninstall the Audio-CD-Folder/Data-CD-Folder
 *
 *                   (C) Chris Wohlgemuth 1999
*/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

 call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
 call SysLoadFuncs

IF SysIni('USER','cwinst','version')\='ERROR:' THEN DO
	rc=SysSetObjectData("<CWINST_PROGRAM>","OPEN=DEFAULT");
	if rc==1 THEN
		Exit
END

'cls'
say ''
Say 'About to uninstall the Audio-CD-Creator'
Say 'and Data-CD-Creator'
Say '(c) Chris Wohlgemuth 1998-2001'
say 'This program and all files are released under the GPL'
say 'See file ''Copying'' for information.'
say ''

say 'Press <ENTER> to uninstall or Ctrl-c to abort now.'
say ''
say '>'
parse value SysCurPos() with aktzeile aktspalte
newposition=SysCurPos(aktzeile-1,2)
pull

/* Clear screen */
'cls'

/* Get curent directory */
curdir = directory()

say 'Current directory is' curdir
say ''

/* Query bootdrive */
say 'Quering boot drive...'
parse upper value VALUE('PATH',,'OS2ENVIRONMENT') with '\OS2\INSTALL' -2 boot +2
parse upper value VALUE('PATH',,'OS2ENVIRONMENT') with '\OS2\SYSTEM' -2 boot2 +2
say ''
if boot=boot2 then say 'Bootdrive seems to be' boot
say 'Enter ''y'' to confirm or the real bootdrive letter'
say ''
say '>'
parse value SysCurPos() with aktzeile aktspalte
newposition=SysCurPos(aktzeile-1,2)
parse pull in

if in='y'|in='Y'  then boot=boot
else boot=in':'


/* Clear screen */
'cls'

progfolderdll=boot'\os2\dll\progfldr.dll' /* Build progfldr.dll path */
helppath=boot'\os2\help\aucdfldr.hlp'

say ''
say ''
say 'Deleting CD-Creator Settings object...'
if SysDestroyObject('<CWCREATOR_SETTINGS>') <>1 then
	say 'Can''t delete the Settings object!'

say 'Deleting CD-Writing-Utilities folder...'
if SysDestroyObject('<CD_WRITING>') <>1 then
	say 'Can''t delete the folder.'

say 'Deleting the Audio-CD-Creator template in the templates folder...'
if SysCreateObject("CWAudioFolder","Audio-CD-Creator","<WP_TEMPS>","OBJECTID=<AUDIOCD_CREATOR>;NODELETE=NO;TEMPLATE=NO","update") then
      if SysDestroyObject('<AUDIOCD_CREATOR>') <>1 then
	say 'Can''t delete the template.'

say 'Deleting the Data-CD-Creator template in the templates folder...'
if SysCreateObject("CWDataFolder","Data-CD-Creator","<WP_TEMPS>","OBJECTID=<DATACD_CREATOR>;NODELETE=NO;TEMPLATE=NO","update") then
      if SysDestroyObject('<DATACD_CREATOR>') <>1 then
	say 'Can''t delete the template.'

say 'Deregistering class ''CWAudioFolder'''
		if SysDeregisterObjectClass("CWAudioFolder") <>1 then
		do
		  say 'Error while deregistering class ''CWAudioFolder'''
                  say 'Exiting...'
		  exit
		 end

say 'Deregistering class ''CWDataFolder'''
		if SysDeregisterObjectClass("CWDataFolder") <>1 then
		do
		  say 'Error while deregistering class ''CWDataFolder'''
		  say 'Exiting...'
		  exit 	
		end

say 'Deregistering class ''CWProgFolder'''
		if SysDeregisterObjectClass("CWProgFolder") <>1 then
		do
		  say 'Error while deregistering class ''CWProgFolder'''
		  say 'Exiting...'
		  exit 	
		end
say 'deleting helpfile: 'helppath
del helppath

say ''
say 'After reboot delete the file 'progfolderdll' and this directory.'


say ''
say 'The Audio-CD-Creator was uninstalled succesful.'
say ''

say 'Press <ENTER> to quit.'
say ''
say '>'
parse value SysCurPos() with aktzeile aktspalte
newposition=SysCurPos(aktzeile-1,2)
pull

exit

