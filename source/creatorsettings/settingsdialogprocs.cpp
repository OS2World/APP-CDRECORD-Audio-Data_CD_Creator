/*
 * This file is (C) Chris Wohlgemuth 1999-2004
 * It is part of the Audio/Date-CD-Creator package
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
#define INCL_GPIBITMAPS

#include "audiofolder.hh"
#include "audiofolderhelp.h"
#include <stdio.h>
#include <stdlib.h>
#include "cddb.h"

PVOID pvScanbusSharedMem;

extern GLOBALDATA globalData;

extern BOOL GrabberSetupDone;
extern char chrConfigDir[CCHMAXPATH];

extern int iNumCD;
extern char cFirstCD;
extern char chosenCD[3];

extern char chrInstallDir[CCHMAXPATH];
extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern char chrDataCDROptions[CCHMAXPATH];
extern char chrAudioCDROptions[CCHMAXPATH];
extern LONG lCDROptions;
extern char chosenWriter[3];

extern char chrDeviceName[CCHMAXPATH];

extern char chrDVDDaoOptions[CCHMAXPATH];
extern char chrDVDDao[CCHMAXPATH];

extern char chrMkisofs[CCHMAXPATH];/* Path to mkisofs */
extern char chrMkisofsOptions[CCHMAXPATH];
extern LONG lMKOptions;
extern BOOL MkisofsSetupDone;
extern int iCodePage;
extern BOOL bDisableCp;

extern int iBus;
extern int iTarget;
extern int iLun;
extern int iSpeed;
extern int iFifo;

extern char chrCDDBServer[100];
extern char chrCDDBUser[100];
extern char chrCDDBUserHost[100];
extern BOOL bUseCDDB;
extern char cddbServer[MAXSERVERS][100];
extern int NUMSERVERS;

extern BOOL setupDone;
extern HMODULE hSettingsResource;

extern char chrTBFlyFontName[CCHMAXPATH];/* Font for toolbar fly over help */
extern RGB rgbTBFlyForeground;
extern RGB rgbTBFlyBackground;
extern BOOL bTBFlyOverEnabled;
extern int iTBFlyOverDelay;

extern BOOL bHintEnabled;
extern BOOL bUseCustomPainting;

extern HPOINTER hptrHand;

/* Extern */
extern void sendConfig();
extern ULONG cwQueryOSRelease();
extern ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);
extern void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
extern ULONG showMsgBox2(HWND hwnd, ULONG ulIDTitle, ULONG ulIDText, HMODULE hModule, ULONG ulFlag);
extern ULONG launchWrapper(PSZ parameter, PSZ folderPath,HWND hwnd, PSZ wrapperExe, PSZ title="CDRecord/2");
extern BOOL checkFileExists(char* chrFileName);
extern BOOL cwMoveNotebookButtonsWarp4(HWND hwndDlg, USHORT usID, USHORT usDelta);
extern BOOL cwEnableNotebookButton(HWND hwndDlg, USHORT usID, BOOL fEnable);
extern BOOL startScanbusHelper(HWND hwnd);
extern void HlpOpenWebPage(char* chrUrl);
void SysWriteToTrapLog(const char* chrFormat, ...);
extern void writeLog(char* logFile, char* logText);
void _Optlink reOpenSettingsThreadFunc (void *arg);
SOMClass* cwGetSomClass(char* chrClassName);

/* Local */
MRESULT EXPENTRY settingsCdrecordOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsGeneralOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsToolbarOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsHintOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY settingsGrabberOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsCdrdaoOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2); 

#if 0
static BOOL _queryExecutableFromProgramObject(WPObject* thisPtr, char * chrIDorPath, char *chrExecutablePath, int iSize)
{
  M_WPObject *m_wpObject;
  WPProgram *wpObject;
  ULONG        ulSize;
  HOBJECT hObject;
  BOOL rc=FALSE;

  if(!somIsObj(thisPtr))
    return FALSE;

  hObject=WinQueryObject(chrIDorPath);
  if(hObject) {
    m_wpObject=(M_WPObject*)thisPtr->somGetClass();
    if(somIsObj(m_wpObject)) {
      wpObject=(WPProgram*)m_wpObject->wpclsQueryObject(hObject);
      if(somIsObj(wpObject)) {
        PPROGDETAILS pProgDetails;
        // Get information about a program object //
        if ((wpObject->wpQueryProgDetails ( (PPROGDETAILS)NULL, &ulSize))){
          if ((pProgDetails = (PPROGDETAILS) wpObject->wpAllocMem(ulSize, NULL)) != NULL){
            /* We have memory */
            if ((wpObject->wpQueryProgDetails ( pProgDetails, &ulSize))){
              strncpy(chrExecutablePath, pProgDetails->pszExecutable, iSize-1);
              wpObject->wpFreeMem((PBYTE)pProgDetails);
              rc=TRUE;
            }/* wpQueryProgDetails() */
          }/* wpAllocMem() */
        }
        wpObject->wpUnlockObject();
      }/* if(somIsObj(wpObject)) */
    }
  }
  return rc;
}
#endif

#if 0
MRESULT EXPENTRY settingsIsoFSOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  CWCreatorSettings* thisPtr;
  M_WPObject *m_wpObject;
  WPProgram *wpObject;
  ULONG        ulSize;

  switch(msg) {
  case WM_INITDLG :
    WinSetWindowULong(WinWindowFromID(hwnd,IDPB_ISOFSHELP),
                      QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.

    WinSendMsg(WinWindowFromID(hwnd,IDEF_MNTISOFS),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
    WinSendMsg(WinWindowFromID(hwnd,IDEF_UMNTISO),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
    /* Try to find programs on our own */
    if(strlen(chrMntIsoFSPath)<3){/* Use 3 so it works even if the user put a space or something into the EF */
      thisPtr=(CWCreatorSettings*)PVOIDFROMMP(mp2);
      _queryExecutableFromProgramObject(thisPtr, ID_ISOFS_MOUNT, chrMntIsoFSPath, sizeof(chrMntIsoFSPath));
    }
    if(strlen(chrUmntIsoPath)<3){/* Use 3 so it works even if the user put a space or something into the EF */
      thisPtr=(CWCreatorSettings*)PVOIDFROMMP(mp2);
      _queryExecutableFromProgramObject(thisPtr, ID_ISOFS_UNMOUNT, chrUmntIsoPath, sizeof(chrUmntIsoPath));
    }
    WinSetWindowText(WinWindowFromID(hwnd,IDEF_MNTISOFS),chrMntIsoFSPath);
    WinSetWindowText(WinWindowFromID(hwnd,IDEF_UMNTISO),chrUmntIsoPath);
    
    /* Move default buttons on Warp 4 */
    winhAssertWarp4Notebook(hwnd,
                            IDPB_ISOFSHELP,    // in: ID threshold
                            20);       // in: dialog units or 0
    
    return (MRESULT) TRUE;
  case WM_DESTROY:
    {
      /* The notebook closes and gets destroyed */
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_MNTISOFS),sizeof(chrMntIsoFSPath),chrMntIsoFSPath);
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_UMNTISO),sizeof(chrUmntIsoPath),chrUmntIsoPath);
      /* Let the WPS save the new data */
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_ISOFSHELP),QWL_USER);
      if(somIsObj(thisPtr))
        thisPtr->wpSaveImmediate();
      break;
    }
  case WM_COMMAND:    
    switch(SHORT1FROMMP(mp1))
      {
      case IDPB_MNTISOFSBROWSE:
        {
          FILEDLG fd = { 0 };
          char text[100];

          /* User pressed the browse button */
          fd.cbSize = sizeof( fd );
          /* It's an centered 'Open'-dialog */
          fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
          /* Title: "Search mntisofs.exe" */
          getMessage(text,IDSTR_FDLGSEARCHMNTISOFSTITLE,sizeof(text), hSettingsResource,hwnd);
          /* Set the title of the file dialog */
          fd.pszTitle = text;
          /* Only show *.exe files */
          sprintf(fd.szFullFile,"%s","*.exe");

          if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
            {
              /* WinFileDlg failed */
              break;
            }
          if( fd.lReturn == DID_OK )
            {
              WinSetWindowText( WinWindowFromID(hwnd,IDEF_MNTISOFS), fd.szFullFile );
            }
        }
        break;
      case IDPB_UMNTISOBROWSE:
        {
          FILEDLG fd = { 0 };
          char text[100];
          /* User pressed the browse button */
          fd.cbSize = sizeof( fd );
          /* It's an centered 'Open'-dialog */
          fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
          /* Title: "Search mntisofs.exe" */
          getMessage(text,IDSTR_FDLGSEARCHUMNTISOTITLE,sizeof(text), hSettingsResource,hwnd);
          /* Set the title of the file dialog */
          fd.pszTitle = text;
          /* Only show *.exe files */
          sprintf(fd.szFullFile,"%s","*.exe");

          if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
            {
              /* WinFileDlg failed */
              break;
            }
          if( fd.lReturn == DID_OK )
            {
              WinSetWindowText( WinWindowFromID(hwnd,IDEF_UMNTISO), fd.szFullFile );
            }
        }
        break;
      default:
        break;
      }
    return (MRESULT) TRUE;
  default:
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
#endif

#define HTML_SET_HANDPTR     1
MRESULT EXPENTRY settingsAboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static BOOL fInit;
  switch(msg) {
#if 0
  case WM_WINDOWPOSCHANGED:
    {
      PSWP pswp=(PSWP)PVOIDFROMMP(mp1);

      if(pswp && pswp->fl & SWP_SHOW)
        DosBeep(5000, 10);
    break;
    }
#endif
  case WM_INITDLG :	
    {
      LONG lClr=0x00ff00ff;//CLR_BLUE;//SYSCLR_BACKGROUND;
      WPObject *wpObject=(WPObject*)LONGFROMMP(mp2);

      WinSetWindowULong(hwnd,QWL_USER,LONGFROMMP(mp2));

      fInit=FALSE;
      WinSetWindowULong(hwnd,QWL_USER,LONGFROMMP(mp2));
      if(wpObject->somIsA(cwGetSomClass("CWObject"))) {
        // Set the background colour of the text area
        WinSetPresParam(WinWindowFromID(hwnd, IDFT_ABOUT),
                        PP_BACKGROUNDCOLOR,(ULONG)sizeof(ULONG), &lClr );
        /* Set hand pointer for html */
        WinSendMsg(WinWindowFromID(hwnd, IDFT_ABOUT), WM_APPTERMINATENOTIFY, MPFROMLONG(HTML_SET_HANDPTR),
                   MPFROMP(hptrHand));
      }
      //    WinSetParent(WinWindowFromID(hwnd, IDFT_ABOUT), WinWindowFromID(hwnd, IDGB_ABOUT), FALSE);
      
      /* Set checkbox */
      WinCheckButton(hwnd, IDCB_SHOWEVERYSETTING, (globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE ? 1 : 0));
      fInit=TRUE;
      
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_ADCINTROHELP, 20);
      
      return (MRESULT) TRUE;
    }
  case WM_HELP:
    {
      CWCreatorSettings* thisPtr;
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd , QWL_USER);
      
      if(somIsObj(thisPtr)) {
        thisPtr->wpDisplayHelp(350, AFHELPLIBRARY);      
      }
      break;
    }
  case WM_COMMAND :
    switch (SHORT1FROMMP(mp1)) {
      // Process commands here //
    case 1: /* This is coming from the formated text control */
      /* Open creator homepage */
      //  DosBeep(1000, 10); /* ACK, so user knows the click got through */
      HlpOpenWebPage( CD_CREATOR_HOME_URL );
      break;
    
    default:
      break;
    }
    /* Don't call WinDefDlgProc here, or the dialog gets closed */
    return (MRESULT) FALSE;
#if 0
      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;

        if(WinQueryFocus(HWND_DESKTOP)!=
           WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT)) {
          mp2=MPFROMLONG(LONGFROMMP(mp2)|0x80000);/*AWP_ACTIVATE 0x00080000L*/
          mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
          return mr;  
        }
        break;
      }
    case WM_FOCUSCHANGE:
      {
        if(!SHORT1FROMMP(mp2)) {
          if(HWNDFROMMP(mp1)==hwnd) {
            MRESULT mr;

            mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
#endif
  case WM_DESTROY:
    {
      CWCreatorSettings* thisPtr;
      
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd , QWL_USER);
      
      if(somIsObj(thisPtr))
        thisPtr->wpSaveImmediate();
      break;
    }
  case WM_CONTROL:
    if(fInit && SHORT1FROMMP(mp1)==IDCB_SHOWEVERYSETTING && SHORT2FROMMP(mp1)==BN_CLICKED) {
      CWCreatorSettings* thisPtr;
      /* Reopen settings */
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd , QWL_USER);
      
      if(somIsObj(thisPtr)) {
        /* Beep only when MB_INFORMATION! */
        DosBeep(1000, 100);
        showMsgBox2(hwnd, IDSTRS_CREATORSETTINGS, IDSTR_REOPENSETTINGSINFO, hSettingsResource, 
                    MB_MOVEABLE|MB_OK|MB_INFORMATION);

        if(WinQueryButtonCheckstate(hwnd, IDCB_SHOWEVERYSETTING)==1)
          globalData.ulGlobalFlags|=GLOBALFLAG_EXPERTMODE;
        else
          globalData.ulGlobalFlags&=~GLOBALFLAG_EXPERTMODE;
        _beginthread(reOpenSettingsThreadFunc ,NULL,8192*16,thisPtr); //Fehlerbehandlung fehlt
      }
      return (MRESULT) FALSE;
    }
    break;

  default:
    break;
  }
  // The WinDefDlgProc() handles the rest of the messages
  return WinDefDlgProc(hwnd, msg, mp1, mp2);

}


/*************************************************************/
/* This dialog procedure handles the hint options page       */
/*************************************************************/
MRESULT EXPENTRY settingsHintOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  CWCreatorSettings* thisPtr;
  
  switch(msg) {
  case WM_INITDLG :	
    WinSetWindowULong(hwnd,QWL_USER,LONGFROMMP(mp2));
    
    /* We have to initialize the dialog controls with the approbiate values */
    if(bHintEnabled)
      WinCheckButton(hwnd, IDCB_ENABLEHINTS, 1);
    else
      WinCheckButton(hwnd, IDCB_ENABLEHINTS, 0);
    
    if(bUseCustomPainting)
      WinCheckButton(hwnd, IDCB_USEOS2LOOK, 0);
    else
      WinCheckButton(hwnd, IDCB_USEOS2LOOK, 1);

    if(globalData.bTipsEnabled)
      WinCheckButton(hwnd, IDCB_ENABLETIPS, 1);
    else
      WinCheckButton(hwnd, IDCB_ENABLETIPS, 0);

    /* Move default buttons on Warp 4 */
    cwMoveNotebookButtonsWarp4(hwnd, IDPB_HINTHELP, 20);
    return (MRESULT) TRUE;

      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;

        if(WinQueryFocus(HWND_DESKTOP)!=
           WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT)) {
          mp2=MPFROMLONG(LONGFROMMP(mp2)|0x80000);/*AWP_ACTIVATE 0x00080000L*/
          mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
          return mr;  
        }
        break;
      }
    case WM_FOCUSCHANGE:
      {
        if(!SHORT1FROMMP(mp2)) {
          if(HWNDFROMMP(mp1)==hwnd) {
            MRESULT mr;

            mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
#if 0
  case WM_WINDOWPOSCHANGED:
    {
      /* This prevents switching the notebook page behind the open folder */
      if(!(WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_FLAGS) & FF_ACTIVE) ) {
        PSWP pswp=(PSWP)PVOIDFROMMP(mp1);
        if(pswp->fl & SWP_ZORDER)
          mp2=MPFROMLONG(0x80000);
      }
      break;
    }
#endif
  case WM_DESTROY:
    /* The notebook closes and gets destroyed */
    /* Set focus to desktop to prevent PM freeze */
    WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

    // Query the enable state
    if(WinQueryButtonCheckstate(hwnd,IDCB_ENABLEHINTS) & 1)
      bHintEnabled=1;
    else
      bHintEnabled=0;

    if(WinQueryButtonCheckstate(hwnd, IDCB_USEOS2LOOK) & 1)
      bUseCustomPainting=FALSE;
    else
      bUseCustomPainting=TRUE;

    if(WinQueryButtonCheckstate(hwnd, IDCB_ENABLETIPS) & 1)
      globalData.bTipsEnabled=TRUE;
    else
      globalData.bTipsEnabled=FALSE;

    // Save data
    thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd , QWL_USER);
    if(somIsObj(thisPtr))
      thisPtr->wpSaveImmediate();
    break;
  case WM_CONTROL:
    /* The window controls send WM_CONTROL messages */
    switch(SHORT1FROMMP(mp1))
      {
        case IDCB_USEOS2LOOK:
          if(WinQueryButtonCheckstate(hwnd, IDCB_USEOS2LOOK) & 1)
            bUseCustomPainting=FALSE;
          else
            bUseCustomPainting=TRUE;
          WinInvalidateRect(HWND_DESKTOP, NULL, TRUE);
          return (MRESULT) FALSE;
      default:
        break;
      } // end switch(SHORT1FROMMP(mp1))
    break;			
  case WM_COMMAND :
    switch (SHORT1FROMMP(mp1)) {
      // Process commands here //
    case IDPB_HINTHELP:
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd , QWL_USER);
      
      if(somIsObj(thisPtr))
        thisPtr->wpDisplayHelp(2010,AFHELPLIBRARY); 
      break;
    case IDPB_HINTUNDO:
      /* The undo button was clicked */
      if(bHintEnabled)
        WinCheckButton(hwnd, IDCB_ENABLEHINTS, 1);
      else 
        WinCheckButton(hwnd, IDCB_ENABLEHINTS, 0);
      
      if(bUseCustomPainting)
        WinCheckButton(hwnd, IDCB_USEOS2LOOK, 0);
      else
        WinCheckButton(hwnd, IDCB_USEOS2LOOK, 1);

      if(globalData.bTipsEnabled)
        WinCheckButton(hwnd, IDCB_ENABLETIPS, 1);
      else
        WinCheckButton(hwnd, IDCB_ENABLETIPS, 0);

      break;
    case IDPB_HINTSTANDARD:
      /* The default button was clicked */
      WinCheckButton(hwnd, IDCB_ENABLEHINTS, 1);
      break;
    }
    /* Don't call WinDefDlgProc here, or the dialog gets closed */
    return (MRESULT) TRUE;
  default:
    break;
  }
  // The WinDefDlgProc() handles the rest of the messages
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/***********************************************************************/
/*                                                                     */
/* This procedure handles the cdrecord settings page for audio folders */
/* The only difference to the data folder is the option entryfield.    */
/* It is possible to specify different options for data and audio      */
/* since version 0.34                                                  */
/*                                                                     */
/***********************************************************************/
MRESULT EXPENTRY settingsDVDDaoOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  FILEDLG fd = { 0 };
  CWCreatorSettings* thisPtr;
  ULONG ulFlags;
  int a;
  char chrCD[4];
  char driveID[40];
  char text[CCHMAXPATH];

  HOBJECT  hObject;

  switch(msg)
    {
    case WM_INITDLG :   
      WinSetWindowULong(hwnd, QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.

      WinSendMsg(WinWindowFromID(hwnd,IDEF_DVDDAOPATH),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_DVDDAOPATH),chrDVDDao);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_DVDDAOOPTIONS),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)sizeof(chrDVDDaoOptions)),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_DVDDAOOPTIONS),chrDVDDaoOptions);
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_CDRECORDHELP, 20);
      return (MRESULT) TRUE;
      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;

        if(WinQueryFocus(HWND_DESKTOP)!=
           WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT)) {
          mp2=MPFROMLONG(LONGFROMMP(mp2)|0x80000);/*AWP_ACTIVATE 0x00080000L*/
          mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
          return mr;  
        }
        break;
      }
    case WM_FOCUSCHANGE:
      {
        if(!SHORT1FROMMP(mp2)) {
          if(HWNDFROMMP(mp1)==hwnd) {
            MRESULT mr;

            mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS,
                       MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }

#if 0
    case WM_WINDOWPOSCHANGED:
      {
        /* This prevents switching the notebook page behind the open folder */
        if(!(WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_FLAGS) & FF_ACTIVE) ) {
          PSWP pswp=(PSWP)PVOIDFROMMP(mp1);
          if(pswp->fl & SWP_ZORDER)
            mp2=MPFROMLONG(0x80000);
        }
        break;
      }
#endif
#if 0
    case WM_HELP:
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {
        case IDEF_CDRECORDOPTIONS:
          return (MRESULT)thisPtr->wpDisplayHelp(IDEF_CDRECORDOPTIONS,AFHELPLIBRARY);
        case IDEF_CDRECORDDATAOPTIONS:
          return (MRESULT)thisPtr->wpDisplayHelp(IDEF_CDRECORDOPTIONS,AFHELPLIBRARY);
        case IDPB_WIDERRUFEN:
          return (MRESULT)thisPtr->wpDisplayHelp(IDPB_WIDERRUFEN,AFHELPLIBRARY);
        case IDPB_CDRECORDBROWSE:
          return (MRESULT)thisPtr->wpDisplayHelp(IDPB_CDRECORDBROWSE,AFHELPLIBRARY);
        case IDEF_CDRECORDPATH:
          return (MRESULT)thisPtr->wpDisplayHelp(IDEF_CDRECORDPATH,AFHELPLIBRARY);
        default:
          break;
        }
      return (MRESULT)thisPtr->wpDisplayHelp(IDDLG_CDRECORDSETUP,AFHELPLIBRARY);
      break;
#endif
    case WM_DESTROY:
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* The notebook closes and gets destroyed */
      /* Query the cdrecord path from the entryfield */
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_DVDDAOPATH),sizeof(chrDVDDao),chrDVDDao);
      /* Query the cdrecord options from the entryfield for audio folders */     
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_DVDDAOOPTIONS),sizeof(chrDVDDaoOptions),chrDVDDaoOptions);

      sendConfig();

      /* Let the WPS save the new data */
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr))
        thisPtr->wpSaveImmediate();
      /* Setup is done */
      setupDone=TRUE;
      return (MRESULT) TRUE;
    case WM_COMMAND:    
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_DVDDAOBROWSE:
          /* User pressed the browse button */
          fd.cbSize = sizeof( fd );
          /* It's an centered 'Open'-dialog */
          fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
          /* Title: "Search CDRecord/2" */
          getMessage(text,IDSTR_FDLGSEARCHDVDDAOTITLE, sizeof(text), hSettingsResource,hwnd);
          /* Set the title of the file dialog */
          fd.pszTitle = text;
          /* Only show *.exe files */
          sprintf(fd.szFullFile,"%s","*.exe");

          if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
            {
              /* WinFileDlg failed */
              break;
            }
          if( fd.lReturn == DID_OK )
            {
              WinSetWindowText( WinWindowFromID(hwnd,IDEF_DVDDAOPATH), fd.szFullFile );
            }
          break;
        case IDPB_WIDERRUFEN:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_DVDDAOPATH),chrDVDDao);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_DVDDAOOPTIONS),chrDVDDaoOptions);
          break;
        default:
          break;
        }
      return (MRESULT) TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/***********************************************************************/
/*                                                                     */
/* This procedure handles the cdrecord settings page for audio folders */
/* The only difference to the data folder is the option entryfield.    */
/* It is possible to specify different options for data and audio      */
/* since version 0.34                                                  */
/*                                                                     */
/***********************************************************************/
MRESULT EXPENTRY settingsCdrecordOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  FILEDLG fd = { 0 };
  CWCreatorSettings* thisPtr;
  ULONG ulFlags;
  int a;
  char chrCD[4];
  char driveID[40];
  char text[CCHMAXPATH];

  HOBJECT  hObject;

  switch(msg)
    {
    case WM_INITDLG :   
      WinSetWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),
                        QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.

      WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRECORDPATH),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRECORDPATH),chrCDRecord);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRECORDOPTIONS),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)sizeof(chrAudioCDROptions)),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRECORDOPTIONS),chrAudioCDROptions);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRECORDDATAOPTIONS),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)sizeof(chrDataCDROptions)),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRECORDDATAOPTIONS),chrDataCDROptions);
#if 0
      for( a=0; a<iNumCD ;a++) {
        chrCD[0]=cFirstCD+a;
        chrCD[1]=':';
        chrCD[2]=0;
        WinSendMsg(WinWindowFromID(hwnd,IDLB_WRITERLETTER),LM_INSERTITEM,MPFROMSHORT(LIT_END),(MPARAM)chrCD);
        if(!a){
          if(!setupDone)
            WinSetWindowText(WinWindowFromID(hwnd,IDLB_WRITERLETTER),chrCD);
          else
            WinSetWindowText(WinWindowFromID(hwnd,IDLB_WRITERLETTER),chosenWriter);
        }
      }
#endif
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_CDRECORDHELP, 20);
      return (MRESULT) TRUE;
      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;

        if(WinQueryFocus(HWND_DESKTOP)!=
           WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT)) {
          mp2=MPFROMLONG(LONGFROMMP(mp2)|0x80000);/*AWP_ACTIVATE 0x00080000L*/
          mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
          return mr;  
        }
        break;
      }
    case WM_FOCUSCHANGE:
      {
        if(!SHORT1FROMMP(mp2)) {
          if(HWNDFROMMP(mp1)==hwnd) {
            MRESULT mr;

            mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS,
                       MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }

#if 0
    case WM_WINDOWPOSCHANGED:
      {
        /* This prevents switching the notebook page behind the open folder */
        if(!(WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_FLAGS) & FF_ACTIVE) ) {
          PSWP pswp=(PSWP)PVOIDFROMMP(mp1);
          if(pswp->fl & SWP_ZORDER)
            mp2=MPFROMLONG(0x80000);
        }
        break;
      }
#endif
    case WM_HELP:
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {
        case IDEF_CDRECORDOPTIONS:
          return (MRESULT)thisPtr->wpDisplayHelp(IDEF_CDRECORDOPTIONS,AFHELPLIBRARY);
        case IDEF_CDRECORDDATAOPTIONS:
          return (MRESULT)thisPtr->wpDisplayHelp(IDEF_CDRECORDOPTIONS,AFHELPLIBRARY);
        case IDPB_WIDERRUFEN:
          return (MRESULT)thisPtr->wpDisplayHelp(IDPB_WIDERRUFEN,AFHELPLIBRARY);
        case IDPB_CDRECORDBROWSE:
          return (MRESULT)thisPtr->wpDisplayHelp(IDPB_CDRECORDBROWSE,AFHELPLIBRARY);
        case IDEF_CDRECORDPATH:
          return (MRESULT)thisPtr->wpDisplayHelp(IDEF_CDRECORDPATH,AFHELPLIBRARY);
        default:
          break;
        }
      return (MRESULT)thisPtr->wpDisplayHelp(IDDLG_CDRECORDSETUP,AFHELPLIBRARY);
      break;
    case WM_DESTROY:
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* The notebook closes and gets destroyed */
      /* Query the cdrecord path from the entryfield */
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDRECORDPATH),sizeof(chrCDRecord),chrCDRecord);
      /* Query the cdrecord options from the entryfield for audio folders */     
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDRECORDOPTIONS),sizeof(chrAudioCDROptions),chrAudioCDROptions);
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDRECORDDATAOPTIONS),sizeof(chrDataCDROptions),chrDataCDROptions);
      sendConfig();

      /* Query the cd-drive */
      //    WinQueryWindowText( WinWindowFromID(hwnd,IDLB_WRITERLETTER),sizeof(chosenWriter),chosenWriter);

      /* Let the WPS save the new data */
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),QWL_USER);
      if(somIsObj(thisPtr))
        thisPtr->wpSaveImmediate();
      /* Setup is done */
      setupDone=TRUE;
      return (MRESULT) TRUE;
    case WM_COMMAND:    
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_CDRECORDBROWSE:
          /* User pressed the browse button */
          fd.cbSize = sizeof( fd );
          /* It's an centered 'Open'-dialog */
          fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
          /* Title: "Search CDRecord/2" */
          getMessage(text,IDSTR_FDLGSEARCHCDR2TITLE,sizeof(text), hSettingsResource,hwnd);
          /* Set the title of the file dialog */
          fd.pszTitle = text;
          /* Only show *.exe files */
          sprintf(fd.szFullFile,"%s","*.exe");

          if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
            {
              /* WinFileDlg failed */
              break;
            }
          if( fd.lReturn == DID_OK )
            {
              WinSetWindowText( WinWindowFromID(hwnd,IDEF_CDRECORDPATH), fd.szFullFile );
            }
          break;
        case IDPB_WIDERRUFEN:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRECORDPATH),chrCDRecord);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRECORDOPTIONS),chrAudioCDROptions);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRECORDDATAOPTIONS),chrDataCDROptions);
          break;
        default:
          break;
        }
      return (MRESULT) TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*
  This function is used to temporarily disable the
  writer controls while scanbus is in progress.
 */
static void enableWriterCtrls(HWND hwnd, BOOL fEnable)
{
  WinEnableWindow(WinWindowFromID(hwnd, IDPB_SCAN), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDDD_DEVICESELECTION), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDSB_SPEED), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDCB_BURNPROOF), fEnable);
}


/***************************************************************************/
/*                                                                         */
/* This procedure handles the Writer settings page of the settings object. */
/*                                                                         */
/***************************************************************************/
MRESULT EXPENTRY settingsGeneralOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWCreatorSettings* thisPtr;
  ULONG ulFlags;
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  char *lbEntry;
  static  BOOL fScanning;
  static BOOL fInitDone;
  /* For Undo */
  static char chrSavedDevice[sizeof(chrDeviceName)];
  static USHORT usEject;
  static USHORT usBurnProof;
  static USHORT usFifo;
  static USHORT usSpeed;

  switch(msg)
    {
    case WM_INITDLG : 
      fInitDone=FALSE;  
      WinSetWindowULong(WinWindowFromID(hwnd,IDPB_SCAN),
                        QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      /* Set current known device name */
      WinSetWindowText(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),chrDeviceName);
      strcpy(chrSavedDevice, chrDeviceName);
      /* Speed spin button */
      WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETLIMITS,MPFROMLONG(64),MPFROMLONG(1));
      WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETCURRENTVALUE,(MPARAM)iSpeed,MPFROMLONG(0));
      usSpeed=(USHORT)iSpeed;
      /* FIFO spin button */
      WinSendMsg(WinWindowFromID(hwnd,IDSB_FIFO),SPBM_SETLIMITS,MPFROMLONG(64),MPFROMLONG(0));
      WinSendMsg(WinWindowFromID(hwnd,IDSB_FIFO),SPBM_SETCURRENTVALUE,(MPARAM)iFifo,MPFROMLONG(0));
      usFifo=(USHORT)iFifo;

      /* Eject */
      if(lCDROptions & IDCDR_NOEJECT) {
        WinCheckButton(hwnd,IDCB_EJECT, 0);
        usEject=0;
      }
      else {
        WinCheckButton(hwnd,IDCB_EJECT,1);
        usEject=1;
      }

      /* BURN-Proof checkbox */
      if(lCDROptions & IDCDR_BURNPROOF) {
        WinCheckButton(hwnd, IDCB_BURNPROOF, 1);
        usBurnProof=1;
      }
      else {
        WinCheckButton(hwnd, IDCB_BURNPROOF, 0);
        usBurnProof=0;
      }

      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_GENERALHELP, 20);

      /* Now scan the bus for new devices */
      WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDPB_SCAN), MPFROM2SHORT(CMDSRC_PUSHBUTTON, TRUE));
      fInitDone=TRUE;
      return (MRESULT) TRUE;

    case WM_MOUSEMOVE:
      if(fScanning) {
        WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE) );
        return (MRESULT)TRUE;
      }
      break;
      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;

        if(WinQueryFocus(HWND_DESKTOP)!=
           WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT)) {
          mp2=MPFROMLONG(LONGFROMMP(mp2)|0x80000);/*AWP_ACTIVATE 0x00080000L*/
          mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
          return mr;  
        }
        break;
      }
    case WM_FOCUSCHANGE:
      {
        if(!SHORT1FROMMP(mp2)) {
          if(HWNDFROMMP(mp1)==hwnd) {
            MRESULT mr;

            mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT),
                       WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
    case WM_HELP:
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_SCAN),QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      /* Switch the right help panel for the control with the focus to the front */
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {
        case IDCB_EJECT:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_EJECT, AFHELPLIBRARY);
          /* */
        case IDCB_BURNPROOF:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_BURNPROOF, AFHELPLIBRARY);
        default:
          break;
        }
      /* Writer page help */
      return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GENERALPAGE, AFHELPLIBRARY);
    case WM_APPTERMINATENOTIFY:
      switch(LONGFROMMP(mp1)) {
      case ACKEY_SCANBUS:
        {
          SHORT sItem;
          /* scanbus.exe terminated. Set the entryfield now */
          
          enableWriterCtrls(hwnd, TRUE);
          /* First check if our saved device is still installed */
          sItem=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),
                                        LM_SEARCHSTRING, MPFROM2SHORT(LSS_PREFIX|LSS_CASESENSITIVE, LIT_FIRST),
                                        MPFROMP(chrDeviceName)));
          if(sItem!=LIT_ERROR && sItem!=LIT_NONE) {
            /* Device found in list so put name into entry field*/
            WinSetWindowText(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),chrDeviceName);
            /* Set saved speed value */
            WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETCURRENTVALUE,(MPARAM)iSpeed,MPFROMLONG(0));
            /* Set saved burnproof value */
            /* BURN-Proof checkbox */
            if(lCDROptions & IDCDR_BURNPROOF)
              WinCheckButton(hwnd, IDCB_BURNPROOF, 1);
            else
              WinCheckButton(hwnd, IDCB_BURNPROOF, 0);
          }
          else {
            /* Our device no longer is installed */
            char chrDevice[CCHMAXPATH];
            if(SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDDD_DEVICESELECTION),LM_QUERYITEMTEXT,
                                  MPFROM2SHORT(0, sizeof(chrDevice)), MPFROMP(chrDevice))))
              WinSetWindowText(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),chrDevice);
          }
          /* Free shared mem if not already done */
          if(pvScanbusSharedMem) {
            DosFreeMem(pvScanbusSharedMem);
            pvScanbusSharedMem=NULL;
          }
          fScanning=FALSE;
          break;
        }
      case ACKEY_LISTBOX:
        /* Scanbus.exe sends us a new device for the listbox */
        if(LONGFROMMP(mp2)==0) {
          /* Delete entries in listbox */
          WinSetWindowText(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),"");
          WinSendMsg(WinWindowFromID(hwnd, IDDD_DEVICESELECTION),LM_DELETEALL,MPFROMLONG(0),MPFROMLONG(0));
        }
        else
          {
            WinSendMsg(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),LM_INSERTITEM,MPFROMSHORT(LIT_END),
                       mp2);
          }
        break;
      case ACKEY_MAXWRITESPEED: /* scanbus.exe queried the max write speed of the device */
        {
          ULONG ulItemHandle;

          ulItemHandle=LONGFROMMR(WinSendMsg(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),
                                             LM_QUERYITEMHANDLE, MPFROMSHORT(SHORT1FROMMP(mp2)), 0L));
          ulItemHandle&=~0xff;
          ulItemHandle|=(SHORT2FROMMP(mp2)&0xff);
          WinSendMsg(WinWindowFromID(hwnd, IDDD_DEVICESELECTION),
                     LM_SETITEMHANDLE, MPFROMSHORT(SHORT1FROMMP(mp2)), MPFROMLONG(ulItemHandle));
          break;
        }
      case ACKEY_BURNSAVE: /* scanbus.exe queried burnproof feature */
        {
          ULONG ulItemHandle;

          ulItemHandle=LONGFROMMR(WinSendMsg(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),
                                             LM_QUERYITEMHANDLE, MPFROMSHORT(SHORT1FROMMP(mp2)), 0L));
          ulItemHandle&=~LBHANDLE_FEATURE_NOBURNPROOF;
          ulItemHandle|=(SHORT2FROMMP(mp2) & LBHANDLE_FEATURE_NOBURNPROOF);
          WinSendMsg(WinWindowFromID(hwnd, IDDD_DEVICESELECTION),
                     LM_SETITEMHANDLE, MPFROMSHORT(SHORT1FROMMP(mp2)), MPFROMLONG(ulItemHandle));
          break;
        }
      default:
        break;
      }/* switch */
      break;
    case WM_CONTROL:
      {
        switch(SHORT1FROMMP(mp1))
          {
          case IDDD_DEVICESELECTION:
            {
              /* The device changed... */
              /* This may also happen when scanbus.exe ended and the default device must be selected. */
              if(SHORT2FROMMP(mp1)==CBN_EFCHANGE) {
                /* Entry field changed */
                SHORT sItem;
                char chrDevice[20];
                char *chrPtr;
                /* Find the item in the list box. The item handle contains
                 info about max write speed and burnproof */
                WinQueryWindowText(HWNDFROMMP(mp2), sizeof(chrDevice), chrDevice);
                if(!strlen(chrDevice))
                  break;
                sItem=SHORT1FROMMR(WinSendMsg(HWNDFROMMP(mp2),
                                              LM_SEARCHSTRING, MPFROM2SHORT(LSS_PREFIX|LSS_CASESENSITIVE,
                                                                            LIT_FIRST),
                                              MPFROMP(chrDevice)));
                if(sItem!=LIT_ERROR && sItem!=LIT_NONE) {
                  /* Device found in list */
                  ULONG ulItemHandle;
                  ulItemHandle=LONGFROMMR(WinSendMsg(HWNDFROMMP(mp2),
                                                     LM_QUERYITEMHANDLE, MPFROMSHORT(sItem), 0L));
                  /* Set values */
                  if(ulItemHandle & 0xFF)
                    {
                      /* Speed */
                      WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETLIMITS,
                                 MPFROMLONG(ulItemHandle & 0xFF),
                                 MPFROMLONG(1));
                      WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETCURRENTVALUE,
                                 (MPARAM)(ulItemHandle & 0xFF),
                                 MPFROMLONG(0));
                      /* Burnproof */
                      if(ulItemHandle & LBHANDLE_FEATURE_NOBURNPROOF) {
                        WinEnableWindow(WinWindowFromID(hwnd,IDCB_BURNPROOF), FALSE);
                        WinCheckButton(hwnd, IDCB_BURNPROOF, 0);
                      }
                      else {
                        WinEnableWindow(WinWindowFromID(hwnd,IDCB_BURNPROOF), TRUE);
                        WinCheckButton(hwnd, IDCB_BURNPROOF, 1);
                      }
                    }
                  else {
                    /* Max speed unknown. It may be just a CD-ROM...or a broken firmware. */
                    WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETLIMITS,
                               MPFROMLONG(64),MPFROMLONG(1));
                    WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETCURRENTVALUE,
                               (MPARAM)1,MPFROMLONG(0));
                    WinEnableWindow(WinWindowFromID(hwnd,IDCB_BURNPROOF), TRUE);
                    WinCheckButton(hwnd, IDCB_BURNPROOF, 1);
                  }
                }
                //    WinStartTimer( WinQueryAnchorBlock(HWND_DESKTOP), hwnd, TIMERID_DEVICESELECT, 500);
                WinQueryWindowText( WinWindowFromID(hwnd,IDDD_DEVICESELECTION), sizeof(chrDeviceName), chrDeviceName);
              }/* CBN_EFCHANGE */
              break;
            }
            case IDCB_BURNPROOF:
              if(SHORT2FROMMP(mp1)==BN_CLICKED) {
                /* BURN-Proof */
                if(fInitDone) {
                  if(WinQueryButtonCheckstate(hwnd, IDCB_BURNPROOF) & 1)
                    lCDROptions|=IDCDR_BURNPROOF;
                  else
                    lCDROptions&=~IDCDR_BURNPROOF;
                }
              }
              break;
          case IDCB_EJECT:
            if(SHORT2FROMMP(mp1)==BN_CLICKED) {
              /* Eject */
              if(fInitDone) {
                if(WinQueryButtonCheckstate(hwnd,IDCB_EJECT) & 1)
                  lCDROptions&=~IDCDR_NOEJECT;
                else
                  lCDROptions|=IDCDR_NOEJECT;
              }
            }
            break;
          case IDSB_SPEED:
            if(SHORT2FROMMP(mp1)==SPBN_CHANGE) {
                ULONG ulFlags;
                if(fInitDone) {
                  WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_QUERYVALUE,MPFROMP(&ulFlags),
                             MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));
                  iSpeed=(int)ulFlags;
                }
            }
            break;
          case IDSB_FIFO:
            if(SHORT2FROMMP(mp1)==SPBN_CHANGE) {
              if(fInitDone) {
                ULONG ulFlags;
                
                WinSendMsg(WinWindowFromID(hwnd,IDSB_FIFO),SPBM_QUERYVALUE,MPFROMP(&ulFlags),
                           MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));
                iFifo=(int)ulFlags;
              }
            }
            break;
          default:
            break;
          }
        break;
      }
#if 0
    case WM_TIMER:
      if(SHORT1FROMMP(mp1)==TIMERID_DEVICESELECT)
        {
          /* This timer is started when a new device is selected. We now check if it's a known
             device by querying the database. */
          char chrDevice[50];
          char *chrPtr;

          WinStopTimer( WinQueryAnchorBlock(HWND_DESKTOP), hwnd, TIMERID_DEVICESELECT);
          WinQueryWindowText(WinWindowFromID(hwnd, IDDD_DEVICESELECTION), sizeof(chrDevice), chrDevice);

          /* Check if we have settings for the selected device */
          if((chrPtr=strchr(chrDevice, '\''))!=NULLHANDLE) {
            //   if(findDriveInDataBase(chrPtr))
            //    DosBeep(500, 100);
          }
          return (MRESULT) 0;
        }
      break;
#endif
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* Stop timer if running */
      // WinStopTimer( WinQueryAnchorBlock(HWND_DESKTOP), hwnd, TIMERID_DEVICESELECT);

      WinQueryWindowText( WinWindowFromID(hwnd,IDDD_DEVICESELECTION),sizeof(chrDeviceName),chrDeviceName);
      sscanf(chrDeviceName,"%d,%d,%d ",&iBus,&iTarget,&iLun);
      WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_QUERYVALUE,MPFROMP(&ulFlags),
                 MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));
      iSpeed=(int)ulFlags;
      
      WinSendMsg(WinWindowFromID(hwnd,IDSB_FIFO),SPBM_QUERYVALUE,MPFROMP(&ulFlags),
                 MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));
      iFifo=(int)ulFlags;
      
      /* Eject */
      if(WinQueryButtonCheckstate(hwnd,IDCB_EJECT) & 1)
        lCDROptions&=~IDCDR_NOEJECT;
      else
        lCDROptions|=IDCDR_NOEJECT;
      /* BURN-Proof */
      if(WinQueryButtonCheckstate(hwnd, IDCB_BURNPROOF) & 1)
        lCDROptions|=IDCDR_BURNPROOF;
      else
        lCDROptions&=~IDCDR_BURNPROOF;
      
      /* Let the WPS save the new data */
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_SCAN),QWL_USER);
      if(somIsObj(thisPtr))
        thisPtr->wpSaveImmediate();      
      /* Free shared mem if not already done */
      if(pvScanbusSharedMem) {
        DosFreeMem(pvScanbusSharedMem);
        pvScanbusSharedMem=NULL;
      }
      /* Setup is done */
      return (MRESULT) TRUE;
    case WM_COMMAND:    
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_GENERALUNDO:
          /* User pressed the UNDO button */
          WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETCURRENTVALUE,MPFROMLONG((LONG)usSpeed),
                     MPFROMLONG(0));
          WinSendMsg(WinWindowFromID(hwnd,IDSB_FIFO),SPBM_SETCURRENTVALUE,MPFROMLONG((LONG)usFifo),
                     MPFROMLONG(0));
          WinSetWindowText(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),chrSavedDevice);
          
          /* Eject */
          WinCheckButton(hwnd,IDCB_EJECT, usEject);

          /* BURN-Proof */
          WinCheckButton(hwnd, IDCB_BURNPROOF, usBurnProof);
          break;
        case IDPB_SCAN:
          if(checkFileExists(chrCDRecord)) {
            /* Allocate shared mem for communication between helper and settings notebook */
            if(!pvScanbusSharedMem) {
              if(DosAllocSharedMem(&pvScanbusSharedMem,SCANBUSSHAREDMEM_NAME, SCANBUSSHAREDMEM_SIZE,
                                   PAG_READ|PAG_WRITE|PAG_COMMIT)) {
                /*
                  Text:  "Can't allocate shared memory. There's probably already a creation process running."
                  Title: "Audio-CD-Creator"                       
                  */             
                messageBox( text, IDSTR_ALLOCSHAREDMEMERROR , sizeof(text),
                            title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                            hSettingsResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
                break;
              }
            }
            else
              {
                /* The shared mem is already allocated. There's another write process running. */
                /*
                  Text:  "Can't allocate shared memory. There's probably already a creation process running."
                  Title: "Audio-CD-Creator"                       
                  */             
                messageBox( text, IDSTR_ALLOCSHAREDMEMERROR , sizeof(text),
                            title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                            hSettingsResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
                break;
              }
            if(launchWrapper(chrInstallDir,"",hwnd,"scanbus.exe","Scanning SCSI bus...")) {
              if(pvScanbusSharedMem) {
                DosFreeMem(pvScanbusSharedMem);
                pvScanbusSharedMem=NULL;
                break;
              }
            }/* launchwrapper() */
            fScanning=TRUE;
            enableWriterCtrls(hwnd, FALSE);
          }
          else {
            /* The path to CDRecord/2 isn't valid */
            /* title: "...%s..." */
            getMessage(title, IDSTR_NOVALIDPATH,sizeof(title), hSettingsResource, hwnd);            
            sprintf(text, title, "CDRecord/2");
            /* title: "Audio-CD-Creator" */
            getMessage(title, IDSTR_AUDIOCDCREATOR,sizeof(title), hSettingsResource, hwnd);                     
            WinMessageBox(  HWND_DESKTOP, hwnd,
                            text,
                            title,
                            0UL, MB_OK | MB_ERROR |MB_MOVEABLE);
          }
          break;
        default:
          break;
        }
      return (MRESULT) TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/******************************************************/
/*                                                    */
/* This procedure handles the general settings page 2 */
/* of the settings object                             */
/*                                                    */
/******************************************************/
MRESULT EXPENTRY settingsGeneralOption2DlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWCreatorSettings* thisPtr;

  switch(msg)
    {
    case WM_INITDLG :   
      WinSetWindowULong(hwnd, QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.

      /* Create shadows */
      if(lCDROptions&IDCDR_CREATESHADOWS)
        WinCheckButton(hwnd,IDCB_CREATESHADOWS,1);
      else
        WinCheckButton(hwnd,IDCB_CREATESHADOWS,0);

      if(lCDROptions&IDCDR_HIDEWINDOW)
        WinCheckButton(hwnd,IDCB_CDRSTARTHIDDEN,1);
      else
        WinCheckButton(hwnd,IDCB_CDRSTARTHIDDEN,0);
      if(lCDROptions&IDCDR_CLOSEWINDOW)
        WinCheckButton(hwnd,IDCB_CDRCLOSEWINDOW,1);
      else
        WinCheckButton(hwnd,IDCB_CDRCLOSEWINDOW,0);

      /* Sony multisession fix */
      if(lCDROptions & IDCDR_SONYMULTISESSION)
        WinCheckButton(hwnd, IDCB_SONYMULTISESSION, 1);
      else
        WinCheckButton(hwnd, IDCB_SONYMULTISESSION, 0);

      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_GENERAL2HELP, 20);
      return (MRESULT) TRUE;
      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;

        if(WinQueryFocus(HWND_DESKTOP)!=
           WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT)) {
          mp2=MPFROMLONG(LONGFROMMP(mp2)|0x80000);/*AWP_ACTIVATE 0x00080000L*/
          mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
          return mr;  
        }
        break;
      }
    case WM_FOCUSCHANGE:
      {
        if(!SHORT1FROMMP(mp2)) {
          if(HWNDFROMMP(mp1)==hwnd) {
            MRESULT mr;

            mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
#if 0
    case WM_WINDOWPOSCHANGED:
      {
        /* This prevents switching the notebook page behind the open folder */
        if(!(WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_FLAGS) & FF_ACTIVE))
          mp2=MPFROMLONG(0x80000);
        break;
      }
#endif
    case WM_HELP:
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd, QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      /* Switch the right help panel for the control with the focus to the front */
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {
        case IDCB_CREATESHADOWS:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CREATESHADOWS, AFHELPLIBRARY);
          /* Start hidden */
        case IDCB_CDRSTARTHIDDEN:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDRSTARTHIDDEN, AFHELPLIBRARY);
          /*  Close helper window */
        case IDCB_CDRCLOSEWINDOW:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDRCLOSEWINDOW, AFHELPLIBRARY);
          /* Create shadows as default */
        case IDCB_SONYMULTISESSION:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_SONYMULTISESSION, AFHELPLIBRARY);
          /* */
        default:
          break;
        }
      /* General page 2 help */
      return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GENERAL2PAGE, AFHELPLIBRARY);
      break;
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)) {

        if(WinQueryButtonCheckstate(hwnd,IDCB_CREATESHADOWS)&1)
          lCDROptions|=IDCDR_CREATESHADOWS;
        else
          lCDROptions&=~IDCDR_CREATESHADOWS;

        if(WinQueryButtonCheckstate(hwnd,IDCB_CDRSTARTHIDDEN)&1)
          lCDROptions|=IDCDR_HIDEWINDOW;
        else
          lCDROptions&=~IDCDR_HIDEWINDOW;
        
        if(WinQueryButtonCheckstate(hwnd,IDCB_CDRCLOSEWINDOW)&1)
          lCDROptions|=IDCDR_CLOSEWINDOW;
        else
          lCDROptions&=~IDCDR_CLOSEWINDOW;
        
        /* Sony multisession fix */
        if(WinQueryButtonCheckstate(hwnd, IDCB_SONYMULTISESSION) & 1)
          lCDROptions|=IDCDR_SONYMULTISESSION;
        else
          lCDROptions&=~IDCDR_SONYMULTISESSION;

        /* Let the WPS save the new data */
        thisPtr->wpSaveImmediate();
      }      
      /* Setup is done */
      return (MRESULT) TRUE;
    case WM_COMMAND:    
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_GENERAL2UNDO:
          thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd, QWL_USER);
          if(somIsObj(thisPtr)) {
            /* User pressed the UNDO button */

            if(lCDROptions&IDCDR_CREATESHADOWS)
              WinCheckButton(hwnd,IDCB_CREATESHADOWS,1);
            else
              WinCheckButton(hwnd,IDCB_CREATESHADOWS,0);

            if(lCDROptions&IDCDR_HIDEWINDOW)
              WinCheckButton(hwnd,IDCB_CDRSTARTHIDDEN,1);
            else
              WinCheckButton(hwnd,IDCB_CDRSTARTHIDDEN,0);
            if(lCDROptions&IDCDR_CLOSEWINDOW)
              WinCheckButton(hwnd,IDCB_CDRCLOSEWINDOW,1);
            else
              WinCheckButton(hwnd,IDCB_CDRCLOSEWINDOW,0);
            
            /* Sony multisession fix */
            if(lCDROptions & IDCDR_SONYMULTISESSION)
              WinCheckButton(hwnd, IDCB_SONYMULTISESSION, 1);
            else
              WinCheckButton(hwnd, IDCB_SONYMULTISESSION, 0);
          }
          break;
        default:
          break;
        }
      return (MRESULT) TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}







/****************************************************/
/*                                                  */ 
/* This procedure handles the grabber settings page */
/* in the settings notebook for expert mode.        */
/*                                                  */
/****************************************************/
MRESULT EXPENTRY settingsGrabberExpertOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  FILEDLG fd = { 0 };
  CWCreatorSettings* thisPtr;
  int a;
  char chrCD[4];
  char text[CCHMAXPATH]; 
  char text2[100]; 
  static BOOL bInit; 
  SHORT  sCount;

  char *ptr;
  HINI hini;
  ULONG ulSize;
  char *pBuffer;
  BOOL bFirst;
 
  switch(msg)
    {
    case WM_INITDLG:	
      bInit=FALSE;
      WinSetWindowULong(WinWindowFromID(hwnd,IDPB_GRABBERBROWSE),
                        QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      thisPtr=(CWCreatorSettings*)PVOIDFROMMP(mp2);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_GRABBERPATH),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)sizeof(globalData.chrGrabberPath)),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_GRABBERPATH),globalData.chrGrabberPath);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_GRABBEROPTIONS),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)CCHMAXPATH),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_GRABBEROPTIONS),globalData.chrGrabberOptions);
      WinCheckButton(hwnd,IDCB_TRACKNUMBERS,globalData.bTrackNumbers);
      WinSetWindowText(WinWindowFromID(hwnd,IDDD_GRABBERNAME),globalData.chrGrabberName);

      for( a=0; a<iNumCD ;a++) {
        chrCD[0]=cFirstCD+a;
        chrCD[1]=':';
        chrCD[2]=0;
        WinSendMsg(WinWindowFromID(hwnd,IDLB_DROPDOWN),LM_INSERTITEM,MPFROMSHORT(LIT_END),(MPARAM)chrCD);
        if(!a){
          if(!GrabberSetupDone)
            WinSetWindowText(WinWindowFromID(hwnd,IDLB_DROPDOWN),chrCD);
          else
            WinSetWindowText(WinWindowFromID(hwnd,IDLB_DROPDOWN),chosenCD);
        }
      }
      /* Fill the listbox with known grabbers from comfig file */
      sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
      hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
      if(hini)
        {
          if(PrfQueryProfileSize(hini, NULL,NULL,&ulSize)) {// Query size of profile data
            pBuffer=(char*)malloc(ulSize);
            if(pBuffer) {
              if(PrfQueryProfileData(hini, NULL,NULL, pBuffer, &ulSize))// Get the whole data
                {
                  ptr=pBuffer;
                  if(strlen(globalData.chrGrabberName)==0)
                    bFirst=TRUE;
                  else
                    bFirst=FALSE;
                  do {
                    /* Only use entries with options key */
                    PrfQueryProfileString(hini,ptr,"options","",text,sizeof(text));
                    if(strlen(text)!=0) {
                      WinSendMsg(WinWindowFromID(hwnd,IDDD_GRABBERNAME),LM_INSERTITEM,
                                 MPFROMSHORT(LIT_SORTASCENDING),
                                 (MPARAM)ptr);
                      /* Insert the first match into the text field */
                      if(bFirst) {
                        bFirst=FALSE;
                        WinSetWindowText(WinWindowFromID(hwnd,IDDD_GRABBERNAME),ptr);
                      }
                    }
                    ptr=strchr(ptr,0);
                    if(!ptr)
                      break;
                    ptr++;
                  }while(*ptr!=0);
                  /* We have all grabber entries in the drop down box */

                  /* Add an user defined entry */
                  /* text: "User defined" */
                  getMessage(text, IDSTRS_DEFAULTUSERGRABBERNAME, sizeof(text), hSettingsResource, hwnd);
                  WinSendMsg(WinWindowFromID(hwnd,IDDD_GRABBERNAME),LM_INSERTITEM,
                             MPFROMSHORT(LIT_SORTASCENDING),
                             (MPARAM)text);
                  if(strlen(globalData.chrGrabberName)==0)
                    {
                      /* This is an upgrade installation or the user defined grabber. 
                         There're entries for the grabber options. */
                      /* Set the default user defined name into the text field */
                      WinSetWindowText(WinWindowFromID(hwnd,IDDD_GRABBERNAME),text);
                    }
                  else {
                    /* Predefined grabber. The user can't change the options. */
                    /* The name is set into the field at the top */
                  }
                }
              free(pBuffer);
            } /* end of if(pBuffer) */
          } /* end of if(PrfQueryProfileSize(hini, NULL,NULL,&ulSize)) */
          PrfCloseProfile(hini);
        }
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_GRABBERHELP, 20);
      bInit=TRUE;
      return (MRESULT) TRUE;
      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;

        if(WinQueryFocus(HWND_DESKTOP)!=
           WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT)) {
          mp2=MPFROMLONG(LONGFROMMP(mp2)|0x80000);/*AWP_ACTIVATE 0x00080000L*/
          mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
          return mr;  
        }
        break;
      }
    case WM_FOCUSCHANGE:
      {
        if(!SHORT1FROMMP(mp2)) {
          if(HWNDFROMMP(mp1)==hwnd) {
            MRESULT mr;

            mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS,
                       MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1))
        {
        case IDDD_GRABBERNAME:
          if(SHORT2FROMMP(mp1)==LN_SELECT && bInit) {
            /* text: "User defined" */
            getMessage(text, IDSTRS_DEFAULTUSERGRABBERNAME, sizeof(text), hSettingsResource, hwnd);
            /* Get the selected grabber name */
            WinQueryWindowText(WinWindowFromID(hwnd,IDDD_GRABBERNAME),sizeof(text2), text2);
            if(strcmpi(text,text2)) {
              /* Predefined grabber */
              sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
              hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
              if(hini)
                {
                  /* Get the selected grabber name */
                  WinQueryWindowText(WinWindowFromID(hwnd,IDDD_GRABBERNAME),sizeof(text), text);
                  if(PrfQueryProfileString(hini,text,"options","", text2,sizeof(text2))){               
                    WinSetWindowText(WinWindowFromID(hwnd,IDEF_GRABBEROPTIONS),text2);                
                    WinCheckButton(hwnd,IDCB_TRACKNUMBERS,PrfQueryProfileInt(hini,text,"addnumbers",0));
                  }
                  PrfCloseProfile(hini);
                }
            }
            else {
              /* User defined grabber */
             
            }
          }
          break;
        default:
          break;
        }
      break;
    case WM_HELP:	
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_GRABBERBROWSE),QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {
        case IDEF_GRABBEROPTIONS:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GRABBEROPTIONS,AFHELPLIBRARY);
        case IDPB_GRABBERUNDO:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GRABBERUNDO,AFHELPLIBRARY);
        case IDPB_GRABBERBROWSE:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GRABBERBROWSE,AFHELPLIBRARY);
        case IDEF_GRABBERPATH:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GRABBERPATH,AFHELPLIBRARY);
        case IDCB_TRACKNUMBERS:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_TRACKNUMBERS,AFHELPLIBRARY);
          
        default:
          break;
        }
      return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GRABBERSETUP_EXPERT,AFHELPLIBRARY);
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* Query the grabber path from the entryfield */     
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_GRABBERPATH),
                          sizeof(globalData.chrGrabberPath), globalData.chrGrabberPath);
      /* Query the grabber options from the entryfield */     
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_GRABBEROPTIONS),
                          sizeof(globalData.chrGrabberOptions), globalData.chrGrabberOptions);
      /* Query the cd-drive */
      WinQueryWindowText( WinWindowFromID(hwnd,IDLB_DROPDOWN),sizeof(chosenCD),chosenCD);
      /* Get the choosen grabber */
      /* text: "User defined" */
      getMessage(text, IDSTRS_DEFAULTUSERGRABBERNAME, sizeof(text), hSettingsResource, hwnd);
      /* Get the choosen grabber name */
      WinQueryWindowText(WinWindowFromID(hwnd,IDDD_GRABBERNAME),sizeof(text2), text2);
      if(strcmpi(text,text2)) {
        /* Predefined grabber. Get the name and save it. */
        WinQueryWindowText(WinWindowFromID(hwnd,IDDD_GRABBERNAME),
                           sizeof(globalData.chrGrabberName), globalData.chrGrabberName);
        /* Get the grabber ID and save it */
        sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
        hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
        if(hini)
          {
            globalData.iGrabberID=PrfQueryProfileInt(hini, globalData.chrGrabberName, "ID", IDGRABBER_UNKNOWN);
            PrfCloseProfile(hini);
          }
      }
      else {
        strcpy(globalData.chrGrabberName,"");/* unknown grabber */
        globalData.iGrabberID=IDGRABBER_UNKNOWN;
      }
      globalData.bTrackNumbers=WinQueryButtonCheckstate(hwnd,IDCB_TRACKNUMBERS);
      /* Let the WPS save the new instance data */
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_GRABBERBROWSE),QWL_USER);
      if(somIsObj(thisPtr)) {
        thisPtr->wpSaveDeferred();
      }
      /* Setup is done */
      GrabberSetupDone=TRUE;
      bInit=FALSE;
      return (MRESULT) TRUE;
    case WM_COMMAND:	
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_GRABBERBROWSE:
          /* User pressed the browse button */
          fd.cbSize = sizeof( fd );
          /* It's an centered 'Open'-dialog */
          fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
          /* Set the title of the file dialog */
          /* Title: "Search Audio-Grabber" */
          getMessage(text,IDSTR_FDLGSEARCHGRABBERTITLE,sizeof(text), hSettingsResource,hwnd);
          fd.pszTitle = text;
          /* Only show *.exe files */
          sprintf(fd.szFullFile,"%s","*.exe");
          
          if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
            {
              /* WinFileDlg failed */
              break;
            }
          if( fd.lReturn == DID_OK )
            {
              /* The user chose a grabber */
              WinSetWindowText( WinWindowFromID(hwnd,IDEF_GRABBERPATH), fd.szFullFile );
              /* Try to get the predefined grabber from choosen executable */
              if((ptr=strrchr(fd.szFullFile,'\\'))!=NULL) {
                ptr++;/* Executable name */
                strcpy(text2,ptr);
                if((ptr=strrchr(text2, '.'))==NULL)
                  break;
                *ptr=0;// Remove Extension
                /* Get count of listbox entries */
                sCount=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDDD_GRABBERNAME), LM_QUERYITEMCOUNT, NULL, NULL));
                for(a=0;a<sCount;a++) {
                  if(SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDDD_GRABBERNAME), LM_QUERYITEMTEXT,
                                             MPFROM2SHORT((SHORT)a,(SHORT) sizeof(text)), &text))) {
                    if(!strcmpi(text2,text)) {
                      //WinSendMsg(WinWindowFromID(hwnd,IDDD_GRABBERNAME), LM_SELECTITEM, (MPARAM)a, MPFROMSHORT(TRUE));// Select item
                      WinSetWindowText(WinWindowFromID(hwnd,IDDD_GRABBERNAME),text);
                      return (MRESULT) FALSE;
                    }
                  }
                }/* for() */
                /* No predefined grabber found. Set to user defined */
                /* text: "User defined" */
                getMessage(text, IDSTRS_DEFAULTUSERGRABBERNAME, sizeof(text), hSettingsResource, hwnd);
                WinSetWindowText(WinWindowFromID(hwnd,IDDD_GRABBERNAME),text);
              }
            }
          break;
        case IDPB_GRABBERUNDO:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_GRABBERPATH), globalData.chrGrabberPath);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_GRABBEROPTIONS), globalData.chrGrabberOptions);
          WinSetWindowText(WinWindowFromID(hwnd,IDLB_DROPDOWN),chosenCD);
          if(strlen(globalData.chrGrabberName)==0) {
            /* text: "User defined" */
            getMessage(text, IDSTRS_DEFAULTUSERGRABBERNAME, sizeof(text), hSettingsResource, hwnd);
            WinSetWindowText(WinWindowFromID(hwnd,IDDD_GRABBERNAME),text);
          }
          else
            WinSetWindowText(WinWindowFromID(hwnd,IDDD_GRABBERNAME), globalData.chrGrabberName);

          break;
        default:
          break;
        }
      return (MRESULT) FALSE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/****************************************************/
/*                                                  */ 
/* This procedure handles the grabber settings page */
/* in the settings notebook for normal mode.        */
/*                                                  */
/****************************************************/
MRESULT EXPENTRY settingsGrabberOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWCreatorSettings* thisPtr;
  int a;
  char chrCD[4];
  static BOOL bInit; 
  static  BOOL fScanning;
 
  switch(msg)
    {
    case WM_INITDLG:	
      bInit=FALSE;
      WinSetWindowULong(hwnd, QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      thisPtr=(CWCreatorSettings*)PVOIDFROMMP(mp2);

      WinSendMsg(WinWindowFromID(hwnd,IDEF_GRABBEROPTIONS), EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)sizeof(globalData.chrGrabberOptions)-1),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_GRABBEROPTIONS),globalData.chrGrabberOptions);
      //WinCheckButton(hwnd,IDCB_TRACKNUMBERS,globalData.bTrackNumbers);

      /* Fill the listbox with known CD drives */
      for( a=0; a<iNumCD ;a++) {

        chrCD[0]=cFirstCD+a;
        chrCD[1]=':';
        chrCD[2]=0;
        WinSendMsg(WinWindowFromID(hwnd,IDLB_DROPDOWN),LM_INSERTITEM,MPFROMSHORT(LIT_END),(MPARAM)chrCD);
        if(!a){
          if(!GrabberSetupDone)
            WinSetWindowText(WinWindowFromID(hwnd,IDLB_DROPDOWN),chrCD);
          else
            WinSetWindowText(WinWindowFromID(hwnd,IDLB_DROPDOWN),chosenCD);
        }
      }

      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_GRABBERHELP, 20);
      bInit=TRUE;

      /* Now scan the bus for new devices */
      WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDPB_SCAN), MPFROM2SHORT(CMDSRC_PUSHBUTTON, TRUE));

      return (MRESULT) TRUE;

      /* Show wait pointer while scanning is in progress */
    case WM_MOUSEMOVE:
      if(fScanning) {
        WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE) );
        return (MRESULT)TRUE;
      }
      break;

      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;

        if(WinQueryFocus(HWND_DESKTOP)!=
           WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT)) {
          mp2=MPFROMLONG(LONGFROMMP(mp2)|0x80000);/*AWP_ACTIVATE 0x00080000L*/
          mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
          return mr;  
        }
        break;
      }
    case WM_FOCUSCHANGE:
      {
        if(!SHORT1FROMMP(mp2)) {
          if(HWNDFROMMP(mp1)==hwnd) {
            MRESULT mr;

            mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS,
                       MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1))
        {
        case IDDD_DEVICESELECTION:
          /* User selected a new device */
          if(SHORT2FROMMP(mp1)==LN_SELECT && bInit) {
            char chrDevice[50];
            char chrTemp[50];
            char *chrPtr;
            /* Get the selected device  */
            WinQueryWindowText(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),sizeof(chrDevice), chrDevice);
            /* Rebuild the options in the hidden entry field */
            if((chrPtr=strchr(chrDevice, ':'))!=NULLHANDLE)
              *chrPtr=0;
            sprintf(chrTemp, "-D%s -H -t %%2", chrDevice);
            WinSetWindowText(WinWindowFromID(hwnd,IDEF_GRABBEROPTIONS), chrTemp);
          }
          break;
        default:
          break;
        }
      break;
    case WM_HELP:	
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd,QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {
        case IDPB_GRABBERUNDO:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GRABBERUNDO,AFHELPLIBRARY);
        default:
          break;
        }
      return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GRABBERSETUP,AFHELPLIBRARY);
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* Query the cd-drive */
      WinQueryWindowText( WinWindowFromID(hwnd,IDLB_DROPDOWN),sizeof(chosenCD),chosenCD);
      /* Query the grabber options from the entryfield */     
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_GRABBEROPTIONS),
                          sizeof(globalData.chrGrabberOptions), globalData.chrGrabberOptions);

      /* Add track numbers when using cdda2wav.exe */
      globalData.bTrackNumbers=1;
      /* Let the WPS save the new instance data */
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)) {
        thisPtr->wpSaveDeferred();
      }
      /* Setup is done */
      GrabberSetupDone=TRUE;
      bInit=FALSE;
      return (MRESULT) TRUE;

    case WM_APPTERMINATENOTIFY:
      switch(LONGFROMMP(mp1)) {
      case ACKEY_SCANBUS:
        {
          SHORT sItem=0;
          char chrOptions[sizeof(globalData.chrGrabberOptions)];
          char *chrPtr;
          /* scanbus.exe terminated. Set the entryfield now */

          strncpy(chrOptions, globalData.chrGrabberOptions, sizeof(globalData.chrGrabberOptions));
          chrOptions[sizeof(globalData.chrGrabberOptions)-1]=0;
          if((chrPtr=strstr(chrOptions, "-D"))!=NULLHANDLE) {
            char *chrEnd;
            /* found device setting in options */
            chrPtr++;
            chrPtr++;
            /* skip leading spaces */
            while(*chrPtr && *chrPtr==' ')
              chrPtr++;
            if((chrEnd=strchr(chrPtr, ' '))!=NULLHANDLE)
              *chrEnd=0;

            /* First check if our saved device is still installed */
            sItem=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDDD_DEVICESELECTION),
                                          LM_SEARCHSTRING, MPFROM2SHORT(LSS_PREFIX|LSS_CASESENSITIVE, LIT_FIRST),
                                          MPFROMP(chrPtr)));
            if(sItem==LIT_ERROR || sItem==LIT_NONE) {
              /* Device not found in list */
              sItem=0;
            }
          }
          if(SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDDD_DEVICESELECTION), LM_QUERYITEMTEXT,
                                     MPFROM2SHORT(sItem, sizeof(chrOptions)), MPFROMP(chrOptions))))
            WinSetWindowText(WinWindowFromID(hwnd,IDDD_DEVICESELECTION), chrOptions);
          /* The options are rebuilt in WM_CONTROL */
          /* Free shared mem if not already done */
          if(pvScanbusSharedMem) {
            DosFreeMem(pvScanbusSharedMem);
            pvScanbusSharedMem=NULL;
          }
          fScanning=FALSE;
          break;
        }
      case ACKEY_LISTBOX:
        if(LONGFROMMP(mp2)==0) {
          /* Delete entries in listbox */
          WinSetWindowText(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),"");
          WinSendMsg(WinWindowFromID(hwnd, IDDD_DEVICESELECTION),LM_DELETEALL,MPFROMLONG(0),MPFROMLONG(0));
        }
        else
          {
            WinSendMsg(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),LM_INSERTITEM,MPFROMSHORT(LIT_END),
                       mp2);
          }
        break;
      default:
        break;
      }/* switch */
      break; 

    case WM_COMMAND:	
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_SCAN:
          if(startScanbusHelper(hwnd))
            fScanning=TRUE;
          break;
        case IDPB_GRABBERUNDO:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDLB_DROPDOWN),chosenCD);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_GRABBEROPTIONS), globalData.chrGrabberOptions);
          break;
        default:
          break;
        }
      return (MRESULT) FALSE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}


/* This procedure handles the mkisofs settings page */ 
MRESULT EXPENTRY settingsMkisofsOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  FILEDLG fd = { 0 };
  CWCreatorSettings* thisPtr;
  ULONG ulFlags;
  char text[CCHMAXPATH];
  char text2[CCHMAXPATH];

  switch(msg)
    {
    case WM_INITDLG :	
      WinSetWindowULong(WinWindowFromID(hwnd,IDPB_MKISOFSBROWSE),
			QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      WinSendMsg(WinWindowFromID(hwnd,IDEF_MKISOFSPATH),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_MKISOFSPATH),chrMkisofs);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_MKISOFSOPTIONS),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_MKISOFSOPTIONS),chrMkisofsOptions);
      /* Codepage disabled? */
      if(bDisableCp)
        WinCheckButton(hwnd,IDCB_DISABLECODEPAGE,1);
      else
        WinCheckButton(hwnd,IDCB_DISABLECODEPAGE,0);

      /* Text: " Codepage for Joliet name creation: %d" */
      getMessage(text,IDSTRD_CODEPAGETEXT, sizeof(text), hSettingsResource,hwnd);

      sprintf(text2,text,iCodePage);
      WinSetWindowText(WinWindowFromID(hwnd,IDST_CODEPAGETEXT),text2);
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_MKISOFSHELP, 20);
      return (MRESULT) TRUE;
      /*    case WM_HELP:
			thisPtr=(CWAudioFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),QWL_USER);
			if(!thisPtr)
            break;
			switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
            {
            case IDEF_CDRECORDOPTIONS:
            return (MRESULT)thisPtr->wpDisplayHelp(IDEF_CDRECORDOPTIONS,AFHELPLIBRARY);
            case IDPB_WIDERRUFEN:
            return (MRESULT)thisPtr->wpDisplayHelp(IDPB_WIDERRUFEN,AFHELPLIBRARY);
            case IDPB_CDRECORDBROWSE:
            return (MRESULT)thisPtr->wpDisplayHelp(IDPB_CDRECORDBROWSE,AFHELPLIBRARY);
            case IDEF_CDRECORDPATH:
            return (MRESULT)thisPtr->wpDisplayHelp(IDEF_CDRECORDPATH,AFHELPLIBRARY);
            default:
            break;
            }
			return (MRESULT)thisPtr->wpDisplayHelp(IDDLG_CDRECORDSETUP,AFHELPLIBRARY);
			break;*/
      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;

        if(WinQueryFocus(HWND_DESKTOP)!=
           WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT)) {
          mp2=MPFROMLONG(LONGFROMMP(mp2)|0x80000);/*AWP_ACTIVATE 0x00080000L*/
          mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
          return mr;  
        }
        break;
      }
    case WM_FOCUSCHANGE:
      {
        if(!SHORT1FROMMP(mp2)) {
          if(HWNDFROMMP(mp1)==hwnd) {
            MRESULT mr;

            mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }

#if 0
    case WM_WINDOWPOSCHANGED:
      {
        /* This prevents switching the notebook page behind the open folder */
        if(!(WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_FLAGS) & FF_ACTIVE))
          mp2=MPFROMLONG(0x80000);
        break;
      }
#endif
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* Query the cdrecord path from the entryfield */     
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_MKISOFSPATH),sizeof(chrMkisofs),chrMkisofs);
      /* Query the cdrecord options from the entryfield */     
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_MKISOFSOPTIONS),sizeof(chrMkisofsOptions),chrMkisofsOptions);
      ulFlags=0;
      lMKOptions=ulFlags;
      if(WinQueryButtonCheckstate(hwnd,IDCB_DISABLECODEPAGE)&1)
        bDisableCp=TRUE;
      else
        bDisableCp=FALSE;
      /* Let the WPS save the new instance data */
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_MKISOFSBROWSE),QWL_USER);
      if(somIsObj(thisPtr)) {
        /*if(WinQueryButtonCheckstate(hwnd,IDCB_LOGFILE)&1)
           thisPtr->cwSetMkisofsFlags(IDMK_LOGFILE,IDMK_LOGFILE);
           else
           thisPtr->cwSetMkisofsFlags(0,IDMK_LOGFILE); */
        thisPtr->wpSaveDeferred();
      }
      /* Setup is done */
      MkisofsSetupDone=TRUE;
      return (MRESULT) TRUE;
    case WM_COMMAND:	
    switch(SHORT1FROMMP(mp1))
      {
      case IDPB_MKISOFSBROWSE:
	/* User pressed the browse button */
	fd.cbSize = sizeof( fd );
	/* It's an centered 'Open'-dialog */
	fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
	/* Set the title of the file dialog */
    /* Text: "Search mkisofs" */
    getMessage(text,IDSTRD_FDLGSEARCHMKISOFS, sizeof(text), hSettingsResource,hwnd);
	fd.pszTitle = text;
	/* Only show *.exe files */
	sprintf(fd.szFullFile,"%s","*.exe");

	if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
	  {
	    /* WinFileDlg failed */
	    break;
	  }
	if( fd.lReturn == DID_OK )
	  {
	    WinSetWindowText( WinWindowFromID(hwnd,IDEF_MKISOFSPATH), fd.szFullFile );
	  }
	break;
      case IDPB_MKISOFSUNDO:
	/* User pressed the UNDO button */

 	WinSetWindowText(WinWindowFromID(hwnd,IDEF_MKISOFSPATH),chrMkisofs);
	WinSetWindowText(WinWindowFromID(hwnd,IDEF_MKISOFSOPTIONS),chrMkisofsOptions);
    /*  
          thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_MKISOFSBROWSE),QWL_USER);
          if(thisPtr) {
          if(thisPtr->cwQueryMkisofsFlags()&IDMK_LOGFILE)
          WinCheckButton(hwnd,IDCB_LOGFILE,1);
          else
          WinCheckButton(hwnd,IDCB_LOGFILE,0);
          } */
	break;
      default:
	break;
      }
      return (MRESULT) TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}


/****************************************************/
/*                                                  */ 
/* This procedure handles the CDDB settings page    */
/* in the settings notebook                         */
/*                                                  */
/****************************************************/
MRESULT EXPENTRY settingsCDDBOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  FILEDLG fd = { 0 };
  CWCreatorSettings* thisPtr;
  int a;
  char chrCD[4];
  char text[CCHMAXPATH]; 
 
  switch(msg)
    {
    case WM_INITDLG :
      {
        char chrUser[CCHMAXPATH];
        WinSetWindowULong(hwnd,QWL_USER,LONGFROMMP(mp2));//Save object ptr.

#if 0        
        WinSendMsg(WinWindowFromID(hwnd,IDEF_CDDBUSER),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)sizeof(chrCDDBUser)),0);
        WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDDBUSER),chrCDDBUser);
        WinSendMsg(WinWindowFromID(hwnd,IDEF_USERHOST),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)sizeof(chrCDDBUserHost)),0);
        WinSetWindowText(WinWindowFromID(hwnd,IDEF_USERHOST),chrCDDBUserHost);
#endif
   
        /* E-mail entry field */
        WinSendMsg(WinWindowFromID(hwnd,IDEF_USEREMAIL),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT) sizeof(chrUser)),0);
        /* Replace the following with snprintf() */
        strncpy(chrUser, chrCDDBUser, sizeof(chrUser));
        chrUser[sizeof(chrUser)-1]=0;
        strncat(chrUser, "@", sizeof(chrUser)-strlen(chrUser));
        strncat(chrUser, chrCDDBUserHost, sizeof(chrUser)-strlen(chrUser));
        WinSetWindowText(WinWindowFromID(hwnd,IDEF_USEREMAIL), chrUser);

        if(bUseCDDB)
          WinCheckButton(hwnd,IDCB_USECDDB,1);
        else {
          WinCheckButton(hwnd,IDCB_USECDDB,0);
          WinEnableWindow(WinWindowFromID(hwnd,IDEF_USEREMAIL), FALSE);
        }
        /* Move default buttons on Warp 4 */
        cwMoveNotebookButtonsWarp4(hwnd, IDPB_CDDBHELP, 20);
        return (MRESULT) TRUE;
      }
      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;

        if(WinQueryFocus(HWND_DESKTOP)!=
           WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT)) {
          mp2=MPFROMLONG(LONGFROMMP(mp2)|0x80000);/*AWP_ACTIVATE 0x00080000L*/
          mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
          return mr;  
        }
        break;
      }
    case WM_FOCUSCHANGE:
      {
        if(!SHORT1FROMMP(mp2)) {
          if(HWNDFROMMP(mp1)==hwnd) {
            MRESULT mr;

            mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
#if 0
    case WM_WINDOWPOSCHANGED:
      {
        /* This prevents switching the notebook page behind the open folder */
        if(!(WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_FLAGS) & FF_ACTIVE))
          mp2=MPFROMLONG(0x80000);
        break;
      }
#endif
    case WM_HELP:
      //thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDDBUNDO),QWL_USER);
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd,QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {
        case IDPB_CDDBUNDO:
          return (MRESULT)thisPtr->wpDisplayHelp(1301,AFHELPLIBRARY);
        case IDCB_USECDDB:
          return (MRESULT)thisPtr->wpDisplayHelp(1302,AFHELPLIBRARY);
#if 0
        case IDEF_CDDBUSER:
          return (MRESULT)thisPtr->wpDisplayHelp(1303,AFHELPLIBRARY);
        case IDEF_USERHOST:
          return (MRESULT)thisPtr->wpDisplayHelp(1306,AFHELPLIBRARY);
#endif
        default:
          break;
        }
      return (MRESULT)thisPtr->wpDisplayHelp(1300,AFHELPLIBRARY);
      break;
    case WM_DESTROY:
      {
        char chrUser[CCHMAXPATH];
        char * chrPtr;

        /* The notebook closes and gets destroyed */
        /* Set focus to desktop to prevent PM freeze */
        WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);
        
        //       WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDDBUSER),sizeof(chrCDDBUser),chrCDDBUser);
        //     WinQueryWindowText( WinWindowFromID(hwnd,IDEF_USERHOST),sizeof(chrCDDBUserHost),chrCDDBUserHost);
        WinQueryWindowText( WinWindowFromID(hwnd,IDEF_USEREMAIL),sizeof(chrUser),chrUser);
        if((chrPtr=strchr(chrUser, '@'))!=NULLHANDLE) {
          *chrPtr=0;
          strncpy(chrCDDBUser, chrUser, sizeof(chrCDDBUser));
          chrCDDBUser[sizeof(chrCDDBUser)-1]=0;

          strncpy(chrCDDBUserHost, ++chrPtr, sizeof(chrCDDBUserHost));
          chrCDDBUserHost[sizeof(chrCDDBUserHost)-1]=0;
        }
        if(WinQueryButtonCheckstate(hwnd,IDCB_USECDDB)&1)
          bUseCDDB=TRUE;
        else
          bUseCDDB=FALSE;
        /* Let the WPS save the new instance data */
        //thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd, IDPB_CDDBUNDO),QWL_USER);
        thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd,QWL_USER);
        if(somIsObj(thisPtr)) {
          thisPtr->wpSaveImmediate();
        }
        /* Setup is done */
        return (MRESULT) TRUE;
      }
    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1))
        {
        case IDCB_USECDDB:
          if(WinQueryButtonCheckstate(hwnd,IDCB_USECDDB) & 1)
            WinEnableWindow(WinWindowFromID(hwnd,IDEF_USEREMAIL), TRUE);
          else
            WinEnableWindow(WinWindowFromID(hwnd,IDEF_USEREMAIL), FALSE);
          break;
        default:
          break;
        }
      break;
    case WM_COMMAND:	
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_CDDBUNDO:
          {
            char chrUser[CCHMAXPATH];
            /* User pressed the UNDO button */
            //  WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDDBUSER),chrCDDBUser);
            //   WinSetWindowText(WinWindowFromID(hwnd,IDEF_USERHOST),chrCDDBUserHost);
            strncpy(chrUser, chrCDDBUser, sizeof(chrUser));
            chrUser[sizeof(chrUser)-1]=0;
            strncat(chrUser, "@", sizeof(chrUser)-strlen(chrUser));
            strncat(chrUser, chrCDDBUserHost, sizeof(chrUser)-strlen(chrUser));
            WinSetWindowText(WinWindowFromID(hwnd,IDEF_USEREMAIL), chrUser);
            
            if(bUseCDDB)
              WinCheckButton(hwnd,IDCB_USECDDB,1);
            else
              WinCheckButton(hwnd,IDCB_USECDDB,0);
            break;
          }
        case IDPB_CDDBHELP:
          //thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd, IDPB_CDDBUNDO),QWL_USER);
          thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd,QWL_USER);
          if(!somIsObj(thisPtr))
            break;
          return (MRESULT)thisPtr->wpDisplayHelp(1300,AFHELPLIBRARY);
          break;
        default:
          break;
        }
      return (MRESULT) TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/****************************************************/
/*                                                  */ 
/* This procedure handles the CDDB settings page 2  */
/* in the settings notebook                         */
/*                                                  */
/****************************************************/
MRESULT EXPENTRY settingsCDDBOptionDlgProc2(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  FILEDLG fd = { 0 };
  CWCreatorSettings* thisPtr;
  int a;
  char chrCD[4];
  char text[CCHMAXPATH]; 
 
  switch(msg)
    {
    case WM_INITDLG :	
      //  WinSetWindowULong(WinWindowFromID(hwnd,IDPB_WIDERRUFEN),
      //                QWL_USER,(ULONG)PVOIDFROMMP(mp2));
      WinSetWindowULong(hwnd,QWL_USER,LONGFROMMP(mp2));//Save object ptr.

      for(a=0;a<NUMSERVERS;a++) {
        WinInsertLboxItem(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LIT_END,
                          cddbServer[a]);
      }
      /* Preselect the first item */
      WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_SELECTITEM,MPFROMSHORT(0),MPFROMSHORT(TRUE));
      WinSendMsg(WinWindowFromID(hwnd,IDEF_NEWSERVER),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)sizeof(cddbServer[0])),0);
      if(MAXSERVERS==NUMSERVERS)
        WinEnableWindow(WinWindowFromID(hwnd,IDPB_SERVERADD),FALSE);
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_CDDB2HELP, 20);
      return (MRESULT) TRUE;
      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;

        if(WinQueryFocus(HWND_DESKTOP)!=
           WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT)) {
          mp2=MPFROMLONG(LONGFROMMP(mp2)|0x80000);/*AWP_ACTIVATE 0x00080000L*/
          mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
          return mr;  
        }
        break;
      }
    case WM_FOCUSCHANGE:
      {
        if(!SHORT1FROMMP(mp2)) {
          if(HWNDFROMMP(mp1)==hwnd) {
            MRESULT mr;

            mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
#if 0
    case WM_WINDOWPOSCHANGED:
      {
        /* This prevents switching the notebook page behind the open folder */
        if(!(WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_FLAGS) & FF_ACTIVE))
          mp2=MPFROMLONG(0x80000);
        break;
      }
#endif
    case WM_HELP:	
      //      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_WIDERRUFEN),QWL_USER);
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd,QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {

        default:
          break;
        }
      return (MRESULT)thisPtr->wpDisplayHelp(1307,AFHELPLIBRARY);
      break;
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      NUMSERVERS=(int)LONGFROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS), LM_QUERYITEMCOUNT,0,0));
      for(a=0;a<NUMSERVERS;a++) {
        strcpy(text,cddbServer[a]);
        WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_QUERYITEMTEXT,
                   MPFROM2SHORT(a,sizeof(cddbServer[0])),cddbServer[a]);
      }
      
      /* Let the WPS save the new instance data */
      //      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_WIDERRUFEN),QWL_USER);
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)) {
        thisPtr->wpSaveImmediate();
      }
      /* Setup is done */
      return (MRESULT) TRUE;
    case WM_COMMAND:	
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_CDDB2UNDO:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_NEWSERVER),"");
          /* Server listbox */
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_DELETEALL,
                     0,0);
          for(a=0;a<NUMSERVERS;a++) {
            WinInsertLboxItem(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LIT_END,
                              cddbServer[a]);
          }
          /* Preselect the first item */
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_SELECTITEM,MPFROMSHORT(0),MPFROMSHORT(TRUE));
          if(MAXSERVERS==NUMSERVERS)
            WinEnableWindow(WinWindowFromID(hwnd,IDPB_SERVERADD),FALSE);
          break;
        case IDPB_SERVERDELETE:
          a=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0));
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_DELETEITEM,MPFROMSHORT(a),0);
          if(MAXSERVERS==NUMSERVERS)
            WinEnableWindow(WinWindowFromID(hwnd,IDPB_SERVERADD),TRUE);      
          break;
        case IDPB_SERVERADD:
          WinQueryWindowText( WinWindowFromID(hwnd,IDEF_NEWSERVER),sizeof(text),text);
          if(!strchr(text,':'))
            break;
          WinInsertLboxItem(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LIT_END,
                            text);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_NEWSERVER),"");
          if(MAXSERVERS==(int)LONGFROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS), LM_QUERYITEMCOUNT,0,0)))
            WinEnableWindow(WinWindowFromID(hwnd,IDPB_SERVERADD),FALSE);
          break;
        case IDPB_SERVERUP:
          /* Get pos of selection */
          a=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0));
          if(a==0)
            break;
          /* Query item text */
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_QUERYITEMTEXT,
                     MPFROM2SHORT(a,sizeof(text)), text);
          /* Delete at current position */
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_DELETEITEM,MPFROMSHORT(a),0);
          /* Insert at new position */
          WinInsertLboxItem(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),a-1,
                            text);
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_SELECTITEM,MPFROMSHORT(a-1),MPFROMSHORT(TRUE));
          break;
        case IDPB_SERVERDOWN:
          /* Get pos of selection */
          a=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0));
          if(a==(int)(LONGFROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS), LM_QUERYITEMCOUNT,0,0))-1))
            break; 
         /* Query item text */
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_QUERYITEMTEXT,
                     MPFROM2SHORT(a,sizeof(text)), text);
          /* Delete at current position */
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_DELETEITEM,MPFROMSHORT(a),0);
          /* Insert at new position */
          WinInsertLboxItem(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),a+1,
                            text);
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CDDBSERVERS),LM_SELECTITEM,MPFROMSHORT(a+1),MPFROMSHORT(TRUE));
          break;
        case IDPB_CDDB2HELP:
          //thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_WIDERRUFEN),QWL_USER);
          thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd,QWL_USER);
          if(!somIsObj(thisPtr))
            break;
          return (MRESULT)thisPtr->wpDisplayHelp(1307,AFHELPLIBRARY);
        default:
          break;
        }
      return (MRESULT) TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/****************************************************/


/* Fly over help */
#define xVal  12      //x-distance of Bubble
#define yVal  8      //y-distance of Bubble
#define shadowDeltaX 25
#define shadowDeltaY 25

#if 0
PFNWP g_oldBubbleProc; /* Default proc for flyover frame */
/* New frame proc for the flyover window handling the removal of the shadow */
MRESULT EXPENTRY fnwpBubbleProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg)
    {
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;
        PSWP pswp=(PSWP)PVOIDFROMMP(mp1);

        if(pswp->fl & SWP_HIDE)
          {
            RECTL rcl;
            pswp++;

            rcl.xRight=pswp->x+pswp->cx+shadowDeltaX;
            rcl.xLeft=pswp->x;
            rcl.yBottom=pswp->y-shadowDeltaY;
            rcl.yTop=pswp->y+pswp->cy;
            pswp--;

            WinInvalidateRect(HWND_DESKTOP, &rcl, TRUE);
          }
        break;
      }
    default:
      break;
    }  
  return g_oldBubbleProc(hwnd,msg,mp1,mp2);
}

void setFlyOverText(HWND hwndBubbleWindow, char * winText)
{
  POINTL ptl;
  POINTL aptlPoints[TXTBOX_COUNT];
  ULONG  ulLen;
  HPS hps;
  RECTL rcl;
  LONG   deltaX, deltaY;
  LONG cy=0;
  LONG cx=0;
  int a=1;
  char *pBuchst;
  char *pRest;

  if(!winText)
    return;
  
  ulLen=strlen(winText);
  if(!ulLen)
    return;

  WinSetWindowText(WinWindowFromID(hwndBubbleWindow, FID_CLIENT), winText);
  
  /* Calculate text size in pixel */
  hps=WinGetPS(WinWindowFromID(hwndBubbleWindow, FID_CLIENT));
  pRest=winText;
  while((pBuchst=strchr(pRest, 0xa))!=NULL) {
    a++;
    /* Get size of this line */
    GpiQueryTextBox(hps, pBuchst-pRest, pRest, TXTBOX_COUNT, aptlPoints);
    cx=(aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x 
        > cx 
        ? aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x 
        : cx);

    cy=(aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y
        > cy 
        ? aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y
        : cy) ;

    *pBuchst=0;
    pBuchst++;
    pRest=pBuchst;
    //if(a>50)
    //DosBeep(100, 500);
  }

  GpiQueryTextBox(hps, strlen(pRest), pRest, TXTBOX_COUNT, aptlPoints);
  cx=(aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x 
      > cx 
      ? aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x 
      : cx);
          
  cy=(aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y
      > cy 
      ? aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y
      : cy) ;

  WinReleasePS(hps);
 
  /* Calculate bubble positon and show bubble */
  WinQueryPointerPos(HWND_DESKTOP,&ptl);//Query pointer position in the desktop window
  WinQueryWindowRect(HWND_DESKTOP,&rcl);//Query desktop size

  deltaX=(cx+7+xVal+ptl.x 
          > rcl.xRight 
          ? -cx-xVal-xVal-7 
          : 0) ;
  deltaY=(cy*a+2+yVal+ptl.y 
          > rcl.yTop 
          ? -cy*a-2*yVal-7
          : 0) ;

  WinSetWindowPos(hwndBubbleWindow,
                  HWND_TOP,
                  ptl.x+xVal+deltaX, ptl.y+yVal+deltaY,
                  cx+8,
                  cy*a+2,
                  SWP_ZORDER | SWP_SIZE | SWP_MOVE /*| SWP_SHOW*/);
  WinShowWindow(hwndBubbleWindow,
                TRUE);
}
#endif

/*************************************************************/
/* This dialog procedure handles the toolbar page            */
/*************************************************************/			
MRESULT EXPENTRY settingsToolbarOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
  HINI hini;
  HOBJECT  hObject;
  ULONG  attrFound;
  ULONG  len;
  char text[250];
  char title[250];
  CWCreatorSettings* thisPtr;
  
  switch(msg) {
  case WM_INITDLG :	
       
    WinSetWindowULong(hwnd,QWL_USER,LONGFROMMP(mp2));
    
    /* We have to initialize the dialog controls with the approbiate values */
    // Set the focus on the demo area
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD));

    // Set the background colour of the demo area
    WinSetPresParam(WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD),
		    PP_BACKGROUNDCOLOR,(ULONG)sizeof(rgbTBFlyBackground), &rgbTBFlyBackground );
    // Set the foreground colour of the demo area
    WinSetPresParam(WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD),
		    PP_FOREGROUNDCOLOR,(ULONG)sizeof(rgbTBFlyForeground), &rgbTBFlyForeground );

    // Set the font of the demo area
    WinSetPresParam(WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD),
		    PP_FONTNAMESIZE,(ULONG)sizeof(chrTBFlyFontName),
		    chrTBFlyFontName );

    if(bTBFlyOverEnabled) {
      WinCheckButton(hwnd, IDCB_ENABLETBFLYOVER, 1);
      WinEnableWindow(WinWindowFromID(hwnd, IDSB_TBFLYOVERDELAY),TRUE);
    }
    else {
      WinCheckButton(hwnd, IDCB_ENABLETBFLYOVER, 0);
      WinEnableWindow(WinWindowFromID(hwnd, IDSB_TBFLYOVERDELAY),FALSE);
    }

    // Set the linits for the delay spin button
    WinSendMsg(WinWindowFromID(hwnd, IDSB_TBFLYOVERDELAY),
	       SPBM_SETLIMITS,(MPARAM)MAXDELAY,(MPARAM)0);
    // Set the current value for delay
    WinSendMsg(WinWindowFromID(hwnd, IDSB_TBFLYOVERDELAY),
	       SPBM_SETCURRENTVALUE,
	       (MPARAM)iTBFlyOverDelay,
	       (MPARAM)NULL);

    /* Move default buttons on Warp 4 */
    cwMoveNotebookButtonsWarp4(hwnd, IDPB_COLORHELP, 20);
    return (MRESULT) TRUE;
      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;

        if(WinQueryFocus(HWND_DESKTOP)!=
           WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT)) {
          mp2=MPFROMLONG(LONGFROMMP(mp2)|0x80000);/*AWP_ACTIVATE 0x00080000L*/
          mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
          return mr;  
        }
        break;
      }
    case WM_FOCUSCHANGE:
      {
        if(!SHORT1FROMMP(mp2)) {
          if(HWNDFROMMP(mp1)==hwnd) {
            MRESULT mr;

            mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }

#if 0
  case WM_WINDOWPOSCHANGED:
    {
      /* This prevents switching the notebook page behind the open folder */
      if(!(WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_FLAGS) & FF_ACTIVE) ) {
        PSWP pswp=(PSWP)PVOIDFROMMP(mp1);
        if(pswp->fl & SWP_ZORDER)
          mp2=MPFROMLONG(0x80000);
      }
      break;
    }
#endif
  case WM_DESTROY:
    /* The notebook closes and gets destroyed */
    /* Set focus to desktop to prevent PM freeze */
    WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

    // Query the current background colour
    len=WinQueryPresParam(WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD),
			  PP_BACKGROUNDCOLOR,0,&attrFound,sizeof(rgbTBFlyBackground),
			  &rgbTBFlyBackground,QPF_NOINHERIT);
    // Query the current font
    len=WinQueryPresParam(WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD),
                          PP_FONTNAMESIZE,0,&attrFound,sizeof(chrTBFlyFontName),
                          chrTBFlyFontName,QPF_NOINHERIT);
    // Query the current foreground colour
    len=WinQueryPresParam(WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD),
                          PP_FOREGROUNDCOLOR,0,&attrFound,sizeof(rgbTBFlyForeground),
                          &rgbTBFlyForeground,QPF_NOINHERIT);
    // Query the enable state
    if(WinQueryButtonCheckstate(hwnd,IDCB_ENABLETBFLYOVER) & 1)
      bTBFlyOverEnabled=1;
    else
      bTBFlyOverEnabled=0;

    // Query delay value
    WinSendMsg(WinWindowFromID(hwnd, IDSB_TBFLYOVERDELAY),
               SPBM_QUERYVALUE,(MPARAM)&iTBFlyOverDelay,
               MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));				
    
    // Save data
    thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd , QWL_USER);

    if(somIsObj(thisPtr))
      thisPtr->wpSaveImmediate();
    
    break;
  case WM_CONTROL:
    /* The window controls send WM_CONTROL messages */
    switch(SHORT1FROMMP(mp1))
      {
      case IDCB_ENABLETBFLYOVER:
        if(WinQueryButtonCheckstate(hwnd,IDCB_ENABLETBFLYOVER) & 1)
          WinEnableWindow(WinWindowFromID(hwnd, IDSB_TBFLYOVERDELAY),1);
        else
          WinEnableWindow(WinWindowFromID(hwnd, IDSB_TBFLYOVERDELAY),0);
        break;
      default:
        break;
      } // end switch(SHORT1FROMMP(mp1))
    break;
			
  case WM_COMMAND :
    switch (SHORT1FROMMP(mp1)) {
      // Process commands here //
      //    case IDPB_COLORHELP:
#if 0
    case WM_HELP:
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd , QWL_USER);
      
      if(somIsObj(thisPtr)) {
        DosBeep(5000,1000);
        thisPtr->wpDisplayHelp(2000,AFHELPLIBRARY);

      }
      break;
#endif
    case IDPB_COLORUNDO:
      /* The undo button was clicked */
      // Set the background colour of the demo area
      WinSetPresParam(WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD),
                      PP_BACKGROUNDCOLOR,(ULONG)sizeof(rgbTBFlyBackground), &rgbTBFlyBackground );
      // Set the foreground colour of the demo area
      WinSetPresParam(WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD),
                      PP_FOREGROUNDCOLOR,(ULONG)sizeof(rgbTBFlyForeground), &rgbTBFlyForeground );
      
      // Set the font of the demo area
      WinSetPresParam(WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD),
                      PP_FONTNAMESIZE,(ULONG)sizeof(chrTBFlyFontName),
                      chrTBFlyFontName );
      
      if(bTBFlyOverEnabled) {
        WinCheckButton(hwnd, IDCB_ENABLETBFLYOVER, 1);
        WinEnableWindow(WinWindowFromID(hwnd, IDSB_TBFLYOVERDELAY),TRUE);
      }
      else {
        WinCheckButton(hwnd, IDCB_ENABLETBFLYOVER, 0);
        WinEnableWindow(WinWindowFromID(hwnd, IDSB_TBFLYOVERDELAY),FALSE);
      }
      
      // Set the current value for delay
      WinSendMsg(WinWindowFromID(hwnd, IDSB_TBFLYOVERDELAY),
                 SPBM_SETCURRENTVALUE,
                 (MPARAM)iTBFlyOverDelay,
                 (MPARAM)NULL);
      break;
    case IDPB_COLORSTANDARD:
      /* The default button was clicked */
      rgbTBFlyBackground.bBlue=180;  // Set the default colours
      rgbTBFlyBackground.bGreen=255; 
      rgbTBFlyBackground.bRed=255;
      rgbTBFlyForeground.bBlue=0;
      rgbTBFlyForeground.bGreen=0;			
      rgbTBFlyForeground.bRed=0;
      // Set the default font
      if(cwQueryOSRelease()>=40)
        strcpy(chrTBFlyFontName,DEFAULT_DIALOG_FONT);
      else
        strcpy(chrTBFlyFontName,DEFAULT_DIALOG_FONT_WARP3);

      // Set the background colour of the demo area
      WinSetPresParam(WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD),
                      PP_BACKGROUNDCOLOR,(ULONG)sizeof(rgbTBFlyBackground), &rgbTBFlyBackground );
      // Set the foreground colour of the demo area
      WinSetPresParam(WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD),
                      PP_FOREGROUNDCOLOR,(ULONG)sizeof(rgbTBFlyForeground), &rgbTBFlyForeground );
      // Set the font of the demo area
      WinSetPresParam(WinWindowFromID(hwnd, IDST_TEXTDEMOFIELD),
                      PP_FONTNAMESIZE,(ULONG)sizeof(chrTBFlyFontName),
                      chrTBFlyFontName );

      WinCheckButton(hwnd, IDCB_ENABLETBFLYOVER, 1);
      WinEnableWindow(WinWindowFromID(hwnd, IDSB_TBFLYOVERDELAY),TRUE);
      
      // Set the value for delay
      WinSendMsg(WinWindowFromID(hwnd, IDSB_TBFLYOVERDELAY),
                 SPBM_SETCURRENTVALUE,
                 (MPARAM)DEFAULTDELAY,
                 (MPARAM)NULL);

      break;
    case IDPB_COLORPALETTE:
      /* Colour... button was clicked */
      // Open the colorpalette 
      if((hObject=WinQueryObject("<WP_HIRESCLRPAL>"))!=NULL) {
        WinOpenObject(hObject,OPEN_DEFAULT,TRUE);
      }
      else {//Error, can't open the palette
        /*  Show an error msg.						   */
        /*
          Text:   "Can't open the color palette. It may have lost it's ID. Check your INI files."
          Title: "CD-Creator settings"                       
          */             
        messageBox( text, IDSTRS_NOCOLORPALETTE , sizeof(text),
                    title, IDSTRS_CREATORSETTINGS , sizeof(title),
                    hSettingsResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);    
      }
      break;
    case IDPB_FONTPALETTE:
      /* Font... button was clicked */
      // Open the fontpalette 
      if((hObject=WinQueryObject("<WP_FNTPAL>"))!=NULL) {
        WinOpenObject(hObject,OPEN_DEFAULT,TRUE);
      }
      else {//Error, can't open the palette
        /*  Show an error msg.						   */
        /*
          Text:   "Can't open the font palette. It may have lost it's ID. Check your INI files."
          Title: "CD-Creator settings"                       
          */             
        messageBox( text, IDSTRS_NOFONTPALETTE , sizeof(text),
                    title, IDSTRS_CREATORSETTINGS , sizeof(title),
                    hSettingsResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
      }
      break;
    }
    /* Don't call WinDefDlgProc here, or the dialog gets closed */
    return (MRESULT) TRUE;
  default:
    break;
  }
  // The WinDefDlgProc() handles the rest of the messages
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
