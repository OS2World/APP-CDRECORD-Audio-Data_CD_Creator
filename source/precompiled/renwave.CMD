/* This Rexx script is called after a track was grabbed. It renames
    the wave using the cddb info fetched earlier by the cddb helper.
*/
/*
 * This file is (C) Chris Wohlgemuth 2000-2005
 *
 * It's part of the Audio/Data-CD-Creator distribution.
 *
 * Visit
 *
 * http://www.os2world.com/cdwriting
 * http://www.geocities.com/SiliconValley/Sector/5785/index.html
 *
 * for more information.
 */
/*
 * Copyright (c) Chris Wohlgemuth 2000-2005
 * All rights reserved.
 *
 * http://www.geocities.com/SiliconValley/Sector/5785/
 * http://www.os2world.com/cdwriting
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The authors name may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

parse arg fileInfo.diskid fileinfo.track ' "'fileInfo.wavefile'" ' fileinfo.installdir

say ''
say ''
say 'DI: !'fileinfo.diskid'!'
say 'Track: !'fileinfo.track'!'
say 'Wavefile: 'fileinfo.wavefile
say 'installdir: 'fileinfo.installdir

/* Waiting some seconds to let folder refresh itself and recalculate the
   time */
cddbData=installdir||'\cddbdata\'||fileinfo.diskid

rc=checkCddbData()
if rc=0 then signal renamedone

SAY 'cddb data vorhanden'

call prepareCP

fileinfo.wavefile=FindWavefile()
if fileinfo.wavefile='' then signal renamedone

SAY 'Wavefile found'

/* Seems we have all we need. Start processing the cddb data */

/* Read the cddb data into a stem */
rc=readCddbData()

/* Put cddb info into stem */
   if fileinfo.track<>1 THEN DO
    fileinfo.title=searchCddbValue("TTITLE"||STRIP(fileinfo.track-1,L,0))
   END
   else DO
      fileinfo.title=searchCddbValue("TTITLE0")
   END
   fileinfo.realdiskid=searchCddbValue("DISCID=")
   fileinfo.disktitle=searchCddbValue("DTITLE=")

   fileinfo._album=''
   fileinfo._artist=''
   thePos=LASTPOS('/',fileinfo.disktitle)
   IF thePos\=0 THEN DO
	fileinfo._album=STRIP(RIGHT(fileinfo.disktitle, LENGTH(fileinfo.disktitle)-thePos))
	fileinfo._artist=STRIP(LEFT(fileinfo.disktitle,thePos-1))
   END
   ELSE DO
	fileinfo._album=fileinfo.disktitle
   	fileinfo._artist=fileinfo.disktitle
   END
/* Write cddb data to .KEYPHRASES EA */
   MyStem1.0 = 5
   MyStem1.0.codepage = 0
   Mystem1.1 = "TRACK="||fileinfo.track
   Mystem1.2 = "DISCID="fileinfo.realdiskid
   if fileinfo.realdiskid<>fileinfo.diskid THEN MyStem1.3 = "FUZZYID="||fileinfo.diskid
   else MyStem1.3 = "FUZZYID="
   Mystem1.4 = "DTITLE="||fileinfo.disktitle
   if fileinfo.track<>1 THEN
   Mystem1.5 = "TTITLE"||STRIP(fileinfo.track-1,L,0)||"="||fileinfo.title
   else Mystem1.5 = "TTITLE0="||fileinfo.title

   call FileKeyphrases "SET", fileinfo.wavefile, "MyStem1"
   say "  Result of FileKeyPhrases is " || result


   call ReadEA

  /* Set instance data if CW-Multimedia classes are installed.

     Visit http://www.os2world.com/cdwriting for info

  */
   setupstring='CWAUDIOTRACKNAME='||fileinfo.title||';CWAUDIOARTIST='||fileinfo._artist||';CWAUDIOALBUM='||fileinfo._album||';'
   rc=SysSetObjectData(fileinfo.wavefile,setupstring);

  /* Rename wavefile */
   setupstring='TITLE='||fileinfo.track||' '||fileinfo.disktitle||' - '||fileinfo.title||'.wav'
   rc=SysSetObjectData(fileinfo.wavefile,setupstring);

renamedone:
  foldername=FILESPEC('drive',fileinfo.wavefile)||FILESPEC('path',fileinfo.wavefile)
  /* Remove trailing backslash */
  foldername=LEFT(foldername,LENGTH(foldername)-1)
  /* Refresh folder */
  SysSetObjectData(foldername, "MENUITEMSELECTED=503");

exit

/*******************************************************************/
/* Procedures */
/*******************************************************************/

 /*                                                                    */
 /* demo subroutine to read the EA                                     */
 /*                                                                    */
 ReadEA:
   say "Now reading ..."
 
 ReadEA1:
   call FileKeyphrases "GET", fileinfo.wavefile, "MyStem"
   say "  Result of FileKeyphrases is " || result
 
   say "  The history list for this file contains " || ,
       MyStem.0 || " entries."
   say "  The codepage of the history list is " || MyStem.0.CodePage || "."
 
   do i = 1 to MyStem.0
     say "  History list entry no " || i  || " is  "
     say "     <" || myStem.i || ">"
   end /* do i = 1 to MyStem.0 */
 RETURN



/* Search an entry in the cddb data file */
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

/* Clear var */
DROP cddb.
cddb.=""
cddb.0=0

/* Read the file */
DO i=cddb.0+1 while lines(fileinfo.cddbdatafile)<>0
   cddb.i=LINEIN(fileinfo.cddbdatafile)
END
cddb.0=i-1
return 0


/***********************************************************/

This procedure checks the wavefile. Depending on the grabber
there may be an extension or an added number. If so we korrekt
the given name.

/***********************************************************/
FindWavefile: PROCEDURE expose fileinfo.

filename=fileinfo.wavefile

if STREAM(filename ,'c','query exists') \=''
      then return filename /* The filename is correct */

if translate(RIGHT(filename,4))\='.WAV' then 
   DO
      /* No extension */
      if STREAM(filename||'.wav' ,'c','query exists') \=''
         then return filename||'.wav'
      /* Check for additional Leech track number */
      if STREAM(filename||'_'||fileinfo.track||'.wav' ,'c','query exists') \=''
         then return filename||'_'||fileinfo.track||'.wav'
      /* Check for Leech track numbering only */
      if STREAM(filename||'_'||fileinfo.track||'.wav' ,'c','query exists') \=''
         then return filename||'_'||fileinfo.track||'.wav'
   END
/* There's a .wav extension */
     /* Add tracknumber */
     if STREAM(LEFT(filename,POS('.WAV',TRANSLATE(filename))-1)||fileinfo.track||'.wav' ,'c','query exists') \=''
         then return LEFT(filename,POS('.WAV',TRANSLATE(filename))-1)||fileinfo.track||'.wav'
     /* Add Leech track number */
     if STREAM(LEFT(filename,POS('.WAV',TRANSLATE(filename))-1)||'_'||fileinfo.track||'.wav' ,'c','query exists') \=''
         then return LEFT(filename,POS('.WAV',TRANSLATE(filename))-1)||'_'||fileinfo.track||'.wav'

/* True filename not found */
return ''



checkCddbData: PROCEDURE expose fileinfo.

cddbData=fileinfo.installdir||'\cddbdata\'||fileinfo.diskid

fileinfo.cddbdatafile=cddbData
rc=STREAM(cddbData ,'c','query exists')
if rc\='' then return 1

fileinfo.cddbdatafile=''
return 0

 /* ------------------------------------------------------------------ */
 /* function: Get, Set or Clear the .KEYPHRASES EA of a file              */
 /*                                                                    */
 /* call:     FileHistory GET, filename, NewHistoryStem                */
 /*           FileHistory ADD, filename, newHistoryEntry {,newStem}    */
 /*           FileHistory SET, filename, CurHistoryStem                */
 /*           FileHistory CLEAR, filename                              */
 /*                                                                    */
 /* where:    GET, ADD, SET, CLEAR                                     */
 /*             - action:                                              */
 /*                GET - get a list of the current entries             */
 /*                ADD - add an entry to the list                      */
 /*                SET - replace the EA with a new list                */
 /*              CLEAR - clear the whole list                          */
 /*           filename                                                 */
 /*             - name of the file                                     */
 /*           NewHistoryStem                                           */
 /*             - stem for the history list entries                    */
 /*           newStem                                                  */
 /*             - stem for the history list entries                    */
 /*           CurHistoryStem                                           */
 /*             - stem _with_ the history list entries                 */
 /*           newHistoryEntry                                          */
 /*             - new entry for the history list                       */
 /*               (ASCII string)                                       */
 /*                                                                    */
 /* returns:  0 - okay                                                 */
 /*           1 - file not found                                       */
 /*           2 - EA is invalid                                        */
 /*           3 - CurHistoryStem.0 is invalid                          */
 /*           4 - CurHistoryStem.0.codepage is invalid                 */
 /*          -1 - invalid parameter                                    */
 /*        else - unexpected error                                     */
 /*                                                                    */
 /* notes:                                                             */
 /*  Do not add the trailing dot to the stem name!                     */
 /*  Format of the stems:                                              */
 /*    history_stem.0 = number of entries                              */
 /*    history_stem.0.codepage = codepage of the EA                    */
 /*                              (def.: 0, use default codepage)       */
 /*    history_stem.n = entry n                                        */
 /*                                                                    */
 /*  The format of the .HISTORY EA is:                                 */
 /*                                                                    */
 /*     EA Type  Code                                                  */
 /*              page Count                                            */
 /*   +--------------------------------------------------+             */
 /*   | EAT_MVMT 0000 0002                               |             */
 /*   |          EAT_ASCII 0017  Joe    Created  2/10/88 |             */
 /*   |          EAT_ASCII 0017  Harry  Changed  2/11/88 |             */
 /*   +--------------------------------------------------+             */
 /*              EA Type  length contents (ASCII string)               */
 /*                                                                    */
 /*  All numeric values are WORDs in INTEL format.                     */
 /*                                                                    */
 /* (see also Extended Attribute Data Types and EAs used by the WPS)   */
 /*                                                                    */
 /*  FileHistory uses the prefix 'FH.' for all local variables. The    */
 /*  local variables are dropped at the end of the routine!            */
 /*                                                                    */
 /* (c) 1996 Bernd Schemmer, Germany, EMail: 100104.613@compuserve.com */
 /*                                                                    */
 FileKeyphrases:
 
                     /* name of the EA to use                          */
                     /* note: change this variable to use the routine  */
                     /*       for the EAs .COMMENTS or .KEYPHRASES.    */
                     /*       In this case you must also delete the    */
                     /*       Codepage related code in this routine.   */
  /* FH.__EAName = '.HISTORY'*/
   FH.__EAName = '.KEYPHRASES'
 
                     /* init the return code                           */
   rc = 0
 
 /* -------------------------- */
                     /* install local error handlers                   */
   SIGNAL ON SYNTAX  NAME FileHistoryEnd
   SIGNAL ON ERROR   NAME FileHistoryEnd
   SIGNAL ON FAILURE NAME FileHistoryEnd
 /* -------------------------- */
                     /* get the parameter                              */
   parse upper arg FH.__action , FH.__file , FH.__variable , .
 
                     /* get the parameter for the ADD action           */
   parse arg  , , FH.__newValue , FH.__tempStem
 
                     /* check the parameter                            */
   select
 
                     /* check the action parameter                     */
     when wordPos( FH.__action, 'GET ADD SET CLEAR' ) = 0 then
       rc = -1
 
                     /* check the parameter for the stem variable      */
     when wordPos( FH.__action, 'GET ADD SET' ) <> 0 & ,
          FH.__variable = '' then
       rc = -1
 
                     /* check the parameter for the filename           */
     when FH.__file = '' then
       rc = -1
 
                     /* test, if the file exists                       */
     when stream( FH.__file, 'c', 'QUERY EXISTS' ) = '' then
       rc = 1
 
                     /* check the number fields in the stem            */
     when FH.__action = 'SET' then
     do
       select
 
                     /* stem.0 must contain the number of entries      */
         when datatype( value( FH.__variable || '.0' ) ) <> 'NUM' then
           rc = 3
                     /* use the default codepage if the entry          */
                     /* stem.0.codepage is missing                     */
         when symbol( FH.__variable || '.0.CodePage' ) <> 'VAR' then
           call value FH.__variable || '.0.CodePage', 0
 
                     /* stem.0.codepage must be a numeric value if     */
                     /* it exist                                       */
         when datatype( value( FH.__variable || '.0' ) ) <> 'NUM' then
           rc = 4
 
         otherwise
           nop
       end /* select */
 
     end /* when */
 
     when FH.__action = 'ADD' then
     do
                     /* use the fourth parameter as name of the stem   */
                     /* if entered                                     */
       if FH.__tempStem <> '' then
         FH.__variable = FH.__tempStem
       else
         FH.__variable = 'FH.__tempStem'
     end /* when */
 
     otherwise
       nop
 
   end /* select */
 
 /* -------------------------- */
 
   if rc = 0 then
   do
                     /* load the necessary REXXUTIL functions          */
 
                     /* use special REXX names to avoid errors if      */
                     /* another program drops the REXXUTIL functions   */
     call rxFuncAdd 'FH_SysGetEA', 'REXXUTIL', 'SysGetEA'
     call rxFuncAdd 'FH_SysPutEA', 'REXXUTIL', 'SysPutEA'
 
 /* -------------------------- */
                     /* constants for the EA type specifier            */
     FH.__EAT_BINARY       = SwapWord( 'FFFE'x )
     FH.__EAT_ASCII        = SwapWord( 'FFFD'x )
     FH.__EAT_BITMAP       = SwapWord( 'FFFB'x )
     FH.__EAT_METAFILE     = SwapWord( 'FFFA'x )
     FH.__EAT_ICON         = SwapWord( 'FFF9'x )
     FH.__EAT_EA           = SwapWord( 'FFEE'x )
     FH.__EAT_MVMT         = SwapWord( 'FFDF'x )
     FH.__EAT_MVST         = SwapWord( 'FFDE'x )
     FH.__EAT_ANS1         = SwapWord( 'FFDD'x )
 
 /* -------------------------- */
 
     if FH.__action = 'CLEAR' then
     do
                     /* clear the history list                         */
                                                              /* v2.80 */
       call FH_SysPutEA FH.__file, FH.__EAName, ''
 
     end /* if FH.__action = 'CLEAR' then */
 
 /* -------------------------- */
 
     if wordPos( FH.__action, 'GET ADD' ) <> 0 then
     do
                     /* read the EA                                    */
 
                     /* init the stem for the EA values                */
       call value FH.__variable || '.', ''
       call value FH.__variable || '.0' , 0
       call value FH.__variable || '.0.codepage', 0
 
                     /* read the EA                                    */
       rc = FH_SysGetEA( FH.__file, FH.__EAName, FH.__historyEA )
       if rc = 0 & FH.__historyEA <> '' then
       do
                     /* split the EA into the header fields and the    */
                     /* values                                         */
         parse var FH.__historyEA FH.__historyEAType +2 ,
                                  FH.__historyEACodePage +2,
                                  FH.__historyEACount +2 ,
                                  FH.__historyEAValues
 
                     /* convert the count value to decimal             */
         FH.__historyEACount = c2d( SwapWord( FH.__HistoryEACount ) )
 
                     /* check the EA type                              */
         if FH.__historyEAType = FH.__EAT_MVMT then
         do
                     /* save the codepage                              */
           call value FH.__variable || '.0.codepage' ,,
                      c2d( SwapWord( FH.__historyEACodePage ) )
 
                     /* split the value into separate fields           */
           do FH.__i = 1 to FH.__HistoryEACount while rc = 0
 
             FH.__HistoryEACurType = substr( FH.__HistoryEAValues, 1, 2 )
             if FH.__HistoryEACurType <> FH.__EAT_ASCII then
               rc = 2    /* invalid EA type                            */
             else
             do
                     /* get the length of this value                   */
               FH.__HistoryEACurLen  = c2d( SwapWord( substr( FH.__HistoryEAValues, 3, 2 ) ) )
 
               parse var FH.__historyEAValues 5 FH.__HistoryEACurVal,
                                              +( FH.__HistoryEACurLen) ,
                                              FH.__historyEAValues
 
                     /* save the value into the stem                   */
               call value FH.__variable || '.' || FH.__i ,,
                          FH.__HistoryEACurVal
 
             end /* else */
           end /* do FH.__i = 1 to c2d( FH.__HistoryEACount ) while rc = 0 */
 
                     /* save the number of entries in stem.0           */
           if rc = 0 then
             call value FH.__variable || '.0' , FH.__i-1
 
         end /* if FH.__historyEAType = FH.__EAT_MVST then */
         else
           rc = 2    /* invalid EA type                                */
 
       end /* if rc = 0 then */
 
     end /* if wordPos( FH.__action, 'GET ADD' ) <> 0 then */
 
 /* -------------------------- */
 
     if FH.__action = 'ADD' & rc = 0 then
     do
                     /* add an entry                                   */
 
       FH.__i = value( FH.__variable || '.0' ) +1
       call value FH.__variable || '.' || FH.__i , FH.__newValue
       call value FH.__variable || '.0' , FH.__i
 
     end /* if FH.__action = 'ADD' & rc = 0 then */
 
 /* -------------------------- */
 
     if wordPos( FH.__action, 'SET ADD' ) <> 0 & rc = 0 then
     do
                     /* write the EA                                   */
 
       FH.__newEA = FH.__EAT_MVMT || ,
          SwapWord( right( '00'x || d2c( value( FH.__variable || '.0.codepage' ) ), 2 ) ) || ,
          SwapWord( right( '00'x || d2c( value( FH.__variable || '.0' ) ), 2 ) )
 
       do FH.__i = 1 to value( FH.__variable || '.0' )
         FH.__curEntry = value( FH.__variable || '.' || FH.__i )
 
         FH.__newEA = FH.__newEA || ,
                      FH.__EAT_ASCII || ,
                      SwapWord( right( '00'x || d2c( length( FH.__curEntry ) ), 2 ) ) || ,
                      FH.__curEntry
       end /* do FH.__i = 1 to value( FH.__variable || '.0' ) */
 
                                                              /* v2.80 */
       call FH_SysPutEA FH.__file, FH.__EAName, FH.__newEA
       rc = result
 
     end /* if wordPos( FH.__action, 'SET ADD' ) <> 0 then */
 
   end /* if rc = 0 then */
 
                     /* label for the local error handler              */
 FileHistoryEnd:
 
                     /* drop the REXXUTIL functions                    */
                     /* (possible and necessary because we use unique  */
                     /*  REXX names!)                                  */
     call rxFuncDrop 'FH_SysGetEA'
     call rxFuncDrop 'FH_SysPutEA'
 
                     /* drop local variables                           */
   drop FH.
 
 RETURN rc
 
 /* ------------------------------------------------------------------ */
 /* function: Convert a hexadecimal WORD from LSB format to MSB format */
 /*           and vice versa                                           */
 /*                                                                    */
 /* call:     SwapWord hexadecimal_word                                */
 /*                                                                    */
 /* where:    hexadecimal_word - input as hexadecimal word             */
 /*                                                                    */
 /* output:   value in MSB format as hexadecimal word                  */
 /*                                                                    */
 SwapWord: PROCEDURE
   RETURN strip( translate( "12", arg(1), "21" ) )
 
 
   
prepareCP:
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

convertCP: procedure expose str1 str2 
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


