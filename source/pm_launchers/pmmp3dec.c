/*
 * pmmp3dec.c (C) Chris Wohlgemuth 2002-2003
 *
 * This helper decodes an MP3
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
#define INCL_DOS
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_OS2MM
#define INCL_MMIOOS2
#define INCL_MCIOS2

#include <os2.h>

#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <string.h>
#include "os2me.h"
#include "audiofolder.h"
#include "audiofolderres.h"


//#define DEBUG

void writeLog2(char * installDir, char * logName, char* logText);
BOOL percentRegisterBarClass(void);
void errorResource2(char * chrTitle);

char chrInstallDir[CCHMAXPATH];
char logName[]="MP3Decod.log";

extern int iMp3Decoder;
extern char chrMpg123Path[];
extern SWP swpWindow;
BOOL bHaveWindowPos=FALSE;

char chrTargetName[CCHMAXPATH]={0};

int numArgs;
char* params[5];
int iMp3;

  /* argv[0]: progname
   * argv[1]: installdir of Audio/Data-CD-Creator
   * argv[2]: 1: Decode an MP3
   * argv[3]: wavefile
   * argv[4]: directory of MP3
   */

  int rate;
  int iBitRate;
  SHORT stereo;
  LONG lSec;

MMAUDIOHEADER mmAudioHeader;

HMODULE RESSOURCEHANDLE=0;
HMODULE CLASSDLLHANDLE=0;

void pmUsage();
BOOL readWindowPosFromIni(char * installDir, char *chrKey);
void HlpSendCommandToObject(char* chrObject, char* command);

BOOL createTargetName(char *sourceName)
{
  char *textPtr;

  strcpy(chrTargetName, sourceName);
  if((textPtr=strrchr(chrTargetName, '.'))==NULLHANDLE)
    return FALSE;
  *textPtr=0;
  strcat(textPtr, ".wav");

  return TRUE;
}


BOOL audioHlpStartMp3Query(char *name, HWND hwnd)
{
  char chrCmd[CCHMAXPATH+100];

  switch(iMp3Decoder)
    {
    case IDKEY_USEMPG123:
      sprintf(chrCmd,"\"%s\\bin\\mp3info.exe\" \"%s\" %d",chrInstallDir, name, iMp3Decoder);
      break;
    case IDKEY_USEZ: /* We have z! use it for the info */
      sprintf(chrCmd,"\"%s\" \"%s\" %d",chrMpg123Path, name, iMp3Decoder);
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

/* This Proc handles the on-the-fly data CD writing */
MRESULT EXPENTRY decodeStatusDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH*4 +10];
  char title[CCHMAXPATH*4];
  SWCNTRL swctl;
  PID pid;
  int a;
  int iPercent;

  switch (msg)
    {      
    case WM_INITDLG:

      /* Add switch entry */
      memset(&swctl,0,sizeof(swctl));
      WinQueryWindowProcess(hwnd,&pid,NULL);
      swctl.hwnd=hwnd;
      swctl.uchVisibility=SWL_VISIBLE;
      swctl.idProcess=pid;
      swctl.bProgType=PROG_DEFAULT;
      swctl.fbJump=SWL_JUMPABLE;
      WinAddSwitchEntry(&swctl);

#if 0
      sprintf(text,"1: %s, 2: %s, 3: %s 4: %s 5: %s 6: %s",params[1],params[2],params[3],
              params[4], params[4],params[4]);
      WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text,
                     params[4],
                     0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
#endif

      WinSendMsg(WinWindowFromID(hwnd,IDST_WAVENAME),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      
      switch(iMp3) 
        {
        case 1:
          /* Query info for MP3 */
          if(iMp3Decoder==IDKEY_USEMPG123) {
            WinMessageBox( HWND_DESKTOP, hwnd,
                           "Sorry, mpg123 as a decoding engine currently not supported. Get z! from http://dink.org for decoding.",
                           "MP3-Decoding",
                           0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
            WinDismissDlg(hwnd,0);
          }
          
          WinSetWindowText(WinWindowFromID(hwnd,IDST_CDBITS), "");
          
          /* Filename */
          WinSetWindowText(WinWindowFromID(hwnd,IDST_WAVENAME),params[3]);
          audioHlpStartMp3Query(params[3], hwnd);
          break;
        default:
          break;
        }
      
      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }

      /* Set percent bars to 0. */
      WinSetWindowText(WinWindowFromID(hwnd,IDSR_DECODINGPROGRESS),"0#0%");

#if 0
      pbarProgressBarFromStatic(WinWindowFromID(hwnd, IDSR_DECODINGPROGRESS), PBA_ALIGNLEFT);
      /* Set percent bar value to zero */
      WinPostMsg(WinWindowFromID(hwnd,IDSR_DECODINGPROGRESS),WM_UPDATEPROGRESSBAR,MPFROMLONG(0),
                 MPFROMLONG(0));
#endif

      WinSendMsg(WinWindowFromID(hwnd,IDEF_TARGETNAME), EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      WinSetWindowText( WinWindowFromID(hwnd,IDEF_TARGETNAME), chrTargetName );
      WinEnableWindow( WinWindowFromID(hwnd,IDPB_OK), FALSE);
      if(!bHaveWindowPos)
        WinSetWindowPos(hwnd,HWND_TOP,0,0,0,0,SWP_ZORDER|SWP_ACTIVATE|SWP_SHOW);
      else
        WinSetWindowPos(hwnd,HWND_TOP,swpWindow.x, swpWindow.y, 0, 0, SWP_MOVE|SWP_ZORDER|SWP_ACTIVATE|SWP_SHOW);

      return (MRESULT) TRUE;
      /* WM_APPTERMINATENOTIFY messages are sent from the helper programs e.g. format checker. */
    case WM_APPTERMINATENOTIFY:
      switch(LONGFROMMP(mp1))
        {
        case ACKEY_MP3INFO:
          rate=SHORT2FROMMP(mp2);
          iBitRate=SHORT1FROMMP(mp2);
          iBitRate>>=2;
          stereo=SHORT1FROMMP(mp2) & 0x3;

          /* Channels */
          getMessage(title, IDSTR_PMWAVEINFOCHANNELS, sizeof(title), RESSOURCEHANDLE, hwnd);
          if(stereo)	
            sprintf(text, title, 2);
          else
            sprintf(text, title, 1);
          WinSetWindowText(WinWindowFromID(hwnd,IDST_CHANNELS),text);
          
          /* Bitrate */
          getMessage(title, IDSTR_PMMP3INFOBITRATE, sizeof(title), RESSOURCEHANDLE, hwnd);	
          sprintf(text, title, iBitRate);
          WinSetWindowText(WinWindowFromID(hwnd,IDST_BITPERSAMPLE),text);
          
          /* Samplerate */
          getMessage(title, IDSTR_PMWAVEINFOSAMPLERATE, sizeof(title), RESSOURCEHANDLE, hwnd);	
          sprintf(text, title, rate);
          WinSetWindowText(WinWindowFromID(hwnd,IDST_SAMPLERATE),text);

          break;
        case ACKEY_PLAYTIME:
          {

            lSec=LONGFROMMP(mp2);
            lSec/=(44100*4);
            
            /* Playtime */
            getMessage(title, IDSTR_PMWAVEINFOPLAYTIME, sizeof(title), RESSOURCEHANDLE, hwnd);	
            sprintf(text, title, lSec/60, lSec%60);
            WinSetWindowText(WinWindowFromID(hwnd,IDST_PLAYTIME),text);
            
            /* argv[0]: progname
             * argv[1]: installdir of Audio/Data-CD-Creator
             * argv[2]: 1: Decode an MP3
             * argv[3]: wavefile
             * argv[4]: directory of MP3
             */

            WinEnableWindow( WinWindowFromID(hwnd,IDPB_OK), TRUE);
            break;
          }
        case ACKEY_MBWRITTEN:          
          iPercent=LONGFROMMP(mp2);
          if(iPercent>100)
            iPercent=100;
          if(iPercent<0)
            iPercent=0;

          /* Update track percent bar value. The helper prog sends us the actual decoded %. */         
          sprintf(text,"%d#%d%%", iPercent, iPercent);
          WinSetWindowText(WinWindowFromID(hwnd,IDSR_DECODINGPROGRESS), text);

#if 0
          WinPostMsg(WinWindowFromID(hwnd, IDSR_DECODINGPROGRESS),WM_UPDATEPROGRESSBAR,MPFROMLONG(LONGFROMMP(mp2)),
                     MPFROMLONG(100));
#endif
          break;
        case ACKEY_MP3DECODE:
          {
            /* Decoding done */
            char chrCommand[400];
            sprintf( chrCommand, "MMAUDIOCOPYTAGTO=%s",chrTargetName);
            /* Set tags in *.wav file */
            HlpSendCommandToObject(params[3], chrCommand);
            HlpSendCommandToObject(chrTargetName, "MMREFRESH=1");
            
            WinPostMsg(hwnd, WM_CLOSE,0,0);
          break;
          }
        default:
          break;
        }
      return FALSE;

    case WM_CLOSE:
      WinQueryWindowPos(hwnd,&swpWindow);
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_BROWSE:
          {
            FILEDLG fd = { 0 };
            /* User pressed the browse button */
            fd.cbSize = sizeof( fd );
            /* It's an centered 'Open'-dialog */
            fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
            /* Title: "Search CDRecord/2" */
            //     getMessage(text,IDSTR_FDLGSEARCHCDR2TITLE,sizeof(text), hSettingsResource,hwnd);
            /* Set the title of the file dialog */
            //            fd.pszTitle = text;
            fd.pszTitle = "Wave name";
            /* Only show * files */
            //sprintf(fd.szFullFile,"%s","*");
            strcpy(fd.szFullFile, chrTargetName);
            if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
              {
                /* WinFileDlg failed */
                break;
              }
            if( fd.lReturn == DID_OK )
              {
                WinSetWindowText( WinWindowFromID(hwnd,IDEF_TARGETNAME), fd.szFullFile );
              }
            break;
          }
        case IDPB_ABORT:
          /* User pressed the OK button */
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        case IDPB_OK:
          {
            char writeParams[1024];
            FSALLOCATE fsAlloc;
            long long lFreeSpace;
            ULONG ulDiskNum;
            char cLetter;
            ULONG ulNeededSize;

            WinQueryWindowText( WinWindowFromID(hwnd,IDEF_TARGETNAME), sizeof(chrTargetName), chrTargetName );

            /* Check if diskspace is sufficient */
            cLetter=tolower(chrTargetName[0]);
            ulDiskNum=cLetter-'a'+1;
            if(DosQueryFSInfo(ulDiskNum, FSIL_ALLOC,&fsAlloc,sizeof(fsAlloc)))
              lFreeSpace=0;
            else
              lFreeSpace=fsAlloc.cUnitAvail*fsAlloc.cbSector*fsAlloc.cSectorUnit;
       
             ulNeededSize=stereo ? 4 : 2;
             ulNeededSize*=lSec*rate;
             if(lFreeSpace<ulNeededSize) {
               /*
                 Text:   
                 Title: 
                 */       
               getMessage(title, IDSTR_PMMP3DECODENOSPACETEXT,sizeof(title), RESSOURCEHANDLE, hwnd);
               sprintf(text,title,ulNeededSize/1000000 );
               getMessage(title, IDSTR_PMMP3DECODENOSPACETITLE,sizeof(title), RESSOURCEHANDLE, hwnd);
               WinMessageBox( HWND_DESKTOP, hwnd, text,
                              title,
                              1234UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
             }
             else {
               /* Start decoding a mp3 file */
               sprintf(writeParams,"\"%s\" \"%s\" \"%s\" %d \"%s\"",chrMpg123Path,
                       params[3], /* wavefile */
                       params[1], iMp3Decoder, chrTargetName);
               launchWrapper(writeParams,params[4],hwnd,"mp3decod.exe","Decoding MP3 file");
               WinEnableWindow( WinWindowFromID(hwnd,IDPB_ABORT), FALSE);            
             }
             break;
          }
        default:
          break;
        }
      return (MRESULT) FALSE;
    default:
      break;
    }/* switch */
  
  return WinDefDlgProc( hwnd, msg, mp1, mp2);
}

int main (int argc, char *argv[])
{
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  short a;
  HWND hwndClient;
  HMMIO hmmio;
  ULONG result;
  LONG lBytesRead=0;
  ULONG rc;

  /* Create a copy of the args */
  /* argv[0]: progname
   * argv[1]: installdir of Audio/Data-CD-Creator
   * argv[2]: 1: Decode an MP3
   * argv[3]: mp3file
   * argv[4]: directory of MP3
   */

  numArgs=argc;

  sprintf(text,"");
  for(a=0;a<argc;a++)
    {
      params[a]=argv[a];
    }


  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {  
      /* Check if user started prog by hand */   
      /*if(argc!=5) {*/
      if(argc<3) {/* Not the actual num of params */
        pmUsage();
      }
      else {
        /*char text[3000];
          sprintf(text,"1: %s,  2: %s,  3: %s 4: %s",params[1],params[2],params[3],
          params[4]);
          WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text,
          params[4],
          0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
          */
        
        /* Save installation directory */
        strcpy(chrInstallDir,params[1]);
        /* Create target name */
        createTargetName(params[3]);
        removeLog2(chrInstallDir, logName);
        /* Get our ressource dll */  
        RESSOURCEHANDLE=queryResModuleHandle2(chrInstallDir);
        iMp3=atoi(params[2]);

        readIni2(chrInstallDir);
        if(readWindowPosFromIni(chrInstallDir, "pmmp3dec"))
          bHaveWindowPos=TRUE;
        rc=0;        
        if(!rc) {

          /* Query info for MP3 */
          if(iMp3Decoder==IDKEY_USEMPG123) {
            WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, 
                           "Sorry, mpg123 as a decoding engine currently not supported. Get z! from http://dink.org for decoding.",
                           "MP3-Decoding",
                           0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
          }
          else {            
            /* Register the percent bar window class */
            percentRegisterBarClass();
            if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, decodeStatusDialogProc, RESSOURCEHANDLE, IDDLG_MP3DECODING, 0) == DID_ERROR )
              {
                DosBeep(100,600);
                errorResource2("Problem with Audio/Data-CD-Creator installation");              
                WinDestroyMsgQueue( hmq );
                WinTerminate( hab );
                return( 1 );
              }
            writeWindowPosToIni(chrInstallDir, "pmmp3dec");
          }
        }
        freeResHandle();
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}









