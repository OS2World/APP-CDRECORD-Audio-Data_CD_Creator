/*
 * ptrkname.c (C) Chris Wohlgemuth 1999
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
#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <string.h>
#include "progbars.h"
#include "audiofolder.h"
#include "audiofolderres.h"

void pmUsage();

//#define DEBUG

ATOM atomStartGrab;

int numArgs;
char* params[99+4];
  /* argv[0]: progname
   * argv[1]: installdir of Audio-CD-Creator
   * argv[2]: folder into which to place the waves
   * argv[3]: trackname
   * argv[4+n]: tracks to grab
   * argv[4+n+1]: discid
   */

HMODULE RESSOURCEHANDLE=0;
HMODULE CLASSDLLHANDLE=0;

char logName[CCHMAXPATH]="Grabbing.log";

int numTracks;
char chrDiscID[10];

extern LONG  lCDROptions;


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
   * argv[4+n]: tracks to grab
   * argv[4+n+1]: discid
   */
 
  sprintf(startParams," /C %s\\bin\\%s %s %02d \"%s\" %s",params[1], rexxFile, chrDiscID, iTrack, params[2], params[1]);
  startData.PgmInputs=startParams;
  
  rc=DosStartSession(&startData,&ulSessionID,&pid);   
  return rc;   
}


MRESULT EXPENTRY trackNameObjectProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  switch(msg)
    {
    case WM_APPTERMINATENOTIFY:
      if(LONGFROMMP(mp1)==ACKEY_CDDBQUERY) {
        if(LONGFROMMP(mp2)==1) {
          /* CDDB query finished */
          /* Launch the Rexxfile which puts the tracknames into the listbox. */
          launchRexx( chrDiscID, numTracks, "Fill listbox","trklistb.cmd");
          WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
        }
      }
      return FALSE;
    default:
      break;
    }
  return WinDefWindowProc( hwnd, msg, mp1, mp2);
}

int main (int argc, char *argv[])
{

  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  char text[20];
  short a;

  HWND hwndClient;

  /* Create a copy of the args */
  /* argv[0]: progname
   * argv[1]: installdir of Audio-CD-Creator
   * argv[2]: folder into which to place the waves
   */
  numArgs=argc;
  for(a=0;a<argc;a++)
    {
      params[a]=argv[a];
    }
  /* Offset */
  numTracks=numArgs-5;
  strncpy(chrDiscID,argv[argc-1],sizeof(chrDiscID));  

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
        /* Load options from cdrecord.ini */
        readIni();
        /* Object window creation */
        hwndClient=WinCreateWindow(HWND_OBJECT,WC_STATIC,"trackNameObject",0,0,0,0,0,NULLHANDLE,HWND_BOTTOM,12343,NULL,NULL);
        if(hwndClient) {
          WinSubclassWindow(hwndClient,&trackNameObjectProc);
          /* Window created. */
          sprintf(text,"%d",hwndClient);
          launchPMWrapper( params[1],text, params[2], "pmcddb.exe", "Query CDDB server");
          while(WinGetMsg(hab,&qmsg,(HWND)NULL,0,0))
            WinDispatchMsg(hab,&qmsg);
          WinDestroyWindow(hwndClient);
        }
        freeResHandle();
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}









