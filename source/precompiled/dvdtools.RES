� �� 0P   d    h   f   g   i   m   k   l   n   o   e   j   �    �   �   �   ��� 0�  buildFormatCommand�launch=globalData._command||" --format "
IF d100.c108.SELECT()=1 THEN DO
	IF globalData._media="DVD+RW" THEN DO
		call RxMessageBox "Inserted Media ("||globalData._media||") not supported for full formatting. Doing fast format instead.", "Information", "OK", "INFORMATION"
		launch=launch||"dvd+rw"
	END
	ELSE launch=launch||"full"
END
ELSE DO
	IF globalData._media="DVD-RW" THEN
	launch=launch||"dvdrwquick"
	ELSE 	IF globalData._media="DVD+RW" THEN
	launch=launch||"dvd+rw"
END
return launchbuildBlankCommand�launch=globalData._command||" --blank "
IF d100.c102.SELECT()=1 THEN
	launch=launch||"fast"
ELSE
	launch=launch||"full"

return launch	dvdThread�	globalData._launch=Val('global._launch')
globalData._zQueue=Val('global._zQueue')
globalData._toDo=Val('global._toDo')
globalData._dvdDaoDir=VAL(global._dvdDaoDir)

notifyMsg='OUTPUT'
IF globalData._todo="CHECK" THEN
	notifyMsg='OUTPUT'||globalData._todo

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
rc=DIRECTORY(globalData._dvdDaoDir)

'@start "1:1 CD-Copy (VIO-helper)"  /C /B /MIN 'launch
rc=DIRECTORY(tempDir)

/* Get output from queue */
DO forever
	str=''
	IF QUEUED()\=0 THEN Do
		PULL str
		IF str='' THEN str=' '
		if STRIP(TRANSLATE(str))='DONE' THEN LEAVE
		call Notify d200, notifyMsg, str
	END
	IF QUEUED()=0 THEN
		rc=SysSleep(1)
END


/* Clean queue */
DO WHILE QUEUED()\=0
	PULL str
END

'RXQUEUE 'globalData._zQueue' /CLEAR'
oldq=RXQUEUE('Set',oldq)

call Notify d200, 'DONE'||globalData._toDo

getSettings�	PROCEDURE EXPOSE globalData.
/*
	Parse the settings file coming from ADC
*/
settingsFile=ARG(1)

data.=''
a=0
Do WHILE LINES(settingsFile)
	a=a+1
	data.a=LINEIN(settingsFile)
END
data.0=a
call STREAM settingsFile, "C", "close"

DO a=1 to data.0
	SELECT
		WHEN TRANSLATE(LEFT(data.a,8))="[DEVICE]" THEN globalData._device=RIGHT(data.a, LENGTH(data.a)-8)
		WHEN TRANSLATE(LEFT(data.a, LENGTH("[DVDDAOPATH]")))="[DVDDAOPATH]" THEN globalData._dvddaoPath=RIGHT(data.a, LENGTH(data.a)-LENGTH("[DVDDAOPATH]"))
		WHEN LEFT(data.a, LENGTH('[INSTALLDIR]'))='[INSTALLDIR]' THEN
		DO
			globalData._installDir=RIGHT(data.a, LENGTH(data.a)-LENGTH('[INSTALLDIR]'))
			globalData._binDir=globalData._installDir'\bin'
			globalData._logfile=globalData._installDir'\logfiles\DVDTools.log'
		END
		WHEN LEFT(data.a, LENGTH('[LANGUAGEDLL]'))='[LANGUAGEDLL]' THEN
			globalData._languageDLL=RIGHT(data.a, LENGTH(data.a)-LENGTH('[LANGUAGEDLL]'))

	OTHERWISE NOP
	END
END
drop data.
ADDRESS CMD 'del "'||settingsFile||'"'

globalData._dvddaodir=FILESPEC("d", globalData._dvddaoPath)||FILESPEC("p", globalData._dvddaoPath)
globalData._dvddaodir=LEFT(globalData._dvddaodir, LENGTH(globalData._dvddaodir)-1)
builddvddaoCommand�
temp=' --buffers '||TRUNC(globalData._fifo/0.174) ||' --speed '||globalData._speed
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

writeLogYcall LINEOUT globalData._logfile, ARG(1)
call STREAM globalData._logfile, 'C', 'close'
errorHandlerwrc=RxMessageBox('Syntax error detected! Instruction is: '||SOURCELINE(SIGL)||'. Contact the author.',,,'ERROR')
return
getTempDir�PROCEDURE

theDir=VALUE("TMP",,"OS2ENVIRONMENT")
IF theDir="" THEN theDir=VALUE("TEMP",,"OS2ENVIRONMENT")
IF theDir="" THEN theDir=VALUE("TMPDIR",,"OS2ENVIRONMENT")
IF theDir="" THEN theDir=VALUE("TEMPDIR",,"OS2ENVIRONMENT")
/* We need a fallback at some point in the future here */

theDir=TRANSLATE(theDir, "\", "/")
if RIGHT(theDir, 1)='\' THEN theDir=LEFT(theDir, LENGTH(theDir)-1)

return theDir

init�call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

/* SIGNAL on SYNTAX NAME errorHandler */

parse source . . globalData._sourceFile

globalData._osVersion=RIGHT(SysOS2Ver(),2)
globalData._zQueue='ZOUT'||TIME('s') /* Build unique queue name */

globaldata._tempDir=getTempDir()
/* Get settings from ADC */
globalData._tempSettings=SysTempFileName(globalData._tempDir||"\ADCSETT?.???")
rc=SysSetObjectData("<CWCREATOR_SETTINGS>", "WRITECONFIGTOFILE="||globalData._tempSettings)

/* PArse settings and put into global var */
call getSettings globalData._tempSettings

/* Build base command */
globalData._command='"'||globalData._dvddaoPath||'" -d 'globalData._device||' '

call DIRECTORY globalData._binDir
call RxFuncAdd 'DVDToolsGetResString', 'drusrctl' , 'CreatorGetResString'

/* Create a unique queue */
rc=RXQUEUE('CREATE',globalData._zQueue)
globalData._oldq=RXQUEUE('SET',globalData._zQueue)


indent="  "theEnd�/* Reset queue */
rc=RXQUEUE('SET',globalData._oldq)
rc=RXQUEUE('DELETE',globalData._zQueue)

/* Save window position */
/*
pos=d100.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdcdcopy", pos)
*/
EXIT(0)�� �d 0�  �  �  ��         	 v  �j f � Y d ���       	 � �Z F P 
 h ����        � � = l 	 f ����        � � 5 l  g ����        �  �� 8    i ����       	 � �Z " P 	 m ����        � �  m 	 k ����        � �  m  l ����        �  ��     n ����        � ��      o ����        �  � 2 � % e ����        �  �  � % j ����DVD Tools    Test only Fast blanking Full Blank Test only Quick format Full Format Close DVD Blanking Format ���d 0	  �d Init�if globalData._osVersion >=40 then
	call Font '9.WarpSans'

call TEXT DVDToolsGetResString(globalData._languageDLL, 4720)

pos=SysIni(globalData._inifile, "windowposition","drddvdtools")
if pos\="ERROR:" & WORDS(pos)=4 THEN DO
	PARSE var pos x y dx dy
	call position x, y
END

call show
Open!call d200.open
call d100.focus
�j Init?call TEXT DVDToolsGetResString(globalData._languageDLL, 4707)
�e Init?call TEXT DVDToolsGetResString(globalData._languageDLL, 4706)
�o Init=call TEXT DVDToolsGetResString(globalData._languageDLL, 4702)ClickCALL theEnd�n Init=call TEXT DVDToolsGetResString(globalData._languageDLL, 4713)Click�globalData._toDo="FORMAT"
/* Show messages */
call d200.c201.delete
call d200.show
index=0

globalData._media="NOMEDIA"

call d200.c201.Add 'Checking disk...'
call d200.c201.Add ' '
call d200.c201.Add indent||'Output from Dvddao:'
call d200.c201.Add ' '

launch=globalData._command

rc=Val('global._launch', launch)
rc=Val('global._zQueue', globalData._zQueue)
rc=Val('global._toDo', "CHECK")
rc=Val('global._dvddaodir', globalData._dvddaodir)

/* Start thread */
rc=start(dvdThread)
�l Init?call TEXT DVDToolsGetResString(globalData._languageDLL, 4709)
�k InitNCALL SELECT 1
call TEXT DVDToolsGetResString(globalData._languageDLL, 4708)
�m InitLCALL SELECT 1
call TEXT DVDToolsGetResString(globalData._languageDLL, 4715)�i Init=call TEXT DVDToolsGetResString(globalData._languageDLL, 4714)Click�globalData._toDo="BLANK"
/* Show messages */
call d200.c201.delete
call d200.show
index=0

globalData._media="NOMEDIA"

call d200.c201.Add 'Checking disk...'
call d200.c201.Add ' '
call d200.c201.Add indent||'Output from Dvddao:'
call d200.c201.Add ' '

launch=globalData._command

rc=Val('global._launch', launch)
rc=Val('global._zQueue', globalData._zQueue)
rc=Val('global._toDo', "CHECK")
rc=Val('global._dvddaodir', globalData._dvddaodir)

/* Start thread */
rc=start(dvdThread)

/*
call writeLog ""
call writeLog "Cdrdao start command is:"
call writeLog  globalData._command
call writeLog ""

/* Disable buttons */
call d100.c105.disable
call d100.c106.disable
*/
�g Init=call TEXT DVDToolsGetResString(globalData._languageDLL, 4711)�f InitNCALL SELECT 1
call TEXT DVDToolsGetResString(globalData._languageDLL, 4710)
�h InitLCALL SELECT 1
call TEXT DVDToolsGetResString(globalData._languageDLL, 4705)�� �� 0�   �   �  ��          �   � � � p � ���          �  �  � T � ����        �  �  w 	 � ����        �    � 
 '  � ����Messages     Scroll text Hide ���� 0  �� InitAcall TEXT DVDToolsGetResString(globalData._languageDLL, 4701)

Notify�8/* These events are coming from the write thread */
call EventData


call eventdata

SELECT
WHEN eventdata.1="OUTPUT" THEN DO
	IF POS("ERROR", TRANSLATE(EventData.2))<>0 THEN
		index=d200.c201.Add(indent||"**** "||EventData.2)
	ELSE
	index=d200.c201.Add(indent||EventData.2)
END
WHEN eventdata.1="DONEFORMAT" THEN DO
		call d200.c201.Add ' '
		call d200.c201.Add '--- Finished with formatting ---'
		call d200.c201.Add ' '
END
WHEN eventdata.1="DONEBLANK" THEN DO
		call d200.c201.Add ' '
		call d200.c201.Add '--- Finished with blanking ---'
		call d200.c201.Add ' '
END
WHEN eventdata.1="OUTPUTCHECK" THEN DO
	IF POS("ERROR", TRANSLATE(EventData.2))<>0 THEN
		index=d200.c201.Add(indent||"**** "||EventData.2)
	ELSE
	index=d200.c201.Add(indent||EventData.2)
	IF POS("MEDIA TYPE", TRANSLATE(EventData.2))<>0 THEN DO
		PARSE VAR Eventdata.2 . ':' . ':' theMedia
		globalData._media=STRIP(TRANSLATE(theMedia))
	END
END
WHEN eventdata.1="DONECHECK" THEN DO
	IF globalData._todo="BLANK" THEN DO
		IF globalData._media="DVD+RW" THEN DO
			call RxMessageBox "Inserted Media ("||globalData._media||") not supported for blanking. Use <Format> instead.", "Error", "OK", "EXCLAMATION"
		END
		ELSE IF globalData._media<>"DVD-RW" THEN DO
			call RxMessageBox "Inserted Media ("||globalData._media||") currently not supported for blanking.", "Error", "OK", "EXCLAMATION"
		END
		ELSE DO
			launch=buildBlankCommand()
			call d200.c201.Add ' '
			call d200.c201.Add '--- Now blanking disk ---'
			call d200.c201.Add ' '
			call d200.c201.Add indent||'Output from Dvddao:'
			call d200.c201.Add ' '

			rc=Val('global._launch', launch)
			rc=Val('global._zQueue', globalData._zQueue)
			rc=Val('global._toDo', globalData._todo)
			rc=Val('global._dvddaodir', globalData._dvddaodir)

			/* Start thread */
			rc=start(dvdThread)
		END
	END
	ELSE IF globalData._todo="FORMAT" THEN DO
		launch=buildFormatCommand()
		call d200.c201.Add ' '
		call d200.c201.Add '--- Now formatting disk ---'
		call d200.c201.Add ' '
		call d200.c201.Add indent||'Output from Dvddao:'
		call d200.c201.Add ' '

		rc=Val('global._launch', launch)
		rc=Val('global._zQueue', globalData._zQueue)
		rc=Val('global._toDo', globalData._todo)
		rc=Val('global._dvddaodir', globalData._dvddaodir)

		/* Start thread */
		rc=start(dvdThread)
	END
END
OTHERWISE NOP

END

/*
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

*/
IF d200.c203.Select()=1 THEN
	call d200.c201.Select index ,'TOP'
�� Clickcall d200.hideInitcall Text '~Hide'�� InitPCALL Select 1
call TEXT DVDToolsGetResString(globalData._languageDLL, 4712)

�