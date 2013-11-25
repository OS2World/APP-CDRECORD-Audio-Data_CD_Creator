/*
 * pmwrtaud.cpp (C) Chris Wohlgemuth 1999-2004
 *
 * This helper handles the GUI stuff for TAO audio CD creation
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

#include <os2.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include "progbars.h"
#include "audiofolder.h"
#include "audiofolderres.h"

#define MULTI_BURN /* We allow multiple writers */

//#define DEBUG
extern char chrCDROptions[CCHMAXPATH];
extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern char chrMpg123Path[CCHMAXPATH];/* Path to mpg123 */
extern BOOL bMpg123SwabBytes;
extern int iMp3Decoder;
extern LONG  lCDROptions;
extern SWP swpWindow;
BOOL bHaveWindowPos=FALSE;

char chrAudioParams[CCHMAXPATH*2];
char chrInstallDir[CCHMAXPATH];

PVOID pvSharedMem;
char *ptrTrack;
char *ptrTrack2;
char * pipePtr;
int numTracks=0;
int currentTrack=1;
char currentTrackName[CCHMAXPATH];

BOOL bNofix=FALSE;
BOOL bEject=FALSE;
ULONG ulTrackSize=0;
ULONG ulTotalWritten=0;
ULONG ulTotalSize=0;
BOOL bDecodingMp3=FALSE;

char logName[CCHMAXPATH]="WriteTAO.log";

int numArgs;
char* params[99+4];
/* params[0]: progname
 * params[1]: installdir of Audio-CD-Creator
 * params[2]: foldername
 * params[3]: # tracks
 * params[4]: size of all tracks
 */

/* The cdrecord start params and the names of the audio files are copied to shared mem
by the classes. This helper gets \\SHAREMEM\\AUDIOCMDLINE memory during processing of
WM_INITDLG. */


HMODULE RESSOURCEHANDLE=0;
HMODULE CLASSDLLHANDLE=0;

/* Funcs in helper.c */
void sendCommand(PSZ command);
ULONG launchWrapper(PSZ parameter, PSZ folderPath,HWND hwnd, PSZ wrapperExe, PSZ pszTitle);
void writeLog(char* logText);
void removeLog(void);
void pmUsage();
BOOL percentRegisterBarClass(void);
void setupGroupBoxControl(HWND hwnd, USHORT id);
void setupStaticTextControl(HWND hwnd, USHORT id);
BOOL HlpPaintFrame(HWND hwnd, BOOL bInclBorder);
void _loadBmps(HMODULE hSettingsResource);
void freeClassDLLHandle(void);
HMODULE queryClassDLLModuleHandle(char *installDir);
void errorResource2(char * chrTitle);


void extractAudioParams(void) {
 char * ptrChr, *ptrChr2;

 /* Find begin of first trackname */
 ptrTrack=strchr(pvSharedMem,'"');

 if(!ptrTrack)
   return;

 *ptrTrack=0;
 sprintf(chrAudioParams,"%s",pvSharedMem);
 *ptrTrack='"';

 /* Check, if we should fix the CD */
 if( strstr(strlwr(chrAudioParams),"-nofix"))
   bNofix=TRUE;

 /* Check, if we should eject the CD */
 if((ptrChr=strstr(strlwr(chrAudioParams),"-eject"))!=NULL) {
   bEject=TRUE;
   /* Remove the eject switch so the CD will not be ejected after everyy track */
   ptrChr2=ptrChr;
   ptrChr2+=6;/* Position after the -eject switch */
   *ptrChr=0;
   strcat(chrAudioParams,ptrChr2);/* Concatenate the parts of the params */
 }

}

ULONG queryFileSize(char * fileName)
{
  HFILE hf;
  ULONG ulAction=0;
  APIRET rc;
  FILESTATUS3 fs3={{0}};  
  char text[CCHMAXPATH];
  
  /* Remove >"< from Name */
  sprintf(text,"%s",fileName+1);
  *(strrchr(text,'"'))=0;

  rc=DosOpen(text,&hf,&ulAction,0L,0L,OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
             OPEN_FLAGS_NOINHERIT|OPEN_ACCESS_READONLY|OPEN_SHARE_DENYNONE,0L);
  if(rc!=NO_ERROR)
    return 0;

  rc=DosQueryFileInfo(hf,FIL_STANDARD,&fs3,sizeof(FILESTATUS3));
  if(rc!=NO_ERROR) {
    DosClose(hf);
    return 0;
  }
 
  DosClose(hf);
  return fs3.cbFile;
}

void writeTrack(HWND hwnd) {
  char * ptrChr;
  char * ptrChr2;
  char writeParams[CCHMAXPATH*2];
  char text[CCHMAXPATH];

  /* Find begin of trackname */
  ptrChr=strchr(ptrTrack,'"');
  if(!ptrChr){
    WinPostMsg(hwnd,WM_CLOSE,0,0);
    return;
  }

  ptrChr2=ptrChr;
  ptrChr2++;
  /* Find end of trackname */
  ptrTrack=strchr(ptrChr2,'"');

  if(!ptrTrack){
    WinPostMsg(hwnd,WM_CLOSE,0,0);
    return;    
  }

  ptrTrack++;
  *ptrTrack=0;

  strncpy(currentTrackName,ptrChr,CCHMAXPATH-1);

  /* Reset decoding flag */
  bDecodingMp3=FALSE;
  
  /* Start writer here */
  if(strstr(strlwr(ptrChr),".wav")){
    /* It's a wave file */

    /* Query size of track */
    ulTrackSize=queryFileSize(ptrChr)-44;/* subtract wave header size */

#if 0
    if((currentTrack!=numTracks)&&!bNofix)
      sprintf(writeParams,"\"%s -nofix\" \"\"%s\"\"",chrAudioParams, ptrChr);
    else
      sprintf(writeParams,"\"%s\" \"\"%s\"\"",chrAudioParams, ptrChr);
#endif

    if((currentTrack!=numTracks)&&!bNofix)
      sprintf(writeParams,"\"%s -nofix",chrAudioParams);
    else
      sprintf(writeParams,"\"%s",chrAudioParams);

    if((currentTrack==numTracks)&& bEject)
      strcat(writeParams, " -eject\" \"\"");
    else
      strcat(writeParams, "\" \"\"");

    strcat(writeParams, ptrChr);
    strcat(writeParams, "\"\"");

    launchWrapper(writeParams,params[2],hwnd,"writeaud.exe","Write audio track");
    writeLog("Writing track: ");
    writeLog(writeParams);  
    writeLog("\n");    
    currentTrack++;
  }
  else {
    /* It's a mp3 */
    /* Text: "Decoding %s" */
    getMessage(text, IDSTRA_DECODINGMSG,sizeof(text), RESSOURCEHANDLE , hwnd);
    bDecodingMp3=TRUE;
    /* Add the filename */
    ptrChr2=strrchr(ptrChr,'\\');
    ptrChr2++;
    sprintf(writeParams,text,ptrChr2);
    WinSetWindowText(WinWindowFromID(hwnd,IDST_WRITESTATUS),writeParams);

    /* Start decoding an mp3 file */
    sprintf(writeParams,"\"%s\" \"\"%s\"\" \"%s\" %d",chrMpg123Path,ptrChr,params[1],iMp3Decoder);
    launchWrapper(writeParams,params[2],hwnd,"mp3decod.exe","Decoding MP3 file");
    writeLog("Decoding MP3: ");
    writeLog(writeParams);
    writeLog("\n");    
  };

  *ptrTrack=' ';
}

void writeDecodedMp3(HWND hwnd) {
  char * ptrChr;
  char * ptrChr2;
  char writeParams[CCHMAXPATH*2];
  char text[CCHMAXPATH];
  char filename[CCHMAXPATH];
  char swab[10];

  /* Start writer here */

  bDecodingMp3=FALSE;
  if(iMp3Decoder==IDKEY_USEMPG123) {
    sprintf(text,"\"%s\\%s\"", params[2],"dec_mp3.raw");
    sprintf(filename,"%s","dec_mp3.raw");
    if(bMpg123SwabBytes)
      sprintf(swab," -swab");
    else
      sprintf(swab,"");
  }
  else if(iMp3Decoder==IDKEY_USEZ){
    sprintf(text,"\"%s\\%s\"", params[2],"dec_mp3.wav");/* z! writes real wave files */
    sprintf(filename,"%s","dec_mp3.wav");
    sprintf(swab,"");
  }
  else if(iMp3Decoder==IDKEY_USEMMIOMP3) {
    sprintf(text,"\"%s\\%s\"", params[2],"dec_mp3.raw");
    sprintf(filename,"%s","dec_mp3.raw");
    if(bMpg123SwabBytes)
      sprintf(swab," -swab");
    else
      sprintf(swab,"");
  }
  ulTrackSize=queryFileSize(text);

#if 0
  if((currentTrack!=numTracks)&&!bNofix)
    sprintf(writeParams,"\"%s -nofix%s\" \"%s\\%s\"",chrAudioParams, swab, params[2],filename);
  else
    sprintf(writeParams,"\"%s%s\" \"%s\\%s\"",chrAudioParams, swab, params[2],filename);
#endif

  if((currentTrack!=numTracks)&&!bNofix)
    sprintf(writeParams,"\"%s -nofix%s",chrAudioParams, swab);
  else
    sprintf(writeParams,"\"%s%s",chrAudioParams, swab);
  
  if((currentTrack==numTracks)&& bEject)
    strcat(writeParams, " -eject\" \"");
  else
    strcat(writeParams, "\" \"");
  
  strcat(writeParams, params[2]);
  strcat(writeParams, "\\");
  strcat(writeParams, filename);
  strcat(writeParams, "\"");
  
  launchWrapper(writeParams,params[2],hwnd,"writeaud.exe","Writing MP3 track");
  writeLog("Writing MP3 track: ");
  writeLog(writeParams);
  writeLog("\n");    
  
  currentTrack++;
}


/* This Proc handles the on-the-fly data CD writing */
MRESULT EXPENTRY audioStatusDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  char *textPtr;
  char *textPtr2; 
  ULONG rc;
  SWCNTRL swctl;
  PID pid;
  static BOOL bAbort;
  PVOID pvPrivateMem;

  switch (msg)
    {      
    case WM_PAINT:
      {
        if(HlpPaintFrame(hwnd, TRUE))
          return (MRESULT)0;
        break;
      }
    case WM_INITDLG:   
      writeLog("Initializing dialog...\n");  
      if(DosGetNamedSharedMem(&pvSharedMem,"\\SHAREMEM\\AUDIOCMDLINE",PAG_READ|PAG_WRITE)) {
        /* Text: "Can't alloc shared memory! Aborting..."
           Title: "Write audio tracks"
           */
        messageBox( text, IDSTRPM_ALLOCSHAREDMEMERROR , sizeof(text),
                    title, IDSTRA_AUDIOWRITETITLE, sizeof(title),
                    RESSOURCEHANDLE, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
        writeLog("Can't allocate shared mem! Aborting...\n");  
        sendCommand("FREESHAREDMEM=1");
        WinPostMsg(hwnd,WM_CLOSE,0,0);
        return (MRESULT) TRUE;
      }
      /* Let the class free the shared mem. If this helper crashes and can't send a msg to the
         class the memory isn't blocked. */
      sendCommand("FREESHAREDMEM=1");
      bAbort=FALSE;

      /* Add switch entry */
      memset(&swctl,0,sizeof(swctl));
      WinQueryWindowProcess(hwnd,&pid,NULL);
      swctl.hwnd=hwnd;
      swctl.uchVisibility=SWL_VISIBLE;
      swctl.idProcess=pid;
      swctl.bProgType=PROG_DEFAULT;
      swctl.fbJump=SWL_JUMPABLE;
      WinAddSwitchEntry(&swctl);

      /* Set percent bar to 0. */
      WinSetWindowText(WinWindowFromID(hwnd,IDSR_AUDIOPERCENT),"0#0%");
      /* Set percent bar to 0. */
      WinSetWindowText(WinWindowFromID(hwnd,IDSR_AUDIOPERCENTALL),"0#0%");

      /* Text: "Writing track %d of %d     " */
      getMessage(text, IDSTRA_WRITEAUDIOTRACK,sizeof(text), RESSOURCEHANDLE , hwnd);

      sprintf(title,text,currentTrack, numTracks);
      WinSetWindowText(WinWindowFromID(hwnd,IDST_WRITESTATUS),title);
      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }

      /* Custom painting */
      setupGroupBoxControl(hwnd, IDGB_WRITESTATUS);
      setupStaticTextControl(hwnd, IDST_WRITESTATUS);

      if(!bHaveWindowPos)        
        WinSetWindowPos(hwnd,HWND_TOP,0,0,0,0,SWP_ZORDER|SWP_ACTIVATE|SWP_SHOW);
      else
        WinSetWindowPos(hwnd,HWND_TOP,swpWindow.x, swpWindow.y, 0, 0, SWP_MOVE|SWP_ZORDER|SWP_ACTIVATE|SWP_SHOW);

#ifdef MULTI_BURN
      /* Copy the shared mem contents to private mem and free the shared mem */    
      if(DosAllocMem(&pvPrivateMem, AUDIOSHAREDMEM_SIZE,PAG_COMMIT|PAG_READ|PAG_WRITE)==NO_ERROR) {
        memcpy(pvPrivateMem, pvSharedMem, AUDIOSHAREDMEM_SIZE);
        DosFreeMem(pvSharedMem);        
        pvSharedMem=pvPrivateMem;
      }
#endif

      /* Extract the cdrecord options from shared mem */
      extractAudioParams();
      /* Write first track */
      writeTrack(hwnd);
      return (MRESULT) TRUE;
    case WM_CLOSE:
      WinQueryWindowPos(hwnd,&swpWindow);
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_HELP:
      sprintf(text,"DISPLAYHELPPANEL=%d",IDDLG_WRITESTATUS);
      sendCommand(text);      
      break;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_WRITEBREAK:
          /* User pressed the BREAK button */
          DosBeep(1000,100);
          WinEnableWindow(WinWindowFromID(hwnd,IDPB_WRITEBREAK),FALSE);
          bAbort=TRUE;
          writeLog("User pressed BREAK.\n");
          break;
        }
      break;
    case WM_APPTERMINATENOTIFY:
      if(1/*thisPtr*/) {
        switch(LONGFROMMP(mp1)) {
        case ACKEY_MBWRITTEN:
          {
            int iPercent;

            if(!bDecodingMp3) {
              iPercent=LONGFROMMP(mp2);
              iPercent*=100;
              iPercent/=(ulTrackSize/1024/1024);
              
              if(iPercent>100)
                iPercent=100;
              if(iPercent<0)
                iPercent=0;
              
              sprintf(text,"%d#%d%%", iPercent, iPercent);
              WinSetWindowText(WinWindowFromID(hwnd,IDSR_AUDIOPERCENT), text);
              
              iPercent=(ulTotalWritten/1024/1024+LONGFROMMP(mp2));
              iPercent*=100;
              iPercent/=(ulTotalSize/1024/1024);

              if(iPercent>100)
                iPercent=100;
              if(iPercent<0)
                iPercent=0;
              
              sprintf(text,"%d#%d%%", iPercent, iPercent);
              WinSetWindowText(WinWindowFromID(hwnd,IDSR_AUDIOPERCENTALL), text);                
            }
            else {
              iPercent=LONGFROMMP(mp2);
              if(iPercent>100)
                iPercent=100;
              if(iPercent<0)
                iPercent=0;
              /* Update percent bar value. The helper prog sends us the actual written percent. */
              sprintf(text,"%d#%d%%", iPercent, iPercent);
              WinSetWindowText(WinWindowFromID(hwnd,IDSR_AUDIOPERCENT), text);  
            }
            break;
          }
        case ACKEY_MP3DECODE:
          if(LONGFROMMP(mp2)==0) {
            /* MP3 decoded, write the track */
            /* Text: "Writing track %d of %d     " */
            getMessage(text, IDSTRA_WRITEAUDIOTRACK,sizeof(text), RESSOURCEHANDLE , hwnd);
            sprintf(title,text,currentTrack, numTracks);
            writeLog(title);
            writeLog("\n");
            WinSetWindowText(WinWindowFromID(hwnd,IDST_WRITESTATUS),title);        	    
            /* Reset track percent bar to zero */
            WinSetWindowText(WinWindowFromID(hwnd,IDSR_AUDIOPERCENT),"0#0%");
            if(!bAbort)
              writeDecodedMp3(hwnd);
        else
          WinPostMsg(hwnd,WM_CLOSE,0,0);
	    break;
	  }
	  else
	    {
	      /* Error while decoding mp3 */
	      /* Remove the wave file which is corrupted */
          if(iMp3Decoder==IDKEY_USEMPG123)
            sprintf(text,"%s\\dec_mp3.raw",params[2]);
          else
            sprintf(text,"%s\\dec_mp3.wav",params[2]);
	      remove(text);
	      /* title: CD writing error!
		 text:  Error while decoding MP3 file. Do you want to write the next track?
		 */
          writeLog("Error while decoding MP3 file!\n");
	      rc=messageBox( text, IDSTRPM_DECODINGERROR , sizeof(text),
			 title, IDSTR_WRITEERRORTITLE, sizeof(title),
			 RESSOURCEHANDLE, hwnd, MB_YESNO | MB_ICONEXCLAMATION|MB_MOVEABLE);
	      if(rc==MBID_YES){
		currentTrack++;/* Next track */
		WinPostMsg(hwnd,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_WRITEAUDIO),MPFROMLONG(0));
		break;
	      }
	      else{
		WinPostMsg(hwnd,WM_CLOSE,0,0);
		break;
	      }
	    }
	  break;
	case ACKEY_WRITEAUDIO:
	  /* The track is written. The wrapper program has ended */
      ulTotalWritten+=ulTrackSize;
	  /* Remove wave file of decoded mp3 */
      if(iMp3Decoder==IDKEY_USEMPG123)
        sprintf(text,"%s\\dec_mp3.raw",params[2]);
      else
        sprintf(text,"%s\\dec_mp3.wav",params[2]);
      remove(text);
	  if(LONGFROMMP(mp2)==0) {
        //        createCover();
        if(currentTrack<=numTracks){
	      /* There're more tracks to write */
	      /* Text: "Writing track %d of %d     " */
	      getMessage(text, IDSTRA_WRITEAUDIOTRACK,sizeof(text), RESSOURCEHANDLE , hwnd);
	      sprintf(title,text,currentTrack, numTracks);
          writeLog(title);
          writeLog("\n");
	      WinSetWindowText(WinWindowFromID(hwnd,IDST_WRITESTATUS),title);        	    
          /* Reset track percent bar to zero */
          WinSetWindowText(WinWindowFromID(hwnd,IDSR_AUDIOPERCENT),"0#0%");
          if(!bAbort)
            writeTrack(hwnd);
          else
            WinPostMsg(hwnd,WM_CLOSE,0,0);
	      return (MRESULT)TRUE;
	    }else
	      {
            //  currentTrack=1;/* CreateCover() subtracts 1 and calls the REXX script with numTrack=0 so the footer is written */
            //  createCover();
		/* Text: "CD-ROM successfully created." */
		getMessage(text, IDSTRLB_CDROMCREATIONSUCCESS, sizeof(text), RESSOURCEHANDLE, hwnd);
        writeLog(text);
        writeLog("\n");
		DosBeep(1000,100);
		DosBeep(2000,100);
		DosBeep(3000,100);
        /* Text: "CD-ROM successfully created." */
        /* Title: "Writing audio CD" */
        messageBox( text, IDSTRLB_CDROMCREATIONSUCCESS, sizeof(text),
                    title, IDSTRPM_DAOTITLE, sizeof(title),
                    RESSOURCEHANDLE, hwnd, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);            
		WinPostMsg(hwnd,WM_CLOSE,0,0);
		break;
	      }/* else 	    if(currentTrack<=numTracks) */
	  }/*	  if(LONGFROMMP(mp2)==0) */
	  else
	    {	  
    /* There was an error while writing */
	      if(currentTrack<=numTracks){
            /* There're some tracks left */
            /* text: "CDRecord/2 returned a Non-zero resultcode. The written track is probably corrupted. 
               Do you want to continue writing?"
               title:"CD writing error!"
               */
            rc=messageBox( text, IDSTR_WRITEERRORTEXT , sizeof(text),
                           title, IDSTR_WRITEERRORTITLE, sizeof(title),
                           RESSOURCEHANDLE, hwnd, MB_YESNO | MB_ICONEXCLAMATION|MB_MOVEABLE);	    
            if(rc==MBID_YES) {
              /* Continue writing */
              /* Text: "Writing track %d of %d     " */
              getMessage(text, IDSTRA_WRITEAUDIOTRACK,sizeof(text), RESSOURCEHANDLE , hwnd);
              sprintf(title,text,currentTrack, numTracks);
              WinSetWindowText(WinWindowFromID(hwnd,IDST_WRITESTATUS),title);        	    
              writeTrack(hwnd);
              return (MRESULT)TRUE;
            }
            WinPostMsg(hwnd,WM_CLOSE,0,0);	      
	      }
	      DosBeep(100,500);
          /* Text:"Error while writing audio CD!" */
          getMessage(text, IDSTRPM_DAOERROR, sizeof(text), RESSOURCEHANDLE, hwnd);
          writeLog(text);
          writeLog("\n");
          /* Text: "Error while writing audio CD!" */
          /* Title: "Writing audio CD" */
          messageBox( text, IDSTRPM_DAOERROR, sizeof(text),
                      title, IDSTRPM_DAOTITLE, sizeof(title),
                      RESSOURCEHANDLE, hwnd, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE); 
	      WinPostMsg(hwnd,WM_CLOSE,0,0);
	      break;	      
	    }
	  break;
        default:
          break;
        }/* switch */
      }/* if(thisPtr) */           
      return WinDefWindowProc( hwnd, msg, mp1, mp2);
    default:
      break;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);    
}


BOOL checkSettings() 
{
  struct stat statBuf;
  BOOL bOk=TRUE;
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];

  /* Check CDRecord/2 path */
  if(stat(chrCDRecord , &statBuf)==-1) {
    bOk=FALSE;
    /* text: "No valid CDRecord/2 path found in cdrecord.ini!"
       title: "Writing audio CD"
       */
    messageBox( text, IDSTRLOG_NOCDRECORD, sizeof(text),
                title, IDSTRPM_DAOTITLE, sizeof(title),
                RESSOURCEHANDLE, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);    
  }


  return bOk;  
}

int main (int argc, char *argv[])
{
  /*  HWND hwndGrab; */
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  SWCNTRL  swctl={0};
  PID pid;
  HWND hwndFrame;
  short a;
  char logText[CCHMAXPATH];

  /* Create a copy of the args */
  /* argv[0]: progname
   * argv[1]: installdir of Audio-CD-Creator
   * argv[2]: foldername
   * argv[3]: # tracks
   * argv[4]: size of all tracks
   */
  numArgs=argc;
  for(a=0;a<argc;a++)
    {
      params[a]=argv[a];
    }
  /* Delete logfile */
  removeLog();
 
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) { 
      writeLog("\"");
      writeLog(argv[0]);
      writeLog("\" started with the following parameters:\n");
      for(a=0;a<argc;a++)
        {
          snprintf(logText,sizeof(logText),"%d: %s\n",a,argv[a]);
          writeLog(logText);
        }
      writeLog("\n\n");

      /* Check if user started prog by hand */   
      if(argc<5)
        pmUsage();
      else {
        /* Save installation directory */
        strcpy(chrInstallDir,params[1]);
        writeLog("Starting to write audio CD\n\n");  
        /* Get our ressource dll */  
        RESSOURCEHANDLE=queryResModuleHandle();
        /* Graphics are bound to the class DLL */
        CLASSDLLHANDLE=queryClassDLLModuleHandle(argv[1]);

        numTracks=atol(argv[3]); 
        ulTotalSize=atol(argv[4]);
        /* Load options from cdrecord.ini */
        readIni();
        /* load background bitmap */
        _loadBmps(CLASSDLLHANDLE);
        if(readWindowPosFromIni(chrInstallDir, "pmwrtaud"))
          bHaveWindowPos=TRUE;
        if(checkSettings()) {
          /* Register the percent bar window class */
          percentRegisterBarClass();
          if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, audioStatusDialogProc, RESSOURCEHANDLE, IDDLG_WRITESTATUS, 0 ) == DID_ERROR )
            {
              if(pvSharedMem)
                DosFreeMem(pvSharedMem);
              DosBeep(100,600);
              errorResource2("Problem with Audio/Data-CD-Creator installation");
              freeResHandle();
              freeClassDLLHandle();
              WinDestroyMsgQueue( hmq );
              WinTerminate( hab );
              sendCommand("FREESHAREDMEM=1");
              return( 1 );
            }        
          if(pvSharedMem) {
            DosFreeMem(pvSharedMem);
          }      
          writeWindowPosToIni(chrInstallDir, "pmwrtaud");
        }/* if(checkSettings()) */
        freeResHandle();
        freeClassDLLHandle();
      }      
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  /* Remove decoded MP3 file if any */
  if(iMp3Decoder==IDKEY_USEMPG123) {
    sprintf(logText,"%s\\%s", params[2],"dec_mp3.raw");
  }
  else if(iMp3Decoder==IDKEY_USEZ){
    sprintf(logText,"%s\\%s", params[2],"dec_mp3.wav");/* z! writes real wave files */
  }
  else if(iMp3Decoder==IDKEY_USEMMIOMP3) {
    sprintf(logText,"%s\\%s", params[2],"dec_mp3.raw");
  }
  remove(logText);
  return 0;
}









