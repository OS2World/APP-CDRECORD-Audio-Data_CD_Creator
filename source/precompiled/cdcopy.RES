� �� 0�   d    f   g   h   t   m   n   p   o   q   1  2  3  4  5  s 
  u   v   i   j   k   e   l   0  r   �    �   �   �   �   �   ,   /  -  .  ��� 0�"  errorHandlerwrc=RxMessageBox('Syntax error detected! Instruction is: '||SOURCELINE(SIGL)||'. Contact the author.',,,'ERROR')
returncheckSpe�/* Path to spe */

IF stream(globalData._spePath, 'C','query exists')='' THEN DO
	globalData._msgTitle='Check installation'
	globalData._msgTxt='The Path to SPE.EXE is not valid.'||'0d'x
	globalData._msgTxt=globalData._msgTxt||'It schould be here:'||'0d'x
 	globalData._msgTxt=globalData._msgTxt||globalData._spePath
	globalData._msgType='OK'
	ret=modalFor(d300)
	exit
END
writeLogYcall LINEOUT globalData._logfile, ARG(1)
call STREAM globalData._logfile, 'C', 'close'
cleanUp�/* */
/*SAY 'Old Queue: 'globalData._oldq
*/

rc=RXQUEUE('SET',globalData._oldq)
rc=RXQUEUE('DELETE',globalData._zQueue)
/*SAY 'DeleteQueue: 'rc*/

/* Save window position */
pos=d100.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdcdcopy", pos)
exit
writeThread�globalData._launch=Val('global._launch')
globalData._zQueue=Val('global._zQueue')
globalData._toDo=Val('global._toDo')
globalData._cdrDaoDir=Val('global._cdrdaoDir')

notifyMsg='OUTPUT'||globalData._toDo
startMsg='START'||globalData._toDo

call Notify d200, startMsg

oldq=RXQUEUE('Set',globalData._zQueue)

launch=globalData._launch||' '||' 2>&1 |RXQUEUE '||globalData._zQueue||' /FIFO & ECHO DONE |RXQUEUE 'globalData._zQueue||' /FIFO'
launch='"'launch'"'

/* Clean queue */
DO WHILE QUEUED()\=0
	PULL str
END
'RXQUEUE 'globalData._zQueue' /CLEAR'

tempDir=DIRECTORY()
rc=DIRECTORY(globalData._cdrDaoDir)

'@start "1:1 CD-Copy (VIO-helper)"  /C /B /MIN 'launch
rc=DIRECTORY(tempDir)

/* Get output from queue */
DO forever
	str=''
	IF QUEUED()\=0 THEN Do
		PULL str
		IF str='' THEN str=' '
		call Notify d200, notifyMsg, str
	END
	if STRIP(TRANSLATE(str))='DONE' THEN LEAVE
	IF QUEUED()=0 THEN
		rc=SysSleep(1)
END


/* Clean queue */
DO WHILE QUEUED()\=0
	PULL str
END

'RXQUEUE 'globalData._zQueue' /CLEAR'
oldq=RXQUEUE('Set',oldq)

call Notify d200, 'DONE'

	stripZero&return LEFT(ARG(1),LENGTH(ARG(1))-1)
buildCdrdaoCommand�temp=' --buffers '||TRUNC(globalData._fifo/0.174) ||' --speed '||globalData._speed
temp=temp||' '||simulate
SELECT
WHEN toDo='COPY' THEN DO
	temp=temp||' '||globalData._cdrdaoSrcOptions||' '
	globalData._command='"'||globalData._cdrdaoSrcPath||'" read-cd --device '||globalData._cdrdaoSrcDevice
	globalData._command=globalData._command||' --driver '||globalData._cdrdaoSrcDriver
	globalData._command=globalData._command||' '||temp
	globalData._command=globalData._command||' --datafile "'||WORD(d100.c115.Text(),1)||'/image.bin" "'||WORD(d100.c115.Text(),1)||'/image.toc"'
	globalData._copyImageToc=WORD(d100.c115.Text(),1)||'/image.toc'
	globalData._cdrDaoDir=globalData._cdrdaoSrcPath
END
WHEN toDo='GRAB' THEN DO
	temp=temp||' '||globalData._cdrdaoSrcOptions||' '
	imagefile=TRANSLATE(globalData._targetImage,'/','\')
	pos=LASTPOS('.',imagefile)
	IF pos\=0 THEN
		tocfile=LEFT(imagefile,pos)||'toc'
	ELSE
		tocfile=imagefile||'.toc'
	globalData._command='"'||globalData._cdrdaoSrcPath||'" read-cd --device '||globalData._cdrdaoSrcDevice
	globalData._command=globalData._command||' --driver '||globalData._cdrdaoSrcDriver
	globalData._command=globalData._command||' '||temp
	globalData._command=globalData._command||' --datafile "'||imagefile||'" "'||tocfile||'"'
	globalData._cdrDaoDir=globalData._cdrdaoSrcPath
END
WHEN toDo='WRITE' THEN DO
	temp=temp||' '||globalData._cdrdaoTargetOptions||' '
	tocfile=TRANSLATE(globalData._sourceImage,'/','\')
	globalData._command='"'||globalData._cdrdaoTargetPath||'" write --device '||globalData._cdrdaoTargetDevice
	globalData._command=globalData._command||' --driver '||globalData._cdrdaoTargetDriver
	globalData._command=globalData._command||' '||temp
	globalData._command=globalData._command||' "'tocfile||'"'
	globalData._cdrDaoDir=globalData._cdrdaoTargetPath
END
WHEN toDo='COPYWRITE' THEN DO
	temp=temp||' '||globalData._cdrdaoTargetOptions||' '
	tocfile=TRANSLATE(globalData._copyImageToc,'/','\')
	globalData._command='"'||globalData._cdrdaoTargetPath||'" write --device '||globalData._cdrdaoTargetDevice
	globalData._command=globalData._command||' --driver '||globalData._cdrdaoTargetDriver
	globalData._command=globalData._command||' '||temp
	globalData._command=globalData._command||' "'tocfile||'"'
	globalData._cdrDaoDir=globalData._cdrdaoTargetPath
END

OTHERWISE
	NOP
END

/* Build cdrdao directory */
globalData._cdrDaoDir=FILESPEC('drive',globalData._cdrDaoDir)||FILESPEC('path',globalData._cdrDaoDir)
globalData._cdrDaoDir=LEFT(globalData._cdrDaoDir,LENGTH(globalData._cdrDaoDir)-1)
readIniValues�/* cdrdao stuff */
globalData._cdrdaoSrcPath=SysIni(globalData._iniFile, 'cdrdaosrc', 'path')
globalData._cdrdaoSrcPath=stripZero(globalData._cdrdaoSrcPath)

globalData._cdrdaoTargetPath=SysIni(globalData._iniFile, 'cdrdaotarget', 'path')
globalData._cdrdaoTargetPath=stripZero(globalData._cdrdaoTargetPath)

globalData._cdrdaoSrcOptions=SysIni(globalData._iniFile, 'cdrdaosrc', 'options')
globalData._cdrdaoSrcOptions=stripZero(globalData._cdrdaoSrcOptions)

globalData._cdrdaoTargetOptions=SysIni(globalData._iniFile, 'cdrdaotarget', 'options')
globalData._cdrdaoTargetOptions=stripZero(globalData._cdrdaoTargetOptions)

globalData._cdrdaoSrcDriver=SysIni(globalData._iniFile, 'cdrdaosrc', 'driver')
globalData._cdrdaoSrcDriver=stripZero(globalData._cdrdaoSrcDriver)

globalData._cdrdaoTargetDriver=SysIni(globalData._iniFile, 'cdrdaotarget', 'driver')
globalData._cdrdaoTargetDriver=stripZero(globalData._cdrdaoTargetDriver)

globalData._cdrdaoSrcWriter=SysIni(globalData._iniFile, 'cdrdaosrc', 'writer')
globalData._cdrdaoSrcWriter=stripZero(globalData._cdrdaoSrcWriter)

globalData._cdrdaoTargetWriter=SysIni(globalData._iniFile, 'cdrdaotarget', 'writer')
globalData._cdrdaoTargetWriter=stripZero(globalData._cdrdaoTargetWriter)

globalData._cdrdaoSrcDevicename=SysIni(globalData._iniFile, 'cdrdaosrc', 'devicename')
globalData._cdrdaoSrcDevicename=stripZero(globalData._cdrdaoSrcDevicename)

globalData._cdrdaoTargetDevicename=SysIni(globalData._iniFile, 'cdrdaotarget', 'devicename')
globalData._cdrdaoTargetDevicename=stripZero(globalData._cdrdaoTargetDevicename)

globalData._fifo=SysIni(globalData._iniFile, 'device', 'fifo')
globalData._fifo=stripZero(globalData._fifo)

globalData._speed=SysIni(globalData._iniFile, 'device', 'speed')
globalData._speed=stripZero(globalData._speed)

/* Vars from last program run */
globalData._drive=LEFT(SysIni(globalData._iniFile, 'cdcopy', 'drive'),2)
IF globalData._drive='' THEN
	globalData._drive='C:'

globalData._sourceImage=SysIni(globalData._iniFile, 'cdcopy', 'sourceimagepath')
globalData._sourceImage=stripZero(globalData._sourceImage)
IF globalData._sourceImage='ERROR' THEN
	globalData._sourceImage='*.toc'
globalData._targetImage=SysIni(globalData._iniFile, 'cdcopy', 'targetimagepath')
globalData._targetImage=stripZero(globalData._targetImage)
IF globalData._targetImage='ERROR' THEN
	globalData._targetImage='*.bin'

/* Get device */
globalData._cdrdaoSrcDevice=WORD(globalData._cdrdaoSrcDevicename,1)
globalData._cdrdaoTargetDevice=WORD(globalData._cdrdaoTargetDevicename,1)

globalData._logFile=globalData._installDir||'logfiles\CD-Copy.log'
'type NUL> 'globalData._logFile

globalData._spePath=globalData._installDir||'bin\spe.exe'
	clearVars�globalData._installDir=''
globalData._cdrdaoSrcPath=''
globalData._cdrdaoTargetPath=''
globalData._cdrdaoSrcOptions=''
globalData._cdrdaoTargetOptions=''
globalData._cdrdaoSrcDriver=''
globalData._cdrdaoTargetDriver=''
globalData._cdrdaoSrcDevice=''
globalData._cdrdaoTargetDevice=''
globalData._cdrdaoSrcDevicename=''
globalData._cdrdaoTargetDevicename=''
globalData._cdrdaoSrcWriter=''
globalData._cdrdaoTargetWriter=''
globalData._command=''

globalData._sourceFile=''init�/* */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

SIGNAL on SYNTAX NAME errorHandler

call clearVars

parse source . . globalData._sourceFile

globalData._installDir=FILESPEC('drive',globalData._sourceFile)||FILESPEC('path',globalData._sourceFile)

globalData._installDir=LEFT(globalData._installDir, LENGTH(globalData._installDir)-1)
globalData._installDir=FILESPEC('drive',globalData._installDir)||FILESPEC('path',globalData._installDir)


globalData._iniFile=globalData._installDir||cdrecord.ini
globalData._zQueue='ZOUT'||TIME('s') /* Build unique queue name */
globalData._osVersion=RIGHT(SysOS2Ver(),2)

call readIniValues

call checkSpe

call DIRECTORY globalData._installDir'bin'
/* Register percent bar */
call RxFuncAdd 'DRCtrlRegister', 'drusrctl' , 'DRCtrlRegister'

call DRCtrlRegister

rc=RXQUEUE('CREATE',globalData._zQueue)
globalData._oldq=RXQUEUE('SET',globalData._zQueue)

�� �d 0�  �  �  ��          �  p G � � d ��         � � |  f ����       
  � � |  g ����         � � |  h ����       	 / �� � B  t ����        9�	 | �  m ����        G �	 q %  n ����        O �	 f %  p ����         X �1 q �  o ����         Y �1 f �  q ����        Z�	 L �  1����        h �	 A %  2����        p �	 6 %  3����         y �1 A � 
 4����         z �1 6 �  5����         { �  � " s ����         |    � 	 u ����        }   �  /  v ����        �  �  /  i ����        �  �9  /  j ����        �  ��  /  k ����        �  � � � - e ����        �  � b � . l ����        �  � 2 � . 0����        �  �  �  r ����1:1 CD-Copy    Copy CD Grab image Write raw image Test only Source device Driver: Options:   Target device Driver: Options:     Browse Ok Cancel Help Action Source device Target device Drive for raw image ���d 0�  �d Opencall d200.openInit�if globalData._osVersion >=40 then
	call Font '9.WarpSans'

pos=SysIni(globalData._inifile, "windowposition","drdcdcopy")
if pos\="ERROR:" & WORDS(pos)=4 THEN DO
	PARSE var pos x y dx dy
	call position x, y
END

call show
�k ClickHret=SysSetObjectData('<CWCREATOR_SETTINGS>' , 'DISPLAYHELPPANEL=5200')
�j Initcall Text '~Cancel'Clickcall cleanUp�i Click�	IF d100.c102.Select()=1 THEN DO
	toDo='COPY'	
END
ELSE
IF d100.c103.Select()=1 THEN DO
	toDo='GRAB'
	globalData._targetImage=d100.c117.text()
END
ELSE DO
	toDo='WRITE'
	globalData._sourceImage=d100.c117.text()
END

IF d100.c116.Select()=1 THEN
	simulate=' --simulate'
ELSE
	simulate=''

Call buildCdrdaoCommand

call writeLog ""
call writeLog "Cdrdao start command is:"
call writeLog  globalData._command
call writeLog ""

/* Disable buttons */
call d100.c105.disable
call d100.c106.disable
/* Show messages */
call d200.c201.delete
call d200.show
index=0
/* Set first line */
IF toDo ='WRITE' THEN DO
	call d200.c201.Add 'Starting to write image...'
	call d200.c201.Add ' '
END
ELSE IF toDo ='COPY' THEN DO
	call d200.c201.Add 'Starting the copy process...'
	call d200.c201.Add ' '
END
ELSE IF toDo ='GRAB' THEN DO
	call d200.c201.Add 'Starting to grab the image...'
	call d200.c201.Add ' '
END


launch=globalData._command
IF toDo='WRITE' THEN
	launch='"'||globalData._installDir||'bin\spe.exe" t15 '||launch


rc=Val('global._launch', launch)
rc=Val('global._zQueue', globalData._zQueue)
rc=Val('global._toDo', toDo)
rc=Val('global._cdrdaoDir',globalData._cdrDaoDir)

/* Start thread */
rc=start(writeThread)
�v Click�sel=d100.c104.Select()
if sel=1 THEN
	file=FilePrompt(globalData._sourceImage,'Image file name (*.toc)',,'O')
else
	file=FilePrompt(globalData._targetImage,'Image file name (*.bin)',,'S')

if file\='' THEN DO
	call d100.c117.text file
	IF sel=1 THEN 
		globalData._sourceImage=file
	ELSE
		globalData._targetImage=file
ENDInitcall Text '~Browse'
�s Init�/* Get free space of drives */
allDrives=SysDriveMap()
DO until allDrives=''
   if SysDriveInfo(Word(allDrives,1))\='' THEN DO
	CALL ADD Word(allDrives,1)||'  '||TRUNC(WORD(SysDriveInfo(Word(allDrives,1)),2)/1024/1024)||' MB'
	IF TRANSLATE(Word(allDrives,1))=TRANSLATE(globalData._drive) THEN
	   call Text Word(allDrives,1)||'  '||TRUNC(WORD(SysDriveInfo(Word(allDrives,1)),2)/1024/1024)||' MB'
   END
	allDrives=SUBWORD(allDrives, 2)
END�5Init)call text globalData._cdrdaoTargetOptions�4Init(call text globalData._cdrdaoTargetDriver�1Init,call text globalData._cdrdaoTargetDevicename�q Init&call text globalData._cdrdaoSrcOptions�o Init%call text globalData._cdrdaoSrcDriver�m Init)call text globalData._cdrdaoSrcDevicename�t Initcall Select 1�h Click�
call d100.c114.text 'Source image (not an ISO)'
call d100.c115.hide
call d100.c117.Text globalData._sourceImage
call d100.c117.Show
call d100.c118.Show
�g Click�call d100.c114.text 'Image file destination'
call d100.c115.hide
call d100.c117.Show
call d100.c117.Text globalData._targetImage
call d100.c118.Show

�f Clicklcall d100.c114.text 'Drive for raw image'
call d100.c115.Show
call d100.c117.Hide
call d100.c118.Hide

Init)call d100.c114.text 'Drive for raw image'�� �� 0    �  ��          �   E� u � ���          �  � . � F � ����        �  � # `  � ����        �    � " 5  � ����     �   �    �  �  � ����     �       �  �  � ����Messages     Scroll text Hide DRD_PERCENTBAR  DRD_PERCENTBAR  ���� 0�  �� 	LoseFocusy/* Save window position */
dlgpos=Position()
	rc=SysIni(globalData._inifile, "windowposition","drdcdcopymsg", dlgpos)
Notify�&/* These events are coming from the write thread */
call EventData

SELECT
WHEN TRANSLATE(EventData.1)='DONE' THEN
DO
	IF toDo\='COPY' THEN DO
		/* Now clean up */
		call d100.c105.enable
		call d100.c106.enable
		call d100.show
		call BEEP 1000, 100
		call BEEP 2000, 100
		call BEEP 3000, 100
	END
	ELSE DO
		toDo='COPYWRITE'
		Call buildCdrdaoCommand
		launch=globalData._command
		launch='"'||globalData._installDir||'bin\spe.exe" t15 '||launch

		rc=Val('global._launch', launch)
		rc=Val('global._zQueue', globalData._zQueue)
		rc=Val('global._toDo', toDo)
		globalData._msgTitle='Copying CD'
		globalData._msgTxt='Please insert an empty CD-R.' 
		globalData._msgType='OKCANCEL'
		ret=modalFor(d300)
		IF ret='OK' THEN DO
			call writeLog ""
			call writeLog "Cdrdao start command is:"
			call writeLog  globalData._command
			call writeLog ""
			/* Start thread */
			rc=start(writeThread)
		END
		ELSE DO
			call d100.c105.enable
			call d100.c106.enable
			call d100.show
		END
	END
	call d200.c204.Text '0 # '
	call d200.c205.Text '0 # '
END
WHEN TRANSLATE(EventData.1)='STARTGRAB' THEN
DO
	/* The thread just started */
	call d200.c204.Text '0 # '
	call d200.c205.Text '0 #No buffer used'
	theStatus=''
END
WHEN TRANSLATE(EventData.1)='STARTCOPY' THEN
DO
	/* The thread just started */
	call d200.c204.Text '0 # '
	call d200.c205.Text '0 #No buffer used'
	theStatus=''
END
WHEN TRANSLATE(EventData.1)='STARTCOPY' THEN
DO
	/* The thread just started */
	call d200.c204.Text '0 # '
	call d200.c205.Text '0 #No buffer information'
	theStatus=''
END
WHEN TRANSLATE(EventData.1)='STARTWRITE' THEN
DO
	/* The thread just started */
	call d200.c204.Text '0 # '
	call d200.c205.Text '0 #No buffer information'
	theStatus=''
END
WHEN TRANSLATE(EventData.1)='OUTPUTGRAB' | TRANSLATE(EventData.1)='OUTPUTCOPY' THEN
DO
	/* We're grabbing */
	call writeLog EventData.2
	/* Get length of track */
	IF POS('LEADOUT',TRANSLATE(EventData.2))\=0 THEN DO
		thelength=STRIP(SUBWORD(TRANSLATE(EventData.2,' ','('),4,1))
		thelength=TRANSLATE(thelength,' ',':')
		lengthSec=STRIP(WORD(thelength,1))*60+STRIP(WORD(thelength,2))
	END
	IF TRANSLATE(EventData.2)\='DONE' THEN DO
		IF STRIP(TRANSLATE(EventData.2,'0','1234567890','0'))\='00:00:00' THEN
			index=d200.c201.Add(EventData.2)
		ELSE DO
			str=STRIP(TRANSLATE(EventData.2,' ',':'))
			curSec=STRIP(WORD(str,1))*60+STRIP(WORD(str,2))
			IF curSec\=0 THEN DO
				percent=TRUNC((curSec/lengthSec)*100)
			END
			ELSE
				percent=0
			call d200.c204.Text percent||' #'||percent' % read'
		END
	END
	ELSE DO
		index=d200.c201.Add(' ')
		index=d200.c201.Add('Grabbing done.')
	END
END

WHEN TRANSLATE(EventData.1)='OUTPUTWRITE' | TRANSLATE(EventData.1)='OUTPUTCOPYWRITE' THEN
DO
	/* We're writing */
	call writeLog EventData.2
	IF TRANSLATE(EventData.2)\='DONE' THEN DO
		IF POS('LEAD-IN',TRANSLATE(EventData.2))\=0 THEN DO
			theStatus='LEADIN'
			index=d200.c201.Add('Writing lead-in...')
			/* percent bar */
			call d200.c204.Text '0 # '
			call d200.c205.Text '0 #No buffer information'
		END
		ELSE IF POS('LEAD-OUT',TRANSLATE(EventData.2))\=0 THEN DO
			theStatus='LEADOUT'
			index=d200.c201.Add('Writing lead-out...')
			/* percent bar */
			call d200.c204.Text '0 # '
			call d200.c205.Text '0 #No buffer information'
		END
		ELSE IF POS('TRACK',TRANSLATE(EventData.2))\=0 THEN DO
			IF theStatus \='DATA' THEN DO
			theStatus='DATA'
			index=d200.c201.Add('Writing data...')
			/* percent bar */
			call d200.c204.Text '0 # '
			END
		END
		IF WORD(TRANSLATE(EventData.2),1)='WROTE' & WORD(TRANSLATE(EventData.2),3)='OF' THEN DO
		/* Write status */
		SELECT
		WHEN theStatus='LEADIN' THEN DO
			percent=TRUNC((WORD(EventData.2, 2)/WORD(EventData.2, 4))*100)
			call d200.c204.Text percent||' #Lead-in: '||EventData.2
		END
		WHEN theStatus='LEADOUT' THEN DO
			percent=TRUNC((WORD(EventData.2, 2)/WORD(EventData.2, 4))*100)
			call d200.c204.Text percent||' #Lead-out: '||EventData.2
		END
		WHEN theStatus='DATA' THEN DO
			percent=TRUNC((WORD(EventData.2, 2)/WORD(EventData.2, 4))*100)
			call d200.c204.Text percent||' #Data: '||SUBWORD(EventData.2,1,5)
			buffer=STRIP(TRANSLATE(SUBWORD(EventData.2,6),'   ','()%'))
			buffer=WORD(buffer,2)
			call d200.c205.Text buffer||' #Buffer: '||buffer||'%'
		END
		OTHERWISE
			NOP
		END
		END
		ELSE
			index=d200.c201.Add(EventData.2)

	END
	ELSE DO
		index=d200.c201.Add(' ')
		index=d200.c201.Add('Writing done.')
/*ELSE IF toDo='COPY' THEN
index=d200.c201.Add('Copying done.')*/
	END
END
WHEN TRANSLATE(EventData.1)='OUTPUTCOPY' THEN
DO
	call writeLog EventData.2

			index=d200.c201.Add(EventData.2)
END
OTHERWISE
	NOP
END

IF d200.c203.Select()=1 THEN
	call d200.c201.Select index ,'TOP'
Init�if globalData._osVersion >=40 then
	call Font '9.WarpSans'

pos=SysIni(globalData._inifile, "windowposition","drdcdcopymsg")
if pos\="ERROR:" & WORDS(pos) =4 THEN DO
	PARSE var pos x y dx dy
	call position x, y
END
�� Initcall TEXT '0 #0 %'
�� Clickcall d200.hideInitcall Text '~Hide'�� InitCALL Select 1
�� �,0�   �   �  ��           �   �5 � 8 ,���          � @�   /����        �   �  5  -����        �   �C  5  .����     Ok Cancel ���,0L  �,InitCif globalData._msgTitle\='' THEN
	call Text globalData._msgTitle
�.Click"call d300.close
return 'CANCEL'
InitJif globalData._msgType\='OKCANCEL' THEN
	call hide

call Text '~Cancel'�-Clickcall d300.close
return 'OK'
Initcall Text '~Ok'�/Init=if globalData._msgTxt\='' THEN
	call Text globalData._msgTxt�