/*
 *
 * Main Source file for class CWProgFolder
 *
 *             (C) Chris Wohlgenuth 1999-2004
 *
 * This class is intended for programmers and provides functions for some
 * common tasks like:
 *                     - Adding dialogs as frame controls to WPS folders
 *                     - Showing 'About'-dialogs
 *                     - Checking of object pointers
 *                     - ...
 *
 * Use IBM VAC++ V3.08 and Gnu-make 3.74.1 to compile
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
#define INCL_DOSQUEUES
#define  INCL_PM
#include "progfolder.h"
#include "progfolder.hh"
#include "menufolder.hh"
#include <stdlib.h>
#include <stdio.h>


#define DEFAULT_DIALOG_FONT   "9.WarpSans"

typedef ULONG   SOMLINK somTP_CWMenuFolder_mfInsertMenuItems(CWMenuFolder *somSelf,
		HWND hwndMenu,
		ULONG iPosition,
		ULONG ulLastMenuId);
typedef somTP_CWMenuFolder_mfInsertMenuItems *somTD_CWMenuFolder_mfInsertMenuItems;

typedef BOOL   SOMLINK somTP_CWMenuFolder_mfCheckMenuItems(CWMenuFolder *somSelf,
		WPObject* wpObject,
		ULONG ulMenuId);
typedef somTP_CWMenuFolder_mfCheckMenuItems *somTD_CWMenuFolder_mfCheckMenuItems;

MRESULT EXPENTRY folderFrameProcedure(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2);
HWND createStoreWindow(HWND hwndCLF);
HWND createLeechFrame(HWND hwndCLF);
BOOL cwRegisterSeparatorWindowClass(void);

HPOINTER hptrHand=NULLHANDLE;

/****************************************************
 *                                                  *
 * This function returns the running OS version:    *
 *                                                  *
 * 30: Warp 3, 40 Warp 4 etc.                       *
 *                                                  *
 ****************************************************/
static ULONG cwQueryOSRelease()
{
  static ULONG ulVersionMinor=0;

  if(!ulVersionMinor)
    if(DosQuerySysInfo(QSV_VERSION_MINOR, QSV_VERSION_MINOR, &ulVersionMinor, sizeof(ulVersionMinor)))
      ulVersionMinor=30;/* Default Warp 3 */

  return ulVersionMinor;

}

/**************************************************************/
/*                                                            */
/* This function returns the module handle of our class-dll   */
/*                                                            */
/**************************************************************/
HMODULE queryModuleHandle(void)
{
  static HMODULE hmod=0;
  
  if(!hmod) {
    PSZ pathname=SOMClassMgrObject  //Query Pathname of class file
      ->somLocateClassFile(somIdFromString("CWProgFolder"),1,2);
    DosQueryModuleHandle(pathname,&hmod);  //Query module handle
  }
  return hmod;
}

/****************************************************
 *                                                  *
 * This funktion inserts a separator into menu      *
 * <hwndMenu> and submenu <hwndSubMenu> at          *
 * position <iPosition>                             *
 *                                                  *
 ****************************************************/
MRESULT cwInsertMenuSeparator(int iPosition, HWND hwndMenu, HWND hwndSubMenu)
{
  MENUITEM mi;

  /* Fill the MENUITEM structure */
  mi.iPosition=iPosition;
  mi.afStyle=MIS_SEPARATOR;
  if(hwndSubMenu)
    mi.afStyle|=MIS_SUBMENU;
  mi.id=0;
  mi.afAttribute=NULL;                
  mi.hwndSubMenu=hwndSubMenu;
  mi.hItem=NULL;

  return WinSendMsg(hwndMenu,MM_INSERTITEM,(MPARAM)&mi,
                    (MPARAM)NULL);                 
}

/**************************************************************/
/*                                                            */
/* Overriden function: wpclsQueryStyle()                      */
/*                                                            */
/* This class doesn't need a template.                        */
/*                                                            */
/**************************************************************/
ULONG M_CWProgFolder::wpclsQueryStyle()
{
  somId mySomId;
  ULONG rc;

  if((mySomId=somIdFromString("wpclsQueryStyle"))!=NULLHANDLE) {
    rc=((somTD_M_WPObject_wpclsQueryStyle)
          somParentNumResolve(__ClassObject->somGetPClsMtabs(),
                              1,
                              __ClassObject->
                              somGetMethodToken(mySomId))
          )(this)/*|CLSSTYLE_DONTTEMPLATE*/|CLSSTYLE_NEVERTEMPLATE;    
    SOMFree(mySomId);
  }
  else
    rc=M_WPFolder::wpclsQueryStyle()/*|CLSSTYLE_DONTTEMPLATE*/|CLSSTYLE_NEVERTEMPLATE;

  return rc;

#if 0
  return ((somTD_M_WPObject_wpclsQueryStyle)
          somParentNumResolve(__ClassObject->somGetPClsMtabs(),
                              1,
                              __ClassObject->
                              somGetMethodToken(somIdFromString
                                                ("wpclsQueryStyle")))
          )(this)|CLSSTYLE_DONTTEMPLATE/*|CLSSTYLE_NEVERTEMPLATE*/;
#endif

}

/**************************************************************/
/*                                                            */
/* Overriden function: wpclsQueryTitle()                      */
/*                                                            */
/* Returns our class name                                     */
/*                                                            */
/**************************************************************/
PSZ M_CWProgFolder::wpclsQueryTitle()
{
  return "Support-Folder";
}


void M_CWProgFolder::wpclsInitData()
{
  somId mySomId;

  /* call parent */
  if((mySomId=somIdFromString("wpclsInitData"))!=NULLHANDLE) {
    ((somTD_M_WPObject_wpclsInitData)
     somParentNumResolve(__ClassObject->somGetPClsMtabs(),
                         1,
                         __ClassObject->
                         somGetMethodToken(mySomId))
     )(this);
    SOMFree(mySomId);
  }
  else
    M_WPFolder::wpclsInitData();

  /* Register the separator window class */
  cwRegisterSeparatorWindowClass();
}

/* This Funktion is called when the close button of the */
/* frame is pressed or if the user chooses 'close' from */
/* the menu. Apparently the wpClose() method isn't      */
/* called when choosing close.                          */
BOOL CWProgFolder::cwClose(HWND hwndFrame)
{
  return TRUE;
}

/**************************************************************/
/*                                                            */
/* Overriden function: wpOpen()                               */
/*                                                            */
/* This function subclasses the folder frame window and       */
/* prepares it for the addition of frame controls.            */
/*                                                            */
/**************************************************************/
HWND CWProgFolder::wpOpen(HWND hwndCnr,ULONG ulView,ULONG ulParam)
{
  HWND hwnd;
  HWND hwndStore;
  PFNWP orgFrameProc;
  char * memPtr;
  somId mySomId;

  if((mySomId=somIdFromString("wpOpen"))!=NULLHANDLE) {
    hwnd=((somTD_WPObject_wpOpen)
          somParentNumResolve(__ClassObject->somGetPClsMtabs(),
                              1,
                              __ClassObject->
                              somGetMethodToken(mySomId))                    
          )(this,hwndCnr, ulView, ulParam);
    SOMFree(mySomId);
  }
  else
    hwnd=WPFolder::wpOpen(hwndCnr, ulView, ulParam);

#if 0
  //  hwnd=WPFolder::wpOpen(hwndCnr, ulView, ulParam);/* Call parent to build folder window */
  hwnd=((somTD_WPObject_wpOpen)
        somParentNumResolve(__ClassObject->somGetPClsMtabs(),
                            1,
                            __ClassObject->
                            somGetMethodToken(somIdFromString
                                              ("wpOpen")))                    
        )(this,hwndCnr, ulView, ulParam);
#endif

  if(ulView==OPEN_SETTINGS)
    /* We only build frame controls for folder frame windows */
    return hwnd;
  if(WinWindowFromID(hwnd,ID_FOLDERSTOREWINDOW))
    return hwnd;/* Already subclassed */
  
  hwndStore=createStoreWindow(hwnd);/* This holds our object pointer */
  if(hwndStore){
    /* Save 'this' ptr. */
    WinSetWindowPtr(hwndStore,0,this);
    /* Subclass the folder frame window */
    orgFrameProc=WinSubclassWindow(hwnd,&folderFrameProcedure);
    /* Save the old window procedure in our store window */
    WinSetWindowPtr(hwndStore,QWP_ORGFRAMEPROC,orgFrameProc);
    /* Save a ptr. to private control data */
    memPtr=(char*)malloc(sizeof(FRAMECTRLDATA));
    if(memPtr)
      memset(memPtr,0,sizeof(FRAMECTRLDATA));
    WinSetWindowPtr(hwndStore,QWP_FCTRLDATA,memPtr);
  }
  
  return hwnd;
}

/**************************************************************/
/*                                                            */
/* This member function returns the count of additional frame */
/* controls we have added to the frame. This fuction is       */
/* called from the frame window procedure                     */
/*                                                            */
/**************************************************************/
ULONG CWProgFolder::cwNumAdditionalFrameControls(HWND hwndFolder)
{
  ULONG rc;
  FRAMECTRLDATA * fctrlData;
  
  fctrlData=(FRAMECTRLDATA*)WinQueryWindowPtr(WinWindowFromID(hwndFolder, ID_FOLDERSTOREWINDOW),QWP_FCTRLDATA);
  if(!fctrlData) return 0;
  
  rc=0;
  
  if(fctrlData->bLeft)rc++;
  if(fctrlData->bRight)rc++;
  if(fctrlData->bTop)rc++;
  if(fctrlData->bBottom)rc++;

  return rc;
}

/**************************************************************/
/*                                                            */
/* This function adjusts the size and position of the client. */
/* It is called from the frame window procedure               */
/*                                                            */
/**************************************************************/
BOOL CWProgFolder::cwCalculateClient(HWND hwndFolder, PRECTL pRectl, BOOL which)
{
  FRAMECTRLDATA * fctrlData;
  
  //  which:
  //     TRUE:  Frame rectangle provided, calculate client
  //     FALSE: Client area rectangle provided, calculate frame

  /* Get our control data */
  fctrlData=(FRAMECTRLDATA*)WinQueryWindowPtr(WinWindowFromID(hwndFolder, ID_FOLDERSTOREWINDOW),QWP_FCTRLDATA);
  if(!fctrlData) return FALSE;


  if (SHORT1FROMMP(which))
    //     TRUE:  Frame rectangle provided, calculate client
    //     FALSE: Client area rectangle provided, calculate frame
    {
      //  TRUE: calculate the rectl of the client;
      //  call default window procedure to subtract child frame
      //  controls from the rectangle's height
      LONG lClientHeight;
      LONG lNewControls;

      lNewControls=0;
      if(fctrlData->bTop)
        lNewControls=fctrlData->sizelTopFrame.cy;
      if(fctrlData->bBottom)
        lNewControls+=fctrlData->sizelBottomFrame.cy;
    
      lClientHeight = pRectl->yTop - pRectl->yBottom;
      if ( lNewControls  > lClientHeight  )
        {
          if(fctrlData->bBottom || fctrlData->bTop)
            // extension is taller than client, so set client height to 0
            pRectl->yTop = pRectl->yBottom;
        }
      else
        {
          //  set the origin of the client and shrink it based upon the
          //  frame control's height
          if(fctrlData->bBottom) {
            pRectl->yBottom +=fctrlData->sizelBottomFrame.cy;
            //pRectl->yTop -= fctrlData->sizelBottomFrame.cy;
          }
          if(fctrlData->bTop) {
            pRectl->yTop -=fctrlData->sizelTopFrame.cy;
            //pRectl->yBottom +=fctrlData->sizelTopFrame.cy;
          }
        }

      lNewControls=0;
      if(fctrlData->bLeft)
        lNewControls=fctrlData->sizelLeftFrame.cx;
      if(fctrlData->bRight)
        lNewControls+=fctrlData->sizelRightFrame.cx;
      
      lClientHeight = pRectl->xRight - pRectl->xLeft;
      if ( lNewControls  > lClientHeight  )
        {
          if(fctrlData->bRight || fctrlData->bLeft)
            // extension is taller than client, so set client height to 0
            pRectl->xRight = pRectl->xLeft;
        }
      else
        {
          if(fctrlData->bLeft) 
            pRectl->xLeft+=fctrlData->sizelLeftFrame.cx;
          if(fctrlData->bRight)
            pRectl->xRight-=fctrlData->sizelRightFrame.cx;
        }
    }
  else
    {
      //  FALSE: calculate the rectl of the frame;
      //  call default window procedure to subtract child frame
      //  controls from the rectangle's height;
      //  set the origin of the frame and increase it based upon the
      //  static text control's height

      return TRUE;

      if(fctrlData->bBottom) {
        pRectl->yBottom +=fctrlData->sizelBottomFrame.cy;
        // pRectl->yTop -= fctrlData->sizelBottomFrame.cy;
      }
      if(fctrlData->bTop)
        pRectl->yTop -=fctrlData->sizelTopFrame.cy;

      if(fctrlData->bLeft) {
        pRectl->xLeft +=fctrlData->sizelLeftFrame.cx;
        //    pRectl->yRight -= fctrlData->sizelLeftFrame.cx;
      }
      if(fctrlData->bRight)
        pRectl->xRight -=fctrlData->sizelRightFrame.cx;

    }
  

  return TRUE;/* Everything's ok */
}



/**************************************************************/
/*                                                            */

/*                                                            */
/**************************************************************/
ULONG CWProgFolder::cwFormatFrame(HWND hwndFolder, USHORT countSWP,PSWP pswp)
{
  /* countSWP:  number of standard framecontrols
     pswp:      Array of SWP describing the framecontrols.
                Since we added the number of our controls to the
                standard controls in the framecontrol there is
                space for the new controls.
                */
  ULONG rc;
  PSWP pswpClient=0, pswpFrameCtl=0;
  int iCount;
  FRAMECTRLDATA * fctrlData;
  int a;
  CNRINFO     CnrInfo;
  HWND hwndClient;

  /* The data of our framecontrols is stored in a structure.
     Get a pointer from the window words of the invisible child. */
  fctrlData=(FRAMECTRLDATA*)WinQueryWindowPtr(WinWindowFromID(hwndFolder, ID_FOLDERSTOREWINDOW),QWP_FCTRLDATA);
  if(!fctrlData) return 0;

  iCount=0;
  rc=0;
    
  for (a = 0; a < countSWP; a++)
    {
      /* Find the client window */
      if ( WinQueryWindowUShort( pswp[a].hwnd, QWS_ID ) == 0x8008 ) // FID_CLIENT )
        {
          pswpClient=&pswp[a];
          
          /* Framecontrol at the top */                      
          if(fctrlData->bTop){
            pswpFrameCtl=&pswp[countSWP+iCount++];/* First free position */

            /* Adjust client and insert the new control into the array of SWPs */ 
             
            /* Decrease the height of the client to have place for the new control */
            pswp[a].cy -= fctrlData->sizelTopFrame.cy;

            /* Insert the HWND of the new control into the structure */
            pswpFrameCtl->hwnd=fctrlData->hwndCtlTop;
            /* The x-position is the same as that of the client */
            pswpFrameCtl->x  = pswpClient->x;
            /* The control is placed above the client */
            pswpFrameCtl->y  = pswpClient->y + pswpClient->cy;           
            /* The control is as wide as the client */
            pswpFrameCtl->cx = pswpClient->cx;
            /* Insert the height of the control */ 
            pswpFrameCtl->cy = fctrlData->sizelTopFrame.cy;
            /* Place the control on top of all windows */
            pswpFrameCtl->hwndInsertBehind = HWND_BOTTOM;
            /* Insert necessary flags */
            pswpFrameCtl->fl = SWP_MOVE | SWP_SIZE | SWP_NOADJUST | SWP_ZORDER | SWP_SHOW;

            /* Adjust position if flags are set. It's possible to place
               the control above the controls on the left and right or let these controls
               cover the full height of the folder. */
            if(fctrlData->bLeft){
              /* There is a frame control on the left side */
              if(!(fctrlData->ulFlagsLeft & FCTL_POSBELOW)) {
                pswpFrameCtl->x  += fctrlData->sizelLeftFrame.cx;
                pswpFrameCtl->cx -= fctrlData->sizelLeftFrame.cx; 
              }
            }
            /* framecontrol on the right side */
            if(!(fctrlData->ulFlagsRight & FCTL_POSBELOW) && fctrlData->bRight)
              pswpFrameCtl->cx -= fctrlData->sizelRightFrame.cx;

            if(!fctrlData->bCnrAdjusted) {
              hwndClient=pswpClient->hwnd;
              if(WinSendMsg(hwndClient,CM_QUERYCNRINFO,MPFROMP(&CnrInfo),MPFROMSHORT(sizeof(CnrInfo)))){
                if(fctrlData->bBottom)
                  CnrInfo.ptlOrigin.y-=fctrlData->sizelBottomFrame.cy;
                if(fctrlData->bTop)
                  CnrInfo.ptlOrigin.y-=fctrlData->sizelTopFrame.cy;
                //CnrInfo.ptlOrigin.y+=0;
                WinSendMsg(hwndClient,CM_SETCNRINFO,MPFROMP(&CnrInfo),MPFROMSHORT(CMA_PTLORIGIN));
              }
              fctrlData->bCnrAdjusted=TRUE;
            }
            /* One new control added */
            rc++;
          }/* end of if(fctrlData->bTop) */


          /* Left frame control */
          if(fctrlData->bLeft){
            pswpFrameCtl=&pswp[countSWP+iCount++];/* First free position */
            
            pswpClient->cx -= fctrlData->sizelLeftFrame.cx;/* Size client */
            pswpFrameCtl->hwnd=fctrlData->hwndCtlLeft;

            pswpFrameCtl->x  =pswpClient->x;
            pswpClient->x  += fctrlData->sizelLeftFrame.cx;/* Size client */
            pswpFrameCtl->y  =pswpClient->y;

            pswpFrameCtl->cx = fctrlData->sizelLeftFrame.cx;
            pswpFrameCtl->cy =pswpClient->cy;
                        
            pswpFrameCtl->hwndInsertBehind = HWND_BOTTOM;
            pswpFrameCtl->fl = SWP_MOVE | SWP_SIZE | SWP_NOADJUST | SWP_ZORDER | SWP_SHOW;
                        
            /* Position this frame control above bottom ctrl if requested */
            if((fctrlData->ulFlagsLeft & FCTL_POSABOVE) && fctrlData->bBottom){
              pswpFrameCtl->y  +=fctrlData->sizelBottomFrame.cy;
              pswpFrameCtl->cy  -=fctrlData->sizelBottomFrame.cy;
            }
            /* Position this frame control below top ctrl if requested */
            if(!(fctrlData->ulFlagsLeft & FCTL_POSBELOW) && fctrlData->bTop){
              pswpFrameCtl->cy  +=fctrlData->sizelTopFrame.cy;
            }
            rc++;
          }/* end of if(fctrlData->bLeft) */
                    
          /* Bottom frame control */
          if(fctrlData->bBottom){
            pswpFrameCtl=&pswp[countSWP+iCount++];/* First free position */
                        
            pswpClient->cy -= fctrlData->sizelBottomFrame.cy;    
            pswpFrameCtl->hwnd=fctrlData->hwndCtlBottom;
                        
            pswpFrameCtl->x  = pswpClient->x;
            pswpFrameCtl->y  = pswpClient->y;
            pswpClient->y += fctrlData->sizelBottomFrame.cy;
                        
            pswpFrameCtl->cx = pswpClient->cx;
            pswpFrameCtl->cy = fctrlData->sizelBottomFrame.cy;
                        
            pswpFrameCtl->hwndInsertBehind = HWND_BOTTOM;
            pswpFrameCtl->fl = SWP_MOVE | SWP_SIZE | SWP_NOADJUST | SWP_ZORDER | SWP_SHOW;
                                                
            /* Adjust position if flags are set */
            if(fctrlData->bLeft){
              if((fctrlData->ulFlagsLeft & FCTL_POSABOVE)) {
                pswpFrameCtl->cx +=fctrlData->sizelLeftFrame.cx;
                pswpFrameCtl->x  -= fctrlData->sizelLeftFrame.cx;
              }
            }
            if(!(fctrlData->ulFlagsRight & FCTL_POSABOVE)&& fctrlData->bRight) {
              pswpFrameCtl->cx -= fctrlData->sizelRightFrame.cx; 
            }
            /* Adjust container view so items do not scroll out of view */
            /*   hwndClient=pswpClient->hwnd;
                 if(WinSendMsg(hwndClient,CM_QUERYCNRINFO,MPFROMP(&CnrInfo),MPFROMSHORT(sizeof(CnrInfo)))){
                 CnrInfo.ptlOrigin.y-=pswpFrameCtl->cy;
                 WinSendMsg(hwndClient,CM_SETCNRINFO,MPFROMP(&CnrInfo),MPFROMSHORT(CMA_PTLORIGIN));
                 };*/

            if(!fctrlData->bCnrAdjusted) {
              hwndClient=pswpClient->hwnd;
              if(WinSendMsg(hwndClient,CM_QUERYCNRINFO,MPFROMP(&CnrInfo),MPFROMSHORT(sizeof(CnrInfo)))){
                if(fctrlData->bBottom)
                  CnrInfo.ptlOrigin.y-=fctrlData->sizelBottomFrame.cy;
                if(fctrlData->bTop)
                  CnrInfo.ptlOrigin.y-=fctrlData->sizelTopFrame.cy;
                //CnrInfo.ptlOrigin.y+=0;
                WinSendMsg(hwndClient,CM_SETCNRINFO,MPFROMP(&CnrInfo),MPFROMSHORT(CMA_PTLORIGIN));
              }
              fctrlData->bCnrAdjusted=TRUE;
            }
          
            rc++; 
          }/* end of if(fctrlData->bBottom) */
                    
          /* Right frame control */
          if(fctrlData->bRight){
            pswpFrameCtl=&pswp[countSWP+iCount++];/* First free position */
            
            pswpClient->cx -= fctrlData->sizelRightFrame.cx;/* Size client */
            pswpFrameCtl->hwnd=fctrlData->hwndCtlRight;

            pswpFrameCtl->x  =pswpClient->x+pswpClient->cx;
            pswpFrameCtl->y  =pswpClient->y;

            pswpFrameCtl->cx = fctrlData->sizelRightFrame.cx;
            pswpFrameCtl->cy =pswpClient->cy;
                        
            pswpFrameCtl->hwndInsertBehind = HWND_BOTTOM;
            pswpFrameCtl->fl = SWP_MOVE | SWP_SIZE | SWP_NOADJUST | SWP_ZORDER | SWP_SHOW;

            /* Position this frame control above bottom ctrl only if requested */
            if(!(fctrlData->ulFlagsRight & FCTL_POSABOVE) && fctrlData->bBottom){
              pswpFrameCtl->y  -=fctrlData->sizelBottomFrame.cy;
              pswpFrameCtl->cy  +=fctrlData->sizelBottomFrame.cy;
            }
            /* Position this frame control below top ctrl only if requested */
            if(!(fctrlData->ulFlagsRight & FCTL_POSBELOW) && fctrlData->bTop){
              pswpFrameCtl->cy  +=fctrlData->sizelTopFrame.cy;
            }
            rc++;
          }/* end of if(fctrlData->bRight) */
          

        }
    }
  return rc;/* Return additional controls */
}


/**************************************************************/
/*                                                            */
/* This function adds a frame control at the specified        */
/* position.                                                  */
/*                                                            */
/**************************************************************/
BOOL CWProgFolder::cwAddFrameCtl(HWND hwndFolder, HWND hwndNewCtl,SIZEL sizel, ULONG ulPos, ULONG ulFlags)
{
  FRAMECTRLDATA * fctrlData;
  
  /* Get the framecontrol datastructure from the invisible control window */
  fctrlData=(FRAMECTRLDATA*)WinQueryWindowPtr(WinWindowFromID(hwndFolder, ID_FOLDERSTOREWINDOW),QWP_FCTRLDATA);
  if(!fctrlData) return FALSE;
  
  /* Fill in the values into the structure */
  /* This is the dialog at the left */
  if(ulPos & FCTL_LEFT) {
    /* The HWND of the dialog used as a framcontrol */
    fctrlData->hwndCtlLeft=hwndNewCtl;
    /* The size of the dialog. Actually only sizel.cx is used
       since this is a dialog at the left which always has
       got the height of the client. */ 
    fctrlData->sizelLeftFrame=sizel;
    /* A flag which determines the positioning of the dialog. 
       If there is a dialog at the top you may tell the folder
       to move it sizel.cx pixel to the right to allow the
       control at the left to cover the full height of the folder.
       The same is with the control at the bottom. This gives
       you the ability to finetune the appearance of the
       folder. Just try the flags FCTL_POSBELOW, FCTL_POSABOVE as
       defined in progfolder.h in conjunction with framecontrols
       at different positions of the folder.
       */
    fctrlData->ulFlagsLeft=ulFlags;
    /* Tell the frame procedure that there's a framecontrol
       at the left. */
    fctrlData->bLeft=TRUE;
    if(!fctrlData->hwndCtlLeft)
      /* If this method is called with hwndNewCtl=NULL the framecontrol
         is disabled. */
      fctrlData->bLeft=FALSE;
  }
  /* This is the dialog at the right */
  if(ulPos & FCTL_RIGHT){
    fctrlData->hwndCtlRight=hwndNewCtl;
    fctrlData->sizelRightFrame=sizel;
    fctrlData->ulFlagsRight=ulFlags;
    fctrlData->bRight=TRUE;
    if(!fctrlData->hwndCtlRight) 
      fctrlData->bRight=FALSE;
  }
  if(ulPos & FCTL_TOP){
    fctrlData->hwndCtlTop=hwndNewCtl;
    fctrlData->sizelTopFrame=sizel;
    fctrlData->bTop=TRUE;
    if(!fctrlData->hwndCtlTop) 
      fctrlData->bTop=FALSE;
  }
  if(ulPos & FCTL_BOTTOM){
    fctrlData->hwndCtlBottom=hwndNewCtl;
    fctrlData->sizelBottomFrame=sizel;
    fctrlData->bBottom=TRUE;
    if(!fctrlData->hwndCtlBottom) 
      fctrlData->bBottom=FALSE;
  }
  return TRUE;
}


/**************************************************************/
/*                                                            */
/* This function returns information about the specified      */
/* private frame control.                                     */
/*                                                            */
/**************************************************************/
HWND CWProgFolder::cwQueryFrameCtl(HWND hwndFolder, SIZEL* sizel, ULONG ulPos, ULONG * ulFlags)
{
  FRAMECTRLDATA * fctrlData;
  
  fctrlData=(FRAMECTRLDATA*)WinQueryWindowPtr(WinWindowFromID(hwndFolder, ID_FOLDERSTOREWINDOW),QWP_FCTRLDATA);
  if(!fctrlData) return FALSE;
  
  
  if(ulPos & FCTL_LEFT){
    *sizel=fctrlData->sizelLeftFrame;
    *ulFlags= fctrlData->ulFlagsLeft;
    return fctrlData->hwndCtlLeft;
  }
  if(ulPos & FCTL_RIGHT){
    *sizel=fctrlData->sizelRightFrame;
    *ulFlags=fctrlData->ulFlagsRight;
    return fctrlData->hwndCtlRight;
  }
  if(ulPos & FCTL_TOP){
    *sizel=fctrlData->sizelTopFrame;
    *ulFlags=0;
    return  fctrlData->hwndCtlTop;
  }
  if(ulPos & FCTL_BOTTOM){
    *sizel=fctrlData->sizelBottomFrame;
    *ulFlags=0;
    return fctrlData->hwndCtlBottom;
  }
  return NULL;
}

/**************************************************************/
/*                                                            */
/* This function requests an update of frame controls so      */
/* added frame controls will become visible.                  */
/* You must call this function after calling cwAddFrameCtl(). */
/*                                                            */
/**************************************************************/
void CWProgFolder::cwUpdateFrame(HWND hwndFolder)
{

  WinSendMsg(hwndFolder,WM_UPDATEFRAME,0,0);

}

/**************************************************************/
/*                                                            */
/* This function returns the pointer to a file system object  */
/* or NULL.                                                   */
/* It follows shadow objects and returns the object handle    */
/* of the linked object if it's a file system object          */
/*                                                            */
/**************************************************************/
WPObject* CWProgFolder::cwGetFileSystemObject(WPObject* wpObject)
{
  if(!somIsObj(wpObject)) return NULL;//No object given
  
  /* Check if it's a shadow */
  if(somResolveByName(wpObject,"wpQueryShadowedObject")){ 
    /* Yes, it's a shadow. Query the linked object. */
    wpObject=((WPShadow*)wpObject)->wpQueryShadowedObject(FALSE);
  }
  
  if(!somIsObj(wpObject)) return NULL;//The link is broken
  
  /* Check if it's a file system object */
  if(somResolveByName(wpObject, "wpQueryRealName")){
    return wpObject;/* Yes */
  }
  else
    return NULL;
}

void paintURL(HPS hps, HWND hwnd, LONG lColor)
{
  POINTL aptl[TXTBOX_COUNT];
  POINTL ptl[2];
  int iLen;
  RECTL rectl;
  char chrURL[500];
  int a;
  LONG lSaveY;
  //  SIZEF fxSize;

  WinQueryWindowText(hwnd, sizeof(chrURL),chrURL);  
  iLen=strlen(chrURL);
  GpiQueryTextBox(hps, iLen, chrURL, TXTBOX_COUNT, aptl);
  WinQueryWindowRect(hwnd, &rectl);
  // WinDrawText(hps, -1L, chrURL,  &rectl, CLR_BLUE, CLR_BACKGROUND, DT_LEFT|DT_UNDERSCORE);

  if(aptl[TXTBOX_TOPRIGHT].x-aptl[TXTBOX_TOPLEFT].x <= rectl.xRight-rectl.xLeft)
    WinDrawText(hps, -1L, chrURL,  &rectl, lColor, CLR_BACKGROUND, DT_CENTER|DT_UNDERSCORE);
  else
    {
      /* Always draw the first 15 chars to speed up */
      a=15;
      ptl[0].x=0;
      lSaveY=rectl.yTop-aptl[TXTBOX_TOPLEFT].y;
      ptl[0].y=lSaveY;

      GpiSetColor(hps, lColor);
      GpiCharStringPosAt(hps, ptl,NULLHANDLE, CHS_UNDERSCORE, a, chrURL, 0);
      //      GpiQueryCharBox(hps, &fxSize);


      //      GpiQueryCharStringPos(hps, 0, 1, &chrURL[a], 0,ptl);
      while(a < iLen) {/* While chars in string */ 
        /* Get draw position of next char */
        GpiQueryCharStringPos(hps, 0, 1,  &chrURL[a], 0, ptl);
        if(ptl[1].x  > rectl.xRight) {/* ptl[1] is position of next char */ 
          /* Center the next line ?? */
          GpiQueryTextBox(hps, strlen(&chrURL[a]), &chrURL[a], TXTBOX_COUNT, aptl);
          if(aptl[TXTBOX_TOPRIGHT].x-aptl[TXTBOX_TOPLEFT].x <= rectl.xRight-rectl.xLeft)
            /* Yes */
            ptl[0].x=(rectl.xRight-rectl.xLeft-(aptl[TXTBOX_TOPRIGHT].x-aptl[TXTBOX_TOPLEFT].x))/2;
          else
            ptl[0].x=0; /* No */
          lSaveY-=aptl[TXTBOX_TOPLEFT].y+2;
          ptl[0].y=lSaveY;
          GpiMove(hps, ptl);
        }
        /* Draw char */
        GpiCharStringPos(hps,NULLHANDLE,CHS_UNDERSCORE,1,&chrURL[a],0);
        a++;
        //        GpiQueryCharStringPos(hps, 0, 1,  &chrURL[a], 0, ptl);
        //        a++;
      };
    } 
}

MRESULT EXPENTRY urlProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  PFNWP pfnwp;
  char chrURL[500];
  STARTDATA startData;
  char chrLoadError[CCHMAXPATH];
  ULONG ulSessionID;
  PID pid;
  ULONG attrValue[32];
  APIRET rc;
  HPS hps;
  RECTL rectl;

  switch( msg )
    {
    case WM_BUTTON1DOWN:
      hps=WinGetPS(hwnd);
      paintURL(hps, hwnd, CLR_RED);
      WinReleasePS(hps);
      return (MRESULT) FALSE;
    case WM_BUTTON1UP:
      hps=WinGetPS(hwnd);
      paintURL(hps, hwnd, CLR_BLUE);
      WinReleasePS(hps);
      return (MRESULT) FALSE;
    case WM_BUTTON1CLICK:
      WinQueryWindowText(hwnd,sizeof(chrURL),chrURL);
      memset(&startData,0,sizeof(startData));
      startData.Length=sizeof(startData);
      startData.Related=SSF_RELATED_INDEPENDENT;
      startData.FgBg=SSF_FGBG_FORE;
      startData.TraceOpt=SSF_TRACEOPT_NONE;
      startData.PgmTitle="";
      startData.PgmName="netscape.exe";
      startData.InheritOpt=SSF_INHERTOPT_SHELL;
      startData.SessionType=SSF_TYPE_DEFAULT;
      startData.PgmControl=SSF_CONTROL_VISIBLE;
      startData.InitXPos=30;
      startData.InitYPos=30;
      startData.InitXSize=500;
      startData.InitYSize=400;
      startData.ObjectBuffer=chrLoadError;
      startData.ObjectBuffLen=(ULONG)sizeof(chrLoadError);
      startData.PgmInputs=chrURL;
      rc=DosStartSession(&startData,&ulSessionID,&pid);					     
      return (MRESULT) FALSE;
    case WM_PAINT:
      hps=WinBeginPaint(hwnd, NULLHANDLE, NULLHANDLE);
      paintURL(hps, hwnd, CLR_BLUE);
      WinEndPaint(hps);
      return (MRESULT) FALSE;
    case WM_MOUSEMOVE:
      if(hptrHand) {
        WinSetPointer(HWND_DESKTOP,hptrHand);
      }
      return (MRESULT)FALSE;
    default:
      break;
    }
  pfnwp=(PFNWP)WinQueryWindowULong(hwnd,QWL_USER);
  if(pfnwp)
    return pfnwp( hwnd, msg, mp1, mp2 );

  return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}


/**************************************************************/
/*                                                            */

/*                                                            */
/**************************************************************/
MRESULT EXPENTRY aboutDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  HENUM hEnum;
  HWND hwndNext;
  char chrText[400];
  ULONG ulStyle;
  char text[50];
  PFNWP pfnwp;
  HMODULE hModule;
  SWP swp;

  switch( msg )
    {
    case WM_INITDLG:
      /* Find text controls with an URL */
      hEnum=WinBeginEnumWindows(hwnd);
      do {
        hwndNext=WinGetNextWindow(hEnum);
        if(hwndNext) {
          WinQueryClassName(hwndNext,sizeof(chrText), chrText);
          if(!strcmpi(chrText,"#5")) {
            ulStyle=WinQueryWindowULong(hwndNext,QWL_STYLE);
            
            if((ulStyle & 0xf) == SS_TEXT) {
              /* Static text control */
              WinQueryWindowText(hwndNext,sizeof(chrText), chrText);          
              if(!strnicmp(chrText,"http:",5)){
                if(!hptrHand) {
                  if(!DosQueryModuleHandle("PROGFLDR.DLL",&hModule))
                    hptrHand=WinLoadPointer(HWND_DESKTOP,hModule,IDPT_HAND);
                }
                pfnwp=WinSubclassWindow(hwndNext, urlProc);
                WinSetWindowULong(hwndNext,QWL_USER,(ULONG)pfnwp);
              }
            }/* end of if((ulStyle & 0xf) == SS_TEXT) */
          }
        }
      }while(hwndNext);
      WinEndEnumWindows(hEnum);
      /* Set dialog font to WarpSans for Warp 4 and above */
      if(cwQueryOSRelease()>=40) {
        WinSetPresParam(hwnd,
                        PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                        DEFAULT_DIALOG_FONT );
      }

      WinQueryWindowPos(hwnd, &swp);

      WinSetWindowPos(hwnd,HWND_TOP,(WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN)-swp.cx)/2, (WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN)-swp.cy)/2,
                      0,0, SWP_MOVE|SWP_ZORDER|SWP_ACTIVATE|SWP_SHOW);
      return (MRESULT) TRUE;
    case WM_DESTROY:
      break;
    case WM_CLOSE:
      break;
    case WM_COMMAND:
      {
    switch( SHORT1FROMMP( mp1 ) )
      {
      default:
        break;
      }
      }
      break;
      return (MRESULT) TRUE;
    }   
  return( WinDefDlgProc( hwnd, msg, mp1, mp2 ) );   
}

/**************************************************************/
/*                                                            */
/* This method shows an about dialog.                         */
/*                                                            */
/**************************************************************/
ULONG CWProgFolder::cwShowAboutDlg(HMODULE hModule, ULONG idDialog)
{
  HWND hwnd;

  /*
    New: 01.05.2004, MAke sure the about dialog is modal to the folder.
   */
  hwnd=WinQueryActiveWindow(HWND_DESKTOP);

  return (WinDlgBox( HWND_DESKTOP, hwnd, aboutDlgProc, hModule,idDialog, 0));
}

/**************************************************************/
/*                                                            */
/* This funktion returns the CD-Drives in the system          */
/*                                                            */
/* iNumCD (output): # of CD-Drives                            */
/* cFirstDrive (output): first cd-Drive letter                */
/* returns TRUE: We have cd-drives                            */
/*                                                            */
/**************************************************************/
BOOL CWProgFolder::cwQueryCDDrives(int *iNumCD, char * cFirstDrive)
{
  HFILE hfDevice;
  ULONG ulAction;
  ULONG ulLen;
  static char cFirst=0;
  static int iNumCDLocal=0;
  static BOOL haveCD=FALSE;
  static BOOL done=FALSE;         
  struct
  {
    USHORT usCountCD;
    USHORT usFirstCD;
  } CDInfo;

  if(!done){
    ulLen = sizeof(CDInfo);
    if(!DosOpen("\\DEV\\CD-ROM2$", &hfDevice, &ulAction, 0,
                FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, NULL))
      {
        if(!DosDevIOCtl(hfDevice, 0x82, 0x60, NULL, 0, NULL, &CDInfo, ulLen, &ulLen))
          {
            if(CDInfo.usCountCD) {
              haveCD=TRUE;
              iNumCDLocal=CDInfo.usCountCD;
              cFirst='A'+ CDInfo.usFirstCD;
            }                                        
          }
        DosClose(hfDevice);
      }
    done=TRUE;
  }
  *iNumCD=iNumCDLocal;
  *cFirstDrive=cFirst;
  return haveCD;              
}


/* Use this method to insert user menu items into a menu when WPSWizard is installed */
BOOL CWProgFolder::cwInsertUserMenuItems(HWND hwndMenu, char* chrMenuFolder, ULONG ulFirstId, BOOL bLeadingSeparator)
{
  M_WPObject *m_wpObject;
  CWMenuFolder * wpFolder;
  HOBJECT hObject;
  somId id;
  somTD_CWMenuFolder_mfInsertMenuItems methodPtr;
  ULONG ulRc=ulFirstId;

  /* Insert the menu items from the user menu folder. WPS-Wizard must be installed for
     this. */
  if((hObject=WinQueryObject(chrMenuFolder))!=NULLHANDLE) {//is there a default menufolder?    
    /* Yes */
    m_wpObject=(M_WPObject*)this->somGetClass();
    if(somIsObj(m_wpObject)) {
      /* We have it */
      /* Get method address */
      if((id=somIdFromString("mfInsertMenuItems"))!=NULLHANDLE) {
        wpFolder=(CWMenuFolder *)m_wpObject->wpclsQueryObject( hObject);
        if(somIsObj(wpFolder)) {
          methodPtr= (somTD_CWMenuFolder_mfInsertMenuItems) (wpFolder->somGetClass())->somFindSMethod(id);
          if(methodPtr) {
            SHORT sIndex;

            if(bLeadingSeparator)
              sIndex=SHORT1FROMMR(cwInsertMenuSeparator(MIT_END, hwndMenu, NULL));
            ulRc=methodPtr(wpFolder, hwndMenu , MIT_END, ulFirstId);
#if 0
            if(ulRc==ulFirstId) {
              /* No item inserted, remove the separator */
              sIndex=SHORT1FROMMR(WinSendMsg(hwndMenu,MM_ITEMIDFROMPOSITION,MPFROMSHORT(sIndex),
                                             (MPARAM)NULL));
              if(sIndex!=MIT_ERROR)            
                WinSendMsg(hwndMenu,MM_DELETEITEM,MPFROM2SHORT(sIndex, TRUE),
                           (MPARAM)NULL);                 
              
            }
#endif
          }
          /* ulCurrentID contains the last used menu id */
          wpFolder->wpUnlockObject();//Object is locked because of wpclsQueryHandle()
        }/* if(somIsObj(wpFolder)) */
        SOMFree(id);
      }/* somId */
    }/* end of if(somIsObj(m_wpObject))  */
  }/* USERMENU_FOLDER */
  return ulRc;
}

BOOL CWProgFolder::cwCheckUserMenuItems( char* chrMenuFolder, USHORT usItemId)
{
  CWMenuFolder * wpFolder;
  HOBJECT hObject;
  somId id;
  somTD_CWMenuFolder_mfCheckMenuItems methodPtr;
  M_WPObject *m_wpObject;
  BOOL bHandled=FALSE;

  if((hObject=WinQueryObject(chrMenuFolder))!=NULLHANDLE) {//is there a default menufolder?    
    /* Get class object */
    m_wpObject=(M_WPObject*)this->somGetClass();
    if(somIsObj(m_wpObject)) {
      /* We have it */
      if((id=somIdFromString("mfCheckMenuItems"))!=NULLHANDLE){
        wpFolder=(CWMenuFolder *)m_wpObject->wpclsQueryObject( hObject);
        if(somIsObj(wpFolder)) {
          methodPtr= (somTD_CWMenuFolder_mfCheckMenuItems) (wpFolder->somGetClass())->somFindSMethod(id);
          SOMFree(id);
          if(methodPtr) {
            /* Check for Items */
            bHandled=methodPtr(wpFolder, this, usItemId);
          }
          wpFolder->wpUnlockObject();//Object is locked because of wpclsQueryHandle()
        }
      }/* somId */
    }/* end of if(somIsObj(m_wpObject))  */
  }
  return bHandled;
}
