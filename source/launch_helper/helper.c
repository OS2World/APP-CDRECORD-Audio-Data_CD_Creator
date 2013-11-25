/*
 * This file is (C) Chris Wohlgemuth 2001/2002
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
#define INCL_WIN
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_GPIBITMAPS

#include <os2.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include "audiofolder.h"
#include "audiofolderres.h"

#define INSTALLDIR 1

/* The subdirectory in the installation directory where to put the log files */
#define LOGFILE_SUBDIR "Logfiles"

extern char* params[];
extern HMODULE RESSOURCEHANDLE;
extern HMODULE CLASSDLLHANDLE;
extern char logName[];

char chrInstallDir[CCHMAXPATH];
char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
char chrCDROptions[CCHMAXPATH];
char chrAudioCDROptions[CCHMAXPATH];
LONG  lCDROptions=0;

char chrMkisofs[CCHMAXPATH];/* Path to mkisofs */
char chrMkisofsOptions[CCHMAXPATH];
LONG lMKOptions;

char chrGrabberPath[CCHMAXPATH];
char chrGrabberOptions[CCHMAXPATH];
int bTrackNumbers;
int iGrabberID;
char chosenCD[3];

char chrMpg123Path[CCHMAXPATH];
BOOL bMpg123SwabBytes;
int iMp3Decoder;

char chrDVDDao[CCHMAXPATH];/* Path to dvddao */

char chrMntIsoFS[CCHMAXPATH];
char chrUmntIso[CCHMAXPATH];

char chrCdrdaoPath[CCHMAXPATH]={0};
char chrCdrdaoDriver[100]={0};
int iBus=0;
int iTarget=0;
int iLun=0;
int iSpeed=1;
int iFifo=4;

BOOL bUseCDDB;
BOOL bTipsEnabled;
BOOL bUseCustomPainting=TRUE;

SWP swpWindow;

HMODULE queryResModuleHandle2(char *installDir);


BOOL HlpQueryHintDatabaseLocation(char * chrInstallationDir, char * chrBuffer, int iBufferSize)
{
  int iLeft=iBufferSize;

  strncpy(chrBuffer, chrInstallationDir, iBufferSize);
  chrBuffer[iBufferSize-1]=0;
  iLeft=iBufferSize-strlen(chrBuffer);
  /* HINTDATABASEPATH defined in audiofolder.h */
  strncat(chrBuffer, HINTDATABASEPATH, iLeft);
  chrBuffer[iBufferSize-1]=0;

  return TRUE;
}

BOOL HlpQueryDaytipDatabaseLocation(char * chrInstallationDir, char * chrBuffer, int iBufferSize)
{
  int iLeft=iBufferSize;

  strncpy(chrBuffer, chrInstallationDir, iBufferSize);
  chrBuffer[iBufferSize-1]=0;
  iLeft=iBufferSize-strlen(chrBuffer);
  /* DAYTIPDATABASEPATH defined in audiofolder.h */
  strncat(chrBuffer, DAYTIPDATABASEPATH, iLeft);
  chrBuffer[iBufferSize-1]=0;

  return TRUE;
}

BOOL HlpQueryTempDir(char * chrInstallationDir, char * chrBuffer, int iBufferSize)
{
  int iLeft=iBufferSize;

  strncpy(chrBuffer, chrInstallationDir, iBufferSize);
  chrBuffer[iBufferSize-1]=0;
  iLeft=iBufferSize-strlen(chrBuffer);
  strncat(chrBuffer, "\\temp", iLeft);
  chrBuffer[iBufferSize-1]=0;

  return TRUE;
}

/****************************************************
 *                                                  *
 * This funktion returns the running OS version:    *
 *                                                  *
 * 30: Warp 3, 40 Warp 4                            *
 *                                                  *
 ****************************************************/
ULONG cwQueryOSRelease()
{
  static ULONG ulVersionMinor=0;

  if(!ulVersionMinor)
    if(DosQuerySysInfo(QSV_VERSION_MINOR, QSV_VERSION_MINOR, &ulVersionMinor, sizeof(ulVersionMinor)))
      ulVersionMinor=30;/* Default Warp 3 */

  return ulVersionMinor;

}
void writeLog(char* logText);
void removeLog(void)
{
  char logNameLocal[CCHMAXPATH];
  
  sprintf(logNameLocal,"%s\\%s\\%s",params[1],LOGFILE_SUBDIR,logName);
  //WinMessageBox(HWND_DESKTOP,HWND_DESKTOP, logNameLocal,"",1234,MB_MOVEABLE|MB_OK);
  //writeLog(logNameLocal);

  remove(logNameLocal);
  //DosBeep(500,100);
}

void removeLog2(char * installDir, char * logName)
{
  char logNameLocal[CCHMAXPATH];

  sprintf(logNameLocal,"%s\\%s\\%s",installDir,LOGFILE_SUBDIR,logName);
  remove(logNameLocal);
}

void writeLog(char* logText)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;

  sprintf(logNameLocal,"%s\\%s\\%s",params[1],LOGFILE_SUBDIR,logName);
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {
    fprintf(fHandle,logText);
    fclose(fHandle);
  }
}

void writeLog2(char * installDir, char * logName, char* logText)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;

  sprintf(logNameLocal,"%s\\%s\\%s",installDir, LOGFILE_SUBDIR, logName);
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {
    fprintf(fHandle,logText);
    fclose(fHandle);
  }
}

BOOL buildLogName( char * outBuf, char * logName,ULONG ulSize)
{
  if(snprintf(outBuf, ulSize,"logfiles\\%s",logName)==EOF)
    return FALSE;
  return TRUE;
}

/* This function launches the wrapper <wrapperExe>         */
/*  with the params given as a PM-session */
/* in PSZ parameter. PSZ folderPath is the path to put the */
/* write.log. HWND hwnd is the window waiting for the      */
/* WM_APPTERMINATE message                                 */
ULONG showLogFile()
{
  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrLoadError[CCHMAXPATH];
  char startParams[CCHMAXPATH*4];
  char exename[CCHMAXPATH]={0};
  /*  char chrFolderPath[CCHMAXPATH+10];
   */
  char logNameLocal[CCHMAXPATH];
  PSZ pszTitle="CD-Creator log:";

  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_FORE;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;
    
  sprintf(exename,"%s","e.exe");

  /*  if(!checkHelper(exename))
    return -1;
    */

  startData.PgmName=exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_PM;
  startData.PgmControl=0;
  startData.InitXPos=30;
  startData.InitYPos=30;
  startData.InitXSize=500;
  startData.InitYSize=400;
  startData.ObjectBuffer=chrLoadError;
  startData.ObjectBuffLen=(ULONG)sizeof(chrLoadError);

  /* Put the exe-path between " " to make sure, spaces are handled correctly */
  /* strcpy(chrFolderPath,folderPath);
     sprintf(startParams,"\"%s\" \"%s\" %s",
     chrInstallDir,chrFolderPath, parameter);
     */

  sprintf(logNameLocal,"%s\\%s\\%s",params[1],LOGFILE_SUBDIR,logName);
  startData.PgmInputs=logNameLocal;

  rc=DosStartSession(&startData,&ulSessionID,&pid);   
  return 0;   
}


/* This function launches the wrapper <wrapperExe>         */
/*  with the params given as a PM-session */
/* in PSZ parameter. PSZ folderPath is the path to put the */
/* write.log. HWND hwnd is the window waiting for the      */
/* WM_APPTERMINATE message                                 */
ULONG launchPMWrapper(PSZ chrInstallDir, PSZ parameter, PSZ folderPath, PSZ wrapperExe, PSZ pszTitle)
{
  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrLoadError[CCHMAXPATH];
  char startParams[CCHMAXPATH*4];
  char exename[CCHMAXPATH]={0};
  char chrFolderPath[CCHMAXPATH+10];
  
  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_FORE;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;
    
  /*  sprintf(exename,"%s",buildWrapName(wrapperExe));*/
  sprintf(exename,"%s\\bin\\%s",params[1],wrapperExe);
  /*
    if(!checkHelper(exename))
    return -1;
    */

  startData.PgmName=exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_PM;
  startData.PgmControl=0;
  startData.InitXPos=30;
  startData.InitYPos=30;
  startData.InitXSize=500;
  startData.InitYSize=400;
  startData.ObjectBuffer=chrLoadError;
  startData.ObjectBuffLen=(ULONG)sizeof(chrLoadError);

  /* Put the exe-path between " " to make sure, spaces are handled correctly */
  strcpy(chrFolderPath,folderPath);
  sprintf(startParams,"\"%s\" \"%s\" %s",
          chrInstallDir,chrFolderPath, parameter);
  startData.PgmInputs=startParams;

  rc=DosStartSession(&startData,&ulSessionID,&pid);   
  return 0;   
}


/* This function launches the writer with the params given */
/* in PSZ parameter. PSZ folderPath is the path to put the */
/* write.log. HWND hwnd is the window waiting for the      */
/* WM_APPTERMINATE message                                 */
ULONG launchWrapper(PSZ parameter, PSZ folderPath,HWND hwnd, PSZ wrapperExe, PSZ pszTitle)
{
  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrLoadError[CCHMAXPATH];
  char startParams[CCHMAXPATH*4];
  char exename[CCHMAXPATH]={0};
  char chrCDRexe[CCHMAXPATH+10];
  struct stat statBuf;


  sprintf(exename,"%s\\bin\\%s",params[1],wrapperExe);
  
  /* Check if the VIO helper is in the bin dir */
  if(stat(exename , &statBuf)==-1) {
    /* text: "Can't find the helper program %s. It should be in the installation directory of Audio/Data-CD-Creator."
       title: "Audio/Data-CD-Creator installation problem!"
       */
    getMessage(chrLoadError, IDSTR_INSTALLNOVIO, sizeof(chrLoadError), RESSOURCEHANDLE , hwnd);
    sprintf(startParams,chrLoadError,wrapperExe);
    getMessage(chrLoadError, IDSTR_INSTALLERRORTITLE, sizeof(chrLoadError), RESSOURCEHANDLE , hwnd);
    WinMessageBox(HWND_DESKTOP,0,
                  startParams,
                  chrLoadError,12345,
                  MB_OK|MB_MOVEABLE|MB_ERROR);
    writeLog("\nCan't find vio helper '");
    writeLog(exename);  
    writeLog("'\n");
    return -1;
  }
  
  writeLog("\nStarting vio helper '");
  writeLog(exename);  
  writeLog("'\nWith the following parameters:\n");

  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_BACK;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;
      
  startData.PgmName=exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_WINDOWABLEVIO;
  startData.PgmControl=0;
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

  /* Put the exe-path between " " to make sure, spaces are handled correctly */
  sprintf(chrCDRexe,"\"%s\"", chrCDRecord);
  if(strlen(folderPath)!=0)
    sprintf(startParams,"%d \"%s\" \"%s\" %s",hwnd,chrCDRexe,folderPath ,parameter);
  else
    sprintf(startParams,"%d  \"%s\" %s",hwnd,chrCDRexe, parameter);
  startData.PgmInputs=startParams;

  writeLog("    ");
  writeLog(startParams);
  writeLog("\n\n");
 
  rc=DosStartSession(&startData,&ulSessionID,&pid);   
  if(rc!=NO_ERROR) {
    writeLog("Error while starting VIO helper: ");
    sprintf(chrLoadError,"%d\n",rc);
    writeLog(chrLoadError);
    return -1;
  }
  return 0;   
}

/* This function launches the writer with the params given */
/* in PSZ parameter. PSZ folderPath is the path to put the */
/* write.log. HWND hwnd is the window waiting for the      */
/* WM_APPTERMINATE message                                 */
ULONG launchDVDWrapper(PSZ parameter, PSZ folderPath,HWND hwnd, PSZ wrapperExe, PSZ pszTitle)
{
  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrLoadError[CCHMAXPATH];
  char startParams[CCHMAXPATH*4];
  char exename[CCHMAXPATH]={0};
  char chrCDRexe[CCHMAXPATH+10];
  struct stat statBuf;

  sprintf(exename,"%s\\bin\\%s",params[1],wrapperExe);
  
  /* Check if the VIO helper is in the bin dir */
  if(stat(exename , &statBuf)==-1) {
    /* text: "Can't find the helper program %s. It should be in the installation directory of Audio/Data-CD-Creator."
       title: "Audio/Data-CD-Creator installation problem!"
       */
    getMessage(chrLoadError, IDSTR_INSTALLNOVIO, sizeof(chrLoadError), RESSOURCEHANDLE , hwnd);
    sprintf(startParams,chrLoadError,wrapperExe);
    getMessage(chrLoadError, IDSTR_INSTALLERRORTITLE, sizeof(chrLoadError), RESSOURCEHANDLE , hwnd);
    WinMessageBox(HWND_DESKTOP,0,
                  startParams,
                  chrLoadError,12345,
                  MB_OK|MB_MOVEABLE|MB_ERROR);
    writeLog("\nCan't find vio helper '");
    writeLog(exename);  
    writeLog("'\n");
    return -1;
  }
  
  writeLog("\nStarting vio helper '");
  writeLog(exename);  
  writeLog("'\nWith the following parameters:\n");

  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_BACK;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;
      
  startData.PgmName=exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_WINDOWABLEVIO;
  startData.PgmControl=0;
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

  /* Put the exe-path between " " to make sure, spaces are handled correctly */
  sprintf(chrCDRexe,"\"%s\"", chrCDRecord);
  if(strlen(folderPath)!=0)
    sprintf(startParams,"%d \"%s\" \"%s\" \"%s\" %s",hwnd,chrCDRexe,folderPath ,chrDVDDao, parameter);
  else
    sprintf(startParams,"%d  \"%s\" \"%s\" %s",hwnd,chrCDRexe, chrDVDDao, parameter);
  startData.PgmInputs=startParams;

  writeLog("    ");
  writeLog(startParams);
  writeLog("\n\n");
 
  rc=DosStartSession(&startData,&ulSessionID,&pid);   
  if(rc!=NO_ERROR) {
    writeLog("Error while starting VIO helper: ");
    sprintf(chrLoadError,"%d\n",rc);
    writeLog(chrLoadError);
    return -1;
  }
  return 0;   
}

ULONG launchCmdExe( PSZ wrapperExe, PSZ parameter, PSZ folderPath, PSZ pszTitle)
{
  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrLoadError[CCHMAXPATH];
  char startParams[CCHMAXPATH*4];
  char exename[CCHMAXPATH]={0};
  char chrCDRexe[CCHMAXPATH+10];
  struct stat statBuf;

  strcpy(exename,wrapperExe);
  
  writeLog("\nStarting '");
  writeLog(exename);
  writeLog("' with the following parameters:\n");
 

  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_BACK;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;
  startData.PgmName=exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_WINDOWABLEVIO;
  startData.PgmControl=0;
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

  startData.PgmInputs=parameter;

  writeLog("\n\n");
 
  rc=DosStartSession(&startData,&ulSessionID,&pid);   
  if(rc!=NO_ERROR) {
    writeLog("Error while starting: ");
    sprintf(chrLoadError,"%d\n",rc);
    writeLog(chrLoadError);
    return -1;
  }
  return 0;   
}

BOOL readWindowPosFromIni(char * installDir, char *chrKey)
{
  char profileName[CCHMAXPATH];
  char moduleName[CCHMAXPATH];
  HINI hini=0;
  ULONG ulSize;
  BOOL bError=FALSE;
  HWND hwnd;

  hwnd=HWND_DESKTOP;

  strncpy(chrInstallDir, installDir,sizeof(chrInstallDir));
  /* Build full path for cdrecord.ini file */
  sprintf(profileName,"%s\\cdrecord.ini", installDir);       
  /* Insert message in Logfile */
  writeLog("Reading Window position from ");
  writeLog(profileName);
  writeLog("\n");

  /* Open ini-file */
  hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),(unsigned char *)profileName);
  do{
    if(!hini) {
      /* profileName: "Warning! Cannot open Ini-file!"
         moduleName: "Data-CD-Creator"
         */
      messageBox( profileName, IDSTR_INIFILEOPENWARNING , sizeof(profileName),
                  moduleName, IDSTRD_DATACDCREATOR, sizeof(moduleName),
                  RESSOURCEHANDLE, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
      break;
    }/* end of if(!hini) */

    ulSize=sizeof(swpWindow); 
    if(!PrfQueryProfileData(hini,"windowposition", chrKey, &swpWindow, &ulSize))
      bError=TRUE;

    if(hini)
      PrfCloseProfile(hini);
    
    if(bError)
      break;
    return TRUE;
  } while(TRUE);
  writeLog("Error while reading cdrecord.ini\n");
  return FALSE;
}

BOOL writeWindowPosToIni(char * installDir, char *chrKey)
{
  char profileName[CCHMAXPATH];
  char moduleName[CCHMAXPATH];
  HINI hini=0;
  ULONG ulSize;
  BOOL bError=FALSE;
  HWND hwnd;

  hwnd=HWND_DESKTOP;

  strncpy(chrInstallDir, installDir,sizeof(chrInstallDir));
  /* Build full path for cdrecord.ini file */
  sprintf(profileName,"%s\\cdrecord.ini", installDir);       
  /* Insert message in Logfile */
  writeLog("Writing Window position to ");
  writeLog(profileName);
  writeLog("\n");

  /* Open ini-file */
  hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),(unsigned char *)profileName);
  do{
    if(!hini) {
      /* profileName: "Warning! Cannot open Ini-file!"
         moduleName: "Data-CD-Creator"
         */
      messageBox( profileName, IDSTR_INIFILEOPENWARNING , sizeof(profileName),
                  moduleName, IDSTRD_DATACDCREATOR, sizeof(moduleName),
                  RESSOURCEHANDLE, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
      break;
    }/* end of if(!hini) */

    if(!PrfWriteProfileData(hini,"windowposition", chrKey, &swpWindow, sizeof(swpWindow)))
      bError=TRUE;

    if(hini)
      PrfCloseProfile(hini);
    
    if(bError)
      break;
    return TRUE;
  } while(TRUE);
  writeLog("Error while writing cdrecord.ini\n");
  return FALSE;
}


BOOL readIni2(char * installDir)
{
  ULONG keyLength;
  char profileName[CCHMAXPATH];
  char moduleName[CCHMAXPATH];
  char *chrPtr;
  char *chrPtr2;
  HINI hini=0;
  char    text[200];
  ULONG ulSize;
  int a;
  char chrCD[4];
  BOOL bError=FALSE;
  HWND hwnd;

  hwnd=HWND_DESKTOP;

  strncpy(chrInstallDir, installDir,sizeof(chrInstallDir));
  /* Build full path for cdrecord.ini file */
  sprintf(profileName,"%s\\cdrecord.ini", installDir);       
  /* Insert message in Logfile */
  writeLog("Reading values from ");
  writeLog(profileName);
  writeLog("\n");

  /* Open ini-file */
  hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),(unsigned char *)profileName);
  do{
    if(!hini) {
      /* profileName: "Warning! Cannot open Ini-file!"
         moduleName: "Data-CD-Creator"
         */
      messageBox( profileName, IDSTR_INIFILEOPENWARNING , sizeof(profileName),
                  moduleName, IDSTRD_DATACDCREATOR, sizeof(moduleName),
                  RESSOURCEHANDLE, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
      break;
    }/* end of if(!hini) */

    keyLength=PrfQueryProfileString(hini,"CDWriter","cdrecord","",chrCDRecord,sizeof(chrCDRecord));
    if(keyLength==1){
      /* Text: "No CDRecord/2 path in cdrecord.ini found!\n" */
      getMessage(text, IDSTRLOG_NOCDRECORD, sizeof(text), RESSOURCEHANDLE , hwnd);      
      writeLog(text);
      writeLog("\n");
      bError=TRUE;
      //  break;/* First opening. We havn't got entries yet */
    }

    ulSize=sizeof(lCDROptions); 
    PrfQueryProfileData(hini,"CDWriter","options",&lCDROptions,&ulSize);

    PrfQueryProfileString(hini,"CDWriter","audiocdroptions","",chrAudioCDROptions,sizeof(chrAudioCDROptions));    
    PrfQueryProfileString(hini,"CDWriter","cdroptions","",chrCDROptions,sizeof(chrCDROptions));    

    ulSize=sizeof(lCDROptions); 
    PrfQueryProfileData(hini,"CDWriter","options",&lCDROptions,&ulSize);
    keyLength=PrfQueryProfileString(hini,"Mkisofs","mkisofs","",chrMkisofs,sizeof(chrMkisofs));
    if(keyLength==1){
      /* Text: "No mkisofs path in cdrecord.ini found!\n" */
      getMessage(text, IDSTRLOG_NOMKISOFS, sizeof(text), RESSOURCEHANDLE , hwnd);      
      writeLog(text);
      writeLog("\n");
      bError=TRUE;
      //break;/* First opening. We havn't got entries yet */
    }

    PrfQueryProfileString(hini,"Mkisofs","mkisofsoptions","",chrMkisofsOptions,sizeof(chrMkisofsOptions));
    //lMKOptions=PrfQueryProfileInt(hini,"Mkisofs","options",IDMK_HIDEWINDOW|IDMK_CLOSEWINDOW);
    ulSize=sizeof(lMKOptions);
    PrfQueryProfileData(hini,"Mkisofs","options",&lMKOptions,&ulSize);   

    keyLength=PrfQueryProfileString(hini,"CDGrabber","grabber","",chrGrabberPath,sizeof(chrGrabberPath));
    if(keyLength==1){
      /* Text: "No grabber path in cdrecord.ini found!\n" */
      getMessage(text, IDSTRLOG_NOGRABBER, sizeof(text), RESSOURCEHANDLE , hwnd);      
      writeLog(text);
      writeLog("\n");
      bError=TRUE;
      //break;/* We havn't got entries yet */
    }
    PrfQueryProfileString(hini,"CDGrabber","graboptions","",chrGrabberOptions,sizeof(chrGrabberOptions));
    PrfQueryProfileString(hini,"CDGrabber","grabdrive","",chosenCD,sizeof(chosenCD));
    bTrackNumbers=PrfQueryProfileInt(hini,"CDGrabber","tracknumbers",1);
    iGrabberID=PrfQueryProfileInt(hini,"CDGrabber","ID", IDGRABBER_UNKNOWN);/* 99: unknown */

    keyLength=PrfQueryProfileString(hini,"mpg123","path","",chrMpg123Path,sizeof(chrMpg123Path));
    if(keyLength==1){
      /* Text: "No mpg123 path in cdrecord.ini found!\n" */
      getMessage(text, IDSTRLOG_NOMPG123, sizeof(text), RESSOURCEHANDLE , hwnd);      
      writeLog(text);
      writeLog("\n");
      bError=TRUE;
      //      break;/* We havn't got entries yet */
    }
    bMpg123SwabBytes=PrfQueryProfileInt(hini,"mpg123","swabbytes",1);
    iMp3Decoder=PrfQueryProfileInt(hini,"mpg123","decoder",IDKEY_USEMPG123);/* Which decoder to use */

    keyLength=PrfQueryProfileString(hini,"cdrdao","path","",chrCdrdaoPath,sizeof(chrCdrdaoPath));
    if(keyLength==1){
      /* Text: "No cdrdao/2 path in cdrecord.ini found!\n" */
      getMessage(text, IDSTRLOG_NOCDRDAO, sizeof(text), RESSOURCEHANDLE , hwnd);      
      writeLog(text);
      writeLog("\n");
      bError=TRUE;
      //      break; /* We havn't got entries yet */
    }

    keyLength=PrfQueryProfileString(hini,"cdrdao","driver","",chrCdrdaoDriver,sizeof(chrCdrdaoDriver));
    if(keyLength==1){
      /* Text: "No driver for cdrdao/2 found in cdrecord.ini!\n" */
      getMessage(text, IDSTRLOG_NOCDRDAODRIVER, sizeof(text), RESSOURCEHANDLE , hwnd);      
      writeLog(text);
      writeLog("\n");
      bError=TRUE;
     /* We havn't got entries yet */
      //break;
    }

    /* DVD stuff */
    PrfQueryProfileString(hini,"dvdwriting","writerpath","",chrDVDDao, sizeof(chrDVDDao));

    PrfQueryProfileString(hini,"isofs","mountpath","",chrMntIsoFS,sizeof(chrMntIsoFS));
    PrfQueryProfileString(hini,"isofs","unmountpath","",chrUmntIso, sizeof(chrUmntIso));
    
    iBus=PrfQueryProfileInt(hini,"device","bus",0);
    iTarget=PrfQueryProfileInt(hini,"device","target",0);
    iLun=PrfQueryProfileInt(hini,"device","lun",0);
    iSpeed=PrfQueryProfileInt(hini,"device","speed",1);
    iFifo=PrfQueryProfileInt(hini,"device","fifo",4);

    bUseCDDB=PrfQueryProfileInt(hini,"cddb","usecddb",0);
    bTipsEnabled=PrfQueryProfileInt(hini,"tips","enable",1);
    bUseCustomPainting=PrfQueryProfileInt(hini,"newlook", "enable",1);

    if(hini)
      PrfCloseProfile(hini);

    if(bError)
      break;
    return TRUE;
  } while(TRUE);
  writeLog("Error while reading cdrecord.ini\n");
  return FALSE;
}

BOOL readIni()
{
  return readIni2(params[1]);
}

static void _HlpBuildProfileName(char* chrBuffer, int iBufferSize)
{
  /* Build full path for cdrecord.ini file */
  snprintf(chrBuffer, iBufferSize, "%s\\cdrecord.ini", chrInstallDir);       
  chrBuffer[iBufferSize-1]=0; /* Always terminate with zero */
}

HINI HlpOpenIni()
{
  char profileName[CCHMAXPATH];

  /* Build full path for cdrecord.ini file */
  _HlpBuildProfileName(profileName, sizeof(profileName));

  /* Open ini-file */
  return PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),(unsigned char *)profileName);
}

void HlpCloseIni(HINI hini)
{
    if(hini)
      PrfCloseProfile(hini);
}

ULONG queryFreeCDSpaceError(HWND hwnd) {
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];

  /* Text: "Can't get free CD space!" */
  getMessage(text, IDSTRLB_CDSPACEERROR, sizeof(text), RESSOURCEHANDLE , hwnd);
  WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_INSERTITEM,MPFROMSHORT(0),text);            

  DosBeep(100,300);
  /* Title: "Writing CD"
     Text: "Can't query free CD space! On some systems detection of free CD space fails 
     so you may override this message if you know what you're doing! Do you want to proceed with writing? "
     */
  writeLog("");
  writeLog(text);
  writeLog("\n");
  return messageBox( text, IDSTRPM_CDSPACEERROR , sizeof(text),
                     title, IDSTRPM_DAOTITLE , sizeof(title),
                     RESSOURCEHANDLE, hwnd, MB_YESNO | MB_ICONEXCLAMATION|MB_MOVEABLE);
  //WinPostMsg(hwnd,WM_QUIT,0,0);
}

BOOL queryFreeCDSpace2(HWND hwnd, char * chrDev) {
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];

  /* Text: "Querying free CD space. Please wait..." */
  getMessage(text, IDSTRD_QUERYFREECDSPACE, sizeof(text), RESSOURCEHANDLE , hwnd);
  writeLog("\n");
  writeLog(text);
  writeLog("\n");
  /* Insert text into listbox */     
  WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_INSERTITEM,MPFROMSHORT(0),
             text);        

  /* Launch the helper */
  /* Title: "Query CD size" */
  getMessage(title, IDSTRVIO_CDSIZE, sizeof(title), RESSOURCEHANDLE , hwnd);
  return launchWrapper(chrDev, "", hwnd,"cdsize.exe",title);
  // return FALSE;
}

BOOL queryFreeCDSpace(HWND hwnd) {
  char text[CCHMAXPATH];

  sprintf(text,"dev=%d,%d,%d",iBus,iTarget,iLun);

  return queryFreeCDSpace2(hwnd,(char*) &text);
}

BOOL queryFreeDVDSpace(HWND hwnd, char * chrDev) {
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];

  /* Text: "Querying free DVD space. Please wait..." */
  getMessage(text, IDSTRD_QUERYFREEDVDSPACE, sizeof(text), RESSOURCEHANDLE , hwnd);
  writeLog("\n");
  writeLog(text);
  writeLog("\n");
  /* Insert text into listbox */     
  WinSendMsg(WinWindowFromID(hwnd,IDLB_CHECKSTATUS),LM_INSERTITEM,MPFROMSHORT(0),
             text);

  /* Launch the helper */
  /* Title: "Query CD size" */
  getMessage(title, IDSTRVIO_CDSIZE, sizeof(title), RESSOURCEHANDLE , hwnd);
  //writeLog(chrDev);
  return launchDVDWrapper(chrDev, "", hwnd,"dvdsize.exe", title);
  // return FALSE;
}


void errorResource2(char * chrTitle)
{

  WinMessageBox(HWND_DESKTOP,0,
                "The resource DLL which contains all the dialogs, graphics and messages cannot be loaded. \
Please check your installation. There must be a file CDFLDxxx.DLL in the installation directory of the \
CD-Creator package. xxx is the country code of your system e.g. 049 for Germany. If there is no \
support for your language there must be at least the file CDFLD001.DLL. The CD creation features \
are not available!",
                chrTitle,12345,
                MB_OK|MB_MOVEABLE|MB_ERROR);
}

/* This function returns the module handle of the class dll */
HMODULE queryClassDLLModuleHandle(char *installDir)
{
  COUNTRYCODE country= {0};
  COUNTRYINFO countryInfo= {0};
  char path[CCHMAXPATH];  
  char buf[CCHMAXPATH];
  char* found;
  APIRET rc;
  ULONG ulInfoLen;
    
  if(!CLASSDLLHANDLE) {
    writeLog("Trying to find class DLL.\n");

    /* Get the country code of our system and load the  
       resource DLL with the right language */
    do {
      sprintf(path,"%s\\aucdfldr.dll", installDir);
      
      /* Insert message in Logfile */
      writeLog("Using the following DLL path: ");
      writeLog(path);
      writeLog("\n");

      //WinMessageBox(  HWND_DESKTOP,0, path, "Search DLL...", 0UL, MB_YESNO | MB_ICONEXCLAMATION|MB_MOVEABLE );
      rc=DosLoadModule(buf,sizeof(buf),path, &CLASSDLLHANDLE);
      if(rc==NO_ERROR) {
        writeLog("Class DLL loaded.\n");
        break;
      }

      /* Insert message in Logfile */
      writeLog("Class DLL not found.\n");
      writeLog("Can't load DLL!\n");
      CLASSDLLHANDLE=NULLHANDLE;
      
      break;
    }while(TRUE);
  }
  return CLASSDLLHANDLE;
}

void freeClassDLLHandle(void)
{
  if(CLASSDLLHANDLE)
    DosFreeModule(CLASSDLLHANDLE);

}

/* This function returns the module handle of our ressource dll */
HMODULE queryResModuleHandle()
{
  return queryResModuleHandle2(params[1]);
}

/* This function returns the module handle of our ressource dll */
HMODULE queryResModuleHandle2(char *installDir)
{
  COUNTRYCODE country= {0};
  COUNTRYINFO countryInfo= {0};
  char path[CCHMAXPATH];  
  char buf[CCHMAXPATH];
  char* found;
  APIRET rc;
  ULONG ulInfoLen;
    
  if(!RESSOURCEHANDLE) {
    writeLog("Trying to load ressource DLL.\n");

    /* Get the country code of our system and load the  
       resource DLL with the right language */
    do {
      rc=DosQueryCtryInfo(sizeof(countryInfo),&country,&countryInfo,&ulInfoLen);
      if(rc!=NO_ERROR) {
        /* Insert message in Logfile */
        writeLog(__FUNCTION__);
        writeLog(": Can't get country info. Ressource-DLL will not be loaded.\n");
        break;
      }
      sprintf(path,"%s", installDir);
      
      sprintf(buf,"\\cdfld%03d.dll",countryInfo.country);
      strcat(path,buf);
      /* Insert message in Logfile */
      writeLog("Using the following DLL path: ");
      writeLog(path);
      writeLog("\n");

      //WinMessageBox(  HWND_DESKTOP,0, path, "Search DLL...", 0UL, MB_YESNO | MB_ICONEXCLAMATION|MB_MOVEABLE );
      rc=DosLoadModule(buf,sizeof(buf),path, &RESSOURCEHANDLE);
      if(rc==NO_ERROR) {
        writeLog("Ressource DLL loaded.\n");
        break;
      }

      /* Insert message in Logfile */
      writeLog("Ressource-DLL for the current countrycode not found. Trying to load default one (CDFLD001.DLL).\n");
      
      /* NLS DLL not found. Try to load default */
      found=strrchr(path,'\\');
      if(!found)
        break;

      *found=0;
      sprintf(buf,"\\cdfld001.dll");
      strcat(path,buf);
      writeLog("Using the following DLL path: ");
      writeLog(path);
      writeLog("\n");

      rc=DosLoadModule(buf,sizeof(buf),path, &RESSOURCEHANDLE);
      if(rc!=NO_ERROR) {
        writeLog("Can't load DLL!\n");
        RESSOURCEHANDLE=NULLHANDLE;
      }
      else {
        writeLog("Ressource DLL loaded.\n");
      }
      break;
    }while(TRUE);
  }
  return RESSOURCEHANDLE;
}

void freeResHandle()
{
  if(RESSOURCEHANDLE)
    DosFreeModule(RESSOURCEHANDLE);
}

void sendCommand(char* command)
{
  HOBJECT hObject;
  char chrCommand[CCHMAXPATH];

  hObject=WinQueryObject(params[2]);
  sprintf(chrCommand,"Querying object pointer for: %s\n",params[2]);
  writeLog(chrCommand);
  if(hObject!=NULLHANDLE) {
    strcpy(chrCommand,command);
    WinSetObjectData(hObject,chrCommand);
    writeLog("Sending command to folder: ");
    writeLog(command);
    writeLog("\n");
  }
  else {
    sprintf(chrCommand,"Can't query object pointer for: %s\n",params[2]);
    writeLog(chrCommand);
  }
}

void HlpSendCommandToObject(char* chrObject, char* command)
{
  HOBJECT hObject;
  char chrCommand[CCHMAXPATH];

  hObject=WinQueryObject(chrObject);
  sprintf(chrCommand,"Querying object pointer for: %s\n", chrObject);
  writeLog(chrCommand);
  if(hObject!=NULLHANDLE) {
    strcpy(chrCommand,command);
    WinSetObjectData(hObject,chrCommand);
    writeLog("Sending command to object: ");
    writeLog(command);
    writeLog("\n");
  }
  else {
    sprintf(chrCommand,"Can't query object pointer for: %s\n", chrObject);
    writeLog(chrCommand);
  }
}

void HlpOpenWebPage(char* chrUrl)
{
  HOBJECT hObject;
  char tempDir[CCHMAXPATH];
  char setup[CCHMAXPATH*2];

  if(HlpQueryTempDir(chrInstallDir, tempDir, sizeof(tempDir)))
    {
      snprintf(setup, sizeof(setup), "LOCATOR=%s;%s", chrUrl, "OPEN=DEFAULT");
      WinCreateObject("WPUrl","tempUrl", setup, tempDir, 
                      CO_UPDATEIFEXISTS);
        /* WinDestroyObject(hObject); Don't use it, blocks PM */
    }
}


ULONG SysQueryFreeDriveSpace(ULONG ulDriveNum, ULONG *ulTotal, ULONG * ulFree, ULONG* ulBytesUnit)
{
  FSALLOCATE fsa;
  ULONG rc;

  if(!ulTotal || !ulFree || !ulBytesUnit)
    return 1;

  if( ulDriveNum<1 || ulDriveNum > 26)
    return ERROR_INVALID_DRIVE;

  if((rc=DosQueryFSInfo(ulDriveNum, FSIL_ALLOC, &fsa, sizeof(FSALLOCATE)))!=NO_ERROR)
    return rc;

  *ulTotal=fsa.cUnit;
  *ulFree=fsa.cUnitAvail;
  *ulBytesUnit=fsa.cbSector*fsa.cSectorUnit;

  return(NO_ERROR);
}



