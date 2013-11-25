/*
 * This file is (C) Chris Wohlgemuth 2002-2004
 * It is part of the MediaFolder package
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
#define INCL_GPILCIDS
#define INCL_GPIPRIMITIVES
#define INCL_GPIBITMAPS
#define INCL_GPILOGCOLORTABLE
#define INCL_SW
#define INCL_GPIREGIONS

#include "audiofolder.hh"
#include "audiofolderres.h"
#include <stdio.h>
#include <stdlib.h>


/* Fonts to use in dialogs */
#define DEFAULT_DIALOG_FONT   "9.WarpSans"


//extern HMODULE hResource;

extern LOADEDBITMAP allBMPs[];
extern BOOL bUseCustomPainting;
extern CONTROLINFO ciControls[];

//PFNWP  oldButtonProc;  //place for original button-procedure
PFNWP  oldStaticTextProc;
PFNWP  oldCheckBoxProc;
PFNWP oldRadioProc;
PFNWP  oldGroupBoxProc;

//PFNWP  oldPlayTimeProc;

/* Extern */

/* Local */


void HlpDrawFrameBorder(HPS hps, HWND hwnd)
{
  RECTL rcl;
  POINTL ptl;
  LONG  lColor;
  ULONG attrFound;

  /* Border */
  GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
  
  WinQueryWindowRect(hwnd, &rcl);
  ptl.x=1;
  ptl.y=0;
  GpiMove(hps, &ptl);
  GpiSetColor(hps, CLR_BLACK);
  ptl.x=rcl.xRight-1;
  GpiLine(hps,&ptl);
  ptl.y=rcl.yTop-1;
  GpiLine(hps,&ptl);
  GpiSetColor(hps, SYSCLR_SHADOW);
  ptl.x=rcl.xLeft;
  GpiLine(hps,&ptl);
  ptl.y=0;
  GpiLine(hps,&ptl);

  rcl.yTop-=1;
  rcl.yBottom+=1;
  rcl.xLeft+=1;
  rcl.xRight-=1;

  WinDrawBorder( hps,&rcl, 1, 1, 0, 0, 0x400);
  rcl.yTop-=1;
  rcl.yBottom+=1;
  rcl.xLeft+=1;
  rcl.xRight-=1;

  /* Get active border color */
  if(WinQueryActiveWindow(HWND_DESKTOP)==hwnd) {
    if ( (WinQueryPresParam(hwnd, PP_BORDERCOLOR, 0, &attrFound, sizeof(attrFound),
                            &lColor, QPF_PURERGBCOLOR)) == 0 )
      lColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0);
  }
  else {
    /* Inactive border color */
    if ( (WinQueryPresParam(hwnd, PP_INACTIVECOLOR, 0, &attrFound, sizeof(attrFound),
                            &lColor, QPF_PURERGBCOLOR)) == 0 )
      lColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0);
  }
  /*            Get Border size */
  WinSendMsg(hwnd, WM_QUERYBORDERSIZE, MPFROMP(&ptl),0);

  WinDrawBorder(hps,&rcl, ptl.x-2, ptl.y-2, lColor, 0, 0);
}

BOOL HlpPaintFrame(HWND hwnd, BOOL bInclBorder)
{
  HPS hps;
  RECTL rcl, rclSource;
  POINTL ptl;
  LONG lTemp;
  
  if(!bUseCustomPainting) 
    return FALSE;
  if(allBMPs[CTRLIDX_BG].hbm) {
    hps=WinBeginPaint(hwnd, NULL, &rcl);
    rclSource.xLeft=0;
    rclSource.yBottom=0;
    rclSource.yTop=allBMPs[CTRLIDX_BG].bmpInfoHdr.cy;
    rclSource.xRight=allBMPs[CTRLIDX_BG].bmpInfoHdr.cx;
    lTemp=rcl.xLeft/rclSource.xRight;
    ptl.x=lTemp*rclSource.xRight;
    lTemp=rcl.yBottom/rclSource.yTop;
    lTemp*=rclSource.yTop;   
    //WinFillRect(hps, &rcl, CLR_RED);
    while(ptl.x<rcl.xRight) {
      ptl.y=lTemp;
      while(ptl.y<rcl.yTop) {/* y direction */
        //DosBeep(5000,100);
        WinDrawBitmap(hps, allBMPs[CTRLIDX_BG].hbm,
                      &rclSource, 
                      (PPOINTL)&ptl,
                      0, 0,
                      DBM_IMAGEATTRS);
        ptl.y+=allBMPs[CTRLIDX_BG].bmpInfoHdr.cy;
        //DosSleep(200);
      };
      ptl.x+=allBMPs[CTRLIDX_BG].bmpInfoHdr.cx; 
    };
    /* Draw border */
    if(bInclBorder)
      HlpDrawFrameBorder( hps, hwnd);
    WinEndPaint(hps);
    return TRUE;
  }
  return FALSE;
}


MRESULT EXPENTRY staticTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg)
    {
    case WM_SETWINDOWPARAMS:
      if(bUseCustomPainting) {
        MRESULT mr;
        PWNDPARAMS pwp=(PWNDPARAMS)PVOIDFROMMP(mp1);
        if(pwp->fsStatus==WPM_TEXT) {
          /* The control text changes. Force repaint of background by frame. */
          if(WinIsWindowVisible(hwnd)) {
            WinShowWindow(hwnd, FALSE);
            mr=oldStaticTextProc(hwnd, msg, mp1, mp2);/* Change text */
            WinShowWindow(hwnd, TRUE);/* Force painting of text */
            return mr;
          }
        }
      }
      break;
    case WM_PAINT:
      {
        HPS hps;
        char text[200];

        if(bUseCustomPainting) {
          hps=WinBeginPaint(hwnd, NULLHANDLE, NULL);
          if(WinQueryWindowText(hwnd, sizeof(text), text))
            {
              RECTL rcl;
              ULONG ulStyle;
              WinQueryWindowRect(hwnd, &rcl);
              GpiCreateLogColorTable(hps, LCOL_PURECOLOR,LCOLF_RGB, 0, 0, NULLHANDLE);
              rcl.xLeft+=1;
              ulStyle=WinQueryWindowULong(hwnd, QWL_STYLE);
              ulStyle&=0xfff2;
              WinDrawText(hps, strlen(text), text, &rcl, 0x00eeeeef,0, ulStyle);
              rcl.xLeft-=1;
              rcl.yBottom+=1;
              WinDrawText(hps, strlen(text),text, &rcl, 0x00333355,0,ulStyle/*DT_VCENTER| DT_CENTER   |DT_TEXTATTRS*/);
              
            }
          WinEndPaint(hps);
          return (MRESULT) 0;
        }
        break;
      }
    default:
      break;
    }
  return oldStaticTextProc(hwnd, msg, mp1, mp2);
}


/* This proc handles the custom painting for check boxes */
MRESULT EXPENTRY groupBoxProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg)
    {
    case WM_PAINT:
      if(bUseCustomPainting) {
        HPS hps;
        RECTL rcl;
        char textBuffer[100];
        ULONG textLength;
        CONTROLINFO ci;
        MRESULT mr;
        POINTL ptl[TXTBOX_COUNT];
        SIZEL charSize;
        POINTL pointl;
        ULONG xDelta;
        ULONG yDelta;
        ULONG drawStyle;

        hps=WinBeginPaint(hwnd, NULLHANDLE, NULL);

        //  mr=oldGroupBoxProc(hwnd, msg, mp1, mp2);
        //  hps=WinGetPS(hwnd);
        WinQueryWindowRect(hwnd, &rcl);

        // Get Textboxdimensions
        WinQueryWindowText(hwnd, sizeof(textBuffer), textBuffer);
        textLength = strlen(textBuffer);  // ACHTUNG!!!!! SYSTEMFUNKTION VERWENDEN??!!

        GpiQueryTextBox(hps, textLength, textBuffer, TXTBOX_COUNT, ptl);
        GpiQueryDefCharBox(hps, &charSize);

        pointl.y = rcl.yTop - charSize.cy / 2 - 3;
        pointl.x = 3;
        GpiMove(hps, &pointl);
        GpiSetColor(hps, SYSCLR_BUTTONLIGHT);  // Change to SYSCLR use

        pointl.x = 1;
        GpiLine(hps, &pointl);
        // left
        pointl.y = 0;
        GpiLine(hps, &pointl);
        // bottom
        pointl.x = rcl.xRight - 1;
        GpiLine(hps, &pointl);
        // right
        pointl.y = rcl.yTop - charSize.cy / 2 - 3;
        GpiLine(hps, &pointl);
        xDelta = ptl[TXTBOX_TOPRIGHT].x + 1;
        if (xDelta == 1)
          xDelta = -1;
        pointl.x = xDelta + 5;
        GpiLine(hps, &pointl);
        pointl.y += 1;
        GpiMove(hps, &pointl);
        GpiSetColor(hps, SYSCLR_WINDOWFRAME);
        pointl.x = rcl.xRight - 2;
        GpiLine(hps, &pointl);
        pointl.y = 1;
        GpiLine(hps, &pointl);
        pointl.x = 0;
        GpiLine(hps, &pointl);
        pointl.y = rcl.yTop - charSize.cy / 2 - 2;
        GpiLine(hps, &pointl);
        pointl.x = 3;
        GpiLine(hps, &pointl);

        /* Looks like a senseless second call but is necessary */
        WinQueryWindowText(hwnd, sizeof(textBuffer), textBuffer);
        drawStyle = WinQueryWindowULong(hwnd, QWL_STYLE);
        drawStyle &= (ULONG) 0xFF00;

        rcl.xRight -= 6;
        rcl.xLeft = 6;
        rcl.yBottom = 0;
        rcl.yTop-=1;
        GpiCreateLogColorTable(hps, LCOL_PURECOLOR,LCOLF_RGB, 0, 0, NULLHANDLE);
        /* Draw shadow text */
        WinDrawText(hps, textLength, textBuffer, &rcl, 0x00eeeeef,0, drawStyle);
        rcl.xLeft-=1;
        // rcl.yBottom+=1;
        rcl.yTop+=1;
        WinDrawText(hps, textLength, textBuffer, &rcl, SYSCLR_WINDOWSTATICTEXT,
                    0, drawStyle);
        
        //  WinDrawText(hps, strlen(text),text, &rcl, 0x00333355,0,DT_VCENTER/*|DT_TEXTATTRS*/);

        WinEndPaint(hps);
        return (MRESULT) 0;
      }
      break;
    default:
      break;
    }
  return oldGroupBoxProc(hwnd, msg, mp1, mp2);
}

/* This proc handles the custom painting for check boxes */
MRESULT EXPENTRY checkBoxProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg)
    {
    case WM_PAINT:
      if(bUseCustomPainting) {
        short       checkState;
        HPS hps;
        RECTL rcl;
        char text[100];
        CONTROLINFO ci;

BOOL fHiLite;

        checkState = SHORT1FROMMR(WinSendMsg(hwnd, BM_QUERYCHECK, NULL, NULL));
        fHiLite=LONGFROMMR(WinSendMsg(hwnd, BM_QUERYHILITE, NULL, NULL));
        if(checkState==0) {
          /* unchecked */
          if(fHiLite)
            ci=ciControls[CTRLIDX_UNCHECKSEL];
          else
            ci=ciControls[CTRLIDX_UNCHECK];
        }
        else if(checkState==1) {
          /* checked */
          ci=ciControls[CTRLIDX_CHECK];
        }

        WinQueryWindowRect(hwnd, &rcl);
        hps=WinBeginPaint(hwnd, NULLHANDLE, NULL);
        if(WinQueryWindowText(hwnd, sizeof(text), text))
          {
            POINTL ptl= {0};
            ULONG ulPos;
            char text2[120];
            RECTL rcl2=rcl;
            ptl.y=(rcl.yTop-ci.rclSource.yTop)/2;

            /* Draw button bitmap */
            WinDrawBitmap(hps, ci.hbmSource,
                          NULL,
                          &ptl,
                          0, 0,
                          DBM_IMAGEATTRS);
            rcl.xLeft+=ci.rclSource.xRight+2;
            GpiCreateLogColorTable(hps, LCOL_PURECOLOR,LCOLF_RGB, 0, 0, NULLHANDLE);
            rcl.xLeft+=1;
            /* Get mnemonic */
            if((ulPos=WinQueryWindowULong(hwnd,QWL_USER))& 0x00010000){
              ulPos&=0x0000FFFF;
              strncpy(text2,text,ulPos);
              text2[ulPos]=0;
              strcat(text2,"~");
              strcat(text2,&text[ulPos]);

              WinDrawText(hps, strlen(text2), text2, &rcl, 0x00eeeeef,0, DT_VCENTER|DT_MNEMONIC);
              //WinDrawText(hps, strlen(text2), text2, &rcl, 0x00eeeeef,0, DT_VCENTER|DT_MNEMONIC|DT_HALFTONE);
              rcl.xLeft-=1;
              rcl.yBottom+=1;
              WinDrawText(hps, strlen(text2), text2, &rcl, 0x00333355,0,/*DT_TEXTATTRS|*/DT_VCENTER|DT_MNEMONIC);
              //WinDrawText(hps, strlen(text2), text2, &rcl, 0x00333355,0,/*DT_TEXTATTRS|*/DT_VCENTER|DT_MNEMONIC|DT_HALFTONE);
            }
            else {
              WinDrawText(hps, strlen(text), text, &rcl, 0x00eeeeef,0, DT_VCENTER|DT_MNEMONIC);
              // WinDrawText(hps, strlen(text), text, &rcl, 0x00eeeeef,0, DT_VCENTER|DT_MNEMONIC|DT_HALFTONE);
              rcl.xLeft-=1;
              rcl.yBottom+=1;
              WinDrawText(hps, strlen(text),text, &rcl, 0x00333355,0,DT_VCENTER/*|DT_TEXTATTRS*/);
            }
            if(!WinIsWindowEnabled(hwnd))
              {
                POINTL pointl;
                POINTL aptl[TXTBOX_COUNT];
                //                WinQueryWindowText(hwnd, sizeof(text), text);
                GpiQueryTextBox(hps, strlen(text), text, TXTBOX_COUNT, aptl);
                /* Draw halftone because button isn't enabled */
                GpiSetPattern(hps,PATSYM_DENSE7);
                //  pointl.x = rcl.xLeft;
                //  pointl.y = rcl.yBottom;
                pointl.x = rcl2.xLeft;
                pointl.y = rcl2.yBottom;
                GpiMove(hps, &pointl);
                pointl.x = rcl.xLeft+aptl[TXTBOX_BOTTOMRIGHT].x+2;
                pointl.y = rcl.yTop - 2;
                GpiBox(hps,DRO_FILL,&pointl,1,1);// Draw as disabled if not enabled using PATSYM_XX
              }
          }
        WinEndPaint(hps);
        return (MRESULT) 0;
      }
      break;
    case WM_ENABLE:
      if(bUseCustomPainting) {
        MRESULT mr;
        if(SHORT1FROMMP(mp1)) {
          if(WinIsWindowVisible(hwnd)) {
            WinShowWindow(hwnd, FALSE);
            mr=oldCheckBoxProc(hwnd, msg, mp1, mp2);
            WinShowWindow(hwnd, TRUE);/* Force painting of text */
            return mr;
          }
        }
      }
      break;
    case BM_SETHILITE:
    case BM_SETDEFAULT:
    case BM_SETCHECK:
      if(bUseCustomPainting) {
        MRESULT mr;
        WinEnableWindowUpdate(hwnd, FALSE);
        mr=oldCheckBoxProc(hwnd, msg, mp1, mp2);
        WinEnableWindowUpdate(hwnd, TRUE);
        WinInvalidateRect(hwnd, NULL,TRUE);
        return mr;
      }
    default:
      break;
    }
  return oldCheckBoxProc(hwnd, msg, mp1, mp2);
}

/* This proc handles the custom painting for check boxes */
MRESULT EXPENTRY radioProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
 
  switch(msg)
    {
    case WM_PAINT:

      if(bUseCustomPainting) {
        short       checkState;
        HPS hps;
        RECTL rcl;
        char text[100];
        CONTROLINFO ci;
        BOOL fHiLite;

        fHiLite=LONGFROMMR(WinSendMsg(hwnd, BM_QUERYHILITE, NULL, NULL));
        checkState = SHORT1FROMMR(WinSendMsg(hwnd,BM_QUERYCHECK,NULL,NULL));
        if(checkState==0) {
          /* unchecked */
          if(fHiLite)
            ci=ciControls[CTRLIDX_RADUNCHECKSEL];
          else
            ci=ciControls[CTRLIDX_RADUNCHECK];
        }
        else if(checkState==1) {
          /* checked */
          if(fHiLite)
            ci=ciControls[CTRLIDX_RADCHECKSEL];
          else
            ci=ciControls[CTRLIDX_RADCHECK];
        }
        
        WinQueryWindowRect(hwnd, &rcl);
        hps=WinBeginPaint(hwnd, NULLHANDLE, NULL);
        if(WinQueryWindowText(hwnd, sizeof(text), text))
          {
            POINTL ptl= {0};
            ULONG ulPos;
            char text2[120];
            HDC hdc;
            HPS hpsSource;
            SIZEL slHps;
            POINTL aptl[3];
            HAB hab=WinQueryAnchorBlock(hwnd);
            RECTL rcl2=rcl;

            rcl.xLeft+=ci.rclSource.xRight+2;
            GpiCreateLogColorTable(hps, LCOL_PURECOLOR,LCOLF_RGB, 0, 0, NULLHANDLE);
            rcl.xLeft+=1;

            // setup source bitmap
            hdc = DevOpenDC(hab, OD_MEMORY, "*", 0L, (PDEVOPENDATA) NULL, NULLHANDLE);

            // create source hps
            slHps.cx = ci.rclSource.xRight;
            slHps.cy = ci.rclSource.yTop;
            hpsSource = GpiCreatePS(hab,
                              hdc,
                              &slHps,
                              PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);


            aptl[0].x=0;
            aptl[0].y=(rcl.yTop-ci.rclSource.yTop)/2;
            aptl[1].x=ci.rclSource.xRight;
            aptl[1].y=aptl[0].y+ci.rclSource.yTop;
            aptl[2].x=0;
            aptl[2].y=0;

            /* Mask */
            // set mask bmp into hps
            if(fHiLite)
              GpiSetBitmap(hpsSource, ciControls[CTRLIDX_RADMASKSEL].hbmSource);
            else
              GpiSetBitmap(hpsSource, ciControls[CTRLIDX_RADMASK].hbmSource);
            GpiBitBlt(hps, hpsSource, 3L, aptl, ROP_MERGEPAINT, BBO_IGNORE);

            // set bmp into hps
            GpiSetBitmap(hpsSource, ci.hbmSource);
            GpiBitBlt(hps, hpsSource, 3L, aptl, ROP_SRCAND, BBO_IGNORE);

            GpiSetBitmap(hpsSource, NULLHANDLE);
            GpiDestroyPS(hpsSource);
            DevCloseDC(hdc);

            /* Get mnemonic */
            if((ulPos=WinQueryWindowULong(hwnd,QWL_USER))& 0x00010000){
              ulPos&=0x0000FFFF;
              strncpy(text2,text,ulPos);
              text2[ulPos]=0;
              strcat(text2,"~");
              strcat(text2,&text[ulPos]);

              WinDrawText(hps, strlen(text2), text2, &rcl, 0x00eeeeef,0, DT_VCENTER|DT_MNEMONIC);
              rcl.xLeft-=1;
              rcl.yBottom+=1;
              WinDrawText(hps, strlen(text2), text2, &rcl, 0x00333355,0,/*DT_TEXTATTRS|*/DT_VCENTER|DT_MNEMONIC);
            }
            else {
              WinDrawText(hps, strlen(text), text, &rcl, 0x00eeeeef,0, DT_VCENTER|DT_MNEMONIC);
              rcl.xLeft-=1;
              rcl.yBottom+=1;
              WinDrawText(hps, strlen(text),text, &rcl, 0x00333355,0,DT_VCENTER/*|DT_TEXTATTRS*/);
            }
            if(!WinIsWindowEnabled(hwnd))
              {
                POINTL pointl;
                POINTL aptl[TXTBOX_COUNT];
                //                WinQueryWindowText(hwnd, sizeof(text), text);
                GpiQueryTextBox(hps, strlen(text), text, TXTBOX_COUNT, aptl);
                /* Draw halftone because button isn't enabled */
                GpiSetPattern(hps,PATSYM_DENSE7);
                pointl.x = rcl2.xLeft;
                pointl.y = rcl2.yBottom;
                GpiMove(hps, &pointl);
                pointl.x = rcl.xLeft+aptl[TXTBOX_BOTTOMRIGHT].x+2;
                pointl.y = rcl.yTop - 2;
                GpiBox(hps,DRO_FILL,&pointl,1,1);// Draw as disabled if not enabled using PATSYM_XX
              }
          }
        WinEndPaint(hps);
        return (MRESULT) 0;
      }
      break;

    case WM_ENABLE:
      if(bUseCustomPainting) {
        MRESULT mr;
        if(SHORT1FROMMP(mp1)) {
          if(WinIsWindowVisible(hwnd)) {
            WinShowWindow(hwnd, FALSE);
            mr=oldRadioProc(hwnd, msg, mp1, mp2);
            WinShowWindow(hwnd, TRUE);/* Force painting of text */
            return mr;
          }
        }
      }
      break;
    case BM_SETHILITE:
    case BM_SETDEFAULT:
    case BM_SETCHECK:
      if(bUseCustomPainting) {
        MRESULT mr;

        WinEnableWindowUpdate(hwnd, FALSE);
        mr=oldRadioProc(hwnd, msg, mp1, mp2);
        WinEnableWindowUpdate(hwnd, TRUE);

        WinInvalidateRect(hwnd, NULL,TRUE);
        return mr;
      }
      break;
    default:
      break;
    }
  return oldRadioProc(hwnd, msg, mp1, mp2);
}

void setupCheckBoxControl(  HWND hwnd, USHORT id)
{
  char text[CCHMAXPATH];
  HWND hwndTemp;
  ULONG ulLen;

  oldCheckBoxProc=WinSubclassWindow(WinWindowFromID(hwnd, id),checkBoxProc);
  /* Find mnemonic */
  hwndTemp=WinWindowFromID(hwnd, id);
  if((ulLen=WinQueryWindowText( hwndTemp, sizeof(text), text))!=0) {
    int a;
    
    text[CCHMAXPATH-1]=0;
    for(a=0;a<ulLen;a++) {
      if(WinSendMsg( hwndTemp, WM_MATCHMNEMONIC,MPFROMSHORT((USHORT)text[a]),0))
        break;/* Ok, mnemonic found */
    }
    if(a<=ulLen) {
      WinSetWindowULong(hwndTemp, QWL_USER, (ULONG)(0x00010000+a));/* For mnemonic */
    }
  }
}

void setupRadioButtonControl(HWND hwnd, USHORT id)
{
  char text[CCHMAXPATH];
  HWND hwndTemp;
  ULONG ulLen;

  oldRadioProc=WinSubclassWindow(WinWindowFromID(hwnd, id), radioProc);
  /* Find mnemonic */
  hwndTemp=WinWindowFromID(hwnd, id);
  if((ulLen=WinQueryWindowText( hwndTemp, sizeof(text), text))!=0) {
    int a;
    
    text[CCHMAXPATH-1]=0;
    for(a=0;a<ulLen;a++) {
      if(WinSendMsg( hwndTemp, WM_MATCHMNEMONIC,MPFROMSHORT((USHORT)text[a]),0))
        break;/* Ok, mnemonic found */
    }
    if(a<=ulLen) {
      WinSetWindowULong(hwndTemp, QWL_USER, (ULONG)(0x00010000+a));/* For mnemonic */
    }
  }
}

void setupStaticTextControl(HWND hwnd, USHORT id)
{
  /* subclass static text control for custom painting */
  oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, id), staticTextProc);
}

void setupGroupBoxControl(HWND hwnd, USHORT id)
{

  oldGroupBoxProc=WinSubclassWindow(WinWindowFromID(hwnd, id), groupBoxProc);

}

