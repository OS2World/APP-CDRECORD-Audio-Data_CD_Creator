/*
 * This file is (C) Chris Wohlgemuth 1999-2004
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

BOOL MkisofsSetupDone=FALSE;

BOOL bHaveIsoFS=FALSE;

char chrMkisofs[CCHMAXPATH];/* Path to mkisofs */
char chrMkisofsOptions[CCHMAXPATH];
LONG lMKOptions;

extern char chrImage[CCHMAXPATH];/* Path to last image file */

extern LONG  lCDROptions;
extern char chrInstallDir[CCHMAXPATH];

PFNWP pfnwpDataGenericFrame;

/* This is the handle for the ressource DLL holding the NLS messages and dialogs */
extern HMODULE hDataResource;

extern PVOID pvSharedMem;

void _Optlink toolsThreadFunc (void *arg);
void errorResource();
ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);
void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
void HlpSendCommandToObject(char* chrObject, char* command);
MRESULT EXPENTRY imageNameDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY mkisofsMainDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY statusLineDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY dataCDStatusLineDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY dataContainerProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY dataFrameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT cwInsertMenuSeparator(int iPosition, HWND hwndMenu, HWND hwndSubMenu);
ULONG launchPMWrapper(PSZ parameter, PSZ folderPath, PSZ wrapperExe, PSZ pszTitle="CDRecord/2");
void addSettingsToFile(FILE *fHandle);
BOOL somObjectIsA(WPObject* wpObject, char * chrClassName);

void _Optlink cdSizeThreadFunc(void *arg);

typedef ULONG   SOMLINK somTP_CWMenuFolder_mfInsertMenuItems(CWMenuFolder *somSelf,
		HWND hwndMenu,
		ULONG iPosition,
		ULONG ulLastMenuId);
//#pragma linkage(somTP_CWMenuFolder_mfInsertMenuItems, system)
typedef somTP_CWMenuFolder_mfInsertMenuItems *somTD_CWMenuFolder_mfInsertMenuItems;


void addDataFolderSettingsToFile(CWDataFolder * thisPtr, FILE *fHandle)
{
  ULONG  ulFlagsLocal;

  ulFlagsLocal=thisPtr->cwQueryCDTypeFlags();

  fprintf(fHandle,"[CDTYPE]\n");
  if(ulFlagsLocal & IDCDT_MULTISESSION)
    fprintf(fHandle,"[MULTISESSION]YES\n");
#if 0
  else
    fprintf(fHandle,"[MULTISESSION]NO\n");
#endif

  if(ulFlagsLocal & IDCDT_BOOTCD)
    fprintf(fHandle,"[BOOTCD]YES\n");
#if 0
  else
    fprintf(fHandle,"[BOOTCD]NO\n");
#endif
  if(ulFlagsLocal & IDCDT_USERDEFINED)
    fprintf(fHandle,"[USERDEFINED]YES\n");
#if 0
  else
    fprintf(fHandle,"[USERDEFINED]NO\n");
#endif

  ulFlagsLocal=thisPtr->cwQueryCreateFlags();
  fprintf(fHandle,"[ACTION]\n");
  if(ulFlagsLocal & IDCR_IMAGEONLY)
    fprintf(fHandle,"[CREATEIMAGE]YES\n");

  if(ulFlagsLocal & IDCR_WRITEIMAGE)
    fprintf(fHandle,"[WRITEIMAGE]YES\n");

  if(ulFlagsLocal & IDCR_ONTHEFLY)
    fprintf(fHandle,"[ONTHEFLY]YES\n");

  if(ulFlagsLocal & IDCR_TESTONLY)
    fprintf(fHandle,"[TEST]YES\n");
  else
    fprintf(fHandle,"[TEST]NO\n");

  ulFlagsLocal=thisPtr->cwQueryMkisofsFlags();
  fprintf(fHandle,"[MKISOFS]\n");

  if(ulFlagsLocal & IDMK_JOLIET)
    fprintf(fHandle,"[JOLIET]YES\n");
  else
    fprintf(fHandle,"[JOLIET]NO\n");

  if(ulFlagsLocal & IDMK_TRANSTABLE)
    fprintf(fHandle,"[TRANSTABLE]YES\n");
  else
    fprintf(fHandle,"[TRANSTABLE]NO\n");

  if(ulFlagsLocal & IDMK_ALLOW32CHARS)
    fprintf(fHandle,"[32CHARISONAMES]YES\n");
  else
    fprintf(fHandle,"[32CHARISONAMES]NO\n");

  if(ulFlagsLocal & IDMK_SHADOWSINROOTONLY)
    fprintf(fHandle,"[FOLLOWSHADOWS]NO\n");
  else
    fprintf(fHandle,"[FOLLOWSHADOWS]YES\n");
}

BOOL CWDataFolder::wpSetupOnce(PSZ pSetupString)
{
  /**************************************************
   *                                                *
   * Supported setupstrings:                        *
   *                                                *
   **************************************************/
  BOOL rcParent=FALSE;
  char buffer[CCHMAXPATH];
  ULONG bufferSize;
  M_CWDataFolder* m_CWData=(M_CWDataFolder*)somGetClass();

  rcParent=CWProgFolder::wpSetupOnce(pSetupString);

  /* Workaround to prevent the WPS from unloading our class. */
  m_CWData->wpclsIncUsage();

  /* Doing our own setup here */
  bufferSize=sizeof(buffer);
  if(!wpScanSetupString(pSetupString,"ICONVIEWPOS",buffer,&bufferSize))
    wpSetup("ICONVIEWPOS=20,15,60,75");/* Insert default */
    
  return rcParent;
}


BOOL CWDataFolder::wpSetup(PSZ pSetupString)
{
  /**************************************************
   *                                                *
   * Supported setupstrings:                        *
   *       FLDRFILENAMEOPTIONS=                     *
   * mkisofs flags:                                 *
   *  IDMK_ALLOW32CHARS     0x0001                  *
   *  IDMK_LEADINGPERIODS   0x0002                  *
   *  IDMK_JOLIET  0x0004                           *
   *  IDMK_ROCKRIDGE  0x0008                        *
   *  IDMK_TRANSTABLE  0x0010                       *
   *  IDMK_ALLFILES  0x0040                         *
   *                                                *
   *  CD type flags:                                *
   *       FLDRCDTYPEOPTIONS=                       *
   *  IDCDT_TRACKDATA        0x0008                 *
   *  see audiofolder.h for more                    *
   *                                                *
   *  Create flags:                                 *
   *       FLDRCREATEOPTIONS=                       *
   *  IDCR_IMAGEONLY 0x0001                         *
   *  IDCR_WRITEIMAGE 0x0002                        *
   *  IDCR_ONTHEFLY    0x0004                       *
   *  IDCR_TESTONLY    0x0008                       *
   *                                                *
   **************************************************/
  char buffer[CCHMAXPATH];
  ULONG bufferSize;
  ULONG ulWFlags;


  /* Filename options */
  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, DATAFLDRSETUP_FILENAMEOPTIONS, buffer,&bufferSize))
    { 
      ulWFlags=atol(buffer);
      ulMkisofsFlags=(ulWFlags & MK_ALLFLAGS);
    }
  /* CD type */
  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, DATAFLDRSETUP_CDTYPEOPTIONS,buffer,&bufferSize))
    { 
      ulWFlags=atol(buffer);
      ulCDTypeFlags=(ulWFlags & CDT_ALLFLAGS);
    }
  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, DATAFLDRSETUP_FLDRCREATEOPTIONS, buffer, &bufferSize))
    { 
      ulWFlags=atol(buffer);
      ulCreateFlags=(ulWFlags&(IDCR_IMAGEONLY| IDCR_WRITEIMAGE |IDCR_ONTHEFLY));
    }

  bufferSize=sizeof(chrVolumeName); 
  if(wpScanSetupString(pSetupString, DATAFLDRSETUP_VOLUMENAME,chrVolumeName,&bufferSize))
    { 
      
    }
  bufferSize=sizeof(chrApplication); 
  if(wpScanSetupString(pSetupString, DATAFLDRSETUP_APPLICATION, chrApplication,&bufferSize))
    { 
      
    }
  bufferSize=sizeof(chrPublisher); 
  if(wpScanSetupString(pSetupString, DATAFLDRSETUP_PUBLISHER, chrPublisher,&bufferSize))
    { 
      
    }
  bufferSize=sizeof(chrPreparer); 
  if(wpScanSetupString(pSetupString, DATAFLDRSETUP_PREPARER, chrPreparer,&bufferSize))
    { 
      
    }
  bufferSize=sizeof(chrImageName); 
  if(wpScanSetupString(pSetupString, DATAFLDRSETUP_IMAGENAME, chrImageName,&bufferSize))
    { 
      
    }
 
  /* An easy way for any PM helper or dialog to provide online help */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString, SETUP_DISPLAYHELPPANEL, buffer, &bufferSize))
    { 
      ulWFlags=atol(buffer);
      wpDisplayHelp(ulWFlags,AFHELPLIBRARY);
    }
  /* Free the sharedmem. This is initiated by the PM helper so the mem is deallocated even
     if the helper crashes during processing */
  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, DATAFLDRSETUP_FREESHAREDMEM,buffer,&bufferSize))
    { 
      ulWFlags=atol(buffer);
      if(ulWFlags==1) {
        DosFreeMem(pvSharedMem);
        pvSharedMem=NULL;
      }
    }

  /* On the fly writing ended */
  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, DATAFLDRSETUP_ONTHEFLYDONE,buffer,&bufferSize))
    { 
      cwSetStatusText("");
      ulWFlags=atol(buffer);
      if(ulWFlags==1) {
        /* 1: CD successfully created */
       if(sDummy != 1 && (ulMkisofsFlags & IDMK_USEARCHIVEBIT) && (ulMkisofsFlags & IDMK_RESETARCHIVEBIT))
         cwResetArchiveBit();/* Only reset archive bits if really written */
      }
    }

  /* Set a new status text */
  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, DATAFLDRSETUP_SETSTATUSTEXT, buffer,&bufferSize))
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
        addDataFolderSettingsToFile(this, fHandle);
        //addAudioFolderSettingsToFile(this, fHandle);
        fclose(fHandle);
      }
    }

  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, DATAFLDRSETUP_FOLLOWALLSHADOWS, buffer,&bufferSize))
    { 
      if(buffer[0]=='1')
        ulMkisofsFlags&=~IDMK_SHADOWSINROOTONLY;
      else if(buffer[0]=='0')
        ulMkisofsFlags|=IDMK_SHADOWSINROOTONLY;
    }
  
  return CWProgFolder::wpSetup(pSetupString);
}

BOOL CWDataFolder::wpMenuItemHelpSelected(ULONG MenuId)
{
  switch(MenuId) {
  case ID_TOOLSITEM:
    return wpDisplayHelp(ID_TOOLSITEM,AFHELPLIBRARY);       
  case ID_CREATECOVERITEM:
    return wpDisplayHelp(ID_CREATECOVERITEM,AFHELPLIBRARY);
  case ID_MOUNTISOFILE:
    return wpDisplayHelp(ID_MOUNTISOFILE, AFHELPLIBRARY);
  case ID_UNMOUNTISOFILE:
    return wpDisplayHelp(ID_UNMOUNTISOFILE, AFHELPLIBRARY);
  case ID_ISOFSITEM:
    return wpDisplayHelp(ID_ISOFSITEM, AFHELPLIBRARY);
  case ID_ABOUTITEM:
  case ID_CDSIZEITEM:
    return FALSE;
  default:
    break;
  }
  return CWProgFolder::wpMenuItemHelpSelected(MenuId);
}

BOOL CWDataFolder::wpMenuItemSelected(HWND hwndFrame,ULONG ulMenuId)
{
  int tid;
  char text[CCHMAXPATH+4];
  char title[CCHMAXPATH];
  ULONG ulSize;

  switch(ulMenuId)
    {
    case ID_CDSIZEITEM:
      _beginthread(cdSizeThreadFunc,NULL,8192*16,this); //Fehlerbehandlung fehlt
      /* Text: "Querying free CD space. Please wait..." */
      getMessage(text,IDSTRD_QUERYFREECDSPACE, sizeof(text), hDataResource,hwndFrame);
      cwSetStatusText(text);    
      return TRUE;
    case ID_TOOLSITEM:
      /* Start a thread for the tools dialog */
      tid=_beginthread(toolsThreadFunc,NULL,8192*8,this); //Fehlerbehandlung fehlt
      return TRUE;
    case ID_ABOUTITEM:
      cwShowAboutDlg(hDataResource,IDDLG_ABOUT);
      return TRUE;
    case ID_UNMOUNTISOFILE:
      ulSize=sizeof(title);
      wpQueryRealName(title, &ulSize,TRUE);
      launchPMWrapper("1", title, "pmunmnt.exe", "");
      return TRUE;
    case ID_MOUNTISOFILE:
      sprintf(text,"\"%s\" 1",chrImageName);
      ulSize=sizeof(title);
      wpQueryRealName(title, &ulSize,TRUE);
      launchPMWrapper(text, title, "pmmntiso.exe", "");
      return TRUE;
    default:
      break;
    }
  return CWProgFolder::wpMenuItemSelected( hwndFrame, ulMenuId);
}


/************************************************/
/* Override function:                           */
/*                                              */
/*  wpModifyPopupMenu()                         */
/*                                              */
/* We insert the 'About' menu and the 'Tools'   */
/* menu into the popup menu of the data folder. */
/*                                              */
/* The menu items for the folder menu bar are   */
/* inserted in the wpOpen() method.             */
/*                                              */
/************************************************/
BOOL CWDataFolder::wpModifyPopupMenu(HWND hwndMenu, HWND hwndCnr, ULONG ulPosition)
{
  if(hDataResource) {
    /* The 'Tools' item */
    wpInsertPopupMenuItems(hwndMenu,1,hDataResource,ID_MENUTOOLS,0);
    /* The 'About' item is inserted into the Help submenu */
    wpInsertPopupMenuItems(hwndMenu,-1,hDataResource,ID_MENUABOUT,WPMENUID_HELP);
  }
  return CWProgFolder::wpModifyPopupMenu(hwndMenu, hwndCnr,  ulPosition); 
}

/************************************************/
/* Override function:                           */
/*                                              */
/*    wpAddSettingsPages(HWND hwndNotebook)     */
/*                                              */
/* The following pages are inserted in the      */
/* settings notebook of the data folder:        */
/*                                              */
/* -CD type                                     */
/* -Filename options                            */
/* -CD name and author                          */
/* -Special                                     */
/*                                              */
/************************************************/
BOOL CWDataFolder::wpAddSettingsPages(HWND hwndNotebook)
{
  BOOL rc;

  rc=CWProgFolder::wpAddSettingsPages(hwndNotebook);
  
  if(!hDataResource) {
    errorResource();
    return rc;
  }

  rc= rc && AddSpecialOptionPage( hwndNotebook) 
    && AddAuthorOptionPage(hwndNotebook)
    && AddFileNameOptionPage(hwndNotebook)
    && AddCDTypeOptionPage(hwndNotebook);

  return rc;
}


MRESULT CWDataFolder::wpDragOver(HWND hwndCnr,PDRAGINFO pDragInfo)
{
  WPObject *wpObject;

  wpObject=(WPObject*)OBJECT_FROM_PREC(DrgQueryDragitemPtr( pDragInfo, 0)->ulItemID);
  if(!somIsObj(wpObject))
    return MRFROM2SHORT( DOR_NEVERDROP, DO_MOVE);
  if(somObjectIsA(wpObject, "WPDisk"))
    return MRFROM2SHORT( DOR_NEVERDROP, DO_MOVE);

  /* The following is necessary because for some reason hwndCnr and pDragInfo->hwndSource
     are equal when the dragged object resides in the same folder as the target and the target
     isn't opened yet.*/
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


HWND CWDataFolder::wpOpen(HWND hwndCnr,ULONG ulView,ULONG ulParam)
{
  HWND hwnd;
  SIZEL sizelLeft={0};
  HWND myFrameHWND;
  RECTL rectl;
  MENUITEM mi;
  HWND hwndMenu;
  int iItemCount;
  char text[CCHMAXPATH];
  char text2[CCHMAXPATH];

  HOBJECT hObject;
  ULONG ul;
    
  hwnd=CWProgFolder::wpOpen(hwndCnr, ulView, ulParam);/* Call parent to build folder window */

#ifdef DEBUG
  HlpWriteToDebugLog("%s: HWND from CWProgFolder is: %x\n",__FUNCTION__, hwnd);
#endif

  if(ulView!=OPEN_SETTINGS){

    if(!hDataResource) {
#ifdef DEBUG
      HlpWriteToDebugLog("%s: No resource DLL!\n",__FUNCTION__);
#endif
      errorResource();
      return hwnd;
    }
    
    TRY_LOUD(DATAFOLDER_OPEN) {
      myFrameHWND=WinWindowFromID(hwnd,FID_MENU);//Get menu hwnd
      
      if(myFrameHWND){
        iItemCount=(int)WinSendMsg(myFrameHWND,MM_QUERYITEMCOUNT,(MPARAM)0,
                                   (MPARAM)0);
        hwndMenu=WinLoadMenu(myFrameHWND,hDataResource,ID_TBMENUTOOLS);
        //723
        if(hwndMenu){
          M_WPObject *m_wpObject;
          CWMenuFolder * wpFolder;
          HOBJECT hObject;
          somId id;
          somTD_CWMenuFolder_mfInsertMenuItems methodPtr;
          
          /* Insert the menu items from the user menu folder. WPS-Wizard must be installed for
             this. */              
          if((hObject=WinQueryObject(USERMENUFOLDER_DATA))!=NULLHANDLE) {//is there a default menufolder?    
            m_wpObject=(M_WPObject*)this->somGetClass();
            if(somIsObj(m_wpObject)) {
              /* We have it */
              /* Get method address */
              id=somIdFromString("mfInsertMenuItems");
              wpFolder=(CWMenuFolder *)m_wpObject->wpclsQueryObject( hObject);
              if(somIsObj(wpFolder)) {
                methodPtr= (somTD_CWMenuFolder_mfInsertMenuItems) (wpFolder->somGetClass())->somFindSMethod(id);  
                if(methodPtr) {
                  cwInsertMenuSeparator(MIT_END, hwndMenu, NULL);
                  methodPtr(wpFolder, hwndMenu , MIT_END, FIRSTUSERITEM);
                }
                //ulCurrentID=methodPtr(wpFolder, hwndMenu,iPosition, MYMENUID);
                /* ulCurrentID contains the last used menu id */
                wpFolder->wpUnlockObject();//Object is locked because of wpclsQueryHandle()
              }/* if(somIsObj(wpFolder)) */
            }/* end of if(somIsObj(m_wpObject))  */
          }/* Menufolder found */
          
          mi.iPosition=iItemCount-1;
          mi.afStyle=MIS_TEXT|MIS_SUBMENU;
          mi.id=ID_TBMENUTOOLS;
          mi.afAttribute=NULL;                
          mi.hwndSubMenu=hwndMenu;
          mi.hItem=NULL;
          /* Text: "Misc-Tools" */
          getMessage(text, IDSTR_MISCTOOLS,sizeof(text), hDataResource, hwnd);
          WinSendMsg(myFrameHWND,MM_INSERTITEM,(MPARAM)&mi,
                     (MPARAM)text);

          /* Check if ISOFS is installed. This is done by looking for the <ISOFS_MOUNT> object. */
          if((hObject=WinQueryObject(ID_ISOFS_MOUNT))!=NULLHANDLE)
            bHaveIsoFS=TRUE;
          else {
            /* Not installed or broken installation. Remove the ISOFS menuitem */
            WinSendMsg(myFrameHWND,MM_DELETEITEM,MPFROM2SHORT(ID_ISOFSITEM, TRUE),
                       (MPARAM)0);
            bHaveIsoFS=FALSE;
          }
        }/* end of if(hwndMenu) */
      }/* if(myFrameHWND) */
      
      myFrameHWND=WinWindowFromID(hwnd,FID_CLIENT);//Get container hwnd
      if(myFrameHWND){
        pfnwpContainer=WinSubclassWindow(myFrameHWND,dataContainerProc);
      }

#ifdef NO_WPSWIZARD      
      /* This is the imagename-frameCtrl. */
      myFrameHWND=WinLoadDlg(hwnd,hwnd,imageNameDialogProc,hDataResource,IDDLG_IMAGENAME,this);
      hwndImageName=myFrameHWND;
      if(myFrameHWND){
        WinQueryWindowRect(myFrameHWND,&rectl);
        sizelLeft.cy=rectl.yTop-rectl.yBottom;
        cwAddFrameCtl(hwnd, myFrameHWND, sizelLeft, FCTL_TOP,0);
      }
#endif
      /* This is the main mkisofs frame control */
      myFrameHWND=WinLoadDlg(hwnd,hwnd,mkisofsMainDialogProc,hDataResource,IDDLG_MKISOFSMAIN,this);
      hwndMkisofsMain=myFrameHWND;
      if(myFrameHWND){
        WinQueryWindowRect(myFrameHWND,&rectl);
        sizelLeft.cx=rectl.xRight-rectl.xLeft;
        cwAddFrameCtl(hwnd, myFrameHWND, sizelLeft, FCTL_LEFT,FCTL_POSBELOW|FCTL_POSABOVE);
      }
      /* This is the statusline frame control */
      myFrameHWND=WinLoadDlg(hwnd,hwnd,dataCDStatusLineDialogProc,hDataResource,IDDLG_STATUS,this);
      hwndStatusCntrl=myFrameHWND;
      if(myFrameHWND){
        WinQueryWindowRect(myFrameHWND,&rectl);
        sizelLeft.cy=rectl.yTop-rectl.yBottom;
        cwAddFrameCtl(hwnd, myFrameHWND, sizelLeft, FCTL_BOTTOM,0);
        // Text: "Put files and directories into this folder to burn them." 
        getMessage(text,IDSTRD_PUTFILES, sizeof(text), hDataResource,hwnd);
        cwSetStatusText(text);
      }
      cwUpdateFrame(hwnd);
      pfnwpFrame=WinSubclassWindow(hwnd, dataFrameProc);
      /* Fall back storage */
      pfnwpDataGenericFrame=pfnwpFrame;
    }
    CATCH(DATAFOLDER_OPEN)
      {
      } END_CATCH;
  }/*   if(ulView!=OPEN_SETTINGS) */
  if(!(globalData.ulGlobalFlags &  GLOBALFLAG_DAYTIPSHOWN) && globalData.bTipsEnabled) {
    sprintf(text, "%s\\bin\\pmdaytip.exe",chrInstallDir);
    HlpSendCommandToObject( text, "OPEN=DEFAULT");
    globalData.ulGlobalFlags|=GLOBALFLAG_DAYTIPSHOWN;
  }
  return hwnd;
}


BOOL CWDataFolder::wpRestoreState(ULONG ulReserved)
{
  ULONG keyLength;
  char profileName[CCHMAXPATH];
  char moduleName[CCHMAXPATH];
  char *chrPtr;
  HINI hini=0;
  ULONG ulSize;

  memset(chrStatusText,0,sizeof(chrStatusText));

  /* Get mkisofsflags from folder-EA */
  if(!wpRestoreLong("CWDataFolder",IDKEY_MKISOFSFLAGS,&ulMkisofsFlags))
    ulMkisofsFlags=IDMK_ALLOW32CHARS|IDMK_LEADINGPERIODS|IDMK_ROCKRIDGE|IDMK_LOGFILE|IDMK_ALLFILES;
  
  /* Get createflags from folder-EA */
  if(!wpRestoreLong("CWDataFolder",IDKEY_CREATEFLAGS,&ulCreateFlags))
    ulCreateFlags=IDCR_IMAGEONLY|IDCR_TESTONLY;
  if(ulCreateFlags & IDCR_TESTONLY)
    sDummy=1;
  else
    sDummy=0;

  /* Get CD-type flags from folder-EA */
  if(!wpRestoreLong("CWDataFolder",IDKEY_CDTYPEFLAGS,&ulCDTypeFlags))
    ulCDTypeFlags=IDCDT_USEWRITER|IDCDT_TRACKDATA;
  keyLength=sizeof(chrApplication);
  if(!wpRestoreString("CWDataFolder",IDKEY_APPLICATION,chrApplication,&keyLength))
    sprintf(chrApplication,"");
  keyLength=sizeof(chrPublisher);
  if(!wpRestoreString("CWDataFolder",IDKEY_PUBLISHER,chrPublisher,&keyLength))
    sprintf(chrPublisher,"");
  keyLength=sizeof(chrPreparer);
  if(!wpRestoreString("CWDataFolder",IDKEY_PREPARER,chrPreparer,&keyLength))
    sprintf(chrPreparer,"");
  keyLength=sizeof(chrCopyright);
  if(!wpRestoreString("CWDataFolder",IDKEY_COPYRIGHT,chrCopyright,&keyLength))
    sprintf(chrCopyright,"");
  keyLength=sizeof(chrVolumeName);
  if(!wpRestoreString("CWDataFolder",IDKEY_VOLUMENAME,chrVolumeName,&keyLength))
    sprintf(chrVolumeName,"");
  keyLength=sizeof(chrBootImage);
  if(!wpRestoreString("CWDataFolder",IDKEY_BOOTIMAGE,chrBootImage,&keyLength))
    sprintf(chrBootImage,"");
  keyLength=sizeof(chrBootCatalog);
  if(!wpRestoreString("CWDataFolder",IDKEY_BOOTCATALOG,chrBootCatalog,&keyLength))
    sprintf(chrBootCatalog,"");
  keyLength=sizeof(chrImageName);
  if(!wpRestoreString("CWDataFolder",IDKEY_IMAGENAME,chrImageName,&keyLength))
    sprintf(chrBootCatalog,"");

  return CWProgFolder::wpRestoreState(ulReserved);
}

BOOL CWDataFolder::wpSaveState()
{
  char profileName[CCHMAXPATH];
  char moduleName[CCHMAXPATH];
  char *chrPtr;
  HINI hini=0;
  BOOL rc=TRUE;
  char chrDev[10];
  ULONG ulFlags;

  if(!wpSaveLong("CWDataFolder",IDKEY_MKISOFSFLAGS,ulMkisofsFlags))
    rc=FALSE;
  ulFlags=ulCreateFlags;
  if(sDummy)
    ulFlags|=IDCR_TESTONLY;
  if(!wpSaveLong("CWDataFolder",IDKEY_CREATEFLAGS,ulFlags))
    rc=FALSE;
  if(!wpSaveLong("CWDataFolder",IDKEY_CDTYPEFLAGS,ulCDTypeFlags))
    rc=FALSE;
  if(!wpSaveString("CWDataFolder",IDKEY_APPLICATION,chrApplication))
    rc=FALSE;
  if(!wpSaveString("CWDataFolder",IDKEY_PUBLISHER,chrPublisher))
    rc=FALSE;
  if(!wpSaveString("CWDataFolder",IDKEY_PREPARER,chrPreparer))
    rc=FALSE;
  if(!wpSaveString("CWDataFolder",IDKEY_COPYRIGHT,chrCopyright))
    rc=FALSE;
  if(!wpSaveString("CWDataFolder",IDKEY_VOLUMENAME,chrVolumeName))
    rc=FALSE;
  if(!wpSaveString("CWDataFolder",IDKEY_BOOTIMAGE,chrBootImage))
    rc=FALSE;
  if(!wpSaveString("CWDataFolder",IDKEY_BOOTCATALOG,chrBootCatalog))
    rc=FALSE;
  if(!wpSaveString("CWDataFolder",IDKEY_IMAGENAME,chrImageName))
    rc=FALSE;


  /* Open the ini-file */
  /* Build full path */
  sprintf(profileName,"%s\\cdrecord.ini",chrInstallDir);       
  hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),profileName);
  if(!hini) {
    /* profileName: "Warning! Cannot open Ini-file!"
       moduleName: "Audio-CD-Creator"
       */
    messageBox( profileName, IDSTR_INIFILEOPENWARNING , sizeof(profileName),
                moduleName, IDSTR_AUDIOCDCREATOR, sizeof(moduleName),
                hDataResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);    
  }/* end of if(!hini) */
  else {
    PrfWriteProfileString(hini,"Mkisofs","imagepath",chrImage);
    PrfCloseProfile(hini);
  }

  if(!rc){
    /* profileName: "Warning! Cannot write instance data!"
       moduleName: "Data-CD-Creator"
       */
    messageBox( profileName, IDSTR_WRITEINSTANCEDATAWARNING , sizeof(profileName),
                moduleName, IDSTRD_DATACDCREATOR, sizeof(moduleName),
                hDataResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
  }
  return CWProgFolder::wpSaveState();
}

BOOL CWDataFolder::cwClose(HWND hwndFrame)
{
  wpSaveDeferred();
  WinSendMsg(WinWindowFromID(hwndImageName,2055),WM_CLOSE,0,0);
  return CWProgFolder::cwClose(hwndFrame);
}




