/*
 * This file is (C) Chris Wohlgemuth 1999-2003
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

#include <process.h>

#include "datafolder.h"
#include "audiofolder.hh"
#include "audiofolderhelp.h"
#include "cwaudio.hh"

#pragma SOMAsDefault(off)
#include <sys\types.h>
#include <sys\stat.h>
#pragma SOMAsDefault(pop)

#include <stdio.h>
#include <string.h>//for memccpy()
#include <stdlib.h>

extern GLOBALDATA globalData;

extern PVOID pvSharedMem;

extern char chrImage[CCHMAXPATH];/* Path to iso-image */
extern char chrInstallDir[CCHMAXPATH];
extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
//extern char chrMpg123Path[CCHMAXPATH];

extern char chrMntIsoFSPath[CCHMAXPATH];

extern char chrDVDDao[CCHMAXPATH];/* Path to dvdao */

extern int iBus;
extern int iTarget;
extern int iLun;

extern int iMp3Decoder; /* Which decoder to use */

extern ATOM atomStartWrite;
extern HMODULE hDataResource;
extern HMODULE hAudioResource;

extern HMTX hmtxFileName;

/* For calling the method in the CWAudio class. Linking to the DLL would require the
   new audio classes. By dynamically querying the method the media folder works without
   installing the audio classes. */  
typedef ULONG   SOMLINK somTP_CWAudio_cwmmQueryTrackInfo(CWAudio *somSelf,
		char** chrString,
		ULONG ulSize,
		int iWhich);
typedef somTP_CWAudio_cwmmQueryTrackInfo *somTD_CWAudio_cwmmQueryTrackInfo;
SOMClass* cwGetSomClass(char* chrClassName);
BOOL cwmmQueryCWAudioTrackInfoMethodPtr();
SOMClass* queryCWAudioClass();
SOMClass* queryMMAudioClass(void);

BOOL checkFileExists(char* chrFileName);
ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);
ULONG showMsgBox2(HWND hwnd, ULONG ulIDTitle, ULONG ulIDText, HMODULE hModule, ULONG ulFlag);

void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
ULONG launchWrapper(PSZ parameter, PSZ folderPath,HWND hwnd, PSZ wrapperExe, PSZ pszTitle);
void buildDataWriteParam(CWDataFolder* thisPtr, char * text ,int iSpeedChosen, char *chrDev);
ULONG launchPMWrapper(PSZ parameter, PSZ folderPath, PSZ wrapperExe, PSZ pszTitle);

MRESULT EXPENTRY CDToolsDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY selectWriterDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY msInfoStatusDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

BOOL buildMkisofsParam(CWDataFolder* thisPtr, char * text, char * outputName, char * pathListFile, char *chrChosenDev);
BOOL buildMkisofsParamShadowsRootOnly(CWDataFolder* thisPtr, char * text, char * outputName, char *chrChosenDev);
BOOL resetArchiveBit(WPFolder* thisPtr,  FILE *file ,char * parentDir, int iDepth, HWND hwndTest, CWDataFolder* thisPtrData);
BOOL audioHlpStartMp3Query(char *name, HWND hwnd);

void SysWriteToTrapLog(const char* chrFormat, ...);

ULONG cwRequestMutex(HMTX  hmtxBMP, ULONG ulTimeOut);
ULONG cwReleaseMutex(HMTX  hmtxBMP);

/* This thread is used to clode and reopen the settings notebook
   after selecting "Expert mode". */
void _Optlink reOpenSettingsThreadFunc (void *arg)
{
  CWCreatorSettings * thisPtr;
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;

  thisPtr=(CWCreatorSettings*)arg;

  if(!somIsObj(thisPtr))
    return;

  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {
      /* Close all views. Only one view is allowed so this will close
         the open settings notebook. */
      if(thisPtr->wpClose()) {
        thisPtr->wpWaitForClose(NULLHANDLE, VIEW_SETTINGS, SEM_INDEFINITE_WAIT, FALSE);
        thisPtr->wpSetup("OPEN=DEFAULT");
        DosSleep(5000);
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
}

BOOL buildBinFileName(char *chrBuffer)
{
  static int iExt=0;

  if(cwRequestMutex(hmtxFileName, 100000)==ERROR_TIMEOUT)
    return FALSE;
  iExt++;
  if(iExt==1000)
    iExt=1;
  sprintf(chrBuffer, "%s\\%s.%03d" , chrInstallDir , SHAREDMEMFILE, iExt);
  cwReleaseMutex(hmtxFileName);
  return TRUE;
}

/* This Proc handles the image size calculation */
MRESULT EXPENTRY mkisofsObjectProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  CWDataFolder * thisPtr;
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  ULONG ulFlags;
  char *textPtr;
  char *textPtr2; 
  HOBJECT hObject;

  switch (msg)
    {
    case WM_APPTERMINATENOTIFY:
      thisPtr=(CWDataFolder*)WinQueryWindowULong(hwnd,QWL_USER);/* Get object ptr */
      if(somIsObj(thisPtr)) {
        HWND hwndOwner=HWND_DESKTOP;

        if(WinIsWindow(WinQueryAnchorBlock(HWND_DESKTOP), thisPtr->hwndMkisofsMain))
          hwndOwner=thisPtr->hwndMkisofsMain;

        thisPtr->cwSetStatusText("");/* Clear status line */      
        switch(LONGFROMMP(mp1)) {
        case ACKEY_IMAGEONLY:
          if(!(LONGFROMMP(mp2))) {
            ulFlags=thisPtr->cwQueryMkisofsFlags();
            if((ulFlags & IDMK_USEARCHIVEBIT) && (ulFlags & IDMK_RESETARCHIVEBIT))
              thisPtr->cwResetArchiveBit();/* Reset archive bits after successful image creation */

            ulFlags=thisPtr->cwQueryCDTypeFlags();        
            if(!(ulFlags & IDCDT_MULTISESSION)) {
              if((hObject=WinQueryObject(ID_ISOFS_MOUNT))!=NULLHANDLE) {
                sprintf(text,"\"%s\"",thisPtr->chrImageName);
                ulFlags=sizeof(title);
                thisPtr->wpQueryRealName(title, &ulFlags,TRUE);
                launchPMWrapper(text, title, "pmmntiso.exe", "");
              }
              else {
                /*
                  Title: "Image file creation"
                  Text:  "CD-ROM image file created. You may burn this file on a CDR now."
                  */
                DosBeep(1000, 100);
                showMsgBox2(hwndOwner, IDSTRD_CREATEIMAGETITLE, IDSTRD_CREATEIMAGEDONE,
                            hDataResource, MB_OK | MB_INFORMATION | MB_MOVEABLE);
              }
            }
            else
              /*
                Title: "Image file creation"
                Text:  "CD-ROM image file created. You may burn this file on a CDR now."
                */
              showMsgBox2(hwndOwner, IDSTRD_CREATEIMAGETITLE, IDSTRD_CREATEIMAGEDONE,
                          hDataResource, MB_OK | MB_INFORMATION | MB_MOVEABLE);
          }
          else
            /*
              Title: "Image file creation"
              Text:  "Error while creating the image file."
             */
            showMsgBox2(hwndOwner, IDSTRD_CREATEIMAGETITLE, IDSTRD_ERRORIMAGECREATION,
                        hDataResource, MB_OK | MB_ERROR|MB_MOVEABLE);
          break;
        case ACKEY_WRITEONLY:
          if(!(LONGFROMMP(mp2)))
            /*
              Title: "Image writing"
              Text:  "Image file written by CDRecord/2."
             */
            showMsgBox2(hwndOwner, IDSTRD_WRITEIMAGETITLE, IDSTRD_WRITEIMAGEDONE,
                        hDataResource, MB_OK | MB_INFORMATION|MB_MOVEABLE);
          else
            /*
              Title: "Image writing"
              Text:  "Error while writing the image file. Examine 'write.log.'"
             */
            showMsgBox2(hwndOwner, IDSTRD_WRITEIMAGETITLE, IDTRSD_ERRORIMAGEWRITING,
                        hDataResource, MB_OK | MB_ERROR|MB_MOVEABLE);
          break;
        case ACKEY_PRINTSIZE:
          {
            if(LONGFROMMP(mp2)==0) {
              /* Text: "There was an error while querying the image size. If merging a previous session the
                 CD with the session must be in the writer."
                 Title:  "Check size"
                 */
              messageBox( text, IDSTRD_CHECKSIZEERRORMULTI , sizeof(text),
                          title, IDSTRD_CHECKSIZETITLE, sizeof(title),
                          hDataResource, hwndOwner, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
            }
            else {
              if(LONGFROMMP(mp2)<=2097152) {
                /* title: "Estimated imagesize is %d.%0.3d.%0.3d bytes" */
                getMessage( title, IDSTRD_IMAGESIZESTATUSLINETEXT, sizeof(title),hDataResource, HWND_DESKTOP);
                sprintf(text, title,
                        LONGFROMMP(mp2)*2048/1000000,(LONGFROMMP(mp2)*2048%1000000)/1000,LONGFROMMP(mp2)*2048%1000);
              }
              else {
                /* title: "Estimated imagesize is %d Mb" */
                getMessage( title, IDSTRD_IMAGESIZESTATUSLINETEXT2, sizeof(title),hDataResource, HWND_DESKTOP);
                sprintf(text, title, LONGFROMMP(mp2)*2/1024);
              }
              thisPtr->cwSetStatusText(text);
              DosBeep(2000,200);
            }
            break;
          }
        case ACKEY_CDSIZE:
        case ACKEY_DVDSIZE:
          if(LONGFROMMP(mp2)<=2097152) {
            /* title: "Free CD space is %d.%0.3d.%0.3d bytes" */
            getMessage( title, IDSTR_CDSIZESTATUSLINETEXT, sizeof(title),hDataResource, HWND_DESKTOP);
            sprintf(text, title,
                    LONGFROMMP(mp2)*2048/1000000,(LONGFROMMP(mp2)*2048%1000000)/1000,LONGFROMMP(mp2)*2048%1000);
          }
          else {
            getMessage( title, IDSTR_CDSIZESTATUSLINETEXT2, sizeof(title),hDataResource, HWND_DESKTOP);
            sprintf(text, title, LONGFROMMP(mp2)*2/1024);
          }
          thisPtr->cwSetStatusText(text);
          DosBeep(2000,200);
          break;
        default:
          break;
        }/* switch */
      }/* if(thisPtr) */           
      WinPostMsg(hwnd,WM_QUIT,0,0);
      return (MRESULT) FALSE;
      break;
    default:
      if(msg == atomStartWrite) {
        thisPtr=(CWDataFolder*)PVOIDFROMMP(mp1);       
        WinSetWindowULong(hwnd,QWL_USER,(ULONG)thisPtr);/* Save object ptr */
        if(!thisPtr) {
          /* Error: quit */
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          return 0;
        }
        switch(LONGFROMMP(mp2)) {   
        case ACKEY_IMAGEONLY:
          return FALSE;
        case ACKEY_PRINTSIZE:
          return FALSE;
        case ACKEY_CDSIZE:
          /* querying cd size */
          sprintf(text,"dev=%d,%d,%d",iBus,iTarget,iLun);
          launchWrapper(text, "", hwnd,"cdsize.exe","Query CD size");
          return FALSE;
        case ACKEY_DVDSIZE:
          /* querying cd size */
          sprintf(text,"\"%s\" \"-d %d,%d,%d\"", chrDVDDao, iBus, iTarget, iLun);
          launchWrapper(text, "", hwnd, "dvdsize.exe","Query DVD size");
          return FALSE;
        default:
          break;
        }/* switch */
      }/* if(msg == atomStartWrite) */
      return WinDefWindowProc( hwnd, msg, mp1, mp2);
    }
  return FALSE;
}


/* This Proc handles  */
MRESULT EXPENTRY onTheFlyObjectProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  CWDataFolder * thisPtr;
  char *textPtr;
  char *textPtr2; 

  switch (msg)
    {
    case WM_APPTERMINATENOTIFY:
      thisPtr=(CWDataFolder*)WinQueryWindowULong(hwnd,QWL_USER);/* Get object ptr */
      if(somIsObj(thisPtr)) {
        thisPtr->cwSetPreviousStartSector(LONGFROMMP(mp1));
        thisPtr->cwSetNextStartSector(LONGFROMMP(mp2));
      }/* if(thisPtr) */           
      WinPostMsg(hwnd,WM_QUIT,0,0);
      return (MRESULT) FALSE;
    default:
      if(msg == atomStartWrite) {
        thisPtr=(CWDataFolder*)PVOIDFROMMP(mp1);       
        WinSetWindowULong(hwnd,QWL_USER,(ULONG)thisPtr);/* Save object ptr */
        if(!thisPtr) {
          /* Error: quit */
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          return 0;
        }
        switch(LONGFROMMP(mp2)) {   
        case ACKEY_IMAGEONLY:
          return FALSE;
        case ACKEY_PRINTSIZE:
          return FALSE;
        case ACKEY_CDSIZE:
          return FALSE;
        default:
          break;
        }/* switch */
      }/* if(msg == atomStartWrite) */
      return WinDefWindowProc( hwnd, msg, mp1, mp2);
    }
  return FALSE;
}

BOOL getMultiSessionInfo(HAB hab, QMSG* pqmsg, CWDataFolder * thisPtr, DEVICEINFO *devInfo)
{
  HWND hwndMkisofs;
  HWND hwndStatus;

  /* Get multisession info */
  /* Putting 0 into the sectors will trigger an error if something goes wrong in the window procedure */
  thisPtr->cwSetPreviousStartSector(0);
  thisPtr->cwSetNextStartSector(0);

  sprintf(devInfo->chrDev,"%d,%d,%d",iBus, iTarget, iLun);
  devInfo->iSpeed=1;
  /* Select writer */        
  if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, selectWriterDialogProc, hDataResource, IDDLG_SELECTDEVICE, devInfo) == DID_ERROR )
    {
      /* User abort */
      return FALSE;
    }
  else {
    char text[25];
    char text2[255];
    /* This window is only for handling the msinfo message giving us the sectors */
    hwndMkisofs=WinCreateWindow(HWND_OBJECT,WC_BUTTON,"myPrintSizeObj",0,0,0,0,0,NULL,HWND_BOTTOM,12343,NULL,NULL);
    if(hwndMkisofs) {
      WinSubclassWindow(hwndMkisofs,&onTheFlyObjectProc);
      /* Send object pointer to our window for later use */
      WinSendMsg(hwndMkisofs,atomStartWrite,MPFROMP(thisPtr),MPFROMLONG(ACKEY_PRINTSIZE));
      
      /* Get msinfo */
      thisPtr->iCreate=2;
      hwndStatus=WinLoadDlg(HWND_DESKTOP,hwndMkisofs,
                            msInfoStatusDialogProc,hDataResource,IDDLG_GETMSINFOSTATUS,0);
      sprintf(text, "dev=%s", devInfo->chrDev);

      /* "Get multisession info" */
      getMessage(text2, IDSTRD_GETMSINFOTITLE, sizeof(text2), hDataResource, HWND_DESKTOP);

      launchWrapper(text, "", hwndMkisofs, "msinfo.exe", text2);
      
      while(WinGetMsg(hab,pqmsg,(HWND)NULL,0,0))
        WinDispatchMsg(hab,pqmsg);
      WinDestroyWindow(hwndMkisofs);
      WinDestroyWindow(hwndStatus);

      return TRUE;
    }/* if(hwndMkisofs) */
  }/* WinDlgBox */
  return TRUE;
}

/* This thread is used for image creation, It waits for the mkisofs process to end */
void _Optlink createImageThreadFunc (void *arg)
{
  HWND hwndMkisofs;
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  CWDataFolder * thisPtr;
  char *text;
  ULONG ulFlags;
  BOOL bBreak=FALSE;
  char chrFileList[CCHMAXPATH+2]="";
  char chrCmdFile[CCHMAXPATH+10]="";
  char chrOutName[CCHMAXPATH+2]="";
  ULONG ulError;
  FILE * file;
  char chrDev[20];
  DEVICEINFO *devInfo;
  int iSpeedLocal;

  thisPtr=(CWDataFolder*)arg;

  if(!somIsObj(thisPtr))
    return;

  if(!buildBinFileName(chrFileList)) {
    return;/* Error */
  }

  sprintf(chrCmdFile,"\"%s\"", chrFileList);
  
  ulError=0;
  if((text=thisPtr->wpAllocMem(SHAREDMEM_SIZE, &ulError))==NULLHANDLE) {
    return;
  }
  memset(text, 0, SHAREDMEM_SIZE);

  sprintf(thisPtr->chrStatusText,"");

  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {
      thisPtr->cwEnableWriteControls(FALSE);
      
      /* Lower priority or the GUI will become slow while querying the tree */
      DosSetPriority(PRTYS_THREAD,PRTYC_IDLETIME,-2,0);

      if((file=fopen(chrFileList,"wb"))==NULL) {
        /* chrCmdFile: "Error while opening the FILELIST.xxx file! 
           Make sure the 'TEMP' directory exists in the Data-CD-Creator installation directory."
           chrFileList: "CD writing error!"
           */
        messageBox( chrCmdFile, IDSTR_NOFILELIST , sizeof(chrCmdFile),
                    chrFileList, IDSTR_WRITEERRORTITLE, sizeof(chrFileList),
                    hDataResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
        bBreak=TRUE;/* break */
      }

      if(!bBreak) {
        /* Check if we should write a multisession CD */
        ulFlags=thisPtr->cwQueryCDTypeFlags();
        
        if(ulFlags & IDCDT_MULTISESSION) {
          /* Check CD for multisession info */          
          /* Get local mem */
          ulError=0;
          if((devInfo=(DEVICEINFO*)thisPtr->wpAllocMem(sizeof(DEVICEINFO), &ulError))!=NULLHANDLE) {
            memset(devInfo, 0, sizeof(DEVICEINFO));         
            if(!getMultiSessionInfo(hab, &qmsg ,thisPtr, devInfo))
              bBreak=TRUE;/* User break */
            /* User selected a writer and speed */
            iSpeedLocal=devInfo->iSpeed;
            strncpy(chrDev , devInfo->chrDev, sizeof(devInfo->chrDev));
            thisPtr->wpFreeMem((PBYTE)devInfo);
          }
        }/* if(ulFlags & IDCDT_MULTISESSION) */
      }

      /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
      
      if(!bBreak) {
        ulFlags=thisPtr->cwQueryMkisofsFlags();
        /* Add the mkisofs command                     */
        /* This routine also check if a boot image     */
        /* is avaiable if boot CD creation is selected */
        if(ulFlags & IDMK_SHADOWSINROOTONLY) {
          sprintf(chrOutName, "\"%s\"",thisPtr->chrImageName);
          if(!buildMkisofsParamShadowsRootOnly(thisPtr, text, chrOutName, chrDev)) {
            thisPtr->cwSetStatusText(NULL);
            bBreak=TRUE;/* The memory is freed in the function */
          }
        }
        else {
          sprintf(chrOutName, "\"%s\"",thisPtr->chrImageName);
          if(!buildMkisofsParam(thisPtr, text, chrOutName, chrFileList, chrDev)) {
            thisPtr->cwSetStatusText(NULL);
            bBreak=TRUE;/* The memory is freed in the function */
          }
        }
      }

      if(!bBreak) {
        if(file) {
          fwrite(text, sizeof(char), SHAREDMEM_SIZE, file);
          hwndMkisofs=WinCreateWindow(HWND_OBJECT,WC_STATIC,"myMkisofsObj",0,0,0,0,0,NULL,HWND_BOTTOM,12341,NULL,NULL);
          if(hwndMkisofs) {
            /* Window created. */        
            WinSubclassWindow(hwndMkisofs,&mkisofsObjectProc);
            WinSendMsg(hwndMkisofs,atomStartWrite,MPFROMP(thisPtr),MPFROMLONG(ACKEY_IMAGEONLY));
            
            if(launchWrapper(chrCmdFile, "",hwndMkisofs, "lnchmkis.exe", "Create ISO image"))
              WinPostMsg(hwndMkisofs,WM_QUIT,(MPARAM)0,(MPARAM)0);
            
            while(WinGetMsg(hab,&qmsg,(HWND)NULL,0,0))
              WinDispatchMsg(hab,&qmsg);
            WinDestroyWindow(hwndMkisofs);
          }
        } /* file */
      }

      fclose(file);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  
  /* Free cmd-line mem */
  if(text)
    thisPtr->wpFreeMem(text);

  if(strlen(chrFileList))
    remove(chrFileList);

  if(strlen(chrCmdFile)) {
    if((text=strrchr(chrCmdFile,'"'))!=NULL)
      *text=0;    
    remove(&chrCmdFile[1]);
  }

  thisPtr->cwEnableWriteControls(TRUE);
  thisPtr->cwSetStatusText("");
  return;
}


/* This thread waits for the image size process to end */
void _Optlink printSizeThreadFunc (void *arg)
{
  HWND hwndMkisofs;
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  char *text;
  ULONG ulFlags;
  char *chrFound;
  BOOL bBreak=FALSE;
  char chrFileList[CCHMAXPATH]="";
  char chrCmdFile[CCHMAXPATH+10]="";
  ULONG ulError;
  FILE * file;
  CWDataFolder * thisPtr;
  char chrDev[20];
  DEVICEINFO *devInfo;
  int iSpeedLocal;

  thisPtr=(CWDataFolder*)arg;

  if(!somIsObj(thisPtr))
    return;
  
  if(!buildBinFileName(chrFileList)) {
    return;/* Error */
  }

  sprintf(chrCmdFile,"\"%s\"", chrFileList);
  
  ulError=0;
  if((text=thisPtr->wpAllocMem(SHAREDMEM_SIZE, &ulError))==NULLHANDLE) {
    return;
  }
  memset(text, 0, SHAREDMEM_SIZE);
  
  sprintf(thisPtr->chrStatusText,"");
  
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {
      HWND hwnd;

      if(WinIsWindow(WinQueryAnchorBlock(HWND_DESKTOP), thisPtr->hwndMkisofsMain))
        hwnd=thisPtr->hwndMkisofsMain;
      else
        hwnd=HWND_DESKTOP;

      thisPtr->cwEnableWriteControls(FALSE);

      if((file=fopen(chrFileList,"wb"))==NULL) {
        /* chrCmdFile: "Error while opening the FILELIST.xxx file!
           Make sure the 'TEMP' directory exists in the Data-CD-Creator installation directory."
           chrFileList: "CD writing error!"
           */
        messageBox( chrCmdFile, IDSTR_NOFILELIST , sizeof(chrCmdFile),
                    chrFileList, IDSTR_WRITEERRORTITLE, sizeof(chrFileList),
                    hDataResource, hwnd, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
        bBreak=TRUE;/* break */
      }
      
      if(!bBreak) {
        /* Check if we should write a multisession CD */
        ulFlags=thisPtr->cwQueryCDTypeFlags();
        
        if(ulFlags & IDCDT_MULTISESSION) {
          /* Check CD for multisession info */
          /* Get local mem */
          ulError=0;
          if((devInfo=(DEVICEINFO*)thisPtr->wpAllocMem(sizeof(DEVICEINFO), &ulError))!=NULLHANDLE) {
            memset(devInfo, 0, sizeof(DEVICEINFO));         
            if(!getMultiSessionInfo(hab, &qmsg ,thisPtr, devInfo))
              bBreak=TRUE;/* User break */
            /* User selected a writer and speed */
            iSpeedLocal=devInfo->iSpeed;
            strncpy(chrDev , devInfo->chrDev, sizeof(devInfo->chrDev));
            thisPtr->wpFreeMem((PBYTE)devInfo);
          }
        }
      }
 
      if(!bBreak) {
        ulFlags=thisPtr->cwQueryMkisofsFlags();
        
        /* First build the filelist                    */
        /* Add the mkisofs command                     */
        /* This routine also check if a boot image     */
        /* is avaiable if boot CD creation is selected */
        
        /* Lower priority or the GUI will become slow while querying the tree */
        DosSetPriority(PRTYS_THREAD,PRTYC_IDLETIME, -2,0);
        
        if(ulFlags & IDMK_SHADOWSINROOTONLY) {
          if(!buildMkisofsParamShadowsRootOnly(thisPtr, text,"-print-size", chrDev)) {
            thisPtr->cwSetStatusText(NULL);
            bBreak=TRUE;/* The memory is freed in the function */
          }
        }
        else {/* Follow all shadows */
          if(!buildMkisofsParam(thisPtr, text,"-print-size", chrFileList, chrDev)) {
            /* There was an error */
            thisPtr->cwSetStatusText(NULL);
            bBreak=TRUE;/* The memory is freed in the function */
          }
        }
      }/* !bBreak */

      if(!bBreak) {
        /* Remove the -o switch */
        chrFound=strstr(text,"-o-print-size");
        if(chrFound){
          *chrFound=' ';
          chrFound++;
          *chrFound=' ';
          /* Window created.    */
          /* Now launch mkisofs */             
          if(file) {
            fwrite(text, sizeof(char), SHAREDMEM_SIZE, file);
            /* Create another window for image calculation */
            hwndMkisofs=WinCreateWindow(HWND_OBJECT,WC_BUTTON,"myPrintSizeObj",0,0,0,0,0,NULL,HWND_BOTTOM,12343,NULL,NULL);
            if(hwndMkisofs) {
              WinSubclassWindow(hwndMkisofs,&mkisofsObjectProc);
              WinSendMsg(hwndMkisofs, atomStartWrite,MPFROMP(arg),MPFROMLONG(ACKEY_PRINTSIZE));

              if(launchWrapper(chrCmdFile, chrInstallDir,hwndMkisofs, "prntsize.exe","Query image size"))
                WinPostMsg(hwndMkisofs,WM_QUIT,(MPARAM)0,(MPARAM)0);
              
              while(WinGetMsg(hab,&qmsg,(HWND)NULL,0,0))
                WinDispatchMsg(hab,&qmsg);
            
              WinDestroyWindow(hwndMkisofs);
            }/* end of if(hwndMkisofs) */
          }else
            WinPostMsg(hwndMkisofs,WM_QUIT,MPFROMLONG(0),MPFROMLONG(0));
        }
      }/* !bBreak */

      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  fclose(file);
  thisPtr->cwEnableWriteControls(TRUE);
  thisPtr->cwSetStatusText("");
  
  /* Free cmd-line mem */
  if(text)
    thisPtr->wpFreeMem(text);

  /* Remove the filelist file */
  if(strlen(chrFileList))
    remove(chrFileList);

  if(strlen(chrCmdFile)) {
    if((text=strrchr(chrCmdFile,'"'))!=NULL)
      *text=0;    
    remove(&chrCmdFile[1]);
  }  
  return;
}

/* This thread waits for the mkisofs process to end */
void _Optlink onTheFlyThreadFunc (void *arg)
{
  HWND hwndMkisofs;
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  CWDataFolder * thisPtr;
  char *text;
  ULONG ulFlags;
  //  ULONG ulCDTypeFlags;
  char *chrPtrParam;
  BOOL bBreak=FALSE;
  ULONG ulSize;
  char chrFileList[CCHMAXPATH]="";
  char name[CCHMAXPATH*2+10];
  FILE * file;
  ULONG ulError;
  DEVICEINFO *devInfo;
  int iSpeedLocal;

  thisPtr=(CWDataFolder*)arg;

  if(!somIsObj(thisPtr))
    return;

  /* Get memory for parameters. The parameters will be written to a file later and the memory freed */ 
  ulError=0;
  if((text=thisPtr->wpAllocMem(SHAREDMEM_SIZE, &ulError))==NULLHANDLE) {
    return;
  }
  memset(text, 0, SHAREDMEM_SIZE);
    
  /* We use the mesg queue and the object window only to have msg queue for some window functions.
     We may use the window for some future enhancements, too. */
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {
      /* Disable controls of the dialog */
      thisPtr->cwEnableWriteControls(FALSE);
      
      /* Select writer and speed */
      /* Get local mem */
      ulError=0;
      if((devInfo=(DEVICEINFO*)thisPtr->wpAllocMem(sizeof(DEVICEINFO), &ulError))!=NULLHANDLE) {
        memset(devInfo, 0, sizeof(DEVICEINFO));         
        if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, selectWriterDialogProc, hDataResource, IDDLG_SELECTDEVICE, devInfo) == DID_ERROR )
          {
            thisPtr->wpFreeMem((PBYTE)devInfo);
            bBreak=TRUE;
          } 
        else {
          /* User selected a writer and speed */
          iSpeedLocal=devInfo->iSpeed;
          strncpy(name , devInfo->chrDev, sizeof(devInfo->chrDev));
          thisPtr->wpFreeMem((PBYTE)devInfo);
        }
      }
      else
        bBreak=TRUE;/* There was some kind of error */
      
      if(!bBreak) {
        /* Lower thread priority or the GUI will become slow while querying the tree */
        DosSetPriority(PRTYS_THREAD,PRTYC_IDLETIME,-2,0);
        
        /* Check if we should write a multisession CD */
        ulFlags=thisPtr->cwQueryCDTypeFlags();
        
        if(ulFlags & IDCDT_MULTISESSION) {
          /* Check CD for multisession info */

          hwndMkisofs=WinCreateWindow(HWND_OBJECT,WC_STATIC,"myMkisofsObj",0,0,0,0,0,NULL,HWND_BOTTOM,12341,NULL,NULL);
          if(hwndMkisofs) {
            char tempText[20];
            WinSubclassWindow(hwndMkisofs,&onTheFlyObjectProc);
            /* Send object pointer to our window for later use */
            WinSendMsg(hwndMkisofs,atomStartWrite,MPFROMP(thisPtr),MPFROMLONG(ACKEY_ONTHEFLY));
                        
            /* We are building the second image of a multisession disk */
            /* Get the sector information */
            /* Putting 0 into the sectors will trigger an error if something goes wrong in the window procedure */
            thisPtr->cwSetPreviousStartSector(0);
            thisPtr->cwSetNextStartSector(0);
            
            /* Get msinfo */
            thisPtr->iCreate=3;
            
            sprintf(tempText, "dev=%s", name);
            launchWrapper(tempText, "", hwndMkisofs, "msinfo.exe", "Query multisession info");
            
            while(WinGetMsg(hab,&qmsg,(HWND)NULL,0,0))
              WinDispatchMsg(hab,&qmsg);
            WinDestroyWindow(hwndMkisofs);
          }/* hwnd */
        }/* if((ulFlags & IDCDT_MULTISESSION) && !(ulFlags & IDCDT_FIRSTSESSION)) */
      }/* end of if(!bBreak) */
      
      if(!bBreak) {
        /* Text: "Collecting files..." */
        getMessage(chrFileList, IDSTRD_COLLECTINGFILES , sizeof(chrFileList), hDataResource,HWND_DESKTOP);
        sprintf(thisPtr->chrStatusText, chrFileList);
        thisPtr->cwSetStatusText(NULL);

        ulFlags=thisPtr->cwQueryMkisofsFlags();
        
        /* Add the mkisofs command                     */
        /* This routine also check if a boot image     */
        /* is avaiable if boot CD creation is selected */
        if(ulFlags & IDMK_SHADOWSINROOTONLY) {
          if(!buildMkisofsParamShadowsRootOnly(thisPtr, text,"-print-size", name)) {
            /* There was an error */
            thisPtr->cwSetStatusText(NULL);
            bBreak=TRUE;/* The memory is freed in the function */
          }
        }
        else {
          /* Follow all Shadows */
          if(!buildMkisofsParam(thisPtr, text,"-print-size", chrFileList, name)) {
            /* There was an error */
            thisPtr->cwSetStatusText(NULL);
            bBreak=TRUE;/* The memory is freed in the function */
          }
        }/* else if(ulFlags & IDMK_SHADOWSINROOTONLY) */
      }

      if(!bBreak) {
        sprintf(thisPtr->chrStatusText,"");
        thisPtr->cwSetStatusText(NULL);

        /* File collecting was successful */
        ulFlags=thisPtr->cwQueryCDTypeFlags();
        /* Add cdrecord command */
        strcat(text," | ");
        /* CDRecord path */
        strcat(text,chrCDRecord);
        strcat(text,"  ");
        chrPtrParam=strrchr(text,' ');
        
        /* Add cdrecord options */
        buildDataWriteParam(thisPtr, chrPtrParam, iSpeedLocal, name);
        
        /* Check if we perform a dummy write */
        if(thisPtr->sDummy==1)
          strcat(text," -dummy");// Dummy write
        
        //  if(!(ulFlags & IDCDT_FIRSTSESSION)) {
        if((thisPtr->cwQueryPreviousStartSector()!=thisPtr->cwQueryNextStartSector()) && (ulFlags & IDCDT_MULTISESSION)) {
          /* We are building the second image of a multisession disk */
          strcat(text," -waiti");/* New switch with V1.8.1a01 of cdrecord */
        }
        /* Tell cdrecord to read from stdin */
        strcat(text," -");
        
        /* This PM wrapper checks the image- and CD size and controls the status
           window */ 
        sprintf(name,"\"%s\" \"", chrFileList);/* The filelist file if follwing all shadows */
        /* Get parameter filename */
        if(buildBinFileName(chrFileList)) {
          strcat(name, chrFileList); 
          strcat(name, "\"");
          if((file=fopen(chrFileList,"wb"))!=NULL) {
            char progTitle[255];

            fwrite(text, sizeof(char), SHAREDMEM_SIZE, file);  
            fclose(file);
            
            /* Text: "Writing on the fly..." */
            getMessage(chrFileList, IDSTR_ONTHEFLYWRITING, sizeof(chrFileList), hDataResource,HWND_DESKTOP);
            thisPtr->cwSetStatusText(chrFileList);
            
            ulSize=sizeof(chrFileList);
            thisPtr->wpQueryRealName(chrFileList, &ulSize, TRUE);

            /* "Writing on the fly" */
            getMessage(progTitle, IDSTRD_ONTHEFLYTITLE, sizeof(progTitle), hDataResource, HWND_DESKTOP);
            launchPMWrapper( name, chrFileList,"pmthefly.exe", progTitle);
            thisPtr->cwEnableWriteControls(TRUE);
            DosSleep(5000);/* Show the writing message in the status line for 5 secs */
          }/* file */
        }/* end of if(buildBinFileName(chrFileList)) */
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  /* Free cmd-line mem */
  if(text)
    thisPtr->wpFreeMem(text);
  
  thisPtr->cwEnableWriteControls(TRUE);
  sprintf(thisPtr->chrStatusText," ");
  thisPtr->cwSetStatusText(NULL); 
}

void _Optlink cdSizeThreadFunc(void *arg)
{
  HWND hwndMkisofs;
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  char text[CCHMAXPATH+100];
  CWDataFolder * thisPtr;

  thisPtr=(CWDataFolder*)arg;
    
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {
      hwndMkisofs=WinCreateWindow(HWND_OBJECT,WC_STATIC,"cdSizeObj",0,0,0,0,0,NULL,HWND_BOTTOM,12343,NULL,NULL);
      if(hwndMkisofs) {
        WinSubclassWindow(hwndMkisofs,&mkisofsObjectProc);
        /* Window created. */             
        WinPostMsg(hwndMkisofs,atomStartWrite,MPFROMP(thisPtr),MPFROMLONG(ACKEY_CDSIZE));
        thisPtr->cwEnableWriteControls(FALSE);
        while(WinGetMsg(hab,&qmsg,(HWND)NULL,0,0))
          WinDispatchMsg(hab,&qmsg);
        WinDestroyWindow(hwndMkisofs);
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  thisPtr->cwEnableWriteControls(TRUE);
  sprintf(thisPtr->chrStatusText,"");
}

void _Optlink dvdSizeThreadFunc(void *arg)
{
  HWND hwndMkisofs;
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  char text[CCHMAXPATH+100];
  CWDataFolder * thisPtr;

  thisPtr=(CWDataFolder*)arg;
    
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {
      hwndMkisofs=WinCreateWindow(HWND_OBJECT,WC_STATIC,"cdSizeObj",0,0,0,0,0,NULL,HWND_BOTTOM,12343,NULL,NULL);
      if(hwndMkisofs) {
        WinSubclassWindow(hwndMkisofs,&mkisofsObjectProc);
        /* Window created. */             
        WinPostMsg(hwndMkisofs, atomStartWrite, MPFROMP(thisPtr), MPFROMLONG(ACKEY_DVDSIZE));
        thisPtr->cwEnableWriteControls(FALSE);
        while(WinGetMsg(hab,&qmsg,(HWND)NULL,0,0))
          WinDispatchMsg(hab,&qmsg);
        WinDestroyWindow(hwndMkisofs);
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  thisPtr->cwEnableWriteControls(TRUE);
  sprintf(thisPtr->chrStatusText,"");
}

/***************************************************************************
 *                                                                         *
 * This Proc handles the calculation of the playtime of waves and mp3s.    *
 * It's the window procedure of an object window. For every wave and mp3   *
 * a VIO-helper is started which checks the format. If the format is       *
 * 16 bit, 44,1kHz the helper returns the size in bytes otherwise zero.    *
 *                                                                         *
 * The calculation is done in a sepatate thread started when opening the   *
 * folder or dropping files etc. The new thread is initiated by the        *
 * status line frame control.                                              *
 *                                                                         *
 ***************************************************************************/
#if 0
MRESULT EXPENTRY playTimeObjectProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH*2];
  char title[CCHMAXPATH];
  char *textPtr;
  char *textPtr2; 

  class playTimeInfo {
  public:
    CWAudioFolder *thisPtr;
    HWND  hwndCnr;
    PMINIRECORDCORE mrc;
    int iNumWrongFormat;
  };

  playTimeInfo * ptiInfo;
  HWND hwndFrame;
  WPObject * contentObject;
  char name[CCHMAXPATH];
  ULONG ulNameSize;
  ULONG ulSize;
  ULONG ulFlags;

  switch (msg)
    {
    case WM_APPTERMINATENOTIFY:
      ptiInfo=(playTimeInfo*)WinQueryWindowULong(hwnd,QWL_USER);/* Get object ptr */
      if(!ptiInfo || !somIsObj(ptiInfo->thisPtr)) {
        /* Shouldn't come to this code! */
        WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
        break;
      }

      switch(LONGFROMMP(mp1)) {
      case ACKEY_PLAYTIME:
        if(!LONGFROMMP(mp2)) /* The track is of wrong format. */
          ptiInfo->iNumWrongFormat++;
        ptiInfo->thisPtr->ulTrackSize+=LONGFROMMP(mp2);
        if(ptiInfo->mrc  && (int)ptiInfo->mrc!=-1){ 
          /* Get next container item */
          ptiInfo->mrc=(PMINIRECORDCORE)WinSendMsg(ptiInfo->hwndCnr,CM_QUERYRECORD,MPFROMP(ptiInfo->mrc),
                                                   MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
          if(ptiInfo->mrc && (int)ptiInfo->mrc!=-1){ 
            /* Get wps-object-ptr. from container item */
            contentObject=(WPObject*)OBJECT_FROM_PREC(ptiInfo->mrc);//Get object
            ulNameSize=sizeof(name);
            if(contentObject) {
              /* Get file system object or NULL */
              contentObject=ptiInfo->thisPtr->cwGetFileSystemObject(contentObject);
            }
            if(contentObject){
              /* It's a file system object */
              /* Check, if it's a mp3 file */
              if(ptiInfo->thisPtr->cwIsMp3File(contentObject)){
                ulFlags=ptiInfo->thisPtr->cwQueryWriteFlags();
                if(checkFileExists(chrMpg123Path) && !(ulFlags&IDWF_DAO)  ||
                   (ulFlags & IDWF_DAO && iMp3Decoder==3)) {
                  // Only if mp3 decoder is avaiable and DAO not selected */
                  /* Yes, query the full path */
                  ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                  /* Launch helper. First helper is started below. */
                    if(!audioHlpStartMp3Query(name, hwnd))
                      WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);/* The helper isn't avaiable or can't be started */
                  return FALSE;
                }
              } /* End of cwIsMp3File(contentObject) */
              else if(ptiInfo->thisPtr->cwIsWaveFile(contentObject)){
                /* Yes, query the full path */
                ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                /* Launch helper */
                sprintf(text," \"%s\"",name);
                if(launchWrapper(text, "", hwnd,"waveinfo.exe","Query wavefile size")==-1)
                  WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);/* The helper isn't avaiable or can't be started */ 
                return FALSE;
              } /* end of else if(ptiInfo->thisPtr->cwIsWaveFile(contentObject)) */
            }/* end of if(contentObject) */           
          }// end of if(mrc)
        }// end of if(mrc)
        if( ptiInfo->mrc && (int)ptiInfo->mrc!=-1) {
          /* We have more container items so continue */
          WinPostMsg(hwnd,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PLAYTIME),MPFROMLONG(0));
          return FALSE;
        }
        ptiInfo->thisPtr->cwSetStatusTime(ptiInfo->thisPtr->ulTrackSize);
        delete(ptiInfo);
        WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
        break;
      default:
        break;
      }/* switch switch(LONGFROMMP(mp1)) */
      break;
    default:
      if(msg == atomStartWrite) {
        ptiInfo=new(playTimeInfo);        
        if(!ptiInfo) {
          /* Error: quit */
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          return 0;
        }
        ptiInfo->thisPtr=(CWAudioFolder*)PVOIDFROMMP(mp1);
        WinSetWindowULong(hwnd,QWL_USER,(ULONG)ptiInfo);/* Save object ptr */
        if(!somIsObj(ptiInfo->thisPtr)) {
          /* Error: quit */
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          return 0;
        }

        switch(LONGFROMMP(mp2)) {   
        case ACKEY_PLAYTIME:
          /* Set staus line text.                          */
          /* Name: "Calculating Play time. Please wait..." */
          getMessage(name, IDSTRA_CALCPLAYTIME, sizeof(name),  hAudioResource, 
                     ptiInfo->thisPtr->hwndStatusFrameCtl);
          ptiInfo->thisPtr->cwSetStatusText(name);

          /* Disable controls in write dialog while calculating the time */
          ptiInfo->thisPtr->cwEnableWriteControls(FALSE);
          /* Get hwnd of folder container */ 
          hwndFrame=WinQueryWindow(ptiInfo->thisPtr->hwndStatusFrameCtl,QW_PARENT);
          ptiInfo->hwndCnr=WinWindowFromID(hwndFrame,FID_CLIENT);

          /* Set total size to zero while calculating */
          ulSize=0;
          ptiInfo->thisPtr->ulTrackSize=ulSize;

          /* Start helpers which calculate playtime of mp3 or wave files. */
          if(ptiInfo->hwndCnr){ /* Catch error */
            /* Get first container item of our folder */
            ptiInfo->mrc=(PMINIRECORDCORE)WinSendMsg(ptiInfo->hwndCnr,CM_QUERYRECORD,NULL, 
                                                     MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
            if(ptiInfo->mrc){ 
              while(ptiInfo->mrc) {
                /* Get wps-object-ptr. from container item */
                contentObject=(WPObject*)OBJECT_FROM_PREC(ptiInfo->mrc);//Get object
                ulNameSize=sizeof(name);
                if(contentObject) {
                  /* Get file system object or NULL */
                  contentObject=ptiInfo->thisPtr->cwGetFileSystemObject(contentObject);
                }
                if(contentObject){
                  /* It's a file system object */
                  /* Check, if it's a mp3 file */
                  if(ptiInfo->thisPtr->cwIsMp3File(contentObject)){
                    ulFlags=ptiInfo->thisPtr->cwQueryWriteFlags();
                    if(checkFileExists(chrMpg123Path) && !(ulFlags&IDWF_DAO) ||
                       (ulFlags & IDWF_DAO && iMp3Decoder==3)) {
                      // Only if mp3 decoder is avaiable and DAO not selected */
                      /* Yes, query the full path */
                      ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                      /* Launch helper */
                        if(!audioHlpStartMp3Query(name, hwnd))
                          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);/* The helper isn't avaiable or can't be started */
                      return FALSE;
                    }// end of if(strlen(chrMpg123Path))
                  } /* End of cwIsMp3File(contentObject) */
                  else if(ptiInfo->thisPtr->cwIsWaveFile(contentObject)){
                    /* Yes, query the full path */
                    ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                    /* Launch helper */
                    sprintf(text," \"%s\"",name);
                    if(launchWrapper(text, "", hwnd,"waveinfo.exe","Query wavefile size")==-1)
                      WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);/* The helper isn't avaiable or can't be started */ 
                    return FALSE;
                  } /* end of else if(ptiInfo->thisPtr->cwIsWaveFile(contentObject)) */
                }/* end of if(contentObject) */           
                /* Get next container item */
                ptiInfo->mrc=(PMINIRECORDCORE)WinSendMsg(ptiInfo->hwndCnr,CM_QUERYRECORD,MPFROMP(ptiInfo->mrc),
                                                         MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
              }// end of while(mrc)
            }// end of if(mrc)
          }

          ptiInfo->thisPtr->cwSetStatusTime(ptiInfo->thisPtr->ulTrackSize);
          delete(ptiInfo);
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          return FALSE;
        default:
          break;
        }/* switch */
      }/* if(msg == atomStartWrite) */
      return WinDefWindowProc( hwnd, msg, mp1, mp2);
    }
  return FALSE;
}
#endif

MRESULT EXPENTRY playTimeObjectProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH*2];
  char title[CCHMAXPATH];
  char *textPtr;
  char *textPtr2; 

  class playTimeInfo {
  public:
    CWAudioFolder *thisPtr;
    HWND  hwndCnr;
    PMINIRECORDCORE mrc;
    int iNumWrongFormat;
  };

  playTimeInfo * ptiInfo;
  HWND hwndFrame;
  WPObject * contentObject;
  char name[CCHMAXPATH];
  ULONG ulNameSize;
  ULONG ulSize;
  ULONG ulFlags;

  switch (msg)
    {
    case WM_APPTERMINATENOTIFY:
      ptiInfo=(playTimeInfo*)WinQueryWindowULong(hwnd,QWL_USER);/* Get object ptr */
      if(!ptiInfo || !somIsObj(ptiInfo->thisPtr)) {
        /* Shouldn't come to this code! */
        WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
        break;
      }

      switch(LONGFROMMP(mp1)) {
      case ACKEY_PLAYTIME:
        if(!LONGFROMMP(mp2)) /* The track is of wrong format. */
          ptiInfo->iNumWrongFormat++;
        ptiInfo->thisPtr->ulTrackSize+=LONGFROMMP(mp2);

        if(ptiInfo->mrc  && (int)ptiInfo->mrc!=-1){ 
          /* Get next container item */
          ptiInfo->mrc=(PMINIRECORDCORE)WinSendMsg(ptiInfo->hwndCnr,CM_QUERYRECORD,MPFROMP(ptiInfo->mrc),
                                                   MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
          if(ptiInfo->mrc && (int)ptiInfo->mrc!=-1){ 
            /* Get wps-object-ptr. from container item */
            contentObject=(WPObject*)OBJECT_FROM_PREC(ptiInfo->mrc);//Get object
            ulNameSize=sizeof(name);
            if(somIsObj(contentObject)) {
              /* Get file system object or NULL */
              contentObject=ptiInfo->thisPtr->cwGetFileSystemObject(contentObject);
            }
            if(somIsObj(contentObject)){
              /* It's a file system object */
              /* Check, if it's a mp3 file */
              //              if(ptiInfo->thisPtr->cwIsMp3File(contentObject)){
              if(ptiInfo->thisPtr->cwIsMp3File(contentObject) ||
                 (ptiInfo->thisPtr->cwIsAudioFile(contentObject) && iMp3Decoder==IDKEY_USEMMIOMP3) ) {
                ulFlags=ptiInfo->thisPtr->cwQueryWriteFlags();
                if(checkFileExists(globalData.chrMpg123Path) && !(ulFlags&IDWF_DAO)  ||
                   (ulFlags & IDWF_DAO && iMp3Decoder==3)) {
                  // Only if mp3 decoder is avaiable and DAO not selected */
                  /* Yes, query the full path */
                  ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                  /* Launch helper. First helper is started below. */
                    if(!audioHlpStartMp3Query(name, hwnd))
                      WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);/* The helper isn't avaiable or can't be started */
                  return FALSE;
                }
              } /* End of cwIsMp3File(contentObject) */
              else if(ptiInfo->thisPtr->cwIsWaveFile(contentObject, FALSE)){
                /* Yes, query the full path */
                ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                /* Launch helper */
                sprintf(text," \"%s\"",name);
                if(launchWrapper(text, "", hwnd,"waveinfo.exe","Query wavefile size")==-1)
                  WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);/* The helper isn't avaiable or can't be started */ 
                return FALSE;
              } /* end of else if(ptiInfo->thisPtr->cwIsWaveFile(contentObject)) */
            }/* end of if(contentObject) */           
          }// end of if(mrc)
        }// end of if(mrc)
        if( ptiInfo->mrc && (int)ptiInfo->mrc!=-1) {
          /* We have more container items so continue */
          WinPostMsg(hwnd,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PLAYTIME),MPFROMLONG(0));
          return FALSE;
        }
        ptiInfo->thisPtr->cwSetStatusTime(ptiInfo->thisPtr->ulTrackSize);
        delete(ptiInfo);
        WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
        break;
      default:
        break;
      }/* switch switch(LONGFROMMP(mp1)) */
      break;
    default:
      if(msg == atomStartWrite) {
        ptiInfo=new(playTimeInfo);        
        if(!ptiInfo) {
          /* Error: quit */
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          return 0;
        }
        ptiInfo->thisPtr=(CWAudioFolder*)PVOIDFROMMP(mp1);
        WinSetWindowULong(hwnd,QWL_USER,(ULONG)ptiInfo);/* Save object ptr */
        if(!somIsObj(ptiInfo->thisPtr)) {
          /* Error: quit */
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          return 0;
        }

        switch(LONGFROMMP(mp2)) {   
        case ACKEY_PLAYTIME:
          /* Set staus line text.                          */
          /* Name: "Calculating Play time. Please wait..." */
          getMessage(name, IDSTRA_CALCPLAYTIME, sizeof(name),  hAudioResource, 
                     ptiInfo->thisPtr->hwndStatusFrameCtl);
          ptiInfo->thisPtr->cwSetStatusText(name);

          /* Disable controls in write dialog while calculating the time */
          ptiInfo->thisPtr->cwEnableWriteControls(FALSE);
          /* Get hwnd of folder container */ 
          hwndFrame=WinQueryWindow(ptiInfo->thisPtr->hwndStatusFrameCtl,QW_PARENT);
          ptiInfo->hwndCnr=WinWindowFromID(hwndFrame,FID_CLIENT);

          /* Set total size to zero while calculating */
          ulSize=0;
          ptiInfo->thisPtr->ulTrackSize=ulSize;

          /* Start helpers which calculate playtime of mp3 or wave files. */
          if(ptiInfo->hwndCnr){ /* Catch error */
            /* Get first container item of our folder */
            ptiInfo->mrc=(PMINIRECORDCORE)WinSendMsg(ptiInfo->hwndCnr,CM_QUERYRECORD,NULL, 
                                                     MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
            if(ptiInfo->mrc){ 
              while(ptiInfo->mrc) {
                /* Get wps-object-ptr. from container item */
                contentObject=(WPObject*)OBJECT_FROM_PREC(ptiInfo->mrc);//Get object
                if(somIsObj(contentObject)) {
                  /* Get file system object or NULL */
                  contentObject=ptiInfo->thisPtr->cwGetFileSystemObject(contentObject);
                }
                ulNameSize=sizeof(name);
                if(somIsObj(contentObject)){
                  /* It's a file system object */
                  /* Check, if it's an mp3file or an audio file if MMIOMP3 is selected */
                  if(ptiInfo->thisPtr->cwIsMp3File(contentObject) ||
                     (ptiInfo->thisPtr->cwIsAudioFile(contentObject) && iMp3Decoder==IDKEY_USEMMIOMP3) ) {
              //                  if(ptiInfo->thisPtr->cwIsMp3File(contentObject)){
                    ulFlags=ptiInfo->thisPtr->cwQueryWriteFlags();
                    if(checkFileExists(globalData.chrMpg123Path) && !(ulFlags&IDWF_DAO) ||
                       (ulFlags & IDWF_DAO && iMp3Decoder==3)) {
                      // Only if mp3 decoder is avaiable and DAO not selected */
                      /* Yes, query the full path */
                      ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                      /* Launch helper */
                        if(!audioHlpStartMp3Query(name, hwnd))
                          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);/* The helper isn't avaiable or can't be started */
                      return FALSE;
                    }// end of if(strlen(chrMpg123Path))
                  } /* End of cwIsMp3File(contentObject) */
                  else if(ptiInfo->thisPtr->cwIsWaveFile(contentObject, FALSE)){
                    /* Yes, query the full path */
                    ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                    /* Launch helper */
                    sprintf(text," \"%s\"",name);
                    if(launchWrapper(text, "", hwnd,"waveinfo.exe","Query wavefile size")==-1)
                      WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);/* The helper isn't avaiable or can't be started */ 
                    return FALSE;
                  } /* end of else if(ptiInfo->thisPtr->cwIsWaveFile(contentObject)) */
                }/* end of if(contentObject) */           
                /* Get next container item */
                ptiInfo->mrc=(PMINIRECORDCORE)WinSendMsg(ptiInfo->hwndCnr,CM_QUERYRECORD,MPFROMP(ptiInfo->mrc),
                                                         MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
              }// end of while(mrc)
            }// end of if(mrc)
          }

          ptiInfo->thisPtr->cwSetStatusTime(ptiInfo->thisPtr->ulTrackSize);
          delete(ptiInfo);
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          return FALSE;
        default:
          break;
        }/* switch */
      }/* if(msg == atomStartWrite) */
      return WinDefWindowProc( hwnd, msg, mp1, mp2);
    }
  return FALSE;
}

//extern somTD_CWAudio_cwmmQueryTrackInfo methodPtr;

/* This thread calculates the playtime of all wave and mp3 files */
void _Optlink playTimeThreadFunc(void *arg)
{
  HWND hwndPlayTime;
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  char text[CCHMAXPATH];
  CWAudioFolder * thisPtr;
  SOMClass *cwAudioClass;
  SOMClass *mmWaveClass;

  //  DosBeep(5000,1000);
  thisPtr=(CWAudioFolder*)arg;
  if(!somIsObj(thisPtr))
    return;

  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {

      TRY_LOUD(AUDIOFOLDER_PLAYTIME) {      
        /* Set staus line text.                          */
        /* Name: "Calculating Play time. Please wait..." */
        getMessage(text, IDSTRA_CALCPLAYTIME, sizeof(text),  hAudioResource, thisPtr->hwndStatusFrameCtl);
        thisPtr->cwSetStatusText(text);

        /* Wait for folder to populate */
        while(!( (thisPtr->wpQueryFldrFlags() & FOI_POPULATEDWITHALL) || thisPtr->bPopulated))
          DosSleep(100);

        thisPtr->bPopulated=TRUE;

        /* Check if audio classes are installed and method available */
        cwAudioClass=queryCWAudioClass();
        if(!somIsObj(cwAudioClass))
          cwAudioClass=queryMMAudioClass();
        mmWaveClass=cwGetSomClass("MMWAV");
        if(!somIsObj(mmWaveClass))
          mmWaveClass=cwGetSomClass("CWWAV");
        if(!( /*somIsObj(cwGetSomClass("CW_Audio")) */
             //&& mmWaveClass
             cwAudioClass
             && cwmmQueryCWAudioTrackInfoMethodPtr())/* Load message pointer */
           )
          {
            /* CWAudio class not installed so use the external programs */
            hwndPlayTime=WinCreateWindow(HWND_OBJECT,WC_STATIC,"playTimeObj",
                                         0,0,0,0,0,NULL,HWND_BOTTOM,12343,NULL,NULL);
            if(hwndPlayTime) {
              WinSubclassWindow(hwndPlayTime,&playTimeObjectProc);
              /* Window created. */             
              WinPostMsg(hwndPlayTime,atomStartWrite,MPFROMP(thisPtr),MPFROMLONG(ACKEY_PLAYTIME));
              while(WinGetMsg(hab,&qmsg,(HWND)NULL,0,0))
                WinDispatchMsg(hab,&qmsg);
              WinDestroyWindow(hwndPlayTime);
            }
          }
        else
          {
            WPObject *wpObject;
            WPObject *fsObject;
            ULONG ulSecs=0;
            ULONG ulFlags=thisPtr->cwQueryWriteFlags();            
            /* Disable checkboxes during calculation */
            thisPtr->cwEnableWriteControls(FALSE);

            /* Use the playtime information from the classes */
            /* Get first object in the container */
            wpObject=thisPtr->wpQueryContent(NULLHANDLE, QC_FIRST);
            while(wpObject)
              {
                /* Follow shadows to the filesystem object (if any) */
                if((fsObject=thisPtr->cwGetFileSystemObject(wpObject))!=NULLHANDLE){/* Filesystem object or NULL */
                  /* Check if the file is an audio file */
                  if(fsObject->somIsA(cwAudioClass)) {
                    /* It's an audio file. Now check if channel, rate etc. are right. */
                    if((globalData.cwmmQueryTrackInfoMthdPtr)((CWAudio*)fsObject, NULLHANDLE, 0, IDINFO_CHANNELS)==2
                       && (globalData.cwmmQueryTrackInfoMthdPtr)((CWAudio*)fsObject, NULLHANDLE, 0, IDINFO_BPS)==16
                       && (globalData.cwmmQueryTrackInfoMthdPtr)((CWAudio*)fsObject, NULLHANDLE, 0, IDINFO_SAMPLERATE)==44100)
                      {
                        if((ulFlags & IDWF_DAO) && (iMp3Decoder!=IDKEY_USEMMIOMP3)) {
                          /* DAO writing ->only use waves if MMIO procedures are not provided */
                          if(fsObject->somIsInstanceOf(mmWaveClass))
                            /* Query playtime in secs */
                            ulSecs+=(globalData.cwmmQueryTrackInfoMthdPtr)((CWAudio*)fsObject,
                                                                           NULLHANDLE, 0, IDINFO_PLAYTIME);
                        }
                        else
                          /* Query playtime in secs */
                          ulSecs+=(globalData.cwmmQueryTrackInfoMthdPtr)((CWAudio*)fsObject,
                                                                         NULLHANDLE, 0, IDINFO_PLAYTIME);
                      }/* Format check */
                  }/* mmAudioClass */
                }
                wpObject=thisPtr->wpQueryContent(wpObject, QC_NEXT);                                
              }/* while */
            thisPtr->ulTrackSize=ulSecs*(2*2*44100);
            thisPtr->cwSetStatusTime(thisPtr->ulTrackSize);              
          }
      }  
      CATCH(AUDIOFOLDER_PLAYTIME)
        {
        } END_CATCH;
        /* Reenable write checkboxes */
        thisPtr->cwEnableWriteControls(TRUE);
        WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
}


/* This thread handles the CDTools-dialog */
void _Optlink toolsThreadFunc (void *arg)
{
  HWND hwndTools;
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  CWAudioFolder *thisPtr;
  
  thisPtr=(CWAudioFolder*)arg;    //Pointer auf CWAudioFolder-Object
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {
      hwndTools=WinLoadDlg(HWND_DESKTOP,HWND_DESKTOP,CDToolsDlgProc,hAudioResource,IDDLG_CDRTOOLS,thisPtr);
      if(hwndTools) {
        /* CDR tools window created */
        while(WinGetMsg(hab,&qmsg,(HWND)NULL,0,0))
          WinDispatchMsg(hab,&qmsg);
        WinDestroyWindow(hwndTools);
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
}

MRESULT EXPENTRY coverObjectProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH*2];
  char title[CCHMAXPATH];
  char *textPtr;
  char *textPtr2; 
  CWAudioFolder* thisPtr;
  HOBJECT hObject;

  class coverInfo {
  public:
    CWAudioFolder *thisPtr;
    HWND  hwndCnr;
    PMINIRECORDCORE mrc;   
    int iTrackNum;
  };

  coverInfo * ptiInfo;
  HWND hwndFrame;
  WPObject * contentObject;
  char name[CCHMAXPATH];
  char folderName[CCHMAXPATH];
  ULONG ulNameSize;
  ULONG ulSize;
  ULONG ulFlags;

  switch (msg)
    {
    case WM_APPTERMINATENOTIFY:
      ptiInfo=(coverInfo*)WinQueryWindowULong(hwnd,QWL_USER);/* Get object ptr */
      if(!ptiInfo || !somIsObj(ptiInfo->thisPtr)) {
        WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
        break;
      }
      switch(LONGFROMMP(mp1)) {
      case ACKEY_CREATECOVER:
        ulNameSize=sizeof(folderName);
        /* Query name of this folder */
        if(!ptiInfo->thisPtr->wpQueryRealName(folderName,&ulNameSize,TRUE)) {
          sprintf(text,"Error while quering the foldername!");  
          WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, text,"Cover creation",
                          0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
          delete(ptiInfo);
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          return FALSE;/* There's an error */
        }

        if(ptiInfo->iTrackNum==0) {
          /* Skript is built, end this thread */          
          delete(ptiInfo);
          sprintf(name,"%s\\back.cwx",folderName);
          /* Text: "Should P>G Pro be started to edit the cover? (P>G Pro must be installed and
           *.cwx files be associated with it."
           Title:  "Audio-CD-Creator"
             */
          if(messageBox( text, IDSTRA_STARTPGPROQUESTION , sizeof(text),
                         title, IDSTR_AUDIOCDCREATOR, sizeof(title),
                         hAudioResource, HWND_DESKTOP, MB_YESNO | MB_ICONQUESTION|MB_MOVEABLE)==MBID_YES) {
            if((hObject=WinQueryObject(name))!=NULL) {
              WinOpenObject(hObject,OPEN_DEFAULT,TRUE); /* Open P>G Pro */
            } 
          }
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          break;/* We are ready. End this thread. */
        }
        
        if(ptiInfo->mrc  && (int)ptiInfo->mrc!=-1){ 
          /* Get next container item */
          ptiInfo->mrc=(PMINIRECORDCORE)WinSendMsg(ptiInfo->hwndCnr,CM_QUERYRECORD,MPFROMP(ptiInfo->mrc),
                                                   MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
          if(ptiInfo->mrc && (int)ptiInfo->mrc!=-1){ 
            /* Get wps-object-ptr. from container item */
            contentObject=(WPObject*)OBJECT_FROM_PREC(ptiInfo->mrc);//Get object
            ulNameSize=sizeof(name);
            if(contentObject) {
              /* Get file system object or NULL */
              contentObject=ptiInfo->thisPtr->cwGetFileSystemObject(contentObject);
            }
            if(contentObject){
              /* It's a file system object */
              
              /* Check, if it's a wave file */
              if(ptiInfo->thisPtr->cwIsWaveFile(contentObject, FALSE)){
                /* Yes, query the full path */
                ulNameSize=sizeof(name);
                ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                ptiInfo->iTrackNum+=1; 
                /* Launch helper */
                sprintf(text,"\"%s\" \"%s\" \"%s\" %d", chrInstallDir, name,folderName,ptiInfo->iTrackNum);
                launchWrapper(text, "", hwnd,"cover.exe","Create cover"); 
                return FALSE;
              }
              
              /* Check if it's a MP3 */
              ulFlags=ptiInfo->thisPtr->cwQueryWriteFlags();
              if(strlen(globalData.chrMpg123Path)&&!(ulFlags&IDWF_DAO)) {// Only if mpg123 avaiable and DAO not selected */
                if(ptiInfo->thisPtr->cwIsMp3File(contentObject)){
                  ptiInfo->iTrackNum+=1;
                  /* Yes, query the full path */
                  ulNameSize=sizeof(name);
                  ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                  /* Launch helper */
                  sprintf(text,"\"%s\" \"%s\" \"%s\" %d", chrInstallDir, name,folderName,ptiInfo->iTrackNum);
                  launchWrapper(text, "", hwnd,"cover.exe","Create cover"); 
                  return FALSE;
                } /* End of cwIsMp3File(contentObject) */           
              }
            }/* end of if(contentObject) */           
          }// end of if(mrc)
          /* We have more container items so continue */
          WinPostMsg(hwnd,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_CREATECOVER),MPFROMLONG(0));
          return FALSE;
        }// end of if(mrc)
        /* Terminate the script by sending track num 0 */
        /* Launch helper */
        ptiInfo->iTrackNum=0; /* Mark that we terminating the script */
        sprintf(text,"\"%s\" \"Terminating cover script\" \"%s\" %d", chrInstallDir, folderName,0);
        launchWrapper(text, "", hwnd,"cover.exe","Create cover"); 
        ptiInfo->thisPtr->cwForceStatusUpdate();  /* Show play time again */
        return FALSE;
      default:
        break;
      }/* switch */
      break;
    default:
      if(msg == atomStartWrite) {
        ptiInfo=new(coverInfo);        
        if(!ptiInfo) {
          /* Error: quit */
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          return 0;
        }
        ptiInfo->thisPtr=(CWAudioFolder*)PVOIDFROMMP(mp1);
        WinSetWindowULong(hwnd,QWL_USER,(ULONG)ptiInfo);/* Save object ptr */
        if(!ptiInfo->thisPtr) {
          /* Error: quit */
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          return 0;
        }
        ptiInfo->iTrackNum=1;
        switch(LONGFROMMP(mp2)) {   
        case ACKEY_CREATECOVER:
          /* Name: "Creating cover. Please wait..." */
          getMessage(name, IDSTRA_CREATECOVER, sizeof(name),  hAudioResource, ptiInfo->thisPtr->hwndStatusFrameCtl);          
          ptiInfo->thisPtr->cwSetStatusText(name);

          /* Get hwnd of folder container */ 
          hwndFrame=WinQueryWindow(ptiInfo->thisPtr->hwndStatusFrameCtl,QW_PARENT);
          ptiInfo->hwndCnr=WinWindowFromID(hwndFrame,FID_CLIENT);
          if(ptiInfo->hwndCnr){ /* Catch error */
            /* Get first container item of our folder */
            ptiInfo->mrc=(PMINIRECORDCORE)WinSendMsg(ptiInfo->hwndCnr,CM_QUERYRECORD,NULL,
                                                     MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
            if(ptiInfo->mrc && (int)ptiInfo->mrc!=-1){ 
              /* Query flags to check if we are in DAO mode */
              ulFlags=ptiInfo->thisPtr->cwQueryWriteFlags();
              while(ptiInfo->mrc) {
                /* Get wps-object-ptr. from container item */
                contentObject=(WPObject*)OBJECT_FROM_PREC(ptiInfo->mrc);//Get object
                ulNameSize=sizeof(name);
                if(contentObject) {
                  /* Get file system object or NULL */
                  contentObject=ptiInfo->thisPtr->cwGetFileSystemObject(contentObject);
                }
                if(contentObject){
                  /* It's a file system object */
                  ulNameSize=sizeof(folderName);
                  /* Query name of this folder */
                  if(!ptiInfo->thisPtr->wpQueryRealName(folderName,&ulNameSize,TRUE)) {
                    sprintf(text,"Error while quering the foldername!");  
                    WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, text,"Cover creation",
                                    0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
                    delete(ptiInfo);
                    WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
                    return FALSE;/* There's an error */
                  }
                  
                  /* Check, if it's a wave file */
                  if(ptiInfo->thisPtr->cwIsWaveFile(contentObject, FALSE)){
                    /* Yes, query the full path */
                    ulNameSize=sizeof(name);
                    ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                    /* Launch helper */
                    sprintf(text,"\"%s\" \"%s\" \"%s\" %d", chrInstallDir, name,folderName,ptiInfo->iTrackNum);
                    launchWrapper(text, "", hwnd,"cover.exe","Create cover"); 
                    return FALSE;
                  }

                  if(strlen(globalData.chrMpg123Path)&&!(ulFlags&IDWF_DAO)) {// Only if mpg123 avaiable and DAO not selected */
                    if(ptiInfo->thisPtr->cwIsMp3File(contentObject)){
                      /* Yes, query the full path */
                      ulNameSize=sizeof(name);
                      ((WPFileSystem*)contentObject)->wpQueryRealName(name,&ulNameSize,TRUE);
                      /* Launch helper */
                      sprintf(text,"\"%s\" \"%s\" \"%s\" %d", chrInstallDir, name,folderName,ptiInfo->iTrackNum);
                      launchWrapper(text, "", hwnd,"cover.exe","Create cover"); 
                      return FALSE;
                    } /* End of cwIsMp3File(contentObject) */           
                  }
                }/* end of if(contentObject) */           
                /* Get next container item */
                ptiInfo->mrc=(PMINIRECORDCORE)WinSendMsg(ptiInfo->hwndCnr,CM_QUERYRECORD,MPFROMP(ptiInfo->mrc),
                                                         MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
              }// end of while(mrc)
            }// end of if(mrc)
          } // end of if(ptiInfo->hwndCnr)

          delete(ptiInfo);
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
          return FALSE;
        default:
          break;
        }/* switch */
      }/* if(msg == atomStartWrite) */
      return WinDefWindowProc( hwnd, msg, mp1, mp2);
    }
  return FALSE;
}

void _Optlink coverThreadFunc (void *arg)
{
  HWND hwnd;
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  CWAudioFolder *thisPtr;
  
  thisPtr=(CWAudioFolder*)arg;    //Pointer auf CWAudioFolder-Object
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {
      hwnd=WinCreateWindow(HWND_OBJECT,WC_STATIC,"coverObj",0,0,0,0,0,NULL,HWND_BOTTOM,12343,NULL,NULL);
      if(hwnd) {
        WinSubclassWindow(hwnd,&coverObjectProc);
        /* Window created. */             
        WinPostMsg(hwnd,atomStartWrite,MPFROMP(thisPtr),MPFROMLONG(ACKEY_CREATECOVER));
        while(WinGetMsg(hab,&qmsg,(HWND)NULL,0,0))
          WinDispatchMsg(hab,&qmsg);
        WinDestroyWindow(hwnd);
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
}


void _Optlink resetArchiveBitThreadFunc(void *arg)
{
  CWDataFolder *thisPtr;

  thisPtr=(CWDataFolder*)arg;    //Pointer auf CWDataFolder-Object

  /* Lower priority or the GUI will become slow while querying the tree */
  DosSetPriority(PRTYS_THREAD,PRTYC_IDLETIME,15,0);

  if(!resetArchiveBit(thisPtr, NULL ,"", 0, thisPtr->hwndStatusCntrl, thisPtr)) {
    /* There was an error */
  }
}

/* This thread handles the about dialog */
void _Optlink aboutThreadFunc (void *arg)
{
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  CWAudioFolder *thisPtr;
  
  thisPtr=(CWAudioFolder*)arg;    //Pointer auf CWAudioFolder-Object
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {
      /* Call the method from CWProgFolder */
      thisPtr->cwShowAboutDlg(hAudioResource,IDDLG_ABOUT);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
}








