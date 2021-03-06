� �� 0s  d   mainl  targetFoldere  tracksn  totalLengtho   # pbAbortp   $ 	pbShowLogq   %  f  okg  cancelm  hintcontrol�   0  �   wait�   ,  msgBox/  -  .  �  popupX  bubbleY 
bubbleText   
burnStatus! lbStatus%  � elapsedTime� 
neededTime( 	statusBar" 	pbStausOk# pbAbort$ 	pbShowLog�  �  	dlgMsgLog� lbMsg�  ��� 0 4  	checkForZ�IF STRIP(globalData._zPath)='' THEN globalData._zPath="NOT VALID"

IF globalData._decoder\=2  | STREAM(globalData._zPath,'C','query exists')=''
THEN DO
	globalData._msgType='OK'
	call ShowMsgBox 'The selected MP3 decoder is not z! or the path is not valid. Check the settings in the main settings notebook. MP3 writing is disabled.', 'Setup problem','OK'
	globalData._MP3='NO'
END
writeLog�call LINEOUT globalData._logfile, ARG(1)
call STREAM globalData._logfile, 'C', 'close'
index=dlgMsgLog.lbMsg.Add( ARG(1))
call dlgMsgLog.lbMsg.Select index, 'TOP'
startCdrdaoThread�
globalData._folder=Val('global._folder')
globalData._cdrdaoPath=Val('global._cdrdaoPath')
globalData._zQueue=Val('global._zQueue')
globalData._binDir=Val('global._binDir')
globalData._cdrdaoCmd=Val('global._cdrdaoCmd')
globaldata._tocFile=Val('global._tocFile')
globaldata._cdLength=Val('global._cdlength')
globalData._logfile=VAL('global._logfile')

mByte=25+15+TRUNC(globalData._cdlength*44.1*4/1000)

status=''
resultCode=0

oldq=RXQUEUE('Set',globalData._zQueue)

call Notify 'burnStatus', 'LOG', ' '
call Notify 'burnStatus', 'LOG', 'Starting to write in DAO mode...'

launch='"'globalData._binDir'\daowrite.exe" '
globalData._cdrdaoPath='""'globalData._cdrdaoPath'""'
launch=launch||' "'globalData._cdrdaoPath'" '
globalData._cdrdaoCmd='""'globalData._cdrdaoCmd'""'
launch=launch||' "'globalData._cdrdaoCmd'"'
globaldata._tocFile='""'globaldata._tocFile'""'
launch=launch||' "'globaldata._tocFile'" '
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
			END
			IF POS('LEAD-OUT',TRANSLATE(str))\=0 THEN DO
				call Notify 'burnStatus', 'cdrdao', 'LEADOUT'
				status='LEADOUT'
			END
			IF WORD(TRANSLATE(str),1)='WROTE' & WORD(TRANSLATE(str),3)='OF' THEN DO
				IF status='LEADIN' THEN DO
					call Notify 'burnStatus', 'PERCENT', WORD(TRANSLATE(str),2) *100 / mByte
				END
				ELSE DO
				IF status='LEADOUT' THEN DO
					call Notify 'burnStatus', 'PERCENT', (mByte-15+WORD(TRANSLATE(str),2)) *100 / mByte
				END
				ELSE DO
					call Notify 'burnStatus', 'PERCENT', (25+WORD(TRANSLATE(str),2)) *100 / mByte
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

	call Notify 'burnStatus', 'cdrdao', 'DONE'
	call Notify 'burnStatus', 'RESULT', resultCode

	call Notify 'burnStatus', 'LOG', ' '
	call Notify 'burnStatus', 'LOG', 'Writing finished with RC='resultCode
queryCDSizeThread�globalData._folder=Val('global._folder')
globalData._cdrecordPath=Val('global._cdrecordPath')
globalData._zQueue=Val('global._zQueue')
globalData._CDLength=Val('global._CDLength')
globalData._binDir=Val('global._binDir')
globalData._logfile=VAL('global._logfile')

oldq=RXQUEUE('Set',globalData._zQueue)

	call Notify 'burnStatus', 'LOG', ' '
	call Notify 'burnStatus', 'LOG', 'Querying free CD space...'

	/* Set status text */
	call Notify 'burnStatus', 'cdsize', 'start'

launch='"'globalData._cdrecordPath'" '
launch=launch||' dev=0,1,0 -v -toc 2>&1 |RXQUEUE '||globalData._zQueue||' /FIFO'
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

writeThread	/* */

checkForCdrecord�IF STRIP(globalData._cdrecordPath)='' THEN globalData._cdrecordPath="NOT VALID"
IF STREAM(globalData._cdrecordPath,'C','query exists')=''
THEN DO
	globalData._msgType='OK'
	theString='The path to CDRecord/2 is not valid. Check the settings in the main settings notebook.'
	call ShowMsgBox theString, 'Setup problem','OK'
	call writeLog ''
	call writeLog theString
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "OPEN=DEFAULT")
	exit
END
calculateLength�/* */

globalData._cdLength=globalData._totalLength+2*globalData.0

theString='Play time: '||globalData._totalLength%60':'globalData._totalLength//60'          '
theString=theString||'CD-length: '||globalData._cdLength%60':'globalData._cdLength//60
call main.totallength.text theString
burnCD�call writeLog 'Tocfile name is: '

/* Save window position */
pos=main.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdwriteaudiotao", pos)

/* Make some vars global */
rc=Val('global._folder',globalData._folder)
rc=Val('global._cdrecordPath',globalData._cdrecordPath)
rc=Val('global._cdrdaoPath',globalData._cdrdaoPath)
rc=Val('global._zQueue',globalData._zQueue)
rc=Val('global._cdLength',globalData._cdLength)
rc=Val('global._binDir',globalData._binDir)
rc=Val('global._cdrdaoCmd',globalData._cdrdaoCmd)
rc=Val('global._tocFile',globaldata._tocFile)
rc=Val('global._cdlength', globaldata._cdLength)
rc=VAL('global._logfile', globalData._logfile)

call main.close

call burnStatus.open , 'Audio CD writing...'
cleanUp�/* */
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

RETURN retinit�+call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs



globalData.=''
globalData.0=0

globalData._contentsFile=ARG(1)


globalData._contentsFile='G:\Projects_working\Audio_cdcreator_051\source\contents.005'
globalData._zPath='E:\Audio-Programme\z-sep30alpha\z z.exe'
globalData._MP3='YES'
globalData._decoder=2
call checkForZ

globalData._zQueue='ZOUT'||TIME('s') /* Build unique queue name */
globalData._osVersion=RIGHT(SysOS2Ver(),2)
globalData._totalLength=0

/* Parse input file */
IF STREAM(globalData._contentsFile,'C','QUERY EXIST')\='ERROR:' THEN DO
	i=0
	globalData._folder=LINEIN(globalData._contentsFile)
	DO WHILE LINES(globalData._contentsFile)\=0
		theLine=LINEIN(globalData._contentsFile)
		SELECT
		WHEN LEFT(theLine,LENGTH('[SHADOW;SELECTED]'))='[SHADOW;SELECTED]' THEN
		DO
		IF TRANSLATE(RIGHT(theLine,3))='WAV' THEN DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[SHADOW;SELECTED]'))
			globalData.0=i
			globalData.i._wave='YES'
		END
		IF TRANSLATE(RIGHT(theLine,3))='MP3' THEN DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[SHADOW;SELECTED]'))
			globalData.0=i
			globalData.i._wave='NO'
		END
		END
		WHEN LEFT(theLine,LENGTH('[SHADOW;UNSELECTED]'))='[SHADOW;UNSELECTED]' THEN
		DO
		IF TRANSLATE(RIGHT(theLine,3))='WAV' THEN DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[SHADOW;UNSELECTED]'))
			globalData.0=i
			globalData.i._wave='YES'
		END
		IF TRANSLATE(RIGHT(theLine,3))='MP3' THEN DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[SHADOW;UNSELECTED]'))
			globalData.0=i
			globalData.i._wave='NO'
		END
		END
		WHEN LEFT(theLine,LENGTH('[FILE;SELECTED]'))='[FILE;SELECTED]' THEN
		DO
		IF TRANSLATE(RIGHT(theLine,3))='WAV' THEN DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[FILE;SELECTED]'))
			globalData.0=i
			globalData.i._wave='YES'
		END
		IF TRANSLATE(RIGHT(theLine,3))='MP3' THEN DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[FILE;SELECTED]'))
			globalData.0=i
			globalData.i._wave='NO'
		END
		END
		WHEN LEFT(theLine,LENGTH('[FILE;UNSELECTED]'))='[FILE;UNSELECTED]' THEN
		DO
		IF TRANSLATE(RIGHT(theLine,3))='WAV' THEN DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[FILE;UNSELECTED]'))
			globalData.0=i
			globalData.i._wave='YES'
		END
		IF TRANSLATE(RIGHT(theLine,3))='MP3' THEN DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[FILE;UNSELECTED]'))
			globalData.0=i
			globalData.i._wave='NO'
		END
		END
		WHEN LEFT(theLine,LENGTH('[INIFILE]'))='[INIFILE]' THEN
		DO
			globalData._inifile=RIGHT(theline, LENGTH(theLine)-LENGTH('[INIFILE]'))
		END
		WHEN LEFT(theLine,LENGTH('[INSTALLDIR]'))='[INSTALLDIR]' THEN
		DO
			globalData._installDir=RIGHT(theline, LENGTH(theLine)-LENGTH('[INSTALLDIR]'))
			globalData._binDir=globalData._installDir'\bin'
			globalData._logfile=globalData._installDir'\logfiles\WriteTAO.log'
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
call writeLog 'Logfile for TAO audio CD writing'
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

call DIRECTORY globalData._binDir
call RxFuncAdd 'DRCtrlRegister', 'drusrctl.dll', 'DRCtrlRegister'
call DRCtrlRegister

call checkForCdrecord


rc=RXQUEUE('CREATE',globalData._zQueue)
/*SAY 'Create Queue: 'rc*/
globalData._oldq=RXQUEUE('SET',globalData._zQueue)
/*SAY 'Old Queue: 'globalData._oldq*/

call main.open , 'Write audio CD -TAO-'�� �d 0Q  Q  �  ��          �  K : >� d ���        �� � . l ����      %   � � G )? e ���         � 2 �  n ����         � ! +  o ����          �6 # 3  #����         �q ! +  p ����            �� " " 
 $����        ! �� ! +  q ����          ( �� " " 
 %����        )  �  5  f ����        ,  �I  5  g ����        3  �  6  m ����        8   �  5  � ����        =  � C 9M 0����Write audio CD -TAO mode-    Text           ����      ���� Device:  Fifo:  Speed:  Ok Cancel Hint Help Files to burn on CD ���d 0�  �d Init�if globalData._osVersion >=40 then
	call Font '9.WarpSans'

pos=SysIni(globalData._inifile, "windowposition","drdwriteaudiotao")
if pos\="ERROR:" THEN DO
	PARSE var pos x y dx dy
	call position x, y, dx, dy
END

call show
�� ClickCcall SysSetObjectData globalData._folder, 'DISPLAYHELPPANEL=5300'
Initcall text '~Help'�m Initcall isDefault 'DIALOGHINT'�g Click�	globalData._msgText='Do you really want to quit?'
	globalData._msgType='YESNO'
	ret=modalFor('msgBox')

if ret='YES' THEN
DO
	/* Save window position */
	pos=main.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdwriteaudiotao", pos)

	call cleanUp
END
Initcall Text '~Cancel'�f Clickcall burnCDInitcall Text '~Burn'�%Init1call Range 0, 32
call Select globalData._speed
�$Init0call Range 0, 32
call Select globalData._fifo
�#Initcall Text globalData._device�e Changed�call EventData
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
/*	call popup.containerpopup.MenuPopUp */
ENDEnter]selected=SELECT()
if selected\=0 THEN DO
   call GetStem data , selected
  i=data.6
END
Init�if globalData._osVersion >=40 then
	call Font '9.WarpSans'


/* Set format */
call view 'D'
data.0=6
data.1='x_!'
data.2='x_!'
data.3='~x_!' /* Pregap */
data.4='x_!'
data.5='x_!'
data.6='~'
data.7='~'
call SetStem data, 'Format'

/* Set title */
data.0=7
data.1='Name'
data.2='Length'
data.3='Pregap'
data.4='Valid format'
data.5='Bitrate'
data.6=''
data.7=''
call SetStem data, 0

/* Set items */
DO i=1 to globalData.0
	item=Add(i)
	data.0=7
	data.1=FILESPEC('NAME',globalData.i)
	data.2='Calculating...'
	data.3=' '
	data.4=' '
	data.5=' '
	data.6=i /* index */
	data.7=' '
	call SetStem data, item
	globalData.i._item=item
END

call wait.open
call wait.Show


DO i=1 to globalData.0
	launch='"'globalData._binDir||'\daoinfo.exe" "'||globalData.i'" |RXQUEUE '||globalData._zQueue||' /FIFO'
	if globalData.i._wave='NO' & globalData._MP3='YES' THEN
		launch='"'globalData._zPath||'" -n "'||globalData.i'" |RXQUEUE '||globalData._zQueue||' /FIFO'

	launch='"'||launch||'"'
	launch
	DO WHILE QUEUED()=0
		rc=SysSleep(1)
	END
	DO WHILE QUEUED()\=0
		PULL str
		IF globalData.i._wave='NO' & globalData._MP3='YES' THEN DO
			parse var str khz channels mp3len bitrate
			globalData.i._length=mp3len
			globalData._totalLength=globalData._totalLength+globalData.i._length
			globalData.i._channels=channels
			globalData.i._bitrate=bitrate
		END
		ELSE DO
			globalData._totalLength=globalData._totalLength+str
			globalData.i._length=str
			globalData.i._channels='2'
			globalData.i._bitrate='---'
		END
		data.0=7
		data.1=FILESPEC('NAME',globalData.i)
		data.2=globalData.i._length%60||':'||RIGHT('0'||globalData.i._length//60,2)
		data.3='0:00'
		globalData.i._pregap=0
		IF i=1 THEN DO
			data.3='0:02'
			globalData.i._pregap=2
		END
		ELSE DO
			data.3='0:00'
			globalData.i._pregap=0
		END
		/* Length of 0 means some kind of error */
		if globalData.i._length\=0 THEN
			data.4='Yes'
		else 
		   IF globalData._MP3='YES' THEN data.4='No'
			ELSE
			    	data.4='No MP3 decoder'

		data.5=globalData.i._bitrate
		data.6=i
		data.7=' '
		call SetStem data, globalData.i._item
	END
	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END
END

call calculateLength

call wait.Hide
call main.ok.enable
call main.cancel.enable

DROP data.
DROP i
�l Init0call text 'Source folder:  '||globalData._folder����d 0   � Help	e container�� �� 0j   j   �  ��           J   � � �  � ��K         O �  �  � ����     Querying wave file info... ���� 0U   �� InitKif globalData._osVersion >=40 then
	call Font '9.WarpSans'

call Owner
�� �,0�   �   �  ��          �   �U w 9 ,���        1 � @�   /����        �   �  1  -����        �   �A  5  .����Question    Decoding in progress. Do you really want to quit? Yes Push ���,0�  �,Open	call BEEP 1000, 100InitCif globalData._msgTitle\="" THEN
	call Text globalData._msgTitle
�.Click call msgBox.close

Return 'NO'InitIif globalData._msgType='YESNO' THEN
	call Text '~No'
ELSE
	call hide
�-InitPif globalData._msgType='YESNO' THEN
	call Text '~Yes'
ELSE
	call Text '~Ok'
Click#call msgBox.close

RETURN 'YES'
�/Initcall text globalData._msgText
�� ��0@   @   �  ��           ,   5 � � , ���< All popup menus    � ��0W   W   �      container_menu 8   �      Define pregap...     Save TOC file... ����0E    containerpopup  playzcall definePregap playzcall saveTocFile� �X0T   T   �  ��           J   �  S  X��K         O �  R  Y����     Text ���X0�   �XInit|call color '-', '#255 255 180'

if globalData._osVersion >=40 then
	call Font '9.WarpSans'

else
	call Font '9.Helv'
�� � 0�  �  �  ��    	      :  �0 i � f  ��K         O �	 1 � & !����        P  �	  6  %����         X �I ! J  �����         Y �� ! J  �����     Z i    	 �  (����        o  @	  -  "����        r  �_  6  #����        x   A  6  $����        �  �  � G �����Audio CD writing     More...   DRD_PERCENTBAR 0#0 % Ok Abort Show log... Messages ��� 0  � Timer�elapsed=TRUNC(TIME('E'))
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
Notify�call EventData

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
		index=lbStatus.add( "Free CD space is: "||sec%60||':'sec//60'min')
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
	WHEN TRANSLATE(EventData.2)='DONE' THEN DO
		call statusBar.hide
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
		index=lbStatus.add( "Writing Track "EventData.2)
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
	percent=EventData.2
	tempTime=TRUNC(TIME('E')-globalData._startWriteTime)/*secs*/
	tempTime=TRUNC((100/percent)*tempTime-tempTime)
	globalData._estimatedTime=tempTime
	if TempTime>0 THEN
		call neededTime.Text '-'tempTime%60':'RIGHT('00'tempTime//60,2)' min'
	ELSE
		call neededTime.Text '0:00 min'
	call statusBar.text TRUNC(EventData.2)||'#'||TRUNC(EventData.2)||' %'

END

if TRANSLATE(EventData.1)='LOG' THEN
DO
	call writeLog EventData.2
END
Open�sec=globalData._cdlength
index=lbStatus.add( "CD size is: "||sec%60||':'sec//60'min')
call lbStatus.Select index ,'TOP'
call writeLog 'CD size is: 'sec%60||':'sec//60'min'
/* Start CD space thread */
rc=start(queryCDSizeThread)

call TIME 'R'
globalData._estimatedTime=0
call TIMER 1000
globalData._writing='YES'
�$ClickA'@start /F "Logfile for DAO writing" e.exe 'globalData._logfile  Initcall Text '~Show log...'�#Click?call pbAbort.hide
call pbStausOk.show
globalData._break='YES'Initcall Text '~Abort'�"Clickcall cleanUpInitcall Text '~Ok'�%Initcall text '~More...'Click'call dlgMsgLog.show
call dlgMsgLog.Top�!InitoIF globalData._testonly='YES' THEN
	call Add '--- Simulating only ---'
ELSE
	call Add '--- Real write ---'
�� ��0~   ~   �  ��          h   � � � f ���t          x  �  � K �����        y   �  6  �����Message log     Hide ����0{   ��Init=if globalData._osVersion >=40 then
	call Font '9.WarpSans'
��Clickcall dlgMsgLog.HideInitcall Text '~Hide'�