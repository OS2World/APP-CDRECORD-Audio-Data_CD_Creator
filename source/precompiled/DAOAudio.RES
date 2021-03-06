� �� 0�  d   mainl  targetFoldere  tracksn  totalLengtho   � efDevicep   x  fifoq   %  f  okg  cancelm  hintcontrol�   0  �   wait�   ,  msgBox/  -  .  �  popupX  bubbleY 
bubbleText�  pregap�  �  � mins�  �  � secs� applyall�  �  �     
burnStatus! lbStatus%  � elapsedTime� 
neededTime( 	statusBar) 	bufferBar" 	pbStausOk# pbAbort$ 	pbShowLog�  �  	dlgMsgLog� lbMsg�  �  ��� 0IH  saveTocFile�oldDir=DIRECTORY(globalData._folder)
file=FilePrompt("*.toc","TOC file name for saving", ,"S")
if file\='' THEN DO
	call buildTocData
	call LINEOUT file, tocData
	call STREAM file,'C','close'
END
call DIRECTORY oldDirbuildTocData�/* Build TOC-File */
tocData='CD_DA'||'0a'x||'0a'x
fileData=''

DO i=1 to globalData.0
	tocData=tocData||'// Track '||i
	tocData=tocData||'0a'x||'TRACK AUDIO'
	IF i\=1 & globalData.i._pregap\=0 THEN DO
		tocData=tocData||'0a'x||'PREGAP 'globalData.i._pregap%60||':'||globalData.i._pregap//60||':0'
	END
	IF globalData._mp3Decoder=3 THEN
		tocData=tocData||'0a'x||'FILE "-" 0 '||globalData.i._timeString
	ELSE
		tocData=tocData||'0a'x||'FILE "'||TRANSLATE(globalData.i,'/','\')||'" 0'
	tocData=tocData||'0a'x||'0a'x
	fileData=fileData||globalData.i
	fileData=fileData||'0a'x
END
writeLog�call LINEOUT globalData._logfile, ARG(1)
call STREAM globalData._logfile, 'C', 'close'
index=dlgMsgLog.lbMsg.Add( ARG(1))
IF dlgMsgLog.c907.Select()=1 THEN
	call dlgMsgLog.lbMsg.Select index, 'TOP'
buildCdrdaoCmd�IF globalData._testOnly='YES' THEN
	globalData._cdrdaoCmd='simulate'
ELSE
	globalData._cdrdaoCmd='write'

globalData._cdrdaoCmd=globalData._cdrdaoCmd||' -n'
globalData._cdrdaoCmd=globalData._cdrdaoCmd||' --device 'globalData._device
globalData._cdrdaoCmd=globalData._cdrdaoCmd||' --driver '||globalData._driver
globalData._cdrdaoCmd=globalData._cdrdaoCmd||' --speed '||globalData._speed
globalData._cdrdaoCmd=globalData._cdrdaoCmd||' --buffers '||TRUNC(globalData._fifo*1024/176)
IF globalData._eject='YES' THEN
	globalData._cdrdaoCmd=globalData._cdrdaoCmd||' --eject'
IF globalData._mp3Decoder=3 THEN
	IF TRANSLATE(globalData._mp3Swap)='YES' THEN
		globalData._cdrdaoCmd=globalData._cdrdaoCmd||' --swap'
globalData._cdrdaoCmd=globalData._cdrdaoCmd||' '||globalData._options

call writeLog 'Cdrdao parameters: 'globalData._cdrdaoCmd
startCdrdaoThread�
globalData._folder=Val('global._folder')
globalData._cdrdaoPath=Val('global._cdrdaoPath')
globalData._zQueue=Val('global._zQueue')
globalData._binDir=Val('global._binDir')
globalData._cdrdaoCmd=Val('global._cdrdaoCmd')
globaldata._tocFile=Val('global._tocFile')
globaldata._nameFile=Val('global._nameFile')
globaldata._cdLength=Val('global._cdlength')
globalData._logfile=VAL('global._logfile')
globalData._mp3Decoder=val('global._mp3Decoder')


mByte=25+15+TRUNC(globalData._cdlength*44.1*4/1000)

status=''
resultCode=0
gotLeadin='NO'

oldq=RXQUEUE('Set',globalData._zQueue)

call Notify 'burnStatus', 'LOG', ' '
IF globalData._mp3Decoder=3 THEN DO
	call Notify 'burnStatus', 'LOG', 'Starting to write in on the fly DAO mode...'
	launch='"'globalData._binDir'\daommio.exe" '
END
ELSE DO
	call Notify 'burnStatus', 'LOG', 'Starting to write in DAO mode...'
	launch='"'globalData._binDir'\daowrite.exe" '
END

globalData._cdrdaoPath='""'globalData._cdrdaoPath'""'
launch=launch||' "'globalData._cdrdaoPath'" '
globalData._cdrdaoCmd='""'globalData._cdrdaoCmd'""'
launch=launch||' "'globalData._cdrdaoCmd'"'
globaldata._tocFile='""'globaldata._tocFile'""'
launch=launch||' "'globaldata._tocFile'" '
globaldata._nameFile='""'globaldata._nameFile'""'
launch=launch||' "'globaldata._nameFile'" '
launch=launch||'2>&1 |RXQUEUE '||globalData._zQueue||' /FIFO'
launch='"'launch'"'

	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END

	'RXQUEUE 'globalData._zQueue' /CLEAR'
	
	call Notify 'burnStatus', 'LOG', ' '
	call Notify 'burnStatus', 'LOG', 'Commandline is: 'launch

	'@start "Write DAO (VIO)" /C /B /MIN cmd.exe /C 'launch

	/* Set status text */
	call Notify 'burnStatus', 'cdrdao', 'start'

	/* Get output from queue */

	DO forever
		IF QUEUED()\=0 THEN DO
			PULL str
			call Notify 'burnStatus', 'LOG', str' '
			IF WORDPOS('LEAD-IN', TRANSLATE(str))\=0 THEN DO
				call Notify 'burnStatus', 'cdrdao', 'LEADIN'
				status='LEADIN'
				gotLeadin='YES'
			END
			IF POS('LEAD-OUT',TRANSLATE(str))\=0 THEN DO
				call Notify 'burnStatus', 'cdrdao', 'LEADOUT'
				status='LEADOUT'
			END
			IF POS('BY SIGPIPE',TRANSLATE(str))\=0 THEN DO
				call Notify 'burnStatus', 'cdrdao', 'TERMINATED'
				status='LEADOUT'
			END
			IF POS('BY SIGINT',TRANSLATE(str))\=0 THEN DO
				call Notify 'burnStatus', 'cdrdao', 'TERMINATED'
				status='LEADOUT'
			END
			IF POS('GIVING UP',TRANSLATE(str))\=0 THEN DO
				call Notify 'burnStatus', 'cdrdao', 'TERMINATED'
				status='LEADOUT'
			END
			IF WORD(TRANSLATE(str),1)='WROTE' & WORD(TRANSLATE(str),3)='OF' THEN DO
				/* Get buffer */
				if POS('%',str)\=0 THEN DO
					buffer=WORD(str,7)
					buffer=STRIP(LEFT(buffer,LENGTH(buffer)-3))
				END
				ELSE
					buffer=''
				IF status='LEADIN' THEN DO
					call Notify 'burnStatus', 'PERCENT', WORD(TRANSLATE(str),2) *100 / mByte
				END
				ELSE DO
				IF status='LEADOUT' THEN DO
					call Notify 'burnStatus', 'PERCENT', (mByte-15+WORD(TRANSLATE(str),2)) *100 / mByte
				END
				ELSE DO
					call Notify 'burnStatus', 'PERCENT', (25+WORD(TRANSLATE(str),2)) *100 / mByte||' '||buffer
				END
				END
			END
			IF WORD(TRANSLATE(str),2)='TRACK' THEN DO
				call Notify 'burnStatus', 'TRACK', WORD(str,3)  
				status='TRACK'
			END

		END
		IF WORD(TRANSLATE(str),1)='RETURN' THEN
			resultCode=WORD(str, 2)

		if STRIP(TRANSLATE(str))='DONE' THEN DO
/*SAY 'Got Done...'*/
LEAVE
END
		IF QUEUED()=0 THEN
			rc=SysSleep(1)
	END
/*SAY 'Cleaning queue...'*/

	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END
	'RXQUEUE 'globalData._zQueue' /CLEAR'

	oldq=RXQUEUE('Set',oldq)

	call Notify 'burnStatus', 'cdrdao', 'DONE'
	call Notify 'burnStatus', 'RESULT', resultCode

	call Notify 'burnStatus', 'LOG', ' '
	call Notify 'burnStatus', 'LOG', 'Writing finished with RC='resultCode
queryCDSizeThread�globalData._folder=Val('global._folder')
globalData._cdrecordPath=Val('global._cdrecordPath')
globalData._zQueue=Val('global._zQueue')
globalData._CDLength=Val('global._CDLength')
globalData._binDir=Val('global._binDir')
globalData._logfile=VAL('global._logfile')
globalData._device=VAL('global._device')
globalData._mp3Decoder=VAL('global._mp3Decoder')

oldq=RXQUEUE('Set',globalData._zQueue)

	call Notify 'burnStatus', 'LOG', ' '
	call Notify 'burnStatus', 'LOG', 'Querying free CD space...'

	/* Set status text */
	call Notify 'burnStatus', 'cdsize', 'start'

launch='"'globalData._cdrecordPath'" '
launch=launch||' dev='globalData._device' -v -toc 2>&1 |RXQUEUE '||globalData._zQueue||' /FIFO'
launch=launch||'  & ECHO DONE |RXQUEUE '||globalData._zQueue||' /FIFO'
launch='"'launch'"'

	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END

	'RXQUEUE 'globalData._zQueue' /CLEAR'
	/* Check size */
	'@start "Query CD space (VIO)" /C /B /MIN cmd.exe /C 'launch

	/* Get output from queue */

leadin=0
leadout=0
	DO forever
		IF QUEUED()\=0 THEN DO
			PULL str
	call Notify 'burnStatus', 'LOG', str' '
			IF TRANSLATE(WORD(str,1))='ATIP' THEN DO
				IF TRANSLATE(WORD(str,5))='OUT:' THEN
					leadout=WORD(str,6)
				IF TRANSLATE(WORD(str,5))='IN:' THEN
					leadin=0
			END
			IF TRANSLATE(WORD(str,1))='TRACK:LOUT' THEN DO
				leadin=WORD(str,3)
			END

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
	call Notify 'burnStatus', 'cdsize', leadout-leadin
	call Notify 'burnStatus', 'LOG', ' '
	call Notify 'burnStatus', 'LOG', 'Free CD space is: 'leadout-leadin
	call Notify 'burnStatus', 'LOG', ' '

writeThread�


globalData._folder=Val('global._folder')
globalData._cdrecordPath=Val('global._cdrecordPath')
globalData._zQueue=Val('global._zQueue')
globalData._CDLength=Val('global._CDLength')
globalData._binDir=Val('global._binDir')

oldq=RXQUEUE('Set',globalData._zQueue)

	launch=globalData._cdrecordPath||' dev=0,1,0 -v -toc  && type 'globalData._binDir'\done.txt" |RXQUEUE '||globalData._zQueue||' /FIFO'

	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END

	'RXQUEUE 'globalData._zQueue' /CLEAR'
	/* Check size */
	'@start /C /B /MIN cmd.exe /C "'launch

	/* Set status text */
	call Notify 'burnStatus', 'cdsize', 'start'

	/* Get output from queue */

leadin=0
leadout=0
	DO forever
		IF QUEUED()\=0 THEN DO
			PULL str
/*	SAY str*/
			IF TRANSLATE(WORD(str,1))='ATIP' THEN DO
				IF TRANSLATE(WORD(str,5))='OUT:' THEN
					leadout=WORD(str,6)
				IF TRANSLATE(WORD(str,5))='IN:' THEN
					leadin=0
			END
			IF TRANSLATE(WORD(str,1))='TRACK:LOUT' THEN DO
				leadin=WORD(str,3)
			END

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
	call Notify 'burnStatus', 'cdsize', leadout-leadin

call Notify 'burnStatus', 'done'
checkForCdrecord�IF STRIP(globalData._cdrecordPath)='' THEN globalData._cdrecordPath="NOT VALID"

IF STREAM(globalData._cdrecordPath,'C','query exists')=''
THEN DO
	globalData._msgType='OK'
	theString='The path to CDRecord/2 is not valid. Check the settings in the main settings notebook.'
	call ShowMsgBox theString, 'Setup problem','OK'
	call writeLog ''
	call writeLog theString
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "DISPLAYHELPPANEL=300")
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "OPEN=DEFAULT")
	exit
END
calculateLength�/* */
pregap=0
DO i=1 to globalData.0
	pregap=pregap+globalData.i._pregap
END

globalData._cdLength=globalData._totalLength+pregap

theString='Play time: '||globalData._totalLength%60':'RIGHT('00'||globalData._totalLength//60,2)'          '
theString=theString||'CD-length: '||globalData._cdLength%60':'RIGHT('00'||globalData._cdLength//60,2)
call main.totallength.text theString
burnCD�call buildTocData

filename=globalData._folder||'\default.toc'
globaldata._tocFile=filename
call writeLog 'Tocfile name is: 'filename
filename='"'filename'"'
'type NUL> 'filename
call LINEOUT globaldata._tocFile,tocdata
call STREAM globaldata._tocFile, 'C', 'close'

filename=globalData._folder||'\default.nam'
globaldata._nameFile=filename
call writeLog 'Namefile name is: 'filename
filename='"'filename'"'
'type NUL> 'filename
call LINEOUT globaldata._nameFile,filedata
call STREAM globaldata._nameFile, 'C', 'close'

/* Save window position */
pos=main.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdwriteaudiodao", pos)

globalData._device=main.efDevice.Text()

call buildCdrdaoCmd

/* Make some vars global */
rc=Val('global._folder',globalData._folder)
rc=Val('global._cdrecordPath',globalData._cdrecordPath)
rc=Val('global._cdrdaoPath',globalData._cdrdaoPath)
rc=Val('global._zQueue',globalData._zQueue)
rc=Val('global._cdLength',globalData._cdLength)
rc=Val('global._binDir',globalData._binDir)
rc=Val('global._cdrdaoCmd', globalData._cdrdaoCmd)
rc=Val('global._tocFile',globaldata._tocFile)
rc=Val('global._nameFile',globaldata._nameFile)
rc=Val('global._cdlength', globaldata._cdLength)
rc=VAL('global._logfile', globalData._logfile)
rc=VAL('global._device', globalData._device)
rc=VAL('global._options', globalData._options)
rc=VAL('global._mp3Decoder', globalData._mp3Decoder)

call main.close

call burnStatus.open , 'Audio CD writing...'
definePregap�/**/
index=globalData._index

if index\='' THEN DO
	call pregap.open
	call pregap.show

/*
	launch=globalData._zPath||' "'||globalData.index'"'
	'start /C /F 'launch
*/
END
checkForDAO�IF STRIP(globalData._cdrdaoPath)='' THEN globalData._cdrdaoPath="NOT VALID"

IF STREAM(globalData._cdrdaoPath,'C','query exists')=''
THEN DO
	theString='The path to cdrdao is not valid. Check the settings in the main settings notebook.'
	globalData._msgType='OK'
	call ShowMsgBox theString, 'Setup problem','OK'
	call writeLog ''
	call writeLog theString
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "DISPLAYHELPPANEL=2800")
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "OPEN=DEFAULT")
	exit
END
cleanUp�/* */
rc=RXQUEUE('SET',globalData._oldq)
rc=RXQUEUE('DELETE',globalData._zQueue)
/*SAY 'DeleteQueue: 'rc*/

/*
SAY 'Checking for registration...'
SAY 'DRCtrlLoadFuncs: 'RXFUNCQUERY('DRCtrlLoadFuncs')
SAY 'DRCtrlVersion: 'RXFUNCQUERY('DRCtrlVersion')
SAY 'DRCtrlDropFuncs: 'RXFUNCQUERY('DRCtrlDropFuncs')

call RxFuncAdd 'DRCtrlDropFuncs', 'G:\Projects_working\Audio_cdcreator_050\source\drusrctl.dll', 'DRCtrlDropFuncs'

call DRCtrlDropFuncs
SAY 'DeRegistered'
call RXFUNCDROP 'DRCtrlDropFuncs'
SAY 'Checking for registration...'
SAY 'DRCtrlLoadFuncs: 'RXFUNCQUERY('DRCtrlLoadFuncs')
SAY 'DRCtrlVersion: 'RXFUNCQUERY('DRCtrlVersion')
SAY 'DRCtrlDropFuncs: 'RXFUNCQUERY('DRCtrlDropFuncs')
*/

filename=globaldata._tocFile
ret=SysDestroyObject(filename)
filename=globaldata._nameFile
ret=SysDestroyObject(filename)

call mciRxExit
exit

ShowMsgBox�/* */
text=ARG(1)
Title=ARG(2)
type=ARG(3)

globalData._msgType=type
globalData._msgText=text
globalData._msgTitle=title

ret=modalFor(msgBox)

DROP text
DROP title
DROP type

RETURN ret
bubbleHelp�/*
if TRANSLATE(hintcontrol.text())='CONTAINER' THEN
DO
	call bubble.open
	call bubble.bubbleText.text 'Right button for more'
	bubble=1
END
ELSE DO
	IF bubble=1 THEN
		call bubble.close
	bubble=0
END
*/init�&call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

globalData.=''
globalData.0=0

globalData._contentsFile=ARG(1)

/*
globalData._contentsFile='G:\Projects_working\Audio_cdcreator_051\source\contents.005'
*/

/* Defaults */
globalData._zQueue='ZOUT'||TIME('s') /* Build unique queue name */
globalData._osVersion=RIGHT(SysOS2Ver(),2)
globalData._totalLength=0
globalData._device='0,1,0'
globalData._StartWriteTime=0
globalData._mp3Decoder=1

/* Parse input file */
IF STREAM(globalData._contentsFile,'C','QUERY EXIST')\='ERROR:' THEN DO
	i=0
	globalData._folder=LINEIN(globalData._contentsFile)
	DO WHILE LINES(globalData._contentsFile)\=0
		theLine=LINEIN(globalData._contentsFile)
		SELECT
		WHEN LEFT(theLine,LENGTH('[SHADOW;SELECTED]'))='[SHADOW;SELECTED]' THEN
		DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[SHADOW;SELECTED]'))
			globalData.0=i
		END
		WHEN LEFT(theLine,LENGTH('[SHADOW;UNSELECTED]'))='[SHADOW;UNSELECTED]' THEN
		DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[SHADOW;UNSELECTED]'))
			globalData.0=i
		END
		WHEN LEFT(theLine,LENGTH('[FILE;SELECTED]'))='[FILE;SELECTED]' THEN
		DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[FILE;SELECTED]'))
			globalData.0=i
		END
		WHEN LEFT(theLine,LENGTH('[FILE;UNSELECTED]'))='[FILE;UNSELECTED]' THEN
		DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[FILE;UNSELECTED]'))
			globalData.0=i
		END
		WHEN LEFT(theLine,LENGTH('[INIFILE]'))='[INIFILE]' THEN
		DO
			globalData._inifile=RIGHT(theline, LENGTH(theLine)-LENGTH('[INIFILE]'))
		END
		WHEN LEFT(theLine,LENGTH('[INSTALLDIR]'))='[INSTALLDIR]' THEN
		DO
			globalData._installDir=RIGHT(theline, LENGTH(theLine)-LENGTH('[INSTALLDIR]'))
			globalData._binDir=globalData._installDir'\bin'
			globalData._logfile=globalData._installDir'\logfiles\WriteDAO.log'
		END
		WHEN LEFT(theLine,LENGTH('[CDRDAOPATH]'))='[CDRDAOPATH]' THEN
		DO
			globalData._cdrdaoPath=RIGHT(theline, LENGTH(theLine)-LENGTH('[CDRDAOPATH]'))
		END
		WHEN LEFT(theLine,LENGTH('[CDRECORDPATH]'))='[CDRECORDPATH]' THEN
		DO
			globalData._cdrecordPath=RIGHT(theline, LENGTH(theLine)-LENGTH('[CDRECORDPATH]'))
		END
		WHEN LEFT(theLine,LENGTH('[DEVICE]'))='[DEVICE]' THEN
		DO
			globalData._device=RIGHT(theline, LENGTH(theLine)-LENGTH('[DEVICE]'))
		END
		WHEN LEFT(theLine,LENGTH('[SPEED]'))='[SPEED]' THEN
		DO
			globalData._speed=RIGHT(theline, LENGTH(theLine)-LENGTH('[SPEED]'))
		END
		WHEN LEFT(theLine,LENGTH('[FIFO]'))='[FIFO]' THEN
		DO
			globalData._fifo=RIGHT(theline, LENGTH(theLine)-LENGTH('[FIFO]'))
		END
		WHEN LEFT(theLine,LENGTH('[CDRDAODRIVER]'))='[CDRDAODRIVER]' THEN
		DO
			globalData._driver=RIGHT(theline, LENGTH(theLine)-LENGTH('[CDRDAODRIVER]'))
		END
		WHEN LEFT(theLine,LENGTH('[CDRDAOOPTIONS]'))='[CDRDAOOPTIONS]' THEN
		DO
			globalData._options=RIGHT(theline, LENGTH(theLine)-LENGTH('[CDRDAOOPTIONS]'))
		END
		WHEN LEFT(theLine,LENGTH('[TESTONLY]'))='[TESTONLY]' THEN
		DO
			globalData._testOnly=RIGHT(theline, LENGTH(theLine)-LENGTH('[TESTONLY]'))
		END
		WHEN LEFT(theLine,LENGTH('[EJECT]'))='[EJECT]' THEN
		DO
			globalData._eject=RIGHT(theline, LENGTH(theLine)-LENGTH('[EJECT]'))
		END
		WHEN LEFT(theLine,LENGTH('[MP3DECODER]'))='[MP3DECODER]' THEN
		DO
			globalData._mp3Decoder=RIGHT(theline, LENGTH(theLine)-LENGTH('[MP3DECODER]'))
		END
		WHEN LEFT(theLine,LENGTH('[MP3SWAPBYTES]'))='[MP3SWAPBYTES]' THEN
		DO
			globalData._mp3Swap=RIGHT(theline, LENGTH(theLine)-LENGTH('[MP3SWAPBYTES]'))
		END
		OTHERWISE
			NOP
		END
	END
	rc=STREAM(globalData._contentsFile,'C','CLOSE')
END
ELSE DO
	exit
END

call dlgMsgLog.open

'TYPE NUL > 'globalData._logfile
call writeLog 'Logfile for DAO audio CD writing'
call writeLog '--------------------------------'
call writeLog ''
call writeLog 'Datafile is: 'globalData._contentsFile
call writeLog 'Path of audio-CD-folder: 'globalData._folder
call writeLog ''
call writeLog 'Contents of data file is:'
call writeLog ''
DO WHILE LINES(globalData._contentsFile)\=0
	theLine=LINEIN(globalData._contentsFile)
	call writeLog theLine
END
rc=STREAM(globalData._contentsFile,'C','CLOSE')

if globalData.0=0 THEN DO
	call ShowMsgBox 'There are no wave files in the folder. Do not know what to do...', 'Decode MP3','OK'
	EXIT
END

call checkForDAO
call checkForCdrecord

call DIRECTORY globalData._binDir
call RxFuncAdd 'DRCtrlRegister', 'drusrctl.dll', 'DRCtrlRegister'
call DRCtrlRegister

call RxFuncAdd 'mciRxInit', "MCIAPI", "mciRxInit"
Call mciRxInit


rc=RXQUEUE('CREATE',globalData._zQueue)
/*SAY 'Create Queue: 'rc*/
globalData._oldq=RXQUEUE('SET',globalData._zQueue)
/*SAY 'Old Queue: 'globalData._oldq*/

call main.open , 'Write audio CD -DAO-'
�� �d 0Q  Q  �  ��          �  � c !� d ���        ��   l ����      %   � � 3 ? e ���         � " �  n ����         �  +  o ����          �3  3  �����         �r  +  p ����            ��  " 	 x ����        ! ��  ,  q ����          ( ��  "  %����        )  �  '  f ����        ,  �4  '  g ����        3  �  '  m ����        8  ��  '  � ����        =  � / M 0����Write audio CD -DAO mode-    Text           ����      ���� Device:  Fifo:  Speed:  Ok Cancel Hint Help Files to burn on CD ���d 0�  �d Timercall bubbleHelpInit�if globalData._osVersion >=40 then
	call Font '9.WarpSans'

pos=SysIni(globalData._inifile, "windowposition","drdwriteaudiodao")
if pos\="ERROR:" THEN DO
	PARSE var pos x y dx dy
	call position x, y, dx, dy
END
call Timer 500

call show
�� ClickAcall SysSetObjectData globalData._folder, 'DISPLAYHELPPANEL=5400'Initcall text '~Help'�m Initcall isDefault 'DIALOGHINT'�g Click�	globalData._msgText='Do you really want to quit?'
	globalData._msgType='YESNO'
	ret=modalFor('msgBox')

if ret='YES' THEN
DO
	/* Save window position */
	pos=main.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdwriteaudiodao", pos)

	call cleanUp
END
Initcall Text '~Cancel'�f Click}globalData._speed=main.c805.text()
globalData._device=main.efDevice.text()
globalData._fifo=main.fifo.text()

call burnCDInitcall Text '~Burn'�%Init1call Range 0, 32
call Select globalData._speed
�x Init0call Range 0, 32
call Select globalData._fifo
��Initcall Text globalData._device�e Changed�call EventData
item=EventData.1
col=EventData.2

IF item\=0 & co\='VALUE' THEN DO
	call getStem data., item
	globalData._index=data.6
	i=data.6
	IF VERIFY(data.3,'0123456789:')=0 THEN DO
		/* valid */
		PARSE VAR data.3 min':'sec
		if min='' then min=0
		if sec='' then sec=0
		if sec>59 THEN sec=0
		if min>99 then min=0
		globalData.i._pregap=min*60+sec
		if i=1 THEN globalData.i._pregap=2
		data.0=5
		data.3=globalData.i._pregap%60||':'||RIGHT('0'||globalData.i._pregap//60,2)
		call SetStem data, item
	END
	ELSE DO
		data.0=5
		data.3=globalData.i._pregap%60||':'||RIGHT('0'||globalData.i._pregap//60,2)
		call SetStem data, item
	END
	call calculateLength
END
ShowMenu�call EventData
item=EventData.1

IF item\=0 THEN DO
	call getStem data., item
	globalData._index=data.6
	call popup.containerpopup.MenuPopUp
ENDEnter]selected=SELECT()
if selected\=0 THEN DO
   call GetStem data , selected
  i=data.6
END
Init�if globalData._osVersion >=40 then
	call Font '9.WarpSans'
/* ELSE
	call Font '9.Helv'
*/


/* Set format */
call view 'D'
data.0=6
data.1='x_!'
data.2='x_!'
data.3='_!'
data.4='x_!'
data.5='~'
data.6='~'
call SetStem data, 'Format'

/* Set title */
data.0=6
data.1='Name'
data.2='Length'
data.3='Pregap'
data.4='Valid format'
data.5='Samplerate'
data.6=''
call SetStem data, 0

/* Set items */
DO i=1 to globalData.0
	item=Add(i)
	data.0=6
	data.1=FILESPEC('NAME',globalData.i)
	data.2='Calculating...'
	data.3=' '
	data.4=' '
	data.5=' '
	data.6=i
	call SetStem data, item
	globalData.i._item=item
END

call wait.open
call wait.Show

call writeLog ''
call writeLog 'Filling container with items.'

DO i=1 to globalData.0
	call writeLog ''

/*
	IF mciRxSendString('open waveaudio alias wave wait ', retstr,'0','0') \=0 THEN ITERATE
	IF mciRxSendString('load wave "'globalData.i'" wait', retstr,'0','0') \=0 THEN ITERATE
	IF mciRxSendString('SET wave TIME FORMAT MMTIME WAIT', retstr,'0','0') \=0 THEN ITERATE
	IF mciRxSendString('status wave length wait', 'retstr','0','0') \=0 THEN ITERATE
	allSecs=TRUNC(retstr/3000)
	mins=TRUNC(retstr/3000/60)
	secs=TRUNC((retstr/3000)//60)
	sectors=FORMAT((retstr//3000)/40,,0)
	IF sectors\=0 THEN allSecs=allSecs+1
	/*SAY mins':'RIGHT('0'||secs,2)':'RIGHT('0'||sectors,2)*/
	IF mciRxSendString('close wave', retstr,'0','0') \=0 THEN ITERATE
	globalData._totalLength=globalData._totalLength+allSecs
	globalData.i._length=allSecs
	globalData.i._timeString=mins':'RIGHT('0'||secs,2)':'RIGHT('0'||sectors,2)
*/

	launch='"'globalData._binDir||'\daoinfo.exe" "'||globalData.i'" |RXQUEUE '||globalData._zQueue||' /FIFO'
	launch='"'||launch||'"'
	launch
	call writeLog 'Command line: '||launch

	DO WHILE QUEUED()=0
		rc=SysSleep(1)
	END
	DO WHILE QUEUED()\=0
		PULL str allSecs rest
		call writeLog 'Data from queue: '||str||', '||allSecs||', '||rest
	END
		IF RIGHT(str,2)\='00' THEN allSecs=allSecs+1
		globalData._totalLength=globalData._totalLength+allSecs
		globalData.i._length=allSecs
		globalData.i._timeString=str
		data.0=5
		data.1=FILESPEC('NAME',globalData.i)
		data.2=globalData.i._timeString
		IF i=1 THEN DO
			data.3='0:02'
			globalData.i._pregap=2
		END
		ELSE DO
			data.3='0:00'
			globalData.i._pregap=0
		END
		if allSecs\=0 THEN
			data.4='Yes'
		else
			data.4='No'
		data.5=' '
		call SetStem data, globalData.i._item

/*
	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END
*/

END

call calculateLength

call wait.Hide
call main.ok.enable
call main.cancel.enable

DROP data.
DROP i
�l Init0call text 'Source folder:  '||globalData._folder����d 0   � Help	e container�� �� 0t   t   �  ��          J �  � � �  � ��Y         ] �  �  � ����Checking files    Querying audio info... ���� 0U   �� InitKif globalData._osVersion >=40 then
	call Font '9.WarpSans'

call Owner
�� �,0�   �   �  ��          �   W y � 4 ,���        1 � @�  �  /����        �   �  '  -����        �   �2  '  .����Question    Decoding in progress. Do you really want to quit? Yes Push ���,0  �,Opencall show
	call BEEP 1000, 100Init�pos=SysIni(globalData._inifile, "windowposition","drdmsgbox")
if pos\="ERROR:" THEN DO
	PARSE var pos x y dx dy
	call position x, y
END

if globalData._msgTitle\="" THEN
	call Text globalData._msgTitle
�.Click�/* Save window position */
pos=msgbox.Position()
rc=SysIni(globalData._inifile, "windowposition","drdmsgbox", pos)

call msgBox.close

Return 'NO'InitIif globalData._msgType='YESNO' THEN
	call Text '~No'
ELSE
	call hide
�-InitPif globalData._msgType='YESNO' THEN
	call Text '~Yes'
ELSE
	call Text '~Ok'
Click�/* Save window position */
pos=msgbox.Position()
rc=SysIni(globalData._inifile, "windowposition","drdmsgbox", pos)

call msgBox.close

RETURN 'YES'
�/Initcall text globalData._msgText
�� ��0@   @   �  ��           ,   5 � � , ���< All popup menus    � ��0W   W   �      container_menu 8   �      Define pregap...     Save TOC file... ����0E    containerpopup  playzcall definePregap playzcall saveTocFile� �X0T   T   �  ��           J   �  S  X��K         O �  R  Y����     Text ���X0�   �XInit|call color '-', '#255 255 180'

if globalData._osVersion >=40 then
	call Font '9.WarpSans'

else
	call Font '9.Helv'
�� ��0�  �  �  ��    
      X  � r � d ���f         j� M �  �����        k � > #  �����          o � 2 " 
 �����        p �? > #  �����        t �1 2   �����          v �? 2 " 
 �����        w �   �  �����        �  �  6  �����        �  �E  6  �����        �  �  � I �����Define pregap     Min  sec :  Apply... Ok Cancel Pregap ����0�  ��Init=if globalData._osVersion >=40 then
	call Font '9.WarpSans'
��Init<i=globalData._index
call text FILESPEC('name',globalData.i)��Init call text '~Apply to all tracks'��InitLcall Range 0,59
i=globalData._index
call Select globalData.i._pregap//60
��InitIcall Range 0,59
i=globalData._index
call Select globalData.i._pregap%60��Clickcall pregap.closeInitcall text '~Cancel'��Click�

if pregap.applyall.Select()=1 THEN DO
	DO i=2 to globalData.0
		call main.tracks.GetStem data, globalData.i._item
		globalData.i._pregap=pregap.mins.Select()*60+pregap.secs.Select()
		data.3=globalData.i._pregap%60||':'||RIGHT('0'||globalData.i._pregap//60,2)
		data.0=5
		call main.tracks.SetStem data, globalData.i._item
	END
END
ELSE DO
	i=globalData._index
	call main.tracks.GetStem data, globalData.i._item
	IF i=1 THEN DO
		data.3='0:02'
		globalData.i._pregap=2
	END
	ELSE DO
		globalData.i._pregap=pregap.mins.Select()*60+pregap.secs.Select()
		data.3=globalData.i._pregap%60||':'||RIGHT('0'||globalData.i._pregap//60,2)
	END
	data.0=5
	call main.tracks.SetStem data, globalData.i._item
END

call calculateLength

call pregap.close
Initcall text '~Ok'�� � 0�  �  �  ��    
      X  �X z � o  ��i         m � 1 � 4 !����        n  �  %  %����         v �H ! J  �����         w �� ! J  �����     x �     �  (����     �  �      �  )����        �  @  %  "����        �  �I  %  #����        �   B  %  $����        �  �  � S �����Audio CD writing     More...   DRD_PERCENTBAR 0#0 % DRD_PERCENTBAR  Ok Abort Show log... Messages ��� 0�  � Timer�elapsed=TRUNC(TIME('E'))
call elapsedTime.Text 'Time:  'elapsed%60':'RIGHT('00'elapsed//60,2)' min' 
IF globalData._estimatedTime\=0 THEN DO
	tempTime=globalData._estimatedTime-1
	if TempTime>0 THEN DO
		call neededTime.Text '-'tempTime%60':'RIGHT('00'tempTime//60,2)' min'
		globalData._estimatedTime=tempTime
	END
	ELSE
		call neededTime.Text '0:00 min'
ENDInit=if globalData._osVersion >=40 then
	call Font '9.WarpSans'
Notify�call EventData

if TRANSLATE(EventData.1)='CDSIZE' THEN
DO
	SELECT
	WHEN EventData.2='start' THEN DO
		index=lbStatus.add( "Querying free CD space. Please wait...")
		call lbStatus.Select index ,'TOP'
	END
	OTHERWISE
		sec=EventData.2/75
		sec=TRUNC(sec)
		index=lbStatus.add( "Free CD space is: "||sec%60||':'RIGHT('00'||sec//60,2)'min')
		call lbStatus.Select index ,'TOP'
		rc='YES'
		if sec<globalData._cdlength THEN DO
			IF globalData._break\='YES' THEN
				rc=ShowMsgBox( 'There is not enough free space on the CD! '||'0d'x||'On some systems the query fails so you may override this message. Do you want to continue?','', 'YESNO' )
			ELSE
				rc='NO'
		END
		IF rc\='YES' then DO
			call pbAbort.Hide
			call pbStausOk.show
			call pbStausOk.enable
			call pbShowLog.show
			call Timer
		END
		ELSE DO
			IF globalData._break\='YES' THEN DO
				call pbAbort.Hide
				call statusBar.Show
				call bufferBar.Show
				rc=start(startcdrdaoThread)
			END
			ELSE
				call pbStausOk.enable
		END
	END
END

if TRANSLATE(EventData.1)='CDRDAO' THEN
DO
	SELECT
	WHEN TRANSLATE(EventData.2)='LEADIN' THEN DO
		index=lbStatus.add( "Writing leadin...")
		call lbStatus.Select index ,'TOP'
		globalData._StartWriteTime=TRUNC(TIME('E'))
	END
	WHEN TRANSLATE(EventData.2)='LEADOUT' THEN DO
		index=lbStatus.add( "Writing leadout...")
		call lbStatus.Select index ,'TOP'
	END
	WHEN TRANSLATE(EventData.2)='TERMINATED' THEN DO
		call beep 100, 700
		call statusBar.hide
		call bufferBar.hide
		call pbAbort.hide
		call pbStausOk.show
		call pbStausOk.enable
		call pbShowLog.show
		index=lbStatus.add( "")
		call lbStatus.Select index ,'TOP'
		index=lbStatus.add( "!!! Error while writing !!!")
		call lbStatus.Select index ,'TOP'
	/* Dlete communication queue. This kills the feeder prozess */
	rc=RXQUEUE('SET',globalData._oldq)
	rc=RXQUEUE('DELETE',globalData._zQueue)
	call dlgMsgLog.show
	call dlgMsgLog.Top
		call Timer
	END
	WHEN TRANSLATE(EventData.2)='DONE' THEN DO
		call statusBar.hide
		call bufferBar.hide
		call pbAbort.hide
		call pbStausOk.show
		call pbStausOk.enable
		call pbShowLog.show
		call Timer
		call neededTime.Text '0:00 min'
		call BEEP 1000, 100
		call BEEP 2000, 100
		call BEEP 3000, 100
	END
	OTHERWISE
		NOP
	END
END


if TRANSLATE(EventData.1)='TRACK' THEN DO
		idx=FORMAT(EventData.2)
		theText="Writing Track "idx" of "globalData.0": "FILESPEC('N', globalData.idx)
		index=lbStatus.add(theText)
		call lbStatus.Select index ,'TOP'
END

if TRANSLATE(EventData.1)='RESULT' THEN DO
	IF EventData.2\=0 THEN DO
		index=lbStatus.add( "Cdrdao returned: "EventData.2)
		call lbStatus.Select index ,'TOP'
		call ShowMsgBox 'Cdrdao returned non-zero result code! The CD may be corrupted.', 'Warning!', 'OK'
	END
END

if TRANSLATE(EventData.1)='PERCENT' THEN
DO
	percent=WORD(EventData.2, 1)
	buffer=WORD(EventData.2, 2)
	/* For those drives without LEADIN message */
	IF globalData._startWriteTime=0 THEN
		globalData._startWriteTime=TRUNC(TIME('E'))

	tempTime=TRUNC(TIME('E')-globalData._startWriteTime)/*secs*/
	tempTime=TRUNC((100/percent)*tempTime-tempTime)
	globalData._estimatedTime=tempTime
	if TempTime>0 THEN
		call neededTime.Text '-'tempTime%60':'RIGHT('00'tempTime//60,2)' min'
	ELSE
		call neededTime.Text '0:00 min'
	/* update statusbar */
	call statusBar.text TRUNC(PERCENT)||'#'||TRUNC(PERCENT)||' %'
	IF buffer\='' THEN
		call bufferBar.text TRUNC(buffer)||'#Buffer: '||TRUNC(buffer)||' %'
	ELSE
		call bufferBar.text '0#No buffer information'

END

if TRANSLATE(EventData.1)='LOG' THEN
DO
	call writeLog EventData.2
END
Open�sec=globalData._cdlength
index=lbStatus.add( "CD size is: "||sec%60||':'RIGHT('00'||sec//60,2)'min')
call lbStatus.Select index ,'TOP'
call writeLog 'CD size is: 'sec%60||':'RIGHT('00'||sec//60,2)'min'
/* Start CD space thread */
rc=start(queryCDSizeThread)

call TIME 'R'
globalData._estimatedTime=0
call TIMER 1000
globalData._writing='YES'
�$Clicktrc=SysSetObjectData(globalData._logfile,"OPEN=DEFAULT")
rc=SysSetObjectData(globalData._logfile,"OPEN=DEFAULT")

Initcall Text '~Show log...'�#Click?call pbAbort.hide
call pbStausOk.show
globalData._break='YES'Initcall Text '~Abort'�"Clickcall cleanUpInitcall Text '~Ok'�%Initcall text '~More...'Click'call dlgMsgLog.show
call dlgMsgLog.Top�!InitoIF globalData._testonly='YES' THEN
	call Add '--- Simulating only ---'
ELSE
	call Add '--- Real write ---'
�� ��0�   �   �  ��          �   � � � f ����          �  �  � N �����        �   �  '  �����        �  �[  �  �����Message log     Hide Scroll Messages ����0�   ��Init=if globalData._osVersion >=40 then
	call Font '9.WarpSans'
��Init-call Text '~Scroll Messages'
call select 1
��Clickcall dlgMsgLog.HideInitcall Text '~Hide'�