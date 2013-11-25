/*
 * This file is (C) Chris Wohlgemuth 2001
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
#define INCL_GPILCIDS
#define INCL_GPIPRIMITIVES
#define INCL_GPIBITMAPS


#include "audiofolder.hh"
#include "launchpad.hh"

#include <os2.h>
#include "stdio.h"
#include "stdlib.h"

extern HWND hwndShadow;// The handle of the help window
extern PFNWP  oldProc;  //place for original button-procedure
extern HWND hwndBubbleWindow;// The handle of the help window
#define  WM_NEWBUBBLE   WM_USER+100 //Use ATOM later
#define xVal  12      //x-distance of Bubble
#define yVal  8      //y-distance of Bubble
extern char chrTBFlyFontName[CCHMAXPATH];/* Font for toolbar fly over help */
extern RGB rgbTBFlyForeground;
extern RGB rgbTBFlyBackground;
extern BOOL bTBFlyOverEnabled;
extern int iTBFlyOverDelay;


#define xButtonDelta 2
#define xButtonOffset 6

BOOL checkFileExists(char* chrFileName);

PFNWP pfnwpOldLPProc;
LPList* lplAllLPads=NULL;

extern PFNWP  oldButtonProc2;  //place for original button-procedure

MRESULT EXPENTRY newButtonProc(HWND hwnd, ULONG msg,MPARAM mp1,MPARAM mp2 );
MRESULT EXPENTRY shadowProc(HWND hwnd, ULONG msg,MPARAM mp1,MPARAM mp2 );

/******************************************************************************/
/*  New button procedure which handles the context menu stuff for the objects */
/* The fly over help is managed by the standard button replacement proc.      */
/******************************************************************************/
static MRESULT EXPENTRY newLaunchPadButtonProc(HWND hwnd, ULONG msg,MPARAM mp1,MPARAM mp2 )
{
  POINTL ptl;
  HWND hwndMenu;
  WPObject * wpObject;
  LPObject* lpo;
  PDRAGINFO pDragInfo;
  static BOOL bDrawn=FALSE;
  static DRAGITEM DragItem;
  static DRAGIMAGE DragImage;
  static DRAGINFO *DragInfo;

  switch (msg)
    {
#if 0
    case WM_BEGINDRAG:
      if(SHORT1FROMMP(mp2)||1) {
        //  DosBeep(5000,130);
        lpo=(LPObject*)WinQueryWindowULong(hwnd,QWL_USER);
        if(lpo) {
          wpObject=lpo->wpObject;
          if(somIsObj(wpObject)) {
#if 0
            DragItem.hwndItem=hwnd;
            DragItem.ulItemID=(ULONG)wpObject;
            DragItem.hstrType=DrgAddStrHandle("DRT_UNKNOWN");
            DragItem.hstrRMF=DrgAddStrHandle("DRM_OBJECT");
            DragItem.hstrContainerName=DrgAddStrHandle("E:\\CD-Brenner\\ADC_050\\!AUDIOCD_TOOLBAR!");
            DragItem.hstrSourceName=DrgAddStrHandle("E:\\CD-Brenner\\ADC_050\\");
#endif
            wpObject->wpFormatDragItem(&DragItem);
            //            DragItem.fsSupportedOps=DO_LINKABLE;
            DragInfo=DrgAllocDraginfo(1);
           
            DrgSetDragitem(DragInfo, &DragItem,(ULONG)sizeof(DragItem),0);
            memset(&DragImage, 0, sizeof(DragImage));
            DragImage.cb=sizeof(DragImage);
            DragImage.fl=DRG_ICON;
            DragImage.hImage=wpObject->wpQueryIcon();
            

            if(DrgDrag(hwnd, DragInfo, &DragImage, 1, VK_ENDDRAG, NULL)) {
              lpo->lpParent->lpRefreshLaunchPad();
            }

            /* Free DragInfo here!!!!!!!!!!! */
            //DrgDeleteDraginfoStrHandles(pDragInfo);
            // ????? DrgFreeDraginfo(DragInfo);
            return (MRESULT)TRUE;
          }/* somIsObj() */
        }/* lpo */
      }
      break;

    case DM_ENDCONVERSATION:
    case DM_DRAGFILECOMPLETE:
    case DM_DROPNOTIFY:
    case DM_FILERENDERED:
    case DM_RENDERCOMPLETE:
      DosBeep(500,500);
break;
      return (MRESULT)FALSE;
    case WM_ENDDRAG:
      DosBeep(500,500);
      return (MRESULT)TRUE;
      break;
#endif
    case WM_PAINT:
      lpo=(LPObject*)WinQueryWindowULong(hwnd,QWL_USER);
      if(lpo) {
        BTNCDATA btCtrl={0};
        WNDPARAMS wndParams={0};

        // memset(&btCtrl,0,sizeof(btCtrl));
        btCtrl.cb=sizeof(btCtrl);
        if(somIsObj(lpo->wpObject)) {
          btCtrl.hImage=lpo->wpObject->wpQueryIcon();
          // memset(&wndParams,0,sizeof(wndParams));
          wndParams.fsStatus=WPM_CTLDATA;
          wndParams.cbCtlData=btCtrl.cb;
          wndParams.pCtlData=&btCtrl;
          WinSendMsg(hwnd,WM_SETWINDOWPARAMS,
                     MPFROMP(&wndParams),0);
        }
      }
      break;
    case WM_CONTEXTMENU:
      if(!SHORT2FROMMP(mp2)) {

        lpo=(LPObject*)WinQueryWindowULong(hwnd,QWL_USER);
        if(lpo) {
          wpObject=lpo->wpObject;
          if(somIsObj(wpObject)) {
            ptl.y=0;
            ptl.x=0;
            hwndMenu=wpObject->wpDisplayMenu(hwnd,hwnd,&ptl,MENU_OBJECTPOPUP,0);
          }
          return (MRESULT) TRUE;
        }

      }/* end of if(!SHORT2FROMMP(mp2)) */
      break;
    case DM_DRAGOVER:
      {
        BOOL bReturn=FALSE;
        
        TRY_LOUD(LP_DRAGOVER) {
        lpo=(LPObject*)WinQueryWindowULong(hwnd,QWL_USER);
        if(lpo) {
          wpObject=lpo->wpObject;
          if(somIsObj(wpObject)) {
            char chrClassName[10];
            LONG lLength;
            
            pDragInfo=(PDRAGINFO) mp1;
            hwndMenu=pDragInfo->hwndSource;
            lLength=sizeof(chrClassName);
            WinQueryClassName(hwndMenu, lLength, chrClassName);
            if(strcmp(chrClassName,"#37"))
              hwndMenu=NULL;/* Source isn't a container */
            if(!bDrawn) {
              HPS hps;
              RECTL rectl;
              SWP swp;

              /* Draw the rect */
              hps=DrgGetPS(hwnd);
              WinQueryWindowPos(hwnd,&swp);
              rectl.xLeft=1;
              rectl.yBottom=1;
              rectl.xRight=swp.cx-1;
              rectl.yTop=swp.cy-1;
              //WinDrawBorder(hps,&rectl, 3, 3, 0, 0, DB_STANDARD|DB_PATINVERT);  
              DrgReleasePS(hps);
              //bDrawn=TRUE;
            }
            bReturn=TRUE;
          }
        }
        }
        CATCH(LP_DRAGOVER)
          {
          } END_CATCH;
          if(bReturn)
            return wpObject->wpDragOver(hwndMenu,(PDRAGINFO) mp1);
          break;
      }
    case DM_DRAGLEAVE:
      {
        RECTL rectl;
        SWP swp;
        HPS hps;
        
        /* Remove target emphasis here */
        hps=DrgGetPS(hwnd);
        WinQueryWindowPos(hwnd,&swp);
        rectl.xLeft=1;
        rectl.yBottom=1;
        rectl.xRight=swp.cx-1;
        rectl.yTop=swp.cy-1;
        // WinDrawBorder(hps,&rectl,3, 3, 0, 0, DB_STANDARD|DB_PATINVERT);  
        bDrawn=FALSE;
        DrgReleasePS(hps);
        return (MRESULT) FALSE;
      }
    case DM_DROP:
      {
        BOOL bReturn=FALSE;
        char chrClassName[10];
        LONG lLength;
        PDRAGITEM pDragItem;

        TRY_LOUD(LP_DROP) {
          /* A drop on a toolbar button */
          lpo=(LPObject*)WinQueryWindowULong(hwnd,QWL_USER);
          if(lpo) {
            wpObject=lpo->wpObject;
            if(somIsObj(wpObject)) { 
              pDragInfo=(PDRAGINFO) mp1;
              pDragItem=DrgQueryDragitemPtr(pDragInfo, 0);
              hwndMenu=pDragInfo->hwndSource;
              lLength=sizeof(chrClassName);
              WinQueryClassName(hwndMenu, lLength, chrClassName);
              if(strcmp(chrClassName,"#37"))
                hwndMenu=NULL;/* Source isn't a container */
              bReturn=TRUE;
            }
          }
        }
        CATCH(LP_DROP)
          {
          } END_CATCH;
          if(bReturn)
            return wpObject->wpDrop(hwndMenu,(PDRAGINFO) mp1, pDragItem);
          break;
      }
    case WM_COMMAND:
      TRY_LOUD(LP_COMMAND) {
      lpo=(LPObject*)WinQueryWindowULong(hwnd,QWL_USER);
      if(lpo) {
        wpObject=lpo->wpObject;
        if(somIsObj(wpObject)) {
          if((SHORT)0x6d==SHORT1FROMMP(mp1)) {
            /* Delete menu item */       
            if(wpObject->wpDelete(CONFIRM_DELETE|CONFIRM_DELETEFOLDER ) == OK_DELETE)
              lpo->lpParent->lpRemoveButton(lpo);
          }
          else
            wpObject->wpMenuItemSelected(NULL, SHORT1FROMMP(mp1));
        }/* end of if(somIsObj(wpObject)) */
      }
      }  
      CATCH(LP_COMMAND)
        {
        } END_CATCH;
        return (MRESULT) FALSE;
    default:
      break;			
    }
  // call the standard flyover button procedure to handle the rest of the messages
  return (newButtonProc)(hwnd,msg,mp1,mp2);	
};

static BOOL bDrawn=FALSE;
static RECTL rclDrawn;
static MRESULT handleDragOver(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
  MRESULT mResult;
  ULONG ulNumberOfObjects;
  PDRAGINFO pDragInfo;
  HPS hps;
  RECTL rectl;
  SWP swp;

  mResult = MRFROM2SHORT( DOR_NODROPOP, DO_LINK);
  
  /* Get access to the dragInfo */
  pDragInfo=(PDRAGINFO)mp1;
  DrgAccessDraginfo(pDragInfo);
  /* Get number of items */
  ulNumberOfObjects = DrgQueryDragitemCount( pDragInfo);

  if(ulNumberOfObjects>1) {
    /* Free the draginfo */
    DrgFreeDraginfo(pDragInfo);
    return MRFROM2SHORT( DOR_NODROPOP, DO_LINK);
  }

  switch(pDragInfo->usOperation)
    {
    case DO_DEFAULT:
    case DO_LINK:
      if(!bDrawn) {
        POINTL ptl;
        launchPad * lp;
        
        ptl.x=SHORT1FROMMP(mp2);
        ptl.y=SHORT2FROMMP(mp2);
        /* Pos in window coords */
        WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1);

        hps=DrgGetPS(hwnd);
        WinQueryWindowPos(hwnd,&swp);

        rectl.xLeft=swp.cx;//-swp.cy;

        lp=(launchPad*)WinQueryWindowULong(hwnd,QWL_USER);
        if(lp) {
          if(lp->lpQueryNumObjects()==0) {
            rectl.xLeft=0;
          }
        }
        
        rectl.yTop=swp.cy-1;    
        rectl.yBottom=1;
        if(ptl.x<rectl.xLeft) {
          int numB;

          numB=(ptl.x-xButtonOffset)/(swp.cy+xButtonDelta);
          rectl.xLeft=((ptl.x-numB*swp.cy) > swp.cy/2 ? numB+1: numB);
          rectl.xLeft*=(swp.cy+xButtonDelta);
          rectl.xLeft-=(xButtonDelta)-xButtonOffset+xButtonDelta;
          rectl.xRight=rectl.xLeft+(xButtonDelta*2);
        }
        else {
          rectl.xRight=swp.cx-1;
        }
        rclDrawn=rectl;
        WinDrawBorder(hps,&rectl, 2, 2, 0, 0, DB_STANDARD|DB_PATINVERT);  
        bDrawn=TRUE; 
        
        DrgReleasePS(hps);
      }
      mResult = MRFROM2SHORT( DOR_DROP, DO_LINK);
      break;
    default:
      break;
    }

  /* Free the draginfo */
  DrgFreeDraginfo(pDragInfo);

  return mResult;
}


static MRESULT handleDragLeave(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
  RECTL rectl;
  SWP swp;
  HPS hps;

  /* Remove target emphasis here */
  hps=DrgGetPS(hwnd);
  WinDrawBorder(hps,&rclDrawn, 2, 2, 0, 0, DB_STANDARD|DB_PATINVERT);  
  DrgReleasePS(hps);
  bDrawn=FALSE; 
  return (MRESULT) FALSE;
}


static MRESULT EXPENTRY launchPadWindowProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  WPFolder* thisPtr;
  launchPad * lp;
  LPObject *lpo;
  static USHORT id=0;//Initialisation new in V1.00a 

  switch(msg)
    {
#if 0
    case DM_ENDCONVERSATION:
    case DM_DRAGFILECOMPLETE:
    case DM_DROPNOTIFY:
    case DM_FILERENDERED:
    case DM_RENDERCOMPLETE:
      DosBeep(500,500);
break;
      return (MRESULT)FALSE;
    case WM_ENDDRAG:
      DosBeep(500,500);
      break;
      return (MRESULT)TRUE;
#endif
    case WM_PAINT:
      {
      RECTL rcl;
      launchPad * lp;

      HPS hps=WinBeginPaint(hwnd,NULLHANDLE, &rcl);
      WinFillRect(hps, &rcl, SYSCLR_DIALOGBACKGROUND);
      lp=(launchPad*)WinQueryWindowULong(hwnd,QWL_USER);
      if(lp) {
        if(lp->lpQueryNumObjects()==0) {
          WinQueryWindowRect(hwnd,&rcl);
          WinDrawBorder(hps,&rcl,1,1,SYSCLR_WINDOWFRAME,SYSCLR_DIALOGBACKGROUND, DB_STANDARD);
        }
      }
      WinEndPaint(hps);
      return (MRESULT) 0;
      }
    case DM_DRAGOVER:
      return handleDragOver(hwnd, mp1, mp2);
    case DM_DROP:
      {
        ULONG ulCount;
        ULONG ulNumberOfObjects;
        PDRAGITEM pDragItem;
        SOMClass *folderClass;
        WPObject * wpObject;
        PDRAGINFO pDragInfo;
       
        TRY_LOUD(LP_FRAMEDROP) { 
          /* A new object dropped on the launchpad */
          pDragInfo=(PDRAGINFO)mp1;
          if(DrgAccessDraginfo(pDragInfo)) {
            /* Get number of items */
            ulNumberOfObjects = DrgQueryDragitemCount( pDragInfo);           
            if(ulNumberOfObjects>1){
              /* Free the draginfo */
              DrgDeleteDraginfoStrHandles(pDragInfo);
              DrgFreeDraginfo(pDragInfo);
            }
            else { 
              ulCount=0;
              
              pDragItem=DrgQueryDragitemPtr( pDragInfo, ulCount);
              wpObject=(WPObject*)OBJECT_FROM_PREC(DrgQueryDragitemPtr( pDragInfo, ulCount)->ulItemID);
              lp=(launchPad*)WinQueryWindowULong(hwnd,QWL_USER);
              if(lp) {
                if(somIsObj(wpObject)) {
  
                  POINTL ptl;
                  int numB;
                  SWP swp;
                  
                  WinQueryWindowPos(hwnd,&swp);
                  ptl.x=pDragInfo->xDrop;
                  ptl.y=pDragInfo->yDrop;
                  /* Pos in window coords */
                  WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1);
                  numB=(ptl.x-xButtonOffset)/(swp.cy+xButtonDelta);
                  numB=((ptl.x-numB*swp.cy) > swp.cy/2 ? numB+1: numB);

                  /* Do a link */
                  lp->lpAddButton(wpObject, numB);
                  handleDragLeave(hwnd, mp1, mp2);
                }
              }
              DrgDeleteDraginfoStrHandles(pDragInfo);
              DrgFreeDraginfo(pDragInfo);
            }
          }
          handleDragLeave(hwnd, mp1, mp2);
        }
        CATCH(LP_FRAMEDROP)
          {
          } END_CATCH;
          
          break;
      }
    case DM_DRAGLEAVE:
      return handleDragLeave(hwnd, mp1, mp2);
      break;
    case WM_COMMAND:
      if(SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON) {
        /* It's a push button */
        lpo=(LPObject*)WinQueryWindowULong(WinWindowFromID(hwnd,SHORT1FROMMP(mp1)),QWL_USER);
        if(lpo) {
          if(somIsObj(lpo->wpObject))
            lpo->wpObject->wpViewObject(NULLHANDLE, OPEN_DEFAULT,0);
        }
      }
      return (MRESULT)FALSE;
      /***********************************************/
      /* Stuff for fly over help                     */ 
    case WM_MOUSEMOVE:
      launchPad * lp;

      lp=(launchPad*)WinQueryWindowULong(hwnd,QWL_USER);
      if(lp) {
        if(lp->lpQueryNumObjects()!=0) {
          break;
        }
      }
      
#if 0
      tempID=WinQueryWindowUShort(hwnd,QWS_ID);/*  get the id of the window under the pointer  */  			
      if(id!=tempID) {	// New Button?	
        WinStartTimer(WinQueryAnchorBlock(hwnd),hwnd,tempID,(ULONG)iTBFlyOverDelay); // New timer for delay
        id=tempID;  // Save ID 
      }
      else {
        if(!hwndBubbleWindow)
          WinStartTimer(WinQueryAnchorBlock(hwnd),hwnd,tempID,(ULONG)iTBFlyOverDelay); // New timer for delay	
      }
#endif
      if(!hwndBubbleWindow)
        WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, 2,(ULONG)iTBFlyOverDelay); // New timer for delay	

      break;
    case WM_DESTROY:
      WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,1);//Stop timer if running
      if(hwndBubbleWindow)
        WinDestroyWindow(hwndBubbleWindow);/*  close the bubblewindow  */
      hwndBubbleWindow=0;
      /* Stop delay timer if running */
      WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd, 2);			
      break;

    case WM_NEWBUBBLE:
      ULONG bubbleEnabled;
      HWND hwndStore;
      POINTL ptl;
      RECTL  rclWork;
      LONG  ulWinTextLen;
      POINTL aptlPoints[TXTBOX_COUNT];
      LONG   deltaX,deltaY;
      HPS  hps;
      RECTL   rcl;
  
      /*  we have to build a new information window  */
      if(hwndBubbleWindow){// if(){...} new in V1.00a 
        WinDestroyWindow(hwndBubbleWindow);/*  close the bubblewindow  */
        hwndBubbleWindow=NULL;
      }
      // Query the pointer position
      WinQueryPointerPos(HWND_DESKTOP,&ptl);
      WinMapWindowPoints(HWND_DESKTOP,hwnd,&ptl,1);
      WinQueryWindowRect(hwnd,&rclWork);				
      if(!hwndBubbleWindow 
         && WinPtInRect(WinQueryAnchorBlock(hwnd),&rclWork,&ptl)
         && bTBFlyOverEnabled) {
        
        static HWND hwndBubbleClient;
        ULONG style=FCF_BORDER|FCF_NOBYTEALIGN;
        char winText[255];
        
        /* Get window text for size calculating */
            lp=(launchPad*)WinQueryWindowULong(hwnd,QWL_USER);
        if(lp) {
          strncpy(winText, lp->lpQueryFlyOverText(), sizeof(winText));
          winText[sizeof(winText)-1]=0;
        }

        ulWinTextLen=(LONG)strlen(winText); // Query text length
        
        /* Delete 'Returns' in object title */
        char *pBuchst;
        char *pRest;
        pRest=winText;
        while((pBuchst=strchr(pRest,13))!=NULL || (pBuchst=strchr(pRest,0x0a))!=NULL) {
          *pBuchst=' ';
          pBuchst++;
          if(*pBuchst==10)
            *pBuchst=' ';
          pRest=pBuchst;
        }
        
        /* Create help window */
        hwndBubbleWindow=WinCreateStdWindow(HWND_DESKTOP,
                                            0,
                                            &style,
                                            WC_STATIC,
                                            "",
                                            SS_TEXT|DT_CENTER|DT_VCENTER,
                                            NULLHANDLE,
                                            400,
                                            &hwndBubbleClient);

        hwndShadow=WinCreateWindow(HWND_DESKTOP,
                                   WC_STATIC,
                                   "",
                                   SS_TEXT|DT_CENTER|DT_VCENTER,
                                   0, 0,
                                   0, 0,
                                   hwndBubbleWindow,
                                   hwndBubbleWindow,
                                   401,
                                   NULLHANDLE,
                                   NULLHANDLE);
        oldProc=WinSubclassWindow(hwndShadow, shadowProc);

        // Set the font for the help
        WinSetPresParam(hwndBubbleClient,PP_FONTNAMESIZE,
                        sizeof(chrTBFlyFontName),
                        chrTBFlyFontName);
        /* Calculate text size in pixel */
        hps=WinBeginPaint(hwndBubbleClient,(HPS)NULL,(PRECTL)NULL);
        GpiQueryTextBox(hps,ulWinTextLen,winText,TXTBOX_COUNT,aptlPoints);
        WinEndPaint(hps);
        
        /* Set colors */
        WinSetPresParam(hwndBubbleClient,
                        PP_BACKGROUNDCOLOR,sizeof(rgbTBFlyBackground) ,
                        &rgbTBFlyBackground );
        WinSetPresParam(hwndBubbleClient,
                        PP_FOREGROUNDCOLOR,sizeof(rgbTBFlyForeground) ,
                        &rgbTBFlyForeground );
        
        /* Calculate bubble positon and show bubble */
        WinQueryPointerPos(HWND_DESKTOP,&ptl);//Query pointer position in the desktop window
        WinQueryWindowRect(HWND_DESKTOP,&rcl);//Query desktop size
        aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x+7+xVal+ptl.x 
          > rcl.xRight 
          ? deltaX=-aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x-xVal-xVal-7 
	      : deltaX=0 ;
        
        aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y+2+yVal+ptl.y 
          > rcl.yTop 
          ? deltaY=-aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y-2*yVal-7
	      : deltaY=0 ;		
        WinSetWindowPos(hwndBubbleWindow,
                        HWND_TOP,
                        ptl.x+xVal+deltaX,ptl.y+yVal+deltaY,  
                        aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x+8,
                        aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y+2,
                        SWP_ZORDER|SWP_SIZE|SWP_MOVE|SWP_SHOW);

        WinSetWindowPos(hwndShadow,
                        hwndBubbleWindow,
                        ptl.x+xVal+deltaX+5
                        ,ptl.y+yVal+deltaY-5,  
                        aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x+8,
                        aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y+2,
                        SWP_ZORDER|SWP_SIZE|SWP_MOVE|SWP_SHOW);
        
        /* Set bubble text */
        WinSetWindowText(hwndBubbleClient,winText);
        WinStartTimer(WinQueryAnchorBlock(hwnd),hwnd,1,35); 
      } // end if(!hwndBubbleWindow)
      break;
    case WM_TIMER:
      switch (SHORT1FROMMP(mp1))
        {
        case 1: //Intervall timer
          {
            POINTL ptl;
            RECTL  rclWork;

            /* Test pointer position */
            WinQueryPointerPos(HWND_DESKTOP, &ptl);
            WinMapWindowPoints(HWND_DESKTOP, hwnd,&ptl, 1);
            WinQueryWindowRect(hwnd, &rclWork);
            if(!WinPtInRect(WinQueryAnchorBlock(hwnd),&rclWork,&ptl))
              {	// Window has changed				 
                WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, 1);  // stop the running timer
                if(hwndBubbleWindow) 
                  WinDestroyWindow(hwndBubbleWindow);/*  close the bubblewindow  */
                hwndBubbleWindow=0;
                id=0;
              }			 			
          break;
          }
        case 2:// delay over
          {//our own timer.
            POINTL ptl;
            RECTL  rclWork;

            WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, 2);//Stop the delay timer
            /* Check the pointer position */
            WinQueryPointerPos(HWND_DESKTOP,&ptl);
            WinMapWindowPoints(HWND_DESKTOP,hwnd,&ptl,1);
            WinQueryWindowRect(hwnd,&rclWork);
            if(WinPtInRect(WinQueryAnchorBlock(hwnd),&rclWork,&ptl))
              WinPostMsg(hwnd,WM_NEWBUBBLE,NULL,NULL);//Request a help window
            return (MRESULT)FALSE;
          }
        default:
          break;
        }
      break;
    default:
      break;    
    }
  return pfnwpOldLPProc(hwnd, msg, mp1, mp2);
}

void LPList::lplRefreshAllPads()
{
  
}

LPObject::LPObject(HPOINTER hptr)
{
  hPtr=hptr;
}

LPObject::~LPObject(void)
{
#if 0
  if(hPtr) {
    if(!WinDestroyPointer(hPtr))
      DosBeep(100,200);
  }
#endif
}

launchPad::launchPad(HWND hwndParent,HWND hwndOwner, BOOL bVisible, WPFolder *wpParent, LPList** lpadList, HWND hwndPrev, ULONG ulFl)
{
  LPList *lplTemp;

  wpParentFolder=wpParent;
  hwndPrevious=hwndPrev;
  fl=ulFl;
  lpList=lpadList;

  /* Reset text */
  chrFlyOverText[0]=0;

  if((hwndLaunchPad=WinCreateWindow(hwndParent ,WC_FRAME, "Toolbar", FS_BORDER| (bVisible ? WS_VISIBLE: 0),
                                    0, 0, 0, 0, hwndOwner, HWND_TOP, 1234, NULLHANDLE, NULLHANDLE))!=NULLHANDLE) {
    WinSetWindowULong(hwndLaunchPad, QWL_USER,(ULONG) this);//Save object ptr.	
    pfnwpOldLPProc=WinSubclassWindow(hwndLaunchPad, launchPadWindowProc);
  }

  ulNumObjects=0;
  lpoObjectList=NULL;

  /* Add this pad to the list of pads */
  if(!lplAllLPads) {
    /* It's the first one */
    if((lplAllLPads=new LPList)!=NULL) {
      lplAllLPads->lpPad=this;
      lplAllLPads->lplNext=NULL;
    }
  }
  else {
    if((lplTemp=new LPList)!=NULL) {
      lplTemp->lpPad=this;
      lplTemp->lplNext=lplAllLPads;
      lplAllLPads=lplTemp;
    }
  }
}

launchPad::~launchPad()
{
  LPList *lplTemp, *lplTemp2;

  lpSaveObjectList();
  lpFreeObjectList();

  if(hwndLaunchPad)
    WinDestroyWindow(hwndLaunchPad);
    
  /* Remove this pad from the list of pads */
  lplTemp=lplAllLPads;

  if(lplTemp->lpPad==this) {
    lplAllLPads=lplTemp->lplNext;
    delete(lplTemp);
  }
  else {
    while(lplTemp->lplNext)
      {
        if(lplTemp->lplNext->lpPad==this) {
          lplTemp2=lplTemp->lplNext;
          lplTemp->lplNext=lplTemp->lplNext->lplNext;
          delete(lplTemp2);
          break;
        }
        lplTemp=lplTemp->lplNext;
      }
  }
}

BOOL launchPad::lpSetFlyOverText(char* chrText)
{
  strncpy(chrFlyOverText, chrText, sizeof(chrFlyOverText));
  chrFlyOverText[sizeof(chrFlyOverText)-1]=0;

  return TRUE;
}

PSZ  launchPad::lpQueryFlyOverText(void)
{
  return chrFlyOverText;
}

BOOL launchPad::lpSetLaunchPadPos(HWND hwndInsertBehind, LONG x, LONG y, LONG cx, LONG cy, ULONG fl)
{
  if(WinIsWindow(WinQueryAnchorBlock(hwndLaunchPad), hwndPrevious)) {
    SWP swp;
    WinQueryWindowPos(hwndPrevious,&swp);
    return WinSetWindowPos(hwndLaunchPad, hwndInsertBehind, swp.x+swp.cx+x, y, cx, cy, fl);
  }
  else
    return WinSetWindowPos(hwndLaunchPad, hwndInsertBehind, x, y, cx, cy, fl);
}


BOOL launchPad::lpSetConfiguration(char * chrTarget, char * chrConfigID_)
{
  HOBJECT hObject;
  char chrSetup[CCHMAXPATH];

  /* Destination of toolbar folder */
  strncpy(chrConfigTarget, chrTarget,sizeof(chrConfigTarget));
  chrConfigTarget[sizeof(chrConfigTarget)-1]=0;
  /* The ID of the toolbar folder */
  strncpy(chrConfigID, chrConfigID_,sizeof(chrConfigID));
  chrConfigID[sizeof(chrConfigID)-1]=0;

  /* First check the config folder */
  if((hObject=WinQueryObject(chrConfigID))==NULLHANDLE)
    {
      /* Toolbar folder lost recreate it */
      if(!checkFileExists(chrConfigTarget))
        return FALSE; /* No install dir defined */

      if(fl & LP_USEOBJECTASPARAM) {
        /* This isn't used yet */
        sprintf(chrSetup, "HIDECLASSPAGE=1;OBJECTID=%s",chrConfigID);
        if((hObject=WinCreateObject("CWMenuFolder", chrConfigID, chrSetup, chrConfigTarget,CO_FAILIFEXISTS))==NULLHANDLE)
          return FALSE; /* Can't create new toolbar folder */
      }
      else {
        sprintf(chrSetup, "OBJECTID=%s", chrConfigID);
        if((hObject=WinCreateObject("WPFolder", chrConfigID, chrSetup, chrConfigTarget,CO_FAILIFEXISTS))==NULLHANDLE)
          return FALSE; /* Can't create new toolbar folder */
      }
    }
  
  lpBuildObjectList(chrConfigID);

  return TRUE;
}

WPFolder* launchPad::lpQueryParentFolder()
{
  return wpParentFolder;
}

void launchPad::lpRefreshLaunchPad()
{
  /* Refresh all launchpads */
  LPList *lplTemp;
  
  //lplTemp=lplAllLPads;
  lplTemp=*lpList;

  while(lplTemp)
    {
      if(lplTemp->lpPad) {
        lplTemp->lpPad->lpFreeObjectList();
        lplTemp->lpPad->lpReBuildObjectList();
        lplTemp->lpPad->lpFillPad();
      }
      lplTemp=lplTemp->lplNext;
    }
  return;
}

BOOL launchPad::lpRemoveButton(LPObject *lpObject)
{
  int a;
  LPObject* lpoTemp, *lpoTemp2;

  /* Find object in list */
  lpoTemp=lpoObjectList;

  if(lpoTemp==lpObject) {
    lpoObjectList=lpoTemp->lpoNext;

    delete(lpObject);
    ulNumObjects--;
  }
  else {
    while(lpoTemp->lpoNext)
      {
        if(lpoTemp->lpoNext==lpObject) {
          lpoTemp->lpoNext=lpObject->lpoNext;

          delete(lpObject);
          ulNumObjects--;
          break;
        }
        lpoTemp=lpoTemp->lpoNext;
      }
  }
  
  lpSaveObjectList();   
  lpRefreshLaunchPad();

  return TRUE;
}


BOOL launchPad::lpAddButton(WPObject* wpObject, int iPos)
{
  HOBJECT hObject;
  SOMClass *folderClass;
  char chrSetup[CCHMAXPATH];
  WPObject *wpTempObject;
  LPObject *lpoTemp, *lpoTemp2, *lpoTempNew;
  int a;


  /* First check the config folder */
  if((hObject=WinQueryObject(chrConfigID))==NULLHANDLE)
    {
      /* Toolbar folder lost recreate it */
      if(!checkFileExists(chrConfigTarget))
        return FALSE; /* No install dir defined */

      if(fl & LP_USEOBJECTASPARAM) {
        sprintf(chrSetup, "HIDECLASSPAGE=1;OBJECTID=%s", chrConfigID);
        if((hObject=WinCreateObject("CWMenuFolder", chrConfigID, chrSetup, chrConfigTarget,CO_FAILIFEXISTS))==NULLHANDLE)
          return FALSE; /* Can't create new toolbar folder */
      }
      else {
        sprintf(chrSetup,  "OBJECTID=%s", chrConfigID);
        if((hObject=WinCreateObject("WPFolder", chrConfigID, chrSetup, chrConfigTarget,CO_FAILIFEXISTS))==NULLHANDLE)
          return FALSE; /* Can't create new toolbar folder */
      }
    }

  if(somIsObj(wpObject) && somIsObj(wpParentFolder)) {
    folderClass=wpParentFolder->somGetClass();
    if(somIsObj(folderClass)) {
      /* Create a shadow in our toolbar folder */
      wpTempObject=wpObject->wpCreateShadowObject((WPFolder*)((M_WPFolder*)folderClass)->wpclsQueryFolder(chrConfigID, FALSE), FALSE);
      if(somIsObj(wpTempObject)) {
        /* Get the persistent object handle */
        hObject=wpTempObject->wpQueryHandle();

        /* Find the insertion point */
        /* iPos is the button after which we insert the new one. The list of objects is in reverse order. */
        lpoTemp2=lpoObjectList;
        if(iPos<ulNumObjects) {
          lpoTemp=lpoTemp2;
          for(a=ulNumObjects;a>iPos && lpoTemp; a--)
            {
              lpoTemp=lpoTemp2;
              lpoTemp2=lpoTemp2->lpoNext;
            }
        }
        else
          lpoTemp=NULLHANDLE;

        /* lpoTemp contains object */
        /* Alloc a struct for this object to insert it on the toolbar */

        if((lpoTempNew=new LPObject(wpTempObject->wpQueryIcon()))!=NULL) {
          ULONG ulRC;
          
          lpoTempNew->wpObject=wpTempObject;
          lpoTempNew->wpObject->wpLockObject();
          lpoTempNew->hObject=hObject;
          //  lpoTempNew->hPtr=wpTempObject->wpQueryIcon();

          /* Title of the object */
          strncpy(lpoTempNew->chrName, wpTempObject->wpQueryTitle(),sizeof(lpoTempNew->chrName));
          lpoTempNew->lpParent=this;

          if(lpoTemp) {
            lpoTempNew->lpoNext=lpoTemp->lpoNext;
            lpoTemp->lpoNext=lpoTempNew;
          }
          else {
            lpoTempNew->lpoNext=lpoObjectList;
            lpoObjectList=lpoTempNew;
          }
          ulNumObjects++;
        }
      }
    }
  }

  lpSaveObjectList();
  lpRefreshLaunchPad();

  return TRUE;
}


BOOL launchPad::lpSaveObjectList()
{
  SOMClass *folderClass;
  WPFolder *wpFolder;
  char chrPath[CCHMAXPATH];
  ULONG ulBufferSize;
  HINI hIni;
  char * memPtr;
  HOBJECT *hObject;
  HOBJECT hObject2;
  int a;
  LPObject *lpoTemp;

  /* First check the config folder */
  if((hObject2=WinQueryObject(chrConfigID))==NULLHANDLE)
    {
      /* Toolbar folder lost recreate it */
      if(!checkFileExists(chrConfigTarget))
        return FALSE; /* No install dir defined */
      
      sprintf(chrPath,"OBJECTID=%s",chrConfigID);
      if((hObject2=WinCreateObject("WPFolder", chrConfigID, chrPath,chrConfigTarget,CO_FAILIFEXISTS))==NULLHANDLE)
        return FALSE; /* Can't create new toolbar folder */
    }
  
  /* Get toolbar folder */
  if(somIsObj(wpParentFolder)) {
    folderClass=wpParentFolder->somGetClass();
    if(somIsObj(folderClass))
      wpFolder=(WPFolder*)((M_WPFolder*)folderClass)->wpclsQueryFolder(chrConfigID, FALSE);
  }

  ulBufferSize=sizeof(chrPath);
  wpFolder->wpQueryRealName(chrPath, &ulBufferSize, TRUE);
  strcat(chrPath,"\\objects.ini");/* Ini-File containing the hobjects */
  
  //  WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, chrPath, "~launchPad", 123, MB_OK| MB_MOVEABLE);  
  do{
    /* Open the ini-file */
    if((hIni=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),chrPath))==NULLHANDLE)
      break; 

    if((memPtr=(char*)malloc(ulNumObjects*sizeof(HOBJECT)))==NULL)
      break;
    

    hObject=(HOBJECT*)memPtr;

    lpoTemp=lpoObjectList;
    for(a=ulNumObjects;a>0 && lpoTemp; a--)
      {
        hObject[a-1]=lpoTemp->hObject;
        lpoTemp=lpoTemp->lpoNext;
      }

    if(!PrfWriteProfileData(hIni,"objects","handles", hObject, ulNumObjects*sizeof(HOBJECT))) {
      free(hObject);
      break;
    }

    free(hObject);
    
    PrfCloseProfile(hIni);
    return TRUE;
  }while(TRUE);

  if(hIni)
    PrfCloseProfile(hIni);
  return FALSE;
}

BOOL launchPad::lpReBuildObjectList()
{
  return lpBuildObjectList( chrConfigID);
}

BOOL launchPad::lpBuildObjectList(char * chrFolderID)
{
  WPObject* wpObject;

  char chrText[200];  
  ULONG ul;

  SOMClass *folderClass;
  WPFolder *wpFolder;
  char chrPath[CCHMAXPATH];
  ULONG ulBufferSize;
  HINI hIni;
  char * memPtr;
  HOBJECT *hObjectArray;
  int a;
  LPObject *lpoTemp;
  WPObject * wpTempObject;


  /* Get toolbar folder */
  if(somIsObj(wpParentFolder)) {
    folderClass=wpParentFolder->somGetClass();
    if(somIsObj(folderClass))
      wpFolder=(WPFolder*)((M_WPFolder*)folderClass)->wpclsQueryFolder(chrFolderID, FALSE);
    if(!somIsObj(wpFolder))
      return FALSE;
  }
  
  /* Build ini name */
  ulBufferSize=sizeof(chrPath);
  wpFolder->wpQueryRealName(chrPath, &ulBufferSize, TRUE);
  strcat(chrPath,"\\objects.ini");/* Ini-File containing the hobjects */
  
  do{
    /* Open the ini-file */
    if((hIni=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),chrPath))==NULLHANDLE)
      break; 
 
    if(!PrfQueryProfileSize(hIni,"objects","handles", &ulBufferSize))
      break;

    if(ulBufferSize==0)
      break;/* No entries yet */

    if((memPtr=(char*)malloc(ulBufferSize))==NULL)
      break;
    
    ulNumObjects=ulBufferSize/sizeof(HOBJECT);    

    if(!PrfQueryProfileData(hIni,"objects","handles", memPtr, &ulBufferSize)){
      free(memPtr);
      break;
    }

    hObjectArray=(HOBJECT*)memPtr;
    ulBufferSize=ulNumObjects;
    for(a=0;a<ulBufferSize && lpoTemp; a++)
      {
        wpTempObject=((M_WPObject*)folderClass)->wpclsQueryObject(hObjectArray[a]);
        if(somIsObj(wpTempObject)) {
          if((lpoTemp=new LPObject(wpTempObject->wpQueryIcon()))!=NULL) {
            lpoTemp->wpObject=wpTempObject;
            wpTempObject->wpLockObject();
            lpoTemp->hObject=hObjectArray[a];
            //            lpoTemp->hPtr=wpTempObject->wpQueryIcon();
            /* Title of the object */
            strncpy(lpoTemp->chrName, wpTempObject->wpQueryTitle(),sizeof(lpoTemp->chrName));
            lpoTemp->lpParent=this;
            
            lpoTemp->lpoNext=lpoObjectList;
            lpoObjectList=lpoTemp;
          }
        }
        else {
          ulNumObjects--;
        }
      }/* for */
    
    free(hObjectArray);
    
    PrfCloseProfile(hIni);
    return TRUE;
  }while(TRUE);

  if(hIni)
    PrfCloseProfile(hIni);
  return FALSE;

}

void launchPad::lpFreeObjectList()
{
  LPObject *lpoTemp, *lpoTemp2;

  /* Free our object list */
  lpoTemp=lpoObjectList;
  while(lpoTemp) {
    lpoTemp2=lpoTemp->lpoNext;
    lpoTemp->wpObject->wpUnlockObject();
    delete (lpoTemp);
    lpoTemp=lpoTemp2;
  }
  lpoObjectList=NULL;
}

BOOL launchPad::lpClearPad()
{
  LPObject *lpoTemp, *lpoTemp2;
  int a;
  HENUM hEnum;
  HWND hwnd;

  /* Destroy all Buttons */
  hEnum=WinBeginEnumWindows(hwndLaunchPad);
  while((hwnd=WinGetNextWindow(hEnum))!=NULLHANDLE)
        WinDestroyWindow(hwnd);
  WinEndEnumWindows(hEnum);

  return TRUE;
}


BOOL launchPad::lpFillPad()
{
  HOBJECT hObject;
  char chrSetup[CCHMAXPATH];
  WPFolder* wpFolder;
  LPObject *lpoTemp;

  SWP swp;
  HWND hwndTemp;
  BTNCDATA btCtrl;
  WNDPARAMS wndParams;
  ULONG ulStyle;
  int a;
  LONG lXSize;
  HENUM hEnum;
  HWND hwndClient;
  LONG lCxSave;

  /* Remove Buttons from pad */
  lpClearPad();

  /* Resize the pad for all objects */
  if(!WinQueryWindowPos(hwndLaunchPad, &swp))
    return FALSE;


  lCxSave=swp.cx;

  lXSize=(ulNumObjects == 0 ? swp.cy : (ulNumObjects) * (swp.cy+xButtonDelta) +2*xButtonOffset);
  lpSetLaunchPadPos( NULLHANDLE,0,0, lXSize , swp.cy, SWP_SIZE);

  /* Move all controls on the right */
  /* Move all client windows so they keep their distance to the left border */
  hEnum=WinBeginEnumWindows(WinQueryWindow(hwndLaunchPad,QW_PARENT));
  while((hwndClient=WinGetNextWindow(hEnum))!=NULLHANDLE) {
    SWP swpClient;

    WinQueryWindowPos(hwndClient,&swpClient);
    if(swpClient.x > swp.x)
      WinSetWindowPos(hwndClient, NULLHANDLE, swpClient.x+(lXSize-lCxSave), swpClient.y, 0, 0, SWP_MOVE);
  }/* while */
  WinEndEnumWindows(hEnum);

  lpoTemp=lpoObjectList;
  for(a=ulNumObjects;a>0 && lpoTemp; a--)
    {
      /* Create Buttons */ 
      if((hwndTemp=WinCreateWindow(hwndLaunchPad, WC_BUTTON, lpoTemp->chrName, WS_VISIBLE, xButtonOffset+(a-1)*(swp.cy+xButtonDelta), 1, swp.cy-2, swp.cy-2,
                                   hwndLaunchPad, HWND_TOP, a*100,
                                   NULLHANDLE, NULLHANDLE))!=NULLHANDLE)
        {
          /* Change Style to mini icon */
          ulStyle=WinQueryWindowULong(hwndTemp,QWL_STYLE);
          ulStyle|=(BS_MINIICON);
          ulStyle&=~BS_TEXT;
          if(WinSetWindowULong(hwndTemp,QWL_STYLE,ulStyle)) {
            memset(&btCtrl,0,sizeof(btCtrl));
            btCtrl.cb=sizeof(btCtrl);
            btCtrl.hImage=lpoTemp->hPtr;
            memset(&wndParams,0,sizeof(wndParams));
            wndParams.fsStatus=WPM_CTLDATA;
            wndParams.cbCtlData=btCtrl.cb;
            wndParams.pCtlData=&btCtrl;
            WinSendMsg(hwndTemp,WM_SETWINDOWPARAMS,
                       MPFROMP(&wndParams),0);
            /* Subclass button for flyover help */
            oldButtonProc2=WinSubclassWindow(hwndTemp, newLaunchPadButtonProc);
            /* Set the class pointer into the window words */
            WinSetWindowULong(hwndTemp,QWL_USER,(ULONG)lpoTemp);//Save object ptr.
          }
        }
      lpoTemp=lpoTemp->lpoNext;
    }
  return TRUE;  
}



