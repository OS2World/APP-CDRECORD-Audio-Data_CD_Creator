/* This Rexx-skript puts the tracknames into the tracks listbox of an audio-folder.
 * It uses the CDDB information fetched earlier by the CDDB helper.
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


parse arg fileInfo.diskid numTracks ' "'fileInfo.folder'" ' fileinfo.installdir

say ''
say ''
say 'This Rexx-skript puts the tracknames into the tracks listbox of an audio-folder.'
say 'Do not start this skript by hand. It is called by a helper with the right parameters.'
SAY ''

say 'ID: 'fileinfo.diskid
say 'Folder: 'fileinfo.folder
say 'installdir: 'fileinfo.installdir
say '# Tracks: 'numTracks

if numTracks < 1 then exit

cddbData=installdir||'\cddbdata\'||fileinfo.diskid

rc=checkCddbData()
if rc=0 then exit

SAY 'cddb data found, now processing...'


/* Seems we have all we need. Start processing the cddb data */

/* Read the cddb data into a stem */
rc=readCddbData()

call prepareCP

DO a=0 to numTracks-1
   /* Put cddb info into stem */
   fileinfo.title=searchCddbValue("TTITLE"||a)
 
   /*  fileinfo.realdiskid=searchCddbValue("DISCID=")
       fileinfo.disktitle=searchCddbValue("DTITLE=") */

  /* Set name of listbox item */
   setupstring='SETTRACKLBTEXT='||a+1||' '||convertCP(fileinfo.title)
   rc=SysSetObjectData(fileinfo.folder,setupstring);
END

exit

/*******************************************************************/
/* Procedures */
/*******************************************************************/



/* Search an entry in the cddb data file */
searchCddbValue: PROCEDURE expose cddb.

parse arg keyword

found=0
DO i=1 to cddb.0 while found=0
  if POS(keyword,cddb.i)=1 then found=i
END

if found<>0 then return SUBSTR(cddb.found,POS('=',cddb.found)+1)

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



checkCddbData: PROCEDURE expose fileinfo.

cddbData=fileinfo.installdir||'\cddbdata\'||fileinfo.diskid

fileinfo.cddbdatafile=cddbData
rc=STREAM(cddbData ,'c','query exists')
if rc\='' then return 1

fileinfo.cddbdatafile=''
return 0

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