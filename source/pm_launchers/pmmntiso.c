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
char logName[CCHMAXPATH]="Mountiso.log";
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
MRESULT EXPENTRY mountIsoDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
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
        
        /* Fill drop down list with drive letters */
        for(i=2;i<26;i++) {
          if(!((ulDriveMap << (31-i)) >>31)) {
            char chrDrive[3]="A:";
            chrDrive[0]='A'+i;
            if(!bDone ) {
              WinSetWindowText(WinWindowFromID(hwnd, IDDD_MOUNTLETTER),chrDrive);
              bDone=TRUE;    
            }
            WinSendMsg(WinWindowFromID(hwnd, IDDD_MOUNTLETTER),LM_INSERTITEM,MPFROMSHORT(LIT_END),(MPARAM)chrDrive);
          }
        }
 
        WinSendMsg(WinWindowFromID(hwnd,IDEF_ISOIMAGEFILE),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);       
        WinSetWindowText(WinWindowFromID(hwnd, IDEF_ISOIMAGEFILE),params[3]);

        if(numArgs==5) {
          getMessage(title,IDSTR_PMMNTISOTITLE,sizeof(title), RESSOURCEHANDLE, hwnd);
          WinSetWindowText(hwnd, title);
          getMessage(text,IDSTR_PMMNTISOEXPLANATION,sizeof(text), RESSOURCEHANDLE, hwnd);
          WinSetWindowText(WinWindowFromID(hwnd, IDST_IMAGEREADY), text);
        }
        else
          DosBeep(3000,200);/* Make noise so the user knows the image is ready */

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
      sendCommand("DISPLAYHELPPANEL=5000");      
      break;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_MOUNT:
          {
            /* User pressed the Mount button */
            AEFS_ATTACH attachparms={0};
            char pszDrive[3]={0};
            HOBJECT hObject;
            attachparms.iOffset=0;
            WinQueryWindowText(WinWindowFromID(hwnd, IDDD_MOUNTLETTER),sizeof(pszDrive), pszDrive);
            WinQueryWindowText(WinWindowFromID(hwnd, IDEF_ISOIMAGEFILE),sizeof(attachparms.szBasePath),attachparms.szBasePath);   
            /* Send the attachment request to the FSD. */
            rc = DosFSAttach(
                             (PSZ) pszDrive,
                             (PSZ) AEFS_IFS_NAME,
                             &attachparms,
                             sizeof(attachparms),
                             FS_ATTACH);
            if (rc) {
              DosBeep(100,700);             
              switch(rc)
                {
                case ERROR_ISOFS_FILEOPEN:
                  sprintf(text, "Can't open ISO imagefile. Check the name and path.\n");
                  break;
                case ERROR_ISOFS_INVALIDOFFSET:
                  sprintf(text, "Invalid offset.\n");
                  break;
                case ERROR_ISOFS_WRONGJOLIETUCS:
                case ERROR_ISOFS_NOJOLIETSVD:
                  sprintf(text, "No Joliet names or unknown Joliet format.\n");
                  break;
                case ERROR_STUBFSD_DAEMON_NOT_RUNNING:
                  sprintf(text, "The ISOFS daemon is not running.\n");
                  break;
                default:
                  sprintf(text, "Make sure the drive letter isn't in use yet.\n");
                  break;
                }
              WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text,
                             "ISO image mount error",
                             0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
            }
            else
              if((hObject=WinQueryObject("<WP_DRIVES>"))!=NULLHANDLE) {
                //              WinSetObjectData(hObject, "MENUITEMSELECTED=503" );/* Refresh */
                WinOpenObject(hObject, 0, TRUE);/* 0=OPEN_DEFAULT*/
              }
            WinEnableWindow(WinWindowFromID(hwnd, IDPB_MOUNT),FALSE);

#if 0
            //            if(!WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, 1,5000))
            //  WinPostMsg(hwnd,WM_CLOSE,0,0);
            sprintf(text, "<WP_DRIVE_%c>",pszDrive[0]);
            if((hObject=WinQueryObject("<WP_DRIVE_P>"))!=NULLHANDLE) {
              WinOpenObject(hObject, 0, TRUE);/* 0=OPEN_DEFAULT*/
              DosBeep(500,500);
            }
#endif
            break;
          }
        case IDPB_OK:
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        case IDPB_BROWSEISOIMAGE:
          {
            FILEDLG fd = { 0 };
            
            /* User pressed the browse button */
            fd.cbSize = sizeof( fd );
            /* It's an centered 'Open'-dialog */
            fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
            /* Title: "Search mntisofs.exe" */
            getMessage(title,IDSTR_FDLGSEARCHISOIMAGETITLE,sizeof(title), RESSOURCEHANDLE, hwnd);
            /* Set the title of the file dialog */
            fd.pszTitle = title;
            /* Only show *.exe files */
            WinQueryWindowText(WinWindowFromID(hwnd, IDEF_ISOIMAGEFILE),sizeof(text),text);
            if(strlen(text))
              sprintf(fd.szFullFile,"%s",text);
            else
              sprintf(fd.szFullFile,"%s","*.raw");
            
            if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
              {
                /* WinFileDlg failed */
                break;
              }
            if( fd.lReturn == DID_OK )
              {
                WinSetWindowText( WinWindowFromID(hwnd,IDEF_ISOIMAGEFILE), fd.szFullFile );
              }
            break;
          }
          break;
        default:
          break;
        }
      return (MRESULT) FALSE;
#if 0
    case WM_TIMER:
      {
        HOBJECT hObject;
        
        DosBeep(500,500);
        
        if((hObject=WinQueryObject("<WP_DRIVE_P>"))!=NULLHANDLE) {
          WinOpenObject(hObject, 0, TRUE);/* 0=OPEN_DEFAULT*/
          DosBeep(5000,500);
        }
        WinPostMsg(hwnd,WM_CLOSE,0,0);
        return (MRESULT)FALSE;
      }
#endif
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
        if(readWindowPosFromIni(chrInstallDir, "pmmntiso"))
          bHaveWindowPos=TRUE;

        /* if(readIni()) {*/
        if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, mountIsoDialogProc, RESSOURCEHANDLE, IDDLG_MOUNTISOFILE, 0 ) == DID_ERROR )
          {
            DosBeep(100,600);
            errorResource2("Problem with Audio/Data-CD-Creator installation");
            WinDestroyMsgQueue( hmq );
            WinTerminate( hab );
            return( 1 );
          }        
        writeWindowPosToIni(chrInstallDir, "pmmntiso");
        freeResHandle();
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}









