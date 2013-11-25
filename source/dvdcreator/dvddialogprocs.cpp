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

extern PFNWP pfnwpDataGenericFrame; /* see dataoveriddenwpmethods.cpp */
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




/************************************************************
 *                                                          *
 * This procedure calculates the positions for the custom   *
 * frame controls.                                          *
 * PM uses the pos to calculate the area which needs        *
 * repainting and the pos of each control. Using this code  *
 * prevents flickering.                                     *
 *                                                          *
 ************************************************************/
static MRESULT handleCalcValidRects(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
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




/********************************************************/ 
/*                                                      */
/*  This procedure handles the main frame extension     */ 
/*                                                      */
/********************************************************/ 
MRESULT EXPENTRY dvdMainDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
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
      if(WinIsWindow(WinQueryAnchorBlock(HWND_DESKTOP), WinWindowFromID(hwnd, IDST_DVDTEXT)))
        oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_DVDTEXT), staticTextProc);

      /* subclass checkbox and find mnemonic */
      setupCheckBoxControl(hwnd , IDCB_DUMMY);
      /* subclass radio buttons */
      //      setupRadioButtonControl(hwnd, IDRB_IMAGEONLY);
      setupRadioButtonControl(hwnd, IDRB_ONTHEFLY);
      //    setupRadioButtonControl(hwnd, IDRB_WRITEIMAGE);
      /* Subclass groupbox */
      setupGroupBoxControl(  hwnd, IDGB_DATADIALOG);

      WinCheckButton(hwnd,IDCB_DUMMY,((CWDataFolder*)(PVOIDFROMMP(mp2)))->sDummy);
      ulFlags=((CWDataFolder*)PVOIDFROMMP(mp2))->cwQueryCreateFlags();

      // WinSendMsg(WinWindowFromID(hwnd,IDEF_IMAGENAME),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      // WinSetWindowText(WinWindowFromID(hwnd,IDEF_IMAGENAME),thisPtr->chrImageName);

      thisPtr=(CWDataFolder*)PVOIDFROMMP(mp2);
#if 0
      //Only on the fly for now 
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

      thisPtr->cwShowImageName(FALSE);
      WinEnableWindow(WinWindowFromID(hwnd,IDEF_IMAGENAME) ,FALSE);
      WinEnableWindow(WinWindowFromID(hwnd,IDPB_IMAGEBROWSE) ,FALSE);
      WinEnableWindow(WinWindowFromID(hwnd,IDST_IMAGENAME) ,FALSE);
#endif
      WinCheckButton(hwnd,IDRB_ONTHEFLY,1);
      
      WinShowWindow(WinWindowFromID(hwnd,IDPB_CREATE),FALSE);
      WinShowWindow(WinWindowFromID(hwnd,IDPB_WRITECD),TRUE);
      WinShowWindow(WinWindowFromID(hwnd,IDCB_DUMMY),TRUE);
      
      // WinShowWindow(WinWindowFromID(hwnd,IDRB_IMAGEONLY),FALSE);
      // WinShowWindow(WinWindowFromID(hwnd,IDRB_WRITEIMAGE),FALSE);
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
#if 0
        if(WinQueryButtonCheckstate(hwnd,IDRB_IMAGEONLY)&1)
          ulFlags|=IDCR_IMAGEONLY;
        if(WinQueryButtonCheckstate(hwnd,IDRB_WRITEIMAGE)&1)
          ulFlags|=IDCR_WRITEIMAGE;
        if(WinQueryButtonCheckstate(hwnd,IDRB_ONTHEFLY)&1)
          ulFlags|=IDCR_ONTHEFLY;
        thisPtr->cwSetCreateFlags( ulFlags,0xF);
        WinQueryWindowText( WinWindowFromID(hwnd,IDEF_IMAGENAME),sizeof(thisPtr->chrImageName),thisPtr->chrImageName);
#endif
        ulFlags|=IDCR_ONTHEFLY;
        thisPtr->cwSetCreateFlags( ulFlags,0xF);
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
            sprintf(fd.szFullFile,"%s","*.raw");
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
            WinQueryWindowText( WinWindowFromID(hwnd,IDEF_IMAGENAME),
                                sizeof(thisPtr->chrImageName), thisPtr->chrImageName);
            thisPtr->cwCreateImage();
            break;
          case IDPB_WRITECD:
            /* User pressed the WRITE button */
#if 0
            WinQueryWindowText( WinWindowFromID(hwnd, IDEF_IMAGENAME),
                                sizeof(thisPtr->chrImageName), thisPtr->chrImageName);      

            if(WinQueryButtonCheckstate(hwnd,IDRB_WRITEIMAGE)&1)
              thisPtr->cwWriteImage(); /* Create an image */
            else 
#endif
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









