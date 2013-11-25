/*
 * pacdgrb2.c (C) Chris Wohlgemuth 1999-2004
 *
 * This helper starts a grab process for an audio track
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
#define INCL_PM

#include <os2.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <string.h>
#include "progbars.h"
#include "audiofolder.h"
#include "audiofolderres.h"

void pmUsage();
BOOL percentRegisterBarClass(void);
void setupGroupBoxControl(HWND hwnd, USHORT id);
void setupStaticTextControl(HWND hwnd, USHORT id);
BOOL HlpPaintFrame(HWND hwnd, BOOL bInclBorder);
void _loadBmps(HMODULE hSettingsResource);
ULONG SysQueryFreeDriveSpace(ULONG ulDriveNum, ULONG *ulTotal, ULONG * ulFree, ULONG* ulBytesUnit);
void freeClassDLLHandle(void);
HMODULE queryClassDLLModuleHandle(char *installDir);
void errorResource2(char * chrTitle);

//#define DEBUG

extern LOADEDBITMAP allBMPs[];
extern BOOL bUseCustomPainting;
extern CONTROLINFO ciControls[];

int numArgs;
char* params[(99*2)+4+1];
  /* argv[0]: progname
   * argv[1]: installdir of Audio-CD-Creator
   * argv[2]: folder into which to place the waves
   * argv[3]: trackname
   * argv[4+n]: tracks to grab
   * argv[4+n+1]: discid
   */
ULONG ulTrackSizes[99];/* This holds the sizes of each track in secs */
HMODULE RESSOURCEHANDLE=0;
HMODULE CLASSDLLHANDLE=0;
HWND hwndGrab;
char logName[CCHMAXPATH]="pacdgrab.log";
int iTrackToGrab; /* Index of track to grab */
int numTracks;
char chrDiscID[10];
ULONG ulTotalSize=0;/* Total playtime of all tracks */
ULONG ulGrabbedSize=0;/* Already grabbed time */
int iCurrentTimeIndex;

extern char chrGrabberPath[CCHMAXPATH];
extern char chrGrabberOptions[CCHMAXPATH];
extern int bTrackNumbers;
extern char chosenCD[3];
extern LONG  lCDROptions;
extern BOOL bUseCDDB;
extern iGrabberID;
extern BOOL bUseCustomPainting;

char grabberParameter[CCHMAXPATH*2];
BOOL bGrab=TRUE;


PSZ buildGrabberParam(int numTrack)
{
  char *charPtr;
  char *charPtr2;
  char tempText[CCHMAXPATH];
    
  /* Build commandline */
  charPtr=strchr(chrGrabberOptions,'%');
  if(charPtr) {
    /* There is a var */
    *charPtr=0;/* String splitten */
    sprintf(grabberParameter,"%s",chrGrabberOptions);
        
    *charPtr='%';
    charPtr++;/* Parameter */ 
    if(*charPtr=='1') {
      /* replace var with cd-drive letter */
      sprintf(tempText,"%c",chosenCD[0]);
      strcat(grabberParameter,tempText);
    }
    else {
      /* replace var with tracknum */
      sprintf(tempText,"%d",numTrack);
      strcat(grabberParameter,tempText);
    }
    /* check rest of line */
    charPtr++;
    /* find second var */
    charPtr2=strchr(charPtr,'%');
    if(!charPtr2) {
      /* No second parameter */
      strcat(grabberParameter,charPtr);
    }
    else {
      *charPtr2=0;/* String splitten */
      strcat(grabberParameter,charPtr);
      *charPtr2='%';
      charPtr2++;
      if(*charPtr2=='1') {
        /* replace var with cd-drive letter */
        sprintf(tempText,"%c",chosenCD[0]);
        strcat(grabberParameter,tempText);
      }
      else {
        /* replace var with tracknum */
        sprintf(tempText,"%d",numTrack);
        strcat(grabberParameter,tempText);
      }
      charPtr2++;
      strcat(grabberParameter,charPtr2);
    }
  }
  /* commandline is ready */                      
  return grabberParameter;   
}

ULONG launchGrabber(HWND hwnd, int iTrack,PSZ grabberParms, PSZ pszTitle)
{
  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrLoadError[CCHMAXPATH];
  char startParams[CCHMAXPATH*4];
  char exename[CCHMAXPATH]={0};
  char tempText[CCHMAXPATH]= {0};
  char *charPtr;
  char trackname[CCHMAXPATH];
    
  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_BACK;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;
  
  sprintf(exename,"%s\\bin\\%s",params[1],"wrapper.exe");
  startData.PgmName=exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_WINDOWABLEVIO;
  if(lCDROptions&IDCDR_HIDEWINDOW)
    startData.PgmControl|=SSF_CONTROL_INVISIBLE;//|SSF_CONTROL_MAXIMIZE|SSF_CONTROL_NOAUTOCLOSE;
  if(!(lCDROptions&IDCDR_CLOSEWINDOW))
    startData.PgmControl|=SSF_CONTROL_NOAUTOCLOSE;
  startData.InitXPos=30;
  startData.InitYPos=30;
  startData.InitXSize=500;
  startData.InitYSize=400;
  startData.ObjectBuffer=chrLoadError;
  startData.ObjectBuffLen=(ULONG)sizeof(chrLoadError);

  /* argv[0]: progname
   * argv[1]: installdir of Audio-CD-Creator
   * argv[2]: folder into which to place the waves
   * argv[3]: trackname
   * argv[4+n-1]: tracks to grab
   * argv[4+n]: discid
   * argv[4+n+n]: tracksizes
   */

  /* build filename */
  sprintf(trackname,params[3]);
  /* check trackname for extension */
  if(bTrackNumbers) {
  /* Find last dot */                 
    charPtr=strrchr(trackname,'.');
    if(charPtr) {
      /* We have an extension */
      *charPtr=0;     /* split string */
      charPtr++;
      sprintf(tempText,"%s%02d.%s",trackname,iTrack,charPtr);/* insert tracknum */
    }
    else {
      /* No extension */                      
      sprintf(tempText,"%s%02d",trackname,iTrack);/* append trackname */
    }
  }
  else {
    /* No tracknumbers */   
    sprintf(tempText,"%s",trackname);/* append trackname */
  }
 
  sprintf(startParams,"%d \"%s\" \"%s\" \"%s\" \"%s\\%s\" %d",hwnd,chrGrabberPath,params[2],grabberParms,params[2],tempText, iGrabberID);
  startData.PgmInputs=startParams;
  
  rc=DosStartSession(&startData,&ulSessionID,&pid);   
  return rc;   
}


ULONG launchRexx( PSZ chrDiscID ,int iTrack, PSZ pszTitle, PSZ rexxFile)
{
  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrLoadError[CCHMAXPATH];
  char startParams[CCHMAXPATH*4];
  char exename[CCHMAXPATH]={0};
  char tempText[CCHMAXPATH]= {0};
  char *charPtr;
  char trackname[CCHMAXPATH];
    
  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_BACK;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;
  
  sprintf(exename,"%s","cmd.exe");
  startData.PgmName=exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_WINDOWABLEVIO;
  if(lCDROptions&IDCDR_HIDEWINDOW)
    startData.PgmControl|=SSF_CONTROL_INVISIBLE;//|SSF_CONTROL_MAXIMIZE|SSF_CONTROL_NOAUTOCLOSE;
  if(!(lCDROptions&IDCDR_CLOSEWINDOW))
    startData.PgmControl|=SSF_CONTROL_NOAUTOCLOSE;
  startData.InitXPos=30;
  startData.InitYPos=30;
  startData.InitXSize=500;
  startData.InitYSize=400;
  startData.ObjectBuffer=chrLoadError;
  startData.ObjectBuffLen=(ULONG)sizeof(chrLoadError);

  /* argv[0]: progname
   * argv[1]: installdir of Audio-CD-Creator
   * argv[2]: folder into which to place the waves
   * argv[3]: trackname
   * argv[4+n-1]: tracks to grab
   * argv[4+n]: discid
   * argv[4+n+n]: tracksizes
   */

  /* build filename to rename */
  sprintf(trackname,params[3]);
  /* check trackname for extension */
  if(bTrackNumbers) {
  /* Find last dot */                 
    charPtr=strrchr(trackname,'.');
    if(charPtr) {
      /* We have an extension */
      *charPtr=0;     /* split string */
      charPtr++;
      sprintf(tempText,"%s%02d.%s",trackname, iTrack, charPtr);/* insert tracknum */
    }
    else {
      /* No extension */                      
      sprintf(tempText,"%s%02d",trackname,iTrack);/* append trackname */
    }
  }
  else {
    /* No tracknumbers */   
    sprintf(tempText,"%s",trackname);/* append trackname */
  }
 
  sprintf(startParams," /C %s\\bin\\%s %s %02d \"%s\\%s\" %s",params[1],rexxFile, chrDiscID, iTrack, params[2],tempText, params[1]);
  startData.PgmInputs=startParams;

  rc=DosStartSession(&startData,&ulSessionID,&pid);   
  return rc;   
}

 
MRESULT EXPENTRY grabStatusDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  struct stat statBuf;
  char text[CCHMAXPATH];
  char statusText[CCHMAXPATH];
  char* charPtr;
  int iPercent;

#ifdef DEBUG
  short a;
#endif
  ULONG rc;

  switch(msg)
    {
    case WM_PAINT:
      {
        if(HlpPaintFrame(hwnd, TRUE))
          return (MRESULT)0;
        break;
      }
    case WM_APPTERMINATENOTIFY:
      switch(LONGFROMMP(mp1)) {
      case ACKEY_PERCENTGRABBED:
        
        if(mp2 != 0){/* Prevents flickering */

          iPercent=LONGFROMMP(mp2);
          if(iPercent>100)
            iPercent=100;
          if(iPercent<0)
            iPercent=0;

          /* Update track percent bar value. The helper prog sends us the actual grabbed percent. */         
          sprintf(text,"%d#%d%%", iPercent, iPercent);
          WinSetWindowText(WinWindowFromID(hwnd,IDSR_AUDIOPERCENT), text);

          iPercent=1+(ulGrabbedSize+(ulTrackSizes[iCurrentTimeIndex]*LONGFROMMP(mp2))/100)*100/ulTotalSize;
          if(iPercent>100)
            iPercent=100;
          if(iPercent<0)
            iPercent=0;

          sprintf(text,"%d#%d%%", iPercent, iPercent);
          WinSetWindowText(WinWindowFromID(hwnd,IDSR_AUDIOPERCENTALL),text);
        }
        break;
      case ACKEY_CDDBQUERY: 
        {
          /* We get another message after the query finished with mp2=1 so be sure only to react on the first one */
          if(LONGFROMMP(mp2)==0) {
            /* Start grabber for first track */
            /* Text: "Grabbing track %d of %d     " */
            getMessage(text, IDSTRA_GRABAUDIOTRACK,sizeof(text), RESSOURCEHANDLE, hwnd);
            sprintf(statusText,text, iTrackToGrab-3,numTracks);
            
            rc=launchGrabber(hwnd, atoi(params[iTrackToGrab]),buildGrabberParam(atoi(params[iTrackToGrab])),statusText);
            break;
          }
          break;
        }
      default:
        /* The track is grabbed */
       
        if(bUseCDDB) {
          /* Put cddb data into EA */
          sprintf(text,"%s\\cddbdata\\%s",params[1],chrDiscID);
          if(stat(text , &statBuf)!=-1) {
            /* Launch the Rexxfile which renames the track. */
            launchRexx( chrDiscID, atoi(params[iTrackToGrab]), "Rename track","renwave.cmd");
            /* Skript will refresh the folder after renaming of the wave */
          }
          else
            /* Send refresh command to folder */
            sendCommand("MENUITEMSELECTED=503");
        }
        else
          /* Send refresh command to folder */
          sendCommand("MENUITEMSELECTED=503");
        
        /* Check if everything went well */
        if(LONGFROMMP(mp2)!=0) {
          /* text: 
             statusText: "Audio-CD-Creator"
             */
          if(messageBox( text, IDSTR_GRABBERERRORTEXT , sizeof(text),
                         statusText, IDSTR_AUDIOCDCREATOR, sizeof(statusText),
                         RESSOURCEHANDLE, HWND_DESKTOP, MB_YESNO | MB_ICONEXCLAMATION|MB_MOVEABLE)==MBID_NO) {    
            WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
            break;
          }
        }
        
        
#ifdef DEBUG
        sprintf(text,"iTrackToGrab: %d numTracks: %d",iTrackToGrab, numTracks);  
        WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, text, "Debug", 0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
#endif
        
        if(iTrackToGrab < numTracks+3) {
          if(!bGrab) {
            WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
            return 0;
          }
          iTrackToGrab++;
          ulGrabbedSize+=ulTrackSizes[iCurrentTimeIndex++];

          //          iCurrentTimeIndex++;

          /* Text: "Grabbing track %d of %d     " */
          getMessage(text, IDSTRA_GRABAUDIOTRACK,sizeof(text), RESSOURCEHANDLE, hwnd);
          sprintf(statusText,text,iTrackToGrab-3, numTracks);
          WinSetWindowText(WinWindowFromID(hwnd,IDST_GRABSTATUS),statusText);
          rc=launchGrabber(hwnd, atoi(params[iTrackToGrab]),buildGrabberParam(atoi(params[iTrackToGrab])),statusText);      
        }
        else
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);    
        break;
      }
      return 0;
    case WM_INITDLG:   
      {
#ifdef DEBUG
        int a;
        char pszTemp[10];
        sprintf(text,"Params:\n");  
        for(a=0;a<numArgs;a++) {
          strcat(text,params[a]);
          strcat(text,"\n");
        }
        sprintf(pszTemp, "%d", ulTotalSize);
        strcat(text, pszTemp);
        WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, text, "Debug", 0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
        //      WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
#endif

        /* Start grabber for first track */
        /* Text: "Grabbing track %d of %d     " */
        getMessage(text, IDSTRA_GRABAUDIOTRACK,sizeof(text), RESSOURCEHANDLE, hwnd);
        sprintf(statusText,text, iTrackToGrab-3, numTracks);
        WinSetWindowText(WinWindowFromID(hwnd,IDST_GRABSTATUS),statusText);
        /* Set dialog font to WarpSans for Warp 4 and above */
        if(cwQueryOSRelease()>=40) {
          WinSetPresParam(hwnd,
                          PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                          DEFAULT_DIALOG_FONT );
        }
        
        /* Set percent bars to 0. */
        WinSetWindowText(WinWindowFromID(hwnd,IDSR_AUDIOPERCENT),"0#0%");
        WinSetWindowText(WinWindowFromID(hwnd,IDSR_AUDIOPERCENTALL),"0#0%");
        /* Custom painting */
        setupGroupBoxControl(hwnd, IDGB_GRABSTATUS);
        setupStaticTextControl(hwnd, IDST_GRABSTATUS);
        
        sprintf(text,"%d",hwnd);
        if(bUseCDDB)
          launchPMWrapper( params[1],text, params[2], "pmcddb.exe", "Query CDDB server");
        else
          WinPostMsg(hwnd,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_CDDBQUERY),MPFROMLONG(0));           
        
        return (MRESULT) TRUE;
      }
    case WM_HELP:
      sprintf(text,"DISPLAYHELPPANEL=%d",IDDLG_GRABSTATUS);
      sendCommand(text);
        /*      thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
                thisPtr->wpDisplayHelp(IDDLG_GRABSTATUS, AFHELPLIBRARY);
                */    
      break;
    case WM_CLOSE:
      /* We are currently grabbing, so hide the window */
      WinShowWindow(hwnd,FALSE);
      /*   else*/
      //WinPostMsg(hwnd,WM_QUIT,0,0);
      return 0;
    case WM_DESTROY:
      /* The dialog closes and gets destroyed */     
      break;    
    case WM_COMMAND:    
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_BREAK:
          DosBeep(1000,100);
          /* thisPtr=(CWAudioFolder*) WinQueryWindowULong(hwnd,QWL_USER);
          thisPtr->cwEnableGrab(FALSE);*/
          bGrab=FALSE;
          WinEnableWindow(WinWindowFromID(hwnd,IDPB_BREAK),FALSE);
          break;
        case IDPB_ABORT:
          DosBeep(5000,300);
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

BOOL checkSettings() 
{
  struct stat statBuf;
  BOOL bOk=TRUE;
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];

  /* Check grabber path */
  if(stat(chrGrabberPath , &statBuf)==-1) {
    bOk=FALSE;
    /* text: "No valid grabber path found in cdrecord.ini!"
       title: "Audio-CD-Creator"
       */
    messageBox( text, IDSTRLOG_NOGRABBER , sizeof(text),
                title, IDSTR_AUDIOCDCREATOR, sizeof(title),
                RESSOURCEHANDLE, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);    
  }
  return bOk;  
}



BOOL checkFreeFileSpace()
{
  ULONG ulTotal , ulFree , ulBytesUnit;
  char text[1000];
  long long llFree;

  if(SysQueryFreeDriveSpace(toupper(params[2][0])-'A'+1, &ulTotal, &ulFree, &ulBytesUnit)!=NO_ERROR)
    return FALSE;

  sprintf(text, "%d %d %d = %d", ulTotal, ulFree, ulBytesUnit, ulBytesUnit*ulFree);
  //  WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, text, "Debug", 0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
  llFree=ulBytesUnit*ulFree/1024;
  if(ulTotalSize*44100  / 256 > llFree) {
    char text[250];
    char statusText[250];
    // char title[250];

    messageBox( text, IDSTR_NOGRABSPACETEXT , sizeof(text),
                statusText, IDSTR_AUDIOCDCREATOR, sizeof(statusText),
                RESSOURCEHANDLE, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);

#if 0
      /* Title: "Audio-CD-Creator" */
      getMessage(text, IDSTR_AUDIOCDCREATOR, sizeof(title), RESSOURCEHANDLE, HWND_DESKTOP);

      WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, 
                      "Not enough free space", title, 0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );

#endif
    return FALSE;
  }
  return TRUE;
}

int main (int argc, char *argv[])
{
  /*  HWND hwndGrab; */
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;

  HATOMTBL hatSystem;
  short a,b;

  ULONG fl;
  HWND hwndClient;

  /* Create a copy of the args */
  /* argv[0]: progname
   * argv[1]: installdir of Audio-CD-Creator
   * argv[2]: folder into which to place the waves
   * argv[3]: trackname
   * argv[4+n-1]: tracks to grab
   * argv[4+n]: discid
   * argv[4+n+n]: Tracksizes
   */
  numArgs=argc;

  for(a=0;a<argc;a++)
    {
      params[a]=argv[a];
    }

  /* Offset */
  iTrackToGrab=4;
  numTracks=(numArgs-5)/2;
  strncpy(chrDiscID, argv[numTracks+4],sizeof(chrDiscID));  
  iCurrentTimeIndex=0;

  /* Calculate total playtime */
  for(a=numTracks+5,b=0; a<numTracks+5+numTracks; a++,b++)
    {
      ulTotalSize+=atoi(argv[a]);
      ulTrackSizes[b]=atoi(argv[a]);
    }

  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {  
      /* Check if user started prog by hand */   
      if(argc<5)
        pmUsage();
      else {
        removeLog();
        /* Get our ressource dll */  
        RESSOURCEHANDLE=queryResModuleHandle();
        /* Graphics are bound to the class DLL */
        CLASSDLLHANDLE=queryClassDLLModuleHandle(argv[1]);
        
        /* Load grabber options from cdrecord.ini */
        readIni();
        if(checkFreeFileSpace()) {
          /* load background bitmap */
          _loadBmps(CLASSDLLHANDLE);
          if(checkSettings()) {
            /* Register the percent bar window class */
            percentRegisterBarClass();
            if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, grabStatusDialogProc, RESSOURCEHANDLE, IDDLG_GRABSTATUS2 , 0 )
                == DID_ERROR )
              {
                DosBeep(100,600);
                errorResource2("Problem with Audio/Data-CD-Creator installation");
                freeClassDLLHandle();
                WinDestroyMsgQueue( hmq );
                WinTerminate( hab );
                /*     if(hini)PrfCloseProfile(hini);*/
                return( 1 );
              }      
          }/* if(checkSettings()) */
        }
        freeResHandle();
        freeClassDLLHandle();
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}









