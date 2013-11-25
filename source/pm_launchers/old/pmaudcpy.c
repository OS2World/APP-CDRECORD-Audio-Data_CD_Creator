/*
 * pmaudcpy.c (C) Chris Wohlgemuth 2001
 *
 * This helper handles the GUI stuff for CD copying
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


//#define DEBUG

#define CDCOPY_APPKEY "cdcopy"

extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */

char chrInstallDir[CCHMAXPATH];
extern int iBus;
extern int iTarget;
extern int iLun;
extern SWP swpWindow;

BOOL bHaveWindowPos=FALSE;
char chrImagePath[CCHMAXPATH];
char chrSourceImagePath[CCHMAXPATH];
char chrDrive[4];
char chrDevice[CCHMAXPATH];
char chrSourceDevice[CCHMAXPATH];
PVOID pvScanbusSharedMem;

char  logName[CCHMAXPATH]="CD-Copy.log";

int numArgs;
char* params[4];

  /* argv[0]: progname
   * argv[1]: installdir of Audio/Data-CD-Creator
   */

long long lFreeSpace[27]= {0};

HMODULE RESSOURCEHANDLE=0;

void removeLog2(char * installDir, char * logName);
void pmUsage();
HINI HlpOpenIni();
void HlpCloseIni(HINI hini);

void showHelp(char* command)
{
  HOBJECT hObject;

  hObject=WinQueryObject("<CWCREATOR_SETTINGS>");
  if(hObject!=NULLHANDLE) {
    WinSetObjectData(hObject, command);
  }
}

ULONG launchRexxCmd( char * chrInstallDir, char* chrParams, char* pszTitle, char *rexxFile)
{
  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrLoadError[CCHMAXPATH];
  char startParams[CCHMAXPATH*4];
  char exename[CCHMAXPATH]={0};

    
  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_FORE;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;
  
  sprintf(exename,"%s","cmd.exe");
  startData.PgmName=exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_WINDOWABLEVIO;

  startData.PgmControl=SSF_CONTROL_NOAUTOCLOSE | SSF_CONTROL_VISIBLE;

#if 0
  if(lCDROptions&IDCDR_HIDEWINDOW)
    startData.PgmControl|=SSF_CONTROL_INVISIBLE;//|SSF_CONTROL_MAXIMIZE|SSF_CONTROL_NOAUTOCLOSE;
  if(!(lCDROptions&IDCDR_CLOSEWINDOW))
    startData.PgmControl|=SSF_CONTROL_NOAUTOCLOSE;
#endif
  startData.InitXPos=30;
  startData.InitYPos=30;
  startData.InitXSize=500;
  startData.InitYSize=400;
  startData.ObjectBuffer=chrLoadError;
  startData.ObjectBuffLen=(ULONG)sizeof(chrLoadError);

  snprintf(startParams, sizeof(startParams)," /C %s\\bin\\%s %s",chrInstallDir, rexxFile, chrParams);
  startData.PgmInputs=startParams;
  
  rc=DosStartSession(&startData,&ulSessionID,&pid);   
  return rc;
}

void _showImagedriveControls(HWND hwnd, BOOL bShow)
{  
  WinShowWindow(WinWindowFromID(hwnd,IDDD_IMAGEFILE), bShow);
  WinShowWindow(WinWindowFromID(hwnd,IDGB_IMAGEDRIVE), bShow);
}

void _showSourceWriterControls(HWND hwnd, BOOL bShow)
{  
  WinShowWindow(WinWindowFromID(hwnd,IDDD_SOURCEDEVICE), bShow);
  WinShowWindow(WinWindowFromID(hwnd,IDGB_DEVICESELECTION), bShow);
}

void _showTargetWriterControls(HWND hwnd, BOOL bShow)
{  
  WinShowWindow(WinWindowFromID(hwnd,IDDD_TARGETDEVICE), bShow);
  WinShowWindow(WinWindowFromID(hwnd,IDGB_TARGETDEVICE), bShow);
}

void _showTargetImageFileControls(HWND hwnd, BOOL bShow)
{
  WinShowWindow(WinWindowFromID(hwnd,IDPB_BROWSEIMAGE), bShow);
  WinShowWindow(WinWindowFromID(hwnd,IDEF_IMAGENAME), bShow);
  WinShowWindow(WinWindowFromID(hwnd,IDGB_IMAGEFILE), bShow);
}

void _showSourceImageFileControls(HWND hwnd, BOOL bShow)
{
  WinShowWindow(WinWindowFromID(hwnd,IDPB_BROWSESOURCEIMAGE), bShow);
  WinShowWindow(WinWindowFromID(hwnd,IDEF_SOURCEIMAGE), bShow);
  WinShowWindow(WinWindowFromID(hwnd,IDGB_SOURCEIMAGEFILE), bShow);
}

/* This Proc handles the on-the-fly data CD writing */
MRESULT EXPENTRY audioCopyStatusDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  char text2[CCHMAXPATH];
  SWCNTRL swctl;
  PID pid;
  int a;
  char *chrPtr;
 
  switch (msg)
    {      
    case WM_INITDLG:
      /* Allocate shared mem for communication between helper and settings notebook */
      if(!pvScanbusSharedMem) {
        if(DosAllocSharedMem(&pvScanbusSharedMem,SCANBUSSHAREDMEM_NAME, SCANBUSSHAREDMEM_SIZE,
                             PAG_READ|PAG_WRITE|PAG_COMMIT)) {
          /*
            Text:   "Can't allocate shared memory. There's probably already a creation process running."
            Title: "Audio-CD-Creator"                       
            */             
          messageBox( text, IDSTR_ALLOCSHAREDMEMERROR , sizeof(text),
                      title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                      RESSOURCEHANDLE, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        }
      }
      else
        {
          /* The shared mem is already allocated. There's another write process running. */
          /*
            Text:   "Can't allocate shared memory. There's probably already a creation process running."
            Title: "Audio-CD-Creator"                       
            */             
          messageBox( text, IDSTR_ALLOCSHAREDMEMERROR , sizeof(text),
                      title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                      RESSOURCEHANDLE, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        }
      /* Add switch entry */
      memset(&swctl,0,sizeof(swctl));
      WinQueryWindowProcess(hwnd,&pid,NULL);
      swctl.hwnd=hwnd;
      swctl.uchVisibility=SWL_VISIBLE;
      swctl.idProcess=pid;
      swctl.bProgType=PROG_DEFAULT;
      swctl.fbJump=SWL_JUMPABLE;
      WinAddSwitchEntry(&swctl);

      /* Show the controls */
      _showTargetWriterControls(hwnd, FALSE);
      _showTargetImageFileControls(hwnd, FALSE);
      _showSourceImageFileControls(hwnd, FALSE);
      _showSourceWriterControls(hwnd, TRUE);
      _showImagedriveControls(hwnd, TRUE);

      /* Not yet used */
      WinShowWindow(WinWindowFromID(hwnd,IDCB_ONTHEFLY), FALSE);

      /* Set path into target name field */
      WinSendMsg(WinWindowFromID(hwnd,IDEF_IMAGENAME),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_IMAGENAME),chrImagePath);

      /* Set path into source name field */
      WinSendMsg(WinWindowFromID(hwnd,IDEF_SOURCEIMAGE),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      if(strlen(chrSourceImagePath))
        WinSetWindowText(WinWindowFromID(hwnd,IDEF_SOURCEIMAGE),chrSourceImagePath);

      WinEnableWindow(WinWindowFromID(hwnd,IDPB_OK),FALSE);
      WinEnableWindow(WinWindowFromID(hwnd,IDPB_CANCEL),FALSE);
      WinCheckButton(hwnd,IDCB_TESTONLY,1);
      /*      sprintf(text,"1: %s, 2: %s, 3: %s",params[1],params[2],params[3]);
              WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text,
              params[3],
              0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
              */

      /* Radio button */
      WinCheckButton(hwnd,IDRB_COPYCD,1);

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

      /* Get the devices on the bus */
      launchWrapper("", params[1], hwnd,"scanbus.exe","Scanning bus...");
      /* Show the space of the harddisks */
      for(a=3;a<=26;a++)
        {
          if(lFreeSpace[a]) {
            sprintf(text,"%c:   %Ld Mbytes free",'A'+a-1 , lFreeSpace[a]/1000000);
            WinSendMsg(WinWindowFromID(hwnd,IDDD_IMAGEFILE),LM_INSERTITEM,MPFROMSHORT(LIT_END),
                       text);
            /* Set the drop down list to the last chosen drive */
            if(strlen(chrDrive)) {
              if('A'+a-1==chrDrive[0])
                WinSetWindowText(WinWindowFromID(hwnd,IDDD_IMAGEFILE),text);
            }
          }
        }
      return (MRESULT) TRUE;
    case WM_CLOSE:
      {
        HINI hIni;

        writeLog("\nDialog is closing...\n\n");
        if(pvScanbusSharedMem) {
          DosFreeMem(pvScanbusSharedMem);
          pvScanbusSharedMem=NULL;
        }
        WinQueryWindowPos(hwnd,&swpWindow);
        /* Write ini file */
        if((hIni=HlpOpenIni())!=NULLHANDLE)
          {
            char *chrPtr;

            /* Write image path to ini */
            WinQueryWindowText(WinWindowFromID(hwnd,IDEF_IMAGENAME),sizeof(chrImagePath), chrImagePath);/* image name */            
            PrfWriteProfileString(hIni,CDCOPY_APPKEY,"targetimagepath",chrImagePath);
            writeLog("Writing target path to ini: ");
            writeLog(chrImagePath);
            writeLog("\n");
            /* Write source path to ini */
            WinQueryWindowText(WinWindowFromID(hwnd,IDEF_SOURCEIMAGE),sizeof(chrSourceImagePath),
                               chrSourceImagePath);/* image name */            
            PrfWriteProfileString(hIni,CDCOPY_APPKEY,"sourceimagepath",chrSourceImagePath);
            writeLog("Writing source path to ini: ");
            writeLog(chrSourceImagePath);
            writeLog("\n");
            /* Write drive to ini */
            WinQueryWindowText(WinWindowFromID(hwnd,IDDD_IMAGEFILE),sizeof(chrDrive),
                               chrDrive);/* drive */
            chrDrive[2]=0;
            PrfWriteProfileString(hIni,CDCOPY_APPKEY,"drive",chrDrive);
            writeLog("Writing drive to ini: ");
            writeLog(chrDrive);
            writeLog("\n");
            /* Write device to ini */
            WinQueryWindowText(WinWindowFromID(hwnd,IDDD_TARGETDEVICE),sizeof(chrDevice),
                               chrDevice);/* device */
            if((chrPtr=strchr(chrDevice, ' '))!=NULLHANDLE)
              *chrPtr=0;
            PrfWriteProfileString(hIni,CDCOPY_APPKEY,"targetdevice",chrDevice);
            writeLog("Writing target device to ini: ");
            writeLog(chrDevice);
            writeLog("\n");
            /* Write device to ini */
            WinQueryWindowText(WinWindowFromID(hwnd,IDDD_SOURCEDEVICE),sizeof(chrSourceDevice),
                               chrSourceDevice);/* device */
            if((chrPtr=strchr(chrSourceDevice, ' '))!=NULLHANDLE)
              *chrPtr=0;
            PrfWriteProfileString(hIni,CDCOPY_APPKEY,"sourcedevice",chrSourceDevice);
            writeLog("Writing source device to ini: ");
            writeLog(chrSourceDevice);
            writeLog("\n");
            HlpCloseIni(hIni);
          }
        else
          writeLog("Can't open INI-file!\n");
        WinDismissDlg(hwnd,0);
        return FALSE;
      }
    case WM_CONTROL:
      /* This part handles the radio button messages */
      switch(SHORT1FROMMP(mp1))
        {
        case IDCB_ONTHEFLY:
          /*   _hideImageFileControls(hwnd, WinQueryButtonCheckstate(hwnd,IDCB_ONTHEFLY)); */
          break;
        case IDRB_COPYCD:
          _showTargetWriterControls(hwnd, FALSE);
          _showTargetImageFileControls(hwnd, FALSE);
          _showSourceImageFileControls(hwnd, FALSE);
          _showSourceWriterControls(hwnd, TRUE);
          _showImagedriveControls(hwnd, TRUE);
          break;
        case IDRB_GRABIMAGE:
          _showTargetWriterControls(hwnd, FALSE);
          _showImagedriveControls(hwnd, FALSE);
          _showSourceImageFileControls(hwnd, FALSE);
          _showSourceWriterControls(hwnd, TRUE);
          _showTargetImageFileControls(hwnd, TRUE);
          break;
        case IDRB_WRITEIMAGE:
          _showImagedriveControls(hwnd, FALSE);
          _showTargetImageFileControls(hwnd, FALSE);
          _showSourceWriterControls(hwnd, FALSE);
          _showSourceImageFileControls(hwnd, TRUE);
          _showTargetWriterControls(hwnd, TRUE);

          break;

        default:                     
          break;
        }
      
      break;
    case WM_HELP:
      showHelp("DISPLAYHELPPANEL=5200");
      break;
   case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_OK:
          /* User pressed the OK button */
          if(! WinQueryButtonCheckstate(hwnd,IDCB_ONTHEFLY)) {
            /* Not on the fly. (isn't implemented anyway) */
            if(WinQueryButtonCheckstate(hwnd,IDRB_COPYCD)) {
              char driveLetter;

              writeLog("\n\nCopy CD selected.\n");
              WinQueryWindowText(WinWindowFromID(hwnd,IDDD_IMAGEFILE),sizeof(title), title);/* Selected drive for image */
              writeLog("\nIntermediate drive is: ");
              writeLog(title);
              writeLog("\n");

              WinQueryWindowText(WinWindowFromID(hwnd,IDDD_SOURCEDEVICE),sizeof(text2), text2);/* Source device */
              writeLog("Source device is: ");
              writeLog(text2);
              writeLog("\n");

              if((chrPtr=strchr(text2,':'))!=NULL)
                *chrPtr=0;
              /* Check the space */
              driveLetter=toupper(title[0]);
              if(lFreeSpace[driveLetter-'A'+1] < 650 * 1024 * 1024) {
                sprintf(text,"%c: Only %Ld Mbytes free on the selected drive. An image usually is about 700MB in size. Do you want to proceed?",
                        driveLetter , lFreeSpace[driveLetter-'A'+1]/1000000);
                if(WinMessageBox( HWND_DESKTOP, hwnd, text,
                               "Information",
                               0UL, MB_YESNO | MB_ICONEXCLAMATION|MB_MOVEABLE )!=MBID_YES)
                  break;
              }
              snprintf(text,sizeof(text),"%s %c: \"%s\" %d", text2, title[0],
                       chrInstallDir, WinQueryButtonCheckstate(hwnd,IDCB_TESTONLY));
              writeLog("Launching 'cdcopy.cmd' with the following parameters:\n");
              writeLog(text);
              writeLog("\n\n");
              launchRexxCmd( chrInstallDir, text, "Copy CD...", "cdcopy.cmd");
            }
            else if(WinQueryButtonCheckstate(hwnd,IDRB_GRABIMAGE)) {
              /* Grab  image only */
              writeLog("\n\nGrab CD selected.\n");
              WinQueryWindowText(WinWindowFromID(hwnd,IDDD_SOURCEDEVICE),sizeof(text2), text2);/* Source device */
              writeLog("Source device is: ");
              writeLog(text2);
              writeLog("\n");

              WinQueryWindowText(WinWindowFromID(hwnd,IDEF_IMAGENAME),sizeof(title), title);/* image name */
              writeLog("\nImage name is: ");
              writeLog(title);
              writeLog("\n");

              if((chrPtr=strchr(text2,':'))!=NULL)
                *chrPtr=0;
              snprintf(text,sizeof(text),"%s  \"%s\" \"%s\" %d", text2, title,
                       chrInstallDir, WinQueryButtonCheckstate(hwnd,IDCB_TESTONLY));
              writeLog("Launching 'grabimg.cmd' with the following parameters:\n");
              writeLog(text);
              writeLog("\n\n");

              launchRexxCmd( chrInstallDir, text, "Grabbing image...", "grabimg.cmd");

              /* Target: 0,1,0
                 Drive for Image:  C:
                 Installation directory: E:\CD-BRENNER\ADC_048
                 Simulate=1
                 Driver: generic-mmc-raw
                 cdrdaoPath: E:\CD-Brenner\cdrdao-1_1_4a3\cdrdao.exe
                 */


            }
            else {
              /* Write a grabbed image */
              writeLog("\n\nWrite image selected.\n");
              WinQueryWindowText(WinWindowFromID(hwnd,IDDD_TARGETDEVICE),sizeof(text2), text2);/* Target device */
              writeLog("Target device is: ");
              writeLog(text2);
              writeLog("\n");

              WinQueryWindowText(WinWindowFromID(hwnd,IDEF_SOURCEIMAGE),sizeof(title), title);/* image name */
              writeLog("\nImage name is: ");
              writeLog(title);
              writeLog("\n");
              if((chrPtr=strchr(text2,':'))!=NULL)
                *chrPtr=0;
              snprintf(text,sizeof(text),"%s  \"%s\" \"%s\" %d", text2, title,
                       chrInstallDir, WinQueryButtonCheckstate(hwnd,IDCB_TESTONLY));
              writeLog("Launching 'writeimg.cmd' with the following parameters:\n");
              writeLog(text);
              writeLog("\n\n");
              launchRexxCmd( chrInstallDir, text, "Writing image...", "writeimg.cmd");
            }
          }
          else {
            /* On the fly */
            WinQueryWindowText(WinWindowFromID(hwnd,IDDD_SOURCEDEVICE),sizeof(title), title);
            WinQueryWindowText(WinWindowFromID(hwnd,IDDD_TARGETDEVICE),sizeof(text), text);
            if(!strncmp(title,text,8)) {
              /* Source and target device the same */ 
              /*
                Text:   "Can't allocate shared memory. There's probably already a creation process running."
                Title: "Audio-CD-Creator"                       
                */             
              messageBox( text, IDSTR_PMAUDCOPYDEVICECLASHTEXT , sizeof(text),
                          title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                          RESSOURCEHANDLE, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
              break;
            }
            else {
              WinQueryWindowText(WinWindowFromID(hwnd,IDDD_TARGETDEVICE),sizeof(text2), text2);/* Source device */
              if((chrPtr=strchr(text2,':'))!=NULL)
                *chrPtr=0;
              WinQueryWindowText(WinWindowFromID(hwnd,IDDD_SOURCEDEVICE),sizeof(title), title);/* Target device */
              if((chrPtr=strchr(title,':'))!=NULL)
                *chrPtr=0;
              
              snprintf(text,sizeof(text),"%s %s \"%s\" %d", text2, title,
                       chrInstallDir, WinQueryButtonCheckstate(hwnd,IDCB_TESTONLY));
              launchRexxCmd( chrInstallDir, text, "Copy CD...", "cdcpyfly.cmd");
            }
          }
          //WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        case IDPB_BROWSEIMAGE:
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
            //fd.pszTitle = "Wave name";
            /* Only show * files */
            if(strlen(chrImagePath))
              strncpy(fd.szFullFile, chrImagePath, CCHMAXPATH);
            else
              sprintf(fd.szFullFile,"%s","*.bin");
            
            if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
              {
                /* WinFileDlg failed */
                break;
              }
            if( fd.lReturn == DID_OK )
              {
                char driveLetter;

                strcpy(text,fd.szFullFile);
                /* Make sure the extension is *.bin */
                if((chrPtr=strrchr(text,'.'))==NULLHANDLE) {
                  strcat(text, ".bin");
                }
                else {
                  if(stricmp(chrPtr,".bin"))
                    strcat(text, ".bin");
                }
                WinSetWindowText( WinWindowFromID(hwnd,IDEF_IMAGENAME), text );
                
                /* Check the space */
                driveLetter=toupper(text[0]);
                if(lFreeSpace[driveLetter-'A'+1] < 650 * 1024 * 1024) {
                  sprintf(text,"%c: Only %Ld Mbytes free on the selected drive. An image usually is about 650MB in size. ",
                          driveLetter , lFreeSpace[driveLetter-'A'+1]/1000000);
                  WinMessageBox( HWND_DESKTOP, hwnd, text,
                                 "Information",
                                 0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
                }
              }
            break;
          }
        case IDPB_BROWSESOURCEIMAGE:
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
             //fd.pszTitle = "Wave name";
             /* Only show * files */
             if(strlen(chrSourceImagePath))
               strncpy(fd.szFullFile, chrSourceImagePath, CCHMAXPATH);
             else
               sprintf(fd.szFullFile,"%s","*.toc");
           
             if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
               {
                 /* WinFileDlg failed */
                 break;
               }
             if( fd.lReturn == DID_OK )
               {
                 WinSetWindowText( WinWindowFromID(hwnd,IDEF_SOURCEIMAGE), fd.szFullFile );
               }
             break;
           }
        case IDPB_CANCEL:
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        default:
          break;
        }
      return (MRESULT) FALSE;
    case WM_APPTERMINATENOTIFY:
      switch(LONGFROMMP(mp1)) 
        {
        case ACKEY_SCANBUS:
          /* Scanbus is done */
          /* Free shared mem if not already done */
          if(pvScanbusSharedMem) {
            DosFreeMem(pvScanbusSharedMem);
            pvScanbusSharedMem=NULL;
          }
          WinEnableWindow(WinWindowFromID(hwnd,IDPB_OK),TRUE);
          WinEnableWindow(WinWindowFromID(hwnd,IDPB_CANCEL),TRUE);
      
          break;          
        case ACKEY_LISTBOX:
          if(!LONGFROMMP(mp2)) {
            /* Delete entries in listbox */
            WinSetWindowText(WinWindowFromID(hwnd,IDDD_TARGETDEVICE),"");
            WinSendMsg(WinWindowFromID(hwnd, IDDD_TARGETDEVICE),LM_DELETEALL,MPFROMLONG(0),MPFROMLONG(0));
          }   
          else
            {
              WinSendMsg(WinWindowFromID(hwnd,IDDD_TARGETDEVICE),LM_INSERTITEM,MPFROMSHORT(LIT_END),
                         mp2);
              WinSendMsg(WinWindowFromID(hwnd,IDDD_SOURCEDEVICE),LM_INSERTITEM,MPFROMSHORT(LIT_END),
                         mp2);
              /* Write log */
              writeLog("Got the following device from scanbus: ");
              writeLog(mp2);
              writeLog("\n");


              sprintf(text ,"%d,%d,%d", iBus, iTarget, iLun);
              if(strstr((char*) PVOIDFROMMP(mp2), text))
                WinSetWindowText(WinWindowFromID(hwnd,IDDD_DEVICESELECTION),(PCSZ)PVOIDFROMMP(mp2));
              if(strlen(chrDevice)) {
                if(strstr((char*) PVOIDFROMMP(mp2), chrDevice))
                  WinSetWindowText(WinWindowFromID(hwnd,IDDD_TARGETDEVICE),(PCSZ)PVOIDFROMMP(mp2));
              }
              else 
                /* Check if this device is the default. If yes insert it into the entry field. */
                if(!strncmp(PVOIDFROMMP(mp2),text,5)) { 
                  WinSetWindowText(WinWindowFromID(hwnd,IDDD_TARGETDEVICE),(PCSZ)PVOIDFROMMP(mp2));
                }
              if(strlen(chrSourceDevice)) {
                if(strstr((char*) PVOIDFROMMP(mp2), chrSourceDevice))
                  WinSetWindowText(WinWindowFromID(hwnd,IDDD_SOURCEDEVICE),(PCSZ)PVOIDFROMMP(mp2));
              }
              else 
                /* Check if this device is the default. If yes insert it into the entry field. */
                if(!strncmp(PVOIDFROMMP(mp2),text,5)) { 
                  WinSetWindowText(WinWindowFromID(hwnd,IDDD_SOURCEDEVICE),(PCSZ)PVOIDFROMMP(mp2));
                }

            }
        default:
          break;
        }
      break;
    default:
      break;
    }/* switch */
  
  return WinDefDlgProc( hwnd, msg, mp1, mp2);
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
       title: "Data-CD-Creator"
       */
    messageBox( text, IDSTRLOG_NOCDRECORD, sizeof(text),
                title, IDSTRD_DATACDCREATOR , sizeof(title),
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
  char logText[CCHMAXPATH];
  HATOMTBL hatSystem;
  short a;
  char *ptrEnd;
  ULONG fl;
  HWND hwndClient;
  FSALLOCATE fsAlloc;
  HINI hIni;

  /* Create a copy of the args */
  /* argv[0]: progname
   */
  numArgs=argc;

  if(argc!=1)
    exit(-1);

  for(a=0;a<argc;a++)
    {
      params[a]=argv[a];
    }
  
  strcpy(chrInstallDir,params[0]);
  ptrEnd=strrchr(chrInstallDir,'\\');
  if(ptrEnd)
    *ptrEnd=0;
  ptrEnd=strrchr(chrInstallDir,'\\');
  if(ptrEnd)
    *ptrEnd=0;
  
  params[1]=chrInstallDir;
  
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {  
      /* Check if user started prog by hand */   
      if(argc!=1)
        pmUsage();
      else {
        /* Get our ressource dll */  
        RESSOURCEHANDLE=queryResModuleHandle2(chrInstallDir);     
        removeLog2(chrInstallDir, logName); 
        writeLog2(chrInstallDir, logName, "\"");
        writeLog2(chrInstallDir, logName, argv[0]);
        writeLog2(chrInstallDir, logName, "\" started with the following parameters:\n");
        for(a=0;a<argc;a++)
          {
            snprintf(logText,sizeof(logText),"%d: %s\n",a,argv[a]);
            writeLog2(chrInstallDir, logName, logText);
          }
        writeLog2(chrInstallDir, logName, "\n");
        DosError(FERR_DISABLEHARDERR);
        for(a=3;a<=26;a++)
          {
            if(DosQueryFSInfo(a, FSIL_ALLOC,&fsAlloc,sizeof(fsAlloc)))
              lFreeSpace[a]=0;
            else
              lFreeSpace[a]=fsAlloc.cUnitAvail*fsAlloc.cbSector*fsAlloc.cSectorUnit;
          }
        DosError(FERR_ENABLEHARDERR);

        /* Load grabber options from cdrecord.ini */
        readIni2(chrInstallDir);
        /* Get last window position */
        if(readWindowPosFromIni(chrInstallDir, "pmaudcpy"))
          bHaveWindowPos=TRUE;
        /* Get some ini settings */
        if((hIni=HlpOpenIni())!=NULLHANDLE)
          {
            ULONG keyLength;
            /* The path for the grabbed image if any */
            keyLength=PrfQueryProfileString(hIni, CDCOPY_APPKEY,"targetimagepath","",chrImagePath,sizeof(chrImagePath));
            if(keyLength!=1){
              writeLog("Target image path found in INI file: ");
              writeLog(chrImagePath);
              writeLog("\n");
            }
            /* The path for the image to be written if any */
            keyLength=PrfQueryProfileString(hIni, CDCOPY_APPKEY,"sourceimagepath","",chrSourceImagePath,sizeof(chrSourceImagePath));
            if(keyLength!=1){
              writeLog("Source image path found in INI file: ");
              writeLog(chrSourceImagePath);
              writeLog("\n");
            }
            /* The chosen drive if any */
            keyLength=PrfQueryProfileString(hIni, CDCOPY_APPKEY,"drive","",chrDrive,sizeof(chrDrive));
            if(keyLength!=1){
              writeLog("Drive letter found in INI file: ");
              writeLog(chrDrive);
              writeLog("\n");
            }
            /* The chosen device if any */
            keyLength=PrfQueryProfileString(hIni, CDCOPY_APPKEY,"targetdevice","",chrDevice,sizeof(chrDevice));
            if(keyLength!=1){
              writeLog("Target device found in INI file: ");
              writeLog(chrDevice);
              writeLog("\n");
            }
            /* The chosen device if any */
            keyLength=PrfQueryProfileString(hIni, CDCOPY_APPKEY,"sourcedevice","",chrSourceDevice,sizeof(chrSourceDevice));
            if(keyLength!=1){
              writeLog("Source device found in INI file: ");
              writeLog(chrSourceDevice);
              writeLog("\n");
            }

            HlpCloseIni(hIni);
          }
        if(checkSettings()) {
          if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, audioCopyStatusDialogProc, RESSOURCEHANDLE, IDDLG_CDRDAOCOPY, 0) == DID_ERROR )
            {
              if(pvScanbusSharedMem) {
                DosFreeMem(pvScanbusSharedMem);
              }      
              WinDestroyMsgQueue( hmq );
              WinTerminate( hab );
              DosBeep(100,600);
              return( 1 );
            }
          if(pvScanbusSharedMem) {
            DosFreeMem(pvScanbusSharedMem);
          }
          writeLog("\n");
          writeWindowPosToIni(chrInstallDir, "pmaudcpy");      
        }/* if(checkSettings()) */
        freeResHandle();
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}









