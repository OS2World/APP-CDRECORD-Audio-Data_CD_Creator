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

#include "datafolder.h"
#include "audiofolder.hh"
#include "audiofolderhelp.h"

#include <stdlib.h>
#include <stdio.h>


#include "menufolder.hh"

extern GLOBALDATA globalData;
/* This is the handle for the ressource DLL holding the NLS messages and dialogs */
extern HMODULE hDataResource;
extern BOOL bHaveIsoFS;
extern char chrInstallDir[CCHMAXPATH];

extern char chrDVDDao[CCHMAXPATH];/* Path to dvdao */
extern char chrDVDDaoOptions[CCHMAXPATH];

void errorResource();
ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);
ULONG showMsgBox2(HWND hwnd, ULONG ulIDTitle, ULONG ulIDText, HMODULE hModule, ULONG ulFlag);
void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
void HlpSendCommandToObject(char* chrObject, char* command);

MRESULT cwInsertMenuSeparator(int iPosition, HWND hwndMenu, HWND hwndSubMenu);
MRESULT EXPENTRY dataContainerProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY dataCDStatusLineDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY dvdMainDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY dataFrameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY selectWriterDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

void SysWriteToTrapLog(const char* chrFormat, ...);
BOOL checkMkisofsExists(HWND hwnd, HMODULE hResource);
void mboxShowMissingProgMsgBox(HWND hwnd, char * chrWhichProg, HMODULE hResource, BOOL bOpenSettings);
BOOL checkFileExists(char* chrFileName);
void buildDataDVDWriteParam(CWDataFolder* thisPtr, char * text ,int iSpeedChosen, char *chrDev);
BOOL buildMkisofsParam(CWDataFolder* thisPtr, char * text, char * outputName, char * pathListFile, char *chrChosenDev);
BOOL buildMkisofsParamShadowsRootOnly(CWDataFolder* thisPtr, char * text, char * outputName, char *chrChosenDev);
BOOL buildBinFileName(char *chrBuffer);
void _Optlink dvdSizeThreadFunc(void *arg);
ULONG launchPMWrapper(PSZ parameter, PSZ folderPath, PSZ wrapperExe, PSZ pszTitle);

typedef ULONG   SOMLINK somTP_CWMenuFolder_mfInsertMenuItems(CWMenuFolder *somSelf,
		HWND hwndMenu,
		ULONG iPosition,
		ULONG ulLastMenuId);
//#pragma linkage(somTP_CWMenuFolder_mfInsertMenuItems, system)
typedef somTP_CWMenuFolder_mfInsertMenuItems *somTD_CWMenuFolder_mfInsertMenuItems;



/* This thread waits for the mkisofs process to end */
void _Optlink dvdOnTheFlyThreadFunc (void *arg)
{
  HWND hwndMkisofs;
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  CWDataFolder * thisPtr;
  char *text;
  ULONG ulFlags;
  //  ULONG ulCDTypeFlags;
  char *chrPtrParam;
  BOOL bBreak=FALSE;
  ULONG ulSize;
  char chrFileList[CCHMAXPATH]="";
  char name[CCHMAXPATH*2+10];
  FILE * file;
  ULONG ulError;
  DEVICEINFO *devInfo;
  int iSpeedLocal;

  thisPtr=(CWDataFolder*)arg;

  if(!somIsObj(thisPtr))
    return;

  /* Get memory for parameters. The parameters will be written to a file later and the memory freed */ 
  ulError=0;
  if((text=thisPtr->wpAllocMem(SHAREDMEM_SIZE, &ulError))==NULLHANDLE) {
    return;
  }
  memset(text, 0, SHAREDMEM_SIZE);
    
  /* We use the mesg queue and the object window only to have msg queue for some window functions.
     We may use the window for some future enhancements, too. */
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {
      /* Disable controls of the dialog */
      thisPtr->cwEnableWriteControls(FALSE);
      
      /* Select writer and speed */
      /* Get local mem */
      ulError=0;
      if((devInfo=(DEVICEINFO*)thisPtr->wpAllocMem(sizeof(DEVICEINFO), &ulError))!=NULLHANDLE) {
        memset(devInfo, 0, sizeof(DEVICEINFO));         
        if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, selectWriterDialogProc, hDataResource, IDDLG_SELECTDEVICE, devInfo)
            == DID_ERROR )
          {
            thisPtr->wpFreeMem((PBYTE)devInfo);
            bBreak=TRUE;
          } 
        else {
          /* User selected a writer and speed */
          iSpeedLocal=devInfo->iSpeed;
          strncpy(name , devInfo->chrDev, sizeof(devInfo->chrDev));
          thisPtr->wpFreeMem((PBYTE)devInfo);
        }
      }
      else
        bBreak=TRUE;/* There was some kind of error */

      /* Lower thread priority or the GUI will become slow while querying the tree */
      DosSetPriority(PRTYS_THREAD,PRTYC_IDLETIME,-2,0);
      
#if 0
      if(!bBreak) {
        /* Lower thread priority or the GUI will become slow while querying the tree */
        DosSetPriority(PRTYS_THREAD,PRTYC_IDLETIME,-2,0);
        
        /* Check if we should write a multisession CD */
        ulFlags=thisPtr->cwQueryCDTypeFlags();
        
        if(ulFlags & IDCDT_MULTISESSION) {
          /* Check CD for multisession info */

          hwndMkisofs=WinCreateWindow(HWND_OBJECT,WC_STATIC,"myMkisofsObj",0,0,0,0,0,NULL,HWND_BOTTOM,12341,NULL,NULL);
          if(hwndMkisofs) {
            char tempText[20];
            WinSubclassWindow(hwndMkisofs,&onTheFlyObjectProc);
            /* Send object pointer to our window for later use */
            WinSendMsg(hwndMkisofs,atomStartWrite,MPFROMP(thisPtr),MPFROMLONG(ACKEY_ONTHEFLY));
                        
            /* We are building the second image of a multisession disk */
            /* Get the sector information */
            /* Putting 0 into the sectors will trigger an error if something goes wrong in the window procedure */
            thisPtr->cwSetPreviousStartSector(0);
            thisPtr->cwSetNextStartSector(0);
            
            /* Get msinfo */
            thisPtr->iCreate=3;
            
            sprintf(tempText, "dev=%s", name);
            launchWrapper(tempText, "", hwndMkisofs, "msinfo.exe", "Query multisession info");
            
            while(WinGetMsg(hab,&qmsg,(HWND)NULL,0,0))
              WinDispatchMsg(hab,&qmsg);
            WinDestroyWindow(hwndMkisofs);
          }/* hwnd */
        }/* if((ulFlags & IDCDT_MULTISESSION) && !(ulFlags & IDCDT_FIRSTSESSION)) */
      }/* end of if(!bBreak) */
#endif
 
      if(!bBreak) {
        /* Text: "Collecting files..." */
        getMessage(chrFileList, IDSTRD_COLLECTINGFILES , sizeof(chrFileList), hDataResource,HWND_DESKTOP);
        sprintf(thisPtr->chrStatusText, chrFileList);
        thisPtr->cwSetStatusText(NULL);

        ulFlags=thisPtr->cwQueryMkisofsFlags();
        
        /* Add the mkisofs command                     */
        /* This routine also check if a boot image     */
        /* is avaiable if boot CD creation is selected */
        if(ulFlags & IDMK_SHADOWSINROOTONLY) {
          if(!buildMkisofsParamShadowsRootOnly(thisPtr, text,"-print-size", name)) {
            /* There was an error */
            thisPtr->cwSetStatusText(NULL);
            bBreak=TRUE;/* The memory is freed in the function */
          }
        }
        else {
          /* Follow all Shadows */
          if(!buildMkisofsParam(thisPtr, text,"-print-size", chrFileList, name)) {
            /* There was an error */
            thisPtr->cwSetStatusText(NULL);
            bBreak=TRUE;/* The memory is freed in the function */
          }
        }/* else if(ulFlags & IDMK_SHADOWSINROOTONLY) */
      }

      if(!bBreak) {
        sprintf(thisPtr->chrStatusText,"");
        thisPtr->cwSetStatusText(NULL);

        /* File collecting was successful */
        //ulFlags=thisPtr->cwQueryCDTypeFlags();
        /* Add dvdao command */
        strcat(text," | ");
        /* CDRecord path */
        strcat(text,chrDVDDao);
        strcat(text,"  ");
        chrPtrParam=strrchr(text,' ');
        
        /* Add dvdao options */
        buildDataDVDWriteParam(thisPtr, chrPtrParam, iSpeedLocal, name);
        
        /* Check if we perform a dummy write */
        if(thisPtr->sDummy==1)
          strcat(text," --dummy");// Dummy write
        
        /* Tell dvddao to read from stdin */
        strcat(text," -");
        
        /* This PM wrapper checks the image- and CD size and controls the status
           window */ 
        sprintf(name,"\"%s\" \"", chrFileList);/* The filelist file if following all shadows */
        /* Get parameter filename */
        if(buildBinFileName(chrFileList)) {
          strcat(name, chrFileList); 
          strcat(name, "\"");
          if((file=fopen(chrFileList,"wb"))!=NULL) {
            char progTitle[255];

            fwrite(text, sizeof(char), SHAREDMEM_SIZE, file);  
            fclose(file);
            
            /* Text: "Writing on the fly..." */
            getMessage(chrFileList, IDSTR_ONTHEFLYWRITING, sizeof(chrFileList), hDataResource,HWND_DESKTOP);
            thisPtr->cwSetStatusText(chrFileList);
            
            ulSize=sizeof(chrFileList);
            thisPtr->wpQueryRealName(chrFileList, &ulSize, TRUE);

            /* "Writing on the fly" */
            getMessage(progTitle, IDSTRD_ONTHEFLYTITLE, sizeof(progTitle), hDataResource, HWND_DESKTOP);
            launchPMWrapper( name, chrFileList,"pmdvdfly.exe", progTitle);
            thisPtr->cwEnableWriteControls(TRUE);
            DosSleep(5000);/* Show the writing message in the status line for 5 secs */
          }/* file */
        }/* end of if(buildBinFileName(chrFileList)) */
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  /* Free cmd-line mem */
  if(text)
    thisPtr->wpFreeMem(text);
  
  thisPtr->cwEnableWriteControls(TRUE);
  sprintf(thisPtr->chrStatusText," ");
  thisPtr->cwSetStatusText(NULL); 
}

static BOOL checkDVDDaoExists(HWND hwnd, HMODULE hResource)
{

  if(checkFileExists(chrDVDDao))
    return TRUE;

  /* The path to CDRecord/2 isn't valid */
  mboxShowMissingProgMsgBox( hwnd, "Dvddao", hResource, TRUE);

  /* Show the cdrecord setup help panel */
  HlpSendCommandToObject("<CWCREATOR_SETTINGS>", SETUP_DISPLAYHELPPANEL"="IDHLP_CDRECORDSETUPSTR);

  return FALSE;
}

ULONG CWDVDFolder::AddCDTypeOptionPage(HWND hwndNotebook)
{
  return SETTINGS_PAGE_REMOVED; 
}

/* This method starts a PM wrapper which handles all the on the fly stuff */
void CWDVDFolder::cwWriteOnTheFly()
{
  ULONG ulFlags;
  ULONG ulSize;

  if(!checkDVDDaoExists(hwndMkisofsMain, hDataResource) || !checkMkisofsExists(hwndMkisofsMain, hDataResource))
    return;

#if 0
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
#endif

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
  
  _beginthread(dvdOnTheFlyThreadFunc,NULL,8192*16,(void*)this); //Fehlerbehandlung fr Start fehlt  

  return;
}

HWND CWDVDFolder::wpOpen(HWND hwndCnr,ULONG ulView,ULONG ulParam)
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


  if(ulView!=OPEN_SETTINGS){
    if(!hDataResource) {
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
          if((hObject=WinQueryObject(USERMENUFOLDER_DATADVD))!=NULLHANDLE) {//is there a default menufolder?    
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

#if 0      
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

#endif

      /* This is the main mkisofs frame control */
      myFrameHWND=WinLoadDlg(hwnd,hwnd, dvdMainDialogProc, hDataResource, IDDLG_DVDMAIN, this);
      hwndMkisofsMain=myFrameHWND;
      if(myFrameHWND){
        WinQueryWindowRect(myFrameHWND,&rectl);
        sizelLeft.cx=rectl.xRight-rectl.xLeft;
        cwAddFrameCtl(hwnd, myFrameHWND, sizelLeft, FCTL_LEFT,FCTL_POSBELOW|FCTL_POSABOVE);
      }


      /* This is the statusline frame control */
      myFrameHWND=WinLoadDlg(hwnd,hwnd, dataCDStatusLineDialogProc, hDataResource, IDDLG_STATUS, this);
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

#if 0
      pfnwpFrame=WinSubclassWindow(hwnd, dataFrameProc);
      /* Fall back storage */
      //      pfnwpDataGenericFrame=pfnwpFrame;
#endif
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

BOOL CWDVDFolder::wpMenuItemSelected(HWND hwndFrame,ULONG ulMenuId)
{
  int tid;
  char text[CCHMAXPATH+4];
  char title[CCHMAXPATH];
  ULONG ulSize;

  switch(ulMenuId)
    {
    case ID_CDSIZEITEM:
      _beginthread(dvdSizeThreadFunc,NULL,8192*16,this); //Fehlerbehandlung fehlt
      /* Text: "Querying free CD space. Please wait..." */
      getMessage(text,IDSTRD_QUERYFREEDVDSPACE, sizeof(text), hDataResource,hwndFrame);
      cwSetStatusText(text);    
      return TRUE;
    default:
      break;
    }
  return CWDataFolder::wpMenuItemSelected( hwndFrame, ulMenuId);
}



