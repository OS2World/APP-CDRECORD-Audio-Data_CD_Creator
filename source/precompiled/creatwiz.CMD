/*                 Create the default CD-Creator config folders
 *
 *                   (C) Chris Wohlgemuth 2001-2005
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


instObj= arg(1)

/* Get curent directory */
curdir = directory()
say ''
instDir=FILESPEC("drive",curdir)||FILESPEC("path",curdir)

instDir2=LEFT(instdir, LENGTH(instdir)-1)

call SysSleep(3)

/* Check if WPS-Wizard is installed */
list.0=0
found=0
rc=SysQueryClassList(list.)
DO a=1 to list.0
	if TRANSLATE(WORD(list.a,1))='CWWIZZSETTINGS' THEN DO
		found=1
		LEAVE
	END
END

IF found=0 THEN DO
	/* WPS-Wizard not yet installed */
	call createWizInst
	exit
END

/* Create a context menu folder for Data-CD-Creator */
say 'Creating menufolder for Data-CD-Creator class...'
/*rc=SysDestroyObject("<MENU_CWDataFolder>")*/
setupstring='DEFAULTVIEW=TREE;SHOWALLINTREEVIEW=YES;CLEARMENUCACHE=1;OBJECTID=<MENU_CWDataFolder>'
if SysCreateObject("CWMenuFolder", "Context menu for^Data-CD Folder", "<CREATOR_CONFIG>",setupstring,"update")<>1 then
             do
                say ''
                say 'Menufolder not created.'
	     /*call createWizInst
	     exit*/
             end

setupstring='DEFAULTVIEW=TREE;SHOWALLINTREEVIEW=YES;OBJECTID=<SUBMENU_DWRITING>'
if SysCreateObject("CWMenuFolder", '~CD-Writing',"<MENU_CWDataFolder>",setupstring,"update") = 1 then
             do
	setupstring='SHADOWID=<CD_WRITING>'
	if SysCreateObject("WPShadow", " ","<SUBMENU_DWRITING>",setupstring,"update")<>1 then
                say 'Menuitem 1 not created.'
	setupstring='SHADOWID=<CWCREATOR_SETTINGS>'
	if SysCreateObject("WPShadow", " ","<SUBMENU_DWRITING>",setupstring,"update")<>1 then
                say 'Menuitem 2 not created.'
	setupstring='SHADOWID=<CWCREATOR_TUTORIAL>'
	if SysCreateObject("WPShadow", " ","<SUBMENU_DWRITING>",setupstring,"update")<>1 then
                say 'Menuitem 3 not created.'
	setupstring='SHADOWID=<CD_COPY>'
	if SysCreateObject("WPShadow", " ","<SUBMENU_DWRITING>",setupstring,"update")<>1 then
                say 'Menuitem 4 not created.'
	end

/* Create the folder for user defined menu items */
/*rc=SysDestroyObject("<USERMENU_DATAFOLDER>")*/
setupstring='DEFAULTVIEW=TREE;SHOWALLINTREEVIEW=YES;CLEARMENUCACHE=1;HIDECLASSPAGE=1;OBJECTID=<USERMENU_DATAFOLDER>'
if SysCreateObject("CWMenuFolder", "User menu for^Data-CD Folder", "<CREATOR_CONFIG>",setupstring,"update")<>1 then
             do
                say ''
                say 'Menufolder not created.'
	/*     call createWizInst
	     exit*/
             end
/* Fill the user menu with some default items */
setupstring='DEFAULTVIEW=TREE;SHOWALLINTREEVIEW=YES;OBJECTID=<USERMENU_DWRITING>'
if SysCreateObject("CWMenuFolder", '~CD-Writing',"<USERMENU_DATAFOLDER>",setupstring,"update") = 1 then
             do
		setupstring='SHADOWID=<CD_WRITING>'
		if SysCreateObject("WPShadow", " ","<USERMENU_DWRITING>",setupstring,"update")<>1 then
                	say 'Menuitem 1 not created.'
		setupstring='SHADOWID=<CWCREATOR_SETTINGS>'
		if SysCreateObject("WPShadow", " ","<USERMENU_DWRITING>",setupstring,"update")<>1 then
	           say 'Menuitem 2 not created.'
		setupstring='SHADOWID=<CWCREATOR_TUTORIAL>'
		if SysCreateObject("WPShadow", " ","<USERMENU_DWRITING>",setupstring,"update")<>1 then
                	say 'Menuitem 3 not created.'
		setupstring='SHADOWID=<CD_COPY>'
		if SysCreateObject("WPShadow", " ","<USERMENU_DWRITING>",setupstring,"update")<>1 then
                	say 'Menuitem 4 not created.'
	END

say 'Creating menufolder for Audio-CD-Creator class...'
/*rc=SysDestroyObject("<MENU_CWAudioFolder>")*/
setupstring='DEFAULTVIEW=TREE;SHOWALLINTREEVIEW=YES;CLEARMENUCACHE=1;OBJECTID=<MENU_CWAudioFolder>'
if SysCreateObject("CWMenuFolder", "Context menu for^Audio-CD Folder", "<CREATOR_CONFIG>",setupstring,"update")<>1 then
             do
                say ''
                say 'Menufolder not created.'
/*	     call createWizInst
	     exit*/
             end
setupstring='DEFAULTVIEW=TREE;SHOWALLINTREEVIEW=YES;OBJECTID=<SUBMENU_AWRITING>'
if SysCreateObject("CWMenuFolder", '~CD-Writing',"<MENU_CWAudioFolder>",setupstring,"update") = 1 then
             do
	setupstring='SHADOWID=<CD_WRITING>'
	if SysCreateObject("WPShadow", " ","<SUBMENU_AWRITING>",setupstring,"update")<>1 then
                say 'Menuitem 1 not created.'
	setupstring='SHADOWID=<CWCREATOR_SETTINGS>'
	if SysCreateObject("WPShadow", " ","<SUBMENU_AWRITING>",setupstring,"update")<>1 then
                say 'Menuitem 2 not created.'
	setupstring='SHADOWID=<CWCREATOR_TUTORIAL>'
	if SysCreateObject("WPShadow", " ","<SUBMENU_AWRITING>",setupstring,"update")<>1 then
                say 'Menuitem 3 not created.'
	setupstring='SHADOWID=<CD_COPY>'
	if SysCreateObject("WPShadow", " ","<SUBMENU_AWRITING>",setupstring,"update")<>1 then
                say 'Menuitem 4 not created.'
	end

/* Create the folder for user defined menu items for audio-folders */
/*rc=SysDestroyObject("<USERMENU_AUDIOFOLDER>")*/
setupstring='DEFAULTVIEW=TREE;SHOWALLINTREEVIEW=YES;CLEARMENUCACHE=1;HIDECLASSPAGE=1;OBJECTID=<USERMENU_AUDIOFOLDER>'
if SysCreateObject("CWMenuFolder", "User menu for^Audio-CD Folder", "<CREATOR_CONFIG>",setupstring,"update")<>1 then
             do
                say ''
                say 'Menufolder not created.'
/*	     call createWizInst
	     exit*/
             end
/* Fill the user menu with some default items */
setupstring='DEFAULTVIEW=TREE;SHOWALLINTREEVIEW=YES;OBJECTID=<USERMENU_AWRITING>'
if SysCreateObject("CWMenuFolder", '~CD-Writing',"<USERMENU_AUDIOFOLDER>",setupstring,"update") = 1 then
             do
		setupstring='SHADOWID=<CD_WRITING>'
		if SysCreateObject("WPShadow", " ","<USERMENU_AWRITING>",setupstring,"update")<>1 then
                	say 'Menuitem 1 not created.'
		setupstring='SHADOWID=<CWCREATOR_SETTINGS>'
		if SysCreateObject("WPShadow", " ","<USERMENU_AWRITING>",setupstring,"update")<>1 then
	           say 'Menuitem 2 not created.'
		setupstring='SHADOWID=<CWCREATOR_TUTORIAL>'
		if SysCreateObject("WPShadow", " ","<USERMENU_AWRITING>",setupstring,"update")<>1 then
                	say 'Menuitem 3 not created.'
		setupstring='SHADOWID=<CD_COPY>'
		if SysCreateObject("WPShadow", " ","<USERMENU_AWRITING>",setupstring,"update")<>1 then
                	say 'Menuitem 4 not created.'
	END
setupstring='DEFAULTVIEW=TREE;SHOWALLINTREEVIEW=YES;OBJECTID=<USERMENU_HOWTO>'
if SysCreateObject("CWMenuFolder", '~How to use this user menu',"<USERMENU_AUDIOFOLDER>",setupstring,"update") = 1 then
             do
		setupstring='EXENAME=help.cmd;PARAMETERS="'instDir'help\tutorial.inf" customization;MINIMIZED=YES'
		if SysCreateObject("WPProgram","~Open topic in tutorial","<USERMENU_HOWTO>",setupstring,"update")<>1 then
		do
                     		say 'Cannot create the object.'
           	end
		setupstring='EXENAME=e.exe;PARAMETERS="%*"'
		if SysCreateObject("WPProgram", "Show ~contents of file...","<USERMENU_HOWTO>",setupstring,"update")<>1 then
                	say 'Menuitem not created.'
		setupstring='SHADOWID=<USERMENU_AUDIOFOLDER>'
		if SysCreateObject("WPShadow", " ","<USERMENU_HOWTO>",setupstring,"update")<>1 then
                	say 'Menuitem not created.'
		setupstring='SHADOWID=<WPSWIZ_FOLDER>'
		if SysCreateObject("WPShadow", " ","<USERMENU_HOWTO>",setupstring,"update")<>1 then
                	say 'Menuitem not created.'
	end

setupstring='SHADOWID=<WPSWIZ_FOLDER>;OBJECTID=<CREATOR_SHADOW_WPSWIZ>'
if SysCreateObject("WPShadow", " ","<CREATOR_CONFIG>",setupstring,"update")<>1 then
               say 'Shadow to WPS-Wizard not created.'

/* Move menu folders to WPS-wizard config folder if not already done */
rc=SysMoveObject("<MENU_CWDataFolder>","<WPSWIZ_CONFIG>")
rc=SysMoveObject("<USERMENU_DATAFOLDER>","<WPSWIZ_CONFIG>")
rc=SysMoveObject("<MENU_CWAudioFolder>","<WPSWIZ_CONFIG>")
rc=SysMoveObject("<USERMENU_AUDIOFOLDER>","<WPSWIZ_CONFIG>")

setupstring='SHADOWID=<WPSWIZ_CONFIG>;OBJECTID=<WPSWIZ_CONFIG_SHADOW>'
rc=SysCreateObject("WPShadow", " ","<CREATOR_CONFIG>",setupstring,"update")

/******************* Toolbar stuff ************************************/
/* Create tb item folder */
SAY 'Creating toolbar item folder...'
setupstring='OBJECTID=<CWCREATOR_TBITEMFOLDER>'
if SysCreateObject("WPFolder", "Creator^Toolbar buttons","<CREATOR_CONFIG>",setupstring,"update")<>1 then
               say 'Toolbar item folder not created.'

/*+++ Create some toolbar items +++*/
/* Open CD settings */
setupstring='EXENAME='instDir'bin\toolbar\settings.cmd;MINIMIZED=YES;ICONFILE='instDir'res\cdprops.ico;OBJECTID=<CWCREATOR_TBISETTINGS>'
if SysCreateObject("WPProgram", "CD settings","<CWCREATOR_TBITEMFOLDER>",setupstring,"update")<>1 then
               say '"CD settings" item not created.'

/* Create audio cover */
setupstring='EXENAME='instDir'bin\toolbar\audcover.cmd;MINIMIZED=YES;ICONFILE='instDir'res\cover.ico;OBJECTID=<CWCREATOR_TBIAUDIOCOVER>'
if SysCreateObject("WPProgram", "Create cover^for audio CD","<CWCREATOR_TBITEMFOLDER>",setupstring,"update")<>1 then
               say '"Create cover for audio CD" item not created.'

/* Decode MP3 files */
setupstring='EXENAME='instDir'bin\toolbar\decmp3.cmd;MINIMIZED=YES;ICONFILE='instDir'res\mp3.ico;OBJECTID=<CWCREATOR_TBIDECMP3FILES>'
if SysCreateObject("WPProgram", "Decode MP3 files","<CWCREATOR_TBITEMFOLDER>",setupstring,"update")<>1 then
               say '"Create cover for audio CD" item not created.'

/* Open folder help */
setupstring='EXENAME='instDir'bin\toolbar\openhlp.cmd;MINIMIZED=YES;ICONFILE='instDir'res\help.ico;OBJECTID=<CWCREATOR_TBIFOLDERHELP>'
if SysCreateObject("WPProgram", "Open this folders help","<CWCREATOR_TBITEMFOLDER>",setupstring,"update")<>1 then
               say '"Open this folders help" item not created.'

/* Open DVD-Tools */
setupstring='EXENAME='instDir'bin\toolbar\dvdtools.cmd;MINIMIZED=YES;ICONFILE='instDir'res\cdrtools.ico;OBJECTID=<CWCREATOR_TBIDVDTOOLS>'
if SysCreateObject("WPProgram", "Open DVD Tools","<CWCREATOR_TBITEMFOLDER>",setupstring,"update")<>1 then
               say '"Open DVD Tools" item not created.'

/*******************************************************/
/*  Create launch areas                                */
/*******************************************************/
/* Create default launchpad. Don't put FPOBJECTS=... into setup string
for now. There's a bug in WPS-wizard 0.5.1 so it doesn't work properly. */
setupstring='NOTVISIBLE=NO;OBJECTID=<CWCREATOR_DEFLAUNCHPAD>;'
if SysCreateObject("CWLaunchArea", "deflaunchpad", instdir2 ,setupstring,"replace")<>1 then
               say 'Object <CWCREATOR_DEFLAUNCHPAD> not created.'
rc=SysSetObjectData('<CWCREATOR_DEFLAUNCHPAD>', 'FPOBJECTS=<CWCREATOR_SETTINGS>,<CD_WRITING>,<CWCREATOR_TUTORIAL>,'||instdir'logfiles')

/* Create folder action area for data folders. Don't put FPOBJECTS=... into setup string
for now. There's a bug in WPS-wizard 0.5.1 so it doesn't work properly. */
setupstring='NOTVISIBLE=NO;OBJECTID=<CWCREATOR_DATAFLDRACTION>;'
if SysCreateObject("CWLaunchArea", "datafldractions", instDir2,setupstring,"replace")<>1 then
               say 'Object <CWCREATOR_DATAFLDRACTION> not created.'
rc=SysSetObjectData('<CWCREATOR_DATAFLDRACTION>', 'FPOBJECTS=<CWCREATOR_TBISETTINGS>,<CDR_TOOLS>,<CWCREATOR_TBIFOLDERHELP>')

/* Create folder action area for dvd folders. Don't put FPOBJECTS=... into setup string
for now. There's a bug in WPS-wizard 0.5.1 so it doesn't work properly. */
setupstring='NOTVISIBLE=NO;OBJECTID=<CWCREATOR_DVDFLDRACTION>;'
if SysCreateObject("CWLaunchArea", "dvdfldractions", instDir2,setupstring,"replace")<>1 then
               say 'Object <CWCREATOR_DVDFLDRACTION> not created.'
rc=SysSetObjectData('<CWCREATOR_DVDFLDRACTION>', 'FPOBJECTS=<CWCREATOR_TBISETTINGS>,<CWCREATOR_TBIDVDTOOLS>,<CWCREATOR_TBIFOLDERHELP>')

/* Create folder action area for audio folders. Don't put FPOBJECTS=... into setup string
for now. There's a bug in WPS-wizard 0.5.1 so it doesn't work properly. */
setupstring='NOTVISIBLE=NO;OBJECTID=<CWCREATOR_AUDIOFLDRACTION>;'
if SysCreateObject("CWLaunchArea", "audiofldractions", instDir2,setupstring,"replace")<>1 then
               say 'Object <CWCREATOR_AUDIOFLDRACTION> not created.'
rc=SysSetObjectData('<CWCREATOR_AUDIOFLDRACTION>', 'FPOBJECTS=<CWCREATOR_TBIAUDIOCOVER>,<CWCREATOR_TBIDECMP3FILES>,<CDR_TOOLS>,<CWCREATOR_TBIFOLDERHELP>')

rc=SysDestroyObject("<INST_WPSWIZADDON>")

if instObj=1 THEN DO
say ''
SAY '<ENTER> for exit.'
say '>'
parse value SysCurPos() with aktzeile aktspalte
newposition=SysCurPos(aktzeile-1,2)
pull taste 
END
exit

createWizInst:
/* parse source . .  thisfile
SAY thisfile */

IF instObj=1 THEN
DO

'cls'
SAY ''
SAY ''
SAY 'The WPS-Wizard Addon adds some additional menu items'
SAY 'to the contextmenus of the Data- and Audio-CD-Creator'
SAY 'classes if WPS-Wizard is installed.'
SAY ''
SAY 'WPS-Wizard is a WPS extension which gives you configurable'
SAY 'context menus for any filesystem object.'
SAY ''
SAY 'Do you want to visit the WPS-Wizard homepage now?'
SAY 'Type <y> to start netscape, any other key to continue.'

say ''
say '>'
parse value SysCurPos() with aktzeile aktspalte
newposition=SysCurPos(aktzeile-1,2)
pull taste
if TRANSLATE(taste)='Y' then do
	'start netscape.exe http://www.geocities.com/SiliconValley/Sector/5785/'
END

'cls'
SAY ''
SAY 'You may restart this installation object after WPS-Wizard is installed.'
SAY '<ENTER> for exit.'
SAY ''
say '>'
parse value SysCurPos() with aktzeile aktspalte
newposition=SysCurPos(aktzeile-1,2)
pull taste


END
ELSE DO
SAY 'Creating an object for WPS-Wizard addon installation...'
setupstring='EXENAME='curdir'\creatwiz.cmd;PARAMETERS=1;OBJECTID=<INST_WPSWIZADDON>;'
if SysCreateObject("WPProgram","Install WPS-Wizard Addon","<CREATOR_CONFIG>",setupstring,"update")<>1 then
		do
                     say 'Cannot create the object.'
            end
END

return
