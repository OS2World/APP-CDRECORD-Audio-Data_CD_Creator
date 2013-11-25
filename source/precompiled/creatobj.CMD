/*           Create all WPS-objects for Audio-CD-Folder/Data-CD-Folder 
 *
 *                   (C) Chris Wohlgemuth 1999-2002
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
say 'Current directory is' curdir
say ''
instDir=FILESPEC("drive",curdir)||FILESPEC("path",curdir)
instdir2=LEFT(instdir,LENGTH(instdir)-1)
say 'Installation directory is '||instDir

helpfile='aucdfldr.hlp'

say 'Customizing the template in the templates folder...'
if SysCreateObject("CWAudioFolder","Create Audio-CD","<WP_TEMPS>","TEMPLATE=YES;FLDRWRITEFLAGS=11;BACKGROUND="instDir"Res\music.jpg,T,,I;WIZSETREXXSCRIPT="instdir"Scripts\cwaudiofolder.rx;OBJECTID=<AUDIOCD_CREATOR>;","update")<>1 then
             say '     Can''t update the template.'
if SysCreateObject("CWDataFolder","Create Data-CD","<WP_TEMPS>","TEMPLATE=YES;FLDRFILENAMEOPTIONS=31;FLDRCREATEOPTIONS=1;FLDRCDTYPEOPTIONS=520;BACKGROUND="instDir"Res\DATACD.jpg,S,1,I;WIZSETREXXSCRIPT="instdir"Scripts\cwdatafolder.rx;OBJECTID=<DATACD_CREATOR>;","update")<>1 then
             say '     Can''t update the template.'


say 'Creating a CD-Writing folder on the desktop...'
setupstring='ICONFILE='instDir'Res\cdrfold1.ico;ICONNFILE=1,'instDir'Res\cdrfold2.ico;HELPLIBRARY='helpfile';HELPPANEL=21;OBJECTID=<CD_WRITING>'
if SysCreateObject("WPFolder","CD-Writing Utilities","<WP_DESKTOP>",setupstring,"update")<>1 then
             do
                say ''
                say 'Can''t create the folder. Exiting...'
		say 'You''ll find a template in your templates folder.'
		exit
             end

/* Create an object for CDR-Tools */
setupstring='EXENAME='instDir'bin\cdrtools.cmd;STARTUPDIR='instDir';MINIMIZED=YES;ICONFILE='instDir'Res\cdrtools.ico;HELPLIBRARY='helpfile';HELPPANEL=23;OBJECTID=<CDR-Tools>;'
say 'Creating an object for CDRTools...'
if SysCreateObject("WPProgram","CDR-Tools","<CD_WRITING>",setupstring,"update")<>1 then
		do
              say ''
              say 'Cannot create a program object for CDR-Tools.'
              say 'You can find CDRTools in 'curdir
              say '' 
            end


say 'Creating a shadow for the documentation folder...'
setupstring='SHADOWID='instDir'Docs'
if SysCreateObject("WPShadow","Docs","<CD_WRITING>",setupstring,"update")<>1 then
	do
              say 'Cannot create the shadow for the folder.'
	      say 'Perhaps on FAT-Drive not HPFS...'
	end


say 'Creating a shadow of the Audio-CD-Creator in the CD-Writing folder...'
if SysCreateObject("WPShadow","Create Audio-CD","<CD_WRITING>","OBJECTID=<ACD_SHADOW>;SHADOWID=<AUDIOCD_CREATOR>","update")<>1 then
              say 'Cannot create the shadow.'


say 'Creating a shadow of the Data-CD-Creator in the CD-Writing folder...'
if SysCreateObject("WPShadow","Create Data-CD","<CD_WRITING>","OBJECTID=<DCD_SHADOW>;SHADOWID=<DATACD_CREATOR>","update")<>1 then
              say 'Cannot create the shadow.'

/* Create the CD-Creator Settings object */
setupstring='OBJECTID=<CWCREATOR_SETTINGS>'
say 'Creating CD-Creator Settings object...'
if SysCreateObject("CWCreatorSettings","CD-Creator Settings","<CD_WRITING>",setupstring,"update")<>1 then
		do
              say ''
              say 'Cannot create a CD-Creator Settings object'
              say '' 
            end

/* Create the tutorial object */
say 'Creating an object for the tutorial in the CD-Writing folder...'
setupstring='EXENAME=view.exe;PARAMETERS='instDir'Help\tutorial.inf;ICONFILE='instDir'Res\tutorial.ico;HELPLIBRARY='helpfile';HELPPANEL=20;OBJECTID=<CWCREATOR_TUTORIAL>'
if SysCreateObject("WPProgram","CD writing tutorial","<CD_WRITING>",setupstring,"update")<>1 then
              say 'Can''t create the tutorial object.'


Say 'Creating Shadows for the CDRecord/2-, mkisofs- and cdda2wav docs...'
/* Get CDRecord/2 path */
cdrini=instDir'cdrecord.ini'
cdrpath=SysIni(cdrini,'CDWriter','cdrecord')
/* Do we have a drive? */
cdrdrive=FILESPEC("drive",cdrpath)
if cdrdrive \='' then
DO
  target=instDir'Docs'
  /* Build path to cdrecord.ps */
  cdrps=cdrdrive||FILESPEC("path",cdrpath)'doc\cdrecord.ps'
  setupstring='SHADOWID='cdrps
  if SysFileTree(cdrps,stamm,'F') =0 then 
  DO
    IF stamm.0=1 THEN  rc=SysCreateObject("WPShadow","cdrecord.ps",target,setupstring,"update")
  END

  mkisofsps=cdrdrive||FILESPEC("path",cdrpath)'doc\mkisofs.ps'
  setupstring='SHADOWID='mkisofsps
  if SysFileTree(mkisofsps,stamm,'F') =0 then
  DO
    IF stamm.0=1 THEN  rc=SysCreateObject("WPShadow","mkisofs.ps",target,setupstring,"update")
  END

  cdda2wavps=cdrdrive||FILESPEC("path",cdrpath)'doc\cdda2wav.ps'
  setupstring='SHADOWID='cdda2wavps
  if SysFileTree(cdda2wavps,stamm,'F') =0 then
  DO
    IF stamm.0=1 THEN  rc=SysCreateObject("WPShadow","cdda2wav.ps",target,setupstring,"update")
  END
END


setupstring='EXENAME='instDir'bin\Drrexx.exe;PARAMETERS='instDir'bin\cdcopy.res;ICONFILE='instDir'Res\pmaudcpy.ico;HELPLIBRARY='helpfile';HELPPANEL=22;OBJECTID=<CD_COPY>;'
say 'Creating an object for 1:1 CD-Copy...'
if SysCreateObject("WPProgram","1:1 CD-Copy","<CD_WRITING>",setupstring,"update")<>1 then
		do
              say ''
              say 'Cannot create a program object for pmaudcpy.exe.'
              say 'You can find pmaudcpy.exe in 'curdir'\bin'
              say '' 
            end


say 'Creating an installation and configuration folder...'
setupstring='ICONFILE='instDir'Res\cdtools1.ico;ICONNFILE=1,'instDir'Res\cdtools2.ico;HELPLIBRARY='helpfile';HELPPANEL=24;OBJECTID=<CREATOR_CONFIG>;'
if SysCreateObject("WPFolder","Configuration and^(Un)Installation","<CD_WRITING>",setupstring,"update")<>1 then
             do
                say ''
                say 'Can''t create the configuration folder.'
             end

SAY 'Creating an object for uninstallation...'
setupstring='EXENAME='instDir'inst\uninstal.cmd;OBJECTID=<CREATOR_UNINSTALL>;'
if SysCreateObject("WPProgram","Uninstall CD-Creator",curdir,setupstring,"update")=1 then
		do
		setupstring='SHADOWID=<CREATOR_UNINSTALL>;OBJECTID=<CDCUNINSTALL_SHADOW>;'
		if SysCreateObject("WPShadow","Uninstall CD-Creator","<CREATOR_CONFIG>",setupstring,"update")<>1 then
                     say 'Cannot create the object.'
            end

SAY 'Creating an object for recreating the WPS-objects...'
setupstring='EXENAME='curdir'\creatobj.cmd;NOAUTOCLOSE=YES;HELPLIBRARY='helpfile';HELPPANEL=25;OBJECTID=<CREATOR_RECREATEOBJ>;'
if SysCreateObject("WPProgram","Recreate WPS-Objects","<CREATOR_CONFIG>",setupstring,"update")<>1 then
		do
                     say 'Cannot create the object.'
            end

SAY 'Creating an object for MP3 information...'
setupstring='EXENAME='instDir'\bin\pmwvinfo.exe;PARAMETERS="'instdir2'" 1 "%*";ASSOCFILTER=*.MP3;OBJECTID=<CWINFO_MP3>'
if SysCreateObject("WPProgram","MP3-file Info", instdir'bin' ,setupstring,"update")<>1 then
		do
                     say 'Cannot create the object.'
            end

SAY 'Creating an object for wave information...'
setupstring='EXENAME='instDir'\bin\pmwvinfo.exe;PARAMETERS='instdir' 0 "%*";ASSOCFILTER=*.WAV;OBJECTID=<CWINFO_WAVE>'
if SysCreateObject("WPProgram","Wavefile Info",instdir'bin',setupstring,"update")<>1 then
		do
                     say 'Cannot create the object.'
            end
SAY 'Creating an object for MP3-Decoding...'
setupstring='EXENAME='instDir'\bin\pmmp3dec.exe;PARAMETERS="'instdir2'" 1 "%*" "%**P";ASSOCFILTER=*.MP3;ICONFILE='instDir'Res\mp3.ico;OBJECTID=<CWDECODE_MP3>'
if SysCreateObject("WPProgram","Decode MP3",instdir'bin',setupstring,"update")<>1 then
		do
                     say 'Cannot create the object.'
            end

SAY 'Creating an URL-object to the homepage ...'
setupstring='BROWSER=netscape.exe;URL='instDir'Help\cdwriting.htm;OBJECTID=<CWURL_CDWRITING>'
if SysCreateObject("WPUrl","CD-writing with OS/2^and add-ons on the Internet", "<CD_WRITING>" ,setupstring,"update")\=1 then
		do
                     say 'Cannot create the object.'
            end

SAY 'Trying to install WPS-Wizard addon...'
createskript=curdir'\creatwiz.cmd'
rc=SysSetObjectData(createskript, "OPEN=DEFAULT")


say ''
say 'Have fun :-) !!'

exit

