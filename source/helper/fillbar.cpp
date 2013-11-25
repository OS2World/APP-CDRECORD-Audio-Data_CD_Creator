/*
 * This file is (C) Chris Wohlgemuth 2001/2004
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
#define INCL_GPI
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Window ULONG for the percent bar */
#define QWL_PERCENT 4 /* The location in the window words to store the percent value */
#define QWL_TEXTPTR 8 /* The ptr to our percent bar text */

/*
 * Paint the percent bar and print the label if necessary.
 */

static VOID paintPercent(int iPercent, HWND hwnd, HPS hps)
{
    POINTL  ptl, ptlText, aptlText[TXTBOX_COUNT];
    RECTL   rcl, rcl2;
    BOOL    bVertical=FALSE;
    CHAR  * ptrChr=NULL;

    WinQueryWindowRect(hwnd, &rcl);
    /* Check if it's a vertical percent bar */
    if(rcl.xRight<rcl.yTop)
      bVertical=TRUE;
    else
      bVertical=FALSE;

    hps=WinGetPS(hwnd);

    GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
    
    /* Draw the bar border */
    WinDrawBorder(hps, &rcl, 1,1,0,0, 0x800);    
    
    rcl.xLeft = 1;
    rcl.xRight -= 1;
    rcl.yBottom = 1;
    rcl.yTop -= 1;
    
    if((ptrChr=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
      {
        /* Text size */
        GpiQueryTextBox(hps, strlen(ptrChr), ptrChr,
                        TXTBOX_COUNT, (PPOINTL)&aptlText);
   
        ptlText.x = rcl.xLeft+(((rcl.xRight-rcl.xLeft)
                                 -(aptlText[TXTBOX_BOTTOMRIGHT].x-aptlText[TXTBOX_BOTTOMLEFT].x))/2);
        ptlText.y = 3 + rcl.yBottom+(((rcl.yTop-rcl.yBottom)
                                      -(aptlText[TXTBOX_TOPLEFT].y-aptlText[TXTBOX_BOTTOMLEFT].y))/2);
      }

    if(!bVertical) {
      /* Horizontal bar */
      rcl2.xLeft = rcl.xLeft;
      rcl2.xRight = (rcl.xRight-rcl.xLeft)*iPercent/1000; 
      rcl2.yBottom = rcl.yBottom;
      rcl2.yTop = rcl.yTop-1;
      rcl.xLeft=rcl2.xRight+1;
    }
    else {
      rcl2.xLeft = rcl.xLeft;
      rcl2.xRight = rcl.xRight-1;
      rcl2.yBottom = rcl.yBottom;
      rcl2.yTop = (rcl.yTop-rcl.yBottom)*iPercent/1000; 
      rcl.yBottom=rcl2.yTop+1;
    }

    /* Background */
    WinFillRect(hps, &rcl,
                WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0));

    //    WinFillRect(hps, &rcl,
    //          WinQuerySysColor(HWND_DESKTOP, CLR_RED, 0));


    /* Percentbar */
    if ((rcl2.xRight > rcl2.xLeft && !bVertical)||(rcl2.yTop > rcl2.yBottom && bVertical)) {
      ULONG ulBG;

      /* Find color. This color is the background color set within DrDialog */
      if(!WinQueryPresParam(hwnd, PP_BACKGROUNDCOLOR, PP_BACKGROUNDCOLORINDEX, NULL, sizeof(ulBG),
                        &ulBG, QPF_ID2COLORINDEX|QPF_NOINHERIT ))
        ulBG=0x002020ff;
      GpiSetColor(hps,ulBG );

      rcl2.yBottom+=1;
      rcl2.xLeft+=1;

      WinFillRect(hps, &rcl2, ulBG);
      WinDrawBorder(hps, &rcl2, 1,1,0,0, 0x400);
    }

    /* Draw 74 min marker */
    GpiSetColor(hps, 0x00ff0000);
    ptl.x=(rcl.xRight*74)/80;
    ptl.y=1;
    GpiMove(hps, &ptl);
    ptl.y=rcl.yTop-1;
    GpiLine(hps, &ptl);

    /* now print the percentage */
    if(ptrChr!=NULLHANDLE)
      {
        ULONG ulFG; 
       
        /* Find color. This color is the foreground color set within DrDialog */
        if(!WinQueryPresParam(hwnd, PP_FOREGROUNDCOLOR, PP_FOREGROUNDCOLORINDEX, NULL, sizeof(ulFG),
                              &ulFG, QPF_ID2COLORINDEX|QPF_NOINHERIT ))
          ulFG=WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONDEFAULT, 0);
        GpiSetColor(hps,ulFG );
        GpiMove(hps, &ptlText);
        GpiCharString(hps, strlen(ptrChr), ptrChr);
      }
    WinReleasePS(hps);
}


/*
 * This is the window proc for the percentbar control
 */

MRESULT EXPENTRY fillBarProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  MRESULT mrc;
  HPS hps;
  PWNDPARAMS pwp;
  int iPercent;
  RECTL rcl;

  switch(msg) {

  case WM_SETWINDOWPARAMS:
    {
      pwp=(PWNDPARAMS)mp1;

      if(pwp->fsStatus==WPM_TEXT) {
        /* The text changed */
        char *ptr;
        char *ptr2;

        /* Get the current percent value for the control */
        iPercent=atol(pwp->pszText);
        if(iPercent>1000)
          iPercent=1000;
        if(iPercent<0)
          iPercent=0;

        /* Check if there is some text for the bar */
        if((ptr=strchr(pwp->pszText, '#'))!=NULLHANDLE) {
          /* Everything after the '#' is treated as the label */
          if((ptr2=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
            free(ptr2); /* Free the old text */
          WinSetWindowPtr(hwnd,QWL_TEXTPTR, NULLHANDLE);
          if(*(ptr++)!=0) {
            /* There's additional text to print */
            if((ptr2=(char*)calloc(strlen(ptr)+1, sizeof(char)))!=NULLHANDLE) {
              strcpy(ptr2,ptr);
              WinSetWindowPtr(hwnd,QWL_TEXTPTR,ptr2);
            }
          }
        }
        mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
        WinSetWindowULong(hwnd, QWL_PERCENT,iPercent);
        WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
        return mrc;
      }
      break;
    }
  case WM_DESTROY:
    {
      char *ptrText;
      /* Free the memory allocated for the text */
      if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
        free(ptrText);
      break;
    }
  case WM_PRESPARAMCHANGED:
    /* The color or the font has changed  */
    /* Force a repaint of the percent bar */
    mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
    if(LONGFROMMP(mp1)==PP_FOREGROUNDCOLOR)
      WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
    else if(LONGFROMMP(mp1)==PP_BACKGROUNDCOLOR)
      WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
    else if(LONGFROMMP(mp1)==PP_FONTNAMESIZE)
      WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
    return mrc;
  case WM_PAINT:
    {
      hps=WinBeginPaint(hwnd, NULLHANDLE, NULLHANDLE);
      paintPercent(WinQueryWindowULong(hwnd,QWL_PERCENT), hwnd, hps);
      WinEndPaint(hps);
    return (MRESULT) FALSE;
    }
  
  default:
    break;
  }

  mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
  return (mrc);
}


BOOL registerFillBarClass(void)
{
  return WinRegisterClass(WinQueryAnchorBlock(HWND_DESKTOP),"CW_FILLBAR", fillBarProc,0L, 8L);
}
