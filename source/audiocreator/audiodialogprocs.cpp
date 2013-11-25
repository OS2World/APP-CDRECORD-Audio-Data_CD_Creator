/*
 * This file is (C) Chris Wohlgemuth 1999-2004
 * It is part of the Audio/Data-CD-Creator package
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
#define INCL_GPILCIDS
#define INCL_GPIPRIMITIVES
#define INCL_GPIBITMAPS
#define INCL_DOSERRORS

#include "audiofolder.hh"
#include "audiofolderhelp.h"
#include <stdio.h>
#include <stdlib.h>
#include "cddb.h"

#include "launchpad.hh"
#include "menufolder.hh"

#define MRFALSE (MRESULT)FALSE

extern GLOBALDATA globalData;

extern PVOID pvAudioSharedMem;

extern BOOL GrabberSetupDone;
extern int iMp3Decoder; /* Which decoder to use */

extern int iNumCD;
extern char cFirstCD;
extern char chosenCD[3];

extern BOOL bGrabberFirst;
extern char chrInstallDir[CCHMAXPATH];
extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern char chrAudioCDROptions[CCHMAXPATH];
extern char chosenWriter[3];

extern char chrCdrdaoPath[CCHMAXPATH];
extern char chrCdrdaoDriver[100];
extern char chrDeviceName[CCHMAXPATH];
extern char chrWriterName[CCHMAXPATH];

extern int iBus;
extern int iTarget;
extern int iLun;
extern int iSpeed;

extern BOOL setupDone;
extern ATOM atomUpdateStatusbar;
extern BOOL bUseCDDB;

extern HMODULE hAudioResource;
extern HPOINTER hPtrTBIcons[];

PFNWP  oldProc;  
HWND hwndShadow;// The handle of the help window
extern char chrTBFlyFontName[CCHMAXPATH];/* Font for toolbar fly over help */
extern RGB rgbTBFlyForeground;
extern RGB rgbTBFlyBackground;
extern BOOL bTBFlyOverEnabled;
extern int iTBFlyOverDelay;

extern bHintEnabled;

extern LPList* lplAllLPads;
extern PFNWP pfnwpGenericFrame; /* see audiooveriddenwpmethods.cpp */

extern PVOID pvScanbusSharedMem;

extern HMTX hmtxFileName;
extern int iExt;

/* For custom BG */
extern LOADEDBITMAP allBMPs[NUM_CTRL_IDX];
extern BOOL bUseCustomPainting;
extern CONTROLINFO ciControls[];
extern PFNWP  oldStaticTextProc;

PFNWP  oldButtonProc2;  //place for original button-procedure
HWND hwndBubbleWindow=NULLHANDLE;// The handle of the help window
#define  WM_NEWBUBBLE   WM_USER+100 //Use ATOM later
#define xVal  12      //x-distance of Bubble
#define yVal  8      //y-distance of Bubble


/* Extern */
ULONG cwQueryOSRelease();
HFILE openDrive(char * drive);
void closeDrive(HFILE hfDrive);
int CDQueryAudioCDTracks(HFILE hfDrive);
LONG CDDBDiscID(char * drive, CDDBINFO * cddbInfo);
LONG CDQueryTrackSize(ULONG numTrack, char * drive);
void getTrackName(LONG lDiskID,int iNumTrack,char * text);
void sendConfig();
ULONG launchPMWrapper(PSZ parameter, PSZ folderPath, PSZ wrapperExe, PSZ pszTitle);
ULONG launchDRDialog(PSZ parameter, PSZ resFile, PSZ pszTitle);
ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);
void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
MRESULT cwInsertMenuItem(int iPosition, HWND hwndMenu, HWND hwndSubMenu, int iID, char * chrText);
MRESULT cwInsertMenuSeparator(int iPosition, HWND hwndMenu, HWND hwndSubMenu);
void _Optlink playTimeThreadFunc(void *arg);
void _Optlink toolsThreadFunc (void *arg);
void _Optlink coverThreadFunc (void *arg);
PSZ buildAudioWriteParam(CWAudioFolder* thisPtr, PSZ trackname, int iSpeedLocal);
PSZ buildAudioWriteParamDAO(CWAudioFolder* thisPtr, PSZ pszDevice, int iSpeedLocal);
BOOL buildTocFile( CWAudioFolder *thisPtr, HWND hwndListBox);
ULONG launchWrapper(PSZ parameter, PSZ folderPath,HWND hwnd, PSZ wrapperExe, PSZ title="CDRecord/2");
BOOL checkFileExists(char* chrFileName);
BOOL audioHlpIsMp3File(char* fileName);
BOOL audioHlpStartMp3Query(char *name, HWND hwnd);
MRESULT handleCalcValidRects(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);/* In dataCddialogProcs.cpp */
void handleWindowPosChanged(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);/* In dataCddialogProcs.cpp */
void handleStatusWindowPosChanged(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* New static text drawing proc */
extern MRESULT EXPENTRY staticTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void setupCheckBoxControl(  HWND hwnd, USHORT id);
void setupRadioButtonControl(HWND hwnd, USHORT id);
void setupGroupBoxControl(HWND hwnd, USHORT id);
BOOL HlpPaintFrame(HWND hwnd, BOOL bInclBorder);
void mboxShowMissingProgMsgBox(HWND hwnd, char * chrWhichProg, HMODULE hResource, BOOL bOpenSettings);
void HlpSendCommandToObject(char* chrObject, char* command);

/* Local */
MRESULT EXPENTRY trackDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY statusLineDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY selectDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY grabDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY writeStatusDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY dialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY trackDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;


typedef BOOL   SOMLINK somTP_CWMenuFolder_mfCheckMenuItems(CWMenuFolder *somSelf,
		WPObject* wpObject,
		ULONG ulMenuId);
//#pragma linkage(somTP_CWMenuFolder_mfCheckMenuItems, system)
typedef somTP_CWMenuFolder_mfCheckMenuItems *somTD_CWMenuFolder_mfCheckMenuItems;


BOOL audioHlpStartMp3Query(char *name, HWND hwnd)
{
  char chrCmd[2*CCHMAXPATH+100];

  switch(iMp3Decoder)
    {
    case IDKEY_USEMPG123:
      sprintf(chrCmd,"\"%s\\bin\\mp3info.exe\" \"%s\" %d",chrInstallDir, name, iMp3Decoder);
      break;
    case IDKEY_USEZ: /* We have z! use it for the info */
      sprintf(chrCmd,"\"%s\" \"%s\" %d", globalData.chrMpg123Path, name, iMp3Decoder);
      break;
    case IDKEY_USEMMIOMP3:
      sprintf(chrCmd,"\"%s\" \"%s\" %d",chrInstallDir, name, iMp3Decoder);
      break;
    default:
      return FALSE;
    }

  /* Launch helper */
  if(launchWrapper(chrCmd, "", hwnd,"mp3size.exe","Query mp3 size")==-1)
    return FALSE;/* The helper isn't avaiable or can't be started */

  return TRUE;
}

/************************************************************
 *                                                          *
 * This frame proc handles the about menuitem of the        *
 * Warp 4 menu bar and removes the Tree view item.          *
 *                                                          *
************************************************************/
MRESULT EXPENTRY audioFrameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWAudioFolder* thisPtr;
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
          thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwndDialog,QWL_USER);
          if(somIsObj(thisPtr)) {
            switch(thisPtr->usLastSelMenuItem)
              {
              case 0x2d3:/* Help menu */
                if(hAudioResource) {
                  /* insert separator */
                  cwInsertMenuSeparator(MIT_END, HWNDFROMMP(mp2), NULL);
                  /* Insert 'About' item */
                  /* Text: "About Audio-CD-Creator" */
                  getMessage(text, IDSTR_MENUABOUTA ,sizeof(text), hAudioResource, hwnd);
                  cwInsertMenuItem(MIT_END, HWNDFROMMP(mp2), NULL, ID_ABOUTITEM, text);
                  break;
                }
                break;
                /* Remove change to Tree from menubar menu */ 
              case 0x2d1:
                WinSendMsg(HWNDFROMMP(mp2),MM_REMOVEITEM,MPFROM2SHORT(717,TRUE),0);
                break;
              default:
                break;
              }
          }
        }
      break;
    case WM_COMMAND:
      if(SHORT1FROMMP(mp2)==CMDSRC_MENU) {
        if(SHORT1FROMMP(mp1)>=FIRSTUSERITEM) { 
          BOOL bHandled=FALSE;
          
          TRY_LOUD(AF_MENU) {
            if((hwndDialog=WinWindowFromID(hwnd,IDDLG_STATUS))!=NULLHANDLE)
              {
                thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwndDialog,QWL_USER);
                if(somIsObj(thisPtr)) {
                  CWMenuFolder * wpFolder;
                  WPObject * contentsFile;
                  HOBJECT hObject;
                  somId id;
                  somTD_CWMenuFolder_mfCheckMenuItems methodPtr;
                  M_WPObject *m_wpObject;
                  
                  if((hObject=WinQueryObject(USERMENUFOLDER_AUDIO))!=NULLHANDLE) {//is there a default menufolder?    
                    /* Get class object */
                    m_wpObject=(M_WPObject*)thisPtr->somGetClass();
                    if(somIsObj(m_wpObject)) {
                      /* We have it */
                      id=somIdFromString("mfCheckMenuItems");
                      wpFolder=(CWMenuFolder *)m_wpObject->wpclsQueryObject( hObject);
                      if(somIsObj(wpFolder)) {
                        methodPtr= (somTD_CWMenuFolder_mfCheckMenuItems) (wpFolder->somGetClass())->somFindSMethod(id);  
                        if(methodPtr) {
                          char fileName[CCHMAXPATH];
                          thisPtr->cwCreateContentsFile(fileName,  hwnd);
                          if((hObject=WinQueryObject(fileName))!=NULLHANDLE) {
                            contentsFile=m_wpObject->wpclsQueryObject( hObject);
                            bHandled=methodPtr(wpFolder, contentsFile, SHORT1FROMMP(mp1));
                            contentsFile->wpUnlockObject();//Object is locked because of wpclsQueryHandle()
                          }
                          else
                            /* Check for Items */
                            bHandled=methodPtr(wpFolder, thisPtr, SHORT1FROMMP(mp1));
                        }
                        wpFolder->wpUnlockObject();//Object is locked because of wpclsQueryHandle()
                      }
                    }/* end of if(somIsObj(m_wpObject))  */
                  }
                }/* if(somIsObj(thisPtr)) */
              }/* Dialog window */
          }
          CATCH(AF_MENU)
            {
            } END_CATCH;
            if(bHandled)
              return (MRESULT) 0;
        }
      }
      break;
    case WM_MENUSELECT:
      if((hwndDialog=WinWindowFromID(hwnd,IDDLG_STATUS))!=NULLHANDLE)
        {
          thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwndDialog,QWL_USER);
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
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwndDialog,QWL_USER);
      if(somIsObj(thisPtr)) {
        if(thisPtr->pfnwpFrame)
          return thisPtr->pfnwpFrame(hwnd, msg, mp1, mp2);
      }
    }
  return  pfnwpGenericFrame(hwnd, msg, mp1, mp2);

}


/************************************************************
 *                                                          *
 * This is the procedure for the audio folder container     *
 * which handles the tabbing from the container back to     *
 * frame dialogs.                                           *
 *                                                          *
 ************************************************************/
MRESULT EXPENTRY audioContainerProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWAudioFolder* thisPtr;
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
      if(!timerID && !bEditing) {
        timerID=WinStartTimer(WinQueryAnchorBlock(hwnd),hwnd, 2 , 3000/*(ULONG)iTBFlyOverDelay*/); // New timer for delay
        WinQueryPointerPos(HWND_DESKTOP, &ptl);
        WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1);
      }
      break;
    case WM_TIMER:
      switch (SHORT1FROMMP(mp1))
        {
        case 2: //delay over
          {
            RECTL rectl;
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
                }/* if(pRecord) */
                else {
                  //writeLogPrintf("debug.log", "WM_TIMER: open hint\n");
                  //WinQueryPointerPos(HWND_DESKTOP, &ptl);
                  //WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1);
                  //  writeLogPrintf("debug.log", "WM_TIMER: new ptl.x: %d tempPtl: %d\n\n", ptl.x, tempPtl.x);
                  //              DosBeep(1000,130);
                  launchPMWrapper("1", "<CWCREATOR_SETTINGS>", "pmhint.exe", "Audio-CD-Folder hint");
              
                }
              }
              WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,timerID);  // stop the running timer
              timerID=0;
            }
            else {
              /* Pointer has moved */             
              WinQueryPointerPos(HWND_DESKTOP, &ptl);
              if(WinWindowFromPoint(HWND_DESKTOP, &ptl, TRUE)==hwnd)
                WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1);
              else
                {
                  // ptl.x=20000;
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
                /* Get HWND of select dialog */
                if((hwndDialog=WinWindowFromID(hwndDialog,IDDLG_SELECTDLG))!=NULLHANDLE)
                  {
                    /* Check if write or grab frame control is shown */
                    if(WinQueryButtonCheckstate(hwndDialog,IDRB_GRAB)) {
                      if((hwndDialog=WinWindowFromID(hwndDialog,IDRB_GRAB))!=NULLHANDLE)
                        if(WinSetFocus(HWND_DESKTOP,hwndDialog))
                          return (MRESULT)TRUE;
                    }
                    else {
                      if((hwndDialog=WinWindowFromID(hwndDialog,IDRB_WRITE))!=NULLHANDLE)
                        if(WinSetFocus(HWND_DESKTOP,hwndDialog))
                          return (MRESULT)TRUE;
                    }
                  }
              }
            break;/* error: fall to the window proc */
          }
          else if(SHORT2FROMMP(mp2)==VK_BACKTAB && !(SHORT1FROMMP(mp1) & KC_KEYUP) )
            {
              /* Check for key down to make sure the focus doesn't switch immediately to the next frame control when just coming
                 from another control. Only if the user hits tab again in the container the focus switches. */
              if((hwndDialog=WinQueryWindow(hwnd,QW_PARENT))!=NULLHANDLE)
                {
                  if(WinIsWindowVisible(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),ID_DIALOG))) {
                    WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(hwndDialog,ID_DIALOG),IDPB_BURN));
                    return (MRESULT)TRUE;
                  }
                  else {
                    WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(hwndDialog,ID_DIALOG),IDPB_GRAB));
                    return (MRESULT)TRUE;
                  }
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
          thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwndDialog,QWL_USER);
          if(somIsObj(thisPtr))
            if(thisPtr->pfnwpContainer)
              return thisPtr->pfnwpContainer(hwnd, msg, mp1, mp2);
        }
    }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


void getTrackName(LONG lDiskID,int iNumTrack,char * text)
{
  HINI hini;
  ULONG keyLength;
  char title[100];
  char text2[100];

  /* Text2: "Track %d" */
  /* FIXME */
  /* ACHTUNG: Buffer overflow m”glich!!!!!!!!*/
  getMessage(text2, IDSTR_LBTRACKNAME, sizeof(text2), hAudioResource, HWND_DESKTOP);
  sprintf(text, text2, iNumTrack);
}


static void setWindowSizes(HWND hwnd)
{
  SWP swp, swpHwnd;

  if(  WinQueryWindowPos(hwnd,&swpHwnd)) {
    HENUM hEnum;
    HWND hwndClient;

    hEnum=WinBeginEnumWindows(hwnd);
    while((hwndClient=WinGetNextWindow(hEnum))!=NULLHANDLE) {
      WinQueryWindowPos(hwndClient,&swp);
      WinSetWindowPos(hwndClient, NULLHANDLE, swp.x, swp.y , swpHwnd.cx-2*swp.x, swp.cy, SWP_SIZE);
    }/* while */
    WinEndEnumWindows(hEnum);
  }
}
 
/********************************************************************************/
/*                                                                              */
/* This procedure handles the statusline-dialog (frame extension) of the audio  */
/* folder.                                                                      */
/*                                                                              */
/********************************************************************************/
MRESULT EXPENTRY statusLineDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWAudioFolder *thisPtr;
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
        /* Move the clients of the framecontrol so they keep their distance to the left */
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
      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }
      /* subclass static text control for custom painting */
      oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_STATUSTOTALTIME),staticTextProc);
      WinSetWindowText(WinWindowFromID(hwnd, IDSL_STATUSPERCENTUSED), "0");

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
    case WM_BUTTON2DOWN:
      /* Display popup menu for status bar */
      WinQueryPointerPos(HWND_DESKTOP,&ptl);
      WinMapWindowPoints(HWND_DESKTOP,hwnd,&ptl,1);
      hwndMenu=WinWindowFromPoint(hwnd,&ptl,FALSE);/* Get child window under ptr */
      switch(WinQueryWindowUShort(hwndMenu,QWS_ID))/* Get child window id */
        {
        default:
          a=IDM_STATUSDEFAULTHELP;
          b=a;
          break;
        }
      hwndMenu=WinLoadMenu(hwnd,hAudioResource,a);
      if(hwndMenu) {
        if(WinPopupMenu(hwnd,hwnd,hwndMenu,ptl.x,ptl.y,b,PU_VCONSTRAIN|PU_HCONSTRAIN|
                        PU_MOUSEBUTTON1|PU_KEYBOARD|PU_NONE|PU_SELECTITEM)) {
										
          return (MRESULT)FALSE;
        }
      }
      break;
    case WM_TIMER:
      if(SHORT1FROMMP(mp1)==1) {
        setWindowSizes(hwnd);
        thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
        /* Calculate the play time of the waves and mp3 in another thread */
        if(somIsObj(thisPtr))
          _beginthread(playTimeThreadFunc,NULL,8192*16*4,(void*)thisPtr); //Fehlerbehandlung fehlt    
        WinStopTimer(WinQueryAnchorBlock(HWND_DESKTOP),hwnd,1);
      }
      return (MRESULT) TRUE;			
    case WM_COMMAND:	
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(thisPtr){
        switch(SHORT1FROMMP(mp1))
          {
          case IDM_STATUSDEFAULTHELP:
            return (MRESULT)thisPtr->wpDisplayHelp(603,AFHELPLIBRARY);
          default:
            break;
          }
      }/* end of if(thisPtr) */
			
      return (MRESULT) TRUE;
    default:
      if(msg == atomUpdateStatusbar){ /* Update the statusbar */
        WinStartTimer(WinQueryAnchorBlock(HWND_DESKTOP),hwnd,1,200);
        return (MRESULT) TRUE;
      }
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}


MRESULT EXPENTRY shadowProc(HWND hwnd, ULONG msg,MPARAM mp1,MPARAM mp2 )
{
  switch (msg)
    {
    case WM_PAINT:
      {
        HPS hps;
        HPS hpsMem;
        BITMAPINFOHEADER2 bmpIH2;
        PBITMAPINFO2 pbmp2;
        char * chrBuff;
        PBYTE ptr;
        HDC hdcMem;
        HBITMAP hbm;
        ULONG ulCx, ulCy;
        SWP swp;

        hps=WinBeginPaint(hwnd, NULLHANDLE, NULLHANDLE/*&rectl*/);

        GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
        WinQueryWindowPos(hwnd, &swp);

        ulCx=swp.cx;
        ulCy=swp.cy;

        bmpIH2.cbFix=sizeof(BITMAPINFOHEADER2);
        bmpIH2.cx=ulCx;
        bmpIH2.cy=ulCy;
        bmpIH2.cPlanes=1;
        bmpIH2.cBitCount=8;
        bmpIH2.cbImage=(((ulCx*(1<<bmpIH2.cPlanes)*(1<<bmpIH2.cBitCount))+31)/32)*bmpIH2.cy;

        chrBuff=(char*)malloc(bmpIH2.cbImage+sizeof(BITMAPINFO2)+256*sizeof(RGB2));
        pbmp2=(PBITMAPINFO2)chrBuff;
        memset(pbmp2, 0, sizeof(BITMAPINFO2)+256*sizeof(RGB2));
        ptr=chrBuff+sizeof(BITMAPINFO2)+256*sizeof(RGB2);

        pbmp2->cbFix=sizeof(BITMAPINFO2);
        pbmp2->cx=ulCx;
        pbmp2->cy=ulCy;
        pbmp2->cPlanes=1;
        pbmp2->cBitCount=8;
        pbmp2->cbImage=((pbmp2->cx+31)/32)*pbmp2->cy;
        pbmp2->ulCompression=BCA_UNCOMP;
        pbmp2->ulColorEncoding=BCE_RGB;

        hdcMem=DevOpenDC(WinQueryAnchorBlock(hwnd),OD_MEMORY,"*", 0L/*4L*/, (PDEVOPENDATA)NULLHANDLE/*pszData*/, NULLHANDLE);
        if(hdcMem) {
          SIZEL sizel= {0,0};

          hpsMem=GpiCreatePS(WinQueryAnchorBlock(hwnd), hdcMem, &sizel, PU_PELS|GPIT_MICRO|GPIA_ASSOC);
          if(hpsMem)
            {                 
              hbm=GpiCreateBitmap(hpsMem, &bmpIH2, FALSE, NULL, pbmp2);
              if(hbm) {
                HPS hpsDesktop;
                POINTL ptl[3]={0};
                RGB2 *prgb2;
                int a, r,g,b;
 
                hpsDesktop=WinGetScreenPS(HWND_DESKTOP);
                GpiSetBitmap(hpsMem, hbm);

                ptl[0].x=0;
                ptl[0].y=0;
                ptl[1].x=0+ulCx;
                ptl[1].y=0+ulCy;

                ptl[2].x=swp.x;
                ptl[2].y=swp.y;

                if(GpiBitBlt(hpsMem, hpsDesktop, 3, ptl , ROP_SRCCOPY, BBO_IGNORE)==GPI_ERROR)
                  {
                  }

                WinReleasePS(hpsDesktop);

                if(GpiQueryBitmapBits(hpsMem, 0, ulCy, ptr, pbmp2)==GPI_ALTERROR)
                  {
                    //   DosBeep(500,300);
                  }

                prgb2=(RGB2*)(++pbmp2);
                for(a=0;a<256; a++, prgb2++) {
                  r=-50;
                  g=-50;
                  b=-50;
                  
                  b+=prgb2->bBlue;
                  g+=prgb2->bGreen;
                  r+=prgb2->bRed;
                  if(r>255)
                    r=255;
                  if(r<0)
                    r=0;
                  prgb2->bRed=r;

                  if(g>255)
                    g=255;
                  if(g<0)
                    g=0;
                  prgb2->bGreen=g;

                  if(b>255)
                    b=255;
                  if(b<0)
                    b=0;
                  prgb2->bBlue=b;        
                }

                if(GpiSetBitmapBits(hpsMem, 0, ulCy, ptr, --pbmp2)!=GPI_ALTERROR)
                  {
                    ptl[0].x=0;
                    ptl[0].y=0;
                    ptl[1].x=ulCx;
                    ptl[1].y=ulCy;
                    ptl[2].x=0;
                    ptl[2].y=0;
                    
                    GpiBitBlt(hps, hpsMem, 3, ptl , ROP_SRCCOPY, BBO_IGNORE);
                  }
                GpiSetBitmap(hpsMem, NULLHANDLE);
                GpiDeleteBitmap(hbm);
              }/* hbm */
              GpiDestroyPS(hpsMem);
            }/* hpsMem */
          DevCloseDC(hdcMem);
        }/* if(hdcMem) */
        WinEndPaint(hps);
        free(chrBuff);
        return MRFALSE;
      }
    default:
      break;
    }
  return (*oldProc)(hwnd,msg,mp1,mp2);	
}


/*****************************************************************************/
/*  New button procedure with fly over help	                                 */
/*****************************************************************************/
MRESULT EXPENTRY newButtonProc(HWND hwnd, ULONG msg,MPARAM mp1,MPARAM mp2 )
{
  static BOOL bBubbleOn=TRUE;
  POINTL ptl;
  HPS  hps;
  FONTMETRICS   fm;
  LONG  ulWinTextLen;
  POINTL aptlPoints[TXTBOX_COUNT];
  RECTL   rcl;
  LONG   deltaX,deltaY;
  RGB    rgb= {200,200,0};
  static USHORT id=0;//Initialisation new in V1.00a 
  RECTL  rclWork;
  HWND hwndStore;
  WPObject * wpObject;

  switch (msg)
    {
    case WM_DESTROY:
      WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,1);//Stop timer if running
      if(hwndBubbleWindow) WinDestroyWindow(hwndBubbleWindow);/*  close the bubblewindow  */
      hwndBubbleWindow=0;
      /* Stop delay timer if running */
      WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,WinQueryWindowUShort(hwnd,QWS_ID));			
      break;
    case WM_NEWBUBBLE:
      ULONG bubbleEnabled;
      HWND hwndStore;
      /*  we have to build a new information window  */
      if(hwndBubbleWindow){// if(){...} new in V1.00a 
        WinDestroyWindow(hwndBubbleWindow);/*  close the bubblewindow  */
        hwndBubbleWindow=NULL;
      }
      // Query the pointer position
      WinQueryPointerPos(HWND_DESKTOP,&ptl);
      WinMapWindowPoints(HWND_DESKTOP,hwnd,&ptl,1);
      WinQueryWindowRect(hwnd,&rclWork);				
      if(!hwndBubbleWindow 
         && WinPtInRect(WinQueryAnchorBlock(hwnd),&rclWork,&ptl)
         && bTBFlyOverEnabled) {

        static HWND hwndBubbleClient;
	ULONG style=FCF_BORDER|FCF_NOBYTEALIGN;
	char winText[255];
	
	/* Get window text (It's the object title) for size calculating */
	WinQueryWindowText(hwnd,sizeof(winText),winText);
	ulWinTextLen=(LONG)strlen(winText); // Query text length
	
	/* Delete 'Returns' in object title */
	char *pBuchst;
    char *pRest;
    pRest=winText;
    while((pBuchst=strchr(pRest,13))!=NULL) {
      *pBuchst=' ';
      pBuchst++;
      if(*pBuchst==10)
        *pBuchst=' ';
      pRest=pBuchst;
    }

	/* Create help window */
	hwndBubbleWindow=WinCreateStdWindow(HWND_DESKTOP,
					    0,
					    &style,
					    WC_STATIC,
					    "",
                        SS_TEXT|DT_CENTER|DT_VCENTER,
					    NULLHANDLE,
					    400,
					    &hwndBubbleClient);
    hwndShadow=WinCreateWindow(HWND_DESKTOP,
                               WC_STATIC,
                               "",
                               SS_TEXT|DT_CENTER|DT_VCENTER,
                               0, 0,
                               0, 0,
                               hwndBubbleWindow,
                               hwndBubbleWindow,
                               401,
                               NULLHANDLE,
                               NULLHANDLE);
    oldProc=WinSubclassWindow(hwndShadow, shadowProc);

	// Set the font for the help
	WinSetPresParam(hwndBubbleClient,PP_FONTNAMESIZE,
                    sizeof(chrTBFlyFontName),
                    chrTBFlyFontName);
	/* Calculate text size in pixel */
	hps=WinBeginPaint(hwndBubbleClient,(HPS)NULL,(PRECTL)NULL);
	GpiQueryTextBox(hps,ulWinTextLen,winText,TXTBOX_COUNT,aptlPoints);
	WinEndPaint(hps);
	
	/* Set colors */
    WinSetPresParam(hwndBubbleClient,
                    PP_BACKGROUNDCOLOR,sizeof(rgbTBFlyBackground) ,
                    &rgbTBFlyBackground );
	WinSetPresParam(hwndBubbleClient,
                    PP_FOREGROUNDCOLOR,sizeof(rgbTBFlyForeground) ,
                    &rgbTBFlyForeground );

	/* Calculate bubble positon and show bubble */
	WinQueryPointerPos(HWND_DESKTOP,&ptl);//Query pointer position in the desktop window
	WinQueryWindowRect(HWND_DESKTOP,&rcl);//Query desktop size
	aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x+7+xVal+ptl.x 
	  > rcl.xRight 
	    ? deltaX=-aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x-xVal-xVal-7 
	      : deltaX=0 ;

	aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y+2+yVal+ptl.y 
	  > rcl.yTop 
	    ? deltaY=-aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y-2*yVal-7
	      : deltaY=0 ;		
	WinSetWindowPos(hwndBubbleWindow,
			HWND_TOP,
			ptl.x+xVal+deltaX,ptl.y+yVal+deltaY,  
			aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x+8,
			aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y+2,
			SWP_ZORDER|SWP_SIZE|SWP_MOVE|SWP_SHOW);

    WinSetWindowPos(hwndShadow,
                    hwndBubbleWindow,
                    ptl.x+xVal+deltaX+5
                    ,ptl.y+yVal+deltaY-5,  
                    aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x+8,
                    aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y+2,
                    SWP_ZORDER|SWP_SIZE|SWP_MOVE|SWP_SHOW);

	/* Set bubble text */
	WinSetWindowText(hwndBubbleClient,winText);
	WinStartTimer(WinQueryAnchorBlock(hwnd),hwnd,1,35); 
      } // end if(!hwndBubbleWindow)
      break;
    case WM_MOUSEMOVE:
      USHORT  tempID;
      ULONG delayValue;
      delayValue=250;
   
      tempID=WinQueryWindowUShort(hwnd,QWS_ID);/*  get the id of the window under the pointer  */  			
      if(id!=tempID) {	// New Button?	
        WinStartTimer(WinQueryAnchorBlock(hwnd),hwnd,tempID,(ULONG)iTBFlyOverDelay); // New timer for delay
        id=tempID;  // Save ID 
      }
      else {
        if(!hwndBubbleWindow)WinStartTimer(WinQueryAnchorBlock(hwnd),hwnd,tempID,(ULONG)iTBFlyOverDelay); // New timer for delay	
      }			
      break;
    case WM_TIMER:			
      switch (SHORT1FROMMP(mp1))
        {
        case 1: //Intervall timer
          /* Test pointer position */
          WinQueryPointerPos(HWND_DESKTOP,&ptl);
          WinMapWindowPoints(HWND_DESKTOP,hwnd,&ptl,1);
          WinQueryWindowRect(hwnd,&rclWork);
          if(!WinPtInRect(WinQueryAnchorBlock(hwnd),&rclWork,&ptl))
            {	// Button has changed				 
              WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,1);  // stop the running timer
              if(hwndBubbleWindow) WinDestroyWindow(hwndBubbleWindow);/*  close the bubblewindow  */
              hwndBubbleWindow=0;
              id=0;
            }			 			
          break;
        default:// delay over
          if (SHORT1FROMMP(mp1)==WinQueryWindowUShort(hwnd,QWS_ID)) {//our own timer. It has got the window id
            WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,SHORT1FROMMP(mp1));//Stop the delay timer
            /* Check the pointer position */
            WinQueryPointerPos(HWND_DESKTOP,&ptl);
            WinMapWindowPoints(HWND_DESKTOP,hwnd,&ptl,1);
            WinQueryWindowRect(hwnd,&rclWork);
            if(WinPtInRect(WinQueryAnchorBlock(hwnd),&rclWork,&ptl))
              WinPostMsg(hwnd,WM_NEWBUBBLE,NULL,NULL);//Request a help window
            return (MRESULT)FALSE;
          }
          break;
        }
      break;

    }
  // call the original button procedure to handle the rest of the messages
  return (*oldButtonProc2)(hwnd,msg,mp1,mp2);	
};


/****************************************************************************/
/*                                                                          */
/* This procedure handles the select-dialog (frame extension) of the folder */
/* (at the top)                                                             */
/*                                                                          */
/****************************************************************************/
MRESULT EXPENTRY selectDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWAudioFolder *thisPtr;
  char name[CCHMAXPATH];
  ULONG ulNameSize;
  SIZEL sizel;
  ULONG ulFlags;
  HWND hwndTemp;
  POINTL ptl;
  SHORT a,b;
  HOBJECT hObject;
    
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
        /* Move the clients of the framecontrol so they keep their distance to the left */
        handleWindowPosChanged(hwnd, msg, mp1, mp2);
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
      thisPtr=(CWAudioFolder*) PVOIDFROMMP(mp2);

      /* subclass radio buttons */
      setupRadioButtonControl(hwnd, IDRB_WRITE);
      setupRadioButtonControl(hwnd, IDRB_GRAB);

      WinCheckButton(hwnd,IDRB_GRAB,thisPtr->bShowGrab);
      WinCheckButton(hwnd,IDRB_WRITE,!thisPtr->bShowGrab);
      if(thisPtr->bShowGrab) {
        if(thisPtr->tid) {
          /* There is a grabbing in progress */
          thisPtr->cwEnableSelectControls(FALSE);
        }
      }
#ifdef NO_WPSWIZARD
      setupToolBarButtons(hwnd);
#endif
      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }

#ifdef NO_WPSWIZARD
      /* Build launchpad control */
      if(somIsObj(thisPtr)) {
        SWP swp, swp2;
        thisPtr->lPad=new launchPad(hwnd, hwnd, TRUE, (WPFolder*) PVOIDFROMMP(mp2), &lplAllLPads, NULLHANDLE, 0);
        WinQueryWindowPos(hwnd,&swp);
        WinQueryWindowPos(WinWindowFromID(hwnd, IDPB_TBTUTORIAL),&swp2);
        if(thisPtr->lPad) {
          thisPtr->lPad->lpSetConfiguration(chrInstallDir, AUDIOCD_TBNAME);
          thisPtr->lPad->lpSetLaunchPadPos(NULLHANDLE,  swp2.x+swp2.cx+20, 1,
                                           swp.cx-swp2.x-swp2.cx-30 , swp.cy-2, SWP_SIZE | SWP_MOVE);
          thisPtr->lPad->lpFillPad();
        }
      }
#endif
      return (MRESULT) TRUE;
    case WM_HELP:
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr))
        return (MRESULT)thisPtr->wpDisplayHelp(IDDLG_GRABDIALOG,AFHELPLIBRARY);
      break;
    case WM_DESTROY:
      /* The dialog closes and gets destroyed */     
      break;    
    case WM_BUTTON2CLICK:
      WinQueryPointerPos(HWND_DESKTOP,&ptl);
      WinMapWindowPoints(HWND_DESKTOP,hwnd,&ptl,1);
      hwndTemp=WinWindowFromPoint(hwnd,&ptl,FALSE);/* Get child window under ptr */
      switch(WinQueryWindowUShort(hwndTemp,QWS_ID))/* Get child window id */
        {
        default:
          a=IDM_DEFAULTHELP;
          b=a;
          break;
        }
      hwndTemp=WinLoadMenu(hwnd,hAudioResource,a);
      if(hwndTemp) {
        if(WinPopupMenu(hwnd,hwnd,hwndTemp,ptl.x,ptl.y,b,PU_VCONSTRAIN|PU_HCONSTRAIN|
                        PU_MOUSEBUTTON1|PU_KEYBOARD|PU_NONE|PU_SELECTITEM)) {
                                        
          return (MRESULT)FALSE;
        }
      }
      break;

    case WM_CONTROL:
      /* This part handles the radio button messages */
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(thisPtr){
        switch(SHORT1FROMMP(mp1))
          {
          case IDRB_WRITE:
            hwndTemp=thisPtr->cwQueryFrameCtl(WinQueryWindow(hwnd,QW_PARENT), &sizel,FCTL_LEFT, &ulFlags);
                        
            if(WinQueryWindowUShort(hwndTemp,QWS_ID)==IDDLG_GRABDIALOG)
              thisPtr->hwndGrab=hwndTemp;
            if(!thisPtr->hwndWrite)
              thisPtr->hwndWrite=WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),WinQueryWindow(hwnd,QW_PARENT),
                                            dialogProc,hAudioResource,ID_DIALOG,thisPtr);
            thisPtr->cwAddFrameCtl(WinQueryWindow(hwnd,QW_PARENT),thisPtr->hwndWrite,sizel,FCTL_LEFT, ulFlags);
            WinShowWindow(thisPtr->hwndGrab,FALSE);
                        
            thisPtr->cwUpdateFrame(WinQueryWindow(hwnd,QW_PARENT));
            break;
          case IDRB_GRAB:
            hwndTemp=thisPtr->cwQueryFrameCtl(WinQueryWindow(hwnd,QW_PARENT), &sizel,FCTL_LEFT, &ulFlags);
            if(WinQueryWindowUShort(hwndTemp,QWS_ID)==ID_DIALOG)
              thisPtr->hwndWrite=hwndTemp;
            if(!thisPtr->hwndGrab) {
              thisPtr->hwndGrab=WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),WinQueryWindow(hwnd,QW_PARENT),
                                           grabDialogProc,hAudioResource,IDDLG_GRABDIALOG,thisPtr);
            }
            thisPtr->cwAddFrameCtl(WinQueryWindow(hwnd,QW_PARENT),thisPtr->hwndGrab, sizel,FCTL_LEFT, ulFlags);
            WinShowWindow(thisPtr->hwndWrite,FALSE);
            thisPtr->cwUpdateFrame(WinQueryWindow(hwnd,QW_PARENT)); 
            break;
          default:
            break;
          }
        /* Tell the WPS to save the instance data */
        //thisPtr->wpSaveDeferred();
      }/* if(thisPtr) */
      break;
    case WM_COMMAND:    
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)){
        switch(SHORT1FROMMP(mp1))
          {
          case IDM_DEFAULTHELP:
            return (MRESULT)thisPtr->wpDisplayHelp(600,AFHELPLIBRARY);
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
            _beginthread(coverThreadFunc,NULL,8192*8,thisPtr); //Fehlerbehandlung fehlt
            break;
          case IDPB_TBHELP:
            thisPtr->wpDisplayHelp(IDHLP_MAIN,AFHELPLIBRARY);
            break;
          case IDPB_TBTUTORIAL:
            if((hObject=WinQueryObject("<CWCREATOR_TUTORIAL>"))!=NULL) {
              WinOpenObject(hObject,OPEN_DEFAULT,TRUE);
            } 
            break;
          case IDPB_MP3DECODE:
            thisPtr->cwCreateContentsFileForDec(name,  WinQueryWindow(hwnd, QW_PARENT));
            launchDRDialog(name, "mp3decod.res", "Decode MP3 files");
            break;
          default:
            break;
          }
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
              if(WinIsWindowVisible(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),ID_DIALOG)))
                WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),ID_DIALOG),IDCB_NOFIX));
              else
                WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                                                         IDDLG_GRABDIALOG),IDEF_GRABNAME));
              return (MRESULT)TRUE;
            }
          }
          else if(SHORT2FROMMP(mp2)==VK_BACKTAB )
            {
              if(WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(hwnd,IDRB_WRITE) || 
                 WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(hwnd,IDRB_GRAB)) 
                { 
                  WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),FID_CLIENT));
                  return (MRESULT)TRUE;
                }
            }
        }
      else {
        /* Send chars to other dialogs to check for accelerator keys. */

        /* Check which dialog is visible */
        if(WinIsWindowVisible(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),ID_DIALOG))) {
          WinSetActiveWindow(HWND_DESKTOP,WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT), ID_DIALOG));
          if(!WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT), ID_DIALOG),WM_APPTERMINATENOTIFY, mp1, mp2))
            WinSetActiveWindow(HWND_DESKTOP,hwnd);/* The char wasn't processed by the other dialog so reactivate this dialog */
          else
            return (MRESULT) TRUE;/* The char was eaten so it was an accelerator key */
        }      
        else {
          WinSetActiveWindow(HWND_DESKTOP,WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT), IDDLG_GRABDIALOG));
          if(!WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT), IDDLG_GRABDIALOG),WM_APPTERMINATENOTIFY, mp1, mp2))
            WinSetActiveWindow(HWND_DESKTOP,hwnd);/* The char wasn't processed by the other dialog so reactivate this dialog */
          else
            return (MRESULT) TRUE;/* The char was eaten so it was an accelerator key */
        }
      }
      break;      
    case WM_APPTERMINATENOTIFY:
      /* This is sent as a replacement for a WM_CHAR msg to prevent circles. We give this dialog a chance to eat
         an accelerator key here. This msg comes from another frame control */
      return WinDefDlgProc(hwnd, WM_CHAR, mp1, mp2);/* Let the dialog check if it was an accelerator */

    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static void daoDisableCB(HWND hwnd, BOOL disable)
{
  WinEnableWindow(WinWindowFromID(hwnd,IDCB_NOFIX),!disable);
}

/**************************************************************************/
/*                                                                        */
/* This procedure handles the main dialog (frame extension) of the folder */
/*                                                                        */
/**************************************************************************/
MRESULT EXPENTRY dialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWAudioFolder *thisPtr;
  HWND hwndTracks;
  WPObject * contentObject;
  char name[CCHMAXPATH];
  char text[CCHMAXPATH];
  ULONG ulNameSize;
  HWND hwndCnr,hwndTemp;
  PMINIRECORDCORE mrc;
  ULONG ulWriteFlagsLocal;
  ULONG ulSize;
  POINTL ptl;
  SHORT a,b;
  HOBJECT hObject;
    
  switch(msg)
    {
    case WM_CALCVALIDRECTS:
      {
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
    case WM_APPTERMINATENOTIFY:
      /* This is sent as a replacement for a WM_CHAR msg to prevent circles. We give this dialog a chance to eat
         an accelerator key here. This msg comes from another frame control */
      return WinDefDlgProc(hwnd, WM_CHAR, mp1, mp2);/* Let the dialog check if it was an accelerator */
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
            if(WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(hwnd,IDPB_BURN)) {
              //WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),IDDLG_SELECTDLG),IDRB_WRITE));
              WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),FID_CLIENT));
              return (MRESULT)TRUE;
            }
          }
          else if(SHORT2FROMMP(mp2)==VK_BACKTAB )
            {
              if(WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(hwnd,IDCB_NOFIX)) {
                WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                                                         IDDLG_SELECTDLG),IDPB_TBTUTORIAL));
                return (MRESULT)TRUE;
              }
            }
        }
      else {
        /* Send chars to other dialogs to check for accelerator keys. */
        WinSetActiveWindow(HWND_DESKTOP,WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT), IDDLG_SELECTDLG));
        /* Should use another window msg here. Will be confusing when rereading the source later. But for now I don't
           want to add atom handling */
        if(!WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT), IDDLG_SELECTDLG),WM_APPTERMINATENOTIFY, mp1, mp2))
          WinSetActiveWindow(HWND_DESKTOP,hwnd);/* The char wasn't processed by the other dialog so reactivate this dialog */
        else
          return (MRESULT) TRUE;/* The char was eaten so it was an accelerator key */
        
      }
      break;      
    case WM_INITDLG :
      {

        WinSetWindowULong(hwnd,QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
        thisPtr=(CWAudioFolder*) PVOIDFROMMP(mp2);

        /* subclass checkbox and find mnemonic */
        setupCheckBoxControl(  hwnd , IDCB_NOFIX);
        setupCheckBoxControl(  hwnd , IDCB_DUMMY);
        //setupCheckBoxControl(  hwnd , IDCB_PREEMP);
        //setupCheckBoxControl(  hwnd , IDCB_DAO);

        /* Subclass groupbox */
        setupGroupBoxControl(  hwnd,  IDGB_AUDIOWRITEMODE);

        /* subclass radio buttons */
        setupRadioButtonControl(hwnd, IDRB_AUDIOWRITEMODEDAO);
        setupRadioButtonControl(hwnd, IDRB_AUDIOWRITEMODETAO);

        /* Set the check buttons according the instance write flags 
           The member function cwQueryWriteFlags() returns the current
           state of the flags. */
        ulWriteFlagsLocal=thisPtr->cwQueryWriteFlags();
        WinCheckButton(hwnd,IDCB_NOFIX,(ulWriteFlagsLocal & IDWF_NOFIX)&&TRUE);
        WinCheckButton(hwnd,IDCB_DUMMY,(ulWriteFlagsLocal & IDWF_DUMMY)&&TRUE);
        //  WinCheckButton(hwnd,IDCB_PREEMP,(ulWriteFlagsLocal & IDWF_PREEMP)&&TRUE);
        //  WinCheckButton(hwnd,IDCB_DAO,(ulWriteFlagsLocal & IDWF_DAO)&&TRUE);
        if(ulWriteFlagsLocal & IDWF_DAO) {
          WinCheckButton(hwnd,IDRB_AUDIOWRITEMODEDAO, TRUE);
          WinCheckButton(hwnd,IDRB_AUDIOWRITEMODETAO, FALSE);
          daoDisableCB(hwnd, TRUE);
        }
        else {
          WinCheckButton(hwnd,IDRB_AUDIOWRITEMODETAO, TRUE);
          WinCheckButton(hwnd,IDRB_AUDIOWRITEMODEDAO, FALSE);
        }
        /* Set dialog font to WarpSans for Warp 4 and above */
        if(cwQueryOSRelease()>=40) {
          WinSetPresParam(hwnd,
                          PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                          DEFAULT_DIALOG_FONT );
        }

        return (MRESULT) TRUE;
      }
    case WM_HELP:
      /* Get the object pointer from the window words. We saved it in WM_INITDLG. */
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr))
        /* Display the helppanel IDDLG_GRABDIALOG */
        return (MRESULT)thisPtr->wpDisplayHelp(IDDLG_GRABDIALOG,AFHELPLIBRARY);
      break;
    case WM_DESTROY:
      /* The dialog closes and gets destroyed */
      /* Get the object pointer */     
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)){
        ulWriteFlagsLocal=0;
        /* Query the checkboxes */
        if(WinQueryButtonCheckstate(hwnd,IDCB_NOFIX))
          ulWriteFlagsLocal|=IDWF_NOFIX;
        if(WinQueryButtonCheckstate(hwnd,IDCB_DUMMY))
          ulWriteFlagsLocal|=IDWF_DUMMY;
        //   if(WinQueryButtonCheckstate(hwnd,IDCB_PREEMP)) Preemph not longer supported V0.54
        //   ulWriteFlagsLocal|=IDWF_PREEMP;
        if(WinQueryButtonCheckstate(hwnd,IDRB_AUDIOWRITEMODEDAO))
          ulWriteFlagsLocal|=IDWF_DAO;
        /* Save checkbox states in the instance varables */
        thisPtr->cwSetWriteFlags( ulWriteFlagsLocal,IDWF_DAO|IDWF_DUMMY|IDWF_NOFIX/*|IDWF_PREEMP*/);
        /* Let the WPS save the instance data */
        thisPtr->wpSaveDeferred();
        thisPtr->hwndWrite=0;
      }
      break;    
    case WM_BUTTON2DOWN:
      WinQueryPointerPos(HWND_DESKTOP,&ptl);
      WinMapWindowPoints(HWND_DESKTOP,hwnd,&ptl,1);
      hwndTracks=WinWindowFromPoint(hwnd,&ptl,FALSE);/* Get child window under ptr */
      switch(WinQueryWindowUShort(hwndTracks,QWS_ID))/* Get child window id */
        {
        case IDCB_NOFIX:
          a=IDM_WRITENOFIXHELP;
          b=a;
          break;
        case IDCB_DUMMY:
          a=IDM_WRITEDUMMYHELP;
          b=a;
          break;

          /*case IDCB_PREEMP:
            a=IDM_WRITEPREEMPHELP;
            b=a;
            break;
        case IDCB_DAO:*/
        case IDRB_AUDIOWRITEMODEDAO:
          a=IDM_WRITEDAOHELP;
          b=a;
          break;
        case IDPB_BURN:
          a=IDM_WRITEBURNHELP;
          b=a;
          break;
        default:
          a=IDM_WRITEDEFAULTHELP;
          b=IDM_CDRECORDSETUP;
          break;
        }
      hwndTracks=WinLoadMenu(hwnd,hAudioResource,a);
      if(hwndTracks) {
        if(WinPopupMenu(hwnd,hwnd,hwndTracks,ptl.x,ptl.y,b,PU_VCONSTRAIN|PU_HCONSTRAIN|
                        PU_MOUSEBUTTON1|PU_KEYBOARD|PU_NONE|PU_SELECTITEM)) {
                                        
          return (MRESULT)FALSE;
        }
      }
      break;
    case WM_CONTROL:
      /* This part handles the check button messages */
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(thisPtr){
        ulWriteFlagsLocal=0;
        switch(SHORT1FROMMP(mp1))
          {
          case IDCB_NOFIX:
            if(WinQueryButtonCheckstate(hwnd,IDCB_NOFIX))
              ulWriteFlagsLocal=IDWF_NOFIX;
            thisPtr->cwSetWriteFlags( ulWriteFlagsLocal,IDWF_NOFIX);
            break;
          case IDCB_DUMMY:
            if(WinQueryButtonCheckstate(hwnd,IDCB_DUMMY))
              ulWriteFlagsLocal=IDWF_DUMMY;
            thisPtr->cwSetWriteFlags( ulWriteFlagsLocal,IDWF_DUMMY);
            break;
            /*
              case IDCB_PREEMP:
              if(WinQueryButtonCheckstate(hwnd,IDCB_PREEMP))
              ulWriteFlagsLocal=IDWF_PREEMP;
              thisPtr->cwSetWriteFlags( ulWriteFlagsLocal,IDWF_PREEMP);
              break;
              case IDCB_DAO:
              if(WinQueryButtonCheckstate(hwnd,IDCB_DAO)) {
              ulWriteFlagsLocal=IDWF_DAO;
              daoDisableCB(hwnd, TRUE);
              }
              else
              daoDisableCB(hwnd, FALSE);
              thisPtr->cwSetWriteFlags( ulWriteFlagsLocal,IDWF_DAO);
              thisPtr->cwForceStatusUpdate();
              break;
              */
          case IDRB_AUDIOWRITEMODEDAO:
            ulWriteFlagsLocal=IDWF_DAO;
            daoDisableCB(hwnd, TRUE);
            thisPtr->cwSetWriteFlags( ulWriteFlagsLocal, IDWF_DAO);
            thisPtr->cwForceStatusUpdate();
            break;
          case IDRB_AUDIOWRITEMODETAO:
            daoDisableCB(hwnd, FALSE);
            thisPtr->cwSetWriteFlags( ulWriteFlagsLocal, IDWF_DAO);
            thisPtr->cwForceStatusUpdate();
            break;
          default:
            break;
          }
        /* Tell the WPS to save the instance data */
        thisPtr->wpSaveDeferred();
      }
      break;
    case WM_COMMAND:    
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)) {
        switch(SHORT1FROMMP(mp1))
          {
            /* Menu slection from popup menus */
          case IDM_WRITEDEFAULTHELP:
            return (MRESULT)thisPtr->wpDisplayHelp(601,AFHELPLIBRARY);
          case IDM_WRITENOFIXHELP:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_WRITENOFIX,AFHELPLIBRARY);
          case IDM_WRITEDUMMYHELP:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_WRITEDUMMY,AFHELPLIBRARY);
            //case IDM_WRITEPREEMPHELP:
            //return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_WRITEPREEMP,AFHELPLIBRARY);
          case IDM_WRITEDAOHELP:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_WRITEDAO,AFHELPLIBRARY);
          case IDM_WRITEBURNHELP:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_WRITEBURN,AFHELPLIBRARY);            
          case IDM_CDRECORDSETUP:
            bGrabberFirst=FALSE;
            if((hObject=WinQueryObject("<CWCREATOR_SETTINGS>"))!=NULL) {
              WinOpenObject(hObject,OPEN_DEFAULT,TRUE);
            } 
            else 
              /*
                Text: "Can't open settings notebook."
                Title: "Audio-CD-Creator"                           
                */             
              messageBox( text, IDSTR_OPENSETTINGSERROR, sizeof(text),
                          name, IDSTR_AUDIOCDCREATOR, sizeof(name),
                          hAudioResource, hwnd, MB_OK | MB_ERROR | MB_MOVEABLE);
                      
            return (MRESULT)TRUE;
          default:
            break;
          }
      }
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_BURN:
          {
            /* Write CD button pressed */
            char fileName[CCHMAXPATH];
            ulSize=0;            

            thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
            if(!somIsObj(thisPtr))
              break;
            if(!setupDone){
              HOBJECT hObject;
              if((hObject=WinQueryObject(ID_CREATORSETTINGS))!=NULLHANDLE)
                WinOpenObject(hObject, OPEN_DEFAULT,TRUE);
            }
            else{
              ulWriteFlagsLocal=thisPtr->cwQueryWriteFlags();
              if(ulWriteFlagsLocal & IDWF_DAO) {
                /* We are writing DAO, launch the DrDialog app */
                thisPtr->cwCreateContentsFile(fileName,  WinQueryWindow(hwnd,QW_PARENT));
                launchDRDialog(fileName, "DAOAudio.res", "Write audio CD -DAO mode-");
                break;
              }
              if(pvAudioSharedMem) {
                WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP,"There is already a CD writing process running." ,"Write audio tracks",
                                0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
                break;
              }
              /* Load the track list dialog */
              hwndTracks=WinLoadDlg(HWND_DESKTOP,hwnd,trackDialogProc,hAudioResource,IDDLG_SHOWTITLES,thisPtr);
              WinSetActiveWindow(HWND_DESKTOP,hwndTracks);
            }
            break;
          }
        default:
          break;
        }
      return (MRESULT) TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}


/**************************************************************************/
/*                                                                        */
/* This procedure handles the grab-dialog (frame extension) of the folder */
/*                                                                        */
/**************************************************************************/
MRESULT EXPENTRY grabDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWAudioFolder *thisPtr;
  int numTracks;
  HFILE hfCD;
  char text[CCHMAXPATH*4];
  char title[CCHMAXPATH];
  ULONG ulSize;    
  SHORT a,b;
  ULONG ulFlags;
  SIZEL sizel;
  HWND hwndTemp;
  POINTL ptl;
  HOBJECT hObject;
  CDDBINFO cddbInfo;
  SHORT lTrackLength[99];/* CD allows 99 tracks */
  
  switch(msg)
    {
    case WM_CALCVALIDRECTS:
      {
        if(WinIsWindowVisible(hwnd))
          return handleCalcValidRects(hwnd,  msg,  mp1,  mp2);
        break;
      }
    case WM_PAINT:
      {
        if(HlpPaintFrame(hwnd, FALSE))
          return (MRESULT)0;
        return WinDefDlgProc(hwnd, msg, mp1, mp2);
      }
    case WM_APPTERMINATENOTIFY:
      /* This is sent as a replacement for a WM_CHAR msg to prevent circles. We give this dialog a chance to eat
         an accelerator key here. This msg comes from another frame control */
      return WinDefDlgProc(hwnd, WM_CHAR, mp1, mp2);/* Let the dialog check if it was an accelerator */
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
            if(WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(hwnd,IDPB_GRAB)) {
              WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),IDDLG_SELECTDLG),IDRB_GRAB));
              return (MRESULT)TRUE;
            }
            if(WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(hwnd,IDPB_REFRESH) && 
               !WinIsWindowEnabled(WinWindowFromID(hwnd,IDPB_GRAB))) {
              WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),IDDLG_SELECTDLG),IDRB_GRAB));
              return (MRESULT)TRUE;
            }
          }
          else if(SHORT2FROMMP(mp2)==VK_BACKTAB )
            {
              if(WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(hwnd,IDEF_GRABNAME)) {
                WinSetFocus(HWND_DESKTOP,WinWindowFromID(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                                                         IDDLG_SELECTDLG),IDPB_TBTUTORIAL));
                
                return (MRESULT)TRUE;
              }
            }
        }
      else {
        /* Send chars to other dialogs to check for accelerator keys. */
        WinSetActiveWindow(HWND_DESKTOP,WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT), IDDLG_SELECTDLG));
        /* Should use another window msg here. Will be confusing when rereading the source later. But for now I don't
           want to add atom handling */
        if(!WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT), IDDLG_SELECTDLG),WM_APPTERMINATENOTIFY, mp1, mp2))
          WinSetActiveWindow(HWND_DESKTOP,hwnd);/* The char wasn't processed by the other dialog so reactivate this dialog */
        else
          return (MRESULT) TRUE;/* The char was eaten so it was an accelerator key */
      }
      break;      
    case WM_INITDLG :   
      WinSetWindowULong(hwnd,QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      thisPtr=(CWAudioFolder*) PVOIDFROMMP(mp2);
      WinEnableWindow(WinWindowFromID(hwnd,IDPB_SELECTALL) ,FALSE);
      WinEnableWindow(WinWindowFromID(hwnd,IDPB_GRAB), FALSE);
      WinEnableWindow(WinWindowFromID(hwnd,IDPB_GRABTOMP3), FALSE);
      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }
      /* subclass static text control for custom painting */
      oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_GRABNAME),staticTextProc);
      /* Subclass groupbox */
      setupGroupBoxControl(  hwnd, IDGB_GRABDIALOG);
      return (MRESULT) TRUE;
    case WM_DESTROY:
      /* The dialog closes and gets destroyed */     
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)){
        thisPtr->hwndGrab=0;
        thisPtr->pfnwpGrabLB=NULL;
      }
      break;    
    case WM_HELP:
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr))
        return (MRESULT)thisPtr->wpDisplayHelp(IDDLG_GRABDIALOG,AFHELPLIBRARY);
      break;
    case WM_CONTROL:
      /* This part handles the radio button messages */
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)){
        switch(SHORT1FROMMP(mp1))
          {
          default:                     
            break;
          }
        /* Tell the WPS to save the instance data */
      }
      break;
    case WM_BUTTON2DOWN:
      //    case WM_CONTEXTMENU:
      /* Handle popup menu */
      WinQueryPointerPos(HWND_DESKTOP,&ptl);
      WinMapWindowPoints(HWND_DESKTOP,hwnd,&ptl,1);
      hwndTemp=WinWindowFromPoint(hwnd,&ptl,FALSE);/* Get child window under ptr */
      switch(WinQueryWindowUShort(hwndTemp,QWS_ID))/* Get child window id */
        {
        case IDLB_GRABTRACKS:
          a=IDM_GRABTRACKLISTHELP;
          b=IDPB_REFRESH;
          break;
        case IDPB_GRAB:
          a=IDM_GRABGRABHELP;
          b=a;
          break;
        case IDPB_REFRESH:
          a=IDM_GRABREFRESHHELP;
          b=a;
          break;
        case IDPB_SELECTALL:
          a=IDM_GRABSELECTALLHELP;
          b=a;
          break;
        default:
          a=IDM_GRABDEFAULTHELP;
          b=IDM_GRABBERSETUP;
          break;
        }
      hwndTemp=WinLoadMenu(hwnd,hAudioResource,a);
      if(hwndTemp) {
        if(WinPopupMenu(hwnd,hwnd,hwndTemp,ptl.x,ptl.y,b,PU_VCONSTRAIN|PU_HCONSTRAIN|
                        PU_MOUSEBUTTON1|PU_KEYBOARD|PU_NONE|PU_SELECTITEM)) {
          if(!(a==IDM_GRABTRACKLISTHELP && WinIsControlEnabled(hwnd,IDPB_SELECTALL)))
            WinEnableMenuItem(hwndTemp,IDPB_SELECTALL,FALSE);
                    
          return (MRESULT)FALSE;
        }
      }
      break;
    case WM_COMMAND:    
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)) {
        switch(SHORT1FROMMP(mp1))
          {
            /* These are the popup menu items */
          case IDM_GRABDEFAULTHELP:
            return (MRESULT)thisPtr->wpDisplayHelp(602,AFHELPLIBRARY);
            break;
          case IDM_GRABGRABHELP:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GRABGRAB,AFHELPLIBRARY);
            break;
          case IDM_GRABTRACKLISTHELP:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GRABTRACKLIST,AFHELPLIBRARY);
            break;                  
          case IDM_GRABREFRESHHELP:                   
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GRABREFRESH,AFHELPLIBRARY);
            break;
          case IDM_GRABSELECTALLHELP:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_GRABSELECTALL,AFHELPLIBRARY);
            break;
          case IDM_GRABBERSETUP:
            bGrabberFirst=TRUE;
            if((hObject=WinQueryObject("<CWCREATOR_SETTINGS>"))!=NULL) {
              WinOpenObject(hObject,OPEN_DEFAULT,TRUE);
            }
            else
              /*
                Text: "Can't open settings notebook."
                Title: "Audio-CD-Creator"                           
                */             
              messageBox( text, IDSTR_OPENSETTINGSERROR, sizeof(text),
                          title, IDSTR_AUDIOCDCREATOR, sizeof(title),
                          hAudioResource, hwnd, MB_OK | MB_ERROR | MB_MOVEABLE);
            return (MRESULT)TRUE;
            /* push buttons */
          default:
            break;
          }
      } /* end of if(thisPtr) */
                        
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_GRABTOMP3:
          {
            char fileName[CCHMAXPATH];

            thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
            if(somIsObj(thisPtr)) {            
              /* Get num Tracks */
              numTracks=(int)SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_GRABTRACKS),LM_QUERYITEMCOUNT,0,0));
              if(!numTracks)
                break;/* Catch error */                        
              a=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_GRABTRACKS),LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0));
              if(a==LIT_NONE) {
                /*
                  Text: "There are no tracks selected"
                  Title: "Grab tracks message"                           
                  */             
                messageBox( text, IDSTR_NOTRACKSSELECTEDTEXT , sizeof(text),
                            title, IDSTR_NOTRACKSSELECTEDTITLE, sizeof(title),
                            hAudioResource, hwnd, MB_OK | MB_INFORMATION | MB_MOVEABLE);
                break;
              }

              thisPtr->cwCreateContentsFile(fileName,  WinQueryWindow(hwnd,QW_PARENT));
              launchDRDialog(fileName, "flyencod.res", "Encode MP3 files on the fly");
            }
            break;
          }
        case IDPB_GRAB:
          {
            thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
            if(somIsObj(thisPtr)) {            
              WinQueryWindowText( WinWindowFromID(hwnd,IDEF_GRABNAME),sizeof(title),title);
              if(!strlen(title)) {
                /* Text:  "You must specify a filename."
                   Title: "Grab tracks message"
                   */
                messageBox( text, IDSTR_NOTRACKNAMETEXT , sizeof(text),
                            title, IDSTR_NOTRACKSSELECTEDTITLE, sizeof(title),
                            hAudioResource, hwnd, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
                break;
              }
              
              /* Get num Tracks */
              numTracks=(int)SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_GRABTRACKS),LM_QUERYITEMCOUNT,0,0));
              if(!numTracks)
                break;/* Catch error */                        
              a=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_GRABTRACKS),LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0));
              if(a==LIT_NONE) {
                /*
                  Text: "There are no tracks selected"
                  Title: "Grab tracks message"                           
                  */             
                messageBox( text, IDSTR_NOTRACKSSELECTEDTEXT , sizeof(text),
                            title, IDSTR_NOTRACKSSELECTEDTITLE, sizeof(title),
                            hAudioResource, hwnd, MB_OK | MB_INFORMATION | MB_MOVEABLE);
                break;
              }

              /* The track name to be used */
              sprintf(text,"\"%s\"",title);
              /* Add tracknumbers to cmd line */
              b=0;
              while(a!=LIT_NONE) {
                sprintf(title," %d",a+1);
                strcat(text,title);
                lTrackLength[b]=CDQueryTrackSize( a+1, chosenCD)/4/44100;/* Tracksize in sec */
                a=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_GRABTRACKS),LM_QUERYSELECTION,MPFROMSHORT(a),0));
                b++;
              };
              /* Add cddb discid */
              sprintf(title," %08x",CDDBDiscID(chosenCD, &cddbInfo));
              strcat(text,title);
              
              /* Add tracksizes */
              for(a=0;a<b;a++) {
                sprintf(title," %d", lTrackLength[a]);
                strcat(text,title);
              }
              
              /* Query folder name */
              ulSize=sizeof(title);
              thisPtr->wpQueryRealName(title,&ulSize,TRUE);
              
              if(checkFileExists(globalData.chrGrabberPath)) {
                if(globalData.iGrabberID==IDGRABBER_UNKNOWN)
                  launchPMWrapper(text,title, "pacdgrab.exe","Grab Tracks PM"); /* Uses statusdialog without percent
                                                                                   bar for unknown grabbers */
                else
                  launchPMWrapper(text,title, "pacdgrb2.exe","Grab Tracks PM"); /* Uses statusdialog with 
                                                                                   percentbar for known grabbers */
              }
              else {
                /* The path to the grabber isn't valid */
                /* name: "...%s..." */
                getMessage(title, IDSTR_NOVALIDPATH,sizeof(title), hAudioResource, hwnd);            
                sprintf(text, title, "the grabber program");
                /* name: "Audio-CD-Creator" */
                getMessage(title, IDSTR_AUDIOCDCREATOR,sizeof(title), hAudioResource, hwnd);                     
                WinMessageBox(  HWND_DESKTOP, hwnd,
                                text,
                                title,
                                0UL, MB_OK | MB_ERROR |MB_MOVEABLE);
              }/* else if(checkFileExists(chrCdrdaoPath)) */
              break;
            }/* end of if(thisPtr) */
            break;
          }
        case IDPB_SELECTALL:
          numTracks=(int)LONGFROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_GRABTRACKS),LM_QUERYITEMCOUNT,0,0));
          if(!numTracks)
            break;
          for(a=0;a<numTracks;a++) {
            WinSendMsg(WinWindowFromID(hwnd,IDLB_GRABTRACKS),LM_SELECTITEM,MPFROMSHORT(a),MPFROMSHORT(TRUE));
          }
          break;
        case IDPB_REFRESH:
          int numTracks;
          int a;
          LONG lCddbId;

          thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
          if(somIsObj(thisPtr)) { 
            if(!GrabberSetupDone){
              thisPtr->wpViewObject(NULL,OPEN_SETTINGS,0);
              break;      
            }

            WinEnableWindow(WinWindowFromID(hwnd,IDEF_GRABNAME), TRUE);
            
            WinSendMsg(WinWindowFromID(hwnd,IDLB_GRABTRACKS),LM_DELETEALL,0,
                       0);
            WinEnableWindow(WinWindowFromID(hwnd,IDPB_SELECTALL) ,FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDPB_GRAB) ,FALSE);
            hfCD=openDrive(chosenCD);
            if(hfCD) {
              numTracks=(int)CDQueryAudioCDTracks(hfCD);
              if(numTracks) {
                if(numTracks==-1) {
                  /*
                    Text: "The Disk is a data disk!"
                    Title: "Track grabbing"                       
                    */             
                  messageBox( text, IDSTR_ERRORISDATACDTEXT , sizeof(text),
                              title, IDSTR_ERRORISDATACDTITLE, sizeof(title),
                              hAudioResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
                }
                else {
                  /* Insert the items into the listbox */
                  lCddbId=CDDBDiscID(chosenCD,&cddbInfo);
                  /* Message: "CDDB-ID: %X, Querying local database..." */
                  getMessage(title,IDSTR_CDDBQUERYLOCALDB, sizeof(title), hAudioResource,hwnd);
                  sprintf(text,title,lCddbId);
                  
                  /* Open CDDB.ini and read track names */                  
                  for(a=1;a<=numTracks;a++) {
                    getTrackName(lCddbId, a, text);             
                    WinInsertLboxItem(WinWindowFromID(hwnd,IDLB_GRABTRACKS),LIT_END,
                                      text);
                  }/* end of for */
                  WinEnableWindow(WinWindowFromID(hwnd,IDPB_SELECTALL), TRUE);
                  WinEnableWindow(WinWindowFromID(hwnd,IDPB_GRAB), TRUE);
                  WinEnableWindow(WinWindowFromID(hwnd,IDPB_GRABTOMP3), TRUE);
                }
              }// end of if(numTracks)
            
              closeDrive(hfCD);
            }/* if(hfCD) */
            else {
              /*
                Text: "Please insert the CD into the drive and press 'Refresh' again."
                Title: "Track grabbing"                       
                */             
              messageBox( text, IDSTR_ERRORNODISK , sizeof(text),
                          title, IDSTR_ERRORISDATACDTITLE, sizeof(title),
                          hAudioResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
            }
            if(bUseCDDB) {
              /* Get num Tracks */
              numTracks=(int)LONGFROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_GRABTRACKS),LM_QUERYITEMCOUNT,0,0));
              if(!numTracks)
                break;/* Catch error */                        
              
              sprintf(text,"\"%s\"",title);
              /* Add tracknumbers to cmd line */
              for(a=1;a<=numTracks;a++) {
                sprintf(title," %d",a);
                strcat(text,title);
              };
              /* Add cddb discid */
              sprintf(title," %x",CDDBDiscID(chosenCD,&cddbInfo));
              strcat(text,title);
              
              /* Query folder name */
              ulSize=sizeof(title);
              thisPtr->wpQueryRealName(title,&ulSize,TRUE);
              
              launchPMWrapper(text,title, "ptrkname.exe","Query tracknames PM");
            }
          }/* (if thisPtr) */
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

/************************************************************************************/


/***********************************************/
/*                                             */
/* This procedure handles the tracklist dialog */
/* and starts the writing process              */
/*                                             */
/***********************************************/
MRESULT EXPENTRY trackDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWAudioFolder *thisPtr;
  char * text;
  ULONG ulBufferSize;
  ULONG ulFlags,ulSize;
  char name[CCHMAXPATH];
  char title [CCHMAXPATH+10];
  char anzahl[100];
  int b;
  int a;
  ULONG rc;
  HWND hwndCnr;

  switch(msg)
    {
    case WM_PAINT:
      {
        if(HlpPaintFrame(hwnd, TRUE))
          return (MRESULT)0;
        return WinDefDlgProc(hwnd, msg, mp1, mp2);
      }
    case WM_INITDLG :	
      WinSetWindowULong(hwnd,QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
    
      thisPtr=(CWAudioFolder*)PVOIDFROMMP(mp2);
      if(!somIsObj(thisPtr)) {
        WinPostMsg(hwnd,WM_CLOSE,0,0);/* Error, quit. */
        return (MRESULT) TRUE;
      }

      thisPtr->hwndWriteLB=WinWindowFromID(hwnd,IDLB_TRACKLIST);
      
      /* Disable controls in write dialog */
      thisPtr->cwEnableWriteControls(FALSE);
      /* Show Warning text if in DAO mode */ 
      ulFlags=thisPtr->cwQueryWriteFlags();
      if(ulFlags&IDWF_DAO)
        WinShowWindow(WinWindowFromID(hwnd,IDST_MP3DISCARDED),TRUE);
      else
        WinShowWindow(WinWindowFromID(hwnd,IDST_MP3DISCARDED),FALSE);

      /* Speed spin button */
      WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETLIMITS,MPFROMLONG(64),MPFROMLONG(1));
      WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETCURRENTVALUE,(MPARAM)iSpeed,MPFROMLONG(0));
      
      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }

      /* subclass static text control for custom painting */
      oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_MP3DISCARDED),staticTextProc);
      oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_TOTALSIZE),staticTextProc);
      oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_TOTALTIME),staticTextProc);
      oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_TEXT1),staticTextProc);
      /* Subclass groupbox */
      setupGroupBoxControl(  hwnd, IDGB_TRACKSTOWRITE);
      setupGroupBoxControl(  hwnd, IDGB_WRONGFORMAT);
      setupGroupBoxControl(  hwnd, IDGB_SELECTSPEED);
      setupGroupBoxControl(  hwnd, IDGB_SELECTWRITER);
      /* Get folder hwnd */
      hwndCnr=WinQueryWindow(thisPtr->hwndWrite,QW_PARENT);
      /* Fill the wrong format listbox with all tracks. We will check these tracks later
         and add correct tracks to the track listbox. */
      thisPtr->cwFillTrackList(hwndCnr,WinWindowFromID(hwnd,IDLB_WRONGTRACKS));

      /* Wrong tracks were already discarded while claculating the total size during 
         CWAudioFolder::cwForceStatusUpdate(void). */
      ulSize=thisPtr->ulTrackSize;
      /* Text: "Total size: %d.%d.%d Bytes" */
      getMessage(title,IDSTR_TOTALSIZE,sizeof(title), hAudioResource,hwnd);
      sprintf(name,title,ulSize/1000000,(ulSize%1000000)/1000,(ulSize%1000));
      WinSetWindowText(WinWindowFromID(hwnd,IDST_TOTALSIZE),name);
      
      ulSize/=(2*2*44100);/* Calculate seconds */
      /* Text: "Total time: %d min %d sec" */
      getMessage(title,IDSTR_TOTALTIME,sizeof(title), hAudioResource,hwnd);
      sprintf(name,title, (ulSize-(ulSize % 60))/60 , (ulSize % 60));
      WinSetWindowText(WinWindowFromID(hwnd,IDST_TOTALTIME),name);
      
      /* Now start format checking. */
      a=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_WRONGTRACKS), LM_QUERYITEMCOUNT, NULL, NULL));
      if(!a){
        /* There're no tracks at all! */
        /* Enable controls in write dialog */
        thisPtr->cwEnableWriteControls(TRUE);
        return (MRESULT) TRUE;
      }

      /* Disable 'ok' button during check */
      WinEnableWindow(WinWindowFromID(hwnd,IDPB_OK),FALSE);
      SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_WRONGTRACKS),LM_QUERYITEMTEXT,
                              MPFROM2SHORT((SHORT)0,(SHORT)sizeof(name)),&name));

      //Save current index and total tracks. In the window word */
      WinSetWindowULong(WinWindowFromID(hwnd,IDLB_WRONGTRACKS),QWL_USER,(ULONG)(a<<16));
      
      /* ********* Check files with external helpers *********/
      /* Check, if it's a mp3 file */
      if(audioHlpIsMp3File(name)){
        /* Launch helper */
        if(!audioHlpStartMp3Query(name, hwnd)) {
          /*
            Text:   "The MP3 decoder program is not installed in the selected path or can't be started!"
            Title: "Audio-CD-Creator"                       
            */             
          messageBox( name, IDSTR_MP3HELPERSTARTERROR , sizeof(name),
                      title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                      hAudioResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
          WinPostMsg(hwnd,WM_CLOSE,0,0);/* The helper isn't available or can't be started */
          return (MRESULT) TRUE;
        }
      } /* End of audioHlpMp3File(name) */           
      else {
        /* It's a wave */
        /* Launch helper */
        sprintf(title," \"%s\"", name);
        if(launchWrapper(title, "", hwnd,"waveinfo.exe","Query wavefile size")==-1) {
          /*
            Text:   "The program WAVEINFO.EXE is not available in the BIN directory or
            can't be started! Make sure MMOS2 is installed."
            Title: "Audio/Data-CD-Creator installation problem!"                       
            */             
          messageBox( name, IDSTR_WAVEINFOSTARTERROR, sizeof(name),
                      title, IDSTR_INSTALLERRORTITLE , sizeof(title),
                      hAudioResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
          WinPostMsg(hwnd,WM_CLOSE,0,0);/* The helper isn't avaiable or can't be started */
          return (MRESULT) TRUE;
        }
      }/* else */
      /********** Check files with MMIO procedures **************/
      WinShowWindow(hwnd, TRUE);
      return (MRESULT) TRUE;
      /* WM_APPTERMINATENOTIFY messages are sent from the helper programs e.g. format checker. */
    case WM_APPTERMINATENOTIFY:
      switch(LONGFROMMP(mp1))
        {
        case ACKEY_SCANBUS:
          /* Free shared mem if not already done */
          if(pvScanbusSharedMem) {
            DosFreeMem(pvScanbusSharedMem);
            pvScanbusSharedMem=NULL;
          }
          WinEnableWindow(WinWindowFromID(hwnd,IDPB_OK),TRUE);
          WinShowWindow(WinWindowFromID(hwnd,IDST_TEXT1),FALSE);
          break;
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
              sprintf(name,"%d,%d,%d", iBus, iTarget, iLun);
              if(strstr((char*) PVOIDFROMMP(mp2), name))
                WinSetWindowText(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),(PCSZ)PVOIDFROMMP(mp2));
            }
          break;
        case ACKEY_PLAYTIME:
          /* The format checker sent this messages. If size is 0 the format is wrong. */
          a=WinQueryWindowULong(WinWindowFromID(hwnd,IDLB_WRONGTRACKS),QWL_USER);
          b=a & 0xffff;/* Current index */
          a>>=16;/* Num items in listbox */
          if(LONGFROMMP(mp2)) {      
            /* Track has valid format */
            /* Get name */
            SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_WRONGTRACKS),LM_QUERYITEMTEXT,
                                    MPFROM2SHORT((SHORT)b,(SHORT)sizeof(name)),&name));
            /* Insert name into valid tracks box */
            WinInsertLboxItem(WinWindowFromID(hwnd,IDLB_TRACKLIST), LIT_END,
                              name);
            /* Remove the track from the wrong format list box */
            WinSendMsg(WinWindowFromID(hwnd,IDLB_WRONGTRACKS),LM_DELETEITEM,MPFROMSHORT((SHORT)b),NULL);
            a--;
            b--;
          }

          if(++b<a) {
            //Save current index and total tracks.
            WinSetWindowULong(WinWindowFromID(hwnd,IDLB_WRONGTRACKS),QWL_USER,(ULONG)((a<<16) + b));
            if(!SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd,IDLB_WRONGTRACKS),LM_QUERYITEMTEXT,
                                        MPFROM2SHORT((SHORT)b,(SHORT)sizeof(name)),&name)))
              return FALSE;
            
            /* Check, if it's a mp3 file */
            if(audioHlpIsMp3File(name)){
              /* Launch helper */
              if(!audioHlpStartMp3Query(name, hwnd)) {
                /*
                  Text:   "The MP3 decoder program is not installed in the selected path or can't be started!"
                  Title: "Audio-CD-Creator"                       
                  */             
                messageBox(name, IDSTR_MP3HELPERSTARTERROR , sizeof(name),
                            title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                            hAudioResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);

                WinPostMsg(hwnd,WM_CLOSE,0,0);/* The helper isn't avaiable or can't be started */
              }
            } /* End of audioHlpMp3File(name) */           
            else {
              /* Launch helper */
              sprintf(title," \"%s\"", name);
              if(launchWrapper(title, "", hwnd,"waveinfo.exe","Query wavefile size")==-1) {
                /*
                  Text:   "The program WAVEINFO.EXE is not available in the BIN directory or
                  can't be started! Make also sure MMOS2 is installed."
                  Title: "Audio/Data-CD-Creator installation problem!"                       
                  */             
                messageBox( name, IDSTR_WAVEINFOSTARTERROR, sizeof(name),
                            title, IDSTR_INSTALLERRORTITLE , sizeof(title),
                            hAudioResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
                WinPostMsg(hwnd,WM_CLOSE,0,0);/* The helper isn't avaiable or can't be started */
              }
            } 
          }
          else{ /* No more tracks to check */
            /* Fill drop down listbox with the avaiable devices by calling scanbus */
            if(checkFileExists(chrCDRecord)) {/* check if cdrecord is avaiable */
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
                              hAudioResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
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
                              hAudioResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
                  break;
                }
              if(launchWrapper(chrInstallDir,"",hwnd,"scanbus.exe","Scanning SCSI bus...")) {
                if(pvScanbusSharedMem) {
                  DosFreeMem(pvScanbusSharedMem);
                  pvScanbusSharedMem=NULL;
                }
              }
            }
            else {
              /* The path to CDRecord/2 isn't valid */
              mboxShowMissingProgMsgBox(hwnd, "CDRecord/2", hAudioResource, TRUE);
              /* Show the cdrecord setup help panel */
              HlpSendCommandToObject("<CWCREATOR_SETTINGS>", SETUP_DISPLAYHELPPANEL"="IDHLP_CDRECORDSETUPSTR);
            }
            break;
          }
          return FALSE;

        default:
          break;
        }
      return FALSE;
    case WM_DESTROY:
      /* The dialog closes and gets destroyed */     
      /* Enable windows in write dialog */
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(!somIsObj(thisPtr))
        break;/* We don't clean up. Better than trapping... */
      thisPtr->hwndWriteLB=0;
      thisPtr->cwEnableWriteControls(TRUE);
      break;

    case WM_CONTROL:
      /* Catch WM_CONTROL to check for doubleclicks in the listbox. */      
      if(SHORT2FROMMP(mp1)==LN_ENTER) {
        /* Check for doubleclick in listbox */
        thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
        if(!somIsObj(thisPtr))
          break;/* Error */
        /* Get selected item */
        if((a=SHORT1FROMMR(WinSendMsg(HWNDFROMMP(mp2),LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),(MPARAM)NULL)))!=LIT_NONE) {
          /* Query the trackname */
          SHORT1FROMMR(WinSendMsg(HWNDFROMMP(mp2),LM_QUERYITEMTEXT,MPFROM2SHORT((SHORT)a,(SHORT)CCHMAXPATH),&name));
          if(audioHlpIsMp3File(name)) {
            /* It's a mp3. Start the waveinfo helper */
            sprintf(title, "\"%s\"",name); /* Put name between " " */
            launchPMWrapper(title, "1", "pmwvinfo.exe", "Query mp3 format");
            return (MRESULT) 0;
          }
          else
            {
              /* It's a wave. Start the waveinfo helper */
              sprintf(title, "\"%s\"",name); /* Put name between " " */
              launchPMWrapper(title, "0", "pmwvinfo.exe", "Query wave format");
              return (MRESULT) 0;
            }
        }
      }/* end of if(SHORT2FROMMP(mp1)==LN_ENTER) */
      break;
    case WM_CLOSE:
      /* Enable the 'Write'-button of the Audio folder */
      WinDismissDlg(hwnd,0);
      WinDestroyWindow(hwnd);
      break;	
    case WM_HELP:
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr))
        thisPtr->wpDisplayHelp(2100,AFHELPLIBRARY);
      return (MRESULT) 0;
    case WM_COMMAND:	
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_CANCEL:
          thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
          thisPtr->cwEnableWriteControls(TRUE);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        case IDPB_OK:
          /*******************************************************************/
          /* We start writing                                                */
          /*******************************************************************/
          if(!pvAudioSharedMem) {
            if(DosAllocSharedMem(&pvAudioSharedMem,"\\SHAREMEM\\AUDIOCMDLINE",AUDIOSHAREDMEM_SIZE,PAG_READ|PAG_WRITE|PAG_COMMIT)) {
              /*
                name:   "Can't allocate shared memory. There's probably already a creation process running."
                Title: "Audio-CD-Creator"                       
                */             
              messageBox( name, IDSTR_ALLOCSHAREDMEMERROR , sizeof(name),
                          title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                          hAudioResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
              WinPostMsg(hwnd,WM_CLOSE,0,0);
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
              messageBox( name, IDSTR_ALLOCSHAREDMEMERROR , sizeof(name),
                          title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                          hAudioResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);

              WinPostMsg(hwnd,WM_CLOSE,0,0);
              break;
            }
          /* Set memory to zero */
          memset(pvAudioSharedMem,0,AUDIOSHAREDMEM_SIZE);
          text= (char*)pvAudioSharedMem; 
          thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
          if(!somIsObj(thisPtr)) {
            DosFreeMem(pvAudioSharedMem);
            pvAudioSharedMem=NULL;
            WinPostMsg(hwnd,WM_CLOSE,0,0);
            break;
          }
          ulFlags=thisPtr->cwQueryWriteFlags();


          /* Put cdrecord path in the shared mem if DAO mode */
          /* TAO (writeaud.exe) currently does not need it */ 

          if(ulFlags&IDWF_DAO) {
            int iTempSpeed;
            /* Build DAO data */
            WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_QUERYVALUE,MPFROMP(&iTempSpeed),
                       MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));

            /* Query the device string */
            WinQueryWindowText(WinWindowFromID(hwnd, IDDD_DEVICESELECTION), sizeof(name), name);
            /* text: shared mem */
            strcat(text,buildAudioWriteParamDAO(thisPtr, name, iTempSpeed));/* Add audio params for DAO */
            strncat(text,"\"",strlen("\""));
            /* Query the full path of our folder*/
            ulBufferSize=sizeof(name);
            thisPtr->wpQueryRealName(name,&ulBufferSize,TRUE);
            strcat(text,name);/* Name of this audio folder */
            strcat(text,"\\default.toc\"");/* add toc file name*/
            buildTocFile( thisPtr, thisPtr->hwndWriteLB);
          }
          else{
            int iTempSpeed;
            /* TAO with CDRecord/2 */
            /* Query the device string */
            WinQueryWindowText(WinWindowFromID(hwnd, IDDD_DEVICESELECTION), sizeof(name), name);
      
            WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_QUERYVALUE,MPFROMP(&iTempSpeed),
                       MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));
      
            strcat(text, buildAudioWriteParam(thisPtr, name, iTempSpeed));/* Audio params for TAO */                       
            /* Add the tracknames */
            a=SHORT1FROMMR(WinSendMsg(thisPtr->hwndWriteLB,LM_QUERYITEMCOUNT, NULL, NULL));
            for(b=0;b<a;b++) {
              if(SHORT1FROMMR(WinSendMsg(thisPtr->hwndWriteLB,LM_QUERYITEMTEXT,MPFROM2SHORT((SHORT)b,(SHORT)CCHMAXPATH),&name))) {
                strcat(text," \"");
                strcat(text,name);
                strcat(text,"\"");
              }
            }/* for() */
          } /* else{  TAO with CDRecord/2 */

          /* Add number of audio files, total size of all tracks and device to parameters so the helper
             prog has not to query it */          
          WinQueryWindowText(WinWindowFromID(hwnd, IDDD_DEVICESELECTION), sizeof(name), name);
          if((text=strchr(name, ' '))!=NULL)
            *text=0;
          sprintf(anzahl,"%d %d %s",a,thisPtr->ulTrackSize, name);          

          /* Query the full path of our folder*/
          ulBufferSize=sizeof(name);
          thisPtr->wpQueryRealName(name,&ulBufferSize,TRUE);

          /* Check if CDRecord path is valid */
          /* We need cdrecord to query the free CD space */
          if(checkFileExists(chrCDRecord)) {
            if(ulFlags&IDWF_DAO) { 
              /* Check if cdrdao path is valid */
              if(checkFileExists(chrCdrdaoPath))
                rc=launchPMWrapper(anzahl, name,"pmauddao.exe","Writing audio tracks (DAO)");
              else {
                /* The path to cdrdao isn't valid */
                mboxShowMissingProgMsgBox(hwnd, "cdrdao/2", hAudioResource, TRUE);
                /* Show the cdrecord setup help panel */
                HlpSendCommandToObject("<CWCREATOR_SETTINGS>", SETUP_DISPLAYHELPPANEL"="IDHLP_CDRDAO1SETUPSTR);

                rc=-1;/* Mark an error */
              }/* else if(checkFileExists(chrCdrdaoPath)) */
            }/* if(ulFlags&IDWF_DAO) */
            else
              /* We're writing with CDRecord/2 */              
              rc=launchPMWrapper(anzahl,name,"pmwrtaud.exe","Writing audio tracks (TAO)");
          }/* else if() */
          else {
            /* The path to CDRecord/2 isn't valid */
            mboxShowMissingProgMsgBox(hwnd, "CDRecord/2", hAudioResource, TRUE);
            /* Show the cdrecord setup help panel */
            HlpSendCommandToObject("<CWCREATOR_SETTINGS>", SETUP_DISPLAYHELPPANEL"="IDHLP_CDRECORDSETUPSTR);
            rc=-1;/* Mark an error */
          }/* else if(checkFileExists(chrCDRecord)) */
          if(rc) {
            DosFreeMem(pvAudioSharedMem);
            pvAudioSharedMem=NULL;
          }
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        default:
          break;
        }
      return (MRESULT) TRUE;
    default:
      return WinDefDlgProc(hwnd, msg, mp1, mp2);
    }
  return (MRESULT) FALSE;
}






