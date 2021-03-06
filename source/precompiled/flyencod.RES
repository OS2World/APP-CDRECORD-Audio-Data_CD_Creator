� �� 0�   d   mainl  targetFolderi  statusk  totalPercentTextj  percentText[ totalPercent\ percent�  doneg  cancel�   �  ,  msgBox/  -  .  �  popup�   �  �  �  �  �  �  �  �  �  �  �  �  �  �  ��� 0�Q  	convertCP�	convertCP: procedure expose str1 str2 
/**************************************************************************/
/* 8859-1-850.CMD: Conversion script created with Text Converter 0.9.2    */
/* Source code page: 8859-1 (ISO Latin 1 Western); EOL: MAC[cr]           */
/* Target code page: 850 (Latin 1 - Multilingual); EOL: DOS[crlf]         */
/* Conversion rules: all identical characters                             */
/* 96 characters for conversion                                           */
/*                                                                        */
/* Syntax: 8859-1-850.CMD source file(s)                                  */
/**************************************************************************/
 
PARSE ARG theText
 
/* Missing parameter */
IF STRIP(theText) = "" THEN DO
    return ""
END
  
/* Nothing to do */
IF str1 = str2 THEN DO
   return theText
END
  
/* Start of conversion */

    /* Conversion of characters */
    SELECT
        WHEN str1 = str2 THEN chars2 = theText
        OTHERWISE chars2 = TRANSLATE(theText,str2,str1)
    END
/* End of conversion */
 
return chars2

	prepareCP�prepareCP:
/* Prepare codepage conversion                           */
/* Source and target translation tables                   */
/* Source code page: 8859-1 (ISO Latin 1 Western);  */
/* Target code page: 850 (Latin 1 - Multilingual);      */
hex1 = "A0A1A2A3A4A5A6A7A8A9AAABACADAEAFB0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"||,
       "C0C1C2C3C4C5C6C7C8C9CACBCCCDCECFD0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"||,
       "E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEFF0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF"
hex2 = "FFADBD9CCFBEDDF5F9B8A6AEAAF0A9EEF8F1FDFCEFE6F4FAF7FBA7AFACABF3A8"||,
       "B7B5B6C78E8F9280D490D2D3DED6D7D8D1A5E3E0E2E5999E9DEBE9EA9AEDE8E1"||,
       "85A083C6848691878A8288898DA18C8BD0A495A293E494F69B97A39681ECE798"
str1 = X2C(hex1)
str2 = X2C(hex2)
return
buildTrackNames�/* */

DO a=1 to globalData.0
	globalData.a._newName=""
	DO b=1 to mp3Name.0
	SELECT
	WHEN mp3Name.b=0 THEN NOP
	WHEN mp3Name.b=1 THEN globalData.a._newName=globalData.a._newName||globaldata._album
	WHEN mp3Name.b=2 THEN globalData.a._newName=globalData.a._newName||globaldata._artist
	WHEN mp3Name.b=3 THEN globalData.a._newName=globalData.a._newName||globaldata.a._title
	WHEN mp3Name.b=4 THEN globalData.a._newName=globalData.a._newName||RIGHT("0"||globaldata.a,2)
	OTHERWISE
		NOP
	END/* SELECT */
	IF b<4 THEN
		globalData.a._newName=globalData.a._newName||mp3Name.b._fillString
	END
	globalData.a._newName=globalData.a._newName||'.mp3'
	globalData.a._newName=TRANSLATE(globalData.a._newName,"    __+", ":?<>*\/&")
ENDrefreshThread�objectToRefresh=Val('objectPath')
/* Let the object rename itself */
rc=SysSleep(2)
rc=SysSetObjectData(objectToRefresh, "MMREFRESH=1;")checkTargetFolder�
/* */
currdir = Directory()
testDir=DIRECTORY(globalData._folder)
currdir = Directory(currdir)

IF testDir="" THEN
DO
	/* Folder doesn't exist, create it */
/*
 * Extended version of the 'mkdir' utility that makes all
 * of the directories in a path. I.e., if e:\tmp does not
 * exist and we invoke DoMkDir('e:\tmp\bin\foo') then it
 * will perform a
 *	mkdir e:\tmp
 *	mkdir e:\tmp\bin
 *	mkdir e:\tmp\bin\foo
 */

	fullpath=globalData._folder

	currdir = Directory()
	output = 1
	if Pos(':\',fullpath) = 0 then
		path = ''
	else do
		parse var fullpath path ':\' fullpath
		path = path':\'
		if Directory(path) = '' then do
			output = 0
			fullpath = ''
		end
	end
	do while fullpath \= ''
		parse var fullpath first '\' fullpath
		path = path || first
		if Directory(path) = '' then DO
			'mkdir "'path'"'
			if rc \= 0 then do
				say 'It was not possible to make' path
				output = 0
				fullpath = ''
			end
			else if Directory(path) = '' then do
				say 'I could not change to' path
				output = 0
				fullpath = ''
			end
		end /* if Directory... */
		path = path || '\'
	end /* do while path... */
          'cd \'
	call Directory currdir

	currdir = Directory()
	testDir=DIRECTORY(globalData._folder)
	currdir = Directory(currdir)

	IF testDir="" THEN RETURN 0
	RETURN 1
END
ELSE
	return 1
getcddbInfo�/*
globalData._cddbid
globalData._cddbdataDir
*/

globaldata._album="Album"
globaldata._artist="Artist"

globalData._cddbdataFile=globalData._cddbdataDir||'\'||globalData._cddbId
if STREAM(globalData._cddbdatafile ,'c','query exists')="" THEN DO
	globalData._haveCddbData="NO"
	globaldata._album="Album"
	globaldata._artist="Artist"
	DO a=1 to globalData.0
		globalData.a._title="Track "RIGHT("0"||globalData.a,2)
		globaldata.a._album=globaldata._album
		globaldata.a._artist=globaldata._artist
	END
	RETURN
END

globalData._haveCddbData="YES"
rc=readCddbData(globalData._cddbdataFile)
globalData._disktitle=searchCddbValue("DTITLE=")


thePos=LASTPOS('/',globaldata._disktitle)
IF thePos\=0 THEN DO
	globaldata._album=STRIP(RIGHT(globaldata._disktitle, LENGTH(globaldata._disktitle)-thePos))
	globaldata._artist=STRIP(LEFT(globaldata._disktitle,thePos-1))
   END
   ELSE DO
	globaldata._album=globaldata._disktitle
   	globaldata._artist=globaldata._disktitle
   END

DO a=1 to globalData.0
	IF globalData.a<>1 THEN
		globalData.a._title=searchCddbValue("TTITLE"||STRIP(globalData.a-1,L,0))
	ELSE
		globalData.a._title=searchCddbValue("TTITLE0")
	   thePos=LASTPOS('/',globaldata._disktitle)
   IF thePos\=0 THEN DO
	globaldata.a._album=STRIP(RIGHT(globaldata._disktitle, LENGTH(globaldata._disktitle)-thePos))
	globaldata.a._artist=STRIP(LEFT(globaldata._disktitle,thePos-1))
   END
   ELSE DO
	globaldata.a._album=globaldata._disktitle
   	globaldata.a._artist=globaldata._disktitle
   END
END

RETURN

searchCddbValue: PROCEDURE expose cddb. str1 str2

parse arg keyword

found=0
DO i=1 to cddb.0 while found=0
  if POS(keyword,cddb.i)=1 then found=i
END

if found<>0 then return convertCP(SUBSTR(cddb.found,POS('=',cddb.found)+1))

return ''

/* Read the cddb data into a compund var */
readCddbData:

cddbdatafile=ARG(1)

/* Clear var */
DROP cddb.
cddb.=""
cddb.0=0

/* Read the file */
DO i=cddb.0+1 while lines(cddbdatafile)<>0
   cddb.i=LINEIN(cddbdatafile)
END
cddb.0=i-1
call stream cddbdatafile, 'C', 'close'
return 0

checkForhelper�IF STREAM(globalData._encodeStatusHelper,'C','query exists')=''
THEN DO
	globalData._msgType='OK'
	call ShowMsgBox 'The executable ENCSTAT.EXE cannot be found in the bin directory.', 'Setup problem','OK'
	exit
END
checkForCdda2wav�IF STRIP(globalData._grabberPath)='' THEN globalData._grabberPath="NOT VALID"

IF STREAM(globalData._grabberPath,'C','query exists')='' | globalData._grabberId\=1
THEN DO
	globalData._msgType='OK'
	call ShowMsgBox 'The path to cdda2wav is not valid or the selected grabber is not cdda2wav. Check the settings in the main settings notebook.', 'Setup problem','OK'
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "OPEN=DEFAULT")
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "DISPLAYHELPPANEL=800")
	exit
END

IF POS("%2", globaldata._grabberOptions)=0
THEN DO
	globalData._msgType='OK'
	call ShowMsgBox 'The parameter %2 is missing in the grabber options. Check the settings in the main settings notebook.', 'Setup problem','OK'
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "OPEN=DEFAULT")
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "DISPLAYHELPPANEL=800")
	exit
ENDencodeThread�%globalData.0=Val('global.0')
DO a=1 to globalData.0
  globalData.a=Val('global.'a)
  globalData.a._length=Val('global.'a'_length')
  globalData.a._title=Val('global.'a'_title')
  globalData.a._album=Val('global.'a'_album')
  globalData.a._artist=Val('global.'a'_artist')
  globalData.a._newName=Val('global.'a'._newName')
END

globalData._folder=Val('global._folder')
globalData._mp3EncoderPath=Val('global._mp3EncoderPath')
globalData._mp3EncoderQuality=VAL('global._mp3EncoderQuality')
globalData._mp3EncoderBitrate=VAL('global._mp3EncoderBitrate')
globalData._mp3EncoderOptions=VAL('global._mp3EncoderOptions')

globalData._encodeStatusHelper=VAL('global._encodeStatusHelper')

globalData._grabberPath=Val('global._grabberPath')
globalData._grabberLog=Val('global._grabberLog')

globalData._grabberOptions=Val('global._grabberOptions')
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
	call writeLog "Encoding Track"||globalData.a

	/*target=globalData._folder||'\encod'||globalData.a||'.mp3'*/
	target=globalData._folder||'\'||globalData.a._newName
	targetSave=target /* We need this for setting the tags */
	target='"'target'"'

	newTitle=RIGHT("0"||globalData.a,2)||' '||globalData.a._title||'.mp3'
	call writeLog "Target name: "||target

	launch='"'globalData._grabberPath||'" '||globalData._grabberOptions||' -O raw - 2>'
	launch=launch||'"'||globalData._grabberlog||'"'

	/* Add the tracknumber */
	IF POS("%2", launch)\=0
		THEN launch=OVERLAY(RIGHT(" "||globalData.a,2), launch , POS("%2", launch))
	
	launch=launch||'|"'globalData._mp3EncoderPath||'" --nohist'

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
	launch=launch||' '||globalData._mp3EncoderOptions||' - '
	/* add source file path */
	launch=launch||target

	launch='"'launch'"'

	/* This helper reads the output of cdda2wav from the logfile and writes it into the queue
	so we get the percent values */
	helper='"'||globalData._encodeStatusHelper||'" "'globalData._grabberlog||'" 2>&1 |RXQUEUE '||globalData._zQueue||' /FIFO'
	helper='"'helper'"'

	/* Clean queue */
	DO WHILE QUEUED()\=0
		PULL str
	END
	'RXQUEUE 'globalData._zQueue' /CLEAR'

	/* Set status text */
	call Notify 'main', 'which', a

	call writeLog "Command line is: "||launch
	call writeLog "Helper command is: "||helper

	'@start "Encode MP3 (VIO)" /C /B /MIN 'launch

	'@start "Encode MP3 Status (VIO)" /C /B /MIN 'helper
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
			if STRIP(TRANSLATE(str))='DONE' THEN LEAVE
			ELSE DO
				thePercent=STRIP(str)
				IF thePercent\="" THEN DO
					call Notify 'main', 'percent', thePercent
					IF globalData.0=1 THEN
						tempDone=thePercent
					ELSE DO
						tempDone=doneLen+TRUNC((globalData.a._length*thePercent)/100)
						tempDone=TRUNC((tempDone/globalData._totalLength)*100)
					END
					call Notify 'main', 'totalpercent', tempDone
				END
			END
				
		END

		IF QUEUED()=0 THEN
			rc=SysSleep(1)
	END
	call Notify 'main', 'percent', "100"
	ret=SysSetObjectData(targetSave, "MMAUDIOTRACKNAME="globalData.a._title)
	ret=SysSetObjectData(targetSave, "MMAUDIOALBUM="globalData.a._album)
	ret=SysSetObjectData(targetSave, "MMAUDIOARTIST="globalData.a._artist)
/*
	rc=SysSetObjectData(targetSave, "TITLE="newTitle";MMREFRESH=1;")
	rc=SysSetObjectData(targetSave, "TITLE="newTitle)
*/
	call Notify 'main', 'refreshobject', targetSave

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

checkForLame�IF STRIP(globalData._mp3EncoderPath)='' THEN globalData._mp3EncoderPath="NOT VALID"

IF STREAM(globalData._mp3EncoderPath,'C','query exists')=''
THEN DO
	globalData._msgType='OK'
	call ShowMsgBox 'The path to LAME is not valid. Check the settings in the main settings notebook.', 'Setup problem','OK'
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "OPEN=DEFAULT")
	ret=SysSetObjectData("<CWCREATOR_SETTINGS>", "DISPLAYHELPPANEL=5500")
	exit
END
writeLogYcall LINEOUT globalData._logfile, ARG(1)
call stream globalData._logfile, 'c', 'close'
cleanUp�/* */
/*SAY 'Old Queue: 'globalData._oldq
*/
rc=RXQUEUE('SET',globalData._oldq)
rc=RXQUEUE('DELETE',globalData._zQueue)
/*SAY 'DeleteQueue: 'rc*/

/* Save window position */
pos=main.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdontheflyencodestatus", pos)
call RxFuncDrop 'FlyEncGetResString'
call RxFuncDrop 'FlyEncMessageBox'
call RxFuncDrop 'FlyEncGetHWND'

exit

ShowMsgBox�/* */
text=ARG(1)
Title=ARG(2)
type=ARG(3)

globalData._msgType=type
globalData._msgText=text
globalData._msgTitle=title

ret=modalFor(msgBox, ,"Message")

DROP text
DROP title
DROP type

RETURN ret	playwithz�/* say 'Playing...' */

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
init�5call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

globalData.=''
globalData.0=0
mp3Name.=''
mp3Name.0=0

globalData._contentsFile=ARG(1)

if globalData._contentsFile='' THEN Exit

parse source . . sourceFile

globalData._zQueue='ZOUT'||TIME('s') /* Build unique queue name */
globalData._osVersion=RIGHT(SysOS2Ver(),2)
globalData._totalLength=0
globalData._break='NO'
globalData._mp3library=""

/* Parse input file */
IF STREAM(globalData._contentsFile,'C','QUERY EXIST')\='ERROR:' THEN DO
	i=0
	globalData._folder=LINEIN(globalData._contentsFile)
	DO WHILE LINES(globalData._contentsFile)\=0
		theLine=LINEIN(globalData._contentsFile)
		SELECT
		WHEN LEFT(theLine,LENGTH('[TRACKSELECTED]'))='[TRACKSELECTED]' THEN
		DO
			i=i+1
			globalData.i=WORD(RIGHT(theline, LENGTH(theLine)-LENGTH('[TRACKSELECTED]')),1)
			globalData.i._length=STRIP(WORD(RIGHT(theline, LENGTH(theLine)-LENGTH('[TRACKSELECTED]')),2))
			globalData.0=i
			globalData._totalLength=globalData._totalLength+globalData.i._Length
		END
		WHEN LEFT(theLine,LENGTH('[CDDBID]'))='[CDDBID]' THEN
		DO
			globalData._cddbid=RIGHT(theline, LENGTH(theLine)-LENGTH('[CDDBID]'))
		END
		WHEN LEFT(theLine,LENGTH('[GRABBERPATH]'))='[GRABBERPATH]' THEN
		DO
			globalData._grabberPath=RIGHT(theline, LENGTH(theLine)-LENGTH('[GRABBERPATH]'))
		END
		WHEN LEFT(theLine,LENGTH('[GRABBEROPTIONS]'))='[GRABBEROPTIONS]' THEN
		DO
			globalData._grabberOptions=RIGHT(theline, LENGTH(theLine)-LENGTH('[GRABBEROPTIONS]'))
		END
		WHEN LEFT(theLine,LENGTH('[GRABBERID]'))='[GRABBERID]' THEN
		DO
			globalData._grabberId=RIGHT(theline, LENGTH(theLine)-LENGTH('[GRABBERID]'))
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
			globalData._grabberlog=globalData._installDir'\logfiles\grabbing.log'
			globalData._cddbdataDir=globalData._installDir'\cddbdata'
		END
		WHEN LEFT(theLine, LENGTH('[LANGUAGEDLL]'))='[LANGUAGEDLL]' THEN
			globalData._languageDLL=RIGHT(theLine, LENGTH(theLine)-LENGTH('[LANGUAGEDLL]'))

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

call prepareCP

globalData._encodeStatusHelper=globalData._binDir||'\encstat.exe'

/* Clear log */
'type NUL > 'globalData._logfile

call writeLog "Trying to register percent bar..."
call DIRECTORY globalData._installDir'\bin'
/* Register percent bar */
call RxFuncAdd 'DRCtrlRegister', 'drusrctl' , 'DRCtrlRegister'
call RxFuncAdd 'FlyEncGetResString', 'drusrctl' , 'CreatorGetResString'
call RxFuncAdd 'FlyEncMessageBox', 'drusrctl' , 'CreatorMessageBox'
call RxFuncAdd 'FlyEncGetHWND', 'drusrctl' , 'CreatorGetHWND'

call DRCtrlRegister

/* Check if helper programs are installed */
call checkForLame
call checkForCdda2wav

/* Try to read FreeDB info from the data file in the FreeDB data dir */
call getcddbInfo
/* Build tracknames from FreeDB info according to user setting.
   This includes removing unsupported characters. */
call buildTrackNames

rc=RXQUEUE('CREATE',globalData._zQueue)
globalData._oldq=RXQUEUE('SET',globalData._zQueue)

call writeLog "Opening main dialog"
call d700.open , 'Encode on the fly'
�� �d 0�  �  �  ��    
      X  P j � [ d ��n         r@� > �  l ����         s � 1 �  i ����        t � # '  k ����        z �  '  j ����     �  �   �1 $ �  [����     �  �   �1  �  \����        �     5  � ����        �  �  3  g ����        �  ��  5  � ����        �  �  � H �����Encode MP3 on the fly      Total Track DRD_PERCENTBAR  DRD_PERCENTBAR  Done Break Help Ecoding status ���d 0  �d Notify�
/* These events are coming from the decode thread */
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
	theText="Encoding "||a||" of "globalData.0||":  Track "globalData.a" - "globalData.a._title 
	call main.status.Text theText
END

if TRANSLATE(EventData.1)=TRANSLATE("refreshobject") THEN
DO
	/* MP3 file created. Read the audio info */
	rc=Val('objectPath', EventData.2)
	call START refreshThread
END

if TRANSLATE(EventData.1)='DONE' THEN
DO
	globalData._decoding='NO'
	call status.text 'Done'
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
call TEXT FlyEncGetResString(globalData._languageDLL, 4721)

pos=SysIni(globalData._inifile, "windowposition","drdontheflyencodestatus")
if pos\="ERROR:" THEN DO
	call writeLog "Reading window position: "||pos
	PARSE var pos x y dx dy
	call position x, y, dx, dy
END
call writeLog ""

call show

��Init=call TEXT FlyEncGetResString(globalData._languageDLL, 4731)
�� ClickEcall SysSetObjectData "<CWCREATOR_SETTINGS>", 'DISPLAYHELPPANEL=5700'Init;call TEXT FlyEncGetResString(globalData._languageDLL, 4716)�g Click�NUMERIC DIGITS 15 /* We need this for caculating with hex numbers */

if globalData._decoding='YES' THEN DO
	hwnd=FlyEncGetHWND(0, 100)

	theText=FlyEncGetResString(globalData._languageDLL, 4734)
	theTitle=FlyEncGetResString(globalData._languageDLL, 4733)
	ret=FlyEncMessageBox(theText, theTitle, "YESNO", "QUESTION", "MOVEABLE", hwnd )
	if ret=6 THEN
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
Init=call TEXT FlyEncGetResString(globalData._languageDLL, 4717)
�� Initcall text '~Done'Clickcall cleanUp�\Initcall TEXT '0 #0 %'�[Initcall TEXT '0 #0 %'
�j Init=call TEXT FlyEncGetResString(globalData._languageDLL, 4730)
�k Init=call TEXT FlyEncGetResString(globalData._languageDLL, 4729)
�l Init.call text 'Destination:  '||globalData._folder����d 0   � Help�� �,0�   �   �  ��          �   Z u 7 ,���          � @�  ! /����        �   �  %  -����        �   �>  %  .����Question     Yes Push ���,0$  �,Init�if globalData._osVersion >=40 then
	call Font '9.WarpSans'

if globalData._msgTitle\="" THEN
	call Text globalData._msgTitle
ELSE 
	call TEXT FlyEncGetResString(globalData._languageDLL, 4733)

pos=SysIni(globalData._inifile, "windowposition","drdmsgbox")
if pos\="ERROR:" THEN DO
	PARSE var pos x y dx dy
	call position x, y
END

call show�.Click�/* Save window position */
pos=msgBox.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdmsgbox", pos)

call msgBox.close

Return 'NO'Init~if globalData._msgType='YESNO' THEN
	call TEXT FlyEncGetResString(globalData._languageDLL, 4732) /* No */
ELSE
	call hide
�-Init�if globalData._msgType='YESNO' THEN
	call TEXT FlyEncGetResString(globalData._languageDLL, 4719) /* Yes */
ELSE
	call TEXT FlyEncGetResString(globalData._languageDLL, 4703) /* Ok */
Click�/* Save window position */
pos=msgBox.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdmsgbox", pos)

call msgBox.close

RETURN 'YES'
�/Initcall text globalData._msgText
�� ��0@   @   �  ��           ,   6 � � , ���< All popup menus    � ��0K   K   �      container_menu ,   �      Play with z!     ID3 info ����0A    containerpopup  playzcall playwithz  call modalFor id3Tag� ��0�  �  �  ��          �  - d � � ����        �� � 5 
 �����        � � z �  �����        � � k 5 
 �����        � � b �  �����           U � 
 �����        " � 8 � 
 �����        > � - � 
 �����        W�   T 
 �����         d �  �  �����        e �  &  �����        l  �2  &  �����        s  ��  &  �����        x  � R � E �����        �  �  � > �����Encode MP3 on the fly    Album: Album Artist: Artist (FreeDB data not available) Place MP3 files into folder Add MP3 files to library Destination:  Encode Cancel Help CD info Location for encoded files ����0�  ��Init�if globalData._osVersion >=40 then
	call Font '9.WarpSans'

call TEXT FlyEncGetResString(globalData._languageDLL, 4721)

pos=SysIni(globalData._inifile, "windowposition","drdontheflyencode")
if pos\="ERROR:" THEN DO
	call writeLog "Reading window position: "||pos
	PARSE var pos x y dx dy
	call position x, y, dx, dy
END
call writeLog ""

call d700.c714.RANGE 256

sel=SysIni(globalData._inifile, "ontheflyencode","destination")
if sel="1" | sel="ERROR:" THEN DO
	call d700.c706.select 1
	call d700.c714.text globalData._folder
END
ELSE DO
	call d700.c709.select 1
	call d700.c714.text globalData._mp3library||'\'||globaldata._artist||'\'||globaldata._album
END
call show

��Init=call TEXT FlyEncGetResString(globalData._languageDLL, 4724)
��Init;call TEXT FlyEncGetResString(globalData._languageDLL, 4725)��Init;call TEXT FlyEncGetResString(globalData._languageDLL, 4716)ClickEcall SysSetObjectData "<CWCREATOR_SETTINGS>", 'DISPLAYHELPPANEL=5720'��Click�/* Save window position */
pos=d700.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdontheflyencode", pos)
EXITInit=call TEXT FlyEncGetResString(globalData._languageDLL, 4704)
��Init;call TEXT FlyEncGetResString(globalData._languageDLL, 4722)Click�rc=Val('global.0',globalData.0)
DO a=1 to globalData.0
  rc=Val('global.'a, globalData.a)
  rc=Val('global.'a'_length', globalData.a._length)
  rc=Val('global.'a'._newName',globalData.a._newName)
  rc=Val('global.'a'_title', globalData.a._title)
  rc=Val('global.'a'_album', globalData.a._album)
  rc=Val('global.'a'_artist', globalData.a._artist)
END

/* Remove forbidden char in folder name (the '&' which confuses the shell when
   inside a command line) */
globalData._folder=TRANSLATE(d700.c714.text(), "+", "&")
/* Check if target folder exists */
ret=checkTargetFolder()
IF ret=0 THEN DO
	/* Folder does not exist and can't be created */
	globalData._msgType='OK'
	call ShowMsgBox 'The target folder cannot be found and cannot be created.', 'Setup problem','OK'
	SIGNAL RETURN
END

rc=Val('global._folder',globalData._folder)
rc=Val('global._MP3EncoderPath',globalData._MP3EncoderPath)
rc=Val('global._grabberPath',globalData._grabberPath)
rc=Val('global._grabberOptions',globalData._grabberOptions)
rc=Val('global._grabberLog',globalData._grabberLog)
rc=Val('global._encodeStatusHelper',globalData._encodeStatusHelper)
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

call main.open , 'Encode on the fly'

/* Start decode thread */
rc=start(encodeThread)
call disable
globalData._decoding='YES'

/* Save window position */
pos=d700.Position()
	rc=SysIni(globalData._inifile, "windowposition","drdontheflyencode", pos)
/* Save selection */
	rc=SysIni(globalData._inifile, "ontheflyencode","destination", d700.c706.select())

call d700.close��Initcall RANGE 256��Init;call TEXT FlyEncGetResString(globalData._languageDLL, 4723)��Init=call TEXT FlyEncGetResString(globalData._languageDLL, 4727)
ClickXcall d700.c714.text globalData._mp3library||'\'||d700.c705.text()||'\'||d700.c704.text()��Init=call TEXT FlyEncGetResString(globalData._languageDLL, 4726)
Click&call d700.c714.text globalData._folder��Initncall TEXT FlyEncGetResString(globalData._languageDLL, 4728)
IF globalData._haveCddbData="NO" THEN call SHOW
��ChangedwIF d700.c709.select()=1 THEN
	call d700.c714.text globalData._mp3library||'\'||d700.c705.text()||'\'||d700.c704.text()Initcall Text globaldata._artist��Init=call TEXT FlyEncGetResString(globalData._languageDLL, 4906)
��ChangedwIF d700.c709.select()=1 THEN
	call d700.c714.text globalData._mp3library||'\'||d700.c705.text()||'\'||d700.c704.text()Initcall Text globaldata._album��Init;call TEXT FlyEncGetResString(globalData._languageDLL, 4907)�