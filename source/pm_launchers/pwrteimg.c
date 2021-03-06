/*
 * pmwrtimg.cpp (C) Chris Wohlgemuth 1999-2004
 *
 * This helper handles the GUI stuff for image writing
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
#include <stdlib.h>
#include "progbars.h"
#include "audiofolder.h"
#include "audiofolderres.h"


//#define DEBUG
extern char chrMkisofs[CCHMAXPATH];/* Path to mkisofs */
extern char chrMkisofsOptions[CCHMAXPATH];

extern char chrCDROptions[CCHMAXPATH];
extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern LONG  lCDROptions;
extern LONG lMKOptions;
extern char chrInstallDir[CCHMAXPATH];
extern int iSpeed;
extern int iBus;
extern int iTarget;
extern int iLun;
extern SWP swpWindow;
extern BOOL bUseCustomPainting;

BOOL bHaveWindowPos=FALSE;

char   chrDev[20];
char * pipePtr;
char  logName[CCHMAXPATH]="WriteImg.log";
int numArgs;
char *ptrLocalMem;

PVOID pvScanbusSharedMem;

char* params[10];
  /* argv[0]: progname
   * argv[1]: installdir of Audio/Data-CD-Creator
   * argv[2]: folder
   * argv[3]: imagename
   * argv[4]: Parameter file
   */

HMODULE RESSOURCEHANDLE=0;
HMODULE CLASSDLLHANDLE=0;

void sendCommand(PSZ command);
void removeLog();
void pmUsage();
BOOL buildLogName( char * outBuf, char * logname,ULONG ulSize);
BOOL queryFreeCDSpace2(HWND hwnd, char * chrDev);
BOOL percentRegisterBarClass(void);
void setupGroupBoxControl(HWND hwnd, USHORT id);
void setupStaticTextControl(HWND hwnd, USHORT id);
BOOL HlpPaintFrame(HWND hwnd, BOOL bInclBorder);
void _loadBmps(HMODULE hSettingsResource);
void freeClassDLLHandle(void);
HMODULE queryClassDLLModuleHandle(char *installDir);
void errorResource2(char * chrTitle);


/* This Proc handles the on-the-fly data CD writing */
MRESULT EXPENTRY writeImageStatusDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH*2+10];
  char title[CCHMAXPATH];
  char *textPtr;
  char *textPtr2; 
  static LONG lCDSize;
  static LONG lImageSize;
  struct stat buf;
  SWCNTRL swctl;
  PID pid;

  switch (msg)
    {      
    case WM_PAINT:
      {
        if(HlpPaintFrame(hwnd, TRUE))
          return (MRESULT)0;
        break;
      }
    case WM_INITDLG:   
      /* Text: "Writing image." */
      getMessage(text, IDSTRD_WRITEIMAGETITLE, sizeof(text), RESSOURCEHANDLE , hwnd);
      WinSetWindowText(hwnd,text);

      /* Add switch entry */
      memset(&swctl,0,sizeof(swctl));
      WinQueryWindowProcess(hwnd,&pid,NULL);
      swctl.hwnd=hwnd;
      swctl.uchVisibility=SWL_VISIBLE;
      swctl.idProcess=pid;
      swctl.bProgType=PROG_DEFAULT;
      swctl.fbJump=SWL_JUMPABLE;
      WinAddSwitchEntry(&swctl);


      WinShowWindow(WinWindowFromID(hwnd,IDPB_STATUSOK),FALSE);
      WinShowWindow(WinWindowFromID(hwnd,IDPB_SHOWLOG),FALSE);      
      WinShowWindow(WinWindowFromID(hwnd,IDPB_ABORTWRITE), TRUE);
      /* Hide percent bar which shows the write progress */
      WinShowWindow(WinWindowFromID(hwnd,IDSR_PERCENT), FALSE);

      /*      sprintf(text,"1: %s, 2: %s, 3: %s",params[1],params[2],params[3]);
              WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text,
              params[3],
              0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
              */

      /* Set percent bar to 0. */
      WinSetWindowText(WinWindowFromID(hwnd,IDSR_PERCENT),"0#0%");

      /* Title for dialog box */
      /* Text: "Writing image %s..." */
      getMessage(text, IDSTRD_WRITINGIMAGE, sizeof(text), RESSOURCEHANDLE , hwnd);
      sprintf(title,text,params[3]);
      WinSetWindowText(WinWindowFromID(hwnd,IDST_ACTIONTEXT),
                       title);

      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }

      /* Custom painting */
      setupGroupBoxControl(hwnd, IDGB_CHECKSTATUS);
      setupStaticTextControl(hwnd, IDST_ACTIONTEXT);

      if(!bHaveWindowPos)      
        WinSetWindowPos(hwnd,HWND_TOP,0,0,0,0,SWP_ZORDER|SWP_ACTIVATE|SWP_SHOW);
      else
        WinSetWindowPos(hwnd,HWND_TOP,swpWindow.x, swpWindow.y, 0, 0, SWP_MOVE|SWP_ZORDER|SWP_ACTIVATE|SWP_SHOW);

      /* Get writer device from parameter memory */
      if((textPtr=strstr(ptrLocalMem, "dev="))!=NULL)
        if((textPtr2=strchr(textPtr, ' '))!=NULL)
          *textPtr2=0;
      if(queryFreeCDSpace2(hwnd, textPtr))
        WinPostMsg(hwnd,WM_CLOSE,0,0);
      if(textPtr2)
        *textPtr2=' ';
      return (MRESULT) TRUE;
    case WM_CLOSE:
      WinShowWindow(WinWindowFromID(hwnd,IDPB_ABORTWRITE),FALSE);
      WinShowWindow(WinWindowFromID(hwnd,IDPB_STATUSOK),TRUE);
      WinShowWindow(WinWindowFromID(hwnd,IDPB_SHOWLOG),TRUE);      
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_ABORTWRITE:
          /* User pressed the ABORT button */
          DosBeep(1000,200);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        case IDPB_STATUSOK:
          WinQueryWindowPos(hwnd,&swpWindow);
          WinDismissDlg(hwnd,0);
          break;
        case IDPB_SHOWLOG:
          showLogFile();
          break;
        default:
          break;
        }
    case WM_APPTERMINATENOTIFY:
      if(1/*thisPtr*/) {
        switch(LONGFROMMP(mp1)) {
        case ACKEY_FREESHAREDMEM:
          break;
        case ACKEY_FIXATING:
          /* This msg. is sent by the helper process when cdrecord begins with fixating the disk */
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_DELETEITEM,MPFROMSHORT(2),0);
          if(LONGFROMMP(mp2)==0)
            /* Text: "Fixating... (may need some minutes)" */
            getMessage(text, IDSTRLB_FIXATING, sizeof(text), RESSOURCEHANDLE, hwnd);
          else
            /* Text: "Writing buffers to CD..." */
            getMessage(text, IDSTRLB_WRITINGBUFFERS, sizeof(text), RESSOURCEHANDLE, hwnd);
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_INSERTITEM,MPFROMSHORT(2),text);
          break;
        case ACKEY_MBWRITTEN:
          {
            int iPercent;
            
            iPercent=LONGFROMMP(mp2);
            iPercent*=100;
            iPercent/=(lImageSize/1024/1024);

            if(iPercent>100)
              iPercent=100;
            if(iPercent<0)
              iPercent=0;
            
            /* Update percent bar value. The helper prog sends us the actual written Mbytes. */
            sprintf(text,"%d#%d%%", iPercent, iPercent);
            WinSetWindowText(WinWindowFromID(hwnd,IDSR_PERCENT), text);
          break;
          }      
        case ACKEY_WRITEONLY:
          /* Writing done. */
          if(!(LONGFROMMP(mp2))) {
            WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_DELETEITEM,MPFROMSHORT(2),0);
            /* Text: "CD-ROM successfully created." */
            getMessage(text, IDSTRLB_CDROMCREATIONSUCCESS, sizeof(text), RESSOURCEHANDLE, hwnd);
            writeLog(text);
            writeLog("\n");

            DosBeep(1000,100);
            DosBeep(2000,100);
            DosBeep(3000,100);
          }
          else {
            DosBeep(100,500);
            WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_DELETEITEM,MPFROMSHORT(2),0);
            /* Text: "Error while writing the image!"
             */
            getMessage(text, IDSTRLB_IMAGEWRITEERROR, sizeof(text), RESSOURCEHANDLE , hwnd);
          }
          writeLog(text);
          writeLog("\n");

          WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_INSERTITEM,MPFROMSHORT(2),text);
          WinShowWindow(WinWindowFromID(hwnd,IDSR_PERCENT),FALSE);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          WinSetWindowPos(hwnd,HWND_TOP,0,0,0,0,SWP_ZORDER|SWP_ACTIVATE);
          break;
        case ACKEY_PRINTSIZE:
          /* Query imagefile size */
          if(stat(params[3],&buf)) {
            /* Text: "Sorry, file size unknown."
               Title: "Write image, check size"
               */
            messageBox( text, IDSTRD_CHECKSIZEERRORNOSIZE , sizeof(text),
                        title, IDSTRD_CHECKSIZEERRORTITLE, sizeof(title),
                        RESSOURCEHANDLE, hwnd, MB_OK | MB_WARNING|MB_MOVEABLE);
            WinPostMsg(hwnd,WM_CLOSE,0,0);
            break;
          }
          
          /* The PM wrapper requested the imagesize */
          /* Delete previous message in listbox */
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_DELETEITEM,MPFROMSHORT(1),0);

          /* Put new msg with imagesize into listbox */
          /* title: "Imagesize is %d.%0.3d.%0.3d bytes" */
          getMessage(title, IDSTRD_IMAGESIZE, sizeof(title), RESSOURCEHANDLE, hwnd);
          sprintf(text,title,
                  buf.st_size/1000000,(buf.st_size%1000000)/1000,buf.st_size%1000);
          writeLog(text);
          writeLog("\n");
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_INSERTITEM,MPFROMSHORT(1),text);
          /* Save imagesize. We need it for the percent bar */
          lImageSize=buf.st_size;

          if(lImageSize >lCDSize && lCDSize!=0) {
            /* Text: "Image is bigger than free CD space! [...]. Do you want to proceed?"
               Title: "Write image, check size"
               */
            if(MBID_NO==messageBox( text, IDSTRPM_IMAGETOBIG , sizeof(text),
                                    title, IDSTRD_CHECKSIZEERRORTITLE, sizeof(title),
                                    RESSOURCEHANDLE, hwnd, MB_YESNO | MB_WARNING|MB_MOVEABLE)) {            
              WinPostMsg(hwnd,WM_CLOSE,0,0);
              break;
            }
          }
          if(strstr(ptrLocalMem," -multi")) {
            if(lImageSize >(lCDSize-(11400*2048))) {
              if(MBID_NO==WinMessageBox( HWND_DESKTOP, hwnd, "Multisessionimage is to big for the CD! \
There is no space for the necessary lead out. Select singlesession instead.\nYou may override this message if\
detection of CD space failed. Do you want to proceed with writing?",
                                         "On the fly writing",
                                         0UL, MB_YESNO | MB_WARNING|MB_MOVEABLE )) {
                WinPostMsg(hwnd,WM_CLOSE,0,0);
                break;
              }
            }
          }
          /* Now starting the write process */

          /* Put a message in the listbox */
          /* title: "Writing image %s..." */
          getMessage(title, IDSTRD_WRITINGIMAGE, sizeof(title), RESSOURCEHANDLE, hwnd);
          /* insert image name */
          sprintf(text,title, params[3]);
          writeLog(text);
          writeLog("\n");
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_INSERTITEM,MPFROMSHORT(2),text);
          /* Hide ABORT Button in the status dialog. We do not let the user interrupt a write because this
             will damage the CD. */
          WinShowWindow(WinWindowFromID(hwnd,IDPB_ABORTWRITE),FALSE);
          /* Set percent bar value */
          WinPostMsg(WinWindowFromID(hwnd,IDSR_PERCENT),WM_UPDATEPROGRESSBAR,MPFROMLONG(0),MPFROMLONG(lImageSize));
          /* Show percent bar which shows the write progress */
          WinShowWindow(WinWindowFromID(hwnd,IDSR_PERCENT),TRUE);

          /* argv[0]: progname
           * argv[1]: installdir of Audio/Data-CD-Creator
           * argv[2]: folder
           * argv[3]: imagename
           */

          /* logfilename as a parameter */
          buildLogName(title, logName,  sizeof(title));
          snprintf(text, sizeof(text), "\"%s\" \"%s\"" ,params[4], title);
          launchWrapper(text, chrInstallDir, hwnd,"writeimg.exe","CDRecord/2");
          break;

        case ACKEY_CDSIZE:
          /* This msg is sent by the helper prog after getting the actual free space of the inserted
             CD */
          /* Save CD-Size */
          lCDSize=LONGFROMMP(mp2)*2048;
          /* Delete previous Message in the listbox */
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_DELETEITEM,MPFROMSHORT(0),0);

          if(lCDSize==0) {
            writeLog("pwrtimg.exe: Error while querying the free CD-space (free space returned as 0).\n");
            /* There was an error. */
            /* Title: "Writing CD"
               Text: "Can't query free CD space! On some systems detection of free CD space fails 
               so you may override this message if you know what you're doing! Do you want to proceed with writing? "
               */
            if(MBID_NO==queryFreeCDSpaceError(hwnd)) {
              WinPostMsg(hwnd,WM_CLOSE,0,0);
              return FALSE;
            }
          }
          /* Delete check size error message in listbox */
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_DELETEITEM,MPFROMSHORT(0),0);

          /* Insert the CD size into the listbox to inform the user */
          /* title: "Free CD space is %d.%0.3d.%0.3d bytes" */
          getMessage(title, IDSTRLB_FREECDSPACE, sizeof(title), RESSOURCEHANDLE, hwnd);
          sprintf(text,title,
                  LONGFROMMP(mp2)*2048/1000000,(LONGFROMMP(mp2)*2048%1000000)/1000,LONGFROMMP(mp2)*2048%1000);
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_INSERTITEM,MPFROMSHORT(0),text);

          /* Put a message into the listbox */
          /* Text: "Querying image size..." */
          getMessage(text, IDSTRLB_QUERYINGIMAGESIZE, sizeof(text), RESSOURCEHANDLE, hwnd);
          WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_INSERTITEM,MPFROMSHORT(1),text);

          WinPostMsg(hwnd,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PRINTSIZE),0);
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
  FILE *file;
  ULONG fl;
  HWND hwndClient;
  char text[CCHMAXPATH+10];
  char title[CCHMAXPATH];

  /* Create a copy of the args */
  /* argv[0]: progname
   * argv[1]: installdir of Audio-CD-Creator
   * argv[2]: foldername
   * argv[3]: imagename
   * argv[4]: Parameter file
   */
  numArgs=argc;
  for(a=0;a<argc;a++)
    {
      params[a]=argv[a];
    }
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
        /* Get our ressource dll */  
        RESSOURCEHANDLE=queryResModuleHandle();     
        /* Load grabber options from cdrecord.ini */
        readIni();
        /* load background bitmap */
        _loadBmps(CLASSDLLHANDLE);
        if(readWindowPosFromIni(chrInstallDir, "pwrtimg"))
          bHaveWindowPos=TRUE;
        if(checkSettings()) {
          if((ptrLocalMem=malloc(SHAREDMEM_SIZE))!=NULLHANDLE) {            
            if((file=fopen(argv[4],"rb"))!=NULL){
              /* Copy command line to local memory */
              fread(ptrLocalMem, sizeof(char), SHAREDMEM_SIZE, file);
              fclose(file);
              /* Register the percent bar window class */
              percentRegisterBarClass();
              if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, writeImageStatusDialogProc, RESSOURCEHANDLE, IDDLG_CHECKSTATUS, 0)
                  == DID_ERROR )
                {
                  if(ptrLocalMem)
                    free(ptrLocalMem);
                  DosBeep(100,600);
                  errorResource2("Problem with Audio/Data-CD-Creator installation");
                  WinDestroyMsgQueue( hmq );
                  WinTerminate( hab );
                  remove(params[4]);
                  return( 1 );
                }
              if(ptrLocalMem)
                free(ptrLocalMem);
              writeWindowPosToIni(chrInstallDir, "pwrtimg");      
            }
          }
          else {
            /* Text: "Can't alloc shared memory! Aborting..."
               Title: "On the fly writing"
               */
            messageBox( text, IDSTRPM_ALLOCSHAREDMEMERROR , sizeof(text),
                        title, IDSTRD_ONTHEFLYTITLE, sizeof(title),
                        RESSOURCEHANDLE, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
          }
        }/* if(checkSettings()) */
        freeResHandle();
        freeClassDLLHandle();
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  if(ptrLocalMem)
    free(ptrLocalMem);
 
  remove(params[4]);
  return 0;
}









