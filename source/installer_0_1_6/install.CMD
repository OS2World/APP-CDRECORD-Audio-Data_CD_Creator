/*           Install the Audio-CD-Folder/Data-CD-Folder 
 *
 *                   (C) Chris Wohlgemuth 1999-2001
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

/* Get curent directory */
curdir = directory()

IF SysIni('USER','cwinst','version')\='ERROR:' THEN DO
	rc=SysSetObjectData(curdir'\adc.ins',"OPEN=DEFAULT");
	if rc==1 THEN
		Exit
END
/* Clear screen */
/*'cls'*/
'inst\pmconfig.exe'
say 'rc: 'RC

if rc\=0 then exit

say 'Current directory is' curdir
say ''

say 'Registering class ''CWProgFolder''...'

if SysRegisterObjectClass("CWProgFolder",'progfldr')<>1 then
	do	 	
                say ''
		say 'Error while registering class ''CWProgFolder'''
		del boot
		say 'Exiting...'
		exit 	
	end 

audiofolderdll=curdir'\aucdfldr.dll'

say 'Registering class ''CWAudioFolder''...'
if SysRegisterObjectClass("CWAudioFolder",audiofolderdll)<>1 then
	do	 	
                say ''
		say 'Error while registering class ''CWAudioFolder'''
		say 'Deregistering class ''CWProgFolder'''
		if SysDeregisterObjectClass("CWProgFolder") <>1 then
			say 'Error while deregistering class ''CWProgFolder'''
		del boot
		say 'Exiting...'
		say 'After reboot delete file 'boot
		exit 	
	end 

say 'Registering class ''CWDataFolder''...'
if SysRegisterObjectClass("CWDataFolder",audiofolderdll)<>1 then
	do	 	
                say ''
		say 'Error while registering class ''CWDataFolder'''
		say 'Deregistering class ''CWAudioFolder'''
		if SysDeregisterObjectClass("CWAudioFolder") <>1 then
			say 'Error while deregistering class ''CWAudioFolder'''
		say 'Deregistering class ''CWProgFolder'''
		if SysDeregisterObjectClass("CWProgFolder") <>1 then
			say 'Error while deregistering class ''CWProgFolder'''
		del boot
                del audiofolderdll
		say 'Exiting...'
		say 'After reboot delete file 'boot
		exit 	
	end 

say 'Registering class ''CWCreatorSettings''...'
if SysRegisterObjectClass("CWCreatorSettings",audiofolderdll)<>1 then
	do	 	
                say ''
		say 'Error while registering class ''CWCreatorSettings'''
		say 'Deregistering class ''CWDataFolder'''
		if SysDeregisterObjectClass("CWDataFolder") <>1 then
			say 'Error while deregistering class ''CWDataFolder'''
		say 'Deregistering class ''CWAudioFolder'''
		if SysDeregisterObjectClass("CWAudioFolder") <>1 then
			say 'Error while deregistering class ''CWAudioFolder'''
		say 'Deregistering class ''CWProgFolder'''
		if SysDeregisterObjectClass("CWProgFolder") <>1 then
			say 'Error while deregistering class ''CWProgFolder'''
		del boot
                del audiofolderdll
		say 'Exiting...'
		say 'After reboot delete file 'boot
		exit 	
	end 


/* Create all WPS-objects */
createskript=curdir'\inst\creatobj.cmd'
rc=SysSetObjectData(createskript, "OPEN=DEFAULT")

/* Open the CD-writing folder */
rc=SysSetObjectData("<CD_WRITING>","OPEN=DEFAULT")



/*say ''
say 'The Audio-CD-Creator and Data-CD-Creator'
say 'were installed succesful.'
say 'You''ll find templates in your templates folder for creating'
say 'Audio-CD and Data-CD projects.'
say 'Before starting to write you have to enter the path to each helper'
say 'program in the settings notebook!'
say ''*/
say ''
say 'If you did an update installation it may be necessary to restart the WPS.'
say ''
say 'Have fun :-) !!'

say ''
say ''
say 'Press <ENTER>'
say ''
say '>'
parse value SysCurPos() with aktzeile aktspalte
newposition=SysCurPos(aktzeile-1,2)
pull

exit

