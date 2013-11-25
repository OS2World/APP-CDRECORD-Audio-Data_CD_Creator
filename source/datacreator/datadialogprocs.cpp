/*
 * Dialog procedures for the Data-CD-Creator class. Settings page procs,
 * frame extension dialog procs etc.
 *
 * This file is (C) Chris Wohlgemuth 1999-2004
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

#define INCL_WINSTDDRAG

#include "datafolder.h"
#include "audiofolder.hh"
#include "audiofolderhelp.h"

#include "launchpad.hh"
#include "menufolder.hh"

#pragma SOMAsDefault(off)
#include <sys\types.h>
#include <sys\stat.h>

#pragma SOMAsDefault(pop)

#include <stdio.h>
#include <stdlib.h>

extern PVOID pvScanbusSharedMem;
extern char chrInstallDir[CCHMAXPATH];
extern int iBus;
extern int iTarget;
extern int iLun;
extern int iSpeed;
extern HMODULE hDataResource;
extern HPOINTER hPtrTBIcons[];
extern LPList* lplAllLPads;

extern PFNWP pfnwpDataGenericFrame; /* see dataoveriddenwpmethods.cpp */
extern PFNWP  oldButtonProc2;  //place for original button-procedure
extern HWND hwndHintDaemon;

extern bHintEnabled;

/* For custom BG */
extern LOADEDBITMAP allBMPs[NUM_CTRL_IDX];
extern BOOL bUseCustomPainting;
extern PFNWP  oldStaticTextProc;

/* We use this to tell the container not to show a hint if editing is in progress */
extern ATOM atomUpdateStatusbar;

ULONG cwQueryOSRelease();
void SysWriteToTrapLog(const char* chrFormat, ...);
ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);
void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
MRESULT cwInsertMenuItem(int iPosition, HWND hwndMenu, HWND hwndSubMenu, int iID, char * chrText);
MRESULT cwInsertMenuSeparator(int iPosition, HWND hwndMenu, HWND hwndSubMenu);
ULONG launchWrapper(PSZ parameter, PSZ folderPath,HWND hwnd, PSZ wrapperExe, PSZ title="CDRecord/2");
void setupToolBarButtons(HWND hwnd);
BOOL winhAssertWarp4Notebook(HWND hwndDlg,
                    USHORT usIdThreshold,    // in: ID threshold
                    ULONG ulDownUnits);       // in: dialog units or 0
BOOL checkImageName(CWDataFolder *thisPtr, HWND hwnd, BOOL bCheckExist);
MRESULT EXPENTRY newButtonProc(HWND hwnd, ULONG msg,MPARAM mp1,MPARAM mp2 );
void _Optlink toolsThreadFunc (void *arg);
void writeLogPrintf(char* logFile, char* format, ...);
/* New static text drawing proc */
extern MRESULT EXPENTRY staticTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void setupCheckBoxControl(  HWND hwnd, USHORT id);
void setupRadioButtonControl(HWND hwnd, USHORT id);
void setupGroupBoxControl(HWND hwnd, USHORT id);
BOOL HlpPaintFrame(HWND hwnd, BOOL bInclBorder);

/* Local */
static void enableCDTypeCntrls(HWND hwnd, ULONG ulFlags);
MRESULT EXPENTRY imageNameDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY dataCDStatusLineDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY msInfoStatusDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
ULONG launchPMWrapper(PSZ parameter, PSZ folderPath, PSZ wrapperExe, PSZ pszTitle);

typedef BOOL   SOMLINK somTP_CWMenuFolder_mfCheckMenuItems(CWMenuFolder *somSelf,
		WPObject* wpObject,
		ULONG ulMenuId);
typedef somTP_CWMenuFolder_mfCheckMenuItems *somTD_CWMenuFolder_mfCheckMenuItems;


MRESULT EXPENTRY selectWriterDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  char *textPtr;
  char *textPtr2; 
  DEVICEINFO *devInfo;

  switch (msg)
    {      
    case WM_PAINT:
      {
        if(HlpPaintFrame(hwnd, TRUE))
          return (MRESULT)0;
        break;
      }
    case WM_INITDLG:   
      WinSetWindowULong(hwnd,QWL_USER,(ULONG)PVOIDFROMMP(mp2));
      WinShowWindow(WinWindowFromID(hwnd,IDPB_STATUSOK),TRUE);
      WinEnableWindow(WinWindowFromID(hwnd,IDPB_STATUSOK),FALSE);
      WinShowWindow(WinWindowFromID(hwnd,IDPB_ABORTWRITE), TRUE);

      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }

      /* Set custom procs */
      /* subclass static text control for custom painting */
      oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_SELECTDEVICE),staticTextProc);
      /* Subclass groupbox */
      setupGroupBoxControl(  hwnd,  IDGB_SELECTWRITER);
      setupGroupBoxControl(  hwnd,  IDGB_SELECTSPEED);

      /* Speed spin button */
      WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETLIMITS,MPFROMLONG(64),MPFROMLONG(1));
      WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETCURRENTVALUE,(MPARAM)iSpeed,MPFROMLONG(0));
      
      WinSetWindowPos(hwnd,HWND_TOP,0,0,0,0,SWP_ZORDER|SWP_ACTIVATE|SWP_SHOW);

      /* Fill drop down listbox with the available devices by calling scanbus */
      
      /* Allocate shared mem for communication between helper and settings notebook */
      if(!pvScanbusSharedMem) {
        if(DosAllocSharedMem(&pvScanbusSharedMem,SCANBUSSHAREDMEM_NAME, SCANBUSSHAREDMEM_SIZE,
                             PAG_READ|PAG_WRITE|PAG_COMMIT)) {
          /*
            Text:   "Can't allocate shared memory. There's probably already a creation process running."
            Title: "Audio-CD-Creator"                       
            */             
          messageBox( text, IDSTR_ALLOCSHAREDMEMERROR , sizeof(text),
                      title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                      hDataResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
          break;
        }
      }
      else
        {
          /* The shared mem is already allocated. There's another write process running. */
          /*
            Text:   "Can't allocate shared memory. There's probably already a creation process running."
            Title: "Audio-CD-Creator"                       
            */             
          messageBox( text, IDSTR_ALLOCSHAREDMEMERROR , sizeof(text),
                      title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                      hDataResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
          break;
        }
      if(launchWrapper(chrInstallDir,"",hwnd,"scanbus.exe","Scanning SCSI bus...")) {
        /* There's an error */
        if(pvScanbusSharedMem) {
          DosFreeMem(pvScanbusSharedMem);
          pvScanbusSharedMem=NULL;
        }
      }
        
      return (MRESULT) TRUE;
    case WM_CLOSE:
      WinDismissDlg(hwnd, DID_ERROR);
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_ABORTWRITE:
          /* User pressed the ABORT button */
          WinDismissDlg(hwnd,DID_ERROR);
          break;
        case IDPB_STATUSOK:
          {
            int iTempSpeed;
            devInfo=(DEVICEINFO*)WinQueryWindowULong(hwnd, QWL_USER);
            /* Query the device to use   */
            WinQueryWindowText(WinWindowFromID(hwnd, IDDD_DEVICESELECTION), sizeof(text), text);

            if((textPtr=strchr(text, ' '))!=NULL)
              *textPtr=0;

            strncpy(devInfo->chrDev,text,sizeof(devInfo->chrDev));
            WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_QUERYVALUE,MPFROMP(&iTempSpeed),
                       MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));
            devInfo->iSpeed=iTempSpeed;
            WinDismissDlg(hwnd,0);
            break;
          }
        default:
          break;
        }
    case WM_APPTERMINATENOTIFY:
      
        switch(LONGFROMMP(mp1)) {
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
              sprintf(text,"%d,%d,%d", iBus, iTarget, iLun);
              if(strstr((char*) PVOIDFROMMP(mp2), text))
                WinSetWindowText(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),(PCSZ)PVOIDFROMMP(mp2));
            }
          break;
        case ACKEY_SCANBUS:
          /* Free shared mem if not already done */
          if(pvScanbusSharedMem) {
            DosFreeMem(pvScanbusSharedMem);
            pvScanbusSharedMem=NULL;
          }
          WinEnableWindow(WinWindowFromID(hwnd,IDPB_STATUSOK), TRUE);
          break;
        default:
          break;
        }/* switch */
      return WinDefWindowProc( hwnd, msg, mp1, mp2);
    default:
      break;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);    
}

/************************************************************
 *                                                          *
 * This frame proc handles the about menuitem of the        *
 * Warp 4 menu bar and the custom menu entries when         *
 * using WPS-wizard                                         *
 *                                                          *
************************************************************/
MRESULT EXPENTRY dataFrameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWDataFolder* thisPtr;
  HWND hwndDialog;
  MENUITEM mi;
  char text[100];

  switch (msg)
    {
    case WM_CONTROL:
      if(SHORT1FROMMP(mp1)==FID_CLIENT)
        {
          switch(SHORT2FROMMP(mp1))
            {
            case CN_BEGINEDIT:
              if(atomUpdateStatusbar)
                WinPostMsg(WinWindowFromID(hwnd,SHORT2FROMMP(mp1)),atomUpdateStatusbar,0,MPFROMSHORT(0));     
              break;
            case CN_ENDEDIT:
                if(atomUpdateStatusbar)
                  WinPostMsg(WinWindowFromID(hwnd,SHORT2FROMMP(mp1)),atomUpdateStatusbar,0,MPFROMSHORT(1));     
              break;
            default:
              break;
            }
        }
      break;

    case WM_INITMENU:
      if((hwndDialog=WinWindowFromID(hwnd,IDDLG_STATUS))!=NULLHANDLE)
        {
          thisPtr=(CWDataFolder*) WinQueryWindowULong(hwndDialog,QWL_USER);
          if(somIsObj(thisPtr)) {
            switch(thisPtr->usLastSelMenuItem)
              {                
              case 0x2d3:/* Help menu */
                if(hDataResource) {
                  /* insert separator */
                  cwInsertMenuSeparator(MIT_END, HWNDFROMMP(mp2), NULL);
                  /* Insert 'About' item */
                  /* Text: "About Data-CD-Creator" */
                  getMessage(text, IDSTR_MENUABOUTD ,sizeof(text), hDataResource, hwnd);
                  cwInsertMenuItem(MIT_END, HWNDFROMMP(mp2), NULL, ID_ABOUTITEM, text);
                  break;
                }
                break;
              default:
                break;
              }
          }
        }
      break;
    case WM_COMMAND:
      if(SHORT1FROMMP(mp2)==CMDSRC_MENU) {
        BOOL bHandled=FALSE;

        TRY_LOUD(DF_MENU) {
          if((hwndDialog=WinWindowFromID(hwnd,IDDLG_STATUS))!=NULLHANDLE)
            {
              thisPtr=(CWDataFolder*) WinQueryWindowULong(hwndDialog,QWL_USER);
              if(somIsObj(thisPtr)) {
                CWMenuFolder * wpFolder;	
                HOBJECT hObject;
                somId id;
                somTD_CWMenuFolder_mfCheckMenuItems methodPtr;
                M_WPObject *m_wpObject;
                
                if((hObject=WinQueryObject(USERMENUFOLDER_DATA))!=NULLHANDLE) {//is there a default menufolder?    
                  /* Get class object */
                  m_wpObject=(M_WPObject*)thisPtr->somGetClass();
                  if(somIsObj(m_wpObject)) {
                    /* We have it */
                    id=somIdFromString("mfCheckMenuItems");      
                    wpFolder=(CWMenuFolder *)m_wpObject->wpclsQueryObject( hObject);
                    if(somIsObj(wpFolder)) {
                      methodPtr= (somTD_CWMenuFolder_mfCheckMenuItems) (wpFolder->somGetClass())->somFindSMethod(id);  
                      SOMFree(id);
                      if(methodPtr) {
                        /* Check for Items */
                        bHandled=methodPtr(wpFolder, thisPtr, SHORT1FROMMP(mp1));
                      }
                      wpFolder->wpUnlockObject();//Object is locked because of wpclsQueryHandle()
                    }
                  }/* end of if(somIsObj(m_wpObject))  */
                }
              }
            }
        }
        CATCH(DF_MENU)
          {
          } END_CATCH;

          if(bHandled)
            return (MRESULT) 0;
      }
      break;
    case WM_MENUSELECT:
      if((hwndDialog=WinWindowFromID(hwnd,IDDLG_STATUS))!=NULLHANDLE)
        {
          thisPtr=(CWDataFolder*) WinQueryWindowULong(hwndDialog,QWL_USER);
          if(somIsObj(thisPtr)) {
            thisPtr->usLastSelMenuItem=SHORT1FROMMP(mp1);
          }
        }
      break;
    default:
      break;
    }
  /* Any other message is handled by the folder menu procedure */
  if((hwndDialog=WinWindowFromID(hwnd,IDDLG_STATUS))!=NULLHANDLE)
    {
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwndDialog,QWL_USER);
      if(somIsObj(thisPtr)) {
        if(thisPtr->pfnwpFrame)
          return thisPtr->pfnwpFrame(hwnd, msg, mp1, mp2);
      }
    }
  return  pfnwpDataGenericFrame(hwnd, msg, mp1, mp2);
}

/************************************************************
 *                                                          *
 * This is the procedure for the data folder container      *
 * which handles the tabbing from the container back to the *
 * Data-CD-creator frame dialogs.                           *
 *                                                          *
 ************************************************************/
MRESULT EXPENTRY dataContainerProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWDataFolder* thisPtr;
  HWND hwndDialog;
  static timerID;
  static POINTL ptl;
  static bEditing=FALSE;
  POINTL tempPtl;

  switch (msg)
    {
    case WM_DESTROY:
      WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,timerID);
      timerID=0;
      break;

    case WM_MOUSEMOVE:
      if(/*!timerID &&*/ !bEditing) {
        timerID=WinStartTimer(WinQueryAnchorBlock(hwnd),hwnd, 2 ,3000/*(ULONG)iTBFlyOverDelay*/); // New timer for delay
        ptl.x=SHORT1FROMMP(mp1);
        ptl.y=SHORT2FROMMP(mp1);
        //  writeLogPrintf("debug.log", "WM_MOUSEMOVE: ptl.x: %d \n", ptl.x);
      }
#if 0
      if(WinQueryWindowUShort(WinWindowFromPoint(hwnd,&ptl, FALSE), QWS_ID)==0x7ff1) {
        CNRINFO cnrInfo;
        PFIELDINFO pFieldInfo;
       /* Right title area */
        writeLogPrintf("debug.log", "WM_MOUSEMOVE: ptl.x: %d \n", ptl.x);
  
       /* Get container info */
        WinSendMsg(hwnd, CM_QUERYCNRINFO, MPFROMP(&cnrInfo) , MPFROMSHORT(sizeof(cnrInfo)));
        pFieldInfo=cnrInfo.pFieldInfoLast;
        if(pFieldInfo) {
          writeLogPrintf("debug.log", "WM_MOUSEMOVE: title: %s \n", pFieldInfo->pTitleData);
          pFieldInfo=pFieldInfo->pNextFieldInfo;
        }
        if(pFieldInfo) {
          writeLogPrintf("debug.log", "WM_MOUSEMOVE: title: %s \n", pFieldInfo->pTitleData);
          writeLogPrintf("debug.log", "WM_MOUSEMOVE: columnwidth: %d \n", pFieldInfo->cxWidth);
          writeLogPrintf("debug.log", "WM_MOUSEMOVE: workspace origin: %d \n", cnrInfo.ptlOrigin.x);
        }
        DosBeep(1000,20);
      }
#endif
      break;
    case WM_TIMER:
      switch (SHORT1FROMMP(mp1))
        {
        case 2: //delay over
          {
            HWND hwndParent;

            /* Test pointer position */
            WinQueryPointerPos(HWND_DESKTOP,&tempPtl);
            WinMapWindowPoints(HWND_DESKTOP, hwnd, &tempPtl, 1);

            if(labs(ptl.x-tempPtl.x) < 20 && abs(ptl.y-tempPtl.y) < 20) {
              PRECORDCORE pRecord;
              QUERYRECFROMRECT qRec;
              RECTL rectl;

              hwndParent=WinQueryWindow(hwnd, QW_PARENT);
              if (WinIsWindowShowing(hwnd) && WinQueryActiveWindow(HWND_DESKTOP)==hwndParent 
                  && atomUpdateStatusbar && bHintEnabled){/* atomUpdateStatusbar must be available because we need it
                                              to check for editing. */
                /* Get record under the pointer */
                qRec.cb=sizeof(QUERYRECFROMRECT);
                qRec.rect.xRight=tempPtl.x+10;
                qRec.rect.xLeft=tempPtl.x-10;
                qRec.rect.yBottom=tempPtl.y-10;
                qRec.rect.yTop=tempPtl.y+10;
                qRec.fsSearch=CMA_PARTIAL|CMA_ZORDER;
                pRecord=(PRECORDCORE)WinSendMsg(hwnd, CM_QUERYRECORDFROMRECT, MPFROMLONG(CMA_FIRST), MPFROMP(&qRec));
                if(pRecord) {
                  /* We have a record */

                  /* We are only showing hints over whitespace for now */
#if 0
                  WPObject *wpObject;
                  ULONG ulError;
                  BYTE *memPtr;
                  PSZ pszID;

                  wpObject=(WPObject*)OBJECT_FROM_PREC(pRecord);
                  if(somIsObj(wpObject)) {
                    /* Check if it's a shadow */
                    if(somResolveByName(wpObject,"wpQueryShadowedObject")) { 
                      /* Yes, it's a shadow. Query the linked object. */
                      wpObject=((WPShadow*)wpObject)->wpQueryShadowedObject(FALSE);
                      if(somIsObj(wpObject)) {
                        if((memPtr=wpObject->wpAllocMem(CCHMAXPATH*2, &ulError))!=NULLHANDLE){
                          pszID=wpObject->wpQueryObjectID();
                          if(pszID)
                            sprintf(memPtr,"%s %s",pszID,"<WPShadow>");
                          else
                            sprintf(memPtr,"%s","<WPShadow>");
                          launchPMWrapper(memPtr, "<CWCREATOR_SETTINGS>", "pmhint.exe", "Data-CD-Folder hint");
                          wpObject->wpFreeMem(memPtr);
                        }
                      }
                      else
                        launchPMWrapper("<WP_Shadow>", "<CWCREATOR_SETTINGS>", "pmhint.exe", "Data-CD-Folder hint");
                    } /* if(somResolveByName(wpObject,"wpQueryShadowedObject")) */
                    else
                      {
                        /* normal object */
                        pszID=wpObject->wpQueryObjectID();
                        if(pszID) 
                          launchPMWrapper(pszID, "<CWCREATOR_SETTINGS>", "pmhint.exe", "Data-CD-Folder hint");
                      }
                  }
#endif
                }/* if(pRecord) */
                else {
                  //writeLogPrintf("debug.log", "WM_TIMER: open hint\n");
                  //WinQueryPointerPos(HWND_DESKTOP, &ptl);
                  //WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1);
                  //  writeLogPrintf("debug.log", "WM_TIMER: new ptl.x: %d tempPtl: %d\n\n", ptl.x, tempPtl.x);
                  //              DosBeep(1000,130);
                  launchPMWrapper("2", "<CWCREATOR_SETTINGS>", "pmhint.exe", "Data-CD-Folder hint");
                  // WinPostMsg(hwndHintDaemon, WM_APPTERMINATENOTIFY, MPFROMSHORT(1), 0);
                }
              }
              WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,timerID);  // stop the running timer
              timerID=0;
            }
            else {
              RECTL rectl;
              /* Pointer has moved */
              WinQueryPointerPos(HWND_DESKTOP, &ptl);
              WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1);
              WinQueryWindowRect(hwnd,&rectl);
              if(!WinPtInRect(WinQueryAnchorBlock(hwnd),&rectl, &ptl)) {
                //    writeLogPrintf("debug.log", "Ptr moved: ptl.x: %d ptl.y: %d\n", ptl.x, ptl.y);
                WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,timerID);  // stop the running timer
                timerID=0;
              }
            }
            break;
          }
        default:
          break;
        }
      break;
    case WM_CHAR:
      /* handle TAB keys so user may tab from the container to the frame controls */
      if(SHORT1FROMMP(mp1) & KC_VIRTUALKEY)
        {
          if(SHORT2FROMMP(mp2)==VK_TAB && !(SHORT1FROMMP(mp1) & KC_KEYUP)) {
            /* Always forward tab to the select framecontrol */
            if((hwndDialog=WinQueryWindow(hwnd,QW_PARENT))!=NULLHANDLE)
              {
                if((hwndDialog=WinWindowFromID(hwndDialog,IDDLG_IMAGENAME))!=NULLHANDLE)
                  if((hwndDialog=WinWindowFromID(hwndDialog,IDPB_TBWRITINGFOLDER))!=NULLHANDLE)
                    if(WinSetFocus(HWND_DESKTOP,hwndDialog))
                      return (MRESULT)TRUE;
              }
            break;/* error: fall to the window proc */
          }
          else if(SHORT2FROMMP(mp2)==VK_BACKTAB && !(SHORT1FROMMP(mp1) & KC_KEYUP) )
            {
              if((hwndDialog=WinQueryWindow(hwnd,QW_PARENT))!=NULLHANDLE)
                {
                  if(WinIsWindowVisible(WinWindowFromID(WinWindowFromID(hwndDialog, IDDLG_MKISOFSMAIN),IDPB_WRITECD)))
                    WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(hwndDialog,IDDLG_MKISOFSMAIN),IDPB_WRITECD));
                  else
                    WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(hwndDialog, IDDLG_MKISOFSMAIN),IDPB_CREATE));
                  return (MRESULT)TRUE;
                }
            }
          break;/* error: fall to the window proc */
        }
      break;
    default:
      if(msg==atomUpdateStatusbar) {
        if(SHORT1FROMMP(mp2)==0) {
          WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,timerID);  // stop the running timer
          timerID=0;
          bEditing=TRUE;
        }
        else 
          if (SHORT1FROMMP(mp2)==1)
            bEditing=FALSE;
      }
      break;
    }
  /* Any other message is handled by the folder container procedure */
  if((hwndDialog=WinQueryWindow(hwnd,QW_PARENT))!=NULLHANDLE)
    {
      if((hwndDialog=WinWindowFromID(hwndDialog,IDDLG_STATUS))!=NULLHANDLE)
        {
          thisPtr=(CWDataFolder*) WinQueryWindowULong(hwndDialog,QWL_USER);
          if(somIsObj(thisPtr))
            if(thisPtr->pfnwpContainer)
              return thisPtr->pfnwpContainer(hwnd, msg, mp1, mp2);
        }
    }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

#ifdef NO_WPSWIZARD
/************************************************************
 *                                                          *
 * This procedure setups the buttons of the toolbar.        *
 * It subclasses every button with the flyover help         *
 * procedure.                                               *
 *                                                          *
 ************************************************************/
static void _setupToolBarButtons(HWND hwnd, CWDataFolder * thisFolder)
{
  BTNCDATA btCtrl;
  WNDPARAMS wndParams;
  ULONG ulStyle;
  HPOINTER hPtr;
  int a;
  HWND hwndTemp;
  HAB hab;
  SOMClass *folderClass;
  WPObject * wpObject;

  hab=WinQueryAnchorBlock(HWND_DESKTOP);

  if(hPtrTBIcons[0]==NULLHANDLE) {
    for(a=0;a<NUMTBICONS;a++) {
      hPtrTBIcons[a]=WinLoadPointer(HWND_DESKTOP, hDataResource, ID_TBICONFIRST+a);
    }
  }

  for(a=0;a<NUMDATATBBUTTONS && a<NUMTBICONS ;a++) {
    hwndTemp=WinWindowFromID(hwnd,IDPB_TBAUDIOFIRST+a);    
    if(WinIsWindow(hab,hwndTemp)) {
      ulStyle=WinQueryWindowULong(hwndTemp,QWL_STYLE);
      ulStyle|=(BS_MINIICON);
      ulStyle&=~BS_TEXT;
      if(WinSetWindowULong(hwndTemp,QWL_STYLE,ulStyle)) {
        memset(&btCtrl,0,sizeof(btCtrl));
        btCtrl.cb=sizeof(btCtrl);
        hPtr=hPtrTBIcons[a];
        btCtrl.hImage=hPtr;
        memset(&wndParams,0,sizeof(wndParams));
        wndParams.fsStatus=WPM_CTLDATA;
        wndParams.cbCtlData=btCtrl.cb;
        wndParams.pCtlData=&btCtrl;
        WinSendMsg(hwndTemp,WM_SETWINDOWPARAMS,
                   MPFROMP(&wndParams),0);
        /* Subclass with new fly over button proc */
        oldButtonProc2=WinSubclassWindow(hwndTemp,newButtonProc);
      }
    }/* end of if(WinIsWindow(hab,hwndTemp)) */
  }
  WinShowWindow(WinWindowFromID(hwnd, IDPB_TBERASECD), FALSE);
}
#endif

/************************************************************
 *                                                          *
 * This procedure calculates the positions for the custom   *
 * frame controls.                                          *
 * PM uses the pos to calculate the area which needs        *
 * repainting and the pos of each control. Using this code  *
 * prevents flickering.                                     *
 *                                                          *
 ************************************************************/
MRESULT handleCalcValidRects(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PRECTL pRectl, pRectlNew;
  PSWP pswp;
  POINTL ptl;
  
  pswp=(PSWP)mp2; 
  if(pswp->fl&SWP_SIZE) {
    pRectl=(PRECTL)mp1;        
    pRectlNew=pRectl;
    pRectlNew++;

    if(pRectlNew->xRight!=pRectl->xRight) { 
      if(WinSendMsg(hwnd,WM_QUERYBORDERSIZE,(MPARAM)&ptl,0)) {
        pRectl->xRight-=ptl.x;
      }else {
        /* Ok, then use some defaults. Better than nothing... */
        pRectl->xRight-=4;
      }
      pRectlNew->xRight=pRectl->xRight;
      return (MRESULT)0;
    }
    else if(pRectlNew->xLeft<pRectl->xLeft) {
      if(WinSendMsg(hwnd,WM_QUERYBORDERSIZE,(MPARAM)&ptl,0)) {
        pRectl->xLeft+=ptl.x;
      }else {
        /* Ok, then use some defaults. Better than nothing... */
        pRectl->xLeft+=4;
      }
      pRectlNew->xLeft=pRectl->xLeft;
      return (MRESULT)0;
    }
    else if(pRectlNew->xLeft<pRectl->xLeft) {
      pRectlNew->xLeft=pRectl->xLeft;
            
      return (MRESULT)0;
    } else if(pRectlNew->yBottom!=pRectl->yBottom || pRectlNew->yTop!=pRectl->yTop) {            
      pRectlNew->yTop=pRectl->yTop;
      pRectlNew->yBottom=pRectl->yBottom;
      return (MRESULT)0;
    }
  }
  return (MRESULT)0;
}

/************************************************************
 *                                                          *
 * When sizing and moving the folder the custom frame       *
 * controls must be moved too so they  keep their visual    *
 * position with respect to the left border of the folder.  *
 *                                                          *
 ************************************************************/
void handleWindowPosChanged(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PSWP pswp=(PSWP)mp1;

  if((pswp->fl & SWP_SIZE)&&!(pswp->fl & SWP_SHOW)) {
    SWP swp;
    HENUM hEnum;
    HWND hwndClient;
    PSWP pswpOld;
    int xPos;//, yPos;
    ULONG ulStyle;

    pswpOld=pswp;
    pswpOld++;

    ulStyle=WinQueryWindowULong(WinQueryWindow(hwnd,QW_PARENT),QWL_STYLE);
    if(!(ulStyle & WS_MINIMIZED) && !(ulStyle & WS_MAXIMIZED)  ) {
      
      if(pswpOld->x != pswp->x) {
        if((pswpOld->cx!=0)) {
          xPos=(pswpOld->x > pswp->x ? pswpOld->x-pswp->x : 0);
          //yPos=(pswpOld->y > pswp->y  ? pswpOld->y-pswp->y : 0);
        }
        else {
          xPos=0;
        }

        /* Move all client windows so they keep their distance to the left border */
        hEnum=WinBeginEnumWindows(hwnd);
        while((hwndClient=WinGetNextWindow(hEnum))!=NULLHANDLE) {
          WinQueryWindowPos(hwndClient,&swp);
          /* Changed for fillbar */
          //     WinSetWindowPos(hwndClient, NULLHANDLE, swp.x-xPos, (pswp->cy-swp.cy)/2/*swp.y-yPos*/, 0, 0, SWP_MOVE);
          WinSetWindowPos(hwndClient, NULLHANDLE, swp.x-xPos, swp.y /* (pswp->cy-swp.cy)/2 */ /*swp.y-yPos*/, 0, 0, SWP_MOVE);
        }/* while */
        WinEndEnumWindows(hEnum);
      }/* if(pswpOld->x != pswp->x) */
    }
  }
}

/************************************************************
 *                                                          *
 * When sizing and moving the folder the custom frame       *
 * controls must be moved too so they  keep their visual    *
 * position with respect to the left border of the folder.  *
 *                                                          *
 ************************************************************/
void handleStatusWindowPosChanged(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PSWP pswp=(PSWP)mp1;

  if((pswp->fl & SWP_SIZE)&&!(pswp->fl & SWP_SHOW)) {
    SWP swp;
    HENUM hEnum;
    HWND hwndClient;
    PSWP pswpOld;
    int xPos;//, yPos;
    ULONG ulStyle;

    pswpOld=pswp;
    pswpOld++;

    ulStyle=WinQueryWindowULong(WinQueryWindow(hwnd,QW_PARENT),QWL_STYLE);
    if(!(ulStyle & WS_MINIMIZED) && !(ulStyle & WS_MAXIMIZED)  ) {
      
      if(pswpOld->x != pswp->x) {
        if((pswpOld->cx!=0)) {
          xPos=(pswpOld->x > pswp->x ? pswpOld->x-pswp->x : 0);
          //yPos=(pswpOld->y > pswp->y  ? pswpOld->y-pswp->y : 0);
        }
        else {
          xPos=0;
        }

        /* Move all client windows so they keep their distance to the left border */
        hEnum=WinBeginEnumWindows(hwnd);
        while((hwndClient=WinGetNextWindow(hEnum))!=NULLHANDLE) {
          WinQueryWindowPos(hwndClient,&swp);
          /* Changed for fillbar */
          WinSetWindowPos(hwndClient, NULLHANDLE, swp.x-xPos, swp.y , pswp->cx-20, swp.cy, SWP_MOVE | SWP_SIZE);
        }/* while */
        WinEndEnumWindows(hEnum);
      }/* if(pswpOld->x != pswp->x) */

      if(pswpOld->cx != pswp->cx) {
        /* Move all client windows so they keep their distance to the left border */
        hEnum=WinBeginEnumWindows(hwnd);
        while((hwndClient=WinGetNextWindow(hEnum))!=NULLHANDLE) {
          WinQueryWindowPos(hwndClient,&swp);
          /* Changed for fillbar */
          WinSetWindowPos(hwndClient, NULLHANDLE, swp.x, swp.y , pswp->cx-2*swp.x, swp.cy, SWP_SIZE);
        }/* while */
        WinEndEnumWindows(hEnum);
      }/* if(pswpOld->x != pswp->x) */
    }
  }
}

#ifdef NO_WPSWIZARD
/********************************************************/
/*                                                      */
/* This procedure handles the imagename frame extension */
/*                                                      */
/* Remark: the imagename entry field has been moved     */
/* to the mksiofsmain dialog                            */
/*                                                      */
/********************************************************/ 
MRESULT EXPENTRY imageNameDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWDataFolder* thisPtr;

  POINTL ptl;
  SHORT a,b;
  HWND hwndTemp;
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  char *textPtr;
  HOBJECT hObject;
  PDRAGINFO pDragInfo;
  SWP swp, swp2;

  switch(msg)
    {
    case WM_CALCVALIDRECTS:
      {
        if(WinIsWindowVisible(hwnd))
          return handleCalcValidRects(hwnd,  msg,  mp1,  mp2);
        break;
      }

    case WM_WINDOWPOSCHANGED:
      {
        /* The folder was resized. Move the client windows so they keep their
           distance to the left border. */
        handleWindowPosChanged(hwnd, msg, mp1, mp2);
        break;
      }
    case WM_PAINT:
      {
        if(HlpPaintFrame(hwnd, FALSE))
          return (MRESULT)0;
        break;
      }
    case WM_INITDLG:
      WinSetWindowULong(hwnd,QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.	
      thisPtr=(CWDataFolder*)PVOIDFROMMP(mp2);
      _setupToolBarButtons(hwnd, (CWDataFolder *)PVOIDFROMMP(mp2));
      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }

      /* Build launchpad control */
      if(somIsObj(thisPtr)) {
        thisPtr->lPad=new launchPad(hwnd, hwnd, TRUE, (WPFolder*) PVOIDFROMMP(mp2), &lplAllLPads, NULLHANDLE, 0);
        WinQueryWindowPos(hwnd,&swp);
        WinQueryWindowPos(WinWindowFromID(hwnd, IDPB_TBTUTORIAL),&swp2);
        if(thisPtr->lPad) {
          thisPtr->lPad->lpSetConfiguration(chrInstallDir, DATACD_TBNAME);
          thisPtr->lPad->lpSetLaunchPadPos(NULLHANDLE,  swp2.x+swp2.cx+20, 1, swp.cx-swp2.x-swp2.cx-30 , swp.cy-2, 
                                           SWP_SIZE | SWP_MOVE);
          thisPtr->lPad->lpFillPad();
        }
      }
      return (MRESULT) TRUE;
    case WM_DESTROY:
      thisPtr=(CWDataFolder*)WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)) {
        if(thisPtr->lPad)
          delete(thisPtr->lPad);
        thisPtr->lPad=NULL;
      }
      break;

    case WM_HELP:
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      thisPtr->wpDisplayHelp(IDHLP_DATADEFAULT,AFHELPLIBRARY);
      break;
    case WM_CONTEXTMENU:
      ULONG ulStyle;
      /* Display popup menu for image name frame extension */
      WinQueryPointerPos(HWND_DESKTOP,&ptl);
      WinMapWindowPoints(HWND_DESKTOP,hwnd,&ptl,1);

      hwndTemp=WinWindowFromPoint(hwnd,&ptl,FALSE);/* Get child window under ptr */

      a=IDM_DATADEFAULT;
      b=a;
      
      hwndTemp=WinLoadMenu(hwnd,hDataResource,a);
      if(hwndTemp) {
        if(WinPopupMenu(hwnd,hwnd,hwndTemp,ptl.x,ptl.y,b,PU_VCONSTRAIN|PU_HCONSTRAIN|
                        PU_MOUSEBUTTON1|PU_KEYBOARD|PU_NONE|PU_SELECTITEM)) {
          return (MRESULT)FALSE;
        }
      }
      break;
    case WM_COMMAND:
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(!somIsObj(thisPtr))
        break;	
      switch(SHORT1FROMMP(mp1))
        {
        case IDM_DATADEFAULT:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_DATADEFAULT,AFHELPLIBRARY);
        case IDPB_TBWRITINGFOLDER:
          if((hObject=WinQueryObject("<CD_WRITING>"))!=NULL) {
            WinOpenObject(hObject,OPEN_DEFAULT,TRUE);
          } 
          break;
        case IDPB_TBCDCREATORSETTINGS:
          if((hObject=WinQueryObject("<CWCREATOR_SETTINGS>"))!=NULL) {
            WinOpenObject(hObject,OPEN_DEFAULT,TRUE);
          } 
            break;
        case IDPB_TBCDRWTOOLS:
          _beginthread(toolsThreadFunc,NULL,8192*8,thisPtr); //Fehlerbehandlung fehlt
          break;
        case IDPB_TBCREATECOVER:
          /* Text: "Should P>G Pro be started to edit the cover? (P>G Pro must be installed and
           *.cwx files be associated with it."
           Title:  "Data-CD-Creator"
             */
          /* if(messageBox( text, IDSTRA_STARTPGPROQUESTION , sizeof(text),
                         title, IDSTRD_DATACDCREATOR, sizeof(title),
                         hDataResource, HWND_DESKTOP, MB_YESNO | MB_ICONQUESTION|MB_MOVEABLE)==MBID_YES) {*/
          sprintf(text,"%s\\bin\\databack.cwx",chrInstallDir);
          if((hObject=WinQueryObject(text))!=NULL) {
            WinOpenObject(hObject,OPEN_DEFAULT,TRUE); /* Open P>G Pro */
          } 
          //}
          
          break;
          case IDPB_TBHELP:
            thisPtr->wpDisplayHelp(IDHLP_DATACDMAIN,AFHELPLIBRARY);
            break;
        case IDPB_TBTUTORIAL:
          if((hObject=WinQueryObject("<CWCREATOR_TUTORIAL>"))!=NULL) {
            WinOpenObject(hObject,OPEN_DEFAULT,TRUE);
          } 
          break;
        case IDPB_TBCDSETTINGS:
          /* User pressed the SETTINGS... button */
          thisPtr->wpViewObject(NULLHANDLE,OPEN_SETTINGS,0);
          break;
          
        default:
          break;
        }
      return (MRESULT) TRUE;
    case WM_CHAR:
      if(SHORT1FROMMP(mp1) & KC_ALT) {  // Send keys with ALT modifier to the folder
        // Save control with focus
        hwndTemp=WinQueryFocus(HWND_DESKTOP);
        // Set focus to container so the key will be processed as usual
        WinSetFocus(HWND_DESKTOP, WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),FID_CLIENT));
        // Set saved control as old focus owner so it will get the focus back when necessary, not the container
        WinSetWindowULong(WinQueryWindow(hwnd,QW_PARENT),QWL_HWNDFOCUSSAVE,hwndTemp);
        return WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),FID_CLIENT),msg,mp1,mp2);
      }
      else if(SHORT1FROMMP(mp1) & KC_VIRTUALKEY)
        {
          if(SHORT2FROMMP(mp2)==VK_TAB ) {
            if(WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(hwnd,IDPB_TBTUTORIAL)) {
              WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                                                       IDDLG_MKISOFSMAIN),IDEF_IMAGENAME));
              return (MRESULT)TRUE;
            }
          }
          else if(SHORT2FROMMP(mp2)==VK_BACKTAB )
            {
              if(WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(hwnd,IDPB_TBWRITINGFOLDER)) {
                WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),FID_CLIENT));
                return (MRESULT)TRUE;
              }
            }
        }
      else {
        /* Send chars to other dialogs so accelerator keys are recognized */
        WinSetActiveWindow(HWND_DESKTOP,WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT), IDDLG_MKISOFSMAIN));
        if(!WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT), IDDLG_MKISOFSMAIN),WM_CHAR, mp1, mp2))
          WinSetActiveWindow(HWND_DESKTOP,hwnd);/* The char wasn't processed by the other dialog so reactivate this dialog */
        else
          return (MRESULT) TRUE;/* The char was eaten so it was an accelerator key */
      }
      break;      
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
#endif


#if 0
char HlpGetBootDriveLetter(void)
{
  ULONG ulSysValue;

  if(!DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,&ulSysValue,sizeof(ulSysValue)))
    return 'a'+ulSysValue-1;

  return 'c';
} 
#define EXCEPTION_LOGFILE_NAME "debug.log"
void HlpWriteToDebugLog(const char* chrFormat, ...)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;

  sprintf(logNameLocal,"%c:\\%s", HlpGetBootDriveLetter(), EXCEPTION_LOGFILE_NAME);
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {
    va_list arg_ptr;
    void *tb;

    va_start (arg_ptr, chrFormat);
    vfprintf(fHandle, chrFormat, arg_ptr);
    va_end (arg_ptr);
    //    fprintf(fHandle,logText);
    fclose(fHandle);
  }
}

/************************************************************
 *                                                          *
 * When sizing and moving the folder the custom frame       *
 * controls must be moved too so they  keep their visual    *
 * position with respect to the left border of the folder.  *
 *                                                          *
 ************************************************************/
void handleVertWindowPosChanged(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PSWP pswp=(PSWP)mp1;


  if((pswp->fl & SWP_SIZE)&&!(pswp->fl & SWP_SHOW)) {
    HENUM hEnum;
    HWND hwndClient;
    PSWP pswpOld;
    //int yPos;
    int xPos, yPos;
    ULONG ulStyle;

    pswpOld=pswp;
    pswpOld++;

    ulStyle=WinQueryWindowULong(WinQueryWindow(hwnd,QW_PARENT),QWL_STYLE);
    if(!(ulStyle & WS_MINIMIZED) && !(ulStyle & WS_MAXIMIZED)  ) {
      
      if(pswpOld->y != pswp->y) {
        if((pswpOld->cy!=0)) {
          //          xPos=(pswpOld->x > pswp->x ? pswpOld->x-pswp->x : 0);
          yPos=(pswpOld->y > pswp->y  ? pswpOld->y-pswp->y : -(pswp->y-pswpOld->y) );
        }
        else {
          yPos=0;
        }
        /* Move all client windows so they keep their distance to the bottom border */
        hEnum=WinBeginEnumWindows(hwnd);
        while((hwndClient=WinGetNextWindow(hEnum))!=NULLHANDLE) {
          SWP swp;
          WinQueryWindowPos(hwndClient,&swp);
          //  WinSetWindowPos(hwndClient, NULLHANDLE, swp.x-xPos, (pswp->cy-swp.cy)/2/*swp.y-yPos*/, 0, 0, SWP_MOVE);
          if(WinQueryWindowUShort(hwndClient, QWS_ID)!=1501) {
            //HlpWriteToDebugLog("Normal: swp.x: %d, yPos: %d\n", swp.x, yPos);
            WinSetWindowPos(hwndClient, NULLHANDLE, swp.x, swp.y-yPos, 0, 0, SWP_MOVE);
          }
          else {
            //HlpWriteToDebugLog("Entry: swp.x: %d, yPos: %d\n", swp.x, yPos);
            WinSetWindowPos(hwndClient, NULLHANDLE, swp.x+yPos, swp.y-yPos, 0, 0, SWP_MOVE);
          }
          WinInvalidateRect(hwndClient, NULL, FALSE);
        }/* while */
        WinEndEnumWindows(hEnum);
      }
    }
  }
}
#endif

/********************************************************/ 
/*                                                      */
/*  This procedure handles the main frame extension     */ 
/*                                                      */
/********************************************************/ 
MRESULT EXPENTRY mkisofsMainDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWDataFolder* thisPtr;
  ULONG ulFlags;
  FILEDLG fd = { 0 };  
  struct stat buf;
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  char text2[CCHMAXPATH];
  POINTL ptl;    SWP swp;
  SHORT a,b;
  HWND hwndTemp;
  static HWND hwndStatus;
  ULONG rc;

  switch(msg)
    {
#if 0
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;
        // mr=WinDefDlgProc(hwnd, msg, mp1, mp2);
        /* The folder was resized. Move the client windows so they keep their
           distance to the left border. */
        handleVertWindowPosChanged(hwnd, msg, mp1, mp2);
        //return mr;
        break;
      }
#endif
    case WM_CALCVALIDRECTS:
      {
        /* Calculate position of frame control so PM doesn't overpaint us with
           the container. Check first if we're visible so we don't mess up the toolbar
           position when currently minimized. */
        if(WinIsWindowVisible(hwnd))
          return handleCalcValidRects(hwnd,  msg,  mp1,  mp2);
        break;
      }
    case WM_PAINT:
      {
        if(HlpPaintFrame(hwnd, FALSE))
          return (MRESULT)0;
        break;
      }
      /* Init dialog */
    case WM_INITDLG:
      WinSetWindowULong(hwnd,QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      thisPtr=(CWDataFolder*)PVOIDFROMMP(mp2);
      if(!somIsObj(thisPtr))
        return (MRESULT) TRUE; /* Object pointer not valid!! */

      /* subclass static text control for custom painting */
      oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_IMAGENAME),staticTextProc);
      /* subclass checkbox and find mnemonic */
      setupCheckBoxControl(hwnd , IDCB_DUMMY);
      /* subclass radio buttons */
      setupRadioButtonControl(hwnd, IDRB_IMAGEONLY);
      setupRadioButtonControl(hwnd, IDRB_ONTHEFLY);
      setupRadioButtonControl(hwnd, IDRB_WRITEIMAGE);
      /* Subclass groupbox */
      setupGroupBoxControl(  hwnd, IDGB_DATADIALOG);

      WinCheckButton(hwnd,IDCB_DUMMY,((CWDataFolder*)(PVOIDFROMMP(mp2)))->sDummy);
      ulFlags=((CWDataFolder*)PVOIDFROMMP(mp2))->cwQueryCreateFlags();

      WinSendMsg(WinWindowFromID(hwnd,IDEF_IMAGENAME),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_IMAGENAME),thisPtr->chrImageName);

      thisPtr=(CWDataFolder*)PVOIDFROMMP(mp2);
      if(ulFlags&IDCR_IMAGEONLY) {
        WinCheckButton(hwnd,IDRB_IMAGEONLY,1);
        WinShowWindow(WinWindowFromID(hwnd,IDPB_WRITECD),FALSE);
        WinShowWindow(WinWindowFromID(hwnd,IDCB_DUMMY),FALSE);
        WinShowWindow(WinWindowFromID(hwnd,IDPB_CREATE),TRUE);
        thisPtr->cwShowImageName(TRUE);
      }
      else {
        WinShowWindow(WinWindowFromID(hwnd,IDPB_CREATE),FALSE);
        WinShowWindow(WinWindowFromID(hwnd,IDPB_WRITECD),TRUE);
        WinShowWindow(WinWindowFromID(hwnd,IDCB_DUMMY),TRUE);
      }
      if(ulFlags&IDCR_WRITEIMAGE) { 
        WinCheckButton(hwnd,IDRB_WRITEIMAGE,1);
        thisPtr->cwShowImageName(TRUE);
      }
      WinShowWindow(hwnd,TRUE);
      if(ulFlags&IDCR_ONTHEFLY) {
        thisPtr->cwShowImageName(FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,IDEF_IMAGENAME) ,FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,IDPB_IMAGEBROWSE) ,FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,IDST_IMAGENAME) ,FALSE);
        WinCheckButton(hwnd,IDRB_ONTHEFLY,1);
      }
      hwndStatus=NULL;
      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }

      return (MRESULT) TRUE;
    case WM_CHAR: /* Handle keyboard navigation */
      if(SHORT1FROMMP(mp1) & KC_ALT) {
        hwndTemp=WinQueryFocus(HWND_DESKTOP);        
        WinSetFocus(HWND_DESKTOP, WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),FID_CLIENT));
        WinSetWindowULong(WinQueryWindow(hwnd,QW_PARENT),QWL_HWNDFOCUSSAVE,hwndTemp);
        return WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),FID_CLIENT),msg,mp1,mp2);
      }
      else if(SHORT1FROMMP(mp1) & KC_VIRTUALKEY)
        {
          if(SHORT2FROMMP(mp2)==VK_TAB ) {
            if(WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(hwnd,IDPB_WRITECD) || 
               WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(hwnd,IDPB_CREATE)) {
              WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),FID_CLIENT));
              return (MRESULT)TRUE;
            }
          }
          else if(SHORT2FROMMP(mp2)==VK_BACKTAB )
            {
              if(WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(hwnd,IDEF_IMAGENAME)) {
                WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                                                         IDDLG_IMAGENAME),IDPB_TBTUTORIAL));
                return (MRESULT)TRUE;
              }
            }
        }
      break;
      
      /* Help stuff */
    case WM_HELP:
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {
        case IDEF_IMAGENAME:
          return (MRESULT)thisPtr->wpDisplayHelp(IDEF_IMAGENAME,AFHELPLIBRARY);
        case IDPB_IMAGEBROWSE:
          return (MRESULT)thisPtr->wpDisplayHelp(IDPB_IMAGEBROWSE,AFHELPLIBRARY);
        case IDRB_IMAGEONLY:
          return (MRESULT)thisPtr->wpDisplayHelp(IDRB_IMAGEONLY,AFHELPLIBRARY);
        case IDRB_WRITEIMAGE:
          return (MRESULT)thisPtr->wpDisplayHelp(IDRB_WRITEIMAGE,AFHELPLIBRARY);
        case IDRB_ONTHEFLY:
          return (MRESULT)thisPtr->wpDisplayHelp(IDRB_ONTHEFLY,AFHELPLIBRARY);
        case IDPB_CHECKSIZE:
          return (MRESULT)thisPtr->wpDisplayHelp(IDPB_CHECKSIZE,AFHELPLIBRARY);
        case IDPB_WRITECD:
          return (MRESULT)thisPtr->wpDisplayHelp(IDPB_WRITECD,AFHELPLIBRARY);
        case IDPB_CREATE:
          return (MRESULT)thisPtr->wpDisplayHelp(IDPB_CREATE,AFHELPLIBRARY);
        case IDCB_DUMMY:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_MKISOFSDUMMY,AFHELPLIBRARY);
        default:
          break;
        }
      break;

      /* Handle popup memu */
    case WM_BUTTON2DOWN:
      /* Display popup menu for control the mouse is over */
      WinQueryPointerPos(HWND_DESKTOP,&ptl);
      WinMapWindowPoints(HWND_DESKTOP,hwnd,&ptl,1);
      hwndTemp=WinWindowFromPoint(hwnd,&ptl,FALSE);/* Get child window under ptr */
      switch(WinQueryWindowUShort(hwndTemp,QWS_ID))/* Get child window id */
        {
        case IDEF_IMAGENAME:
          a=IDM_IMAGENAME;
          b=a;
          break;
        case IDPB_IMAGEBROWSE:
          a=IDM_IMAGEBROWSE;
          b=a;
          break;
        case IDRB_IMAGEONLY:
          a=IDM_IMAGEONLY;
          b=a;
          break;
        case IDRB_WRITEIMAGE:
          a=IDM_WRITEIMAGE;
          b=a;
          break;
        case IDRB_ONTHEFLY:
          a=IDM_ONTHEFLY;
          b=a;
          break;
        case IDPB_CHECKSIZE:
          a=IDM_CHECKSIZE;
          b=a;
          break;
        case IDPB_MKISOFSSETTINGS:
          a=IDM_MKISOFSSETTINGS;
          b=a;
          break;
        case IDCB_DUMMY:
          a=IDM_MKISOFSDUMMY;
          b=a;
          break;
#if 0
        case IDPB_WRITECD:
          a=IDPB_WRITECD;
          b=a;
          break;
        case IDPB_CREATE:
          a=IDPB_CREATE;
          b=a;
          break;
#endif
        default:
          a=IDM_DATADEFAULT;
          b=a;
          break;
        }
      hwndTemp=WinLoadMenu(hwnd,hDataResource,a);
      if(hwndTemp) {
        if(WinPopupMenu(hwnd,hwnd,hwndTemp,ptl.x,ptl.y,b,PU_VCONSTRAIN|PU_HCONSTRAIN|
                        PU_MOUSEBUTTON1|PU_KEYBOARD|PU_NONE|PU_SELECTITEM)) {
          return (MRESULT)FALSE;
        }
      }
      break;
    case WM_DESTROY:
      /* The folderwindow closes and gets destroyed */
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)) {
        thisPtr->sDummy=WinQueryButtonCheckstate(hwnd,IDCB_DUMMY);
        ulFlags=0;
        if(WinQueryButtonCheckstate(hwnd,IDRB_IMAGEONLY)&1)
          ulFlags|=IDCR_IMAGEONLY;
        if(WinQueryButtonCheckstate(hwnd,IDRB_WRITEIMAGE)&1)
          ulFlags|=IDCR_WRITEIMAGE;
        if(WinQueryButtonCheckstate(hwnd,IDRB_ONTHEFLY)&1)
          ulFlags|=IDCR_ONTHEFLY;
        thisPtr->cwSetCreateFlags( ulFlags,0xF);
        WinQueryWindowText( WinWindowFromID(hwnd,IDEF_IMAGENAME),sizeof(thisPtr->chrImageName),thisPtr->chrImageName);      
      }

      return (MRESULT) TRUE;
    case WM_CONTROL:
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(!somIsObj(thisPtr)) return (MRESULT) TRUE;
      switch(SHORT1FROMMP(mp1))
        {
        case IDCB_DUMMY:
          thisPtr->sDummy=WinQueryButtonCheckstate(hwnd,IDCB_DUMMY);
          break;
        case IDRB_WRITEIMAGE:
        case IDRB_IMAGEONLY:
        case IDRB_ONTHEFLY:
          ulFlags=0;
          if(WinQueryButtonCheckstate(hwnd,IDRB_IMAGEONLY)&1) {
            ulFlags|=IDCR_IMAGEONLY;
            WinShowWindow(WinWindowFromID(hwnd,IDPB_WRITECD),FALSE);
            WinShowWindow(WinWindowFromID(hwnd,IDCB_DUMMY),FALSE);
            WinShowWindow(WinWindowFromID(hwnd,IDPB_CREATE),TRUE);
            thisPtr->cwShowImageName(TRUE);
          }
          else {
            if(WinQueryButtonCheckstate(hwnd,IDRB_WRITEIMAGE)&1) {
              ulFlags|=IDCR_WRITEIMAGE;
              thisPtr->cwShowImageName(TRUE);
            }
            if(WinQueryButtonCheckstate(hwnd,IDRB_ONTHEFLY)&1) {
              ulFlags|=IDCR_ONTHEFLY;
              thisPtr->cwShowImageName(FALSE);
            }
            WinShowWindow(WinWindowFromID(hwnd,IDPB_CREATE),FALSE);
            WinShowWindow(WinWindowFromID(hwnd,IDPB_WRITECD),TRUE);
            WinShowWindow(WinWindowFromID(hwnd,IDCB_DUMMY),TRUE);
          }
          thisPtr->cwSetCreateFlags( ulFlags,0xF);
          break;
        default:
          break;
        }
      return (MRESULT) TRUE;
      /* User pressed a push button */
    case WM_COMMAND:	
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)) {
        switch(SHORT1FROMMP(mp1))
          {
        case IDPB_IMAGEBROWSE:
          /* User pressed the BROWSE button */
          fd.cbSize = sizeof( fd );
          /* It's an centered 'Save'-dialog */
          fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
          /* Set the title of the file dialog */
          /* Text: "Path for ISO-Image file" */
          getMessage(text, IDSTRD_IMAGENAMEBROWSE,sizeof(text), hDataResource, hwnd);
          fd.pszTitle = text;
          /* Only show *.exe files */
          if(strlen(thisPtr->chrImageName)==0)
            sprintf(fd.szFullFile,"%s","*.raw;*.iso");
          else
            sprintf(fd.szFullFile,"%s", thisPtr->chrImageName);
          if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
            {
              /* WinFileDlg failed */
              break;
            }
          if( fd.lReturn == DID_OK )
            {
              WinSetWindowText( WinWindowFromID(hwnd,IDEF_IMAGENAME), fd.szFullFile );
              sprintf(thisPtr->chrImageName,"%s",fd.szFullFile);
            }
          break;          

          case IDPB_CREATE:
            /* User pressed the CREATE button */
            /* Get current image name */
            WinQueryWindowText( WinWindowFromID(hwnd,IDEF_IMAGENAME),sizeof(thisPtr->chrImageName), thisPtr->chrImageName);
            thisPtr->cwCreateImage();
            break;
          case IDPB_WRITECD:
            /* User pressed the WRITE button */
            WinQueryWindowText( WinWindowFromID(hwnd,IDEF_IMAGENAME),sizeof(thisPtr->chrImageName), thisPtr->chrImageName);      
            if(WinQueryButtonCheckstate(hwnd,IDRB_WRITEIMAGE)&1)
              thisPtr->cwWriteImage(); /* Create an image */
            else 
              thisPtr->cwWriteOnTheFly(); /* Create a CD on the fly */
            break;
          case IDPB_ABORTCHECKSIZE:
            thisPtr->bStopCreateThread=TRUE;
            break;
          case IDPB_CHECKSIZE:
            if(WinQueryButtonCheckstate(hwnd,IDRB_WRITEIMAGE)&1) {
              /* Get current filename from entryfield */
              WinQueryWindowText( WinWindowFromID(hwnd, IDEF_IMAGENAME),
                                  sizeof(thisPtr->chrImageName),thisPtr->chrImageName);
              /* Check if we have an output filename */
              if(!checkImageName(thisPtr, hwnd, TRUE))
                break;
              if(stat(thisPtr->chrImageName,&buf))
                /* Text: "Sorry, file size unknown."
                   Title: "Write image, check size"
                   */
                messageBox( text, IDSTRD_CHECKSIZEERRORNOSIZE , sizeof(text),
                            title, IDSTRD_CHECKSIZEERRORTITLE, sizeof(title),
                            hDataResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
              else {
                /* Title: "Imagesize is %d.%0.3d.%0.3d bytes" */
                getMessage(text2, IDSTRD_IMAGESIZE, sizeof(text2), hDataResource,HWND_DESKTOP);
                sprintf(text,text2,
                        buf.st_size/1000000,(buf.st_size%1000000)/1000,buf.st_size%1000);
                thisPtr->cwSetStatusText(text);
                DosBeep(2000,200);
              }
            }/* if(WinQueryButtonCheckstate(hwnd,IDRB_WRITEIMAGE)&1) */
            else {/* if(WinQueryButtonCheckstate(hwnd,IDRB_WRITEIMAGE)&1)*/
              /* We check the size of the image which will be built. This part applies to 'On the fly'
                 and 'Create image only' slections.*/
              thisPtr->cwGetImageSize();
            }
            break;
            /* Popup menu commands: */
          case IDM_IMAGENAME:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_IMAGENAME,AFHELPLIBRARY);
          case IDM_IMAGEBROWSE:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_IMAGEBROWSE,AFHELPLIBRARY);
          case IDM_IMAGEONLY:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_IMAGEONLY,AFHELPLIBRARY);
          case IDM_WRITEIMAGE:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_WRITEIMAGE,AFHELPLIBRARY);
          case IDM_ONTHEFLY:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_ONTHEFLY,AFHELPLIBRARY);
          case IDM_CHECKSIZE:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CHECKSIZE,AFHELPLIBRARY);
          case IDM_WRITECD:
            return (MRESULT)thisPtr->wpDisplayHelp(IDPB_WRITECD,AFHELPLIBRARY);
          case IDM_CREATE:
            return (MRESULT)thisPtr->wpDisplayHelp(IDPB_CREATE,AFHELPLIBRARY);
          case IDM_MKISOFSSETTINGS:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDSETTINGS,AFHELPLIBRARY);
          case IDM_MKISOFSDUMMY:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_DUMMY,AFHELPLIBRARY);    
          case IDM_DATADEFAULT:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_DATADEFAULT,AFHELPLIBRARY);
#if 0
          case IDPB_WRITECD:
            return (MRESULT)thisPtr->wpDisplayHelp(IDPB_WRITECD,AFHELPLIBRARY);
          case IDPB_CREATE:
            return (MRESULT)thisPtr->wpDisplayHelp(IDPB_CREATE,AFHELPLIBRARY);
#endif
          default:
            break;
          }
      }
      return (MRESULT) TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}


MRESULT EXPENTRY dataCDStatusLineDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWDataFolder *thisPtr;
  HWND hwndMenu;
  POINTL ptl;
  SHORT a,b;
    
  switch(msg)
    {
    case WM_CALCVALIDRECTS:
      {
        if(WinIsWindowVisible(hwnd))
          return handleCalcValidRects(hwnd,  msg,  mp1,  mp2);
        break;
      }
    case WM_WINDOWPOSCHANGED:
      {
        handleStatusWindowPosChanged(hwnd, msg, mp1, mp2);
        break;
      }
    case WM_PAINT:
      {
        if(HlpPaintFrame(hwnd, FALSE))
          return (MRESULT)0;
        break;
      }
    case WM_INITDLG :   
      WinSetWindowULong(hwnd,QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.

      WinSendMsg(WinWindowFromID(hwnd, IDST_STATUSTOTALTIME), EM_SETTEXTLIMIT, MPFROMSHORT((SHORT)CCHMAXPATH), 0);

      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }
      /* subclass static text control for custom painting */
      oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_STATUSTOTALTIME), staticTextProc);
      //WinSetWindowText(WinWindowFromID(hwnd, IDSL_STATUSPERCENTUSED), "0");
      /* Status bar for data folders not yet supported... */
      WinShowWindow(WinWindowFromID(hwnd, IDSL_STATUSPERCENTUSED), FALSE);

      return (MRESULT) TRUE;
      /* A click on the status bar activates the container */
    case WM_BUTTON1CLICK:
      WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),FID_CLIENT));
      return (MRESULT) TRUE;
      /* If the status bar is activated we move the focus to the container. WinSetFocus()
         does not work during WM_ACTIVATE so it's done indirect */
    case WM_ACTIVATE:
      if(SHORT1FROMMP(mp1)) {
        WinPostMsg(hwnd,WM_BUTTON1CLICK,NULL, NULL);
      }    
      return (MRESULT) FALSE;
    case WM_HELP:
      break;
    case WM_DESTROY:
      /* The dialog closes and gets destroyed */     
      break;	
     case WM_COMMAND:	
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(thisPtr){
        switch(SHORT1FROMMP(mp1))
          {
          case IDM_STATUSDEFAULTHELP:
            //       return (MRESULT)thisPtr->wpDisplayHelp(603,AFHELPLIBRARY);
          default:
            break;
          }
      }/* end of if(thisPtr) */
			
      return (MRESULT) TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}


MRESULT EXPENTRY msInfoStatusDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  
  switch(msg)
    {
    case WM_INITDLG :   
      WinShowWindow(WinWindowFromID(hwnd,IDPB_STATUSOK),FALSE);
      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }
      return (MRESULT) TRUE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_ABORTWRITE:
          /* User pressed the ABORT button */
          DosBeep(1000,200);
          WinEnableWindow(WinWindowFromID(hwnd,IDPB_ABORTWRITE),FALSE);          
          WinPostMsg(WinQueryWindow(hwnd,QW_OWNER),WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_ABORT),0);
          break;
        case IDPB_STATUSOK:
          WinPostMsg(WinQueryWindow(hwnd,QW_OWNER),WM_CLOSE,0,0);
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











