/*
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

#define INCL_WIN
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "audiofolderres.h"

#ifndef BS_NOTEBOOKBUTTON
#define BS_NOTEBOOKBUTTON 8L   /* Warp 4 notebook style */  
#endif

extern char chrInstallDir[CCHMAXPATH];

ULONG showMsgBox2(HWND hwnd, ULONG ulIDTitle, ULONG ulIDText, HMODULE hModule, ULONG ulFlag);
APIRET APIENTRY DosReplaceModule ( PSZ pszOldModule, PSZ pszNewModule, PSZ pszBackupModule );

/* Mutex semaphores to protect filename generation */
ULONG cwCreateMutex(HMTX * hmtxBMP) {

  return DosCreateMutexSem( NULL, hmtxBMP, 0, FALSE);
}

ULONG cwCloseMutex(HMTX  hmtxBMP) {

  return DosCloseMutexSem( hmtxBMP );
}

ULONG cwRequestMutex(HMTX  hmtxBMP, ULONG ulTimeOut) {

  return DosRequestMutexSem( hmtxBMP, ulTimeOut );
}

ULONG cwReleaseMutex(HMTX  hmtxBMP) {

  return DosReleaseMutexSem( hmtxBMP );
}


/****************************************************
 *                                                  *
 * This function returns the running OS version:    *
 *                                                  *
 * 30: Warp 3, 40 Warp 4 etc.                       *
 *                                                  *
 ****************************************************/
ULONG cwQueryOSRelease()
{
  static ULONG ulVersionMinor=0;

  if(!ulVersionMinor)
    if(DosQuerySysInfo(QSV_VERSION_MINOR, QSV_VERSION_MINOR, &ulVersionMinor, sizeof(ulVersionMinor)))
      ulVersionMinor=30;/* Default Warp 3 */

  return ulVersionMinor;

}
/****************************************************
 *                                                  *
 * This function moves the default buttons off      *
 * notebook pages on Warp 4.                        *
 * usDelta specifies the units all controls have    *
 * to be moved down. usID is the ID of the last     *
 * button that has to be moved on the main frame    *
 * of the notebook. This value is a threshold.      *
 *                                                  *
 ****************************************************/
BOOL cwMoveNotebookButtonsWarp4(HWND hwndDlg, USHORT usID, USHORT usDelta)
{
  if (cwQueryOSRelease()>=40) {
    HENUM henum;
    HWND hwnd;
    
    /* Move the default notebook buttons */
    if((henum=WinBeginEnumWindows(hwndDlg))!=NULLHANDLE) {
      while((hwnd=WinGetNextWindow(henum))!=NULLHANDLE) {
        if(WinQueryWindowUShort(hwnd,QWS_ID)<=usID)
          WinSetWindowBits(hwnd, QWL_STYLE,
                           BS_NOTEBOOKBUTTON, BS_NOTEBOOKBUTTON);
        else {
          SWP swp;
          POINTL ptl= {0};
          ptl.y=usDelta;

          WinMapDlgPoints(hwndDlg, &ptl, 1, TRUE);
          /* Move all other controls */
          if(WinQueryWindowPos(hwnd, &swp))
            WinSetWindowPos(hwnd, NULLHANDLE, swp.x, swp.y-ptl.y,0,0, SWP_MOVE);
        }
      }
      WinEndEnumWindows(henum);
    }
  }
  return TRUE;
}

/****************************************************
 *                                                  *
 * This function disables a settings notebook       *
 * button. On Warp 4 it climbs up the windows       *
 * to do so.                                        *
 *                                                  *
 ****************************************************/
BOOL cwEnableNotebookButton(HWND hwndDlg, USHORT usID, BOOL fEnable)
{
  if (cwQueryOSRelease()>=40) {
    HWND hwnd;
    
    /* Disable the notebook button */
    if((hwnd=WinQueryWindow(hwndDlg, QW_PARENT))!=NULLHANDLE)
      {
        /* hwnd: notebook child #54 */
        if((hwnd=WinQueryWindow(hwnd, QW_PARENT))!=NULLHANDLE) {
          /* hwnd: notebook handle */
          HENUM henum;
          if((henum=WinBeginEnumWindows(hwnd))!=NULLHANDLE) {
            while((hwnd=WinGetNextWindow(henum))!=NULLHANDLE) {
              if(WinQueryWindowUShort(hwnd,QWS_ID)==usID && 
                 WinIsWindowVisible(hwnd)) {
                WinEnableWindow(hwnd, fEnable);
                break;
              }
            }/* while() */
            WinEndEnumWindows(henum);
          }/* henum*/
        }
      }
  }
  else
    WinEnableWindow(WinWindowFromID(hwndDlg, usID), fEnable);

  return TRUE;
}

/****************************************************
 *                                                  *
 * This funktion inserts a separator into menu      *
 * <hwndMenu> and submenu <hwndSubMenu> at          *
 * position <iPosition>                             *
 *                                                  *
 ****************************************************/
MRESULT cwInsertMenuSeparator(int iPosition, HWND hwndMenu, HWND hwndSubMenu)
{
  MENUITEM mi;

  /* Fill the MENUITEM structure */
  mi.iPosition=iPosition;
  mi.afStyle=MIS_SEPARATOR;
  if(hwndSubMenu)
    mi.afStyle|=MIS_SUBMENU;
  mi.id=0;
  mi.afAttribute=NULL;                
  mi.hwndSubMenu=hwndSubMenu;
  mi.hItem=NULL;

  return WinSendMsg(hwndMenu,MM_INSERTITEM,(MPARAM)&mi,
                    (MPARAM)NULL);                 
}

/****************************************************
 *                                                  *
 * This funktion inserts an item into menu          *
 * <hwndMenu> and submenu <hwndSubMenu> at          *
 * position <iPosition>                             *
 *                                                  *
 ****************************************************/
MRESULT cwInsertMenuItem(int iPosition, HWND hwndMenu, HWND hwndSubMenu, int iID, char * chrText)
{
  MENUITEM mi;

  /* Fill the MENUITEM structure */
  mi.iPosition=iPosition;
  mi.afStyle=MIS_TEXT;
  if(hwndSubMenu)
    mi.afStyle|=MIS_SUBMENU;
  mi.id=iID;
  mi.afAttribute=NULL;                
  mi.hwndSubMenu=hwndSubMenu;
  mi.hItem=NULL;
 
  return WinSendMsg(hwndMenu,MM_INSERTITEM,(MPARAM)&mi,
                    (MPARAM)chrText);                 
}


/* This function checks if the given file exists */ 
BOOL checkFileExists(char* chrFileName)
{
  struct stat statBuf;

  /* Check file path */
  if(stat(chrFileName , &statBuf)==-1)
    return FALSE;
  else
    return TRUE;
}


/******************************************************/
/* This function installs a new language DLL in the   */
/* right place.                                       */
/******************************************************/
BOOL installLanguageDLL(char *dllPath, HMODULE hModule)
{
  char targetPath[CCHMAXPATH];
  char *ptr;

  if(!dllPath || dllPath[0]==0)
    return FALSE;

  if((ptr=strrchr(dllPath, '\\'))==NULLHANDLE)
    return FALSE;

  if(checkFileExists(dllPath))
    if(showMsgBox2(HWND_DESKTOP, IDSTR_INSTLANGUAGETITLE, IDSTR_INSTLANGUAGEDLLEXISTS, hModule, MB_MOVEABLE|MB_YESNO|MB_ICONQUESTION)==MBID_NO)
      return FALSE;
 
  sprintf(targetPath, "%s%s", chrInstallDir,ptr );
  if(DosCopy( dllPath, targetPath, DCPY_EXISTING))
    DosReplaceModule(targetPath, dllPath, NULL);

  return TRUE;
}

/******************************************************/
/* This function checks if the file is a mp3 file.    */
/* It only checks the extension.                      */
/******************************************************/
BOOL audioHlpIsMp3File(char* fileName)
{
  char* chrExtension;
  
  if(!checkFileExists(fileName))
    return FALSE;
  
  /* Check the extension */
  chrExtension=strrchr(fileName,'.');
  if(!chrExtension)
    return FALSE; /* No extension */
  
  chrExtension=strstr(strlwr(chrExtension),".mp3");
  if(chrExtension)
    if(*(chrExtension+4)==0)return TRUE;//It's a mp3file
  
  return FALSE;
}

void sendCommand(char *chrObject, char* command)
{
  HOBJECT hObject;
  char chrCommand[CCHMAXPATH*2];

  hObject=WinQueryObject(chrObject);
  if(hObject!=NULLHANDLE) {
    strncpy(chrCommand,command, sizeof(chrCommand));
    chrCommand[sizeof(chrCommand)-1]=0;
    WinSetObjectData(hObject,chrCommand);
  }
}

void HlpSendCommandToObject(char* chrObject, char* command)
{
  HOBJECT hObject;
  char chrCommand[CCHMAXPATH];

  hObject=WinQueryObject(chrObject);
  if(hObject!=NULLHANDLE) {
    strcpy(chrCommand,command);
    WinSetObjectData(hObject,chrCommand);
  }
}

void HlpOpenWebPage(char* chrUrl)
{
  PROGDETAILS pd;
  char chrBrowser[CCHMAXPATH];
  char chrWorkDir[CCHMAXPATH];
  char chrLoadError[CCHMAXPATH];

  if(0==PrfQueryProfileString(HINI_PROFILE, "WPURLDEFAULTSETTINGS", "DefaultBrowserExe", NULLHANDLE,
                        chrBrowser, sizeof(chrBrowser)))
    {
      /* No default browser in user ini */
      WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, 
                    "No default browser set in the user ini file.", "Error", 1, 
                    MB_ICONEXCLAMATION|MB_OK|MB_MOVEABLE);
      return;
    }
  if(!checkFileExists(chrBrowser))
    {
      /* Browser path is not valid */
      WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, 
                    "The browser path set in the user ini file is not valid.", "Error", 1, 
                    MB_ICONEXCLAMATION|MB_OK|MB_MOVEABLE);
      return;
    }

  if(0==PrfQueryProfileString(HINI_PROFILE, "WPURLDEFAULTSETTINGS", "DefaultWorkingDir", NULLHANDLE,
                              chrWorkDir, sizeof(chrWorkDir)))
    {
      char * chr;
      /* No working dir. Build it from browser path */
      strncpy(chrWorkDir, chrBrowser, sizeof(chrWorkDir));
      chrWorkDir[sizeof(chrWorkDir)-1]=0;
      if((chr=strrchr(chrWorkDir, '\\'))!=NULLHANDLE)
        *chr=0;
    }


  /* Now start the browser with the URL */
  memset(&pd,0,sizeof(pd));
  pd.Length=sizeof(pd);
  pd.progt.progc=PROG_DEFAULT;
  pd.progt.fbVisible=SHE_VISIBLE;
  
  pd.pszExecutable=chrBrowser;
  //pd.pszParameters=chrUrl;
  pd.pszStartupDir=chrWorkDir;
  pd.swpInitial.fl=SWP_ACTIVATE;
  pd.swpInitial.hwndInsertBehind=HWND_TOP;
  
  if(!WinStartApp(NULLHANDLE, &pd, chrUrl, NULLHANDLE, 0))
    {
      WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, 
                    "Starting of the browser failed.", "Error", 1, 
                    MB_ICONEXCLAMATION|MB_OK|MB_MOVEABLE);
      
    };
  return;  
}



