� �� 0�  d   install�   �   page1�   �   �   �  �  ,  cdrecord�  �  �  �  �  �  �  �  �  �  -  �  base�  �  �  �  �  �  �  emx�  �  �  �  �  �              	      �  X  aspi\  ]  ^  b  _  `  a  c  Z  [  Y  �  mkisofs�  �  �  �  �  �  �  �  �  �  -     emxrev$  )  &  (  #  "  *  !  ��� 0�  checkForCdrTools2�thePath=ARG(1)

IF LENGTH(thePath)<6 THEN return 0

if TRANSLATE(RIGHT(thePath, 5))="2.EXE" THEN RETURN 1
ELSE
	RETURN 0
checkCDRVersion�ver=ARG(1)

IF ver="" then return 0

IF TRANSLATE(WORD(ver, 1))\="CDRECORD" THEN return 0

IF POS("EMX", TRANSLATE(ver))=0 THEN return 1

SELECT
	WHEN TRANSLATE(WORD(ver, 2))="1.11A05" THEN return 2

/* Not supported: */
	WHEN TRANSLATE(WORD(ver, 2))="1.11A16" THEN return 1
	WHEN TRANSLATE(WORD(ver, 2))="1.11A17" THEN return 1
	WHEN TRANSLATE(WORD(ver, 2))="1.11A18" THEN return 1
	WHEN TRANSLATE(WORD(ver, 2))="1.11A16" THEN return 1
	WHEN TRANSLATE(WORD(ver, 2))="1.11A13" THEN return 1
OTHERWISE
	return 2
END
init�call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

PARSE SOURCE  . . sourceFile

installDir=FILESPEC("drive", sourceFile)||FILESPEC("path", sourceFile)
installDir=LEFT(installDir, LENGTH(installDir)-1)
global._installDir=FILESPEC("drive", installDir)||FILESPEC("path", installDir)

classDLL=global._installDir||'AUCDFLDR.DLL'

global._iniFile=global._installDir||'cdrecord.ini'

global._cdrecordPath=SysIni(global._iniFile, "CDWriter", "cdrecord")

if global._cdrecordPath="ERROR:" THEN
	global._cdrecordPath=""
ELSE
	global._cdrecordPath=STRIP(TRANSLATE(global._cdrecordPath, " ", '0'x))

global._mkisofsPath=SysIni(global._iniFile, "Mkisofs", "mkisofs")

if global._mkisofsPath="ERROR:" THEN
	global._mkisofsPath=""
ELSE
	global._mkisofsPath=STRIP(TRANSLATE(global._mkisofsPath, " ", '0'x))

/* Get bootdrive */
 bootdrv = translate(value('PATH', , 'os2environment'))
 parse var bootdrv bootdrv':\OS2;'rest
 globalData._bootDrive=substr(bootdrv, length(bootdrv))||":"
�� �d 0f   f   �  ��          J   �# 0 ?� d ��]         a   �  1  � ����Check installation    Exit ���d 0H   �d Open-call page1.open
call page1.Owner "Install"
�� ClickEXIT(0)�� �� 0�  �  �  ��           �    # P ;� � ���        � � @� � )* � ����        f@ � K > � n��        �@ �  ' � ����        �  ��  5  �����         �  �  4� �����     Check your installation on the following pages. If you find problems read the tutorial for trouble shooting. Note that this is only a very basic test program! Warning +                  12.Helvetica Bold      URL Next  ���� 0�  �� Open	call showInitIpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+60)
��Click�/* Close this dialog page */
call closeFOR Dialog() 

/* open new dialog page and display it */
call base.open
call base.Owner "Install"
�� Init�
theString="http://www.os2world.com/cdwriting"
theString=theString||'0D'x||"http://www.geocities.com/SiliconValley/Sector/5785/"
call text theString�� Init�theString="Be aware that all Cdrtools2 (note the '2') packages floating around are not compatible with "
theString=theString||"Audio/Data-CD-Creator because of some arbitrary introduced limitations. Use ONLY Cdrecord releases from:"
call text theString
�� Init�theString="Check your installation on the following pages. If you find problems read the tutorial for trouble shooting."
theString=theString||'0d'x'0d'x||TRANSLATE('Note that this is only a very limited test tool!')
call text theString

�� �,0�  �  �  ��           v   � � <� ,��w        {� � b  �����         �, � � 
 �����        � � � 5  �����         �@ � z �  ����        � � m X  �����        �   � x L  �����        �@ � V  �����         �@ �  : �����        �  ��  5  �����        �  ��  4  �����       
 �  �	  *� -����     Path:  Path valid:              Version check: Open settings... Please wait...  Prev Next CDRecord/2 ���,0e
  �,MoveIpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+50)
Open�call show

call cdrecord.c411.text("OK")
haveCDRecord=YES

IF global._cdrecordPath="" THEN DO
	call cdrecord.c411.color "+", "#255 0 0"
	call cdrecord.c411.text("Path to CDRecord/2 is not valid!  Set the right path in the settings note book.")
	call cdrecord.c412.show
	haveCDRecord=NO
END
ELSE DO
if STREAM(global._cdrecordPath, "C","Query exists")="" THEN DO
	call cdrecord.c411.color "+", "#255 0 0"
	call cdrecord.c411.text("Path to CDRecord/2 is not valid! Set the right path in the settings note book.")
	call cdrecord.c412.show
	haveCDRecord=NO
END
ELSE
	call cdrecord.c411.text("Path OK")
END

if haveCDRecord=YES THEN DO
	newQ=RXQUEUE('CREATE')
	oldq=RXQUEUE('SET', newQ)
	theCmd=global._cdrecordPath||' -version | RXQUEUE '||newq||' /FIFO'

	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END
	'RXQUEUE 'newq' /CLEAR'

	'@start "Version check" /C /B /MIN cmd.exe /C 'theCmd
	version=""
	DO WHILE QUEUED()\=0
		PULL str
		version=version||str
	END
	ret=RXQUEUE('SET', oldq)
	res=RXQUEUE('DELETE', newQ)
	call c415.text version

	ret=checkCDRVersion(version)

	SELECT
		WHEN ret=0 THEN DO
			call c416.text "The program is not cdrecord.exe!"
			call cdrecord.c416.color "+", "#255 0 0"
		END
		WHEN ret=1 THEN DO
			call c416.text "This version of cdrecord.exe is not compatible with Audio/Data-CD-Creator!"
			call cdrecord.c416.color "+", "#255 0 0"
		END
	OTHERWISE
		call cdrecord.c416.color "+", "#0 0 255"
	END

	ret=checkForCdrTools2(global._cdrecordPath)

	IF ret=1 THEN DO
		call c416.text "This version of cdrecord.exe is not compatible with Audio/Data-CD-Creator! Get a working release from:"||'0d'x||'0d'x||"   http://www.os2world.com/cdwriting"||'0d'x||"   http://www.geocities.com/SiliconValley/Sector/5785/"
		call cdrecord.c416.color "+", "#255 0 0"
	END
END
Initcpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+60)

call Owner "install"

��Click�/* Close this dialog page */
call closeFOR Dialog() 

/* open new dialog page and display it */
call mkisofs.open
call mkisofs.Owner "Install"
��Click�/* Close this dialog page */
call closeFOR Dialog() 

/* open new dialog page and display it */
call aspi.open
call aspi.Owner "Install"
��Click�ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "DISPLAYHELPPANEL=300")
ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "OPEN=DEFAULT")��Init�call Range 300
call text global._cdrecordPath

call cdrecord.c411.color "+", "#0 0 255"
call cdrecord.c411.text("")
call cdrecord.c412.hide

�� ��0�  �  �  ��           �    � R ;� ����         � � � �  �����       k @ �  ! �����        q  �r g L  �����        �  ��  7  �����        �  ��  7  �����        �  �  -� �����     Objects, files and WPS classes: Use the Generic Installer to check the files, objects and WPS classes of the Audio/Data-CD-Creator package. Open installer Prev Next Step 1 ����0z  ��MoveIpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+50)
Open	call showInitIpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+60)
��Click�/* Close this dialog page */
call closeFOR Dialog() 

/* open new dialog page and display it */
call emx.open
call emx.Owner "Install"
��Click�/* Close this dialog page */
call closeFOR Dialog() 

/* open new dialog page and display it */
call page1.open
call page1.Owner "Install"
��Click�cwinst=SysIni("USER","cwinst", "instdir")
IF cwinst\="ERROR:" THEN DO

rc=SysSetObjectData(cwinst||"\install.exe", "OPEN=DEFAULT")
END�� ��0�  �  �  ��              � s 8� ���        � � _  �����       	  � � _  �����        # � � _  �����        / � r _  �����        < � c _  �����        I � T _  �����         U �s � s  ����         V �s � s  ����         W �s � t  ����         X �s r s  ����         Y �s c s  ����         Z �s T s  ����       W [@    1 	���        �  �� 
 5  ����        �  �� 
 6  ����        �  �  ,� �����     EMX.DLL EMXIO.DLL EMXLIBC.DLL EMXLIBCM.DLL EMXLIBCS.DLL EMXWRAP.DLL       The EMX runtime libraries are not in your libpath. Correct this error before proceding.             Prev Next Check EMX runtime library path ����0�  ��MoveIpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+50)
Open�call show

/* Check EMX runtime libraries */
call SysFileSearch "LIBPATH", globalData._bootDrive||"\config.sys", "info."
a=info.0
theLibPath=info.a
thePos=POS('=',theLibPAth)
theLibPath=RIGHT(theLibPath, LENGTH(theLibPath)-thePos)
newEnv=VALUE("CWLIBPATH", theLibPath,"OS2ENVIRONMENT")
theLibPath=VALUE("CWLIBPATH",,"OS2ENVIRONMENT")


DO a=3 to 8
ctrl="c503"
dlg="emx"
ret=SysSearchPath("CWLIBPATH", TextFor(emx, "C"||500+a))
IF ret\="" THEN
	ret=textFor(emx, "C"||510+a, "OK")
ELSE DO
	ret=textFor(emx, "C"||510+a, "Missing!")
	ret=colorFor(emx, "C"||510+a, "+", "#255 0 0")
	call c520.disable
	call c521.show
END
END

newEnv=VALUE("CWLIBPATH","","OS2ENVIRONMENT")
InitIpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+60)
�Click�/* Close this dialog page */
call closeFOR Dialog() 

/* open new dialog page and display it */
call emxrev.open
call emxrev.Owner "Install"
�Click�/* Close this dialog page */
call closeFOR Dialog() 

/* open new dialog page and display it */
call base.open
call base.Owner "Install"
�� �X0�  �  �  ��           v   � [ 7� X��w        {� � h  \����        � � � 5  ]����         �, �L � � 
 ^����         �@ � y  b���        � � b h  _����        � � Q 5  `����         � �N R � 
 a����         �@ � 3  c���        �  ��  5  Z����        �  ��  5  [����        �  �  +� Y����     OS2ASPI.DMD: Found:               ASPIROUT.SYS: Found:               Prev Next ASPI installation ���X0�  �XOpen�call show

/* Look for OS2ASPI.DMD */
call SysFileSearch "BASEDEV", globalData._bootDrive||"\config.sys", "info.", "N"
dmdLine=0
noAllSwitch=0
DO a=1 to info.0
	IF POS("OS2ASPI.DMD", TRANSLATE(STRIP(SUBWORD(info.a,2))))\=0 THEN DO

		if POS("BASEDEV",TRANSLATE(STRIP(SUBWORD(info.a,2))))=1 then DO
			dmdLine=STRIP(WORD(info.a,1))
			call c606.text STRIP(SUBWORD(info.a,2))
			IF POS("/ALL", TRANSLATE(STRIP(SUBWORD(info.a,2))))=0 THEN
				noAllSwitch=1
		END
	END
END

if dmdLine=0 THEN
	call c610.text "BASEDEV=OS2ASPI.DMD /ALL is not installed. This driver must be present in Config.sys!"
ELSE DO
if noAllSwitch=1 THEN
	call c610.text "The switch /ALL must be added to the line BASEDEV=OS2ASPI in Config.sys."
END

/* Look for ASPIROUT.SYS */
aspiLine=0
call SysFileSearch "DEVICE", globalData._bootDrive||"\config.sys", "info.", "N"
DO a=1 to info.0
	IF POS("ASPIROUT.SYS", TRANSLATE(STRIP(SUBWORD(info.a,2))))\=0 THEN DO
		/* ASPIROUT.SYS found */
		if POS("DEVICE",TRANSLATE(STRIP(SUBWORD(info.a,2))))=1 then DO
			aspiLine=STRIP(WORD(info.a,1))
			call c609.text STRIP(SUBWORD(info.a,2))
		END
	END
END

if aspiLine=0 THEN
	call c611.text "The ASPI-Router ASPIROUT.SYS is not installed. This driver must be present in Config.sys!"
ELSE DO
	/* Check the Path */
	thePath=c609.text()
	thePos=POS("=", thePath)
	IF thePos\=0 THEN DO
		thePath=RIGHT(thePAth, LENGTH(thePath)-thePos)
		/* Remove parameters if any */
		thePos=POS("/", thePath)
		IF thePos\=0 THEN
			thePath=LEFT(thePath, thePos-1)
		IF STREAM(thePath, "C", "query exists")="" THEN
			call c611.text "The ASPIROUT.SYS path specified in Config.sys is not valid!"

	END
END
MoveIpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+50)
InitIpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+60)
�[Click�/* Close this dialog page */
call closeFOR Dialog() 

/* open new dialog page and display it */
call cdrecord.open
call cdrecord.Owner "Install"
�ZClick�/* Close this dialog page */
call closeFOR Dialog() 

/* open new dialog page and display it */
call emxrev.open
call emxrev.Owner "Install"
�aInitcall range 255�^Initcall range 255�� ��0�  �  �  ��           v   � � <� ���w        {� � b  �����         �, � � 
 �����        � � � 5  �����         �@ � z �  ����        � � n Y  �����        �   � x L  �����        �@ � Z  �����         �@ �  = �����        �  ��  5  �����        �   �  4  �����        �  �	  *� -����     Path:  Path valid:              Version check: Open settings... Please wait...  Prev Next Mkisofs ����0V	  ��MoveIpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+50)
Open�call show

call c711.color "+", "#0 0 255"
call c711.text("")
call c712.hide


call c711.text("OK")
havemkisofs=YES

IF global._mkisofsPath="" THEN DO
	call c711.color "+", "#255 0 0"
	call c711.text("Path to mkisofs.exe is not valid!  Set the right path in the settings note book.")
	call c712.show
	havemkisofs=NO
END
ELSE DO
if STREAM(global._mkisofsPath, "C","Query exists")="" THEN DO
	call mkisofs.c711.color "+", "#255 0 0"
	call mkisofs.c711.text("Path to mkisofs.exe is not valid! Set the right path in the settings note book.")
	call mkisofs.c712.show
	havemkisofs=NO
END
ELSE
	call mkisofs.c711.text("OK")
END

if havemkisofs=YES THEN DO
	newQ=RXQUEUE('CREATE')
	oldq=RXQUEUE('SET', newQ)
	theCmd=global._mkisofsPath||' -version | RXQUEUE '||newq||' /FIFO'

	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END
	'RXQUEUE 'newq' /CLEAR'

	'@start "Version check" /C /B /MIN cmd.exe /C 'theCmd
	version=""
	DO WHILE QUEUED()\=0
		PULL str
		version=version||str
	END
	ret=RXQUEUE('SET', oldq)
	res=RXQUEUE('DELETE', newQ)
	call c715.text version

	ret=checkCDRVersion version

	SELECT
		WHEN ret=0 THEN DO
			call c716.text "The program is not cdrecord.exe!"
			call c716.color "+", "#255 0 0"
		END
		WHEN ret=1 THEN DO
			call c716.text "This version of cdrecord.exe is not compatible with Audio/Data-CD-Creator!"
			call c716.color "+", "#255 0 0"
		END
	OTHERWISE
		call c716.color "+", "#0 0 255"
	END

	ret=checkForCdrTools2(global._mkisofsPath)

	IF ret=1 THEN DO
		call c716.text "This version of MKISOFS is not compatible with Audio/Data-CD-Creator! Get a working release from:"||'0d'x||'0d'x||"   http://www.os2world.com/cdwriting"||'0d'x||"   http://www.geocities.com/SiliconValley/Sector/5785/"
		call c716.color "+", "#255 0 0"
	END
END
Initcpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+60)

call Owner "install"

��Click�/* Close this dialog page */
call closeFOR Dialog() 

/* open new dialog page and display it */
call cdrecord.open
call cdrecord.Owner "Install"
��Click�ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "DISPLAYHELPPANEL=1100")
ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "OPEN=DEFAULT")��Init1call Range 300
call text global._mkisofsPath

�� � 0z  z  �  ��              � � 1�  ��       m !@� n � @ $����        �  � ] F  )����         �@�� ^ } O &����       � �@�  2 (����        L  �� 	 5  #����        Q  �� 	 6  "����        V  �� Z � _ *����        g  �  )� !����     Click the button to open a command window which shows the revisions of the currently installed EMX libraries. Get Revision  The right side shows the minimal revisions to be installed. If the numbers in the revision command window are higher or the same than the ones on the right, everything is ok. Prev Next Minimal revision Check EMX revision ��� 0  � MoveIpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+50)
Opencall show



InitIpos=Install.Position()
PARSE VAR pos x y rest
rc=Position(x+12, y+60)
�"Click�/* Close this dialog page */
call closeFOR Dialog() 

/* open new dialog page and display it */
call aspi.open
call aspi.Owner "Install"
�#Click�/* Close this dialog page */
call closeFOR Dialog() 

/* open new dialog page and display it */
call emx.open
call emx.Owner "Install"
�&Init�
emxrev="EMX:		"61
emxrev=emxrev||'0D'x||"EMXIO:		"60
emxrev=emxrev||'0D'x||"EMXLIBC:	"63
emxrev=emxrev||'0D'x||"EMXLIBCM:	"64
emxrev=emxrev||'0D'x||"EMXLIBCS:	"64
emxrev=emxrev||'0D'x||"EMXWRAP:	"60

call text emxrev�)Click�'Start "EMX revision" /F /C /PGM "'global._installDir||'bin\emxrev.cmd"'
/*
emxrv="EMX:		"getrevision("EMX")
emxrv=emxrv||'0D'x||"EMXIO:		"getrevision("EMXIO")
emxrv=emxrv||'0D'x||"EMXLIBC:	"getrevision("EMXLIBC")
emxrv=emxrv||'0D'x||"EMXLIBCM:	"getrevision("EMXLIBCM")
emxrv=emxrv||'0D'x||"EMXLIBCS:	"getrevision("EMXLIBCS")
emxrv=emxrv||'0D'x||"EMXWRAP:	"getrevision("EMXWRAP")

call emxrev.c804.text emxrv
SIGNAL RETURN

getRevision:
    call RxFuncAdd 'emx_revision', ARG(1), 'emx_revision'
  rev = emx_revision()
  call RxFuncDrop emx_revision
  return rev
*/�$Init�theString="Click the button to open a command window which shows the revisions of the currently installed EMX libraries."
theString=theString||" (Note: the window may open in the background.)"
call text theString
�