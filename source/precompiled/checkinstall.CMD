/* Check installation of CD-Creator */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

/* Build install dir */
instDir=DIRECTORY()
instDir=FILESPEC('drive',instDir)||FILESPEC('path',instDir)
/* Build class DLL */
classDLL=instDir||'AUCDFLDR.DLL'

call buildFileList

objectlist.1._id='<CD_WRITING>'
objectlist.1._desc='CD writing folder'
objectlist.2._id='<CDR_TOOLS>'
objectlist.2._desc='CDR-Tools'
objectlist.3._id='<CWCREATOR_SETTINGS>'
objectlist.3._desc='CD-Creator Settings'
objectlist.4._id='<CD_COPY>'
objectlist.4._desc='1:1 CD-Copy'
objectlist.5._id='<AUDIOCD_CREATOR>'
objectlist.5._desc='Create Audio-CD template'
objectlist.6._id='<DATACD_CREATOR>'
objectlist.6._desc='Create Data-CD template'
objectlist.7._id='<CREATOR_CONFIG>'
objectlist.7._desc='Configuration and (Un)Installation folder'
objectlist.8._id='<CREATOR_RECREATEOBJ>'
objectlist.8._desc='Recreate WPS-Objects'
objectlist.9._id='<CWDECODE_MP3>'
objectlist.9._desc='Decode MP3 (in the bin-dir)'
objectlist.10._id='<CWINFO_MP3>'
objectlist.10._desc='MP3-file Info (in the bin-dir)'
objectlist.11._id='<CWCREATOR_TUTORIAL>'
objectlist.11._desc='CD writing tutorial'
objectlist.12._id='<ACD_SHADOW>'
objectlist.12._desc='Create Audio-CD, shadow of template'
objectlist.13._id='<DCD_SHADOW>'
objectlist.13._desc='Create Data-CD, shadow of template'
objectlist.14._id='<CWCREATOR_DOCS>'
objectlist.14._desc='Shadow of Docs folder'
objectlist.15._id=''
objectlist.15._desc=''

objectlist.0=14

/* The classes */
wpsclasslist.1._name='CWAudioFolder'
wpsclasslist.1._dll=classDLL
wpsclasslist.2._name='CWDataFolder'
wpsclasslist.2._dll=classDLL
wpsclasslist.3._name='CWCreatorSettings'
wpsclasslist.3._dll=classDLL
wpsclasslist.4._name='CWProgFolder'
wpsclasslist.4._dll=''

wpsclasslist.0=4

/* Define colors */
RED=31

'@ANSI ON'
SAY "1B"x||"[3h"

call generalInfo

SAY ''
'@PAUSE'

/* Check the most important objects */
SAY ''
call checkObjects

SAY ''
'@PAUSE'

SAY ''
call checkclasses

SAY ''
'@PAUSE'

EXIT

/*******************************/
generalInfo:

call invertcolor
SAY 'General installation information'
call normalColor
SAY ''

SAY 'Installation directory: 'instDir
SAY 'Class DLL: 'classDLL
SAY ''
SAY 'Make sure, this is correct' 
RETURN

/*******************************/
checkObjects:
call invertcolor
SAY 'Checking if objects are present.'
call normalColor
SAY '(This will NOT check the parameters!)'
SAY ''

/* Check objects */
DO i=1 to objectlist.0
	IF SysSaveObject(objectlist.i._id, 'TRUE')=0 THEN DO
		/* Object not existant */
		call CHAROUT ,objectlist.i._desc||' ('||objectlist.i._id||'): '
		call Color RED
		SAY 'Probably lost'
		call normalColor 
	END
	ELSE DO
		SAY objectlist.i._desc||' ('||objectlist.i._id||'): OK' 
	END
END
return

/*******************************/
checkClasses:
call invertcolor
SAY 'Checking classes'
call normalcolor
SAY ''

chkwpsclasslist.=''
chkwpsclasslist.0=0

/* Check classes */
IF wpsclasslist.0 \= 0 THEN DO
	CALL SysQueryClassList("liste.")
	DO i=1 to wpsclasslist.0
		chkwpsclasslist.i._name=wpsclasslist.i._name
		chkwpsclasslist.i._dll=''
		chkwpsclasslist.i._reg='NO'
		chkwpsclasslist.0=i
		DO b=1 to liste.0
		   if WORD(liste.b , 1)=chkwpsclasslist.i._name THEN DO
			chkwpsclasslist.i._dllreg=SUBWORD(liste.b , 2)
			chkwpsclasslist.i._reg='YES'
			iterate i
		   END 
		END
	END
/* progfolder needs special handling */
	i=wpsclasslist.0
	wpsclasslist.i._dll=chkwpsclasslist.i._dllreg
	
DROP liste.
DO b=1 to wpsclasslist.0
  IF chkwpsclasslist.b._reg='YES' THEN DO
	IF TRANSLATE(chkwpsclasslist.b._dllreg)=TRANSLATE(wpsclasslist.b._dll) THEN 
		SAY wpsclasslist.b._name||' registered OK using 'chkwpsclasslist.b._dllreg
	ELSE DO
		SAY 'Class '||wpsclasslist.b._name||' registered using 'chkwpsclasslist.b._dllreg
		CALL CHAROUT ,wpsclasslist.b._name||': '
		CALL Color RED
		SAY 'It must be '||wpsclasslist.b._dll||'!!!'
		call normalColor
		call reRegisterClass b
	END
  END
  ELSE DO
	call CHAROUT ,'Class '||wpsclasslist.b._name||' '
	call color RED
	SAY 'NOT registered!'
	call normalColor
	call reRegisterClass b
  END
END

DROP chkwpsclasslist.
RETURN

/**************** Ansi color handling ************************************/
invertColor:
call CHAROUT ,"1B"x||'[7m'
RETURN

normalColor:
Call CHAROUT ,"1B"x||'[0m'
RETURN

Color: PROCEDURE
clr=ARG(1)

CALL CHAROUT ,"1B"x||'[1m'||"1B"x||'['||clr||'m'
RETURN


buildFileList:

RETURN

reRegisterClass:

idx=ARG(1)

SAY ''
SAY 'Do you want to register class '||wpsclasslist.idx._name||' using '||wpsclasslist.idx._dll||'?'
SAY ''
say '(y/n)'
say '>'
parse value SysCurPos() with aktzeile aktspalte
newposition=SysCurPos(aktzeile-1,2)
pull chr
IF chr='Y' THEN DO
if SysRegisterObjectClass(wpsclasslist.idx._name, wpsclasslist.idx._dll)<>1 then
	do	 	
                say ''
		say 'Error while registering class '||wpsclasslist.idx._name
                say ''
	end 
	ELSE DO
                say ''
		SAY 'Class '||wpsclasslist.idx._name||' registered'
		SAY ''
	END
END
return
