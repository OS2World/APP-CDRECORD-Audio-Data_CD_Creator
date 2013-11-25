/*
 * pmwvinfo.c (C) Chris Wohlgemuth 2001
 *
 * This helper shows the format of a wavefile in a dialog box
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


char chrInstallDir[CCHMAXPATH];
char logName[]="WaveInfo.log";

extern int iMp3Decoder;
extern char chrMpg123Path[];
extern SWP swpWindow;
BOOL bHaveWindowPos=FALSE;

int numArgs;
char* params[4];
int iMp3;

  /* argv[0]: progname
   * argv[1]: installdir of Audio/Data-CD-Creator
   * argv[2]: wavefile
   */

MMAUDIOHEADER mmAudioHeader;

HMODULE RESSOURCEHANDLE=0;
HMODULE CLASSDLLHANDLE=0;

void pmUsage();
void errorResource2(char * chrTitle);

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
    default:
      return FALSE;
    }

  /* Launch helper */
  if(launchWrapper(chrCmd, "", hwnd,"mp3size.exe","Query mp3 size")==-1)
    return FALSE;/* The helper isn't avaiable or can't be started */

  return TRUE;
}

/* This Proc handles the on-the-fly data CD writing */
MRESULT EXPENTRY waveinfoStatusDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH*2 +10];
  char title[CCHMAXPATH];
  SWCNTRL swctl;
  PID pid;
  int a;
  int rate;
  int iBitRate;
  SHORT stereo;
  LONG lSec;

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

      /*      sprintf(text,"1: %s, 2: %s, 3: %s",params[1],params[2],params[3]);
              WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text,
              params[3],
              0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
              */

      WinSendMsg(WinWindowFromID(hwnd,IDST_WAVENAME),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      
      if(!iMp3 || (iMp3 && iMp3Decoder==IDKEY_USEMMIOMP3)) {
        /* Query a wave */          
        switch (mmAudioHeader.mmXWAVHeader.WAVEHeader.usFormatTag)
          {
          case DATATYPE_WAVEFORM:
            getMessage(text, IDSTR_PMWAVEINFOPCM, sizeof(text), RESSOURCEHANDLE, hwnd);
            break;
          case DATATYPE_ALAW:
            getMessage(text, IDSTR_PMWAVEINFOALAW, sizeof(text), RESSOURCEHANDLE, hwnd);
            break;
          case DATATYPE_MULAW:
            getMessage(text, IDSTR_PMWAVEINFOMULAW, sizeof(text), RESSOURCEHANDLE, hwnd);
            break;
          case DATATYPE_ADPCM_AVC:
            getMessage(text, IDSTR_PMWAVEINFOADPCM, sizeof(text), RESSOURCEHANDLE, hwnd);
            break;
          default:
            getMessage(text, IDSTR_PMWAVEINFOUNKNOWN, sizeof(text), RESSOURCEHANDLE, hwnd);	
            break;
          }
        
        /* Channels */
        getMessage(title, IDSTR_PMWAVEINFOCHANNELS, sizeof(title), RESSOURCEHANDLE, hwnd);	
        sprintf(text, title, mmAudioHeader.mmXWAVHeader.WAVEHeader.usChannels);
        WinSetWindowText(WinWindowFromID(hwnd,IDST_CHANNELS),text);
        
        /* Bit per sample */
        getMessage(title, IDSTR_PMWAVEINFOBITPERSAMPLE, sizeof(title), RESSOURCEHANDLE, hwnd);	
        sprintf(text, title,mmAudioHeader.mmXWAVHeader.WAVEHeader.usBitsPerSample);
        WinSetWindowText(WinWindowFromID(hwnd,IDST_BITPERSAMPLE),text);
        
        /* Samplerate */
        getMessage(title, IDSTR_PMWAVEINFOSAMPLERATE, sizeof(title), RESSOURCEHANDLE, hwnd);	
        sprintf(text, title, mmAudioHeader.mmXWAVHeader.WAVEHeader.ulSamplesPerSec);
        WinSetWindowText(WinWindowFromID(hwnd,IDST_SAMPLERATE),text);
        
        /* Filename */
        WinSetWindowText(WinWindowFromID(hwnd,IDST_WAVENAME),params[3]);
        
        /* Playtime */
        getMessage(title, IDSTR_PMWAVEINFOPLAYTIME, sizeof(title), RESSOURCEHANDLE, hwnd);	
        sprintf(text, title, mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
					 mmAudioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec/60,
					 mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
					 mmAudioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec%60);
        WinSetWindowText(WinWindowFromID(hwnd,IDST_PLAYTIME),text);

      }
      else
        {
          /* Query info for MP3 */
          getMessage(title, IDSTR_PMMP3INFODLGTITLE, sizeof(title), RESSOURCEHANDLE, hwnd);	
          WinSetWindowText(hwnd,title);

          WinSetWindowText(WinWindowFromID(hwnd,IDST_CDBITS), "");
        
          /* Filename */
          WinSetWindowText(WinWindowFromID(hwnd,IDST_WAVENAME),params[3]);

          audioHlpStartMp3Query(params[3], hwnd);
        }
      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }

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
          lSec=LONGFROMMP(mp2);
          lSec/=(44100*4);

          /* Playtime */
          getMessage(title, IDSTR_PMWAVEINFOPLAYTIME, sizeof(title), RESSOURCEHANDLE, hwnd);	
          sprintf(text, title, lSec/60, lSec%60);
          WinSetWindowText(WinWindowFromID(hwnd,IDST_PLAYTIME),text);

          break;
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
        case IDPB_OK:
          /* User pressed the OK button */
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
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
     argv[1]: installdir
     argv[2]: 0: query wave, 1: query mp3
     argv[3]: wavefile
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
      if(argc!=4) {
        pmUsage();}
      else {
        /* Save installation directory */
        strcpy(chrInstallDir,params[1]);
        /* Get our ressource dll */  
        RESSOURCEHANDLE=queryResModuleHandle2(chrInstallDir);
        iMp3=atoi(params[2]);

        removeLog2(chrInstallDir, logName);
        readIni2(chrInstallDir);
        if(readWindowPosFromIni(chrInstallDir, "pmwvinfo"))
          bHaveWindowPos=TRUE;
        rc=-1;
        if(!iMp3 || (iMp3 && iMp3Decoder==IDKEY_USEMMIOMP3)) {
          // open the wave file
          do {
            MMIOINFO mmioinfo;

            memset(&mmioinfo,0, sizeof(mmioinfo));
            mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
            mmioinfo.ulFlags=MMIO_READ|MMIO_DENYNONE;

            hmmio = mmioOpen(params[3], &mmioinfo, MMIO_READ);
            if(!hmmio) {
              /*
                Text:   "Can't open file %s"
                Title:  "Wavefile Information"                  
                */
              
              getMessage(title, IDSTR_PMWAVEINFOFILEERROR, sizeof(title), RESSOURCEHANDLE, HWND_DESKTOP);
              sprintf(text, title, params[2]);	
              getMessage(title, IDSTR_PMWAVEINFOTITLE, sizeof(title), RESSOURCEHANDLE, HWND_DESKTOP);	
              WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text,
                             title,
                             0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
              
              break;
            }
            memset(&mmAudioHeader,0,sizeof(MMAUDIOHEADER));
            result = mmioGetHeader(hmmio, &mmAudioHeader,sizeof(MMAUDIOHEADER),
                                   &lBytesRead, 0, 0);
            
            if(result!=MMIO_SUCCESS) {
              mmioClose(hmmio, 0);
              /*
                Text:   "Can't get MMAUDIOHEADER"
                Title:  "Wavefile Information"                
                */             
              messageBox( text, IDSTR_PMWAVEINFOMMAUDIOERROR , sizeof(text),
                          title, IDSTR_PMWAVEINFOTITLE , sizeof(title),
                          RESSOURCEHANDLE, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
              break;
            }
            mmioClose(hmmio, 0);
            rc=0;
            break;
          }while (FALSE);
        }
        else {
          rc=0;
        }
 
       if(!rc) {
          if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, waveinfoStatusDialogProc, RESSOURCEHANDLE, IDDLG_WAVEINFO, 0) == DID_ERROR )
            {
              DosBeep(100,600);
              errorResource2("Problem with Audio/Data-CD-Creator installation");              
              WinDestroyMsgQueue( hmq );
              WinTerminate( hab );
              return( 1 );
            }
          writeWindowPosToIni(chrInstallDir, "pmwvinfo");      
        }
        freeResHandle();
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}









