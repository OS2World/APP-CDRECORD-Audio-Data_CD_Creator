/*
 * Copyright (c) Chris Wohlgemuth 1999-2005
 * All rights reserved.
 *
 * http://www.os2world.com/cdwriting
 * http://www.geocities.com/SiliconValley/Sector/5785/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The authors name may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "audiofolder.hh"
#include "audiofolderhelp.h"

extern HMODULE hSettingsResource;
extern GLOBALDATA globalData;

extern char chrInstallDir[CCHMAXPATH];
extern char chrCdrdaoPath[CCHMAXPATH];

/* extern */
MRESULT EXPENTRY settingsGeneralOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsGeneralOption2DlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsCdrdaoOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsCdrdaoOptionDlgProcExpert(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsCdrdaoOptionDlgProc2(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsCdrdaoOptionDlgProc2Expert(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsCdrdaoOptionDlgProc3(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsCdrdaoOptionDlgProc3Expert(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsGrabberOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsGrabberExpertOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsMpg123OptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsCdrecordOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsMkisofsOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsCDDBOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsCDDBOptionDlgProc2(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;
MRESULT EXPENTRY settingsToolbarOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY settingsHintOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY settingsMP3EncoderOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY settingsMP3EncoderOptionDlgProc2(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY settingsAboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY settingsDVDDaoOptionDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
extern BOOL checkFileExists(char* chrFileName);
SOMClass* cwGetSomClass(char* chrClassName);

/* local */

#if 0
/********************************************************/
/* New class function which inserts the general options */
/* page 2 into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddIsoFSOptionPage(HWND hwndNotebook)
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
  pageinfo.pfnwp = settingsIsoFSOptionDlgProc;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  //The ID of the dialog template
  pageinfo.dlgid = IDDLG_ISOFSOPTIONS;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  //pageinfo.idDefaultHelpPanel = IDDLG_GENERAL2PAGE;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "ISO filesystem" */
  getMessage(pageName, IDSTRSETTING_ISOFS, sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}
#endif

/********************************************************/
/* New class function which inserts the about page      */
/* into the settings notebook                           */
/********************************************************/
BOOL CWCreatorSettings::cwAddAboutPage(HWND hwndNotebook)
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
  pageinfo.pfnwp = settingsAboutDlgProc;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  if(this->somIsA(cwGetSomClass("CWObject")))
    //The ID of the dialog template
    pageinfo.dlgid = IDDLG_ADCINTRO;
  else
    pageinfo.dlgid = IDDLG_ADCINTRO_NOWIZARD;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = 350;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "~About" */
  getMessage(pageName, IDSTRSETTING_ABOUTPAGE, sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the General options */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddGeneralOptionPage(HWND hwndNotebook)
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
  pageinfo.pfnwp = settingsGeneralOptionDlgProc;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  //The ID of the dialog template
  //  if(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE)
  //   pageinfo.dlgid = IDDLG_GENERAL_EXPERT;
  //else
  pageinfo.dlgid = IDDLG_GENERAL;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDDLG_GENERAL;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "~Writer Setup" */
  getMessage(pageName, IDSTRSETTING_WRITERSETUP, sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the general options */
/* page 2 into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddGeneralOptionPage2(HWND hwndNotebook)
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
  pageinfo.pfnwp = settingsGeneralOption2DlgProc;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  //The ID of the dialog template
  pageinfo.dlgid = IDDLG_GENERAL2PAGE;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDDLG_GENERAL2PAGE;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "General" */
  getMessage(pageName, IDSTRSETTING_GENERAL, sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}


/********************************************************/
/* New class function which inserts the General options */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddToolbarOptionPage(HWND hwndNotebook)
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
  pageinfo.pfnwp = settingsToolbarOptionDlgProc;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  //The ID of the dialog template
  pageinfo.dlgid = IDDLG_TOOLBARSETTINGS;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = 2000;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "General" */
  getMessage(pageName, IDSTRSETTING_TOOLBAR, sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;

  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the hint options    */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddHintOptionPage(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
    char pageName[100];

  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  pageinfo.usPageStyleFlags = BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = settingsHintOptionDlgProc;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  //The ID of the dialog template
  pageinfo.dlgid = IDDLG_HINTOPTIONS;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = 2010;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "ISO filesystem" */
  //getMessage(pageName, IDSTRSETTING_ISOFS, sizeof(pageName),  hSettingsResource, hwndNotebook);
  getMessage(pageName, IDSTRSETTING_TOOLBAR, sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the Cdrecord option */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddCdrecordOptionPage(HWND hwndNotebook)
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
  pageinfo.pfnwp = settingsCdrecordOptionDlgProc;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  //The ID of the dialog template
  pageinfo.dlgid = IDDLG_CDRECORDSETUP;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDDLG_CDRECORDSETUP;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "CDRecord/2" */
  getMessage(pageName, IDSTRSETTING_CDRSETTINGS, sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}


/*
  This checks if the cdrdao path is valid and pointing to the provided
  cdrdao.exe (the one coming with adc).
 */
static BOOL cdrdaoPathValid(char *thePath)
{
  char tempPath[CCHMAXPATH];

  sprintf(tempPath, "%s\\bin\\cdrdao.exe", chrInstallDir);
  if(!stricmp(tempPath, thePath))
    return TRUE;

  return FALSE;
}


/********************************************************/
/* New class function which inserts the cdrdao          */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddCdrdaoOptionPage(HWND hwndNotebook)
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
  pageinfo.pfnwp = settingsCdrdaoOptionDlgProcExpert;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  //The ID of the dialog template
  pageinfo.dlgid = IDDLG_CDRDAOSETUPEXPERT;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDHLP_CDRDAOSETUPEXPERT;

  /* Default cdrdao/2 and not expert mode. Show a settings page
     without path selection.*/
  if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE) && 
     cdrdaoPathValid(chrCdrdaoPath))
    {
      pageinfo.dlgid = IDDLG_CDRDAOSETUP;
      pageinfo.idDefaultHelpPanel = IDHLP_CDRDAOSETUP;
      pageinfo.pfnwp = settingsCdrdaoOptionDlgProc;
    }

  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "cdrdao/2" */
  getMessage(pageName, IDSTRSETTING_CDRDAOSETTINGS, sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the cdrdao          */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddCdrdaoOptionPage2(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
  char pageName[100];

  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  pageinfo.usPageStyleFlags =  BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = settingsCdrdaoOptionDlgProc2Expert;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  //The ID of the dialog template
  pageinfo.dlgid = IDDLG_CDRDAOSETUP2_EXPERT;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDHLP_CDRDAO2SETUP;

  /* Default cdrdao/2 and not expert mode. Show a settings page
     without path selection.*/
  if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE) && 
     cdrdaoPathValid(globalData.chrCdrdaoPath2))
    {
      pageinfo.dlgid = IDDLG_CDRDAOSETUP2;
      pageinfo.idDefaultHelpPanel = IDHLP_CDRDAO2SETUP;
      pageinfo.pfnwp = settingsCdrdaoOptionDlgProc2;
    }

  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "cdrdao/2" */
  //getMessage(pageName, IDSTRSETTING_CDRDAOSETTINGS, sizeof(pageName),  hSettingsResource, hwndNotebook);
  //pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the cdrdao          */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddCdrdaoOptionPage3(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
  char pageName[100];

  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  pageinfo.usPageStyleFlags = BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = settingsCdrdaoOptionDlgProc3Expert;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  //The ID of the dialog template
  pageinfo.dlgid = IDDLG_CDRDAOSETUP3_EXPERT; /* No separate help panel yet!!! */
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDHLP_CDRDAO3SETUP;
  //Tell the WPS the help library name

  /* Default cdrdao/2 and not expert mode. Show a settings page
     without path selection.*/
  if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE) && 
     cdrdaoPathValid(globalData.chrCdrdaoPath3))
    {
      pageinfo.dlgid = IDDLG_CDRDAOSETUP3;
      pageinfo.idDefaultHelpPanel = IDHLP_CDRDAO3SETUP;
      pageinfo.pfnwp = settingsCdrdaoOptionDlgProc3;
    }

  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "cdrdao/2" */
  //  getMessage(pageName, IDSTRSETTING_CDRDAOSETTINGS, sizeof(pageName),  hSettingsResource, hwndNotebook);
  //pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

  
/********************************************************/
/* New class function which inserts the Grabber option  */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddGrabOptionPage(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
  char pageName[100];
      
  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  pageinfo.usPageStyleFlags =  BKA_MAJOR | BKA_STATUSTEXTON;
  //pageinfo.usPageStyleFlags = BKA_MAJOR | BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = settingsGrabberExpertOptionDlgProc;
  //The resource DLL
  //pageinfo.resid = queryModuleHandle();
  pageinfo.resid = hSettingsResource;
  pageinfo.dlgid = IDDLG_GRABBERSETUP_EXPERT;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDHLP_GRABBERSETUP_EXPERT;
  if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE) && 
     checkFileExists(globalData.chrGrabberPath))
    {
      char *chrPtr;
      if((chrPtr=strrchr(globalData.chrGrabberPath, '\\'))!=NULLHANDLE) {
        chrPtr++;
        if(!strnicmp(chrPtr, "cdda2wav.exe", strlen("cdda2wav.exe"))) {
          /* cdda2wav selected as grabber */
          pageinfo.dlgid = IDDLG_GRABBERSETUP;
          pageinfo.idDefaultHelpPanel = IDHLP_GRABBERSETUP;
          pageinfo.pfnwp = settingsGrabberOptionDlgProc;
        }
      }
    }
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "Grabber" */
  getMessage(pageName, IDSTRSETTING_GRABBER, sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;

  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the mpg123 option   */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddMpg123OptionPage(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
  char pageName[100];

  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  // pageinfo.usPageStyleFlags =  BKA_STATUSTEXTON;
  pageinfo.usPageStyleFlags = BKA_MAJOR | BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = settingsMpg123OptionDlgProc;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  pageinfo.dlgid = IDDLG_MPG123SETUP;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;

  //The ID of the help panel for this page
  if(!(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE) && 
     (globalData.ulGlobalFlags & GLOBALFLAG_MMIOMP3INSTALLED))
    pageinfo.idDefaultHelpPanel = IDHLP_MP3DECODERSETUP;
  else
    pageinfo.idDefaultHelpPanel = IDHLP_MP3DECODERSETUP_EXPERT;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "MP3 decoder" */
  getMessage(pageName, IDSTRSETTING_MPG123, sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the mkisofs setup */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddMkisofsOptionPage(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
  char pageName[100];

  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  //pageinfo.usPageStyleFlags = BKA_STATUSTEXTON;
  pageinfo.usPageStyleFlags = BKA_MAJOR | BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = settingsMkisofsOptionDlgProc;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  pageinfo.dlgid = IDDLG_MKISOFSSETUP;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDDLG_MKISOFSSETUP;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName:  "Mkisofs"*/
  getMessage(pageName, IDSTRSETTING_MKISOFSSETTINGS,sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the CDDB setup      */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddCDDBOptionPage(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
  char pageName[100];

  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  //pageinfo.usPageStyleFlags = BKA_STATUSTEXTON;
  pageinfo.usPageStyleFlags = BKA_MAJOR | BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = settingsCDDBOptionDlgProc;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  pageinfo.dlgid = IDDLG_CDDBSETUP;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = 1300;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName:  "CDDB setup" */
  getMessage(pageName, IDSTRSETTING_CDDBSETTINGS,sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

/********************************************************/
/* New class function which inserts the CDDB setup      */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddCDDBOptionPage2(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
  char pageName[100];

  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  pageinfo.usPageStyleFlags = BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = settingsCDDBOptionDlgProc2;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  pageinfo.dlgid = IDDLG_CDDBSETUP2;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = 1307;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName:  "Mkisofs"*/
  /*getMessage(pageName, IDSTRSETTING_CDDBSETTINGS,sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  */
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}


BOOL CWCreatorSettings::cwAddMP3EncoderOptionPage(HWND hwndNotebook)
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
  pageinfo.pfnwp = settingsMP3EncoderOptionDlgProc;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  //The ID of the dialog template
  pageinfo.dlgid = IDDLG_MP3ENCODERSETUP;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  //  pageinfo.idDefaultHelpPanel = IDDLG_MP3ENCODERSETUP;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "MP3 encoder" */
  getMessage(pageName, IDSTRSETTING_MP3ENCODER, sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}

BOOL CWCreatorSettings::cwAddMP3EncoderOptionPage2(HWND hwndNotebook)
{
  PAGEINFO pageinfo;
    char pageName[100];

  //Clear the pageinfo structure
  memset((PCH)&pageinfo, 0, sizeof(PAGEINFO));
  //Fill the pageinfo structure
  pageinfo.cb = sizeof(PAGEINFO);
  pageinfo.hwndPage = NULLHANDLE;
  pageinfo.usPageStyleFlags = BKA_STATUSTEXTON;
  pageinfo.usPageInsertFlags = BKA_FIRST;
  //We want page numbers
  pageinfo.usSettingsFlags = SETTINGS_PAGE_NUMBERS;
  //The dialog procedure for this page
  pageinfo.pfnwp = settingsMP3EncoderOptionDlgProc2;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  //The ID of the dialog template
  pageinfo.dlgid = IDDLG_MP3ENCODER2SETUP;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  pageinfo.idDefaultHelpPanel = IDDLG_MP3ENCODER2SETUP;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "MP3 encoder" */
  //  getMessage(pageName, IDSTRSETTING_MP3ENCODER, sizeof(pageName),  hSettingsResource, hwndNotebook);
  // pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}


/********************************************************/
/* New class function which inserts the Cdrecord option */
/* page into the settings notebook                      */
/********************************************************/
BOOL CWCreatorSettings::cwAddDVDDaoOptionPage(HWND hwndNotebook)
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
  pageinfo.pfnwp = settingsDVDDaoOptionDlgProc;
  //The resource DLL
  pageinfo.resid = hSettingsResource;
  //pageinfo.resid = queryModuleHandle();
  //The ID of the dialog template
  pageinfo.dlgid = IDDLG_DVDDAOSETUP;
  //We need a pointer to our WPS-object in the dialog procedure
  //to call class functions
  pageinfo.pCreateParams = this;
  //The ID of the help panel for this page
  //  pageinfo.idDefaultHelpPanel = IDDLG_DVDDAOSETUP;
  //Tell the WPS the help library name
  pageinfo.pszHelpLibraryName = AFHELPLIBRARY;
  //We have a major tab so we need a name
  /* pageName: "CDRecord/2" */
  getMessage(pageName, IDSTRSETTING_DVDDAOSETUP, sizeof(pageName),  hSettingsResource, hwndNotebook);
  pageinfo.pszName = pageName;
  
  //Insert the page into the settings notebook
  return wpInsertSettingsPage(hwndNotebook,&pageinfo);
}
