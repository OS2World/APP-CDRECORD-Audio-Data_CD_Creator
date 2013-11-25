/*
 * This file is (C) Chris Wohlgemuth 1999-2003
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

#include "datafolder.h"
#include "audiofolder.hh"
#include "audiofolderhelp.h"

#include <stdlib.h>
#include <stdio.h>


PVOID pvSharedMem;

char chrImage[CCHMAXPATH]={0};/* Path to iso-image */
char lnchParam[1024];
extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern char chrMkisofs[CCHMAXPATH];

HMODULE hDataResource=NULLHANDLE;

/* Get strings for messagebox from resource-DLL */
ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);
ULONG showMsgBox2(HWND hwnd, ULONG ulIDTitle, ULONG ulIDText, HMODULE hModule, ULONG ulFlag);
void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
MRESULT EXPENTRY cdTypeOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY authorOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY cdrecordOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY mkisofsOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY fileNameOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY dataGeneralOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY specialOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY selectWriterDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void _Optlink imageWriteThreadFunc (void *arg);
void _Optlink onTheFlyThreadFunc (void *arg);
void _Optlink printSizeThreadFunc (void *arg);
void _Optlink resetArchiveBitThreadFunc(void *arg);
void _Optlink createImageThreadFunc (void *arg);
void SysWriteToTrapLog(const char* chrFormat, ...);

void buildDataWriteParam(CWDataFolder* thisPtr, char * text ,int iSpeedChosen, char *chrDev);
//void buildDataWriteParam(CWDataFolder* thisPtr, char * text);
ULONG launchPMWrapper(PSZ parameter, PSZ folderPath, PSZ wrapperExe, PSZ pszTitle);
BOOL buildBinFileName(char *chrBuffer);
BOOL checkFileExists(char* chrFileName);
void mboxShowMissingProgMsgBox(HWND hwnd, char * chrWhichProg, HMODULE hResource, BOOL bOpenSettings);
void HlpSendCommandToObject(char* chrObject, char* command);

static BOOL checkCDRecordExists(HWND hwnd, HMODULE hResource)
{

  if(checkFileExists(chrCDRecord))
    return TRUE;

  /* The path to CDRecord/2 isn't valid */
  mboxShowMissingProgMsgBox( hwnd, "CDRecord/2", hResource, TRUE);

  /* Show the cdrecord setup help panel */
  HlpSendCommandToObject("<CWCREATOR_SETTINGS>", SETUP_DISPLAYHELPPANEL"="IDHLP_CDRECORDSETUPSTR);

  return FALSE;
}

BOOL checkMkisofsExists(HWND hwnd, HMODULE hResource)
{

  if(checkFileExists(chrMkisofs))
    return TRUE;

  /* The path to mkisofs isn't valid */
  mboxShowMissingProgMsgBox(hwnd, "mkisofs", hResource, TRUE);

  /* Show the mkisofs setup help panel */
  HlpSendCommandToObject("<CWCREATOR_SETTINGS>", SETUP_DISPLAYHELPPANEL"="IDHLP_MKISOFSSETUPSTR);
  
  return FALSE;
}

BOOL checkImageName(CWDataFolder *thisPtr, HWND hwnd, BOOL bCheckExist)
{
  char  title[CCHMAXPATH];
  char  text2[CCHMAXPATH];
  BOOL bExists=TRUE;

  if(bCheckExist)
    bExists=checkFileExists(thisPtr->chrImageName);

  /* Check if we have an output filename */
  if(strlen(thisPtr->chrImageName)<=3 || !bExists) {
    /* 
       "You have to specify a valid filename including the path. Make sure the specified path exists."
       */
    messageBox( text2,IDSTRD_CHECKSIZEERRORFILENAME , sizeof(text2),
                title, IDSTRD_CREATEIMAGETITLE, sizeof(title),
                hDataResource, hwnd, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
    return FALSE;
  }
  /* Check if we have a full path (just a quick check) */
  if(thisPtr->chrImageName[1]!=':') {
    /* Text: "You have to specify a full path."
       Title: "Image file creation."
       */
    messageBox( text2, IDSTRD_CHECKSIZEERRORFULLPATH, sizeof(text2),
                title, IDSTRD_CREATEIMAGETITLE, sizeof(title),
                hDataResource, hwnd, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
    return FALSE;
  }
  return TRUE;
}

/********************************************************/
/* New class function which inserts the special option  */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWDataFolder::AddSpecialOptionPage(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
  char pageName[100];

  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  pageinfo.usPageStyleFlags = BKA_MAJOR | BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = specialOptionDlgProc;
  //The resource DLL
  pageinfo.resid = hDataResource;
  pageinfo.dlgid = IDDLG_DATAFOLDERSPECIAL;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDHLP_DATAFOLDERSPECIAL;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "Special" */
  getMessage(pageName, IDSTRSETTING_DATAFOLDERSPECIAL,sizeof(pageName),  hDataResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the filename option */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWDataFolder::AddFileNameOptionPage(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
  char pageName[100];

  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  pageinfo.usPageStyleFlags = BKA_MAJOR | BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = fileNameOptionDlgProc;
  //The resource DLL
  pageinfo.resid = hDataResource;
  pageinfo.dlgid = IDDLG_FILENAMEOPTIONS;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDDLG_FILENAMEOPTIONS;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "Filename options" */
  getMessage(pageName, IDSTRSETTING_FILENAME,sizeof(pageName),  hDataResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the author          */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWDataFolder::AddAuthorOptionPage(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
  char pageName[100];

  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  pageinfo.usPageStyleFlags = BKA_MAJOR | BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = authorOptionDlgProc;
  //The resource DLL
  pageinfo.resid = hDataResource;
  pageinfo.dlgid = IDDLG_AUTHOR;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDDLG_AUTHOR;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "CD name and author"*/
  getMessage(pageName, IDSTRSETTING_CDDESCRIPTION,sizeof(pageName),  hDataResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the CD-Type         */
/* page into the settings notebook                      */
/********************************************************/
ULONG CWDataFolder::AddCDTypeOptionPage(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
  char pageName[100];

  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  pageinfo.usPageStyleFlags = BKA_MAJOR | BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = cdTypeOptionDlgProc;
 //The resource DLL
  pageinfo.resid = hDataResource;
  pageinfo.dlgid = IDDLG_CDTYPE;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDDLG_CDTYPE;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "CD-Type selection" */
  getMessage(pageName, IDSTRSETTING_CDTYPE,sizeof(pageName),  hDataResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

void CWDataFolder::cwSetStatusText(PSZ pszText)
{
  if(pszText)
    WinSetWindowText(WinWindowFromID(hwndStatusCntrl,IDST_STATUSTOTALTIME),pszText);
  else
    WinSetWindowText(WinWindowFromID(hwndStatusCntrl,IDST_STATUSTOTALTIME),chrStatusText);
}

ULONG CWDataFolder::cwQueryMkisofsFlags()
{
  return ulMkisofsFlags;
}

ULONG CWDataFolder::cwSetMkisofsFlags(ULONG ulNewMkisofsFlags,ULONG ulMask)
{
  ULONG rc;

  rc=cwQueryMkisofsFlags();
  ulMkisofsFlags=((ulNewMkisofsFlags&ulMask) | (~ulMask & ulMkisofsFlags));

  return rc;/* return previous flags */
}

LONG CWDataFolder::cwQueryPreviousStartSector()
{
  return lPreviousStart;
}

LONG CWDataFolder::cwQueryNextStartSector()
{
  return lNextStart;
}

void CWDataFolder::cwSetPreviousStartSector(LONG lSector)
{
  lPreviousStart=lSector;
}

void CWDataFolder::cwSetNextStartSector(LONG lSector)
{
  lNextStart=lSector;
}

ULONG CWDataFolder::cwQueryCDTypeFlags()
{
  return ulCDTypeFlags;
}

ULONG CWDataFolder::cwSetCDTypeFlags(ULONG ulNewCDTypeFlags,ULONG ulMask)
{
  ULONG rc;

  rc=cwQueryCDTypeFlags();
  ulCDTypeFlags=((ulNewCDTypeFlags&ulMask) | (~ulMask & ulCDTypeFlags));

  return rc;/* return previous flags */
}

#if 0
void CWDataFolder::cwEnableMultiSessionCntrls(HWND hwnd, BOOL bEnable)
{
  /* hwnd:    Dialog-HWND
     nEnable: New enable state */

  WinEnableWindow(WinWindowFromID(hwnd,IDGB_MERGESESSION),bEnable);//Groupbox
  WinEnableWindow(WinWindowFromID(hwnd,IDST_MERGETEXT),bEnable);//Text

}
#endif

ULONG CWDataFolder::cwQueryCreateFlags()
{
  return ulCreateFlags;
}

ULONG CWDataFolder::cwSetCreateFlags(ULONG ulNewCreateFlags,ULONG ulMask)
{
  ULONG rc;

  rc=cwQueryCreateFlags();
  ulCreateFlags=((ulNewCreateFlags&ulMask) | (~ulMask & ulCreateFlags));

  return rc;/* return previous flags */
}

void CWDataFolder::cwEnableWriteControls(BOOL bEnable)
{
  RECTL rectl;

  if(!hwndMkisofsMain)
    return;
  WinEnableWindow(WinWindowFromID(hwndMkisofsMain,IDRB_WRITEIMAGE) ,bEnable);
  WinEnableWindow(WinWindowFromID(hwndMkisofsMain,IDRB_ONTHEFLY) ,bEnable);
  WinEnableWindow(WinWindowFromID(hwndMkisofsMain,IDRB_IMAGEONLY) ,bEnable);
  WinShowWindow(WinWindowFromID(hwndMkisofsMain,IDPB_CHECKSIZE) ,bEnable);
  WinShowWindow(WinWindowFromID(hwndMkisofsMain,IDPB_ABORTCHECKSIZE) ,!bEnable);
  WinEnableWindow(WinWindowFromID(hwndMkisofsMain,IDCB_DUMMY) ,bEnable);

  if(WinQueryButtonCheckstate(hwndMkisofsMain,IDRB_IMAGEONLY))
    WinEnableWindow(WinWindowFromID(hwndMkisofsMain,IDPB_CREATE) ,bEnable);
  else 
    WinEnableWindow(WinWindowFromID(hwndMkisofsMain,IDPB_WRITECD) ,bEnable);

  if(bEnable) {
    WinEnableWindow(WinWindowFromID(hwndMkisofsMain,IDPB_CREATE) ,bEnable);
    WinQueryWindowRect(hwndMkisofsMain,&rectl);
    WinInvalidateRect(hwndMkisofsMain,&rectl,TRUE);
  }
}

void CWDataFolder::cwShowImageName(BOOL bShow)
{
  WinEnableWindow(WinWindowFromID(hwndMkisofsMain,IDEF_IMAGENAME) ,bShow);
  WinEnableWindow(WinWindowFromID(hwndMkisofsMain,IDPB_IMAGEBROWSE) ,bShow);
  WinEnableWindow(WinWindowFromID(hwndMkisofsMain,IDST_IMAGENAME) ,bShow);
}



/* This method is called if we write an already created image */
void CWDataFolder::cwWriteImage()
{
  char * text;
  ULONG ulSize;
  char* tempText;
  /* For msgbox strings */
  char text2[CCHMAXPATH*2+10];
  char title[CCHMAXPATH];
  text=lnchParam;
  ULONG  ulFlags;
  ULONG ulError;
  FILE * file;
  DEVICEINFO *devInfo;
  int iSpeedLocal;
  HWND hwnd=HWND_DESKTOP;

  if(WinIsWindow(WinQueryAnchorBlock(HWND_DESKTOP), hwndMkisofsMain))
    hwnd=hwndMkisofsMain;

  if(!checkCDRecordExists(hwnd, hDataResource))
    return;

  /* Check if we have an output filename */
  if(!checkImageName(this, hwnd, TRUE))
    return;

  /* Check if userdefined CD with Fix unchecked */
  ulFlags=cwQueryCDTypeFlags();
  if(!(ulFlags & IDCDT_FIXDISK) && (ulFlags & IDCDT_USERDEFINED)) {
    /* Text: "Fix disk is unchecked!\n " */
    /* Title: "Write image." */
    getMessage(text2, IDSTRD_FIXUNCHECKED, sizeof(text2), hDataResource,HWND_DESKTOP);
    getMessage(title, IDSTRD_WRITEIMAGETITLE, sizeof(title), hDataResource,HWND_DESKTOP);
    if(WinMessageBox(  HWND_DESKTOP, WinQueryWindow(hwndMkisofsMain, QW_PARENT), text2,title,
                       0UL, MB_OKCANCEL | MB_ICONASTERISK|MB_MOVEABLE )==MBID_CANCEL)
      return;
  }

  /* Check if we perform a dummy write */
  if(sDummy!=1)
    {
      /* Text: "Dummy is unchecked!\n Do you want to perform a real write?" */
      /* Title: "Write image." */
      getMessage(text2, IDSTR_DUMMYUNCHECKED, sizeof(text2), hDataResource,HWND_DESKTOP);
      getMessage(title, IDSTRD_WRITEIMAGETITLE, sizeof(title), hDataResource,HWND_DESKTOP);
      if(WinMessageBox(  HWND_DESKTOP, WinQueryWindow(hwndMkisofsMain, QW_PARENT), text2,title,
                         0UL, MB_YESNO | MB_ICONEXCLAMATION|MB_MOVEABLE )==MBID_NO)
        return;
    }
  
  /* Get local mem */
  ulError=0;
  if((devInfo=(DEVICEINFO*)wpAllocMem(sizeof(DEVICEINFO), &ulError))==NULLHANDLE)
    return;
  memset(devInfo, 0, sizeof(DEVICEINFO));

  if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, selectWriterDialogProc, hDataResource, IDDLG_SELECTDEVICE, devInfo) == DID_ERROR )
    {
      wpFreeMem((PBYTE)devInfo);
      return;
    }  

  /* User selected a writer and speed */
  iSpeedLocal=devInfo->iSpeed;
  strncpy(title, devInfo->chrDev, sizeof(devInfo->chrDev));
  wpFreeMem((PBYTE)devInfo);

  /*  sprintf(text2,"speed: %d : %s",iSpeedLocal ,title);
      WinMessageBox(  HWND_DESKTOP, HWND_DESKTOP,
      text2,"Write image", 0UL, MB_OK | MB_ERROR |MB_MOVEABLE); 
      */
  
  /* Get local mem */
  ulError=0;
  if((text=wpAllocMem(SHAREDMEM_SIZE, &ulError))==NULLHANDLE)
    return;
  memset(text, 0, SHAREDMEM_SIZE);
    
  sprintf(text,"%s\"",chrCDRecord);/* The '"' marks the end of the cdrecord path */
  tempText=strchr(text,0);
  if(!tempText) {
    wpFreeMem(text);

    /* Text: "Internal error! Can't find end of command. Aborting..."
       Title: "Write image."
       */
    showMsgBox2(hwndMkisofsMain, IDSTRD_WRITEIMAGETITLE, IDSTR_CDRSTRINGERROR, hDataResource, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);

    return;
  }
  
  /* Build cdrecord/2 cmd-line parameters */  
  buildDataWriteParam(this, tempText+1, iSpeedLocal, title);

  *tempText=' ';
  /* Check if we perform a dummy write */
  if(sDummy==1)
    strcat(text," -dummy");// Dummy write

  /* Add the imagefile name */
  strcat(text," ");
  strcat(text, chrImageName);

  /* Start cdrecord */

  /* Text: "Writing image %s..." */
  getMessage(text2, IDSTRD_WRITINGIMAGE, sizeof(text2), hDataResource,HWND_DESKTOP);
  sprintf(chrStatusText, text2, chrImageName);

  /* Get a filename for the parameter file */  
  if(!buildBinFileName(title)) {
    wpFreeMem(text);
    return;
  }
  if((file=fopen(title,"wb"))==NULL) {
    wpFreeMem(text);
    return;
  }

  fwrite(text, sizeof(char), SHAREDMEM_SIZE, file);
  fclose(file);

  sprintf(text,"\"%s\" \"%s\"",chrImageName, title);

  ulSize=sizeof(title);
  wpQueryRealName(title, &ulSize, TRUE);
  /* This PM wrapper checks the image- and CD size and controls the status
     window */ 
  launchPMWrapper(text, title, "pwrteimg.exe", chrStatusText);
   wpFreeMem(text);
}


void changeBackslash(char* text)
{
  char * chrPtr;
  
  chrPtr=strchr(text,'\\');
  
  while(chrPtr!=NULL) {
    *chrPtr='/';
    chrPtr=strchr(text,'\\');
  }
}


/* This method starts a PM wrapper which handles all the on the fly stuff */
void CWDataFolder::cwWriteOnTheFly()
{
  ULONG ulFlags;
  ULONG ulSize;

  if(!checkCDRecordExists(hwndMkisofsMain, hDataResource) || !checkMkisofsExists(hwndMkisofsMain, hDataResource))
    return;

  /* Check if userdefined CD with Fix unchecked */
  ulFlags=cwQueryCDTypeFlags();
  if(!(ulFlags & IDCDT_FIXDISK) && (ulFlags & IDCDT_USERDEFINED)) {
    /* Text: "Fix disk is unchecked!\n " */
    /* Title: "Write image." */
    if(showMsgBox2(hwndMkisofsMain, IDSTRD_WRITEIMAGETITLE, IDSTRD_FIXUNCHECKED, 
                   hDataResource, MB_OKCANCEL | MB_ICONEXCLAMATION|MB_MOVEABLE)
       ==MBID_CANCEL)
      return;
  }

  /* First check the -dummy switch and ask the user */
  if(sDummy!=1)
    {
      /* Text: "Dummy is unchecked!\n Do you want to perform a real write?" */
      /* Title: "Write on the fly" */
      if(showMsgBox2(hwndMkisofsMain, IDSTRD_ONTHEFLYTITLE, IDSTR_DUMMYUNCHECKED, 
                     hDataResource, MB_YESNO | MB_ICONEXCLAMATION|MB_MOVEABLE)
         ==MBID_NO)
        return;
    }
  
  bStopCreateThread=FALSE;
  
  _beginthread(onTheFlyThreadFunc,NULL,8192*16,(void*)this); //Fehlerbehandlung fÅr Start fehlt  

  return;
}

void CWDataFolder::cwCreateImage()
{
  char text2[CCHMAXPATH];

  /* Check if path to mkisofs.exe is valid */
  if(!checkMkisofsExists(hwndMkisofsMain, hDataResource))
    return;

  /* Check if we have an output filename */
  if(!checkImageName(this, hwndMkisofsMain, FALSE))
    return;

  /* Text: "Creating image %s..." */
  getMessage(text2, IDSTRD_CREATINGIMAGE, sizeof(text2), hDataResource,HWND_DESKTOP);
  sprintf(chrStatusText, text2, chrImageName);
  cwSetStatusText(NULL);

  bStopCreateThread=FALSE;
  
  _beginthread(createImageThreadFunc,NULL,8192*16,(void*)this); //Fehlerbehandlung fehlt  
  
  return;
}

void CWDataFolder::cwGetImageSize()
{
  /* Check if path to mkisofs.exe is valid */
  if(!checkMkisofsExists(hwndMkisofsMain, hDataResource))
    return;

  /* Text: "Calculating image size. Please wait..." */
  getMessage(chrStatusText, IDSTRD_CALCULATINGIMAGESIZE, sizeof(chrStatusText), hDataResource,HWND_DESKTOP);
  cwSetStatusText(NULL);

  bStopCreateThread=FALSE;

  _beginthread(printSizeThreadFunc,NULL,8192*16,(void*)this); //Fehlerbehandlung fehlt    

}


void CWDataFolder::cwResetArchiveBit()
{

  _beginthread(resetArchiveBitThreadFunc,NULL,8192*16,(void*)this); //Fehlerbehandlung fehlt    

}












