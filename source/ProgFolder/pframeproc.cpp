/*
 * pframeproc.cpp: window procedure for WPS folder which
 *                 handles our private frame controls
 *
 *             (C) Chris Wohlgenuth 1999
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
#include "progfolder.h"
#include "progfolder.hh"
#include <stdio.h>
        
/**************************************************************/
/*                                                            */
/* New frame window procedure which handles all the stuff     */
/* related to our private frame controls.                     */
/* It calls member functions of CWProgFolder to size and      */
/* position our frame controls.                               */
/*                                                            */
/**************************************************************/
MRESULT EXPENTRY folderFrameProcedure(HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2)
{
  MRESULT mr;
  PSWP  pswp, pswpClient=0, pswpFrame=0;
  PFNWP orgFrameProc;
  
  switch(msg) 
    {


    case WM_CALCVALIDRECTS:
      {
        MRESULT mr;
        PRECTL pRectl, pRectlNew;
        PSWP pswp;
        POINTL ptl;
        HWND hwndClient;
        FRAMECTRLDATA * fctrlData;
        SWP swp;

        pswp=(PSWP)mp2; 
        if(pswp->fl&SWP_SIZE) {
          pRectl=(PRECTL)mp1;        
          pRectlNew=pRectl;
          pRectlNew++;
          
          
          if(WinSendMsg(hwnd,WM_QUERYBORDERSIZE,(MPARAM)&ptl,0)) {
            pRectl->yBottom+=ptl.y;
            pRectl->yTop-=ptl.y;
            pRectl->xRight-=ptl.x;
            pRectl->xLeft+=ptl.x;
          }else {
            /* Ok, then use some defaults. Better than nothing... */
            pRectl->yBottom+=4;
            pRectl->yTop-=4;
            pRectl->xRight-=4;
            pRectl->xLeft+=4;
          }
          *pRectlNew=*pRectl;
          return (MRESULT)0;
        }
     
      }   
#if 0
    case WM_CALCVALIDRECTS:
      {
        PRECTL pRectl, pRectlNew;
        PSWP pswp;
        POINTL ptl;
        
        pswp=(PSWP)mp2; 
        if(pswp->fl&SWP_SIZE) {
          pRectl=(PRECTL)mp1;        
          pRectlNew=pRectl;
          pRectlNew++;

          if(pRectlNew->yBottom!=pRectl->yBottom) {
            if(WinSendMsg(hwnd,WM_QUERYBORDERSIZE,(MPARAM)&ptl,0)) {
              pRectlNew->xLeft+=ptl.x;
              pRectl->xLeft+=ptl.x;
              pRectlNew->xRight-=ptl.x;
              pRectl->xRight-=ptl.x;
              pRectl->yBottom+=ptl.y;
            }else {
              /* Ok, then use some defaults. Better than nothing... */
              pRectlNew->xLeft+=4;
              pRectl->xLeft+=4;
              pRectlNew->xRight-=4;
              pRectl->xRight-=4;
              pRectl->yBottom+=4;
            }
            pRectlNew->yBottom=pRectl->yBottom;
            return (MRESULT)0;
          }
#if 0
          else if(pRectlNew->yTop!=pRectl->yTop) {
              if(WinSendMsg(hwnd,WM_QUERYBORDERSIZE,(MPARAM)&ptl,0)) {
                pRectl->yBottom+=ptl.y;
                pRectl->yTop-=ptl.y;
                pRectl->xRight-=ptl.x;
                pRectl->xLeft+=ptl.x;
              }else {
                /* Ok, then use some defaults. Better than nothing... */
                pRectl->yBottom+=4;
                pRectl->yTop-=4;
                pRectl->xRight-=4;
                pRectl->xLeft+=4;
              }
              *pRectlNew=*pRectl;
              return (MRESULT)0;
          }
        }
#endif
        else if(pRectlNew->xLeft!=pRectl->xLeft || pRectlNew->xRight!=pRectl->xRight) {
          /* Make sure the border will be redrawn */
          if(WinSendMsg(hwnd,WM_QUERYBORDERSIZE,(MPARAM)&ptl,0)) {
            pRectl->yBottom+=ptl.y;
            pRectl->yTop-=ptl.y;
            pRectl->xRight-=ptl.x;
            pRectl->xLeft+=ptl.x;
          }else {
            /* Ok, then use some defaults. Better than nothing... */
            pRectl->yBottom+=4;
            pRectl->yTop-=4;
            pRectl->xRight-=4;
            pRectl->xLeft+=4;
          }
          *pRectlNew=*pRectl;
          return (MRESULT)0;
        }
        return (MRESULT)0;
      }
#endif

    case WM_CALCFRAMERECT:
    
      /* Get the original frame procedure from the window words of the invisible child window */ 
      orgFrameProc=(PFNWP)WinQueryWindowPtr(WinWindowFromID(hwnd, ID_FOLDERSTOREWINDOW),QWP_ORGFRAMEPROC);
      if(!orgFrameProc)return WinDefWindowProc(hwnd,msg,mp1,mp2);/* Error occured! The original frame proc is lost */
      
      mr=(*orgFrameProc)(hwnd,msg,mp1,mp2);

      //     mp2:
      //
      //     TRUE:  Frame rectangle provided, calculate client
      //     FALSE: Client area rectangle provided, calculate frame
      
      /* Calculate new size.
         This is done in a member function of our folder class.
         We get the pointer to our folder object by querying the
         window ptr. of the hidden child window. */ 
      ((CWProgFolder*)WinQueryWindowPtr(WinWindowFromID(hwnd, ID_FOLDERSTOREWINDOW),0))
        ->cwCalculateClient(hwnd, (PRECTL)mp1, SHORT1FROMMP( mp2));
      
      return mr;

    case WM_FORMATFRAME:
      SHORT countSwp;
      SWP  *sav;
    
      orgFrameProc=(PFNWP)WinQueryWindowPtr(WinWindowFromID(hwnd, ID_FOLDERSTOREWINDOW),QWP_ORGFRAMEPROC);
      if(!orgFrameProc)return WinDefWindowProc(hwnd,msg,mp1,mp2);/* Error occured */
      /* Query the count of standard frame controls */
      countSwp = (int) (*orgFrameProc)(hwnd, msg, mp1, mp2);

      /*
       * Reformat the frame to move the client
       * over and make room for the framecontrols.
       * Add the new controls to the array of
       * frame controls.
       * After doing so add our own framecontrols
       * to the total number of framecontrols.
       * This is done in the member function of the class.
       */
      return MRFROMSHORT(countSwp+((CWProgFolder*)WinQueryWindowPtr(WinWindowFromID(hwnd, ID_FOLDERSTOREWINDOW),
                                                                    0))
                         ->cwFormatFrame(hwnd, countSwp,(PSWP)mp1));//Our Frame(s) added
    case WM_QUERYFRAMECTLCOUNT:
      /* The system asks for the total count of framecontrols.
         First we ask the old frame procedure for the number of standard
         controls. After that we add our controls to the count. */
      orgFrameProc=(PFNWP)WinQueryWindowPtr(WinWindowFromID(hwnd, ID_FOLDERSTOREWINDOW),QWP_ORGFRAMEPROC);
      if(!orgFrameProc)return WinDefWindowProc(hwnd,msg,mp1,mp2);/* Error occured */
      
      return MRFROMSHORT((ULONG)((*orgFrameProc)(hwnd,msg,mp1,mp2))
                         +((CWProgFolder*)WinQueryWindowPtr(WinWindowFromID(hwnd, ID_FOLDERSTOREWINDOW),0))
                         ->cwNumAdditionalFrameControls(hwnd));//Our Frame(s) added

    case WM_CLOSE:
      orgFrameProc=(PFNWP)WinQueryWindowPtr(WinWindowFromID(hwnd, ID_FOLDERSTOREWINDOW),QWP_ORGFRAMEPROC);
      if(!orgFrameProc)
        return WinDefWindowProc(hwnd,msg,mp1,mp2);/* Error occured */
      /* Call the cwClose() method */
      if(((CWProgFolder*)WinQueryWindowPtr(WinWindowFromID(hwnd, ID_FOLDERSTOREWINDOW),0))
         ->cwClose(hwnd))
        return (MRESULT)(*orgFrameProc)(hwnd,msg,mp1,mp2);
      return (MRESULT) 0;
    default:
      orgFrameProc=(PFNWP)WinQueryWindowPtr(WinWindowFromID(hwnd, ID_FOLDERSTOREWINDOW),QWP_ORGFRAMEPROC);
      if(!orgFrameProc)return WinDefWindowProc(hwnd,msg,mp1,mp2);/* Error occured */
      
      return (*orgFrameProc)(hwnd,msg,mp1,mp2);
    }
  orgFrameProc=(PFNWP)WinQueryWindowPtr(WinWindowFromID(hwnd, ID_FOLDERSTOREWINDOW),QWP_ORGFRAMEPROC);
  if(!orgFrameProc)return WinDefWindowProc(hwnd,msg,mp1,mp2);/* Error occured */
  
  return (*orgFrameProc)(hwnd,msg,mp1,mp2);
  return WinDefWindowProc(hwnd,msg,mp1,mp2);/* Error occured */
}




