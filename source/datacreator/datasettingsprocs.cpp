/*
 * Settingsdialog procedures for the Data-CD-Creator class.
 *
 * This file is (C) Chris Wohlgemuth 1999-2001
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

//#include "menufldr.h"
#include "menufolder.hh"

#pragma SOMAsDefault(off)
#include <sys\types.h>
#include <sys\stat.h>

#pragma SOMAsDefault(pop)

#include <stdio.h>
#include <stdlib.h>

extern char chrInstallDir[CCHMAXPATH];
#if 0
char chosenMultiSessionCD[3];
#endif

BOOL bMultiSessionDone=FALSE;
extern HMODULE hDataResource;

ULONG cwQueryOSRelease();
ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);
void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
BOOL cwMoveNotebookButtonsWarp4(HWND hwndDlg, USHORT usID, USHORT usDelta);

/* Local */
static void enableCDTypeCntrls(HWND hwnd, ULONG ulFlags);
MRESULT EXPENTRY fileNameOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY cdTypeOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY authorOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;


/* This procedure handles the filename options page */ 
MRESULT EXPENTRY fileNameOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWDataFolder* thisPtr;
  ULONG ulFlags;

  switch(msg)
    {
    case WM_INITDLG :	
      WinSetWindowULong(hwnd, QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.

      thisPtr=(CWDataFolder*)PVOIDFROMMP(mp2);
      if(!somIsObj(thisPtr))
        return (MRESULT) TRUE;
      if(thisPtr->cwQueryMkisofsFlags()&IDMK_ALLOW32CHARS) {
        WinCheckButton(hwnd,IDRB_32CHARNAMES,1);
        WinCheckButton(hwnd,IDRB_DOSNAMES,0);
      }
      else {
        WinCheckButton(hwnd,IDRB_32CHARNAMES,0);
        WinCheckButton(hwnd,IDRB_DOSNAMES,1);
      }

      /* TRANS.TBL files */
      if(thisPtr->cwQueryMkisofsFlags() & IDMK_TRANSTABLE)
        WinCheckButton(hwnd,IDCB_TRANSTABLE,1);
      else
        WinCheckButton(hwnd,IDCB_TRANSTABLE,0);

      if(((CWDataFolder*)PVOIDFROMMP(mp2))->cwQueryMkisofsFlags()&IDMK_JOLIET)
        WinCheckButton(hwnd,IDCB_JOLIET,1);

      WinShowWindow(WinWindowFromID(hwnd,IDCB_DEEPDIRECTORIES),FALSE);
      
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_FILENAMEHELP, 20);
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

      /* Let the WPS save the new instance data */
          //      thisPtr=(CWDataFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_FILENAMEUNDO),QWL_USER);
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)) {
        ulFlags=0;


        if(WinQueryButtonCheckstate(hwnd,IDCB_JOLIET) &1 )
          ulFlags|=IDMK_JOLIET;
        if(WinQueryButtonCheckstate(hwnd,IDRB_32CHARNAMES) &1 )
          ulFlags|=IDMK_ALLOW32CHARS;
        /* TRANS.TBL files */
        if(WinQueryButtonCheckstate(hwnd,IDCB_TRANSTABLE) &1 )
          ulFlags|=IDMK_TRANSTABLE;

        thisPtr->cwSetMkisofsFlags(ulFlags,
                                   IDMK_ALLOW32CHARS|IDMK_JOLIET| IDMK_TRANSTABLE);
        thisPtr->wpSaveImmediate();
      }
      /* Setup is done */   
      return (MRESULT) TRUE;
    case WM_COMMAND:	
    switch(SHORT1FROMMP(mp1))
      {
      case IDPB_FILENAMEUNDO:
        /* User pressed the UNDO button */
        //thisPtr=(CWDataFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_FILENAMEUNDO),QWL_USER);
        thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
        if(somIsObj(thisPtr)) {
          ulFlags=thisPtr->cwQueryMkisofsFlags();
          if(ulFlags&IDMK_ALLOW32CHARS) {
            WinCheckButton(hwnd,IDRB_32CHARNAMES,1);
            WinCheckButton(hwnd,IDRB_DOSNAMES,0);
          }
          else {
            WinCheckButton(hwnd,IDRB_32CHARNAMES,0);
            WinCheckButton(hwnd,IDRB_DOSNAMES,1);
          }

          if(ulFlags&IDMK_TRANSTABLE)
            WinCheckButton(hwnd,IDCB_TRANSTABLE,1);
          else
            WinCheckButton(hwnd,IDCB_TRANSTABLE,0);

          if(ulFlags&IDMK_JOLIET)
            WinCheckButton(hwnd,IDCB_JOLIET,1);
          else
            WinCheckButton(hwnd,IDCB_JOLIET,0);
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

static void _showTrackCntrls(HWND hwnd, BOOL bEnable2)
{
  /* hwnd:    Dialog-HWND
     bEnable2: New enable state */

  WinShowWindow(WinWindowFromID(hwnd,IDGB_TRACKTYPE),bEnable2);
  WinShowWindow(WinWindowFromID(hwnd,IDRB_TRACKDATA),bEnable2);
  WinShowWindow(WinWindowFromID(hwnd,IDRB_TRACKMODE2),bEnable2);
  WinShowWindow(WinWindowFromID(hwnd,IDRB_TRACKXA1),bEnable2);
  WinShowWindow(WinWindowFromID(hwnd,IDRB_TRACKXA2),bEnable2);
  WinShowWindow(WinWindowFromID(hwnd,IDRB_TRACKCDI),bEnable2);
  WinShowWindow(WinWindowFromID(hwnd,IDCB_FIXDISK),bEnable2);
}


static void _showMultiSessionCntrls(HWND hwnd, BOOL bEnable)
{
  /* hwnd:    Dialog-HWND
     bEnable: New enable state */

  WinShowWindow(WinWindowFromID(hwnd,IDGB_MERGESESSION),bEnable);//Groupbox
  WinShowWindow(WinWindowFromID(hwnd,IDST_MERGETEXT),bEnable);//Text

}

static void _showSingleSessionCntrls(HWND hwnd, BOOL bEnable)
{
  /* hwnd:    Dialog-HWND
     bEnable: New enable state */

  WinShowWindow(WinWindowFromID(hwnd,IDGB_MERGESESSION),bEnable);//Groupbox
  WinShowWindow(WinWindowFromID(hwnd,IDST_SINGLESESSION),bEnable);//Text

}

static void _showBootCDCntrls(HWND hwnd, BOOL bEnable)
{
  /* hwnd:    Dialog-HWND
     bEnable: New enable state */

  WinShowWindow(WinWindowFromID(hwnd,IDGB_BOOTCD),bEnable);
  WinShowWindow(WinWindowFromID(hwnd,IDST_BOOTIMAGE),bEnable);
  WinShowWindow(WinWindowFromID(hwnd,IDST_BOOTCATALOG),bEnable);
  WinShowWindow(WinWindowFromID(hwnd,IDPB_BOOTCDCONFIGURE),bEnable);
}

/*
  This function enables the controls on the CD-type page
  according to the single-/multisession choice.
 */
static void enableCDTypeCntrls(HWND hwnd, ULONG ulFlags)
{

  if(ulFlags&IDCDT_MULTISESSION) {
    /* Multisession selected */
    _showTrackCntrls(hwnd, FALSE);
    _showBootCDCntrls(hwnd, FALSE);
    _showSingleSessionCntrls(hwnd, FALSE);
    /* Show multisession controls */
    _showMultiSessionCntrls(hwnd, TRUE);
    WinCheckButton(hwnd,IDRB_MULTISESSION,1);
    //thisPtr->cwEnableMultiSessionCntrls(hwnd,TRUE);
  }
  else 
    if(ulFlags&IDCDT_BOOTCD) {
      /* Boot-CD selected */
      WinCheckButton(hwnd,IDRB_BOOTCD,1);
      _showTrackCntrls(hwnd, FALSE);
      _showMultiSessionCntrls(hwnd, FALSE);
      _showSingleSessionCntrls(hwnd, FALSE);
      _showBootCDCntrls(hwnd, TRUE);
    }
    else 
      if(ulFlags&IDCDT_USERDEFINED) {// Userdefined selected. Disable the multisession stuff
        WinCheckButton(hwnd,IDRB_USERDEFINED,1);
        _showBootCDCntrls(hwnd, FALSE);
        _showMultiSessionCntrls(hwnd, FALSE);
        _showSingleSessionCntrls(hwnd, FALSE);
        /* Show tracktype controls */
        _showTrackCntrls(hwnd, TRUE);
      }
      else{// singlesession selected. Disable the multisession stuff
        WinCheckButton(hwnd,IDRB_SINGLESESSION,1);
        _showBootCDCntrls(hwnd, FALSE);
        _showMultiSessionCntrls(hwnd, FALSE);
        /* Hide tracktype controls */
        _showTrackCntrls(hwnd, FALSE);
        _showSingleSessionCntrls(hwnd, TRUE);
      }

  /* Check tracktype button */
  if(ulFlags&IDCDT_TRACKMODE2) 
    WinCheckButton(hwnd,IDRB_TRACKMODE2,1);
  else if(ulFlags&IDCDT_TRACKXA1) 
    WinCheckButton(hwnd,IDRB_TRACKXA1,1);
  else if(ulFlags&IDCDT_TRACKXA2) 
    WinCheckButton(hwnd,IDRB_TRACKXA2,1);
  else if(ulFlags&IDCDT_TRACKCDI) 
    WinCheckButton(hwnd,IDRB_TRACKCDI,1);  
  else
    WinCheckButton(hwnd,IDRB_TRACKDATA,1); /* Default */
  /* Fixdisk checkbutton */
  if(ulFlags&IDCDT_FIXDISK) 
    WinCheckButton(hwnd,IDCB_FIXDISK,1);
  else
    WinCheckButton(hwnd,IDCB_FIXDISK,0);

}

static void enableArchiveCntrls(HWND hwnd, BOOL bEnable)
{

  /* Use archive bit */
  WinEnableWindow(WinWindowFromID(hwnd,IDCB_USEARCHIVEBIT),bEnable);

  /* Reset archive bit */
  WinEnableWindow(WinWindowFromID(hwnd, IDCB_RESETARCHIVEBIT),bEnable);

  /* Reset now push button */
  WinEnableWindow(WinWindowFromID(hwnd, IDPB_RESETNOW),bEnable);

}

BOOL checkBootImage(char * chrName)
{
  struct stat statBuf;

  /* Check file path */
  if(stat(chrName , &statBuf)==-1)
    return FALSE;

  if(statBuf.st_size != 1474560 && statBuf.st_size != 1474560*2)
    return FALSE;

  return TRUE;
}

static MRESULT EXPENTRY _bootCDOptionsDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  CWDataFolder* thisPtr;
  FILEDLG fd = { 0 };
  /* char text[CCHMAXPATH];*/
  char title[CCHMAXPATH];
  ULONG rc;
  THISPTR *tpPtr;

  switch(msg)
    {
    case WM_INITDLG :	
      tpPtr=(THISPTR*)PVOIDFROMMP(mp2);
      thisPtr=(CWDataFolder*)tpPtr->thisPtr;

      /*    sprintf(text,"tpPtr: %x thisPtr: %x",tpPtr,thisPtr);
            WinMessageBox(  HWND_DESKTOP, hwnd,
            text,"2", 0UL, MB_OK | MB_ERROR |MB_MOVEABLE); */

      WinSetWindowULong(WinWindowFromID(hwnd,IDPB_OK),
                        QWL_USER,(ULONG)thisPtr);//Save object ptr.
              
      if(somIsObj(thisPtr)) {        
        WinSetWindowText( WinWindowFromID(hwnd,IDEF_BOOTIMAGE), thisPtr->chrBootImage );
        WinSetWindowText( WinWindowFromID(hwnd,IDEF_BOOTCATALOG), thisPtr->chrBootCatalog );
      }
      break;
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_OK:
          /* User pressed the OK button */
          thisPtr=(CWDataFolder*)WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_OK),QWL_USER);
          if(somIsObj(thisPtr)) {
            WinQueryWindowText(WinWindowFromID(hwnd,IDEF_BOOTIMAGE),sizeof(thisPtr->chrBootImage),thisPtr->chrBootImage);
            WinQueryWindowText(WinWindowFromID(hwnd,IDEF_BOOTCATALOG),sizeof(thisPtr->chrBootCatalog),thisPtr->chrBootCatalog);
          }
          WinDismissDlg(hwnd,DID_OK);
          return (MRESULT) FALSE;          
        case IDPB_CANCEL:
          WinDismissDlg(hwnd,0);
          return FALSE;
        case IDPB_BROWSE:
          /* User pressed the browse button */
          fd.cbSize = sizeof( fd );
          /* It's an centered 'Open'-dialog */
          fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
          /* Title: "Select boot image" */
          getMessage(title,IDSTR_FDLGSEARCHBOOTIMGTITLE,sizeof(title), hDataResource,hwnd);
          /* Set the title of the file dialog */
          fd.pszTitle = title;
          /* Show all files */
          sprintf(fd.szFullFile,"%s","*.*");
          if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
            {
              /* WinFileDlg failed */
              break;
            }
          if( fd.lReturn == DID_OK )
            {
              WinSetWindowText( WinWindowFromID(hwnd,IDEF_BOOTIMAGE), fd.szFullFile );
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

MRESULT EXPENTRY cdTypeOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  CWDataFolder* thisPtr;
  ULONG ulFlags;
  LONG lSpinValue;
  static HWND hwndStatus;
  int a;
  char chrCD[4];
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  char text2[40];

  ULONG rc;
  THISPTR thisPtrStruct;

  switch(msg)
    {
    case WM_INITDLG :	
      WinSetWindowULong(hwnd, QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.

      thisPtr=(CWDataFolder*)PVOIDFROMMP(mp2);
      if(somIsObj(thisPtr)) {
        ulFlags=((CWDataFolder*)PVOIDFROMMP(mp2))->cwQueryCDTypeFlags();
        // enableCDTypeCntrls(hwnd,(CWDataFolder*)PVOIDFROMMP(mp2));
        enableCDTypeCntrls(hwnd,ulFlags);

        /*        if(ulFlags&IDCDT_FIXDISK) 
          WinCheckButton(hwnd,IDCB_FIXDISK,1);*/

        getMessage(title,IDSTRD_BOOTIMAGE,sizeof(title), hDataResource,hwnd);
        getMessage(text2,IDSTRD_BOOTIMAGENAME,sizeof(text2), hDataResource,hwnd);
        sprintf(text,title,text2);
        WinSetWindowText( WinWindowFromID(hwnd,IDST_BOOTIMAGE), text );
        getMessage(title,IDSTRD_BOOTCATALOG,sizeof(title), hDataResource,hwnd);
        getMessage(text2,IDSTRD_BOOTCATALOGNAME,sizeof(text2), hDataResource,hwnd);
        sprintf(text,title,text2);
        WinSetWindowText( WinWindowFromID(hwnd,IDST_BOOTCATALOG), text );
      }

      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_CDTYPEHELP, 20);
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
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* Let the WPS save the new instance data */
      //      thisPtr=(CWDataFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDTYPEUNDO),QWL_USER);
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd, QWL_USER);
      if(somIsObj(thisPtr)) {
        ulFlags=0;
        if(WinQueryButtonCheckstate(hwnd,IDRB_MULTISESSION)&1) {
          ulFlags|=IDCDT_MULTISESSION;
        }
        if(WinQueryButtonCheckstate(hwnd,IDRB_USERDEFINED)&1)
          ulFlags|=IDCDT_USERDEFINED;

        if(WinQueryButtonCheckstate(hwnd,IDRB_BOOTCD)&1)
          ulFlags|=IDCDT_BOOTCD;
        
        if(WinQueryButtonCheckstate(hwnd,IDRB_TRACKDATA)&1)
          ulFlags|=IDCDT_TRACKDATA;
        else
          if(WinQueryButtonCheckstate(hwnd,IDRB_TRACKMODE2)&1)
            ulFlags|=IDCDT_TRACKMODE2;
          else
            if(WinQueryButtonCheckstate(hwnd,IDRB_TRACKXA1)&1)
              ulFlags|=IDCDT_TRACKXA1;
            else
              if(WinQueryButtonCheckstate(hwnd,IDRB_TRACKXA2)&1)
                ulFlags|=IDCDT_TRACKXA2;
              else
                if(WinQueryButtonCheckstate(hwnd,IDRB_TRACKCDI)&1)
                  ulFlags|=IDCDT_TRACKCDI;
        
        if(WinQueryButtonCheckstate(hwnd,IDCB_FIXDISK)&1)
          ulFlags|=IDCDT_FIXDISK;
        thisPtr->cwSetCDTypeFlags(ulFlags,CDT_ALLFLAGS);
        thisPtr->wpSaveImmediate();
      }
      /* Setup is done */   
      bMultiSessionDone=TRUE;
      return (MRESULT) FALSE;
    case WM_CONTROL:
      //      thisPtr=(CWDataFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDTYPEUNDO),QWL_USER);
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(!somIsObj(thisPtr)) return (MRESULT) TRUE;
      ulFlags=thisPtr->cwQueryCDTypeFlags();/* Get current flags */
      ulFlags&=(IDCDT_ALLTRACKTYPES|IDCDT_FIXDISK);/* Keep the tracktype information */
      switch(SHORT1FROMMP(mp1))
        {
        case IDRB_SINGLESESSION:
          enableCDTypeCntrls(hwnd, ulFlags);
          break;
        case IDRB_MULTISESSION:
          ulFlags|=IDCDT_MULTISESSION;
          enableCDTypeCntrls(hwnd, ulFlags);
          break;
        case IDRB_BOOTCD:
          ulFlags|=IDCDT_BOOTCD;
          enableCDTypeCntrls(hwnd, ulFlags);
          break;
        case IDRB_USERDEFINED:
          ulFlags|=IDCDT_USERDEFINED;
          enableCDTypeCntrls(hwnd, ulFlags);
          break;
#if 0
        case IDRB_SINGLESESSION:
          _showMultiSessionCntrls(hwnd,FALSE);
          _showBootCDCntrls(hwnd, FALSE);
          _showTrackCntrls(hwnd, FALSE);
          _showSingleSessionCntrls(hwnd,TRUE);
          break;
        case IDRB_MULTISESSION:
          _showTrackCntrls(hwnd, FALSE);
          _showBootCDCntrls(hwnd, FALSE);
          _showMultiSessionCntrls(hwnd,TRUE);
          thisPtr->cwEnableMultiSessionCntrls(hwnd,FALSE);
          break;
        case IDRB_BOOTCD:
          _showTrackCntrls(hwnd, FALSE);
          _showMultiSessionCntrls(hwnd,FALSE);
          _showBootCDCntrls(hwnd, TRUE);
          break;
        case IDRB_USERDEFINED:
          _showMultiSessionCntrls(hwnd,FALSE);
          _showBootCDCntrls(hwnd, FALSE);
          _showTrackCntrls(hwnd, TRUE);
          break;
#endif
        default:
          break;
        }
      break;
    case WM_COMMAND:	
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_BOOTCDCONFIGURE:
          //thisPtr=(CWDataFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDTYPEUNDO),QWL_USER);
          thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
          if(!somIsObj(thisPtr)) return (MRESULT) TRUE;

          thisPtrStruct.usSize=sizeof(thisPtrStruct);
          thisPtrStruct.thisPtr=thisPtr;
          if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, _bootCDOptionsDialogProc, hDataResource, 
                         IDDLG_BOOTCDOPTIONS, &thisPtrStruct) == DID_OK )
            {
              getMessage(title,IDSTRD_BOOTIMAGE,sizeof(title), hDataResource,hwnd);
              sprintf(text,title,thisPtr->chrBootImage);
              WinSetWindowText( WinWindowFromID(hwnd,IDST_BOOTIMAGE), text );
              
              getMessage(title,IDSTRD_BOOTCATALOG,sizeof(title), hDataResource,hwnd);
              sprintf(text,title,thisPtr->chrBootCatalog);
              WinSetWindowText( WinWindowFromID(hwnd,IDST_BOOTCATALOG), text );
            }
          break;
        case IDPB_CDTYPEUNDO:
          // thisPtr=(CWDataFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDTYPEUNDO),QWL_USER);
          thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
          /* User pressed the UNDO button */
          if(somIsObj(thisPtr)) {
            //ulFlags=thisPtr->cwQueryCDTypeFlags();
            /* Enable all multisesson controls */
            //  thisPtr->cwEnableMultiSessionCntrls(hwnd,TRUE);
            //            enableCDTypeCntrls(hwnd,thisPtr);
            enableCDTypeCntrls(hwnd, thisPtr->cwQueryCDTypeFlags());
            /*            if(ulFlags&IDCDT_FIXDISK) 
                          WinCheckButton(hwnd,IDCB_FIXDISK,1);
                          else
                          WinCheckButton(hwnd,IDCB_FIXDISK,0);*/
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


/* This procedure handles the author settings page */ 
MRESULT EXPENTRY authorOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWDataFolder* thisPtr;

  switch(msg)
    {
    case WM_INITDLG :	
      WinSetWindowULong(hwnd, QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      thisPtr=(CWDataFolder*)PVOIDFROMMP(mp2);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_APPLICATION),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)128),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_APPLICATION),thisPtr->chrApplication);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_PUBLISHER),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)128),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_PUBLISHER),thisPtr->chrPublisher);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_PREPARER),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)128),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_PREPARER),thisPtr->chrPreparer);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_VOLUMENAME),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)32),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_VOLUMENAME),thisPtr->chrVolumeName);

      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_AUTHORHELP, 20);
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
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* Let the WPS save the new instance data */
      //      thisPtr=(CWDataFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_AUTHORUNDO),QWL_USER);
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)) {
        WinQueryWindowText( WinWindowFromID(hwnd,IDEF_APPLICATION),sizeof(thisPtr->chrApplication),thisPtr->chrApplication);
        WinQueryWindowText( WinWindowFromID(hwnd,IDEF_PUBLISHER),sizeof(thisPtr->chrPublisher),thisPtr->chrPublisher);
        WinQueryWindowText( WinWindowFromID(hwnd,IDEF_PREPARER),sizeof(thisPtr->chrPreparer),thisPtr->chrPreparer);
        WinQueryWindowText( WinWindowFromID(hwnd,IDEF_VOLUMENAME),sizeof(thisPtr->chrVolumeName),thisPtr->chrVolumeName);
        thisPtr->wpSaveDeferred();
      }
      /* Setup is done */
      return (MRESULT) TRUE;
    case WM_COMMAND:	
    switch(SHORT1FROMMP(mp1))
      {
      case IDPB_AUTHORUNDO:
        /* User pressed the UNDO button */
        //   thisPtr=(CWDataFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_AUTHORUNDO),QWL_USER);
        thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
        if(somIsObj(thisPtr)) {
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_APPLICATION),thisPtr->chrApplication);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_PUBLISHER),thisPtr->chrPublisher);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_PREPARER),thisPtr->chrPreparer);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_VOLUMENAME),thisPtr->chrVolumeName);
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


MRESULT EXPENTRY specialOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  CWDataFolder* thisPtr;
  ULONG ulFlags;
  LONG lSpinValue;
  static HWND hwndStatus;
  int a;
  char chrCD[4];
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  char text2[40];

  ULONG rc;
  THISPTR thisPtrStruct;

  switch(msg)
    {
    case WM_INITDLG :	
      WinSetWindowULong(hwnd, QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      thisPtr=(CWDataFolder*)PVOIDFROMMP(mp2);
      if(somIsObj(thisPtr)) {
        ulFlags=((CWDataFolder*)PVOIDFROMMP(mp2))->cwQueryMkisofsFlags();
        
        if(ulFlags & IDMK_SHADOWSINROOTONLY) { 
          WinCheckButton(hwnd,IDRB_SHADOWSINROOTONLY,1);
          enableArchiveCntrls(hwnd, FALSE);
        }
        else {
          WinCheckButton(hwnd,IDRB_FOLLOWALLSHADOWS,1);
          enableArchiveCntrls(hwnd, TRUE);
        }
        /* Use archive bit */
        if(ulFlags & IDMK_USEARCHIVEBIT) 
          WinCheckButton(hwnd, IDCB_USEARCHIVEBIT, 1);
        else
          WinCheckButton(hwnd, IDCB_USEARCHIVEBIT, 0);
        /* Reset archive bit */
        if(ulFlags & IDMK_RESETARCHIVEBIT) 
          WinCheckButton(hwnd, IDCB_RESETARCHIVEBIT,1);
        else
          WinCheckButton(hwnd, IDCB_RESETARCHIVEBIT, 0);
      }
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_SPECIALHELP, 20);
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
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* Let the WPS save the new instance data */
      //      thisPtr=(CWDataFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_SPECIALUNDO),QWL_USER);
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(somIsObj(thisPtr)) {
        ulFlags=thisPtr->cwQueryMkisofsFlags();
        if(WinQueryButtonCheckstate(hwnd,IDRB_SHADOWSINROOTONLY) & 1) {
          ulFlags|=IDMK_SHADOWSINROOTONLY;
        }
        else
          ulFlags&=~IDMK_SHADOWSINROOTONLY;
        /* Use archive bit */
        if(WinQueryButtonCheckstate(hwnd, IDCB_USEARCHIVEBIT) & 1) {
          ulFlags|=IDMK_USEARCHIVEBIT;
        }
        else
          ulFlags&=~IDMK_USEARCHIVEBIT;
        /* Reset archive bit */
        if(WinQueryButtonCheckstate(hwnd, IDCB_RESETARCHIVEBIT) & 1) {
          ulFlags|=IDMK_RESETARCHIVEBIT;
        }
        else
          ulFlags&=~IDMK_RESETARCHIVEBIT;

        thisPtr->cwSetMkisofsFlags(ulFlags,MK_ALLFLAGS);
        thisPtr->wpSaveImmediate();
      }
      /* Setup is done */   
    
      return (MRESULT) FALSE;
    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1))
        {
        case IDRB_SHADOWSINROOTONLY:
          enableArchiveCntrls(hwnd, FALSE);
          break;
        case IDRB_FOLLOWALLSHADOWS:
          enableArchiveCntrls(hwnd, TRUE);          
          break;
        default:
          break;
        }
      break;
    case WM_COMMAND:	
      //thisPtr=(CWDataFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_SPECIALUNDO),QWL_USER);
      thisPtr=(CWDataFolder*) WinQueryWindowULong(hwnd,QWL_USER);
      if(!somIsObj(thisPtr)) 
        return (MRESULT) TRUE;
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_SPECIALUNDO:
          /* User pressed the UNDO button */
          ulFlags=thisPtr->cwQueryMkisofsFlags();
          if(ulFlags & IDMK_SHADOWSINROOTONLY) 
            WinCheckButton(hwnd,IDRB_SHADOWSINROOTONLY,1);
          else
            WinCheckButton(hwnd, IDRB_FOLLOWALLSHADOWS,1);
          /* Use archive bit */
          if(ulFlags & IDMK_USEARCHIVEBIT) 
            WinCheckButton(hwnd, IDCB_USEARCHIVEBIT, 1);
          else
            WinCheckButton(hwnd, IDCB_USEARCHIVEBIT, 0);
          /* Reset archive bit */
          if(ulFlags & IDMK_RESETARCHIVEBIT) 
            WinCheckButton(hwnd, IDCB_RESETARCHIVEBIT,1);
          else
            WinCheckButton(hwnd, IDCB_RESETARCHIVEBIT, 0);          
          break;  
        case IDPB_SPECIALHELP:
          thisPtr->wpDisplayHelp(IDHLP_DATAFOLDERSPECIAL,AFHELPLIBRARY);
          break;
        case IDPB_RESETNOW:
          /* Text: ""
             Title: ""
             */
          rc=messageBox( text, IDSTRD_RESETARCHIVEBITCONFIRM , sizeof(text),
                      title, IDSTRD_RESETARCHIVEBITTITLE, sizeof(title),
                      hDataResource, HWND_DESKTOP, MB_YESNO | MB_ICONQUESTION | MB_MOVEABLE | MB_DEFBUTTON2);          
          if(rc==MBID_YES)
            DosBeep(5000,1000);
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













