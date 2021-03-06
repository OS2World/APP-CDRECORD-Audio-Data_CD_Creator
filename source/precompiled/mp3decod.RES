� �� 0j  d   mainl  targetFoldere  tracksi  statusk  totalPercentText[ totalPercentj  percentText\ percentf  ok�  doneg  cancelm  hintcontrol�   0  �   wait�   ,  msgBox/  -  .  �  popup�  id3tag�  �  �  �  �  �  � id3title� 	id3artist� id3album� 
id3comment  id3Genre�  Z  X  bubbleY 
bubbleText��� 0�%  writeLogYcall LINEOUT globalData._logfile, ARG(1)
call stream globalData._logfile, 'c', 'close'
cleanUp�/* */
/*SAY 'Old Queue: 'globalData._oldq
*/
rc=RXQUEUE('SET',globalData._oldq)
rc=RXQUEUE('DELETE',globalData._zQueue)
/*SAY 'DeleteQueue: 'rc*/

/* Save window position */
pos=main.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdmp3decode", pos)
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

RETURN ret	checkForZ�
decoder=globalData._mp3Decoder

IF decoder=1   THEN
	globalData._zPath=globalData._binDir||'\mp3decod.exe'
ELSE
	globalData._zPath=globalData._mp3DecoderPath

IF STRIP(globalData._zPath)='' THEN globalData._zPath="NOT VALID"

IF decoder=1  | (decoder=2 & STREAM(globalData._zPath,'C','query exists')='')
THEN DO
	globalData._msgType='OK'
	call ShowMsgBox 'The selected MP3 decoder is either mpg123 or the path to z! is not valid or if using MMIO procedures the helper programm mp3decod.exe is lost. Check the settings in the main settings notebook.', 'Setup problem','OK'
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "OPEN=DEFAULT")
	exit
END

bubbleHelp�if TRANSLATE(hintcontrol.text())='CONTAINER' THEN
DO
	call bubble.open
	call bubble.bubbleText.text 'Right button for more'
	bubble=1
END
ELSE DO
	IF bubble=1 THEN
		call bubble.close
	bubble=0
END	playwithz�/* say 'Playing...' */

IF STREAM(globalData._zPath,'C','query exists')=''
THEN DO
	globalData._msgType='OK'
	call ShowMsgBox 'The path to z! is not valid! Check the settings in the main settings notebook. Playing is only possible when z! is installed.', 'Setup problem','OK'
	signal return
END

index=globalData._index
if index\='' THEN DO
	launch='"'globalData._zPath||'" "'||globalData.index'"'
	launch='"'launch'"'
	'@start "Playing: 'FILESPEC('NAME',globalData.index)'" /C /F 'launch
END
decodeAllThread�globalData.0=Val('global.0')
DO a=1 to globalData.0
  globalData.a=Val('global.'a)
  globalData.a._length=Val('global.'a'_length')
END

globalData._folder=Val('global._folder')
globalData._zPath=Val('global._zPath')
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

init�call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
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
			globalData._logfile=globalData._installDir'\logfiles\MP3-dec.log'
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
		WHEN LEFT(theLine,LENGTH('[MP3DECODERPATH]'))='[MP3DECODERPATH]' THEN
		DO
			globalData._mp3DecoderPath=RIGHT(theline, LENGTH(theLine)-LENGTH('[MP3DECODERPATH]'))
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

if globalData.0=0 THEN DO
	call ShowMsgBox 'There are no MP3s selected in the folder. Do not know what to do...', 'Decode MP3','OK'
	EXIT
END

/* Clear log */
'type NUL > 'globalData._logfile

call writeLog "Trying to register percent bar..."
call DIRECTORY globalData._installDir'\bin'
/* Register percent bar */
call RxFuncAdd 'DRCtrlRegister', 'drusrctl' , 'DRCtrlRegister'

call DRCtrlRegister

call checkForZ

rc=RXQUEUE('CREATE',globalData._zQueue)
globalData._oldq=RXQUEUE('SET',globalData._zQueue)

call writeLog "Opening main dialog"
call main.open , 'Decode MP3 files'
�� �d 09  9  �  ��         
 �  M ? � d ���        �� �  l ����      %   � � ? ? e ���         �� .  i ����        � � ! !  k ����     �  �   �& ! �  [����        � �  !  j ����           �&  �  \����         �
  $  f ����             $  � ����          �>  $  g ����          �  6  m ����        $  ��  $  � ����        )  � : M 0����Decode MP3    Text           ����      ���� Total DRD_PERCENTBAR  Track DRD_PERCENTBAR  Ok Done Cancel Hint Help Files to decode ���d 0(  �d Timercall bubbleHelpNotify�/* These events are coming from the decode thread */
call EventData

if TRANSLATE(EventData.1)='PERCENT' THEN
DO
	call main.percent.Text EventData.2||' #'EventData.2' %'
END

if TRANSLATE(EventData.1)='TOTALPERCENT' THEN
DO
	call main.totalpercent.Text EventData.2||' #'EventData.2' %'
END

if TRANSLATE(EventData.1)='WHICH' THEN
DO
	a=EventData.2
	theText="Decoding "||a||" of "globalData.0||":  "globalData.a
	call main.status.Text theText
END

if TRANSLATE(EventData.1)='DONE' THEN
DO
	globalData._decoding='NO'
	call status.text 'Done'
	call ok.hide
	call cancel.hide
	call done.show
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

pos=SysIni(globalData._inifile, "windowposition","drdmp3decode")
if pos\="ERROR:" THEN DO
	call writeLog "Reading window position: "||pos
	PARSE var pos x y dx dy
	call position x, y, dx, dy
END
call writeLog ""
call Timer 500

call show

�� ClickCcall SysSetObjectData globalData._folder, 'DISPLAYHELPPANEL=5300'
Initcall text '~Help'�m Initcall isDefault 'DIALOGHINT'�g Click�if globalData._decoding='YES' THEN DO
	call beep 1000, 100
	globalData._msgText='Decoding in progress. Do you really want to quit?'||'0D'x||'0D'x||'Note: Decoding of the current file will continue.'
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
Initcall Text '~Cancel'�� Initcall text '~Done'Clickcall cleanUp
�f Click�/*call decodeAll
*/

rc=Val('global.0',globalData.0)
DO a=1 to globalData.0
  rc=Val('global.'a, globalData.a)
  rc=Val('global.'a'_length', globalData.a._length)

END
rc=Val('global._folder',globalData._folder)
rc=Val('global._zPath',globalData._zPath)
rc=Val('global._zQueue',globalData._zQueue)
rc=Val('global._totalLength',globalData._totalLength)
rc=Val('global._break', globalData._break)
rc=VAL('global._mp3Decoder', globalData._mp3Decoder)
rc=VAL('global._installDir', globalData._installDir)
rc=VAL('global._logfile', globalData._logfile)

/* Start decode thread */
rc=start(decodeAllThread)
call disable
globalData._decoding='YES'
Initcall Text '~Decode'�\Initcall TEXT '0 #0 %'�[Initcall TEXT '0 #0 %'
�e ShowMenu�call EventData
item=EventData.1

IF item\=0 THEN DO
	call getStem data., item
	globalData._index=data.6
	call popup.containerpopup.MenuPopUp
ENDEnter�selected=SELECT()
if selected\=0 THEN DO
   call GetStem data , selected
   globalData._index=data.6
  call playWithZ  
END
Init�if globalData._osVersion >=40 then
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
data.2='Length'
data.3='Bitrate'
data.4='Channels'
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
thePos=main.Position()
PARSE var thePos x y dx dy
call wait.Position x+20, TRUNC(y+dy/2)

call wait.Show

decError=0
/* Call z! to get the size */
DO i=1 to globalData.0
/*
	launch='"'globalData._zPath||'" -n "'||globalData.i'" |RXQUEUE '||globalData._zQueue||' /FIFO'
	
LAUNCH='"'||launch||'"'
launch
*/
	IF globalData._mp3Decoder=2 THEN
		launch='"'globalData._zPath||'" -n "'||globalData.i'" |RXQUEUE '||globalData._zQueue||' /FIFO'
	ELSE
		launch='"'globalData._binDir||'\daoinfo.exe" "'||globalData.i'" 3 |RXQUEUE '||globalData._zQueue||' /FIFO'
	launch='"'||launch||'"'
	launch

	DO WHILE QUEUED()=0
		rc=SysSleep(1)
	END
	DO WHILE QUEUED()\=0
		IF globalData._mp3Decoder=2 THEN DO
			PULL str
			parse var str khz channels mp3len bitrate
			IF VERIFY(str,'1234567890 ')\=0 THEN DO
			call ShowMsgBox 'The executable selected on the MP3 decoder page seems not to be z!'||'0d'x||'0d'x||'Decoding program will be terminated.', 'Decode MP3','OK'
				decError=1
				LEAVE
			END
			data.2=mp3len%60||':'||RIGHT('0'||mp3len//60,2)
		END
		ELSE DO
			PULL str mp3len channels khz .
			data.2=str
		END
		data.0=5
		data.1=FILESPEC('NAME',globalData.i)
		data.3='---'
		data.4=channels
		data.5=khz
		globalData.i._length=mp3len
		globalData._totalLength=globalData._totalLength+mp3len
		call SetStem data, globalData.i._item
	END
	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END
	IF decError=1 THEN LEAVE
END
call wait.Hide
call main.ok.enable
call main.cancel.enable


DROP data.
DROP i

IF decError=1 THEN call cleanUp�l Init.call text 'Destination:  '||globalData._folder����d 0   � Help	e container�� �� 0d   d   �  ��           J   � � �  � ��K         O �   �  � ����     Querying MP3 info... ���� 0U   �� InitKif globalData._osVersion >=40 then
	call Font '9.WarpSans'

call Owner
�� �,0�   �   �  ��          �   Z | � , ,���          � @�  �  /����        �   �  #  -����        �   �.  #  .����Question     Yes Push ���,0  �,Open	call showInit�if globalData._osVersion >=40 then
	call Font '9.WarpSans'

if globalData._msgTitle\="" THEN
	call Text globalData._msgTitle

pos=SysIni(globalData._inifile, "windowposition","drdmsgbox")
if pos\="ERROR:" THEN DO
	PARSE var pos x y dx dy
	call position x, y
END
�.Click~
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
Click�
pos=msgbox.Position()
rc=SysIni(globalData._inifile, "windowposition","drdmsgbox", pos)

call msgBox.close

RETURN 'YES'
�/Initcall text globalData._msgText
�� ��0@   @   �  ��           ,   6 � � , ���< All popup menus    � ��0K   K   �      container_menu ,   �      Play with z!     ID3 info ����0A    containerpopup  playzcall playwithz  call modalFor id3Tag� ��0    �  ��          �  �� G � v ����         �� X �  �����        � � I *  �����        � � = *  �����        � � 1 *  �����        � � % *  �����        � �  *  �����         � �1 I |  �����         � �1 = |  �����         � �1 1 |  �����         � �1 % |  �����         � �1  |   ����        �  �  '  �����        �  �  � \ Z����ID3 tag info     Title: Artist: Album: Comment: Genre:      Push ID3 tag info ����0�  ��Init�if globalData._osVersion >=40 then
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

call Text 'File:  '||FILESPEC("name", globalData.i)�� �X0T   T   �  ��           J   �  S  X��K         O �  Q 
 Y����     Text ���X0�   �XInit|call color '-', '#255 255 180'

if globalData._osVersion >=40 then
	call Font '9.WarpSans'

else
	call Font '9.Helv'
�