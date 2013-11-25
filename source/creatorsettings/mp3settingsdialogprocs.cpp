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

#define IDTIMER_MP3PRIORITY 1

extern GLOBALDATA globalData;

extern char chrDeviceName[CCHMAXPATH];

extern char chrConfigDir[CCHMAXPATH];
extern int iMp3Decoder;

extern char chrInstallDir[CCHMAXPATH];

extern char chrMP3EncoderPath[CCHMAXPATH];
extern char chrMP3EncoderOptions[CCHMAXPATH];
extern ULONG ulMP3Bitrate;
extern ULONG ulMP3Quality;
extern ULONG ulMP3EncoderPriority;

extern HMODULE hSettingsResource;

/* MP3 encoder page 2 strings */
extern char g_chrMP3Names[NUM_MP3NAMES][MP3NAME_LEN];
extern char g_cChosenMP3Name[NUM_MP3NAMEPARTS];
extern char g_chrMP3NameExample[NUM_MP3NAMES][MP3NAME_LEN];
extern char g_chrMP3NameFillStrings[NUM_MP3NAME_FILLSTRINGS][MP3NAME_FILLSTRING_LEN];


/* Extern */
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

void SysWriteToTrapLog(const char* chrFormat, ...);

/* Local */
MRESULT EXPENTRY settingsMpg123OptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsMP3EncoderOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);


/*
  This checks if the mpg123 path is valid and pointing to the provided
  mpg123.exe.
 */
static BOOL mpg123PathValid()
{
  char tempPath[CCHMAXPATH];

  sprintf(tempPath, "%s\\bin\\mpg123.exe", chrInstallDir);
  //  SysWriteToTrapLog("mpg123: %s, calculated: %s\n", globalData.chrMpg123Path, tempPath);
  if(!stricmp(tempPath, globalData.chrMpg123Path))
    return TRUE;

  return FALSE;
}
/*
  Returns TRUE if autoconfiguration succeeded for MP3 decoding.
 */
static BOOL mp3AutomaticallyConfigured()
{
  if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE) && 
     (globalData.ulGlobalFlags & GLOBALFLAG_MMIOMP3INSTALLED && iMp3Decoder==IDKEY_USEMMIOMP3 ||
      (iMp3Decoder==IDKEY_USEMPG123 && mpg123PathValid())) )
    return TRUE;

  return FALSE;
}
/*
  Show/hide the swap bytes setting and an informative message about
  autoconfiguration.
 */
static void showSwapByteSetting(HWND hwnd, BOOL bShow)
{
  if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE) && 
     (globalData.ulGlobalFlags & GLOBALFLAG_MMIOMP3INSTALLED && iMp3Decoder==IDKEY_USEMMIOMP3 ||
     (iMp3Decoder==IDKEY_USEMPG123 && mpg123PathValid())) ) {
    WinEnableWindow(WinWindowFromID(hwnd, IDCB_MPG123SWABBYTES), bShow);
    WinEnableWindow(WinWindowFromID(hwnd, IDGB_MPG123SPECIAL), bShow);
    WinShowWindow(WinWindowFromID(hwnd, IDST_MP3DECKNOWNSETTINGS), !bShow);
    /* Disable Undo when automatic settings are in effect. Otherwise
       this will end in chaos because the Undo may be override the default
       setting for a known writer without the user noticing it. */
    cwEnableNotebookButton(hwnd, IDPB_MPG123UNDO, bShow);
  }
}


/*
  This function hides decoder settings not used in non-expert mode.
 */
static void hideOrNotMP3DecoderSettings(HWND hwnd)
{
  /* Hide no settings in expert mode */
  if(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE) {
    /* In expert mode we don't have any automatic settings. */
    showSwapByteSetting(hwnd, TRUE);
    return;
  }

  if(globalData.ulGlobalFlags & GLOBALFLAG_MMIOMP3INSTALLED &&
     iMp3Decoder==IDKEY_USEMMIOMP3
     ||
     (iMp3Decoder==IDKEY_USEMPG123 && mpg123PathValid()))
    {
      /* We only hide when using MMIOMP3 for now. */
      WinShowWindow(WinWindowFromID(hwnd,IDRB_MPG123), FALSE);
      WinShowWindow(WinWindowFromID(hwnd,IDRB_Z), FALSE);
      WinShowWindow(WinWindowFromID(hwnd,IDRB_MMIOMP3), FALSE);
      WinShowWindow(WinWindowFromID(hwnd,IDGB_MP3DECODERSETUP), FALSE);
      WinShowWindow(WinWindowFromID(hwnd,IDST_MPG123PATH), FALSE);
      WinShowWindow(WinWindowFromID(hwnd,IDEF_MPG123PATH), FALSE);
      WinShowWindow(WinWindowFromID(hwnd,IDPB_MPG123BROWSE), FALSE);

      //      showSwapByteSetting(hwnd, TRUE);
    }
}

/*
  This function queries the config data file for the 'Swap bytes' setting.
  This is done by searching for an entry in the format <'vendor' 'Identification'>.

  If the current writer is unknown the entry will not be present and MP3SWAP_NOTFOUND
  returned.

 */
#define MP3SWAP_NOTFOUND 2
static ULONG getSwapByteSettingFromDataBase(char* chrDevice)
{
  HINI hini;
  char text[CCHMAXPATH];
  ULONG rc=MP3SWAP_NOTFOUND;

  /* Open database with drivers */
  sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
  hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
  if(hini)
    {
      char * chrPtr;
      if((chrPtr=strchr(chrDevice, '\''))!=NULLHANDLE) {
        PrfQueryProfileString(hini, chrPtr, "drivename", "", text, sizeof(text));

        if(strlen(text)!=0) {
          /* Driver name found */
          rc=PrfQueryProfileInt(hini, text, "mp3swapbytes",  2);
        }
      }
      PrfCloseProfile(hini);
    }
  return rc;
}

/*
  Check for the 'Swap bytes' setting, set the option and hide/disable some
  controls to indicate autosetting.
 */
static void checkForKnownMP3DecSetting(HWND hwnd)
{
  switch(getSwapByteSettingFromDataBase(chrDeviceName))
    {
    case 1:
      /* We know the selected writer, so we can set the swap bytes option.
         The checkbox may be hidden. */
      WinCheckButton(hwnd, IDCB_MPG123SWABBYTES,1);
      showSwapByteSetting(hwnd, FALSE);
      break;
    case 0:
      WinCheckButton(hwnd,IDCB_MPG123SWABBYTES,0);
      showSwapByteSetting(hwnd, FALSE);
      break;
    default:
      /* Writer unknown. Use the global info and show the checkbox */
      if(globalData.bMpg123SwabBytes)
        WinCheckButton(hwnd,IDCB_MPG123SWABBYTES,1);
      else
        WinCheckButton(hwnd,IDCB_MPG123SWABBYTES,0);
      showSwapByteSetting(hwnd, TRUE);
      break;
    }
}

/****************************************************/
/*                                                  */ 
/* This procedure handles the mp3 decoder page      */
/* in the settings notebook                         */
/*                                                  */
/****************************************************/
MRESULT EXPENTRY settingsMpg123OptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
 
  switch(msg)
    {
    case WM_INITDLG :
      {
        CWCreatorSettings* thisPtr;

        WinSetWindowULong(hwnd, QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
        thisPtr=(CWCreatorSettings*)PVOIDFROMMP(mp2);
        /* Set MP3 decoder path */
        WinSendMsg(WinWindowFromID(hwnd,IDEF_MPG123PATH),EM_SETTEXTLIMIT,
                   MPFROMSHORT((SHORT)sizeof(globalData.chrMpg123Path)),0);
        WinSetWindowText(WinWindowFromID(hwnd,IDEF_MPG123PATH), globalData.chrMpg123Path);
        /* Set swap bytes setting. This may be overriden by automatic configuration
           if selected. */
        if(globalData.bMpg123SwabBytes)
          WinCheckButton(hwnd,IDCB_MPG123SWABBYTES,1);
        else
          WinCheckButton(hwnd,IDCB_MPG123SWABBYTES,0);
        
        /* Check if we know the given writer and set the swap bytes option accordingly but only if not
           in expert mode. In expert mode there's not autoconfiguration. */
        if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE))
          checkForKnownMP3DecSetting(hwnd);
        
        /* Which decoder do we use */
        switch(iMp3Decoder)
          {
          case IDKEY_USEMPG123:
            WinCheckButton(hwnd,IDRB_MPG123,1);
            WinEnableWindow(WinWindowFromID(hwnd,IDCB_MPG123SWABBYTES),TRUE);
            break;
          case IDKEY_USEZ:
            WinCheckButton(hwnd,IDRB_Z,1);
            WinEnableWindow(WinWindowFromID(hwnd,IDCB_MPG123SWABBYTES),FALSE);
            break;
          case IDKEY_USEMMIOMP3:
            WinCheckButton(hwnd,IDRB_MMIOMP3,1);
            WinEnableWindow(WinWindowFromID(hwnd,IDCB_MPG123SWABBYTES),TRUE);
            break;
          default:
            break;
          }
        /* Disable MMIOMP3 if not installed */
        if(!(globalData.ulGlobalFlags & GLOBALFLAG_MMIOMP3INSTALLED))
          WinEnableWindow(WinWindowFromID(hwnd,IDRB_MMIOMP3),FALSE);
        /* Hide settings not used in normal mode */
        hideOrNotMP3DecoderSettings(hwnd);
        /* Move default buttons on Warp 4 */
        cwMoveNotebookButtonsWarp4(hwnd, IDPB_MPG123HELP, 20);
        return (MRESULT) TRUE;
      }
      /* This prevents switching the notebook page behind the open folder */
    case WM_WINDOWPOSCHANGED:
      {
        MRESULT mr;
        PSWP pswp=(PSWP)PVOIDFROMMP(mp1);

        if(pswp && pswp->fl & SWP_SHOW)
          {
            /* The notebook page is shown. Maybe the user changed the writer in the 
               meantime so update the settings to the right ones. */
            
            /* Check if we know the given writer and set the swap bytes option accordingly but only if not
               in expert mode. In expert mode there's not autoconfiguration. */
            if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE))
              checkForKnownMP3DecSetting(hwnd);
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
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), 
                       WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
    case WM_HELP:
      {
        CWCreatorSettings* thisPtr;
        thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd,QWL_USER);
        if(!somIsObj(thisPtr))
          break;
        switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
          {
            /*        case IDPB_MPG123UNDO:
                      return (MRESULT)thisPtr->wpDisplayHelp(IDPB_GRABBERUNDO,AFHELPLIBRARY);
                      case IDPB_MPG123BROWSE:
                      return (MRESULT)thisPtr->wpDisplayHelp(IDPB_GRABBERBROWSE,AFHELPLIBRARY);
                      case IDEF_MPG123PATH:
                      return (MRESULT)thisPtr->wpDisplayHelp(IDEF_GRABBERPATH,AFHELPLIBRARY);*/
          default:
            break;
          }
        if(mp3AutomaticallyConfigured())
          return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_AUTOMATICCONFIG, AFHELPLIBRARY);
        break;
      }
    case WM_DESTROY:
      {
        CWCreatorSettings* thisPtr;
        /* The notebook closes and gets destroyed */
        /* Set focus to desktop to prevent PM freeze */
        WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);
        
        /* Query the mpg123 path from the entryfield */     
        WinQueryWindowText( WinWindowFromID(hwnd,IDEF_MPG123PATH),sizeof(globalData.chrMpg123Path),
                            globalData.chrMpg123Path);
        /* Swap bytes setting */
        if(WinQueryButtonCheckstate(hwnd,IDCB_MPG123SWABBYTES)&1)
          globalData.bMpg123SwabBytes=TRUE;
        else
          globalData.bMpg123SwabBytes=FALSE;
        /* Which decoder to use */
        if(WinQueryButtonCheckstate(hwnd,IDRB_MPG123)&1)
          iMp3Decoder=IDKEY_USEMPG123;
        else if(WinQueryButtonCheckstate(hwnd,IDRB_Z)&1)
          iMp3Decoder=IDKEY_USEZ;
        else if(WinQueryButtonCheckstate(hwnd,IDRB_MMIOMP3)&1)
          iMp3Decoder=IDKEY_USEMMIOMP3;
        
        /* Let the WPS save the new instance data */
        thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd, QWL_USER);
        if(somIsObj(thisPtr))
          thisPtr->wpSaveImmediate();
        
        /* Setup is done */
        return (MRESULT) TRUE;
      }
    case WM_CONTROL:
      {
        CWCreatorSettings* thisPtr;

        thisPtr=(CWCreatorSettings*) WinQueryWindowULong(hwnd, QWL_USER);
        if(!somIsObj(thisPtr))
          return (MRESULT) TRUE;
        /* Enable/disable swap bytes setting according to the grabber. z! never uses this setting so it
           may be disabled when z! is selected. */
        switch(SHORT1FROMMP(mp1))
          {
          case IDRB_MMIOMP3:
          case IDRB_MPG123:
            if(getSwapByteSettingFromDataBase(chrDeviceName)==MP3SWAP_NOTFOUND ||
               globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE)
              /* Allow changes only for unknown writers or in expert mode. */
              WinEnableWindow(WinWindowFromID(hwnd,IDCB_MPG123SWABBYTES),TRUE);
            break;
          case IDRB_Z:
            WinEnableWindow(WinWindowFromID(hwnd,IDCB_MPG123SWABBYTES),FALSE);
            break;
            
          default:
            break;
          }
        break;
      }
    case WM_COMMAND:	
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_MPG123BROWSE:
          {
            FILEDLG fd = { 0 };
            char text[CCHMAXPATH];
            /* User pressed the browse button */
            fd.cbSize = sizeof( fd );
            /* It's an centered 'Open'-dialog */
            fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
            /* Set the title of the file dialog */
            /* Title: "Search Audio-Grabber" */
            getMessage(text, IDSTR_FDLGSEARCHMPG123TITLE, sizeof(text), hSettingsResource,hwnd);
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
                WinSetWindowText( WinWindowFromID(hwnd,IDEF_MPG123PATH), fd.szFullFile );
              }
            break;
          }
        case IDPB_MPG123UNDO:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_MPG123PATH), globalData.chrMpg123Path);
          if(globalData.bMpg123SwabBytes)
            WinCheckButton(hwnd,IDCB_MPG123SWABBYTES,1);
          else
            WinCheckButton(hwnd,IDCB_MPG123SWABBYTES,0);
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



MRESULT EXPENTRY settingsMP3EncoderOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  FILEDLG fd = { 0 };
  CWCreatorSettings* thisPtr;
  int a;
  char chrCD[4];
  char text[CCHMAXPATH]; 
  static BOOL fPriorityWarned;
 
  switch(msg)
    {
    case WM_INITDLG :	
      WinSetWindowULong(WinWindowFromID(hwnd,IDPB_MP3ENCODERBROWSE),
                        QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      thisPtr=(CWCreatorSettings*)PVOIDFROMMP(mp2);
      WinSendMsg(WinWindowFromID(hwnd,IDEF_MP3ENCODERPATH),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT)sizeof(chrMP3EncoderPath)),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_MP3ENCODERPATH), chrMP3EncoderPath);
      WinSendMsg(WinWindowFromID(hwnd, IDEF_MP3ENCODEROPTIONS),EM_SETTEXTLIMIT,
                 MPFROMSHORT((SHORT) sizeof(chrMP3EncoderOptions)),0);
      WinSetWindowText(WinWindowFromID(hwnd, IDEF_MP3ENCODEROPTIONS),chrMP3EncoderOptions);

      /* Bitrate spin button */
      WinSendMsg(WinWindowFromID(hwnd,IDSB_BITRATE),SPBM_SETLIMITS,MPFROMLONG(320),MPFROMLONG(4));
      WinSendMsg(WinWindowFromID(hwnd,IDSB_BITRATE),SPBM_SETCURRENTVALUE,(MPARAM) ulMP3Bitrate, MPFROMLONG(0));

      WinEnableWindow(WinWindowFromID(hwnd, IDSB_BITRATE), FALSE);

      /* Priority spin button */
      WinSendMsg(WinWindowFromID(hwnd,IDSB_ENCODERPRIORITY),SPBM_SETLIMITS,MPFROMLONG(4),MPFROMLONG(0));
      WinSendMsg(WinWindowFromID(hwnd,IDSB_ENCODERPRIORITY),SPBM_SETCURRENTVALUE,
                 (MPARAM) ulMP3EncoderPriority, MPFROMLONG(0));

      switch(ulMP3Quality)
        {
        case IDQUALITY_VBRSTANDARD:
          WinCheckButton(hwnd, IDRB_VBRSTANDARD, 1);
          break;
        case IDQUALITY_VBREXTREME:
          WinCheckButton(hwnd, IDRB_VBREXTREME, 1);
          break;
        case IDQUALITY_AVERAGE:
          WinCheckButton(hwnd, IDRB_AVERAGE, 1);
          WinEnableWindow(WinWindowFromID(hwnd, IDSB_BITRATE), TRUE);
          break;
        case IDQUALITY_USERDEFINED:
          WinCheckButton(hwnd, IDRB_MP3ENCUSERDEFINED, 1);
          break;
        default:
          break;
        }
#if 0
      if(checkFileExists(chrMP3EncoderPath) && )
        {

        }
#endif
      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_MP3ENCODERHELP, 20);
      fPriorityWarned=FALSE;

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
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), 
                       WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
    case WM_HELP:	
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_MP3ENCODERBROWSE),QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {
          /*        case IDPB_MPG123UNDO:
                    return (MRESULT)thisPtr->wpDisplayHelp(IDPB_GRABBERUNDO,AFHELPLIBRARY);
                    case IDPB_MPG123BROWSE:
                    return (MRESULT)thisPtr->wpDisplayHelp(IDPB_GRABBERBROWSE,AFHELPLIBRARY);
                    case IDEF_MPG123PATH:
                    return (MRESULT)thisPtr->wpDisplayHelp(IDEF_GRABBERPATH,AFHELPLIBRARY);*/
        default:
          break;
        }
      return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_MP3ENCODERSETUP, AFHELPLIBRARY);
      break;
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      WinStopTimer(WinQueryAnchorBlock(HWND_DESKTOP), hwnd, IDTIMER_MP3PRIORITY);

      /* Query the mp3 encoder path from the entryfield */     
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_MP3ENCODERPATH), sizeof(chrMP3EncoderPath),chrMP3EncoderPath);
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_MP3ENCODEROPTIONS), sizeof(chrMP3EncoderOptions),
                          chrMP3EncoderOptions);

      WinSendMsg(WinWindowFromID(hwnd,IDSB_BITRATE), SPBM_QUERYVALUE,MPFROMP(&ulMP3Bitrate),
                 MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));

      WinSendMsg(WinWindowFromID(hwnd,IDSB_ENCODERPRIORITY), SPBM_QUERYVALUE,MPFROMP(&ulMP3EncoderPriority),
                 MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));

      if(WinQueryButtonCheckstate(hwnd,IDRB_VBRSTANDARD)&1)
        ulMP3Quality=IDQUALITY_VBRSTANDARD;
      else  if(WinQueryButtonCheckstate(hwnd,IDRB_VBREXTREME)&1)
        ulMP3Quality=IDQUALITY_VBREXTREME;
      else if(WinQueryButtonCheckstate(hwnd,IDRB_AVERAGE)&1)
        ulMP3Quality=IDQUALITY_AVERAGE;
      else if(WinQueryButtonCheckstate(hwnd, IDRB_MP3ENCUSERDEFINED)&1)
        ulMP3Quality=IDQUALITY_USERDEFINED;
      
      /* Let the WPS save the new instance data */
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDPB_MP3ENCODERBROWSE),QWL_USER);
      if(somIsObj(thisPtr))
        thisPtr->wpSaveImmediate();
      /* Setup is done */
      return (MRESULT) TRUE;
    case WM_CONTROL:
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd, IDPB_MP3ENCODERBROWSE),QWL_USER);
      if(!somIsObj(thisPtr))
        return (MRESULT) TRUE;
      switch(SHORT1FROMMP(mp1))
        {
        case IDRB_VBRSTANDARD:
        case IDRB_VBREXTREME:
        case IDRB_MP3ENCUSERDEFINED:
          WinEnableWindow(WinWindowFromID(hwnd, IDSB_BITRATE), FALSE);
          break;
        case IDRB_AVERAGE:
          WinEnableWindow(WinWindowFromID(hwnd, IDSB_BITRATE), TRUE);
          break;
        case IDSB_ENCODERPRIORITY:
          if(SHORT2FROMMP(mp1)==SPBN_CHANGE)
            if(!fPriorityWarned)
              WinStartTimer(WinQueryAnchorBlock(HWND_DESKTOP), hwnd, IDTIMER_MP3PRIORITY, 700);
          break;
        default:
          break;
        }
      break;
    case WM_TIMER:
      if(SHORT1FROMMP(mp1)==IDTIMER_MP3PRIORITY) {
        ULONG ulTemp;
        char text[255];
        WinStopTimer(WinQueryAnchorBlock(HWND_DESKTOP), hwnd, IDTIMER_MP3PRIORITY);
        WinSendMsg(WinWindowFromID(hwnd,IDSB_ENCODERPRIORITY), SPBM_QUERYVALUE,MPFROMP(&ulTemp),
                   MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));
        if(ulTemp>2 && !fPriorityWarned) {
          fPriorityWarned=TRUE;
#if 0
          /* Create fly over help window */
          HWND hwndBubbleClient;
          HWND hwndBubbleWindow;
          ULONG style=FCF_BORDER|FCF_NOBYTEALIGN;

          /* Create help window. Irt will automatically be destroyed when the notebook
             closes, because the notebook page is the owner. */
          hwndBubbleWindow=WinCreateStdWindow(HWND_DESKTOP,
                                              0,
                                              &style,
                                              WC_STATIC,
                                              // WC_FLYOVERCLIENT,
                                              "",
                                              SS_TEXT|DT_CENTER|DT_VCENTER,
                                              NULLHANDLE,
                                              400,
                                              &hwndBubbleClient);
          /* Done to remove the shadow */
          //     g_oldBubbleProc=WinSubclassWindow(hwndBubbleWindow, fnwpBubbleProc);
          WinSetOwner(hwndBubbleWindow, hwnd);
          getMessage(text, IDSTR_PRIORITYMESSAGETEXT, sizeof(text), hSettingsResource, hwnd);
          setFlyOverText(hwndBubbleWindow, text);
#endif
          DosBeep(1000, 100);
          showMsgBox2(hwnd, IDSTR_PRIORITYMESSAGETITLE, IDSTR_PRIORITYMESSAGETEXT, 
                      hSettingsResource, MB_MOVEABLE|MB_OK|MB_INFORMATION);
        }
        return (MRESULT) 0;
      }
      break;
    case WM_COMMAND:	
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_MP3ENCODERBROWSE:
          /* User pressed the browse button */
          fd.cbSize = sizeof( fd );
          /* It's an centered 'Open'-dialog */
          fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
          /* Set the title of the file dialog */
          /* Title: "Search LAME" */
          getMessage(text, IDSTR_FDLGSEARCHLAMETITLE, sizeof(text), hSettingsResource,hwnd);
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
              WinSetWindowText( WinWindowFromID(hwnd,IDEF_MP3ENCODERPATH), fd.szFullFile );
            }
          break;
        case IDPB_MP3ENCODERUNDO:
          /* User pressed the UNDO button */
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_MP3ENCODERPATH),chrMP3EncoderPath);
          WinSetWindowText(WinWindowFromID(hwnd, IDEF_MP3ENCODEROPTIONS),chrMP3EncoderOptions);
          /* Bitrate spin button */
          WinSendMsg(WinWindowFromID(hwnd,IDSB_BITRATE),SPBM_SETCURRENTVALUE,(MPARAM) ulMP3Bitrate, MPFROMLONG(0));
          WinEnableWindow(WinWindowFromID(hwnd, IDSB_BITRATE), FALSE);
          
          WinSendMsg(WinWindowFromID(hwnd,IDSB_ENCODERPRIORITY),
                     SPBM_SETCURRENTVALUE,(MPARAM) ulMP3EncoderPriority, MPFROMLONG(0));
          switch(ulMP3Quality)
            {
            case IDQUALITY_VBRSTANDARD:
              WinCheckButton(hwnd, IDRB_VBRSTANDARD, 1);
              break;
            case IDQUALITY_VBREXTREME:
              WinCheckButton(hwnd, IDRB_VBREXTREME, 1);
              break;
            case IDQUALITY_AVERAGE:
              WinCheckButton(hwnd, IDRB_AVERAGE, 1);
              WinEnableWindow(WinWindowFromID(hwnd, IDSB_BITRATE), TRUE);
              break;
            case IDQUALITY_USERDEFINED:
              WinCheckButton(hwnd, IDRB_MP3ENCUSERDEFINED, 1);
              break;
            default:
              break;
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

static void _fillMP3NameDropDown(HWND hwnd, int iWhichNamePart)
{
  WinSendMsg(hwnd, LM_INSERTITEM, MPFROMSHORT(0), (MPARAM)g_chrMP3Names[IDMP3NAMING_NONE]);
  WinSendMsg(hwnd, LM_INSERTITEM, MPFROMSHORT(LIT_SORTASCENDING), (MPARAM)g_chrMP3Names[IDMP3NAMING_ALBUM]);
  WinSendMsg(hwnd, LM_INSERTITEM, MPFROMSHORT(LIT_SORTASCENDING), (MPARAM)g_chrMP3Names[IDMP3NAMING_ARTIST]);
  WinSendMsg(hwnd, LM_INSERTITEM, MPFROMSHORT(LIT_SORTASCENDING), (MPARAM)g_chrMP3Names[IDMP3NAMING_TITLE]);
  WinSendMsg(hwnd, LM_INSERTITEM, MPFROMSHORT(LIT_SORTASCENDING), (MPARAM)g_chrMP3Names[IDMP3NAMING_TRACK]);
  WinSetWindowText(hwnd, g_chrMP3Names[g_cChosenMP3Name[iWhichNamePart]]);
} 

static void _fillMP3NameFillStringFields(HWND hwndDlg)
{
  WinSetWindowText(WinWindowFromID(hwndDlg, IDEF_MP3NAME1), g_chrMP3NameFillStrings[0]);
  WinSetWindowText(WinWindowFromID(hwndDlg, IDEF_MP3NAME2), g_chrMP3NameFillStrings[1]);
  WinSetWindowText(WinWindowFromID(hwndDlg, IDEF_MP3NAME3), g_chrMP3NameFillStrings[2]);
}

static int _idMP3NamingFromDropDown(HWND hwnd)
{
  int a;
  char chrText[50];

  WinQueryWindowText(hwnd, sizeof(chrText), 
                     chrText);
  for(a=0;a<NUM_MP3NAMES;a++)
    {
      if(!stricmp(chrText, g_chrMP3Names[a]))
        return a;
    }
  return IDMP3NAMING_NONE;
}


static void _buildMP3NameExample(HWND hwndDlg)
{
  char chrExample[200]={0};
  char chrText[10];

  /* First drop down box */
  strncat(chrExample,g_chrMP3NameExample[_idMP3NamingFromDropDown(WinWindowFromID(hwndDlg, IDDD_MP3NAME1))],
          sizeof(chrExample)-strlen(chrExample)-1);
  /* First entry field */
  WinQueryWindowText(WinWindowFromID(hwndDlg, IDEF_MP3NAME1), sizeof(chrText), 
                     chrText);
  strncat(chrExample, chrText,
          sizeof(chrExample)-strlen(chrExample)-1);

  /* Second drop down box */
  strncat(chrExample,g_chrMP3NameExample[_idMP3NamingFromDropDown(WinWindowFromID(hwndDlg, IDDD_MP3NAME2))],
          sizeof(chrExample)-strlen(chrExample)-1);
  WinQueryWindowText(WinWindowFromID(hwndDlg, IDEF_MP3NAME2), sizeof(chrText), 
                     chrText);
  strncat(chrExample, chrText,
          sizeof(chrExample)-strlen(chrExample)-1);

  strncat(chrExample,g_chrMP3NameExample[_idMP3NamingFromDropDown(WinWindowFromID(hwndDlg, IDDD_MP3NAME3))],
          sizeof(chrExample)-strlen(chrExample)-1);
  WinQueryWindowText(WinWindowFromID(hwndDlg, IDEF_MP3NAME3), sizeof(chrText), 
                     chrText);
  strncat(chrExample, chrText,
          sizeof(chrExample)-strlen(chrExample)-1);

  strncat(chrExample,g_chrMP3NameExample[_idMP3NamingFromDropDown(WinWindowFromID(hwndDlg, IDDD_MP3NAME4))],
          sizeof(chrExample)-strlen(chrExample)-1);

  strncat(chrExample, ".mp3",
          sizeof(chrExample)-strlen(chrExample)-1);

  WinSetWindowText(WinWindowFromID(hwndDlg, IDST_MP3NAMEEXAMPLE), chrExample);
}


MRESULT EXPENTRY dirDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  switch(msg)
    {
    case WM_COMMAND:
      {
        if(SHORT1FROMMP(mp1)==DID_OK)
          {
            FILEDLG *fd;
            fd=(FILEDLG*)WinQueryWindowULong(hwnd, QWL_USER);
            fd->szFullFile[strlen(fd->szFullFile)-1]=0;
            //            WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, fd->szFullFile, "", 123, MB_MOVEABLE|MB_OK);
            fd->lReturn=DID_OK;
            WinDismissDlg(hwnd, DID_OK);
            return (MRESULT)FALSE;
          }
        break;
      }
    case WM_CONTROL:
      {
        if(SHORT2FROMMP(mp1)==CBN_EFCHANGE) {
          FILEDLG *fd;
          char text[CCHMAXPATH];

          fd=(FILEDLG*)WinQueryWindowULong(hwnd, QWL_USER);
          strncpy(text, fd->szFullFile,CCHMAXPATH);
          text[CCHMAXPATH-1]=0;
          text[strlen(text)-1]=0;
          WinSetWindowText(WinWindowFromID(hwnd, 4096), text);
        }
        break;
      }
    case WM_INITDLG:
      {
        MRESULT mr;
#if 0
        FILEDLG *fd;
        
        fd=(FILEDLG*)WinQueryWindowULong(hwnd, QWL_USER);
        if(fd->szFullFile[0]!=0)
          {
            WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, fd->szFullFile, "", 123, MB_MOVEABLE|MB_OK);
            if(fd->szFullFile[strlen(fd->szFullFile)]!='\\') {
              strcat(fd->szFullFile, "\\*.*");
         WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, fd->szFullFile, "", 123, MB_MOVEABLE|MB_OK);
              
            }
          }
#endif
        mr=WinDefFileDlgProc(hwnd, msg, mp1, mp2);
        WinSetWindowText(WinWindowFromID(hwnd, 258),"");
        WinEnableWindow(WinWindowFromID(hwnd, DID_OK), TRUE);
        return mr;
      }
    default:
      break;
    }
  return WinDefFileDlgProc(hwnd, msg, mp1, mp2);
}

/****************************************************/
MRESULT EXPENTRY settingsMP3EncoderOptionDlgProc2(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  FILEDLG fd = { 0 };
  CWCreatorSettings* thisPtr;
  int a;
  char chrCD[4];
  char text[CCHMAXPATH]; 
 
  switch(msg)
    {
    case WM_INITDLG :	
      WinSetWindowULong(WinWindowFromID(hwnd,IDEF_MP3NAME1),
                        QWL_USER,(ULONG)PVOIDFROMMP(mp2));//Save object ptr.
      thisPtr=(CWCreatorSettings*)PVOIDFROMMP(mp2);

      _fillMP3NameDropDown(WinWindowFromID(hwnd, IDDD_MP3NAME1), 0);
      _fillMP3NameDropDown(WinWindowFromID(hwnd, IDDD_MP3NAME2), 1);
      _fillMP3NameDropDown(WinWindowFromID(hwnd, IDDD_MP3NAME3), 2);
      _fillMP3NameDropDown(WinWindowFromID(hwnd, IDDD_MP3NAME4), 3);

      _fillMP3NameFillStringFields(hwnd);
      WinSetWindowText(WinWindowFromID(hwnd, IDEF_MP3LIBRARY), globalData.chrMP3LibraryPath);

      /* Move default buttons on Warp 4 */
      cwMoveNotebookButtonsWarp4(hwnd, IDPB_MP3ENCODERHELP, 20);
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
            WinSendMsg(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT), QW_PARENT), 
                       WM_SETFOCUS, MPFROMHWND(hwnd), (MPARAM)TRUE);
            return mr;
          }
        }
        break;
      }
    case WM_HELP:	
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDEF_MP3NAME1),QWL_USER);
      if(!somIsObj(thisPtr))
        break;
      switch(WinQueryWindowUShort(WinQueryFocus(HWND_DESKTOP),QWS_ID))
        {
          /*        case IDPB_MPG123UNDO:
                    return (MRESULT)thisPtr->wpDisplayHelp(IDPB_GRABBERUNDO,AFHELPLIBRARY);
                    case IDPB_MPG123BROWSE:
                    return (MRESULT)thisPtr->wpDisplayHelp(IDPB_GRABBERBROWSE,AFHELPLIBRARY);
                    case IDEF_MPG123PATH:
                    return (MRESULT)thisPtr->wpDisplayHelp(IDEF_GRABBERPATH,AFHELPLIBRARY);*/
        default:
          break;
        }
      return (MRESULT)thisPtr->wpDisplayHelp(IDHLP_MP3ENCODER2SETUP, AFHELPLIBRARY);
      break;
    case WM_DESTROY:
      /* The notebook closes and gets destroyed */
      /* Set focus to desktop to prevent PM freeze */
      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);

      /* Query the entryfield strings */
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_MP3NAME1), sizeof(g_chrMP3NameFillStrings[0]),
                          g_chrMP3NameFillStrings[0]);
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_MP3NAME2), sizeof(g_chrMP3NameFillStrings[0]),
                          g_chrMP3NameFillStrings[1]);
      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_MP3NAME3), sizeof(g_chrMP3NameFillStrings[0]),
                          g_chrMP3NameFillStrings[2]);

      WinQueryWindowText( WinWindowFromID(hwnd,IDEF_MP3LIBRARY), sizeof(globalData.chrMP3LibraryPath),
                          globalData.chrMP3LibraryPath);

      g_cChosenMP3Name[0]=_idMP3NamingFromDropDown(WinWindowFromID(hwnd, IDDD_MP3NAME1));
      g_cChosenMP3Name[1]=_idMP3NamingFromDropDown(WinWindowFromID(hwnd, IDDD_MP3NAME2));
      g_cChosenMP3Name[2]=_idMP3NamingFromDropDown(WinWindowFromID(hwnd, IDDD_MP3NAME3));
      g_cChosenMP3Name[3]=_idMP3NamingFromDropDown(WinWindowFromID(hwnd, IDDD_MP3NAME4));

      /* Let the WPS save the new instance data */
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd,IDEF_MP3NAME1),QWL_USER);
      if(somIsObj(thisPtr)) {
        thisPtr->wpSaveImmediate();
      }
      /* Setup is done */
      return (MRESULT) TRUE;
    case WM_CONTROL:
      thisPtr=(CWCreatorSettings*) WinQueryWindowULong(WinWindowFromID(hwnd, IDEF_MP3NAME1),QWL_USER);
      if(!somIsObj(thisPtr)) return (MRESULT) TRUE;

      //      if(SHORT2FROMMP(mp1)==CBN_EFCHANGE)
      //     DosBeep(5000, 200);
      if(SHORT2FROMMP(mp1)==CBN_EFCHANGE) {
        switch(SHORT1FROMMP(mp1))
          {
          case IDDD_MP3NAME1:
          case IDDD_MP3NAME2:
          case IDDD_MP3NAME3:
          case IDDD_MP3NAME4:
            _buildMP3NameExample(hwnd);
          default:
            break;
          }
      }

      if(SHORT2FROMMP(mp1)==EN_CHANGE) {
        switch(SHORT1FROMMP(mp1))
          {
          case IDEF_MP3NAME1:
          case IDEF_MP3NAME2:
          case IDEF_MP3NAME3:
            _buildMP3NameExample(hwnd);
          default:
            break;
          }
      }
      break;
    case WM_COMMAND:	
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_MP3LIBRARYBROWSE:
          /* User pressed the browse button */
          fd.cbSize = sizeof( fd );
          /* It's an centered 'Open'-dialog */
          fd.fl = FDS_CUSTOM|FDS_OPEN_DIALOG|FDS_CENTER;
          fd.pfnDlgProc=dirDialogProc;
          fd.usDlgId=IDDLG_DIRECTORY;
          fd.hMod=hSettingsResource;
          /* Set the title of the file dialog */
          /* Title: "Search Audio-Grabber" */
          //   getMessage(text, IDSTR_FDLGSEARCHMPG123TITLE, sizeof(text), hSettingsResource,hwnd);
          //   fd.pszTitle = text;
          /* Only show *.exe files */
          strncpy(fd.szFullFile, globalData.chrMP3LibraryPath, CCHMAXPATH);
          strcat(fd.szFullFile, "\\");
          fd.szFullFile[CCHMAXPATH-1]=0;
          if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
            {
              /* WinFileDlg failed */
              break;
            }
          if( fd.lReturn == DID_OK )
            {
              WinSetWindowText( WinWindowFromID(hwnd,IDEF_MP3LIBRARY), fd.szFullFile );
            }
          break;
        case IDPB_MP3ENCODERUNDO:
          /* User pressed the UNDO button */
          _fillMP3NameDropDown(WinWindowFromID(hwnd, IDDD_MP3NAME1), 0);
          _fillMP3NameDropDown(WinWindowFromID(hwnd, IDDD_MP3NAME2), 1);
          _fillMP3NameDropDown(WinWindowFromID(hwnd, IDDD_MP3NAME3), 2);
          _fillMP3NameDropDown(WinWindowFromID(hwnd, IDDD_MP3NAME4), 3);
          _fillMP3NameFillStringFields(hwnd);
          WinSetWindowText(WinWindowFromID(hwnd, IDEF_MP3LIBRARY), globalData.chrMP3LibraryPath);

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

