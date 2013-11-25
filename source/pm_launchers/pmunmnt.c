/*
 * pmmntiso.c (C) Chris Wohlgemuth 2001
 *
 * This helper handles the GUI stuff for DAO CD creation
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
#include "audiofolder.h"
#include "audiofolderres.h"
#include "aefsdint.h"

void pmUsage();
void errorResource2(char * chrTitle);

//#define DEBUG

extern char chrMntIsoFS[CCHMAXPATH];

extern SWP swpWindow;

BOOL bHaveWindowPos=FALSE;
char chrInstallDir[CCHMAXPATH];
char logName[CCHMAXPATH]="UnmntISO.log";
ULONG ulDriveMap=0;
ULONG ulDriveNum=0;

int numArgs;
char* params[4];
/* params[0]: progname
 * params[1]: installdir of Audio-CD-Creator
 * params[2]: foldername
 * params[3]: image name
 */


HMODULE RESSOURCEHANDLE=0;
HMODULE CLASSDLLHANDLE=0;

void sendCommand(PSZ command);
void removeLog(void);
void writeLog(char* logText);


/* This Proc handles the ISO image mounting */
MRESULT EXPENTRY unmountIsoDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];

  ULONG rc;
  SWCNTRL swctl;
  PID pid;

  switch (msg)
    {      
    case WM_INITDLG:
      {
        BOOL bDone=FALSE;
        int i;

        writeLog("Initializing dialog...\n");  
        
        /* Add switch entry */
        memset(&swctl,0,sizeof(swctl));
        WinQueryWindowProcess(hwnd,&pid,NULL);
        swctl.hwnd=hwnd;
        swctl.uchVisibility=SWL_VISIBLE;
        swctl.idProcess=pid;
        swctl.bProgType=PROG_DEFAULT;
        swctl.fbJump=SWL_JUMPABLE;
        WinAddSwitchEntry(&swctl);
        
        /*sprintf(text,"%d",params[4]);*/ 
        // sprintf(text,"params[1]: %s ",params[1]);
        /* WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, pvSharedMem,
           params[4],
           0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
           WinPostMsg(hwnd,WM_CLOSE,0,0);
           return (MRESULT) TRUE;
           */
        
        /* Get free drive letters */
        if((rc=DosQueryCurrentDisk(&ulDriveNum, &ulDriveMap))!=NO_ERROR)
          WinPostMsg(hwnd,WM_CLOSE,0,0);

        DosError(FERR_DISABLEHARDERR);

        for(i=2;i<26;i++) {
          if(( (ulDriveMap << (31-i)) >>31)) {
            char chrDrive[3]="A:";
            BYTE fsqBuf2[sizeof(FSQBUFFER2)+3*CCHMAXPATH]={0};
            PFSQBUFFER2 pfsqBuf2=(PFSQBUFFER2) &fsqBuf2;
            ULONG ulLength;

            /* Get FS */
            chrDrive[0]='A'+i;
            ulLength=sizeof(fsqBuf2);
            if(DosQueryFSAttach(chrDrive,0L,FSAIL_QUERYNAME, (PFSQBUFFER2)&fsqBuf2, &ulLength)==NO_ERROR) {
              if(!strcmp(pfsqBuf2->szName+pfsqBuf2->cbName+1,"ISOFS")) {
                FSINFO fsInfo;

                if(DosQueryFSInfo(i+1, FSIL_VOLSER, &fsInfo,sizeof(fsInfo))==NO_ERROR)
                  sprintf(text, "%s      (%s)",chrDrive,  fsInfo.vol.szVolLabel); 
                else
                  sprintf(text, "%s      (unknown)",chrDrive); 
                WinSendMsg(WinWindowFromID(hwnd, IDLB_UNMOUNTLETTER),LM_INSERTITEM,MPFROMSHORT(LIT_END),MPFROMP(text));
              }
            }
            else
              printf("%s %s\n",chrDrive, "---"); 
          }
        }
        DosError(FERR_ENABLEHARDERR);

        /* Set dialog font to WarpSans for Warp 4 and above */
        if(cwQueryOSRelease()>=40) {
          WinSetPresParam(hwnd,
                          PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                          DEFAULT_DIALOG_FONT );
        }
        
        if(!bHaveWindowPos)
          WinSetWindowPos(hwnd,HWND_TOP,0,0,0,0,SWP_ZORDER|SWP_ACTIVATE);
        else
          WinSetWindowPos(hwnd,HWND_TOP,swpWindow.x, swpWindow.y, 0, 0, SWP_MOVE|SWP_ZORDER|SWP_ACTIVATE|SWP_SHOW);
        
        return (MRESULT) TRUE;
      }
    case WM_CLOSE:
      WinQueryWindowPos(hwnd,&swpWindow);
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_HELP:
      sendCommand("DISPLAYHELPPANEL=5100");      
      break;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_UNMOUNT:
          {
            /* User pressed the Unount button */
            AEFS_DETACH detachparms={0};
            char pszDrive[3]={0};
            HOBJECT hObject;
            SHORT sSelected;
            memset(&detachparms, 0, sizeof(detachparms));

            /* Get the drive letter */
            sSelected=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDLB_UNMOUNTLETTER),LM_QUERYSELECTION,
                                             MPFROMSHORT(LIT_FIRST),MPFROMLONG(0L)));
            if(sSelected==LIT_NONE)
              break;

            WinSendMsg(WinWindowFromID(hwnd, IDLB_UNMOUNTLETTER),LM_QUERYITEMTEXT,
                       MPFROM2SHORT(sSelected,2),MPFROMP(pszDrive));

            /* Send the attachment request to the FSD. */
            rc = DosFSAttach(
                             //                             (PSZ) "",
                             (PSZ) pszDrive,
                             (PSZ) AEFS_IFS_NAME,
                             &detachparms,
                             sizeof(detachparms),
                             FS_DETACH);
            if (rc) {
              DosBeep(100,400);             
                            
              sprintf(text, "Error while unmounting rc=%d. Make sure there're no open files on the drive.\n", rc);
              WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text,
                             "ISO image unmount error",
                             0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
            }else {
              WinSendMsg(WinWindowFromID(hwnd, IDLB_UNMOUNTLETTER),LM_DELETEITEM,
                         MPFROMSHORT(sSelected),MPFROMLONG(0L));
              sSelected=SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwnd, IDLB_UNMOUNTLETTER),LM_QUERYITEMCOUNT,
                                                MPFROMLONG(0L),MPFROMLONG(0L)));
              if(sSelected==0)
                WinEnableWindow(WinWindowFromID(hwnd,IDPB_UNMOUNT), FALSE);
            }

            break;
          }
        case IDPB_UNMOUNTCLOSE:
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        default:
          break;
        }
      return (MRESULT) FALSE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);    
}

int main (int argc, char *argv[])
{
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
   * argv[3]: imagename
   * argv[4]: if 1, a generic mount is requested. Change the title of the dialog and the text.
   */
  numArgs=argc;
  for(a=0;a<argc;a++)
    {
      params[a]=argv[a];
    }
  
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {  
      /* Check if user started prog by hand */   
      if(argc<4)
        pmUsage();
      else {
        /* Save installation directory */
        strcpy(chrInstallDir,params[1]);
        /* Delete logfile */
        removeLog();
        writeLog("\"");
        writeLog(argv[0]);
        writeLog("\" started with the following parameters:\n");
        for(a=0;a<argc;a++)
          {
            snprintf(logText,sizeof(logText),"%d: %s\n",a,argv[a]);
            writeLog(logText);
          }
        writeLog("\n");
        /* Get our ressource dll */  
        RESSOURCEHANDLE=queryResModuleHandle();
        /* Load options from cdrecord.ini */
        readIni();
        if(readWindowPosFromIni(chrInstallDir, "pmunmnt"))
          bHaveWindowPos=TRUE;

        /* if(readIni()) {*/
        if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, unmountIsoDialogProc, RESSOURCEHANDLE, IDDLG_UNMOUNT, 0 ) == DID_ERROR )
          {
            DosBeep(100,600);
            errorResource2("Problem with Audio/Data-CD-Creator installation");
            WinDestroyMsgQueue( hmq );
            WinTerminate( hab );
            return( 1 );
          }        
        writeWindowPosToIni(chrInstallDir, "pmunmnt");
        freeResHandle();
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}









