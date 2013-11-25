/*
 * This file is (C) Chris Wohlgemuth 1999-2003
 *
 * It is part of the Audio/Data-CD-Creator package
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

#include "audiofolder.hh"
#include "audiofolderhelp.h"

#include <stdlib.h>
#include <stdio.h>

#include "menufolder.hh"

extern GLOBALDATA globalData;

PVOID pvAudioSharedMem=NULL;
BOOL setupDone=FALSE;
BOOL GrabberSetupDone=FALSE;
BOOL bGrabberFirst=FALSE;
BOOL bGotIniValues=FALSE;

int iNumCD;
char cFirstCD;
char chosenCD[3];
char chosenWriter[3];// Drive letter of CD-writer

int iMp3Decoder=IDKEY_USEMPG123; /* Which decoder to use */

char chrAudioCDROptions[CCHMAXPATH];

char chrCDRecord[CCHMAXPATH]={0};/* Path to cdrecord */
char chrDataCDROptions[CCHMAXPATH]={0};
LONG  lCDROptions=0;

char chrCdrdaoPath[CCHMAXPATH]={0};
char chrCdrdaoDriver[100]={0};
char chrDeviceName[CCHMAXPATH]={0};
char chrWriterName[CCHMAXPATH]={0};

extern char chrLBFontName[CCHMAXPATH];/* Font for grab listbox */

int iBus=0;
int iTarget=0;
int iLun=0;
int iSpeed=1;
int iFifo=4;

char chrInstallDir[CCHMAXPATH]={0};

ATOM atomUpdateStatusbar;
ATOM atomStartGrab;
ATOM atomStartWrite;
PFNWP pfnwpGenericFrame;

/* This is the handle for the ressource DLL holding the NLS messages and dialogs */
HMODULE hAudioResource=NULLHANDLE;

void _Optlink toolsThreadFunc (void *arg);
void _Optlink coverThreadFunc (void *arg);
void _Optlink aboutThreadFunc (void *arg);
void errorResource();
void HlpSendCommandToObject(char* chrObject, char* command);
void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);
SOMClass* cwGetSomClass(char* chrClassName);

ULONG launchDRDialog(PSZ parameter, PSZ resFile, PSZ pszTitle);

MRESULT EXPENTRY selectDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY statusLineDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY dialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY grabDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY audioContainerProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY audioFrameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT cwInsertMenuSeparator(int iPosition, HWND hwndMenu, HWND hwndSubMenu);
void addAudioFolderSettingsToFile(CWAudioFolder * thisPtr,   FILE *fHandle);
void addSettingsToFile(FILE *fHandle);
void SysWriteToTrapLog(const char* chrFormat, ...);

typedef ULONG   SOMLINK somTP_CWMenuFolder_mfInsertMenuItems(CWMenuFolder *somSelf,
		HWND hwndMenu,
		ULONG iPosition,
		ULONG ulLastMenuId);
//#pragma linkage(somTP_CWMenuFolder_mfInsertMenuItems, system)
typedef somTP_CWMenuFolder_mfInsertMenuItems *somTD_CWMenuFolder_mfInsertMenuItems;


#if 0
BOOL CWAudioFolder::wpRestoreString(PSZ pClass, ULONG ulKey, PSZ pszValue, PULONG pulValue)
{
  BOOL rc;
  char text[1000];

  if(pClass&&ulKey&&pszValue&&pulValue) {
    if (strcmp(pClass, "WPFolder") == 0)
      {
        /*      sprintf(text,"ulKey: %d; pValue: %s", ulKey, pszValue);
                WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,text,"wpRestoreString",1234, MB_OK|MB_MOVEABLE);
                */
        if((ulKey==2934||ulKey==2921) && pszValue) {
          // *pszValue=0;
          return FALSE;
        }
      }
  }
  rc=CWProgFolder::wpRestoreString( pClass, ulKey, pszValue, pulValue);
  return rc;
}
/*#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>*/
BOOL CWAudioFolder::wpSaveString(PSZ pClass, ULONG ulKey, PSZ pValue)
{
  BOOL rc;
  char text[1000];

  /*  if(ulKey==2934||ulKey==2921)
    */
  if (strcmp(pClass, "WPFolder") == 0)
    {
      /*      if(ulKey==2934||ulKey==2921)
        DosBeep(500,400);*/
      /* sprintf(text,"ulKey: %d; pValue: %s\n", ulKey,pValue);
      int hndl=open("D:\\save.txt",O_APPEND|O_CREAT|O_WRONLY,S_IWRITE);
      write(hndl,text,strlen(text));
      close(hndl);*/
      // WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,text,"wpSaveData",1234, MB_OK|MB_MOVEABLE);
      // cwSetStatusText(text);
    }

  rc=CWProgFolder::wpSaveString( pClass, ulKey, pValue);

  /*  if(!pClass||!ulKey||!pValue)
      return rc;
    */
  return rc;
}
PFNWP pfnwpGenericNB; 
PFNWP pfnwpBgNB;
MRESULT EXPENTRY bgNBProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  switch(msg)
    {
    case WM_DESTROY:
      if(WinQueryButtonCheckstate(hwnd,555)==1)
        DosBeep(500,500);
      else
        DosBeep(100,500);  
    default:
      break;
    }
  return  pfnwpBgNB(hwnd, msg, mp1, mp2);
}
MRESULT EXPENTRY audioNBProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  CWAudioFolder* thisPtr;
  char text[200];
  switch(msg)
    {
    case WM_DESTROY:
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,555),QWL_USER);// object ptr.
      if(somIsObj(thisPtr)) {
        thisPtr->ulPageID=0;
      }
      break;
    case BKM_SETPAGEWINDOWHWND:
      thisPtr=(CWAudioFolder*) WinQueryWindowULong(WinWindowFromID(hwnd,555),QWL_USER);// object ptr.
      if(somIsObj(thisPtr)) {
        if(thisPtr->ulPageID==LONGFROMMP(mp1)) {
          // DosBeep(500,1600);
          SWP swp={0};
          WinQueryWindowPos(WinWindowFromID(HWNDFROMMP(mp2),0x132),&swp);
          WinCreateWindow(HWNDFROMMP(mp2), WC_BUTTON, "Transparenz", 
                          WS_VISIBLE|BS_AUTOCHECKBOX, swp.cx+120, swp.y,100,swp.cy,HWNDFROMMP(mp2),HWND_TOP,555,NULL,NULL);
          pfnwpBgNB=WinSubclassWindow(HWNDFROMMP(mp2),bgNBProc);
        }
      }
      // sprintf(text,"HWND: %d", mp1);
      // WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,text,"Proc()",1234, MB_OK|MB_MOVEABLE);
      // DosBeep(5000,600);
      break;
    default:
      break;
    }
  return  pfnwpGenericNB(hwnd, msg, mp1, mp2);
}
ULONG CWAudioFolder::wpAddFolderBackgroundPage(HWND hwndNotebook)
{
  ULONG ulRC;
  PAGEINFO pageInfo;
  //  char text[200];
  HWND hwnd;
  ulPageID=0;
  ulRC=CWProgFolder::wpAddFolderBackgroundPage(hwndNotebook);
  /* ulRC: pageID */
  /*  hwnd=(HWND)WinSendMsg(hwndNotebook,BKM_QUERYPAGEWINDOWHWND,MPFROMLONG(ulRC),MPFROMLONG(0L));
      if(hwnd==NULLHANDLE)
      DosBeep(100,1600);
      else if(hwnd==BOOKERR_INVALID_PARAMETERS)
      DosBeep(500,600);
      */
  if((hwnd=WinCreateWindow(hwndNotebook, WC_BUTTON, "Obj_Ptr_Save",0,0,0,0,0,hwndNotebook,HWND_TOP,555,NULL,NULL))!=NULLHANDLE) {
    WinSetWindowULong(hwnd,QWL_USER,(ULONG)this);//Save object ptr.
    pfnwpGenericNB=WinSubclassWindow(hwndNotebook,audioNBProc);
    ulPageID=ulRC;
  }
  /*  sprintf(text,"HWND: %f", pageInfo.hwndPage);
        */
  /*
    sprintf(text,"HWND: %d", ulRC);
    DosBeep(5000,600);
    WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,text,"wpAddFolderBackgroundPage()",1234, MB_OK|MB_MOVEABLE);
    */
  return ulRC;
}
#endif



BOOL CWAudioFolder::wpSetupOnce(PSZ pSetupString)
{
  /**************************************************
   *                                                *
   * Supported setupstrings:                        *
   *       FLDRWRITEFLAGS=1+2+4+8                   *
   *                                                *
   **************************************************/
  BOOL rcParent=FALSE;
  char buffer[CCHMAXPATH];
  ULONG bufferSize;
  int iIndex;
  M_CWAudioFolder* m_CWAudiof=(M_CWAudioFolder*)somGetClass();

  rcParent=CWProgFolder::wpSetupOnce(pSetupString);

  /* Workaround to prevent the WPS from unloading our class. */
  m_CWAudiof->wpclsIncUsage();

  /* Doing our own setup here if not done by the user. */
  bufferSize=sizeof(buffer);
  if(!wpScanSetupString(pSetupString,"ICONVIEW",buffer,&bufferSize))
    wpSetup( "ICONVIEW=NONFLOWED,MINI");/* Fill in defaults */

  bufferSize=sizeof(buffer);
  if(!wpScanSetupString(pSetupString,"DEFAULTVIEW",buffer,&bufferSize))
    wpSetup( "DEFAULTVIEW=DETAILS");/* Fill in defaults */

  /* Size and position of the folder window */
  bufferSize=sizeof(buffer);
  if(!wpScanSetupString(pSetupString,"ICONVIEWPOS",buffer,&bufferSize))
    wpSetup( "ICONVIEWPOS=20,10,60,75");/* Fill in defaults */
  
  /* The default write flags  */
  bufferSize=sizeof(buffer);
  if(!wpScanSetupString(pSetupString,"FLDRWRITEFLAGS",buffer,&bufferSize))
    ulWriteFlags=IDWF_PAD|IDWF_NOFIX|IDWF_DUMMY; /* No key, set defaults */


  /* Set folder details- and sortclass to CWAudio so track infos are shown */
  bufferSize=sizeof(buffer);
  if(!wpScanSetupString(pSetupString,"DETAILSCLASS",buffer,&bufferSize)) {
    wpSetup("DETAILSCLASS=CWAudio");/* Fill in defaults */
    /* Hide some folder columns. This will not always work as desired if for
       example the folder is already subclassed and new columns are added by the superclass.
       In that case the index is not correct. For now it works sufficently well. */
    //    for(iIndex=3;iIndex<=4 ;iIndex++)
    //   wpSetDetailsColumnVisibility( iIndex, FALSE);
  }

  bufferSize=sizeof(buffer);
  if(!wpScanSetupString(pSetupString,"SORTCLASS",buffer,&bufferSize)) {
    wpSetup("SORTCLASS=CWAudio");/* Fill in defaults */
#if 0
    for(iIndex=0; iIndex<=100; iIndex++)
      wpSetSortAttribAvailable(iIndex, FALSE);
    DosBeep(500,400);
#endif

    /* This works only on Warp 4 and above */
    wpSetup("SORTBYATTR=24,25,26,27,28,29,30,31,32,33");/* Fill in defaults */
  }

  bufferSize=sizeof(buffer);
  if(!wpScanSetupString(pSetupString,"DETAILSTODISPLAY",buffer,&bufferSize))
    /* This works only on Warp 4 and above */
    wpSetup("DETAILSTODISPLAY=0,1,4,12,13,14,15,16,17,18,19,20,21");/* Fill in defaults */

  return rcParent;
}

BOOL CWAudioFolder::wpSetup(PSZ pSetupString)
{
  /**************************************************
   *                                                *
   * Supported setupstrings:                        *
   *       FLDRWRITEFLAGS=1+2+4+8                   *
   *                                                *
   **************************************************/
  char buffer[CCHMAXPATH];
  ULONG bufferSize;
  ULONG ulWFlags;
  SHORT sIndex;
  HWND hwndLB;
  HAB hab;
  int tid;

  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, AUDIOFLDRSETUP_SHOWABOUTDIALOG, buffer,&bufferSize))
    { 
      tid=_beginthread(aboutThreadFunc,NULL,8192*8,this); //Fehlerbehandlung fehlt
    }
  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, AUDIOFLDRSETUP_FLDRWRITEFLAGS,buffer,&bufferSize))
    { 
      ulWFlags=atol(buffer);
      cwSetWriteFlags( ulWFlags,WF_ALLFLAGS);
    }
  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, SETUP_DISPLAYHELPPANEL, buffer, &bufferSize))
    { 
      ulWFlags=atol(buffer);
      wpDisplayHelp(ulWFlags,AFHELPLIBRARY);
    }
  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, AUDIOFLDRSETUP_FREESHAREDMEM, buffer, &bufferSize))
    { 
      ulWFlags=atol(buffer);
      if(ulWFlags==1)
        DosFreeMem(pvAudioSharedMem);
      pvAudioSharedMem=NULL;
    }
  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, AUDIOFLDRSETUP_SETSTATUSTEXT,buffer,&bufferSize))
    { 
      cwSetStatusText(buffer);
    }

  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, SETUP_WRITECONFIGTOFILE, buffer, &bufferSize))
    { 
      FILE *fHandle;
      /* Write the current settings to the given file */
      fHandle=fopen(buffer,"a");
      if(fHandle) {
        addSettingsToFile(fHandle);
        addAudioFolderSettingsToFile(this, fHandle);
        fclose(fHandle);
      }
    }

  bufferSize=sizeof(buffer);
  /* The tracknames in the track listbox are set with setup strings from a REXX skript
     after querying the FREEDB database */
  if(wpScanSetupString(pSetupString, AUDIOFLDRSETUP_SETTRACKLBTEXT, buffer, &bufferSize))
    { 
      sIndex=atoi(buffer); /* The index to rename */
      if(sIndex!=0) {
        do {
          if(!WinIsWindow(WinQueryAnchorBlock(HWND_DESKTOP),hwndGrab))
            break;/* Non fatal error */
          hab=WinQueryAnchorBlock(HWND_DESKTOP);
          hwndLB=WinWindowFromID(hwndGrab,IDLB_GRABTRACKS);
          
          if(!WinIsWindow(hab,hwndGrab))
            break;/* Non fatal error */
          
          /* Set item text */
          WinSendMsg( hwndLB, LM_SETITEMTEXT,MPFROMSHORT(sIndex-1),buffer);
          hwndLB=WinWindowFromID(hwndGrab, IDEF_GRABNAME);
          if(!WinIsWindow(hab,hwndLB))
            break;/* Non fatal error */
          WinQueryWindowText( hwndLB,sizeof(buffer),buffer);
          if(!strlen(buffer)) { /* The user didn't provide a track name */
            WinSetWindowText(hwndLB, DEFAULT_WAVENAME); /* Default name. The tracks will be renamed so the name isn't that important */       
          }
          WinEnableWindow(hwndLB,FALSE);
          break;
        }while(TRUE);
      }
    }

  return CWProgFolder::wpSetup(pSetupString);
}


/************************************************/
/* Override function: Remove the tree view page */
/************************************************/
ULONG CWAudioFolder::wpAddFolderView2Page(HWND hwndNotebook)
{
  return SETTINGS_PAGE_REMOVED;
}


BOOL CWAudioFolder::wpAddSettingsPages(HWND hwndNotebook)
{
  BOOL rc;

#if 0
  /* Call the parent so it can insert its settings pages */
  rc=((somTD_WPObject_wpAddSettingsPages)
      somParentNumResolve(__ClassObject->somGetPClsMtabs(),
                          1,
                          __ClassObject->
                          somGetMethodToken(somIdFromString
                                            ("wpAddSettingsPages")))
      )(this,hwndNotebook);
#endif

  rc=CWProgFolder::wpAddSettingsPages(hwndNotebook);

  if(!hAudioResource) {
    errorResource();
    return rc;
  }
    
  return rc;
  // Durch obige Konstruktion wird die Elternmethode des direkten VorgÑngers aufgerufen,
  // und nicht die ursprÅngliche von WPLaunchPad, falls WPLaunchPad replaced wurde.
  // Durch diesen Aufruf gehen eventuelle Erweiterungen durch Vererbung verloren:
  //            < return WPLaunchPad::wpMenuItemSelected(hwndFrame, ulMenuId); >
  // Dieser Aufruf macht ErgÑnzungen in wpobject.hh erforderlich!!! Wird durch ein VAC-Fixpack vielleicht spÑter
  //erledigt    
}

/************************************************/
/* Override function: Remove the tree view open */
/*                    item from the menu.       */
/************************************************/
ULONG CWAudioFolder::wpFilterPopupMenu(ULONG ulFlags, HWND hwndCnr, BOOL fMultiSelect)
{
  return CWProgFolder::wpFilterPopupMenu( ulFlags, hwndCnr, fMultiSelect) & ~CTXT_TREE;
}

/************************************************/
/* Override function:                           */
/************************************************/
BOOL CWAudioFolder::wpModifyPopupMenu(HWND hwndMenu, HWND hwndCnr, ULONG ulPosition)
{
  /* Remove the change to tree view item from menu */ 
  /* This does not remove it from the menubar :( */
  WinSendMsg(hwndMenu,MM_REMOVEITEM,MPFROM2SHORT(717,TRUE),0);

  if(hAudioResource) {
    wpInsertPopupMenuItems(hwndMenu,1,hAudioResource,ID_MENUTOOLS,0);
    wpInsertPopupMenuItems(hwndMenu,-1,hAudioResource,ID_MENUABOUT,WPMENUID_HELP);
  }

  return CWProgFolder::wpModifyPopupMenu(hwndMenu, hwndCnr,  ulPosition); 
}

BOOL CWAudioFolder::wpMenuItemSelected(HWND hwndFrame,ULONG ulMenuId)
{
  int tid;

  switch(ulMenuId)
    {
    case 717://Change to tree view
      DosBeep(100,100);
      return TRUE;
    case ID_TOOLSITEM:
      tid=_beginthread(toolsThreadFunc,NULL,8192*8,this); //Fehlerbehandlung fehlt
      return TRUE;
    case ID_ABOUTITEM:
      cwShowAboutDlg(hAudioResource,IDDLG_ABOUT);
      return TRUE;
    case ID_CREATECOVERITEM:
      tid=_beginthread(coverThreadFunc,NULL,8192*8,this); //Fehlerbehandlung fehlt
      return TRUE;
    case ID_MP3DECODEITEM:
      {
        char fileName[CCHMAXPATH];
        /* Hack: Not very elegant way to get the frame. Will change in the future */
        if(!hwndFrame && hwndSelect)
          hwndFrame=WinQueryWindow(hwndSelect, QW_PARENT);
        cwCreateContentsFileForDec(fileName,  hwndFrame);
        launchDRDialog(fileName, "mp3decod.res", "Decode MP3 files");
      }
      return TRUE;
    case ID_MP3ENCODEITEM:
      {
        char fileName[CCHMAXPATH];
        /* Hack: Not very elegant way to get the frame. Will change in the future */
        if(!hwndFrame && hwndSelect)
          hwndFrame=WinQueryWindow(hwndSelect, QW_PARENT);
        cwCreateContentsFileForEncoding(fileName,  hwndFrame);
        launchDRDialog(fileName, "mp3encod.res", "Encode MP3 files");
      }
      return TRUE;
    default:
      break;
    }

  return CWProgFolder::wpMenuItemSelected( hwndFrame, ulMenuId);
}


BOOL CWAudioFolder::wpMenuItemHelpSelected(ULONG MenuId)
{
  switch(MenuId) {
  case ID_TOOLSITEM:
    return wpDisplayHelp(ID_TOOLSITEM,AFHELPLIBRARY);       
  case ID_CREATECOVERITEM:
    return wpDisplayHelp(ID_CREATECOVERITEM,AFHELPLIBRARY);
  case ID_ABOUTITEM:
    return FALSE;
  case ID_MP3DECODEITEM:
    return wpDisplayHelp(ID_MP3DECODEITEM, AFHELPLIBRARY);
  case ID_MP3ENCODEITEM:
    return wpDisplayHelp(ID_MP3ENCODEITEM, AFHELPLIBRARY);
  default:
    break;
  }
  return CWProgFolder::wpMenuItemHelpSelected(MenuId);
}


/* This function is called when the object is created from a 
   template, too */
BOOL CWAudioFolder::wpRestoreState(ULONG ulReserved)
{
  
  /* Restore write flags */
  if(!wpRestoreLong("CWAudioFolder",IDKEY_FDRWRITEFLAGS, &ulWriteFlags))
    ulWriteFlags=IDWF_PAD|IDWF_NOFIX|IDWF_DUMMY;  
  /* Remove preemp flag as of V0.54. It's not supported anymore */
  ulWriteFlags&=~IDWF_PREEMP;
  return CWProgFolder::wpRestoreState(ulReserved);
}

BOOL CWAudioFolder::wpSaveState()
{
  char profileName[CCHMAXPATH];
  char moduleName[CCHMAXPATH];
  char *chrPtr;
  char chrDev[10];
  HINI hini=0;
  BOOL bError=TRUE;

  
#if 0
  /* Save Settings to ini */
  /* The ini file entries will be loaded in CWCreatorSettings::wpclsInitData() */
  do{  
    /* Open the ini-file */
    hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),profileName);
    if(!hini) {
      /* profileName: "Warning! Cannot open Ini-file!"
         moduleName: "Audio-CD-Creator"
         */
      messageBox( profileName, IDSTR_INIFILEOPENERROR , sizeof(profileName),
                  moduleName, IDSTR_AUDIOCDCREATOR, sizeof(moduleName),
                  hAudioResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
      break;
    }/* end of if(!hini) */

    /* Presparameters */
    /* Font for track listbox in grab frame control */
    if(!PrfWriteProfileString(hini,"font","grablistbox", chrLBFontName))
      break;

    PrfCloseProfile(hini);
    bError=FALSE;
    break;
  }while(TRUE);
  if(bError) {
    /* profileName: "Warning! Cannot write to Ini-file!"
       moduleName: "Audio-CD-Creator"
       */
    messageBox( profileName, IDSTR_WRITEINIERROR, sizeof(profileName),
                moduleName, IDSTR_AUDIOCDCREATOR, sizeof(moduleName),
                hAudioResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
    if(hini)PrfCloseProfile(hini);
  }
#endif

  /* This saves the instance write flags */
  if(!wpSaveLong("CWAudioFolder", IDKEY_FDRWRITEFLAGS,ulWriteFlags))
    /* profileName: "Warning! Cannot write instance data!"
       moduleName: "Audio-CD-Creator"
       */
    messageBox( profileName, IDSTR_WRITEINSTANCEDATAWARNING , sizeof(profileName),
                moduleName, IDSTR_AUDIOCDCREATOR, sizeof(moduleName),
                hAudioResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);

    
  return CWProgFolder::wpSaveState();
}


MRESULT CWAudioFolder::wpDragOver(HWND hwndCnr,PDRAGINFO pDragInfo)
{
  WPObject *wpObject;

  /* The following is necessary because for some reason hwndCnr and pDragInfo->hwndSource
     are equal when the dragged object resides in the same folder as the target and the target
     isn't opened yet.*/
  wpObject=(WPObject*)OBJECT_FROM_PREC(DrgQueryDragitemPtr( pDragInfo, 0)->ulItemID);
  if(!somIsObj(wpObject))
    return MRFROM2SHORT( DOR_NEVERDROP, DO_MOVE);
  if(this->wpQueryFolder()==wpObject->wpQueryFolder())
    if(lCDROptions&IDCDR_CREATESHADOWS)
      return MRFROM2SHORT( DOR_DROP, DO_LINK);

  /* Do not create shadows if the source is the current container */
  if(hwndCnr==pDragInfo->hwndSource)
    return MRFROM2SHORT( DOR_DROP, DO_MOVE);
  /* Option is create shadows */
  if(lCDROptions&IDCDR_CREATESHADOWS)
    return MRFROM2SHORT( DOR_DROP, DO_LINK);
  /* System default is moving */
  return MRFROM2SHORT( DOR_DROP, DO_MOVE);

}



void _Optlink dropThreadFunc (void *arg)
{
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  PDROPTHREADPARAMS threadParams;
  CWAudioFolder *thisPtr;
  CWAudio* cwAudio;
  HWND hwndCnr;

  threadParams=(PDROPTHREADPARAMS) arg;
  if(!threadParams)
    return;
  thisPtr=(CWAudioFolder*)threadParams->thisPtr;    //Pointer auf CWAudioFolder-Object
  
  if(!somIsObj(thisPtr))
    return;
  
#if 0
  if(thisPtr->mfData)
    thisPtr->mfData->bProcessing=TRUE;
#endif

  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {
      WPObject *wpObject;
      somId mySomId;
      M_WPObject *mAudioObject;
      SOMClass*  scCWShadowClass;
      ULONG ulCount;
      ULONG ulNumObjects;
      /* Needed for specifying the drop point */
      POINTL ptl;
      PMINIRECORDCORE pmrc; 
      CNRINFO cnrInfo;
      int ID;
      HWND hwndSource;

      hwndCnr=threadParams->hwndCnr;

      /* The objects are already checked by the wpDragOver() method. */

      //      TRY_LOUD(MEDIAFLDR_DROP) {

        hwndSource=threadParams->hwndSource;
        ulNumObjects=threadParams->ulNumObjects;

        if((mySomId=somIdFromString(SHADOW_CLASS_NAME))!=NULLHANDLE) {
          WPFileSystem *wpFSObject;
          /* Get the CWAudioShadow class object */
          scCWShadowClass=(SOMClass*)SOMClassMgrObject->somClassFromId(mySomId);
          SOMFree(mySomId);
          /* Check shadow class */
          if(somIsObj(scCWShadowClass)) {
            if(hwndSource!=hwndCnr){/* Dropped on white space of an open folder */
              QUERYRECFROMRECT qRecRcl;
              ptl.x=threadParams->ptl.x;
              ptl.y=threadParams->ptl.y;
              
              /* Map to CNR */
              WinMapWindowPoints(HWND_DESKTOP, hwndCnr,&ptl, 1);
              ptl.y+=10;          
              /* Window below the mouse ptr. We need it to check if we are over the column title area. */
              ID=WinQueryWindowUShort(WinWindowFromPoint(hwndCnr,&ptl, TRUE),QWS_ID);
              ptl.y-=10;
             
              WinSendMsg(hwndCnr, CM_QUERYCNRINFO, MPFROMP(&cnrInfo), MPFROMSHORT(sizeof(cnrInfo)));        
 
              /* Find the record near the drop point */
              qRecRcl.cb=sizeof(qRecRcl);
              if(cnrInfo.flWindowAttr& CV_DETAIL) {
                qRecRcl.rect.yBottom=ptl.y-10;
                qRecRcl.rect.yTop=ptl.y;
              }
              else {
                qRecRcl.rect.yBottom=ptl.y;
                qRecRcl.rect.yTop=ptl.y+18;
              }
              qRecRcl.rect.xLeft=ptl.x;
              qRecRcl.rect.xRight=ptl.x+10;
              qRecRcl.fsSearch=CMA_PARTIAL|CMA_ITEMORDER;
              pmrc=(PMINIRECORDCORE)WinSendMsg(hwndCnr, CM_QUERYRECORDFROMRECT,MPFROMLONG(CMA_FIRST), MPFROMP(&qRecRcl));

            }/*if(hwndSource!=hwndCnr)*/

            /* For all dropped objects */ 
            for(ulCount=0; ulCount<ulNumObjects; ulCount++) {
              if(ulCount%10)/* Redraw window every 10 objects */
                WinEnableWindowUpdate(hwndCnr, FALSE);
              else
                WinEnableWindowUpdate(hwndCnr, TRUE);

              wpObject=(WPObject*) threadParams->wpObject[ulCount];
              /* Get the filesystem object in case it's a shadow */
              wpFSObject=(WPFileSystem*)thisPtr->cwGetFileSystemObject(wpObject);
              if(somIsObj(wpFSObject)){
                if(hwndSource==hwndCnr)/* The folder is closed or dropped on the folder icon. */
                  wpFSObject->wpCreateShadowObjectExt(thisPtr, FALSE, "", (M_WPObject*)scCWShadowClass);
                else {
                  /* Dropped on white space of the folder */
                  WPObject *wpo;
                  if((cnrInfo.flWindowAttr& CV_DETAIL) || (cnrInfo.flWindowAttr&CV_NAME ))
                    {
                      /* Detailsview */
                      RECORDINSERT recInsert;
                      
                      recInsert.cb=sizeof(recInsert);
                      recInsert.pRecordParent=NULL;
                      recInsert.fInvalidateRecord=TRUE;
                      recInsert.zOrder=CMA_TOP;
                      recInsert.cRecordsInsert=1;

                      /* Create the shadow */
                      wpo=wpFSObject->wpCreateShadowObjectExt(thisPtr, FALSE, "", (M_WPObject*) scCWShadowClass);
                      /* Move the record to the right position in the details view */
                      if(somIsObj(wpo)) {
                        wpo->wpCnrRemoveObject(hwndCnr);
                        if(pmrc && ((int)pmrc)!=-1) {
                          recInsert.pRecordOrder=(PRECORDCORE)pmrc;
                          /* Insert at the right place. For some reason wpSetNextIconPos() does not work!? */
                          wpo->wpCnrInsertObject(hwndCnr, &ptl, NULL, &recInsert);
                        }
                        else {
                          /* No record found check if we are at the top or the bottom */

                          if(ID!=0x7ff0 && ID!=0x7ff1) {
                            if((cnrInfo.flWindowAttr & CV_NAME) && ID!=FID_CLIENT)
                              recInsert.pRecordOrder=(PRECORDCORE)CMA_FIRST;
                            else
                              recInsert.pRecordOrder=(PRECORDCORE)CMA_END;
                          }
                          else
                            recInsert.pRecordOrder=(PRECORDCORE)CMA_FIRST;

                          /* Insert at the right place. For some reason wpSetNextIconPos() does not work!? */
                          wpo->wpCnrInsertObject(hwndCnr, &ptl, NULL, &recInsert);
                        }
                      }/* if(somIsObj(wpo)) */
                    }/* if(cnrInfo.flWindowAttr& CV_DETAIL) */
                  else
                    /* Icon view */
                    wpo=wpFSObject->wpCreateShadowObjectExt(thisPtr, FALSE, "", (M_WPObject*) scCWShadowClass);
                  if(somIsObj(wpo))
                    wpo->wpCnrRefreshDetails();
                }/* else(hwndSource==hwndCnr) */
              }/* if(somIsObj(wpFSObject)) */
            }/* for(ulCount=0;ulCount<ulNumObjects ; ulCount++) */
            WinEnableWindowUpdate(hwndCnr, TRUE);
          }/* if(somIsObj(scCWShadowClass)) */
        }/* if((mySomId=somIdFromString(SHADOW_CLASS_NAME))!=NULLHANDLE)*/
#if 0
        // }/* Try */
        //   CATCH(MEDIAFLDR_DROP)
        //     {
          
        //   } END_CATCH;
        
#endif    
        WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
#if 0
  if(thisPtr->mfData)
    thisPtr->mfData->bProcessing=FALSE;
#endif

  thisPtr->wpFreeMem((PBYTE)threadParams);
}


MRESULT CWAudioFolder::wpDrop(HWND hwndCnr,PDRAGINFO pDragInfo,PDRAGITEM pDragItem)
{
  MRESULT mr;

  if(!cwGetSomClass(SHADOW_CLASS_NAME)||pDragInfo->usOperation==DO_MOVE||pDragInfo->usOperation==DO_COPY)
    mr=CWProgFolder::wpDrop(hwndCnr, pDragInfo, pDragItem);
  else {
    TRY_LOUD(MEDIAFLDR_DROP) {
      PDROPTHREADPARAMS pThreadParams;
      ULONG ulErr;
      WPObject* wpObj;
      ULONG ulNumObjects=DrgQueryDragitemCount(pDragInfo);
      ULONG ulCount;

      pThreadParams=(PDROPTHREADPARAMS)wpAllocMem(sizeof(DROPTHREADPARAMS)+(ulNumObjects-1)*sizeof(WPObject*), &ulErr);
      pThreadParams->hwndCnr=hwndCnr;
      pThreadParams->hwndSource=pDragInfo->hwndSource;
      pThreadParams->ulNumObjects=DrgQueryDragitemCount(pDragInfo);
      pThreadParams->thisPtr=this;
      pThreadParams->ptl.x=pDragInfo->xDrop;
      pThreadParams->ptl.y=pDragInfo->yDrop;
      
      /* For all dropped objects */ 
      for(ulCount=0;ulCount<ulNumObjects ; ulCount++) {
        pThreadParams->wpObject[ulCount]=(WPObject*)OBJECT_FROM_PREC(DrgQueryDragitemPtr( pDragInfo, ulCount)->ulItemID);
      }      
      _beginthread(dropThreadFunc,NULL,8192*16,(void*)pThreadParams); //Fehlerbehandlung fehlt
      
    }
    CATCH(MEDIAFLDR_DROP)
      {
        
      } END_CATCH;

      DrgFreeDraginfo(pDragInfo);
      mr=(MRESULT)RC_DROP_DROPCOMPLETE;

   
  }/* else */

  cwForceStatusUpdate();/* User dropped something. Recalculate the time */
  return mr;
}


BOOL  CWAudioFolder::wpDeleteFromContent(WPObject* Object)
{
  BOOL rc;    
    
  rc=CWProgFolder::wpDeleteFromContent(Object);
  cwForceStatusUpdate();
  return rc;
}

BOOL CWAudioFolder::wpAddToContent(WPObject* Object)
{
  BOOL rc;
 
  rc=CWProgFolder::wpAddToContent(Object);
  //cwForceStatusUpdate()   ;
  return rc;
}

BOOL CWAudioFolder::wpPopulate(ULONG ulReserved,PSZ Folder,BOOL fFoldersOnly)
{
  BOOL rc;

  rc=CWProgFolder::wpPopulate(ulReserved,Folder,fFoldersOnly);

  /* Set total time into status line */
  cwForceStatusUpdate();
  return rc;  
}


HWND CWAudioFolder::wpOpen(HWND hwndCnr,ULONG ulView,ULONG ulParam)
{
  HWND hwnd;
  SIZEL sizelLeft={0};
  HWND myFrameHWND;
  RECTL rectl;
  MENUITEM mi;
  HWND hwndMenu;
  int iItemCount;
  char text[CCHMAXPATH];


  /* Call parent to build folder window */
  hwnd=CWProgFolder::wpOpen(hwndCnr, ulView, ulParam);    

  /* Only subclass folder frames, not settings notebooks */
  if(ulView!=OPEN_SETTINGS){
    
    if(!hAudioResource) {
      errorResource();
      return hwnd;
    }

    //    DosBeep(5000,1000);
    TRY_LOUD(AUDIOFOLDER_OPEN) {

      if(!(ulWriteFlags & FLAG_ALREADYSIZED)) {
        /* Make sure on first open the container is visible */
        SWP swp;
        WinQueryWindowPos(hwnd, &swp);
        WinSetWindowPos(hwnd, NULLHANDLE, swp.x, swp.y-400, swp.cx, swp.cy+400, SWP_SIZE|SWP_MOVE);
        ulWriteFlags|=FLAG_ALREADYSIZED;
        wpSaveDeferred();
        //  DosBeep(500,1000);
      }

      myFrameHWND=WinWindowFromID(hwnd,FID_MENU);//Get menu hwnd
      
      /* Remove the change to tree view item from menubar */ 
      //WinSendMsg(myFrameHWND,MM_DELETEITEM,MPFROM2SHORT(717,TRUE),0);
      
      if(WinIsWindow(WinQueryAnchorBlock(hwnd), myFrameHWND)) {
        /* Query the number of items in the menubar */
        iItemCount=(int)WinSendMsg(myFrameHWND,MM_QUERYITEMCOUNT,(MPARAM)0,
                                   (MPARAM)0);
        /* Load the Misc-tools menu from the resource */
        hwndMenu=WinLoadMenu(myFrameHWND,hAudioResource,ID_AUDIOTBMENUTOOLS);
        //723
        if(hwndMenu){
          M_WPObject *m_wpObject;
          CWMenuFolder * wpFolder;
          HOBJECT hObject;
          somId id;
          somTD_CWMenuFolder_mfInsertMenuItems methodPtr;
          
          /* Insert the menu items from the user menu folder. WPS-Wizard must be installed for
             this. */
          if((hObject=WinQueryObject(USERMENUFOLDER_AUDIO))!=NULLHANDLE) {//is there a default menufolder?    
            m_wpObject=(M_WPObject*)this->somGetClass();
            if(somIsObj(m_wpObject)) {
              /* We have it */
              /* Get method address */
              id=somIdFromString("mfInsertMenuItems");
              wpFolder=(CWMenuFolder *)m_wpObject->wpclsQueryObject( hObject);
              if(somIsObj(wpFolder)) {
                // DosBeep(5000,300);
                methodPtr= (somTD_CWMenuFolder_mfInsertMenuItems) (wpFolder->somGetClass())->somFindSMethod(id);  
                if(methodPtr) {
                  //  DosBeep(100,300);
                  cwInsertMenuSeparator(MIT_END, hwndMenu, NULL);
                  methodPtr(wpFolder, hwndMenu , MIT_END, FIRSTUSERITEM);
                }
                //ulCurrentID=methodPtr(wpFolder, hwndMenu,iPosition, MYMENUID);
                /* ulCurrentID contains the last used menu id */
                wpFolder->wpUnlockObject();//Object is locked because of wpclsQueryHandle()
              }
            }/* end of if(somIsObj(m_wpObject))  */
          }
          
          /* Fill the MENUITEM structure */
          mi.iPosition=iItemCount-1;
          mi.afStyle=MIS_TEXT|MIS_SUBMENU;
          mi.id=ID_TBMENUTOOLS;
          mi.afAttribute=NULL;                
          mi.hwndSubMenu=hwndMenu;
          mi.hItem=NULL;
          /* Add the Misc-tools item to the folder menubar */
          /* Text: "Misc-Tools" */
          getMessage(text, IDSTR_MISCTOOLS,sizeof(text), hAudioResource, hwnd);
          WinSendMsg(myFrameHWND,MM_INSERTITEM,(MPARAM)&mi,
                     (MPARAM)text); 
        }/* end of if(hwndMenu) */
        /* Subclass the menubar so we can insert items to standard dropdown menues */
      }
      
      myFrameHWND=WinWindowFromID(hwnd,FID_CLIENT);//Get container hwnd
      if(myFrameHWND){
        pfnwpContainer=WinSubclassWindow(myFrameHWND,audioContainerProc);
      }
      /* Now add the framecontrols */
      
      /* This is the select-frameCtrl. */
      /* Load the dialog from the resource. The object pointer of this
         folder is given to the dialog procedure (this). In WM_INITDLG
         the procedure saves this pointer for later use. */
      myFrameHWND=WinLoadDlg(hwnd,hwnd,selectDialogProc,hAudioResource,IDDLG_SELECTDLG,this);
      /* Save the HWND in the instance data */    
      hwndSelect=myFrameHWND;
      
      if(tid)     
        /* The folder currently grabs some tracks from CD so disable the controls. */
        cwEnableSelectControls(FALSE);
      
      if(myFrameHWND){
        /* Query the size of the dialog */
        WinQueryWindowRect(myFrameHWND,&rectl);
        /* It's the top dialog so we need the height but not the width. The width
           is always the width of the folder. */
        sizelLeft.cy=rectl.yTop-rectl.yBottom;
        /* Call a method of CWProgFolder to add the dialog as a framecontrol. 
           FCTL_TOP means put it at the top of the folder.
           After calling cwUpdateFrame(hwnd) it will be visible */
        cwAddFrameCtl(hwnd, myFrameHWND, sizelLeft, FCTL_TOP,0);
      }
      
      if(tid) {
        /* The folder currently grabs some tracks from CD so show grab framecontrol. */
        /* This is the grab-framectrl. */
        myFrameHWND=WinLoadDlg(hwnd,hwnd,grabDialogProc,hAudioResource,IDDLG_GRABDIALOG,this);
        hwndGrab=myFrameHWND;
        cwEnableGrabControls(FALSE);
      }
      else {
        /* This is the write-framectrl. */
        myFrameHWND=WinLoadDlg(hwnd,hwnd,dialogProc,hAudioResource,ID_DIALOG,this);
        hwndWrite=myFrameHWND;
        if(hwndWrite&&tidWrite)
          cwEnableWriteControls(FALSE);
      }
      if(myFrameHWND){
        WinQueryWindowRect(myFrameHWND,&rectl);
        sizelLeft.cx=rectl.xRight-rectl.xLeft;
        /* Put the dialog at the left side of the folder */
        cwAddFrameCtl(hwnd, myFrameHWND, sizelLeft, FCTL_LEFT,FCTL_POSBELOW|FCTL_POSABOVE);
      }
      /* Statusbar */
      hwndStatusFrameCtl=WinLoadDlg(hwnd,hwnd,statusLineDialogProc,hAudioResource,IDDLG_STATUS,this);
      if(hwndStatusFrameCtl) {
        WinQueryWindowRect(hwndStatusFrameCtl,&rectl);
        sizelLeft.cy=rectl.yTop-rectl.yBottom;
        /* Put the dialog at the bottom of the folder */
        cwAddFrameCtl(hwnd, hwndStatusFrameCtl, sizelLeft, FCTL_BOTTOM,0);
      }
      /* Refresh frame to show the new framecontrols */
      cwUpdateFrame(hwnd);
      /* Let the statusbar show the right time */
      cwForceStatusUpdate();
      cwEnableWriteControls(FALSE);
      pfnwpFrame=WinSubclassWindow(hwnd, audioFrameProc);
      pfnwpGenericFrame=pfnwpFrame;
    }  
    CATCH(AUDIOFOLDER_OPEN)
      {
      } END_CATCH;
  }

  if(!(globalData.ulGlobalFlags &  GLOBALFLAG_DAYTIPSHOWN) && globalData.bTipsEnabled) {
    sprintf(text, "%s\\bin\\pmdaytip.exe",chrInstallDir);
    HlpSendCommandToObject( text, "OPEN=DEFAULT");
    globalData.ulGlobalFlags|=GLOBALFLAG_DAYTIPSHOWN;
  }

  return hwnd;
}

BOOL CWAudioFolder::cwClose(HWND hwndFrame)
{
  wpSaveDeferred();
  
  return CWProgFolder::cwClose(hwndFrame);
}









