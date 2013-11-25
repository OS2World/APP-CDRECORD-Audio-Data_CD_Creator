/*
 * This file is (C) Chris Wohlgemuth 1999-2004
 * 
 * It's part of the Audio/Data-CD-Creator distribution
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define INCL_DOSERRORS
#define INCL_DOSQUEUES
#include "audiofolder.hh"
#include "audiofolderhelp.h"
#include <stdio.h>
#include <stdlib.h>
#include <wpshadow.hh>
#include <string.h>

#include "cddb.h"

extern PVOID pvAudioSharedMem;
extern HMODULE hAudioResource;

extern GLOBALDATA globalData;

extern ATOM atomUpdateStatusbar;

extern char chrInstallDir[];
extern char chrLanguageDLL[];

extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern char chrAudioCDROptions[CCHMAXPATH];
extern char chrDataCDROptions[CCHMAXPATH];
extern LONG lCDROptions;
extern int iBus;
extern int iTarget;
extern int iLun;
extern int iSpeed;
extern int iFifo;

extern char chrCdrdaoPath[CCHMAXPATH];
extern char chrCdrdaoDriver[100];
extern char chrCdrdaoOptions[CCHMAXPATH];
extern int iMp3Decoder; /* Which decoder to use */

extern char chosenCD[];
//extern BOOL bMpg123SwabBytes;

/* MP3 encoder settings */
extern char chrMP3EncoderPath[CCHMAXPATH];
extern char chrMP3EncoderOptions[CCHMAXPATH];
extern ULONG ulMP3Bitrate;
extern ULONG ulMP3Quality;
extern ULONG ulMP3EncoderPriority;

extern char g_cChosenMP3Name[NUM_MP3NAMEPARTS];/* Array holding the indexes into the names array for each MP3 name part */
extern char g_chrMP3NameFillStrings[NUM_MP3NAME_FILLSTRINGS][MP3NAME_FILLSTRING_LEN];

extern char chrDVDDao[CCHMAXPATH];

extern HMTX hmtxFileName;

/* Extension counter for temp files */
int iExt=0;

//int iExt=0;

typedef struct
{
  USHORT cb;
  CWAudioFolder* thisPtr;
}WCNTRLDATA;

void SysWriteToTrapLog(const char* chrFormat, ...);

void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
ULONG launchWrapper(PSZ parameter, PSZ folderPath,HWND hwnd,PSZ wrapperExe, PSZ pszTitle);
PSZ buildAudioWriteParam(CWAudioFolder* thisPtr, PSZ trackname, int iSpeedLocal);
void changeBackslash(char* text);
ULONG cwRequestMutex(HMTX  hmtxBMP, ULONG ulTimeOut);
ULONG cwReleaseMutex(HMTX  hmtxBMP);
SOMClass* queryMMAudioClass(void);
SOMClass* queryCWAudioClass(void);
void writeLog(char* logFile, char* logText);
SOMClass* cwGetSomClass(char* chrClassName);
LONG CDQueryTrackSize(ULONG numTrack, char * drive);
LONG CDDBDiscID(char * drive, CDDBINFO * cddbInfo);

/* Local */
HMODULE queryModuleHandle(void);

void CWAudioFolder::cwWriteAudioTracks(HWND hwnd, char* trackName, char* folderName)
{
  return;
}

/******************************************************/
/* This function checks if the object is a wave file. */
/* It only checks the extension because cdrecord      */
/* doesn't know types and needs the extension to      */
/* distinguish between wavefiles and raw data files.  */
/*                                                    */
/* If bWithoutExt==TRUE then allow wave files withour */
/* extension. This checks for the class.              */
/******************************************************/
BOOL CWAudioFolder::cwIsWaveFile(WPObject* wpObject, BOOL bWithoutExt)
{
  char objectName[CCHMAXPATH];
  ULONG ulNameSize;
  char* chrExtension;

  if(!somIsObj(wpObject))
    return FALSE;

  if(!somResolveByName(wpObject,"wpQueryRealName"))
    /* It's not a file system object */
    return FALSE;

  /* Check if it's folder */
  if(somResolveByName(wpObject,"wpQueryContent"))
    /* It's a folder */
    return FALSE;

  /* Check the extension */
  ulNameSize=sizeof(objectName);
  ((WPFileSystem*)wpObject)->wpQueryRealName(objectName,&ulNameSize,FALSE);
  chrExtension=strrchr(objectName,'.');
  if(!chrExtension && !bWithoutExt)
    return FALSE; /* No extension */

  if(chrExtension) {
    chrExtension=strstr(strlwr(chrExtension),".wav");
    if(chrExtension)
      if(*(chrExtension+4)==0)
        return TRUE;//It's a wavefile
  }

  if(wpObject->somIsA(cwGetSomClass("MMWAV")))
    return TRUE;

  return FALSE;
}

/******************************************************/
/* This function checks if the object is a mp3 file.  */
/* It only checks the extension.                      */
/******************************************************/
BOOL CWAudioFolder::cwIsMp3File(WPObject* wpObject)
{
  char objectName[CCHMAXPATH];
  ULONG ulNameSize;
  char* chrExtension;

  if(!somIsObj(wpObject))
    return FALSE;

  if(!somResolveByName(wpObject,"wpQueryRealName"))
    /* It's not a file system object */
    return FALSE;

  /* Check if it's folder */
  if(somResolveByName(wpObject,"wpQueryContent"))
    /* It's a folder */
    return FALSE;

  /* Check the extension */
  ulNameSize=sizeof(objectName);
  ((WPFileSystem*)wpObject)->wpQueryRealName(objectName,&ulNameSize,FALSE); /* Get real name */
  chrExtension=strrchr(objectName,'.');
  if(!chrExtension)
    return FALSE; /* No extension */

  chrExtension=strstr(strlwr(chrExtension),".mp3");
  if(chrExtension)
    if(*(chrExtension+4)==0)return TRUE;//It's a mp3file
  
  return FALSE;
}


/******************************************************/
/* This function checks if the object is an audio     */
/* file. (Instance of MMAudio or CWAudio)                        */
/******************************************************/
BOOL CWAudioFolder::cwIsAudioFile(WPObject* wpObject)
{
  SOMClass *clsAudio;
  SOMClass *clsCWAudio;

  if(!somIsObj(wpObject))
    return FALSE;

  if(!somResolveByName(wpObject,"wpQueryRealName"))
    /* It's not a file system object */
    return FALSE;

  /* Check if it's folder */
  if(somResolveByName(wpObject,"wpQueryContent"))
    /* It's a folder */
    return FALSE;

  /* Check the class */
  clsAudio=queryMMAudioClass();
  clsCWAudio=queryCWAudioClass();
  if(!somIsObj(clsAudio) && !somIsObj(clsCWAudio))
    return FALSE;

  if(wpObject->somIsA(clsAudio))
    return TRUE;
  if(wpObject->somIsA(clsCWAudio))
    return TRUE;

  return FALSE;
}

void CWAudioFolder::cwSetStatusTime(ULONG ulTrackSize)
{
  char text[200];
  ULONG ulSize;
    
  ulSize=ulTrackSize;
  ulSize/=(2*2*44100);/* Calculate seconds */
  sprintf(text,"Total time: %d min %02d sec",ulSize/60 , (ulSize % 60));
  WinSetWindowText(WinWindowFromID(hwndStatusFrameCtl,IDST_STATUSTOTALTIME),text);
  /* Set percent bar. The value is calculated for 80 min CD-Rs */
  sprintf(text,"%d", (ulSize*1000)/(80*60)); /* Used capacity in % multiplied with 10 */
  //SysWriteToTrapLog("%s, %d\n", text, ulSize);
  WinSetWindowText(WinWindowFromID(hwndStatusFrameCtl,IDSL_STATUSPERCENTUSED),text);
}

ULONG CWAudioFolder::cwQueryWriteFlags()
{
  return ulWriteFlags;
}

ULONG CWAudioFolder::cwSetWriteFlags(ULONG ulNewWriteFlags,ULONG ulMask)
{
  ULONG rc;

  rc=cwQueryWriteFlags();
  ulWriteFlags=((ulNewWriteFlags&ulMask) | (~ulMask & ulWriteFlags));

  return rc;/* return previous flags */
}

void CWAudioFolder::cwEnableGrab(BOOL bEnable)
{
  bGrabEnabled=bEnable;
}

BOOL CWAudioFolder::cwQueryGrabEnabled(void)
{   
  return bGrabEnabled;    
}

void CWAudioFolder::cwEnableWrite(BOOL bEnable)
{
  bWriteEnabled=bEnable;
}

BOOL CWAudioFolder::cwQueryWriteEnabled(void)
{   
  return bWriteEnabled;    
}

void CWAudioFolder::cwEnableGrabControls(BOOL bEnable)
{
  if(!hwndGrab)
    return;
  WinEnableWindow(WinWindowFromID(hwndGrab,IDPB_SELECTALL) ,bEnable);
  WinEnableWindow(WinWindowFromID(hwndGrab,IDLB_GRABTRACKS) ,bEnable);
  WinEnableWindow(WinWindowFromID(hwndGrab,IDPB_GRAB) ,bEnable);
  WinEnableWindow(WinWindowFromID(hwndGrab,IDPB_REFRESH) ,bEnable);
    
}

void CWAudioFolder::cwEnableSelectControls(BOOL bEnable)
{
  if(!hwndSelect)
    return;
  WinEnableWindow(WinWindowFromID(hwndSelect,IDRB_WRITE) ,bEnable);
  WinEnableWindow(WinWindowFromID(hwndSelect,IDRB_GRAB) ,bEnable);
}

void CWAudioFolder::cwForceStatusUpdate(void)
{
  if(atomUpdateStatusbar)
     WinPostMsg(hwndStatusFrameCtl,atomUpdateStatusbar,0,MPFROMP(this));     
}

void CWAudioFolder::cwEnableWriteControls(BOOL bEnable)
{
  if(!hwndWrite)
    return;

  WinEnableWindow(WinWindowFromID(hwndWrite,IDRB_AUDIOWRITEMODEDAO) ,bEnable);
  WinEnableWindow(WinWindowFromID(hwndWrite,IDRB_AUDIOWRITEMODETAO) ,bEnable);
  if(!(cwQueryWriteFlags() & IDWF_DAO))
    WinEnableWindow(WinWindowFromID(hwndWrite,IDCB_NOFIX) ,bEnable);
  WinEnableWindow(WinWindowFromID(hwndWrite,IDCB_DUMMY) ,bEnable);
  WinEnableWindow(WinWindowFromID(hwndWrite,IDPB_BURN) ,bEnable);
  //  WinEnableWindow(WinWindowFromID(hwndWrite,IDCB_DAO) ,bEnable);
  //  WinEnableWindow(WinWindowFromID(hwndWrite,IDCB_PREEMP) ,bEnable);
}

void CWAudioFolder::cwSetStatusText(char * text)
{ 
  WinSetWindowText(WinWindowFromID(hwndStatusFrameCtl,IDST_STATUSTOTALTIME),text);    
}

/*********************************************************************/
/*                                                                   */
/* This procedure fills the given listbox with all the tracks of the */
/* given folder. If no listbox is given only the size of all waves   */
/* is returned. Mp3 are currently ignored while calculating!         */
/*                                                                   */
/* Parameters:                                                       */
/*              hwndFrame:   frame handle of wpFolder window         */
/*              hwndListBox: handle of listbox where to insert the   */
/*                           wavefile names                          */
/*                                                                   */
/* Returns: Size of all wave tracks in byte                          */
/*                                                                   */
/*********************************************************************/
ULONG CWAudioFolder::cwFillTrackList(HWND hwndFrame, HWND hwndListBox)
{
  HWND hwndCnr;
  PMINIRECORDCORE mrc;   
  WPObject * contentObject;
  char name[CCHMAXPATH];
  ULONG ulNameSize;
  ULONG ulSize;
  BOOL bIsMp3;

  ulSize=0;
  /* Get hwnd of folder container */ 
  hwndCnr=WinWindowFromID(hwndFrame,FID_CLIENT);
  if(hwndCnr){ /* Catch error */
    /* Get first container item of our folder */
    mrc=(PMINIRECORDCORE)WinSendMsg(hwndCnr,CM_QUERYRECORD,NULL,
                                    MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
    if(mrc){ 
      while(mrc) {
        /* Get wps-object-ptr. from container item */
        contentObject=(WPObject*)OBJECT_FROM_PREC(mrc);//Get object
        if(somIsObj(contentObject))
          /* Get file system object or NULL */
          contentObject=cwGetFileSystemObject(contentObject);
        if(somIsObj(contentObject)){
          /* It's a file system object */
          /* Check, if it's a wave file or a mp3file */
          bIsMp3=cwIsMp3File(contentObject);
          //  if(cwIsWaveFile(contentObject)|| (bIsMp3 && strlen(chrMpg123Path) && !(ulWriteFlags & IDWF_DAO))){
          if(cwIsWaveFile(contentObject, FALSE)|| (bIsMp3 && strlen(globalData.chrMpg123Path))  || 
             (cwIsAudioFile(contentObject) && iMp3Decoder==IDKEY_USEMMIOMP3) ){
            /* Yes, query the full path */
            ulNameSize=sizeof(name);
            ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
            /* Add tracksize of wavefile to totalsize */ 
            if(!bIsMp3)          
              ulSize+=((WPFileSystem*)contentObject)->wpQueryFileSize()-44;
            else {
              /* Add playtime of mp3file to totalsize */ 
            }
            /* Add 2 seconds for pause between tracks */
            //ulSize+=8*44100;
            /* insert the name into the listbox */ 
            if(hwndListBox)
              WinInsertLboxItem(hwndListBox,LIT_END,
                                name);
          }           
        }/* end of if(contentObject) */           
        /* Get next container item */
        mrc=(PMINIRECORDCORE)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(mrc),
                                        MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
      }// end of while(mrc)
    }// end of if(mrc)
  }
  return ulSize;  
}

/* This thread waits 120 sec and removes the contents file from the temp dir */
void _Optlink _contentsFileDeleteThread (void *arg)
{
  char * memPtr;

  memPtr=(char*)arg;

  /* Wait 120 sec */
  DosSleep(120000);
  remove(memPtr);
  free(arg);
}


void addSettingsToFile(FILE *fHandle)
{
  /* Put config data into file */
  fprintf(fHandle,"[CONFIG]\n");
  fprintf(fHandle,"[INSTALLDIR]%s\n", chrInstallDir);
  fprintf(fHandle,"[INIFILE]%s\\cdrecord.ini\n", chrInstallDir);
  fprintf(fHandle,"[FIFO]%d\n",iFifo);
  fprintf(fHandle,"[SPEED]%d\n",iSpeed);
  fprintf(fHandle,"[DEVICE]%d,%d,%d\n",iBus, iTarget, iLun);
  if(lCDROptions & IDCDR_BURNPROOF)
    fprintf(fHandle,"[BURNSAVE]YES\n");
  else
    fprintf(fHandle,"[BURNSAVE]NO\n");
  if(lCDROptions & IDCDR_SONYMULTISESSION)
    fprintf(fHandle,"[MULTISESSIONFIX]YES\n");
  else
    fprintf(fHandle,"[MULTISESSIONFIX]NO\n");

  /* Cdrdao settings */
  fprintf(fHandle,"[CDRDAOPATH]%s\n", chrCdrdaoPath);
  fprintf(fHandle,"[CDRDAODRIVER]%s\n", chrCdrdaoDriver);
  fprintf(fHandle,"[CDRDAOOPTIONS]%s\n", chrCdrdaoOptions);
  fprintf(fHandle,"[CDRDAOPATH2]%s\n", globalData.chrCdrdaoPath2);
  fprintf(fHandle,"[CDRDAODRIVER2]%s\n", globalData.chrCdrdaoDriver2);
  fprintf(fHandle,"[CDRDAOOPTIONS2]%s\n", globalData.chrCdrdaoOptions2);
  fprintf(fHandle,"[CDRDAODEVICE2]%s\n", globalData.chrDeviceName2);
  fprintf(fHandle,"[CDRDAOPATH3]%s\n", globalData.chrCdrdaoPath3);
  fprintf(fHandle,"[CDRDAODRIVER3]%s\n", globalData.chrCdrdaoDriver3);
  fprintf(fHandle,"[CDRDAOOPTIONS3]%s\n", globalData.chrCdrdaoOptions3);
  fprintf(fHandle,"[CDRDAODEVICE3]%s\n", globalData.chrDeviceName3);

  /* Cdrecord */
  fprintf(fHandle,"[CDRECORDPATH]%s\n", chrCDRecord);
  fprintf(fHandle,"[CDRECORDOPTIONS]%s\n", chrAudioCDROptions);
  fprintf(fHandle,"[CDRECORDOPTIONSDATA]%s\n", chrDataCDROptions);

  /* MP3 decoder settings */
  fprintf(fHandle,"[MP3DECODERPATH]%s\n", globalData.chrMpg123Path);
  fprintf(fHandle,"[MP3DECODER]%d\n", iMp3Decoder);
  if(globalData.bMpg123SwabBytes)
    fprintf(fHandle,"[MP3SWAPBYTES]YES\n");
  else
    fprintf(fHandle,"[MP3SWAPBYTES]NO\n");

  /* MP3 encoder settings */
  fprintf(fHandle,"[MP3ENCODERPATH]%s\n", chrMP3EncoderPath);
  fprintf(fHandle,"[MP3ENCODEROPTIONS]%s\n", chrMP3EncoderOptions);
  fprintf(fHandle,"[MP3ENCODERQUALITY]%d\n", ulMP3Quality);
  fprintf(fHandle,"[MP3ENCODERBITRATE]%d\n", ulMP3Bitrate);
  fprintf(fHandle,"[MP3ENCODERPRIORITY]%d\n", ulMP3EncoderPriority);

  fprintf(fHandle,"[MP3NAMEPART1]%d\n", g_cChosenMP3Name[0]);
  fprintf(fHandle,"[MP3NAMEPART2]%d\n", g_cChosenMP3Name[1]);
  fprintf(fHandle,"[MP3NAMEPART3]%d\n", g_cChosenMP3Name[2]);
  fprintf(fHandle,"[MP3NAMEPART4]%d\n", g_cChosenMP3Name[3]);
  fprintf(fHandle,"[MP3NAMEFILLSTRING1]%s\n", g_chrMP3NameFillStrings[0]);
  fprintf(fHandle,"[MP3NAMEFILLSTRING2]%s\n", g_chrMP3NameFillStrings[1]);
  fprintf(fHandle,"[MP3NAMEFILLSTRING3]%s\n", g_chrMP3NameFillStrings[2]);
  fprintf(fHandle,"[MP3LIBRARY]%s\n", globalData.chrMP3LibraryPath);
  /* Grabber settings */
  fprintf(fHandle,"[GRABBERPATH]%s\n", globalData.chrGrabberPath);
  fprintf(fHandle,"[GRABBEROPTIONS]%s\n", globalData.chrGrabberOptions);
  if(globalData.bTrackNumbers)
    fprintf(fHandle,"[GRABBERADDNUMBERS]YES\n");
  else
    fprintf(fHandle,"[GRABBERADDNUMBERS]NO\n");
  fprintf(fHandle,"[GRABBERCDDRIVE]%s\n", chosenCD );
  fprintf(fHandle,"[GRABBERID]%d\n", globalData.iGrabberID );
  /* DVDDAO settings */
  fprintf(fHandle,"[DVDDAOPATH]%s\n", chrDVDDao);

  /* lang subdir */
  fprintf(fHandle,"[LANGUAGEFILES]%s\\lang\n", chrInstallDir);
  fprintf(fHandle,"[LANGUAGEDLL]%s\n", chrLanguageDLL);
}

void addGrabInfoToFile(CWAudioFolder * thisPtr, FILE *fHandle)
{
  SHORT a;
  int numTracks;
  CDDBINFO cddbInfo;

  if(!WinIsWindow(WinQueryAnchorBlock(HWND_DESKTOP), thisPtr->hwndGrab)
     ||!WinIsWindowVisible(thisPtr->hwndGrab))
    {
      /* Grab window not visible */
      fprintf(fHandle,"[GRABBING]NO");
      return;
    }
  fprintf(fHandle,"[GRABBING]YES\n");
  fprintf(fHandle,"[CDDBID]%08x\n", CDDBDiscID(chosenCD, &cddbInfo));
  /* Get num Tracks */
  numTracks=SHORT1FROMMR(WinSendMsg(WinWindowFromID(thisPtr->hwndGrab,
                                                    IDLB_GRABTRACKS),LM_QUERYITEMCOUNT,0,0));
  fprintf(fHandle, "[NUMTRACKS]%d\n", numTracks);
  if(!numTracks)
    return;

  a=SHORT1FROMMR(WinSendMsg(WinWindowFromID(thisPtr->hwndGrab,IDLB_GRABTRACKS),
                            LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0));

  while(a!=LIT_NONE) {
    fprintf(fHandle,"[TRACKSELECTED]%d ", a+1);
    /* Tracksize in sec */
    fprintf(fHandle,"%d\n", CDQueryTrackSize( a+1, chosenCD)/4/44100);
    a=SHORT1FROMMR(WinSendMsg(WinWindowFromID(thisPtr->hwndGrab,IDLB_GRABTRACKS), LM_QUERYSELECTION,MPFROMSHORT(a),0));
  };
}

void addAudioFolderSettingsToFile(CWAudioFolder * thisPtr, FILE *fHandle)
{
  ULONG  ulWriteFlagsLocal;

  ulWriteFlagsLocal=thisPtr->cwQueryWriteFlags();

  /* Folder options */
  if(ulWriteFlagsLocal & IDWF_DUMMY)
    fprintf(fHandle,"[TESTONLY]YES\n");
  else
    fprintf(fHandle,"[TESTONLY]NO\n");
  if(!(lCDROptions & IDCDR_NOEJECT))
    fprintf(fHandle,"[EJECT]YES\n");
  else
    fprintf(fHandle,"[EJECT]NO\n");
#if 0
  if(ulWriteFlagsLocal & IDWF_PREEMP)
    fprintf(fHandle,"[PREEMPHASIS]YES\n");
  else
    fprintf(fHandle,"[PREEMPHASIS]NO\n");
#endif
  if(ulWriteFlagsLocal & IDWF_NOFIX)
    fprintf(fHandle,"[NOFIX]YES\n");
  else
    fprintf(fHandle,"[NOFIX]NO\n");
  if(ulWriteFlagsLocal & IDWF_DAO)
    fprintf(fHandle,"[DAO]YES\n");
  else
    fprintf(fHandle,"[DAO]NO\n");

}

BOOL  CWAudioFolder::cwCreateContentsFile(char * fileName, HWND hwndFrame)
{
  FILE *fHandle;
  char name[CCHMAXPATH];
  ULONG ulNameSize;
  HWND hwndCnr;
  PMINIRECORDCORE mrc;   
  WPObject * contentObject;
  WPObject * tempObject;
  BOOL bIsShadow;
  void * memPtr;
  ULONG  ulWriteFlagsLocal;
  int iExtPriv;

  /* Build full path */
  if(cwRequestMutex(hmtxFileName, 30000)==ERROR_TIMEOUT)
    return FALSE;  
  iExt++;
  if(iExt==1000)
    iExt=1;
  /* The extension of our filelist file */
  iExtPriv=iExt;
  cwReleaseMutex(hmtxFileName);
  
  sprintf(fileName,"%s\\temp\\%s%03d",chrInstallDir,"contents.",iExtPriv);      
  remove(fileName);
  fHandle=fopen(fileName,"a");
  if(fHandle) {
    ulNameSize=sizeof(name);
    /* Query the full path */
    this->wpQueryRealName(name,&ulNameSize,TRUE);
    /* Insert the folder name */
    fprintf(fHandle,name);
    fprintf(fHandle,"\n[CONTENTS]\n");
    /* Get hwnd of folder container */ 
    hwndCnr=WinWindowFromID(hwndFrame,FID_CLIENT);
    if(hwndCnr){ /* Catch error */
      /* Get first container item of our folder */
      mrc=(PMINIRECORDCORE)WinSendMsg(hwndCnr,CM_QUERYRECORD,NULL,
                                      MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
      if(mrc){ 
        while(mrc) {
          bIsShadow=FALSE;
          /* Get wps-object-ptr. from container item */
          contentObject=(WPObject*)OBJECT_FROM_PREC(mrc);//Get object
          if(somIsObj(contentObject)) {
            tempObject=contentObject;
            /* Get file system object or NULL */
            contentObject=cwGetFileSystemObject(contentObject);
            if(tempObject!=contentObject)
              bIsShadow=TRUE;
          }
          if(somIsObj(contentObject)){
            /* It's a file system object */
            /* Check, if it's a wave file or a mp3file or an audio file if MMIOMP3 is selected */
            //  if(cwIsWaveFile(contentObject)|| cwIsMp3File(contentObject)){
            if(cwIsWaveFile(contentObject, FALSE) /*|| cwIsMp3File(contentObject)*/ ||
               (cwIsAudioFile(contentObject) && iMp3Decoder==IDKEY_USEMMIOMP3) ) {
              /* Yes, query the full path */
              ulNameSize=sizeof(name);
              ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
              if(bIsShadow)
                fprintf(fHandle,"[SHADOW;");
              else
                fprintf(fHandle,"[FILE;");
              if(mrc->flRecordAttr&CRA_SELECTED)
                fprintf(fHandle,"SELECTED]");
              else
                fprintf(fHandle,"UNSELECTED]");
              fprintf(fHandle,name);
              fprintf(fHandle,"\n");
            }
          }/* end of if(contentObject) */           
          /* Get next container item */
          mrc=(PMINIRECORDCORE)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(mrc),
                                          MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
        }// end of while(mrc)
      }// end of if(mrc)      
    }

    addSettingsToFile(fHandle);
    addAudioFolderSettingsToFile(this, fHandle);
    addGrabInfoToFile(this, fHandle);

    fclose(fHandle);
    if((memPtr=malloc(sizeof(name)))!=NULL) {
      strncpy((char*)memPtr, fileName, sizeof(name));
      /* This thread deletes the contents file after a few minutes to clean the temp dir */
      _beginthread(_contentsFileDeleteThread,NULL,8192*2, memPtr); //Fehlerbehandlung fehlt    
    }
    return TRUE;    
  }
  return FALSE;
}

/* Create a contents file for mp3-decoding */
BOOL  CWAudioFolder::cwCreateContentsFileForDec(char * fileName, HWND hwndFrame)
{
  FILE *fHandle;
  char name[CCHMAXPATH];
  ULONG ulNameSize;
  HWND hwndCnr;
  PMINIRECORDCORE mrc;   
  WPObject * contentObject;
  WPObject * tempObject;
  BOOL bIsShadow;
  void * memPtr;
  ULONG  ulWriteFlagsLocal;
  /* Extension counter */
  //  static int iExt=0;
  int iExtPriv;

  /* Build full path */
  if(cwRequestMutex(hmtxFileName, 30000)==ERROR_TIMEOUT)
    return FALSE;  
  iExt++;
  if(iExt==1000)
    iExt=1;
  /* The extension of our filelist file */
  iExtPriv=iExt;
  cwReleaseMutex(hmtxFileName);
  
  sprintf(fileName,"%s\\temp\\%s%03d",chrInstallDir,"contents.",iExtPriv);      
  remove(fileName);
  fHandle=fopen(fileName,"a");
  if(fHandle) {
    ulNameSize=sizeof(name);
    /* Query the full path */
    this->wpQueryRealName(name,&ulNameSize,TRUE);
    /* Insert the folder name */
    fprintf(fHandle,name);
    fprintf(fHandle,"\n[CONTENTS]\n");
    /* Get hwnd of folder container */ 
    hwndCnr=WinWindowFromID(hwndFrame,FID_CLIENT);
    if(hwndCnr){ /* Catch error */
      /* Get first container item of our folder */
      mrc=(PMINIRECORDCORE)WinSendMsg(hwndCnr,CM_QUERYRECORD,NULL,
                                      MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));

      if(mrc){ 
        while(mrc) {
          bIsShadow=FALSE;
          /* Get wps-object-ptr. from container item */
          contentObject=(WPObject*)OBJECT_FROM_PREC(mrc);//Get object
          if(somIsObj(contentObject)) {
            tempObject=contentObject;
            /* Get file system object or NULL */
            contentObject=cwGetFileSystemObject(contentObject);
            if(tempObject!=contentObject)
              bIsShadow=TRUE;
          }
          if(somIsObj(contentObject)){
            /* It's a file system object */
            /* Check, if it's a wave file or a mp3file or an audio file if MMIOMP3 is selected */
            //  if(cwIsWaveFile(contentObject)|| cwIsMp3File(contentObject)){
            if(iMp3Decoder==IDKEY_USEMMIOMP3) {
              if( cwIsMp3File(contentObject) ||
                  cwIsAudioFile(contentObject)
                                ) {
                /* Yes, query the full path */
                ulNameSize=sizeof(name);
                ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                if(bIsShadow)
                  fprintf(fHandle,"[SHADOW;");
                else
                  fprintf(fHandle,"[FILE;");
                if(mrc->flRecordAttr&CRA_SELECTED)
                  fprintf(fHandle,"SELECTED]");
                else
                  fprintf(fHandle,"UNSELECTED]");
                fprintf(fHandle,name);
                fprintf(fHandle,"\n");
              }
            }/* iMp3Decoder==IDKEY_USEMMIOMP3 */
              else if(cwIsMp3File(contentObject) ) {
                /* Yes, query the full path */
                ulNameSize=sizeof(name);
                ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                if(bIsShadow)
                  fprintf(fHandle,"[SHADOW;");
                else
                  fprintf(fHandle,"[FILE;");
                if(mrc->flRecordAttr&CRA_SELECTED)
                  fprintf(fHandle,"SELECTED]");
                else
                  fprintf(fHandle,"UNSELECTED]");
                fprintf(fHandle,name);
                fprintf(fHandle,"\n");
              }
          }/* end of if(contentObject) */           
          /* Get next container item */
          mrc=(PMINIRECORDCORE)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(mrc),
                                          MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
        }// end of while(mrc)
      }// end of if(mrc)      
    }

    addSettingsToFile(fHandle);
    addAudioFolderSettingsToFile(this, fHandle);

    fclose(fHandle);
    if((memPtr=malloc(sizeof(name)))!=NULL) {
      strncpy((char*)memPtr, fileName, sizeof(name));
      /* This thread deletes the contents file after a few minutes to clean the temp dir */
      _beginthread(_contentsFileDeleteThread,NULL,8192*2, memPtr); //Fehlerbehandlung fehlt    
    }
    return TRUE;    
  }
  return FALSE;
}

/* Create a contents file for mp3 encoding of wave files */
BOOL  CWAudioFolder::cwCreateContentsFileForEncoding(char * fileName, HWND hwndFrame)
{
  FILE *fHandle;
  char name[CCHMAXPATH];
  ULONG ulNameSize;
  HWND hwndCnr;
  PMINIRECORDCORE mrc;   
  WPObject * contentObject;
  WPObject * tempObject;
  BOOL bIsShadow;
  void * memPtr;
  ULONG  ulWriteFlagsLocal;
  /* Extension counter */
  //  static int iExt=0;
  int iExtPriv;

  /* Build full path */
  if(cwRequestMutex(hmtxFileName, 30000)==ERROR_TIMEOUT)
    return FALSE;  
  iExt++;
  if(iExt==1000)
    iExt=1;
  /* The extension of our filelist file */
  iExtPriv=iExt;
  cwReleaseMutex(hmtxFileName);
  
  sprintf(fileName,"%s\\temp\\%s%03d",chrInstallDir,"contents.",iExtPriv);      
  remove(fileName);
  fHandle=fopen(fileName,"a");
  if(fHandle) {
    ulNameSize=sizeof(name);
    /* Query the full path */
    this->wpQueryRealName(name,&ulNameSize,TRUE);
    /* Insert the folder name */
    fprintf(fHandle,name);
    fprintf(fHandle,"\n[CONTENTS]\n");
    /* Get hwnd of folder container */ 
    hwndCnr=WinWindowFromID(hwndFrame,FID_CLIENT);
    if(hwndCnr){ /* Catch error */
      /* Get first container item of our folder */
      mrc=(PMINIRECORDCORE)WinSendMsg(hwndCnr,CM_QUERYRECORD,NULL,
                                      MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));

      if(mrc){ 
        while(mrc) {
          bIsShadow=FALSE;
          /* Get wps-object-ptr. from container item */
          contentObject=(WPObject*)OBJECT_FROM_PREC(mrc);//Get object
          if(somIsObj(contentObject)) {
            tempObject=contentObject;
            /* Get file system object or NULL */
            contentObject=cwGetFileSystemObject(contentObject);
            if(tempObject!=contentObject)
              bIsShadow=TRUE;
          }
          if(somIsObj(contentObject)){
            /* It's a file system object */
            /* Check, if it's a wave file or a mp3file or an audio file if MMIOMP3 is selected */
            if(cwIsWaveFile(contentObject, TRUE))
              {
                /* Yes, query the full path */
                ulNameSize=sizeof(name);
                ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                if(bIsShadow)
                  fprintf(fHandle,"[SHADOW;");
                else
                  fprintf(fHandle,"[FILE;");
                if(mrc->flRecordAttr&CRA_SELECTED)
                  fprintf(fHandle,"SELECTED]");
                else
                  fprintf(fHandle,"UNSELECTED]");
                fprintf(fHandle,name);
                fprintf(fHandle,"\n");
              }
          }/* end of if(contentObject) */           
          /* Get next container item */
          mrc=(PMINIRECORDCORE)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(mrc),
                                          MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
        }// end of while(mrc)
      }// end of if(mrc)      
    }

    addSettingsToFile(fHandle);
    addAudioFolderSettingsToFile(this, fHandle);

    fclose(fHandle);
    if((memPtr=malloc(sizeof(name)))!=NULL) {
      strncpy((char*)memPtr, fileName, sizeof(name));
      /* This thread deletes the contents file after a few minutes to clean the temp dir */
      _beginthread(_contentsFileDeleteThread,NULL,8192*2, memPtr); //Fehlerbehandlung fehlt    
    }
    return TRUE;    
  }
  return FALSE;
}


/*********************************************************************/
/*             hwndListBox: handle of listbox holding the tracknames */
/*********************************************************************/
BOOL buildTocFile( CWAudioFolder *thisPtr, HWND hwndListBox)
{
  char name[CCHMAXPATH];
  ULONG ulNameSize;
  ULONG ulSize;
  int b;
  LONG a;
  FILE *fStream;
  ULONG ulFlags;
  BOOL bFirstTrack;

  /* Query the full path of our folder*/
  ulNameSize=sizeof(name);
  thisPtr->wpQueryRealName(name,&ulNameSize,TRUE);
  strcat(name,"\\default.toc");
  fStream=fopen(name,"w");
  if(!fStream)
    return FALSE;

  bFirstTrack=TRUE;
  ulFlags=thisPtr->cwQueryWriteFlags();
  /* Build toc file */
  fprintf(fStream,"CD_DA\n\n");
  a=SHORT1FROMMR(WinSendMsg(hwndListBox,LM_QUERYITEMCOUNT, NULL, NULL));
  for(b=0;b<a;b++) {
    if(SHORT1FROMMR(WinSendMsg(hwndListBox,LM_QUERYITEMTEXT,MPFROM2SHORT((SHORT)b,(SHORT)CCHMAXPATH),&name))) {
      changeBackslash(name);        
      fprintf(fStream,"//Track %d\n",b+1);
      fprintf(fStream,"TRACK AUDIO\n");
      if(ulFlags & IDWF_PREEMP)
        fprintf(fStream,"PRE_EMPHASIS\n");
      /* Pregap */
      /*      if(bFirstTrack) {
              fprintf(fStream,"PREGAP 0:2:0\n");
              bFirstTrack=FALSE;
              }*/
      fprintf(fStream,"FILE \"%s\" 0\n\n", name);
    }
  }
  fclose(fStream);
  return TRUE;
}

























