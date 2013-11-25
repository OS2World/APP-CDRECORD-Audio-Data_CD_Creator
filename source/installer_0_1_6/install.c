/*
 * installer.c (C) Chris Wohlgemuth 2001
 *
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
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <direct.h> 

#include "installer.h"

char chrMkisofsPath[CCHMAXPATH]={0};/* Path to mkisofs */
char chrCDRecordPath[CCHMAXPATH]={0};/* Path to cdrecord */
char chrCdrdaoPath[CCHMAXPATH]={0};
char chrGrabberPath[CCHMAXPATH]={0};
char chrDeviceName[CCHMAXPATH]={0};
char chrDriverName[CCHMAXPATH]={0};
char chrWriterName[CCHMAXPATH]={0};
char chrMpg123Path[CCHMAXPATH]={0};

char * pipePtr;
char  logName[CCHMAXPATH]="logfiles\\install.log";
char licenceFileName[CCHMAXPATH]="docs\\COPYING";
int iSpeed,iFifo;

BOOL  bProgFolderCopied=TRUE;
BOOL  bHelpCopied=TRUE;

char bootDrive;
char drive;
char workDir[CCHMAXPATH]={0};

HMODULE RESSOURCEHANDLE;
int numArgs;

ULONG ulResult=-1;

#define PIPESIZE 255
#define HF_STDOUT 1      /* Standard output handle */

APIRET APIENTRY DosReplaceModule(PSZ old, PSZ new, PSZ backup);

MRESULT EXPENTRY page1Proc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY page2Proc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY checkPathPageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY mkisofsPageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY grabberPageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY cdrdaoPageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY licencePageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY instructionPageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

char getBootDrive(void)
{
  ULONG ulSysValue;
  char drive;

  DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, &ulSysValue, sizeof(ulSysValue));
  drive='A';
  drive+=(char)(ulSysValue-1);
  return drive;
}

ULONG replaceModule(PSZ chrSource, PSZ chrDestination)
{
  ULONG rc;
  
  /* First try normal copy */
  rc=DosCopy(chrSource,chrDestination,DCPY_EXISTING|DCPY_FAILEAS);
  /*  if(rc==32) */
  if(rc!=NO_ERROR)
	{
      /* This call only works for *.DLL and *.EXE */
      rc=DosReplaceModule(chrDestination,chrSource,NULL);
	}
  return rc;
}

void removeLog(void)
{
  char logNameLocal[CCHMAXPATH];

  sprintf(logNameLocal,"%c:\\%s\\%s", drive, workDir,logName);
  remove(logNameLocal);
}

void writeLog(char* logText)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;

  sprintf(logNameLocal,"%c:\\%s\\%s", drive, workDir,logName);
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {
    fprintf(fHandle,logText);
    fclose(fHandle);
  }
}

int copyFiles(void)
{
  char chrSource[CCHMAXPATH];
  char chrDestination[CCHMAXPATH];
  ULONG rc;

  sprintf(chrDestination, "%c:\\os2\\dll\\progfldr.dll", getBootDrive());
  sprintf(chrSource,"%c:\\%s\\progfldr.dll", drive,workDir);
  writeLog("Copy ");
  writeLog(chrSource);
  writeLog(" to ");
  writeLog(chrDestination);
  rc=replaceModule( chrSource,  chrDestination);
  if(rc!=NO_ERROR) {
    writeLog(" +++++ Error while copying!!\n");
    bProgFolderCopied=FALSE;
    return -1;
  }
  else
    writeLog("\n");

  sprintf(chrDestination, "%c:\\os2\\help\\aucdfldr.hlp", getBootDrive());
  sprintf(chrSource,"%c:\\%s\\aucdfldr.hlp", drive,workDir);
  writeLog("Copy ");
  writeLog(chrSource);
  writeLog(" to ");
  writeLog(chrDestination);
  writeLog("\n");
  rc=replaceModule( chrSource,  chrDestination);
  if(rc!=NO_ERROR) {
    writeLog(" +++++ Error while copying!!\n");
    bHelpCopied=FALSE;
  }
  else 
    writeLog("\n");
  
  return 0;
}

void findProg(char * progNameBuffer)
{
  FILEDLG fd = { 0 };  

  /* User pressed the BROWSE button */
  fd.cbSize = sizeof( fd );
  /* It's an centered 'Save'-dialog */
  fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
  /* Set the title of the file dialog */
  /* Text: "Path for ISO-Image file" */
  /* getMessage(text, IDSTRD_IMAGENAMEBROWSE,sizeof(text), hDataResource, hwnd);*/
  fd.pszTitle = "Find Program";
  /* Only show *.exe files */
  sprintf(fd.szFullFile,"%s","*.exe");
  
  if( WinFileDlg( HWND_DESKTOP, HWND_DESKTOP, &fd ) == NULLHANDLE )
    {
      /* WinFileDlg failed */
      return;
    }
  if( fd.lReturn == DID_OK )
    {
      strcpy(progNameBuffer, fd.szFullFile );      
    }  
}

void readIni(void)
{
  ULONG keyLength;
  char profileName[CCHMAXPATH];
  char moduleName[CCHMAXPATH];
  char *chrPtr;
  HINI hini=0;
  char    text[200];
  ULONG ulSize;

  /* Build full path for cdrecord.ini file */
  sprintf(profileName,"%c:\\%s\\cdrecord.ini",drive,workDir);       
  hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),profileName);
  do{
    if(!hini) {
      /*      WinMessageBox(  HWND_DESKTOP, HWND_DESKTOP,
                      profileName,
                      workDir,
                      0UL, MB_OK | MB_ERROR |MB_MOVEABLE);
                      */

      break;
    }/* end of if(!hini) */
    
    keyLength=PrfQueryProfileString(hini,"CDWriter","cdrecord","",chrCDRecordPath,sizeof(chrCDRecordPath));
    
    keyLength=PrfQueryProfileString(hini,"Mkisofs","mkisofs","",chrMkisofsPath,sizeof(chrMkisofsPath));

    keyLength=PrfQueryProfileString(hini,"CDGrabber","grabber","",chrGrabberPath,sizeof(chrGrabberPath));

    keyLength=PrfQueryProfileString(hini,"mpg123","path","",chrMpg123Path,sizeof(chrMpg123Path));

    keyLength=PrfQueryProfileString(hini,"cdrdao","path","",chrCdrdaoPath,sizeof(chrCdrdaoPath));
    keyLength=PrfQueryProfileString(hini,"cdrdao","writer","",chrWriterName,sizeof(chrWriterName));

    iSpeed=PrfQueryProfileInt(hini,"device","speed",1);
    iFifo=PrfQueryProfileInt(hini,"device","fifo",4);


    break;
  } while(TRUE);
  if(hini)PrfCloseProfile(hini);


}

void writeIni(void)
{
  char profileName[CCHMAXPATH];
  char moduleName[CCHMAXPATH];
  char *chrPtr;
  ULONG ulLength;
  HINI hini=0;
  BOOL rc=TRUE;
  char chrDev[10];

  rc=TRUE;

  /* Build full path */
  DosQueryCurrentDir(0,moduleName,&ulLength);
  /*  sprintf(profileName,"%s\\cdrecord.ini",moduleName);       
   */
  sprintf(profileName,"cdrecord.ini");

  do{  
    hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),profileName);
    writeLog("Creating INI file ");
    writeLog(profileName);
    writeLog("\n");   
    if(!hini) {
    /* profileName: "Warning! Cannot open Ini-file!"
       moduleName: "Data-CD-Creator"
       */
      /*  messageBox( profileName, IDSTR_INIFILEOPENERROR , sizeof(profileName),
          moduleName, IDSTRD_DATACDCREATOR, sizeof(moduleName),
          hDataResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
          */    
      writeLog("Can't create INI file!\n");   
      break;
    }/* end of if(!hini) */
    writeLog("\nWriting the following values to INI file:\n");
   
    /* Save cdrecord-path */
    if(!PrfWriteProfileString(hini,"CDWriter","cdrecord",chrCDRecordPath)) {
      rc=FALSE; 
      break;    
    }
    writeLog("CDRecord/2 path: ");
    writeLog(chrCDRecordPath);   
    writeLog("\n");      

    if(!PrfWriteProfileString(hini,"Mkisofs","mkisofs",chrMkisofsPath)){
      rc=FALSE;
      break;
    }
    writeLog("Mkisofs path: ");
    writeLog(chrMkisofsPath);   
    writeLog("\n");      

    if(!PrfWriteProfileString(hini,"CDGrabber","grabber",chrGrabberPath)) {
      rc=FALSE;
      break;
    }
    writeLog("Grabber path: ");
    writeLog(chrGrabberPath);   
    writeLog("\n");      

    if(!PrfWriteProfileString(hini,"cdrdao","path",chrCdrdaoPath)) {
      rc=FALSE;
      break;
    }
    writeLog("Cdrdao/2 path: ");
    writeLog(chrCdrdaoPath);   
    writeLog("\n");      

    if(!PrfWriteProfileString(hini,"cdrdao","writer",chrWriterName)) {
      rc=FALSE;
      break;
    }
    writeLog("Cdrdao/2 writer name: ");
    writeLog(chrWriterName);   
    writeLog("\n");      

    if(!PrfWriteProfileString(hini,"cdrdao","driver",chrDriverName)) {
      rc=FALSE;
      break;
    }
    writeLog("Cdrdao/2 driver name: ");
    writeLog(chrDriverName);   
    writeLog("\n");      

    /*   if(!PrfWriteProfileString(hini,"device","devicename",chrDeviceName)) {
         rc=FALSE;
         break;
         }
         writeLog("Device name: ");
         writeLog(chrDeviceName);   
         writeLog("\n");      
         */
    sprintf(chrDev,"%d",iSpeed);
    if(!PrfWriteProfileString(hini,"device","speed",chrDev)) {
      rc=FALSE;
      break;
    }
    writeLog("Max writing speed: ");
    writeLog(chrDev);   
    writeLog("\n");      

    sprintf(chrDev,"%d",iFifo);
    if(!PrfWriteProfileString(hini,"device","fifo",chrDev)) {
      rc=FALSE;
      break;
    }
    writeLog("Fifo size in MBytes: ");
    writeLog(chrDev);   
    writeLog("\n");      

    /* mpg123 is included so insert the path of the new version */
    sprintf(chrMpg123Path,"%c:\\%s\\bin\\mpg123.exe",drive,workDir);       
    if(!PrfWriteProfileString(hini,"mpg123","path",chrMpg123Path)) {
      rc=FALSE;
      break;
    }
    writeLog("Mpg123 path: ");
    writeLog(chrMpg123Path);   
    writeLog("\n");      
    
    break;
  }while(TRUE);
  if(hini)PrfCloseProfile(hini);
  if(!rc) {
    /* profileName: "Warning! Cannot write to Ini-file!"
       moduleName: "Data-CD-Creator"
       */
    /*    messageBox( profileName, IDSTR_WRITEINIERROR, sizeof(profileName),
                moduleName, IDSTRD_DATACDCREATOR, sizeof(moduleName),
                hDataResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
                */
    writeLog("Error while writing INI file!\n");   
  }
}

BOOL checkProgram(PSZ progName) 
{
  struct stat statBuf;

  /* Check path */
  if(stat(progName , &statBuf)==-1)
    return FALSE;
  
  
  return TRUE;  
}


MRESULT EXPENTRY finishPageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  char *textPtr;
  char *textPtr2; 
  static LONG lCDSize;
  static LONG lImageSize;
  struct stat buf;
  SWCNTRL swctl;
  PID pid;
  SWP swp,swp2;
  SHORT x,y;

  switch (msg)
    {      
    case WM_INITDLG:   
      WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),(PSWP)&swp);
      WinQueryWindowPos(hwnd,(PSWP)&swp2);
      x=(swp.cx-swp2.cx)/2;
      y=(swp.cy-swp2.cy)/2;
      if(checkProgram(chrMkisofsPath))
        WinSetWindowText(WinWindowFromID(hwnd,IDST_MKISOFSPATH), "configured");
      else
        WinSetWindowText(WinWindowFromID(hwnd,IDST_MKISOFSPATH), "NOT configured - no creation of data CDs");

      if(checkProgram(chrCDRecordPath))
        WinSetWindowText(WinWindowFromID(hwnd,IDST_CDRECORDPATH), "configured");
      else
        WinSetWindowText(WinWindowFromID(hwnd,IDST_CDRECORDPATH), "NOT configured - no creation of CDs");

      if(checkProgram(chrGrabberPath))
        WinSetWindowText(WinWindowFromID(hwnd,IDST_GRABBERPATH), "configured");
      else
        WinSetWindowText(WinWindowFromID(hwnd,IDST_GRABBERPATH), "NOT configured - grabbing not possible");

      if(checkProgram(chrCdrdaoPath))
        WinSetWindowText(WinWindowFromID(hwnd,IDST_CDRDAOPATH), "configured");
      else
        WinSetWindowText(WinWindowFromID(hwnd,IDST_CDRDAOPATH), "NOT configured - no DAO writing");

      WinSetWindowPos(hwnd,HWND_TOP, x, y, 0, 0,SWP_MOVE|SWP_ACTIVATE);
      return (MRESULT) TRUE;
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_FINISH:
          writeIni();
          ulResult=0;
          copyFiles();
          /*if(copyFiles()==-1)
            break;*/
          WinPostMsg(hwnd,WM_QUIT,0,0);
          break;
        case IDPB_EXIT:
          ulResult=-1;
          WinPostMsg(hwnd,WM_QUIT,0,0);
          break;
        case IDPB_NEXT:
          break;
        case IDPB_BACK:
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE,mkisofsPageProc,0,IDDLG_MKISOFS,NULL);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        default:
          break;
        }
      return (MRESULT)TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);    
}

MRESULT EXPENTRY mkisofsPageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  char *textPtr;
  char *textPtr2; 
  static LONG lCDSize;
  static LONG lImageSize;
  struct stat buf;
  SWCNTRL swctl;
  PID pid;
  SWP swp,swp2;
  SHORT x,y;

  switch (msg)
    {      
    case WM_INITDLG:   
      WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),(PSWP)&swp);
      WinQueryWindowPos(hwnd,(PSWP)&swp2);
      x=(swp.cx-swp2.cx)/2;
      y=(swp.cy-swp2.cy)/2;
      WinSendMsg(WinWindowFromID(hwnd,IDEF_MKISOFSPATH),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_MKISOFSPATH),chrMkisofsPath);
      WinSetWindowPos(hwnd,HWND_TOP, x, y, 0, 0,SWP_MOVE|SWP_ACTIVATE);
      return (MRESULT) TRUE;
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_BROWSE:
          findProg(chrMkisofsPath);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_MKISOFSPATH),chrMkisofsPath);
          break;
        case IDPB_EXIT:
          WinPostMsg(hwnd,WM_QUIT,0,0);
          break;
        case IDPB_NEXT:
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT), NULLHANDLE, finishPageProc, 0, IDDLG_FINISHPAGE, NULL);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        case IDPB_BACK:
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE,grabberPageProc,0,IDDLG_PAGEGRABBER,NULL);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        default:
          break;
        }
      return (MRESULT)TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);    
}

MRESULT EXPENTRY grabberPageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SWP swp,swp2;
  SHORT x,y;

  switch (msg)
    {      
    case WM_INITDLG:   
      WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),(PSWP)&swp);
      WinQueryWindowPos(hwnd,(PSWP)&swp2);
      x=(swp.cx-swp2.cx)/2;
      y=(swp.cy-swp2.cy)/2;
      WinSendMsg(WinWindowFromID(hwnd,IDEF_GRABBER),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_GRABBER),chrGrabberPath);
      WinSetWindowPos(hwnd,HWND_TOP, x, y, 0, 0,SWP_MOVE|SWP_ACTIVATE);
      return (MRESULT) TRUE;
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_BROWSE:
          findProg(chrGrabberPath);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_GRABBER),chrGrabberPath);
          break;
        case IDPB_EXIT:
          WinPostMsg(hwnd,WM_QUIT,0,0);
          break;
        case IDPB_NEXT:
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE,mkisofsPageProc,0,IDDLG_MKISOFS,NULL);
          WinPostMsg(hwnd,WM_CLOSE,0,0);        
          break;
        case IDPB_BACK:
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE,cdrdaoPageProc,0,IDDLG_PAGECDRDAO,NULL);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        default:
          break;
        }
      return (MRESULT)TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);    
}


MRESULT EXPENTRY cdrdaoPageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SWP swp,swp2;
  SHORT x,y;
  VOID *pBuffer;
  ULONG ulSize;
  HINI hini;
  char *ptr;
  char chrTemp[100];
  BOOL bFirst=FALSE;

  switch (msg)
    {      
    case WM_INITDLG:   
      WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),(PSWP)&swp);
      WinQueryWindowPos(hwnd,(PSWP)&swp2);
      x=(swp.cx-swp2.cx)/2;
      y=(swp.cy-swp2.cy)/2;
      WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRDAOPATH),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOPATH),chrCdrdaoPath);   
      WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),chrWriterName);
      hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),"cfgdata.ini");
      if(hini)
        {
          if(PrfQueryProfileSize(hini, NULL,NULL,&ulSize)) {
            pBuffer=(VOID*)malloc(ulSize);
            if(pBuffer) {
              if(PrfQueryProfileData(hini, NULL,NULL, pBuffer, &ulSize))
                {
                  ptr=pBuffer;
                  if(strlen(chrWriterName)==0)
                    bFirst=TRUE;        
                  do {
                    PrfQueryProfileString(hini,ptr,"driver","",chrTemp,sizeof(chrTemp));
                    if(strlen(chrTemp)!=0) {
                      WinSendMsg(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),LM_INSERTITEM,MPFROMSHORT(LIT_SORTASCENDING),
                                 (MPARAM)ptr);
                      if(bFirst) {
                        bFirst=FALSE;
                        WinSetWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),ptr);
                      }
                    }
                    ptr=strchr(ptr,0);
                    if(!ptr)
                      break;
                    ptr++;
                  }while(*ptr!=0);
                }
              free(pBuffer);
            }
          }
          PrfCloseProfile(hini);
        }
      WinSetWindowPos(hwnd,HWND_TOP, x, y, 0, 0,SWP_MOVE|SWP_ACTIVATE);
      return (MRESULT) TRUE;
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_BROWSE:
          findProg(chrCdrdaoPath);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRDAOPATH),chrCdrdaoPath);
          break;
        case IDPB_EXIT:
          WinPostMsg(hwnd,WM_QUIT,0,0);
          break;
        case IDPB_NEXT:
          WinQueryWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),sizeof(chrDriverName),chrWriterName);
          hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),"cfgdata.ini");
          if(hini)
            {
              PrfQueryProfileString(hini,chrWriterName,"driver","",chrDriverName,sizeof(chrDriverName));
              PrfCloseProfile(hini);
            }          
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE,grabberPageProc,0,IDDLG_PAGEGRABBER,NULL);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        case IDPB_BACK:
          WinQueryWindowText(WinWindowFromID(hwnd,IDLB_CDRDAODRIVER),sizeof(chrDriverName),chrWriterName);
          hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),"cfgdata.ini");
          if(hini)
            {
              PrfQueryProfileString(hini,chrWriterName,"driver","",chrDriverName,sizeof(chrDriverName));
              PrfCloseProfile(hini);
            }          
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE, page2Proc,0,IDDLG_PAGE2,NULL);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        default:
          break;
        }
      return (MRESULT)TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);    
}


MRESULT EXPENTRY page2Proc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  ULONG ulValue;
  SWP swp,swp2;
  SHORT x,y;

  switch (msg)
    {      
    case WM_INITDLG:   
      WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),(PSWP)&swp);
      WinQueryWindowPos(hwnd,(PSWP)&swp2);
      x=(swp.cx-swp2.cx)/2;
      y=(swp.cy-swp2.cy)/2;
      WinSendMsg(WinWindowFromID(hwnd,IDEF_CDRECORDPATH),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
      WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRECORDPATH),chrCDRecordPath);
      /* Speed spin button */
      WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETLIMITS,MPFROMLONG(16),MPFROMLONG(1));
      WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_SETCURRENTVALUE,(MPARAM)iSpeed,MPFROMLONG(0));
      /* FIFO spin button */
      WinSendMsg(WinWindowFromID(hwnd,IDSB_FIFO),SPBM_SETLIMITS,MPFROMLONG(64),MPFROMLONG(4));
      WinSendMsg(WinWindowFromID(hwnd,IDSB_FIFO),SPBM_SETCURRENTVALUE,(MPARAM)iFifo,MPFROMLONG(0));

      WinSetWindowPos(hwnd,HWND_TOP, x, y, 0, 0,SWP_MOVE|SWP_ACTIVATE);
      return (MRESULT) TRUE;
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_BROWSE:
          findProg(chrCDRecordPath);
          WinSetWindowText(WinWindowFromID(hwnd,IDEF_CDRECORDPATH),chrCDRecordPath);
          break;
        case IDPB_EXIT:
          WinPostMsg(hwnd,WM_QUIT,0,0);
          break;
        case IDPB_NEXT:
          WinSendMsg(WinWindowFromID(hwnd,IDSB_SPEED),SPBM_QUERYVALUE,MPFROMP(&ulValue),
                     MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));
          iSpeed=ulValue;
          
          WinSendMsg(WinWindowFromID(hwnd,IDSB_FIFO),SPBM_QUERYVALUE,MPFROMP(&ulValue),
                     MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));
          iFifo=ulValue;
      
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE,cdrdaoPageProc,0,IDDLG_PAGECDRDAO,NULL);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        case IDPB_BACK:
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE, instructionPageProc,0,IDDLG_INSTRUCTION,NULL);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        default:
          break;
        }
      return (MRESULT)TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);    
}

MRESULT EXPENTRY instructionPageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  ULONG ulValue;
  SWP swp,swp2;
  SHORT x,y;
  FILE *stream;
  ULONG count;
  ULONG ulRead;
  ULONG ulInsert;

  switch (msg)
    {      
    case WM_INITDLG:   
      WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),(PSWP)&swp);
      WinQueryWindowPos(hwnd,(PSWP)&swp2);
      x=(swp.cx-swp2.cx)/2;
      y=(swp.cy-swp2.cy)/2;
      WinSetWindowPos(hwnd,HWND_TOP, x, y, 0, 0,SWP_MOVE|SWP_ACTIVATE);
      return (MRESULT) TRUE;
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_EXIT:
          WinPostMsg(hwnd,WM_QUIT,0,0);
          break;
        case IDPB_NEXT:
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE,page2Proc,0,IDDLG_PAGE2,NULL);       
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        case IDPB_BACK:
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE, licencePageProc,0,IDDLG_LICENCE, NULL);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        default:
          break;
        }
      return (MRESULT)TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);    
}


MRESULT EXPENTRY licencePageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  ULONG ulValue;
  SWP swp,swp2;
  SHORT x,y;
  FILE *stream;
  char licenceFile[CCHMAXPATH];
  ULONG count;
  ULONG ulRead;
  ULONG ulInsert;

  switch (msg)
    {      
    case WM_INITDLG:   
      WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),(PSWP)&swp);
      WinQueryWindowPos(hwnd,(PSWP)&swp2);
      x=(swp.cx-swp2.cx)/2;
      y=(swp.cy-swp2.cy)/2;

      sprintf(licenceFile,"%c:\\%s\\%s", drive, workDir,licenceFileName);
      stream=fopen(licenceFile,"r");
      if(stream) {
        WinSendMsg(WinWindowFromID(hwnd,IDMLE_LICENCE),MLM_SETIMPORTEXPORT,MPFROMP(licenceFile),
                       MPFROMSHORT((SHORT)sizeof(licenceFile)));
        do {
          ulRead=fread(licenceFile,sizeof(char),sizeof(licenceFile),stream);
          count=(ULONG)WinSendMsg(WinWindowFromID(hwnd,IDMLE_LICENCE),MLM_IMPORT,MPFROMP(&ulInsert),
                       MPFROMLONG(ulRead));
        }while(count);
        fclose(stream);
      }
      WinSetWindowPos(hwnd,HWND_TOP, x, y, 0, 0,SWP_MOVE|SWP_ACTIVATE);
      return (MRESULT) TRUE;
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_EXIT:
          WinPostMsg(hwnd,WM_QUIT,0,0);
          break;
        case IDPB_NEXT:
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE,instructionPageProc,0,IDDLG_INSTRUCTION,NULL);       
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        case IDPB_BACK:
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE, page1Proc,0,IDDLG_PAGE1,NULL);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        default:
          break;
        }
      return (MRESULT)TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);    
}


/* This Proc handles the on-the-fly data CD writing */
MRESULT EXPENTRY page1Proc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  char *textPtr;
  char *textPtr2; 
  static LONG lCDSize;
  static LONG lImageSize;
  struct stat buf;
  SWCNTRL swctl;
  PID pid;
  SWP swp,swp2;
  SHORT x,y;

  switch (msg)
    {      
    case WM_INITDLG:   
      WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),(PSWP)&swp);
      WinQueryWindowPos(hwnd,(PSWP)&swp2);
      x=(swp.cx-swp2.cx)/2;
      y=(swp.cy-swp2.cy)/2;
      WinSetWindowPos(hwnd,HWND_TOP, x, y, 0, 0,SWP_MOVE|SWP_ACTIVATE);
      return (MRESULT) TRUE;
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_EXIT:
          WinPostMsg(hwnd,WM_QUIT,0,0);
          break;
        case IDPB_NEXT:
          WinLoadDlg(WinQueryWindow(hwnd,QW_PARENT),NULLHANDLE,licencePageProc,0,IDDLG_LICENCE,NULL);
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        default:
          break;
        }
      return (MRESULT)TRUE;
    default:
      break;
    }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);    
}

MRESULT EXPENTRY mainProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH];
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
    case WM_INITDLG:   
      /* getMessage(text, IDSTRD_WRITEIMAGETITLE, sizeof(text), RESSOURCEHANDLE , hwnd);
       */
      /*      if(DosGetNamedSharedMem(&pvSharedMem,"\\SHAREMEM\\MKISOFSCMDLINE",PAG_READ|PAG_WRITE)) {
              WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, "Can't alloc shared mem! Aborting...",
              "Writing image",
              0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
              sendCommand("FREESHAREDMEM=1");
              WinPostMsg(hwnd,WM_CLOSE,0,0);
              }*/
          
      /* Add switch entry */
      memset(&swctl,0,sizeof(swctl));
      WinQueryWindowProcess(hwnd,&pid,NULL);
      swctl.hwnd=hwnd;
      swctl.uchVisibility=SWL_VISIBLE;
      swctl.idProcess=pid;
      swctl.bProgType=PROG_DEFAULT;
      swctl.fbJump=SWL_JUMPABLE;
      WinAddSwitchEntry(&swctl);
      WinLoadDlg(hwnd, hwnd, page1Proc,0,IDDLG_PAGE1,NULL);
      WinSetWindowPos(hwnd,HWND_TOP,0,0,0,0,SWP_ZORDER|SWP_ACTIVATE);
      writeLog("Starting installation for Audio/Data-CD-Creator\n\n");
      return (MRESULT) TRUE;
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case IDPB_NEXT:
          break;
        default:
          break;
        }
      return (MRESULT)TRUE;
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
  ULONG ulLength;
  int iTemp;

  numArgs=argc;
  
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {
      ulLength=sizeof(workDir);  
      DosQueryCurrentDir(0,workDir,&ulLength);
       
      /*DosQueryCurrentDir(0,chrCDRecordPath,&ulLength);
       */
      iTemp=_getdrive();
      iTemp+='A'-1;
      drive=(char)iTemp;
      bootDrive=getBootDrive();
      removeLog();
      /* Load options from cdrecord.ini */
      readIni();
      if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, mainProc, 0, IDDLG_MAIN, 0 ) == DID_ERROR )
        {
          WinDestroyMsgQueue( hmq );
          WinTerminate( hab );
          DosBeep(100,600);
          return( 1 );
        }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return ulResult;
}








