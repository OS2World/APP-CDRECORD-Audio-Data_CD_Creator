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

#define TIMERID_DEVICESELECT 1

extern PVOID pvScanbusSharedMem;

extern GLOBALDATA globalData;
extern char chrConfigDir[CCHMAXPATH];

extern char chrCdrdaoPath[CCHMAXPATH];
extern char chrCdrdaoDriver[100];
extern char chrDeviceName[CCHMAXPATH];
extern char chrWriterName[CCHMAXPATH];
extern char chrCdrdaoOptions[CCHMAXPATH];

extern HMODULE hSettingsResource;

extern BOOL checkFileExists(char* chrFileName);
extern BOOL cwMoveNotebookButtonsWarp4(HWND hwndDlg, USHORT usID, USHORT usDelta);
extern BOOL cwEnableNotebookButton(HWND hwndDlg, USHORT usID, BOOL fEnable);
extern BOOL startScanbusHelper(HWND hwnd);
extern void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);

/*

 */
static void showCdrdaoSettings(HWND hwnd, BOOL bShow)
{
  /* Show all settings in expert mode */
  if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE)) {
    WinShowWindow(WinWindowFromID(hwnd, IDST_CDRDAODRIVER), bShow);
    WinShowWindow(WinWindowFromID(hwnd, IDLB_CDRDAODRIVER), bShow);
    WinShowWindow(WinWindowFromID(hwnd, IDST_CDRDAOOPTIONS), bShow);
    WinShowWindow(WinWindowFromID(hwnd, IDEF_CDRDAOOPTIONS), bShow);
    WinShowWindow(WinWindowFromID(hwnd, IDST_CDRDAOKNOWNSETTINGS), !bShow);
    /* Disable Undo when automatic settings are in effect. Otherwise
       this will end in chaos because the Undo may be override the default
       setting for a known writer without the user noticing it. */
    cwEnableNotebookButton(hwnd, IDPB_CDRDAOUNDO, bShow);

    if(checkFileExists(chrCdrdaoPath)) {
      WinShowWindow(WinWindowFromID(hwnd, IDST_CDRDAOPATH), bShow);
      WinShowWindow(WinWindowFromID(hwnd, IDEF_CDRDAOPATH), bShow);
      WinShowWindow(WinWindowFromID(hwnd, IDPB_CDRDAOBROWSE), bShow);
    }
  }
}

static BOOL setKnownCdrdaoDecSettings(HWND hwnd)
{
  HINI hini;
  char text[CCHMAXPATH];
  BOOL rc=FALSE;

  /* Open database with drivers */
  sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
  hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
  if(hini)
    {
      char * chrPtr;
      /* Find beginning of ident string */
      if((chrPtr=strchr(chrDeviceName, '\''))!=NULLHANDLE) {
        PrfQueryProfileString(hini, chrPtr, "drivename", "", text, sizeof(text));
        if(strlen(text)!=0) {
          /* Drive name found e.g Yamaha CRW-2200S */
          /* Insert the writer name */
          WinSetWindowText(WinWindowFromID(hwnd, IDLB_CDRDAODRIVER), text);
          showCdrdaoSettings(hwnd, FALSE);
          rc=TRUE;
        }
        else {
          /* This writer is unknown */
          showCdrdaoSettings(hwnd, TRUE);
        }
      }
      PrfCloseProfile(hini);
    }

  return rc;
}


static BOOL setKnownCdrdaoSettingsCopy(HWND hwnd)
{
  HINI hini;
  char text[CCHMAXPATH];
  BOOL rc=FALSE;

  /* Open database with drivers */
  sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
  hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
  if(hini)
    {
      char * chrPtr;
      char chrDev[100];
      /* Get selected device */
      WinQueryWindowText( WinWindowFromID(hwnd,IDDD_CDRDAODEVICE),
                          sizeof(chrDev), chrDev);
      /* Find beginning of ident string */
      if((chrPtr=strchr(chrDev, '\''))!=NULLHANDLE) {
        PrfQueryProfileString(hini, chrPtr, "drivename", "", text, sizeof(text));
        if(strlen(text)!=0) {
          /* Drive name found e.g Yamaha CRW-2200S */
          /* Insert the writer name */
          WinSetWindowText(WinWindowFromID(hwnd, IDLB_CDRDAODRIVER), text);
          showCdrdaoSettings(hwnd, FALSE);
          rc=TRUE;
        }
        else {
          /* This writer is unknown */
          showCdrdaoSettings(hwnd, TRUE);
        }
      }
      PrfCloseProfile(hini);
    }
  return rc;
}

/***********************************************************************/
/*                                                                     */
/* This procedure handles the cdrdao settings page for cdrdao in       */
/* normal mode. Normal mode is non-expert and default cdrdao (the one  */
/* coming with ADC. If another cdrdao release is selected by the user  */
/* but non-expert mode is also in effect, the proc                     */
/* "settingsCdrdaoOptionDlgProc()" will be used, so path selection is  */
/* possible.                                                           */
/*                                                                     */
/***********************************************************************/
MRESULT EXPENTRY settingsCdrdaoOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  ULONG ulFlags;
  static BOOL fAutomaticallyConfigured; 
 
  switch(msg)
    {
    case WM_INITDLG:
      {
        HINI hini;
        char text[CCHMAXPATH];
        CWCreatorSettings* thisPtr;

        WinSetWindowULong(hwnd, QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
        /* NO path here. */
        WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
        WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),chrCdrdaoOptions);
        /* Set the writer name (not the driver) */
        WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER), chrWriterName);
        /* Open database with drivers */
        sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
        hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
        if(hini)
          {
            char *ptr;
            ULONG ulSize;

            if(PrfQueryProfileSize(hini, NULL,NULL,&ulSize)) { /* Get data size */
              char *pBuffer;

              pBuffer=(char*)malloc(ulSize);
              if(pBuffer) {
                if(PrfQueryProfileData(hini, NULL,NULL, pBuffer, &ulSize))
                  {
                    BOOL bFirst;
                    
                    ptr=pBuffer;
                    if(strlen(chrWriterName)==0)
                      bFirst=TRUE;/* If we have no writer name, use the first one */
                    else
                      bFirst=FALSE;
                    do {
                      /* Get first with a driver information (grabbers don't have this info) */
                      PrfQueryProfileString(hini,ptr,"driver","",text,sizeof(text));
                      if(strlen(text)!=0) {
                        /* Insert the writer name */
                        WinSendMsg(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),LM_INSERTITEM,MPFROMSHORT(LIT_SORTASCENDING),
                                   (MPARAM)ptr);
                        if(bFirst) {
                          bFirst=FALSE;
                          WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),ptr);
                        }
                      }
                      ptr=strchr(ptr,0);
                      if(!ptr)
                        break;
                      ptr++; /* Next database entry */
                    }while(*ptr!=0);
                  }
                free(pBuffer);
              }
            }
            PrfCloseProfile(hini);
          }
        /* Check if we know the given writer and set the options accordingly if not
           in expert mode. */
        if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE))
          fAutomaticallyConfigured=setKnownCdrdaoDecSettings(hwnd);

        /* Move default buttons on Warp 4 */
        cwMoveNotebookButtonsWarp4(hwnd, IDPB_CDRDAOHELP, 20);
        return (MRESULT) TRUE;
        /* This prevents switching the notebook page behind the open folder */
      }
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;
        PSWP pswp=(PSWP)PVOIDFROMMP(mp1);

        if(pswp && pswp->fl & SWP_SHOW)
          {
            /* The notebook page is shown. Maybe the user changed the writer in the 
               meantime so update the settings to the right ones. */
           
            /* Check if we know the given writer and set the options accordingly if not
               in expert mode. */
            if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE))
              fAutomaticallyConfigured=setKnownCdrdaoDecSettings(hwnd);
          }
        
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
      {
        HINI hini;
        char text[CCHMAXPATH];
        CWCreatorSettings* thisPtr;

        /* The notebook closes and gets destroyed */
        /* Set focus to desktop to prevent PM freeze */
        WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);
        
        /* Query the options. */
        WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),sizeof(chrCdrdaoOptions),chrCdrdaoOptions);
        /* Query the cdrdao writer name (not driver) from the entryfield for audio folders */     
        WinQueryWindowText( WinWindowFromID(hwnd,IDLB_CDRDAODRIVER ),sizeof(chrWriterName),chrWriterName);
        /* Now get the driver from that name. */
        sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
        hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
        if(hini)
          {
            /* Get the associated driver */
            PrfQueryProfileString(hini,chrWriterName,"driver","",chrCdrdaoDriver, sizeof(chrCdrdaoDriver));
            PrfCloseProfile(hini);
          }
        thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd, QWL_USER);
        if(somIsObj(thisPtr))
          thisPtr->wpSaveImmediate();
        /* Setup is done */
        return (MRESULT) TRUE;
      }
    case WM_HELP:
      {
        CWCreatorSettings* thisPtr;
        thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd, QWL_USER);
        if(!somIsObj(thisPtr))
          break;
        
        if(fAutomaticallyConfigured)
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_AUTOMATICCONFIG, AFHELPLIBRARY);
        
        /* Switch the right help panel for the control with the focus to the front */
        switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
          {
            /*  */
          case IDEF_CDRDAOOPTIONS:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDRDAOOPTIONS, AFHELPLIBRARY);
            /* */
          default:
            break;
          }
        return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDRDAOSETUP, AFHELPLIBRARY);
      }
    case WM_COMMAND:    
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_CDRDAOUNDO:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),chrWriterName);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),chrCdrdaoOptions);
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
/* This procedure handles the cdrdao settings page for cdrdao when an  */
/* unknown cdrdao release is used. This means, path selection is       */
/* possible for the user.                                              */
/*                                                                     */
/***********************************************************************/
MRESULT EXPENTRY settingsCdrdaoOptionDlgProcExpert(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  ULONG ulFlags;
  static BOOL fAutomaticallyConfigured; 
 
  switch(msg)
    {
    case WM_INITDLG:
      {
        HINI hini;
        char text[CCHMAXPATH];
        CWCreatorSettings* thisPtr;

        WinSetWindowULong(hwnd, QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
        WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRDAOPATH),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
        WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOPATH),chrCdrdaoPath);
        WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
        WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),chrCdrdaoOptions);
        /* Set the writer name (not the driver) */
        WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER), chrWriterName);
        /* Open database with drivers */
        sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
        hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
        if(hini)
          {
            char *ptr;
            ULONG ulSize;

            if(PrfQueryProfileSize(hini, NULL,NULL,&ulSize)) { /* Get data size */
              char *pBuffer;

              pBuffer=(char*)malloc(ulSize);
              if(pBuffer) {
                if(PrfQueryProfileData(hini, NULL,NULL, pBuffer, &ulSize))
                  {
                    BOOL bFirst;
                    
                    ptr=pBuffer;
                    if(strlen(chrWriterName)==0)
                      bFirst=TRUE;/* If we have no writer name, use the first one */
                    else
                      bFirst=FALSE;
                    do {
                      /* Get first with a driver information (grabbers don't have this info) */
                      PrfQueryProfileString(hini,ptr,"driver","",text,sizeof(text));
                      if(strlen(text)!=0) {
                        /* Insert the writer name */
                        WinSendMsg(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),LM_INSERTITEM,MPFROMSHORT(LIT_SORTASCENDING),
                                   (MPARAM)ptr);
                        if(bFirst) {
                          bFirst=FALSE;
                          WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),ptr);
                        }
                      }
                      ptr=strchr(ptr,0);
                      if(!ptr)
                        break;
                      ptr++; /* Next database entry */
                    }while(*ptr!=0);
                  }
                free(pBuffer);
              }
            }
            PrfCloseProfile(hini);
          }
        /* Check if we know the given writer and set the options accordingly if not
           in expert mode. */
        if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE))
          fAutomaticallyConfigured=setKnownCdrdaoDecSettings(hwnd);

        /* Move default buttons on Warp 4 */
        cwMoveNotebookButtonsWarp4(hwnd, IDPB_CDRDAOHELP, 20);
        return (MRESULT) TRUE;
        /* This prevents switching the notebook page behind the open folder */
      }
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;
        PSWP pswp=(PSWP)PVOIDFROMMP(mp1);

        if(pswp && pswp->fl & SWP_SHOW)
          {
            /* The notebook page is shown. Maybe the user changed the writer in the 
               meantime so update the settings to the right ones. */
           
            /* Check if we know the given writer and set the options accordingly if not
               in expert mode. */
            if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE))
              fAutomaticallyConfigured=setKnownCdrdaoDecSettings(hwnd);
          }
        
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
      {
        HINI hini;
        char text[CCHMAXPATH];
        CWCreatorSettings* thisPtr;

        /* The notebook closes and gets destroyed */
        /* Set focus to desktop to prevent PM freeze */
        WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);
        
        /* Query the cdrdao path from the entryfield */     
        WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDRDAOPATH),sizeof(chrCdrdaoPath),chrCdrdaoPath);
        /* Query the options. */
        WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),sizeof(chrCdrdaoOptions),chrCdrdaoOptions);
        /* Query the cdrdao writer name (not driver) from the entryfield for audio folders */     
        WinQueryWindowText( WinWindowFromID(hwnd,IDLB_CDRDAODRIVER ),sizeof(chrWriterName),chrWriterName);
        /* Now get the driver from that name. */
        sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
        hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
        if(hini)
          {
            /* Get the associated driver */
            PrfQueryProfileString(hini,chrWriterName,"driver","",chrCdrdaoDriver, sizeof(chrCdrdaoDriver));
            PrfCloseProfile(hini);
          }
        thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd, QWL_USER);
        if(somIsObj(thisPtr))
          thisPtr->wpSaveImmediate();
        /* Setup is done */
        return (MRESULT) TRUE;
      }
    case WM_HELP:
      {
        CWCreatorSettings* thisPtr;
        thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd, QWL_USER);
        if(!somIsObj(thisPtr))
          break;
        
        if(fAutomaticallyConfigured)
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_AUTOMATICCONFIG, AFHELPLIBRARY);
        
        /* Switch the right help panel for the control with the focus to the front */
        switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
          {
            /*  */
          case IDEF_CDRDAOOPTIONS:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDRDAOOPTIONS, AFHELPLIBRARY);
            /* */
          default:
            break;
          }
        return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDRDAOSETUPEXPERT, AFHELPLIBRARY);
      }
    case WM_COMMAND:    
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_CDRDAOBROWSE:
          {
            char text[CCHMAXPATH];
            FILEDLG fd = { 0 };
            /* User pressed the browse button */
            fd.cbSize = sizeof( fd );
            /* It's an centered 'Open'-dialog */
            fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
            /* Title: "Search CDRecord/2" */
            getMessage(text,IDSTR_FDLGSEARCHCDRDAOTITLE,sizeof(text), hSettingsResource,hwnd);
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
                WinSetWindowText( WinWindowFromID(hwnd,IDEF_CDRDAOPATH), fd.szFullFile );
              }
            break;
          }
        case IDPB_CDRDAOUNDO:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),chrWriterName);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOPATH),chrCdrdaoPath);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),chrCdrdaoOptions);
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
/* This procedure handles the cdrdao settings page for 1:1 copy source */
/*                                                                     */
/***********************************************************************/
MRESULT EXPENTRY settingsCdrdaoOptionDlgProc2(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWCreatorSettings* thisPtr;
  ULONG ulFlags;
  char text[CCHMAXPATH];

  char *ptr;
  HINI hini;
  ULONG ulSize;
  char *pBuffer;
  static  BOOL fScanning;
  static BOOL fAutomaticallyConfigured;

  switch(msg)
    {
    case WM_INITDLG :   

      WinSetWindowULong(WinWindowFromID(hwnd,IDPB_CDRDAOBROWSE),
                        QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)sizeof(globalData.chrCdrdaoOptions2)),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),globalData.chrCdrdaoOptions2);
      /* Set the writer name (not the driver) */
      WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER), globalData.chrWriterName2);
      WinSetWindowText( WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), globalData.chrDeviceName2);
      /* Open database with drivers */
      sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
      hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
      if(hini)
        {
          if(PrfQueryProfileSize(hini, NULL,NULL,&ulSize)) { /* Get data size */
            pBuffer=(char*)malloc(ulSize);
            if(pBuffer) {
              if(PrfQueryProfileData(hini, NULL,NULL, pBuffer, &ulSize))
                {
                  BOOL bFirst;

                  ptr=pBuffer;
                  if(strlen(globalData.chrWriterName2)==0)
                    bFirst=TRUE;/* If we have no writer name, use the first one */
                  else
                    bFirst=FALSE;
                  do {
                    /* Get first entry with a driver information (grabbers don't have this info) */
                    PrfQueryProfileString(hini,ptr,"driver","",text,sizeof(text));
                    if(strlen(text)!=0) {
                      /* Insert the writer name */
                      WinSendMsg(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),LM_INSERTITEM,
                                 MPFROMSHORT(LIT_SORTASCENDING),
                                 (MPARAM)ptr);
                      if(bFirst) {
                        bFirst=FALSE;
                        WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),ptr);
                      }
                    }
                    ptr=strchr(ptr,0);
                    if(!ptr)
                      break;
                    ptr++; /* Next database entry */
                  }while(*ptr!=0);
                }
              free(pBuffer);
            }
          }
          PrfCloseProfile(hini);
        }
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_CDRDAOHELP, 20);

      /* Hide controls while scanning */
      showCdrdaoSettings(hwnd, FALSE);
      WinShowWindow(WinWindowFromID(hwnd, IDST_CDRDAOKNOWNSETTINGS), FALSE);

      /* Now scan the bus for new devices */
      WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDPB_CDRDAOSCAN), MPFROM2SHORT(CMDSRC_PUSHBUTTON, TRUE));

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
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS,
                       MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
    case WM_HELP:
      {
        thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),
                                                         QWL_USER);
        if(!somIsObj(thisPtr))
          break;
        /* Switch the right help panel for the control with the focus to the front */
        switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
          {
            /*  */
          case IDEF_CDRDAOOPTIONS:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDRDAOOPTIONS, AFHELPLIBRARY);
            /* */
          default:
            break;
          }
        /* Default page help */
        break;
      }

    case WM_CONTROL:
      {
        switch(SHORT1FROMMP(mp1))
          {
          case IDDD_CDRDAODEVICE:
            {
              /* The device changed... */
              /* This may also happen when scanbus.exe ended and the default device must be selected. */
              if(SHORT2FROMMP(mp1)==CBN_EFCHANGE) {
                /* Entry field changed */

                /* Check if we know the given writer and set the options accordingly if not
                   in expert mode. */
                /* if fScanning==FALSE the user changed the entry field */
                //                if(!fScanning)
                WinStartTimer( WinQueryAnchorBlock(HWND_DESKTOP), hwnd, TIMERID_DEVICESELECT, 300);
#if 0
                else
                  if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE))
                    fAutomaticallyConfigured=setKnownCdrdaoSettingsCopy(hwnd);
#endif
              }/* CBN_EFCHANGE */
              break;
            }
          default:
            break;
          }
        break;
      }
      return (MRESULT) FALSE;
    case WM_TIMER:
      if(SHORT1FROMMP(mp1)==TIMERID_DEVICESELECT)
        {
          /* This timer is started when a new device is selected. We now check if it's a known
             device by querying the database. */

          WinStopTimer( WinQueryAnchorBlock(HWND_DESKTOP), hwnd, TIMERID_DEVICESELECT);

          /* Check if we know the given writer and set the options accordingly if not
             in expert mode. */
          if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE))
            fAutomaticallyConfigured=setKnownCdrdaoSettingsCopy(hwnd);

          return (MRESULT) 0;
        }
      break;
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* Stop timer if running */
      WinStopTimer( WinQueryAnchorBlock(HWND_DESKTOP), hwnd, TIMERID_DEVICESELECT);

      /* Query the options. */
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),
                          sizeof(globalData.chrCdrdaoOptions2), globalData.chrCdrdaoOptions2);
      /* Query the cdrdao writer (not driver) from the entryfield for audio folders */     
      WinQueryWindowText( WinWindowFromID(hwnd,IDLB_CDRDAODRIVER ),
                          sizeof(globalData.chrWriterName2), globalData.chrWriterName2);
      WinQueryWindowText( WinWindowFromID(hwnd,IDDD_CDRDAODEVICE ),
                          sizeof(globalData.chrDeviceName2), globalData.chrDeviceName2);

      sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
      hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
      if(hini)
        {
          /* Get the associated driver */
          PrfQueryProfileString(hini, globalData.chrWriterName2 ,"driver","",globalData.chrCdrdaoDriver2,
                                sizeof(globalData.chrCdrdaoDriver2));
          PrfCloseProfile(hini);
        }          
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),QWL_USER);
      if(somIsObj(thisPtr))
        thisPtr->wpSaveImmediate();      
      /* Setup is done */
      return (MRESULT) TRUE;
    case WM_COMMAND:    
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_CDRDAOSCAN:
          if(startScanbusHelper(hwnd))
            fScanning=TRUE;
          break;
        case IDPB_CDRDAOUNDO:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),globalData.chrWriterName2);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),globalData.chrCdrdaoOptions2);
          WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), globalData.chrDeviceName2);
          break;
        default:
          break;
        }
      return (MRESULT) TRUE;
    case WM_APPTERMINATENOTIFY:
      switch(LONGFROMMP(mp1)) {
      case ACKEY_SCANBUS:
        {
          SHORT sItem;
          /* scanbus.exe terminated. Set the entryfield now */

          /* First check if our saved device is still installed */
          sItem=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE),
                                        LM_SEARCHSTRING, MPFROM2SHORT(LSS_PREFIX|LSS_CASESENSITIVE, LIT_FIRST),
                                        MPFROMP(globalData.chrDeviceName2)));
          if(sItem!=LIT_ERROR && sItem!=LIT_NONE) {
            /* Device found in list so put name into entry field*/
            WinSetWindowText(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE), globalData.chrDeviceName2);
          }
          else {
            /* Our device no longe is installed */
            char chrDevice[CCHMAXPATH];
            if(SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE), LM_QUERYITEMTEXT,
                                       MPFROM2SHORT(0, sizeof(globalData.chrWriterName2)),
                                       MPFROMP(globalData.chrWriterName2))))
              WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), globalData.chrWriterName2);
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
        if(LONGFROMMP(mp2)==0) {
          /* Delete entries in listbox */
          WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE),"");
          WinSendMsg(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE),LM_DELETEALL,MPFROMLONG(0),MPFROMLONG(0));
        }
        else
          {
            WinSendMsg(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE),LM_INSERTITEM,MPFROMSHORT(LIT_END),
                       mp2);
          }
        break;
      default:
        break;
      }/* switch */
      break; 
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/***********************************************************************/
/*                                                                     */
/* This procedure handles the cdrdao settings page for 1:1 copy source */
/*                                                                     */
/***********************************************************************/
MRESULT EXPENTRY settingsCdrdaoOptionDlgProc2Expert(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWCreatorSettings* thisPtr;
  ULONG ulFlags;
  char text[CCHMAXPATH];

  char *ptr;
  HINI hini;
  ULONG ulSize;
  char *pBuffer;
  static  BOOL fScanning;

  switch(msg)
    {
    case WM_INITDLG :   
      WinSetWindowULong(WinWindowFromID(hwnd,IDPB_CDRDAOBROWSE),
                        QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRDAOPATH),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)sizeof(globalData.chrCdrdaoPath2)),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOPATH),globalData.chrCdrdaoPath2);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)sizeof(globalData.chrCdrdaoOptions2)),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),globalData.chrCdrdaoOptions2);
      /* Set the writer name (not the driver) */
      WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER), globalData.chrWriterName2);
      WinSetWindowText( WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), globalData.chrDeviceName2);
      /* Open database with drivers */
      sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
      hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
      if(hini)
        {
          if(PrfQueryProfileSize(hini, NULL,NULL,&ulSize)) { /* Get data size */
            pBuffer=(char*)malloc(ulSize);
            if(pBuffer) {
              if(PrfQueryProfileData(hini, NULL,NULL, pBuffer, &ulSize))
                {
                  BOOL bFirst;

                  ptr=pBuffer;
                  if(strlen(globalData.chrWriterName2)==0)
                    bFirst=TRUE;/* If we have no writer name, use the first one */
                  else
                    bFirst=FALSE;
                  do {
                    /* Get first entry with a driver information (grabbers don't have this info) */
                    PrfQueryProfileString(hini,ptr,"driver","",text,sizeof(text));
                    if(strlen(text)!=0) {
                      /* Insert the writer name */
                      WinSendMsg(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),LM_INSERTITEM,
                                 MPFROMSHORT(LIT_SORTASCENDING),
                                 (MPARAM)ptr);
                      if(bFirst) {
                        bFirst=FALSE;
                        WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),ptr);
                      }
                    }
                    ptr=strchr(ptr,0);
                    if(!ptr)
                      break;
                    ptr++; /* Next database entry */
                  }while(*ptr!=0);
                }
              free(pBuffer);
            }
          }
          PrfCloseProfile(hini);
        }
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_CDRDAOHELP, 20);

      /* Now scan the bus for new devices */
      WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDPB_CDRDAOSCAN), MPFROM2SHORT(CMDSRC_PUSHBUTTON, TRUE));

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
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS,
                       MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
    case WM_HELP:
      {
        thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),
                                                         QWL_USER);
        if(!somIsObj(thisPtr))
          break;
        /* Switch the right help panel for the control with the focus to the front */
        switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
          {
            /*  */
          case IDEF_CDRDAOOPTIONS:
            return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDRDAOOPTIONS, AFHELPLIBRARY);
            /* */
          default:
            break;
          }
        /* Default page help */
        break;
      }
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* Query the cdrdao path from the entryfield */     
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDRDAOPATH),sizeof(globalData.chrCdrdaoPath2),
                          globalData.chrCdrdaoPath2);
      /* Query the options. */
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),
                          sizeof(globalData.chrCdrdaoOptions2), globalData.chrCdrdaoOptions2);
      /* Query the cdrdao writer (not driver) from the entryfield for audio folders */     
      WinQueryWindowText( WinWindowFromID(hwnd,IDLB_CDRDAODRIVER ),
                          sizeof(globalData.chrWriterName2), globalData.chrWriterName2);
      WinQueryWindowText( WinWindowFromID(hwnd,IDDD_CDRDAODEVICE ),
                          sizeof(globalData.chrDeviceName2), globalData.chrDeviceName2);

      sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
      hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
      if(hini)
        {
          /* Get the associated driver */
          PrfQueryProfileString(hini, globalData.chrWriterName2 ,"driver","",globalData.chrCdrdaoDriver2,
                                sizeof(globalData.chrCdrdaoDriver2));
          PrfCloseProfile(hini);
        }          
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),QWL_USER);
      if(somIsObj(thisPtr))
        thisPtr->wpSaveImmediate();      
      /* Setup is done */
      return (MRESULT) TRUE;
    case WM_COMMAND:    
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_CDRDAOBROWSE:
          {
            FILEDLG fd = { 0 };

            /* User pressed the browse button */
            fd.cbSize = sizeof( fd );
            /* It's an centered 'Open'-dialog */
            fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
            /* Title: "Search CDRecord/2" */
            getMessage(text,IDSTR_FDLGSEARCHCDRDAOTITLE,sizeof(text), hSettingsResource,hwnd);
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
                WinSetWindowText( WinWindowFromID(hwnd,IDEF_CDRDAOPATH), fd.szFullFile );
              }
            break;
          }
        case IDPB_CDRDAOSCAN:
          if(startScanbusHelper(hwnd))
            fScanning=TRUE;
          break;
        case IDPB_CDRDAOUNDO:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),globalData.chrWriterName2);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOPATH),globalData.chrCdrdaoPath2);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),globalData.chrCdrdaoOptions2);
          WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), globalData.chrDeviceName2);
          break;
        default:
          break;
        }
      return (MRESULT) TRUE;
    case WM_APPTERMINATENOTIFY:
      switch(LONGFROMMP(mp1)) {
      case ACKEY_SCANBUS:
        {
          SHORT sItem;
          /* scanbus.exe terminated. Set the entryfield now */

          /* First check if our saved device is still installed */
          sItem=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE),
                                        LM_SEARCHSTRING, MPFROM2SHORT(LSS_PREFIX|LSS_CASESENSITIVE, LIT_FIRST),
                                        MPFROMP(globalData.chrDeviceName2)));
          if(sItem!=LIT_ERROR && sItem!=LIT_NONE) {
            /* Device found in list so put name into entry field*/
            WinSetWindowText(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE), globalData.chrDeviceName2);
          }
          else {
            /* Our device no longe is installed */
            char chrDevice[CCHMAXPATH];
            if(SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE), LM_QUERYITEMTEXT,
                                       MPFROM2SHORT(0, sizeof(globalData.chrWriterName2)),
                                       MPFROMP(globalData.chrWriterName2))))
              WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), globalData.chrWriterName2);
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
        if(LONGFROMMP(mp2)==0) {
          /* Delete entries in listbox */
          WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE),"");
          WinSendMsg(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE),LM_DELETEALL,MPFROMLONG(0),MPFROMLONG(0));
        }
        else
          {
            WinSendMsg(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE),LM_INSERTITEM,MPFROMSHORT(LIT_END),
                       mp2);
          }
        break;
      default:
        break;
      }/* switch */
      break; 
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/***********************************************************************/
/*                                                                     */
/* This procedure handles the cdrdao settings page for 1:1 copy target */
/*                                                                     */
/***********************************************************************/
MRESULT EXPENTRY settingsCdrdaoOptionDlgProc3(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWCreatorSettings* thisPtr;
  ULONG ulFlags;
  char text[CCHMAXPATH];

  char *ptr;
  HINI hini;
  ULONG ulSize;
  char *pBuffer;
  static  BOOL fScanning;
  static BOOL fAutomaticallyConfigured;

  switch(msg)
    {
    case WM_INITDLG :   
      WinSetWindowULong(WinWindowFromID(hwnd,IDPB_CDRDAOBROWSE),
                        QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)sizeof(globalData.chrCdrdaoOptions3)),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),globalData.chrCdrdaoOptions3);
      /* Set the writer name (not the driver) */
      WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER), globalData.chrWriterName3);
      WinSetWindowText( WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), globalData.chrDeviceName3);
      /* Open database with drivers */
      sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
      hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
      if(hini)
        {
          if(PrfQueryProfileSize(hini, NULL,NULL,&ulSize)) { /* Get data size */
            pBuffer=(char*)malloc(ulSize);
            if(pBuffer) {
              if(PrfQueryProfileData(hini, NULL,NULL, pBuffer, &ulSize))
                {
                  BOOL bFirst;

                  ptr=pBuffer;
                  if(strlen(globalData.chrWriterName3)==0)
                    bFirst=TRUE;/* If we have no writer name, use the first one */
                  else
                    bFirst=FALSE;
                  do {
                    /* Get first entry with a driver information (grabbers don't have this info) */
                    PrfQueryProfileString(hini,ptr,"driver","",text,sizeof(text));
                    if(strlen(text)!=0) {
                      /* Insert the writer name */
                      WinSendMsg(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),LM_INSERTITEM,
                                 MPFROMSHORT(LIT_SORTASCENDING),
                                 (MPARAM)ptr);
                      if(bFirst) {
                        bFirst=FALSE;
                        WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),ptr);
                      }
                    }
                    ptr=strchr(ptr,0);
                    if(!ptr)
                      break;
                    ptr++; /* Next database entry */
                  }while(*ptr!=0);
                }
              free(pBuffer);
            }
          }
          PrfCloseProfile(hini);
        }
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_CDRDAOHELP, 20);

      /* Hide controls while scanning */
      showCdrdaoSettings(hwnd, FALSE);
      WinShowWindow(WinWindowFromID(hwnd, IDST_CDRDAOKNOWNSETTINGS), FALSE);

      /* Now scan the bus for new devices */
      WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDPB_CDRDAOSCAN), MPFROM2SHORT(CMDSRC_PUSHBUTTON, TRUE));

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
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
    case WM_HELP:
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),
                                                       QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      /* Switch the right help panel for the control with the focus to the front */
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {
          /*  */
        case IDEF_CDRDAOOPTIONS:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDRDAOOPTIONS, AFHELPLIBRARY);
          /* */
        default:
          break;
        }
      /* General page 2 help */
      return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDRDAOSETUP, AFHELPLIBRARY);
    case WM_CONTROL:
      {
        switch(SHORT1FROMMP(mp1))
          {
          case IDDD_CDRDAODEVICE:
            {
              /* The device changed... */
              /* This may also happen when scanbus.exe ended and the default device must be selected. */
              if(SHORT2FROMMP(mp1)==CBN_EFCHANGE) {
                /* Entry field changed */

                /* Check if we know the given writer and set the options accordingly if not
                   in expert mode. */

                /* if fScanning==FALSE the user changed the entry field */
                // if(!fScanning)

                WinStartTimer( WinQueryAnchorBlock(HWND_DESKTOP), hwnd, TIMERID_DEVICESELECT, 500);

#if 0
                else
                  if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE))
                    fAutomaticallyConfigured=setKnownCdrdaoSettingsCopy(hwnd);
#endif
              }/* CBN_EFCHANGE */
              break;
            }
          default:
            break;
          }
        break;
      }
      return (MRESULT) FALSE;
    case WM_TIMER:
      if(SHORT1FROMMP(mp1)==TIMERID_DEVICESELECT)
        {
          /* This timer is started when a new device is selected. We now check if it's a known
             device by querying the database. */

          WinStopTimer( WinQueryAnchorBlock(HWND_DESKTOP), hwnd, TIMERID_DEVICESELECT);

          /* Check if we know the given writer and set the options accordingly if not
             in expert mode. */
          if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE))
            fAutomaticallyConfigured=setKnownCdrdaoSettingsCopy(hwnd);

          return (MRESULT) 0;
        }
      break;
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* Stop timer if running */
      WinStopTimer( WinQueryAnchorBlock(HWND_DESKTOP), hwnd, TIMERID_DEVICESELECT);

      /* Query the options. */
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),
                          sizeof(globalData.chrCdrdaoOptions3), globalData.chrCdrdaoOptions3);
      /* Query the cdrdao writer (not driver) from the entryfield for audio folders */
      WinQueryWindowText( WinWindowFromID(hwnd,IDLB_CDRDAODRIVER ),
                          sizeof(globalData.chrWriterName3), globalData.chrWriterName3);
      WinQueryWindowText( WinWindowFromID(hwnd,IDDD_CDRDAODEVICE ),
                          sizeof(globalData.chrDeviceName3), globalData.chrDeviceName3);

      sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
      hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
      if(hini)
        {
          /* Get the associated driver */
          PrfQueryProfileString(hini, globalData.chrWriterName3 ,"driver","",globalData.chrCdrdaoDriver3,
                                sizeof(globalData.chrCdrdaoDriver3));
          PrfCloseProfile(hini);
        }          
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),QWL_USER);
      if(somIsObj(thisPtr))
        thisPtr->wpSaveImmediate();      
      /* Setup is done */
      return (MRESULT) TRUE;
    case WM_COMMAND:    
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_CDRDAOSCAN:
          if(startScanbusHelper(hwnd))
            fScanning=TRUE;
          break;
        case IDPB_CDRDAOUNDO:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),globalData.chrWriterName3);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),globalData.chrCdrdaoOptions3);
          WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), globalData.chrDeviceName3);
          break;
        default:
          break;
        }
      return (MRESULT) TRUE;
    case WM_APPTERMINATENOTIFY:
      switch(LONGFROMMP(mp1)) {
      case ACKEY_SCANBUS:
        {
          SHORT sItem;
          /* scanbus.exe terminated. Set the entryfield now */
          
          /* First check if our saved device is still installed */
          sItem=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE),
                                         LM_SEARCHSTRING, MPFROM2SHORT(LSS_PREFIX|LSS_CASESENSITIVE, LIT_FIRST),
                                         MPFROMP(globalData.chrDeviceName3)));

          if(sItem!=LIT_ERROR && sItem!=LIT_NONE) {
            /* Device found in list so put name into entry field*/
            WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), globalData.chrDeviceName3);
          }
          else {
            /* Our device no longer is installed */
            char chrDevice[CCHMAXPATH];
            if(SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE), LM_QUERYITEMTEXT,
                                  MPFROM2SHORT(0, sizeof(chrDevice)), MPFROMP(chrDevice))))
              WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), chrDevice);
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
        if(LONGFROMMP(mp2)==0) {
          /* Delete entries in listbox */
          WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE),"");
          WinSendMsg(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE),LM_DELETEALL,MPFROMLONG(0),MPFROMLONG(0));
        }
        else
          {
            WinSendMsg(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE),LM_INSERTITEM,MPFROMSHORT(LIT_END),
                       mp2);
          }
        break;
      default:
        break;
      }/* switch */
      break; 
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/***********************************************************************/
/*                                                                     */
/* This procedure handles the cdrdao settings page for 1:1 copy target */
/*                                                                     */
/***********************************************************************/
MRESULT EXPENTRY settingsCdrdaoOptionDlgProc3Expert(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWCreatorSettings* thisPtr;
  ULONG ulFlags;
  char text[CCHMAXPATH];

  char *ptr;
  HINI hini;
  ULONG ulSize;
  char *pBuffer;
  static  BOOL fScanning;

  switch(msg)
    {
    case WM_INITDLG :   
      WinSetWindowULong(WinWindowFromID(hwnd,IDPB_CDRDAOBROWSE),
                        QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRDAOPATH),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)sizeof(globalData.chrCdrdaoPath3)),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOPATH),globalData.chrCdrdaoPath3);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)sizeof(globalData.chrCdrdaoOptions3)),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),globalData.chrCdrdaoOptions3);
      /* Set the writer name (not the driver) */
      WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER), globalData.chrWriterName3);
      WinSetWindowText( WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), globalData.chrDeviceName3);
      /* Open database with drivers */
      sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
      hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
      if(hini)
        {
          if(PrfQueryProfileSize(hini, NULL,NULL,&ulSize)) { /* Get data size */
            pBuffer=(char*)malloc(ulSize);
            if(pBuffer) {
              if(PrfQueryProfileData(hini, NULL,NULL, pBuffer, &ulSize))
                {
                  BOOL bFirst;

                  ptr=pBuffer;
                  if(strlen(globalData.chrWriterName3)==0)
                    bFirst=TRUE;/* If we have no writer name, use the first one */
                  else
                    bFirst=FALSE;
                  do {
                    /* Get first entry with a driver information (grabbers don't have this info) */
                    PrfQueryProfileString(hini,ptr,"driver","",text,sizeof(text));
                    if(strlen(text)!=0) {
                      /* Insert the writer name */
                      WinSendMsg(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),LM_INSERTITEM,
                                 MPFROMSHORT(LIT_SORTASCENDING),
                                 (MPARAM)ptr);
                      if(bFirst) {
                        bFirst=FALSE;
                        WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),ptr);
                      }
                    }
                    ptr=strchr(ptr,0);
                    if(!ptr)
                      break;
                    ptr++; /* Next database entry */
                  }while(*ptr!=0);
                }
              free(pBuffer);
            }
          }
          PrfCloseProfile(hini);
        }
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_CDRDAOHELP, 20);

      /* Now scan the bus for new devices */
      WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDPB_CDRDAOSCAN), MPFROM2SHORT(CMDSRC_PUSHBUTTON, TRUE));

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
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
    case WM_HELP:
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),
                                                       QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      /* Switch the right help panel for the control with the focus to the front */
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {
          /*  */
        case IDEF_CDRDAOOPTIONS:
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDRDAOOPTIONS, AFHELPLIBRARY);
          /* */
        default:
          break;
        }
      /* General page 2 help */
      return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_CDRDAOSETUP, AFHELPLIBRARY);
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* Query the cdrdao path from the entryfield */     
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDRDAOPATH),sizeof(globalData.chrCdrdaoPath3),
                          globalData.chrCdrdaoPath3);
      /* Query the options. */
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),
                          sizeof(globalData.chrCdrdaoOptions3), globalData.chrCdrdaoOptions3);
      /* Query the cdrdao writer (not driver) from the entryfield for audio folders */
      WinQueryWindowText( WinWindowFromID(hwnd,IDLB_CDRDAODRIVER ),
                          sizeof(globalData.chrWriterName3), globalData.chrWriterName3);
      WinQueryWindowText( WinWindowFromID(hwnd,IDDD_CDRDAODEVICE ),
                          sizeof(globalData.chrDeviceName3), globalData.chrDeviceName3);

      sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
      hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
      if(hini)
        {
          /* Get the associated driver */
          PrfQueryProfileString(hini, globalData.chrWriterName3 ,"driver","",globalData.chrCdrdaoDriver3,
                                sizeof(globalData.chrCdrdaoDriver3));
          PrfCloseProfile(hini);
        }          
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_CDRECORDBROWSE),QWL_USER);
      if(somIsObj(thisPtr))
        thisPtr->wpSaveImmediate();      
      /* Setup is done */
      return (MRESULT) TRUE;
    case WM_COMMAND:    
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_CDRDAOBROWSE:
          {
            FILEDLG fd = { 0 };

            /* User pressed the browse button */
            fd.cbSize = sizeof( fd );
            /* It's an centered 'Open'-dialog */
            fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
            /* Title: "Search CDRecord/2" */
            getMessage(text,IDSTR_FDLGSEARCHCDRDAOTITLE,sizeof(text), hSettingsResource,hwnd);
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
                WinSetWindowText( WinWindowFromID(hwnd,IDEF_CDRDAOPATH), fd.szFullFile );
              }
            break;
          }
        case IDPB_CDRDAOSCAN:
          if(startScanbusHelper(hwnd))
            fScanning=TRUE;
          break;
        case IDPB_CDRDAOUNDO:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),globalData.chrWriterName3);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOPATH),globalData.chrCdrdaoPath3);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOOPTIONS),globalData.chrCdrdaoOptions3);
          WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), globalData.chrDeviceName3);
          break;
        default:
          break;
        }
      return (MRESULT) TRUE;
    case WM_APPTERMINATENOTIFY:
      switch(LONGFROMMP(mp1)) {
      case ACKEY_SCANBUS:
        {
          SHORT sItem;
          /* scanbus.exe terminated. Set the entryfield now */
          
          /* First check if our saved device is still installed */
          sItem=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE),
                                         LM_SEARCHSTRING, MPFROM2SHORT(LSS_PREFIX|LSS_CASESENSITIVE, LIT_FIRST),
                                         MPFROMP(globalData.chrDeviceName3)));

          if(sItem!=LIT_ERROR && sItem!=LIT_NONE) {
            /* Device found in list so put name into entry field*/
            WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), globalData.chrDeviceName3);
          }
          else {
            /* Our device no longer is installed */
            char chrDevice[CCHMAXPATH];
            if(SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE), LM_QUERYITEMTEXT,
                                  MPFROM2SHORT(0, sizeof(chrDevice)), MPFROMP(chrDevice))))
              WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE), chrDevice);
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
        if(LONGFROMMP(mp2)==0) {
          /* Delete entries in listbox */
          WinSetWindowText(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE),"");
          WinSendMsg(WinWindowFromID(hwnd, IDDD_CDRDAODEVICE),LM_DELETEALL,MPFROMLONG(0),MPFROMLONG(0));
        }
        else
          {
            WinSendMsg(WinWindowFromID(hwnd,IDDD_CDRDAODEVICE),LM_INSERTITEM,MPFROMSHORT(LIT_END),
                       mp2);
          }
        break;
      default:
        break;
      }/* switch */
      break; 
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
