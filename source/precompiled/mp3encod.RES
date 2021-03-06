� �� 04  d   mainl  targetFoldere  tracksi  statusj  percentText\ percentf  ok�  doneg  cancel�   0  �   wait�   ,  msgBox/  -  .  �  popup�  id3tag�  �  �  �  �  �  � id3title� 	id3artist� id3album� 
id3comment  id3Genre�  Z  X  bubbleY 
bubbleText��� 0�5  encodeThread�globalData.0=Val('global.0')
DO a=1 to globalData.0
  globalData.a=Val('global.'a)
  globalData.a._length=Val('global.'a'_length')
END

globalData._folder=Val('global._folder')
globalData._mp3EncoderPath=Val('global._mp3EncoderPath')
globalData._mp3EncoderQuality=VAL('global._mp3EncoderQuality')
globalData._mp3EncoderBitrate=VAL('global._mp3EncoderBitrate')
globalData._mp3EncoderOptions=VAL('global._mp3EncoderOptions')
globalData._zQueue=Val('global._zQueue')
globalData._length=Val('global._length')
globalData._totalLength=Val('global._totalLength')
globalData._mp3Decoder=Val('global._mp3Decoder')
globalData._installDir=VAL('global._installDir')
globalData._logfile=VAL('global._logfile')
globalData._mp3EncoderPriority=VAL('global._mp3EncoderPriority')

oldq=RXQUEUE('Set',globalData._zQueue)

call writeLog "Encoding thread started."
doneLen=0

DO a=1 to globalData.0
	done=0

	call writeLog ""
	call writeLog "Encoding "||globalData.a

	IF TRANSLATE(RIGHT(globalData.a ,4))\='.MP3' THEN
		target=globalData._folder||'\'LEFT(FILESPEC('name',globalData.a),LENGTH(FILESPEC('name',globalData.a))-3)||'mp3'
	ELSE
		target=globalData._folder||'\1_'LEFT(FILESPEC('name',globalData.a),LENGTH(FILESPEC('name',globalData.a))-3)||'mp3'
	targetSave=target /* We need this for tag copy */
	target='"'target'"'

	call writeLog "Filename: "||target

	launch='"'globalData._mp3EncoderPath||'" --nohist'

	/* Set quality */
	SELECT
	WHEN globalData._mp3EncoderQuality=1 THEN
		launch=launch||' --preset standard'
	WHEN globalData._mp3EncoderQuality=2 THEN
		launch=launch||' --preset extreme'
	WHEN globalData._mp3EncoderQuality=3 THEN
		launch=launch||' --preset 'globalData._mp3EncoderBitrate
	OTHERWISE
		NOP
	END
	/* Add priority setting */
	launch=launch||' --priority '||globalData._mp3EncoderPriority

	/* Add user options */
	launch=launch||' '||globalData._mp3EncoderOptions
	/* add source file path */
	launch=launch||' "'||globalData.a'" '
	launch=launch||target||' 2>&1 |RXQUEUE '||globalData._zQueue||' /FIFO & ECHO DONE |RXQUEUE 'globalData._zQueue||' /FIFO'
	launch='"'launch'"'

	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END
	'RXQUEUE 'globalData._zQueue' /CLEAR'
/*
	/* Check size */
	bytes=globalData.a._length*44100*2*2
	diskInfo=SysDriveInfo(filespec("drive",globalData._folder))
	PARSE VAR diskInfo drive freeSpace rest
	if bytes>freespace THEN DO
		call Notify 'main', 'nospace', bytes||' '||freespace||' 'a
		Done=1
		LEAVE
	END
*/

	/* Set status text */
	call Notify 'main', 'which', a

	call writeLog "Command line is: "||launch

	'@start "Encode MP3 (VIO)" /C /B /MIN 'launch
		done=0

	call writeLog ""
	call writeLog "Now reading output from Encoding program..."
	call writeLog ""
	/* Get percent from queue */
	DO forever
		str=''
		IF QUEUED()\=0 THEN Do
			PULL str
			call writeLog str
			IF POS("%", str)\=0 & POS("AVER", TRANSLATE(str))=0 THEN DO
				PARSE VAR str WITH . '(' thePercent '%)' rest
				thePercent=STRIP(thePercent)
				IF thePercent\="" THEN DO
					call Notify 'main', 'percent', thePercent
				END
			END
				
		END
		if STRIP(TRANSLATE(str))='DONE' THEN LEAVE

		IF QUEUED()=0 THEN
			rc=SysSleep(1)
	END

	/* Now copy track info */
	ret=SysSetObjectData(globalData.a, "MMAUDIOCOPYTAGTO="targetSave)
	/* And refresh details view */
	ret=SysSetObjectData(targetSave, "MMREFRESH=1")

/*
	/* Already decoded */
	doneLen=doneLen+globalData.a._length
*/
	IF Val('global._break')='YES' then leave 
END
IF done=0 THEN
	IF Val('global._break')\='YES' then
		call Notify 'main', 'totalpercent', 100

/* Clean queue */
DO WHILE QUEUED()\=0
	PULL str
END

'RXQUEUE 'globalData._zQueue' /CLEAR'
oldq=RXQUEUE('Set',oldq)

call Notify 'main', 'done'

checkForLame�IF STRIP(globalData._mp3EncoderPath)='' THEN globalData._mp3EncoderPath="NOT VALID"

IF STREAM(globalData._mp3EncoderPath,'C','query exists')=''
THEN DO
	globalData._msgType='OK'
	call ShowMsgBox 'The path to LAME is not valid. Check the settings in the main settings notebook.', 'Setup problem','OK'
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "OPEN=DEFAULT")
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "DISPLAYHELPPANEL=800")
	exit
END
writeLogYcall LINEOUT globalData._logfile, ARG(1)
call stream globalData._logfile, 'c', 'close'
cleanUp�/* */
/*SAY 'Old Queue: 'globalData._oldq
*/
rc=RXQUEUE('SET',globalData._oldq)
rc=RXQUEUE('DELETE',globalData._zQueue)
/*SAY 'DeleteQueue: 'rc*/

/* Save window position */
pos=main.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdmp3encode", pos)
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

RETURN retdecodeAllThread�globalData.0=Val('global.0')
DO a=1 to globalData.0
  globalData.a=Val('global.'a)
  globalData.a._length=Val('global.'a'_length')
END

globalData._folder=Val('global._folder')
globalData._mp3EncoderPath=Val('global._mp3EncoderPath')
globalData._zQueue=Val('global._zQueue')
globalData._length=Val('global._length')
globalData._totalLength=Val('global._totalLength')
globalData._mp3Decoder=Val('global._mp3Decoder')
globalData._installDir=VAL('global._installDir')
globalData._logfile=VAL('global._logfile')

oldq=RXQUEUE('Set',globalData._zQueue)

call writeLog "Decoding thread started."
doneLen=0
DO a=1 to globalData.0
	done=0

	call writeLog ""
	call writeLog "Decoding "||globalData.a

	IF TRANSLATE(RIGHT(globalData.a ,3))\='WAV' THEN
		target=globalData._folder||'\'LEFT(FILESPEC('name',globalData.a),LENGTH(FILESPEC('name',globalData.a))-3)||'wav'
	ELSE
		target=globalData._folder||'\1_'LEFT(FILESPEC('name',globalData.a),LENGTH(FILESPEC('name',globalData.a))-3)||'wav'
	targetSave=target /* We need this for tag copy */
	target='"'target'"'

	call writeLog "Filename: "||target
	
	IF globalData._mp3Decoder=2 THEN DO
		launch='"'globalData._zPath||'" -w2'
		launch=launch||' "'||globalData.a'" '
		launch=launch||target||' |RXQUEUE '||globalData._zQueue||' /FIFO & ECHO DONE |RXQUEUE 'globalData._zQueue||' /FIFO'
		launch='"'launch'"'
	END
	ELSE DO
		/* Decoder is MMIOMP3 */
		launch='"'globalData._installDir||'\bin\mp3decod.exe" 0 DUMMY1 DUMMY2 DUMMY3 '
		launch=launch||' "'||globalData.a'" "'||globalData._installDir||'" '||globalData._mp3Decoder||' '
		launch=launch||target||' |RXQUEUE '||globalData._zQueue||' /FIFO & ECHO DONE |RXQUEUE 'globalData._zQueue||' /FIFO'
		launch='"'launch'"'
	END


	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END
	'RXQUEUE 'globalData._zQueue' /CLEAR'

	/* Check size */
	bytes=globalData.a._length*44100*2*2
	diskInfo=SysDriveInfo(filespec("drive",globalData._folder))
	PARSE VAR diskInfo drive freeSpace rest
	if bytes>freespace THEN DO
		call Notify 'main', 'nospace', bytes||' '||freespace||' 'a
		Done=1
		LEAVE
	END

	/* Set status text */
	call Notify 'main', 'which', a

	call writeLog "Command line is: "||launch

	'@start "Decode MP3 (VIO)" /C /B /MIN cmd.exe /C 'launch
		done=0

	call writeLog ""
	call writeLog "Now reading output from decoding program..."
	call writeLog ""
	/* Get percent from queue */
	DO forever
		str=''
		IF QUEUED()\=0 THEN Do
			PULL str
			call writeLog str

			IF RIGHT(str,1)='%' THEN DO
				if LENGTH(str)>1 THEN
					b=LEFT(str,LENGTH(str)-1)
				ELSE
					b=0
				call Notify 'main', 'percent', b
				/* Total done */
				tempLen=(b*globalData.a._length)%100
				tempSel=1+((tempLen+doneLen)*100)%globalData._totallength
				
/*SAY globalData.a._length' tempLen: 'tempLen' tempSel: 'tempSel' done: '||x||'b: '||b*/
				call Notify 'main', 'totalpercent', tempSel			
			END
		END
		if STRIP(TRANSLATE(str))='DONE' THEN LEAVE

		IF QUEUED()=0 THEN
			rc=SysSleep(1)
	END

	/* Now copy track info */
	ret=SysSetObjectData(globalData.a, "MMAUDIOCOPYTAGTO="targetSave)
	/* And refresh details view */
	ret=SysSetObjectData(targetSave, "MMREFRESH=1")

	/* Already decoded */
	doneLen=doneLen+globalData.a._length
	IF Val('global._break')='YES' then leave 
END
IF done=0 THEN
	IF Val('global._break')\='YES' then
		call Notify 'main', 'totalpercent', 100

/* Clean queue */
DO WHILE QUEUED()\=0
	PULL str
END

'RXQUEUE 'globalData._zQueue' /CLEAR'
oldq=RXQUEUE('Set',oldq)

call Notify 'main', 'done'

init�(call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

globalData.=''
globalData.0=0

globalData._contentsFile=ARG(1)

/*globalData._contentsFile='G:\Projects_working\Audio_cdcreator_050\source\contents.005'
*/
if globalData._contentsFile='' THEN Exit

parse source . . sourceFile

globalData._zQueue='ZOUT'||TIME('s') /* Build unique queue name */
globalData._osVersion=RIGHT(SysOS2Ver(),2)
globalData._totalLength=0
globalData._break='NO'

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
		WHEN LEFT(theLine,LENGTH('[FILE;SELECTED]'))='[FILE;SELECTED]' THEN
		DO
			i=i+1
			globalData.i=RIGHT(theline, LENGTH(theLine)-LENGTH('[FILE;SELECTED]'))
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
			globalData._logfile=globalData._installDir'\logfiles\MP3-Enc.log'
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
		WHEN LEFT(theLine,LENGTH('[MP3ENCODERQUALITY]'))='[MP3ENCODERQUALITY]' THEN
		DO
			globalData._mp3EncoderQuality=RIGHT(theline, LENGTH(theLine)-LENGTH('[MP3ENCODERQUALITY]'))
		END
		WHEN LEFT(theLine,LENGTH('[MP3ENCODERBITRATE]'))='[MP3ENCODERBITRATE]' THEN
		DO
			globalData._mp3EncoderBitrate=RIGHT(theline, LENGTH(theLine)-LENGTH('[MP3ENCODERBITRATE]'))
		END
		WHEN LEFT(theLine,LENGTH('[MP3ENCODERPATH]'))='[MP3ENCODERPATH]' THEN
		DO
			globalData._mp3EncoderPath=RIGHT(theline, LENGTH(theLine)-LENGTH('[MP3ENCODERPATH]'))
		END
		WHEN LEFT(theLine,LENGTH('[MP3ENCODEROPTIONS]'))='[MP3ENCODEROPTIONS]' THEN
		DO
			globalData._mp3EncoderOptions=RIGHT(theline, LENGTH(theLine)-LENGTH('[MP3ENCODEROPTIONS]'))
		END
		WHEN LEFT(theLine,LENGTH('[MP3ENCODERPRIORITY]'))='[MP3ENCODERPRIORITY]' THEN
		DO
			globalData._mp3EncoderPriority=RIGHT(theline, LENGTH(theLine)-LENGTH('[MP3ENCODERPRIORITY]'))
		END
		WHEN LEFT(theLine,LENGTH('[MP3NAMEPART'))='[MP3NAMEPART' THEN DO
				idx=RIGHT(theline, LENGTH(theLine)-LENGTH('[MP3NAMEPART'))
				idx=LEFT(idx,1)
				mp3Name.idx=STRIP(RIGHT(theLine,1))
				mp3Name.0=idx
		END
		WHEN LEFT(theLine,LENGTH('[MP3NAMEFILLSTRING'))='[MP3NAMEFILLSTRING' THEN DO
				idx=RIGHT(theline, LENGTH(theLine)-LENGTH('[MP3NAMEFILLSTRING'))
				idx=LEFT(idx,1)
				mp3Name.idx._fillString=RIGHT(theline, LENGTH(theLine)-LENGTH('[MP3NAMEFILLSTRING1]'))
		END
		WHEN LEFT(theLine,LENGTH('[MP3LIBRARY]'))='[MP3LIBRARY]' THEN
				globalData._mp3Library=RIGHT(theline, LENGTH(theLine)-LENGTH('[MP3LIBRARY]'))
		OTHERWISE
			NOP
		END
	END
	rc=STREAM(globalData._contentsFile,'C','CLOSE')
END
ELSE DO
	exit
END

if globalData.0=0 THEN DO
	call ShowMsgBox 'There are no wave files selected in the folder. Do not know what to do...', 'Encode MP3','OK'
	EXIT
END

/* Clear log */
'type NUL > 'globalData._logfile

call writeLog "Trying to register percent bar..."
call DIRECTORY globalData._installDir'\bin'
/* Register percent bar */
call RxFuncAdd 'DRCtrlRegister', 'drusrctl' , 'DRCtrlRegister'

call DRCtrlRegister

call checkForLame

rc=RXQUEUE('CREATE',globalData._zQueue)
globalData._oldq=RXQUEUE('SET',globalData._zQueue)

call writeLog "Opening main dialog"
call main.open , 'Encode files to MP3'
�� �d 0�  �  �  ��    
      X  C > � d ��j         n�   l ����      %   o � 4 ? e p��         �� #  i ����        � �  '  j ����     �  �   �.  �  \����        � �  &  f ����        �     &  � ����        �  �8  &  g ����        �  ��  &  � ����        �  � 0 M 0����Encode wave files               ����      ���� Track: DRD_PERCENTBAR  Encode Done Cancel Help Files to encode ���d 0�  �d Notify�/* These events are coming from the decode thread */
call EventData

if TRANSLATE(EventData.1)='PERCENT' THEN
DO
	call main.percent.Text EventData.2||' #'EventData.2' %'
END

if TRANSLATE(EventData.1)='WHICH' THEN
DO
	a=EventData.2
	theText="Encoding "||a||" of "globalData.0||":  "globalData.a
	call main.status.Text theText
END

if TRANSLATE(EventData.1)='DONE' THEN
DO
	globalData._decoding='NO'
	call status.text 'Done'
	call ok.hide
	call cancel.hide
	call done.show
	call beep 1000, 100
	call beep 2000, 100
	call beep 3000, 100
END

if TRANSLATE(EventData.1)='NOSPACE' THEN
DO
	PARSE VAR EventData.2 bytes freespace a
	globalData._decoding='NO'
	globalData._msgText='There is not enough space on the drive with the destination folder.'
	globalData._msgText=globalData._msgText||'0d'X||'0d'X||'Needed: 'bytes||' bytes for "'||FILESPEC('name',globalData.a)||'"'||'0d'X
	globalData._msgText=globalData._msgText||'Available: 'freespace' bytes'
	globalData._msgType='OK'
	rc=modalFor('msgBox')
END

Init�if globalData._osVersion >=40 then
	call Font '9.WarpSans'

pos=SysIni(globalData._inifile, "windowposition","drdmp3encode")
if pos\="ERROR:" THEN DO
	call writeLog "Reading window position: "||pos
	PARSE var pos x y dx dy
	call position x, y, dx, dy
END
call writeLog ""

call show

�� ClickAcall SysSetObjectData globalData._folder, 'DISPLAYHELPPANEL=5600'Initcall text '~Help'�g Click�if globalData._decoding='YES' THEN DO
	call beep 1000, 100
	globalData._msgText='Encoding in progress. Do you really want to quit?'||'0D'x||'0D'x||'Note: Encoding of the current file will continue.'
	globalData._msgType='YESNO'
	ret=modalFor('msgBox')
	if ret='YES' THEN
	DO
		globalData._break='YES'
		rc=Val('global._break', globalData._break)
		call disable
	END
END
ELSE
DO
	call cleanUp
END
Initcall Text '~Cancel'�� Initcall text '~Done'Clickcall cleanUp�f Click�/*call decodeAll
*/

rc=Val('global.0',globalData.0)
DO a=1 to globalData.0
  rc=Val('global.'a, globalData.a)
  rc=Val('global.'a'_length', globalData.a._length)
END

rc=Val('global._folder',globalData._folder)
rc=Val('global._MP3EncoderPath',globalData._MP3EncoderPath)
rc=Val('global._zQueue',globalData._zQueue)
rc=VAL('global._mp3EncoderQuality', globalData._mp3EncoderQuality)
rc=VAL('global._mp3EncoderBitrate', globalData._mp3EncoderBitrate)
rc=VAL('global._mp3EncoderOptions', globalData._mp3EncoderOptions)
rc=Val('global._totalLength',globalData._totalLength)
rc=Val('global._break', globalData._break)
rc=VAL('global._mp3Decoder', globalData._mp3Decoder)
rc=VAL('global._installDir', globalData._installDir)
rc=VAL('global._logfile', globalData._logfile)
rc=VAL('global._mp3EncoderPriority', globalData._mp3EncoderPriority)

/* Start decode thread */
rc=start(encodeThread)
call disable
globalData._decoding='YES'
Initcall Text '~Encode'�\Initcall TEXT '0 #0 %'�e Init�if globalData._osVersion >=40 then
	call Font '9.WarpSans'


/* Set format */
call view 'D'
data.0=6
data.1='x_!'
data.2='x_!'
data.3='~'
data.4='x_!'
data.5='x_!'
data.6='~'
call SetStem data, 'Format'

/* Set title */
data.0=6
data.1='Name'
/*data.2='Length'
data.3='Bitrate'
data.4='Channels'
data.5='Samplerate'*/
data.2=' '
data.3=' '
data.4=' '
data.5=' '
data.6=''
call SetStem data, 0

/* Set items */
DO i=1 to globalData.0
	item=Add(i)
	data.0=6
	data.1=FILESPEC('NAME',globalData.i)
/*	data.2='Calculating...'*/
	data.2='---'
	data.3=' '
	data.4=' '
	data.5=' '
	data.6=i
	call SetStem data, item
	globalData.i._item=item
END


DO i=1 to globalData.0
		data.0=5
		data.1=FILESPEC('NAME',globalData.i)
		data.3='---'
		data.4="---"/*channels*/
		data.5="---"/*khz*/
		globalData.i._length=0/*mp3len*/
		globalData._totalLength=globalData._totalLength+0
		call SetStem data, globalData.i._item
END


DROP data.
DROP i

call main.ok.enable
call main.cancel.enable

�l Init.call text 'Destination:  '||globalData._folder����d 0   � Help�� �� 0d   d   �  ��           J   � � �  � ��K         O �  �  � ����     Querying MP3 info... ���� 0U   �� InitKif globalData._osVersion >=40 then
	call Font '9.WarpSans'

call Owner
�� �,0�   �   �  ��          �   \ x � 5 ,���          � @�  �  /����        �   �  !  -����        �   �,  !  .����Question     Yes Push ���,0  �,Open	call showInit�if globalData._osVersion >=40 then
	call Font '9.WarpSans'

if globalData._msgTitle\="" THEN
	call Text globalData._msgTitle

pos=SysIni(globalData._inifile, "windowposition","drdmsgbox")
if pos\="ERROR:" THEN DO
	PARSE var pos x y dx dy
	call position x, y
END
�.Click|pos=msgbox.Position()
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
Clickpos=msgbox.Position()
rc=SysIni(globalData._inifile, "windowposition","drdmsgbox", pos)

call msgBox.close

RETURN 'YES'
�/Initcall text globalData._msgText
�� ��0@   @   �  ��           ,   5 � � , ���< All popup menus    � ��0K   K   �      container_menu ,   �      Play with z!     ID3 info ����0A    containerpopup  playzcall playwithz  call modalFor id3Tag� ��0    �  ��          �  �� > � � ����         ��	 p �  �����        � �	 ^ *  �����        � �	 N *  �����        � �	 ? *  �����        � �	 1 *  �����        � �	 " *  �����         � �3 ^ |  �����         � �3 N |  �����         � �3 ? |  �����         � �3 1 |  �����         � �3 " |   ����        �  �	  5  �����        �  �  � i Z����ID3 tag info     Title: Artist: Album: Comment: Genre:      Push ID3 tag info ����0�  ��Init�if globalData._osVersion >=40 then
	call Font '9.WarpSans'

if globalData._decoding='YES' THEN DO
	call ShowMsgBox 'Sorry, no ID3 tag querying during decoding.','ID3 tag','OK' 
	call close
	SIGNAL return
END

call Top
call Owner main
call wait.Owner id3tag
call wait.Top
call wait.Show

i=globalData._index
/* Clean queue */
DO WHILE QUEUED()\=0
	PULL str
END
/* Call mp3info to get the tag */
launch='"'globalData._binDir||'\mp3info.exe" -f "%t!$!%a!$!%l!$!%c!$!%g" "'||globalData.i'"  |RXQUEUE '||globalData._zQueue||' /FIFO'
launch='"'||launch||'"'
launch
/* Wait for Queue to contain contents */
waited=0
brk=0
DO WHILE QUEUED()=0
	rc=SysSleep(1)
	waited=waited+1
	IF waited>2 THEN DO
		brk=1
		leave
	END
END
if brk=0 THEN DO
	parse PULL track'!$!'artist'!$!'album'!$!'comment'!$!'genre

	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END
	call id3title.text track
	call id3artist.text artist
	call id3album.text album
	call id3comment.text comment
	call id3genre.text genre
END

call wait.Owner main
call wait.hide
��Initcall text '~Ok'Clickcall id3tag.close��InitJi=globalData._index

call Text 'File:  '||FILESPEC("name", globalData.i)�� �X0T   T   �  ��           J   �  S  X��K         O �  R  Y����     Text ���X0�   �XInit|call color '-', '#255 255 180'

if globalData._osVersion >=40 then
	call Font '9.WarpSans'

else
	call Font '9.Helv'
�