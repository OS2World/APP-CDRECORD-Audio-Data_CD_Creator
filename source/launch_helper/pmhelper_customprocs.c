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
#define INCL_WIN
#define INCL_GPIREGIONS
#define INCL_GPIBITMAPS

#include <os2.h>
#include "audiofolderres.h"
#include "audiofolder.h"
#include <stdio.h>
#include <stdlib.h>


/* Fonts to use in dialogs */
#define DEFAULT_DIALOG_FONT   "9.WarpSans"

/* For custom BG */
extern BOOL bUseCustomPainting;

LOADEDBITMAP allBMPs[NUM_CTRL_IDX]={0};
CONTROLINFO ciControls[NUM_CTRL_IDX]={
  {0,{0,0,590,70}, NULLHANDLE,{0}},/* main */
  {1, {0, 0, 13,13},NULLHANDLE, { 151, 18, 187, 54} }, /* Placeholder for checkbutton */
  {1, {0, 0, 13,13},NULLHANDLE, { 151, 18, 187, 54} }, /* Placeholder for checkbutton */
  {1, {0, 0, 13,13},NULLHANDLE, { 151, 18, 187, 54} }, /* Placeholder for radiobutton */
  {1, {0, 0, 13,13},NULLHANDLE, { 151, 18, 187, 54} }, /* Placeholder for radiobutton */
  {1, {0, 0, 13,13},NULLHANDLE, { 151, 18, 187, 54} } /* Placeholder for checkbutton sel */
};

//PFNWP  oldButtonProc;  //place for original button-procedure
PFNWP  oldStaticTextProc;
PFNWP  oldCheckBoxProc;
PFNWP oldRadioProc;
PFNWP  oldGroupBoxProc;

//PFNWP  oldPlayTimeProc;

/* Extern */

/* Local */
void _loadBmps(HMODULE hSettingsResource)
{
  HPS hps;
  BITMAPINFOHEADER bmpInfoHdr;

  if(allBMPs[CTRLIDX_BG].hbm)
    return; /* already loaded */

  hps=WinGetPS(HWND_DESKTOP);

  /* The main BMP */
  allBMPs[CTRLIDX_BG].hbm=GpiLoadBitmap(hps, hSettingsResource, IDBMP_BACKGROUND, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_BG].hbm, &allBMPs[CTRLIDX_BG].bmpInfoHdr);

  /* Checkbutton checked */
  allBMPs[CTRLIDX_CHECK].hbm=GpiLoadBitmap(hps, hSettingsResource, IDBMP_CHECK, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_CHECK].hbm, &allBMPs[CTRLIDX_CHECK].bmpInfoHdr);
  ciControls[CTRLIDX_CHECK].hbmSource=allBMPs[CTRLIDX_CHECK].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_CHECK].rclSource.yBottom=0;
  ciControls[CTRLIDX_CHECK].rclSource.yTop=allBMPs[CTRLIDX_CHECK].bmpInfoHdr.cy;
  ciControls[CTRLIDX_CHECK].rclSource.xLeft=0;
  ciControls[CTRLIDX_CHECK].rclSource.xRight=allBMPs[CTRLIDX_CHECK].bmpInfoHdr.cx;

  /* Checkbutton unchecked */
  allBMPs[CTRLIDX_UNCHECK].hbm=GpiLoadBitmap(hps, hSettingsResource, IDBMP_UNCHECK, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_UNCHECK].hbm, &allBMPs[CTRLIDX_UNCHECK].bmpInfoHdr);
  ciControls[CTRLIDX_UNCHECK].hbmSource=allBMPs[CTRLIDX_UNCHECK].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_UNCHECK].rclSource.yBottom=0;
  ciControls[CTRLIDX_UNCHECK].rclSource.yTop=allBMPs[CTRLIDX_UNCHECK].bmpInfoHdr.cy;
  ciControls[CTRLIDX_UNCHECK].rclSource.xLeft=0;
  ciControls[CTRLIDX_UNCHECK].rclSource.xRight=allBMPs[CTRLIDX_UNCHECK].bmpInfoHdr.cx;

  /* Checkbutton unchecked selected */
  allBMPs[CTRLIDX_UNCHECKSEL].hbm=GpiLoadBitmap(hps, hSettingsResource, IDBMP_UNCHECKSEL, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_UNCHECKSEL].hbm, &allBMPs[CTRLIDX_UNCHECKSEL].bmpInfoHdr);
  ciControls[CTRLIDX_UNCHECKSEL].hbmSource=allBMPs[CTRLIDX_UNCHECKSEL].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_UNCHECKSEL].rclSource.yBottom=0;
  ciControls[CTRLIDX_UNCHECKSEL].rclSource.yTop=allBMPs[CTRLIDX_UNCHECKSEL].bmpInfoHdr.cy;
  ciControls[CTRLIDX_UNCHECKSEL].rclSource.xLeft=0;
  ciControls[CTRLIDX_UNCHECKSEL].rclSource.xRight=allBMPs[CTRLIDX_UNCHECKSEL].bmpInfoHdr.cx;

  /* Radiobutton checked */
  allBMPs[CTRLIDX_RADCHECK].hbm=GpiLoadBitmap(hps, hSettingsResource, IDBMP_RADCHECK, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_RADCHECK].hbm, &allBMPs[CTRLIDX_RADCHECK].bmpInfoHdr);
  // get bitmap info
  ciControls[CTRLIDX_RADCHECK].bmpInfoHdr2.cbFix = sizeof(ciControls[CTRLIDX_RADCHECK].bmpInfoHdr2);
  GpiQueryBitmapInfoHeader(allBMPs[CTRLIDX_RADCHECK].hbm, &ciControls[CTRLIDX_RADCHECK].bmpInfoHdr2);
  ciControls[CTRLIDX_RADCHECK].hbmSource=allBMPs[CTRLIDX_RADCHECK].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_RADCHECK].rclSource.yBottom=0;
  ciControls[CTRLIDX_RADCHECK].rclSource.yTop=allBMPs[CTRLIDX_RADCHECK].bmpInfoHdr.cy;
  ciControls[CTRLIDX_RADCHECK].rclSource.xLeft=0;
  ciControls[CTRLIDX_RADCHECK].rclSource.xRight=allBMPs[CTRLIDX_RADCHECK].bmpInfoHdr.cx;

  /* Radiobutton checked selected */
  allBMPs[CTRLIDX_RADCHECKSEL].hbm=GpiLoadBitmap(hps, hSettingsResource, IDBMP_RADCHECKSEL, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_RADCHECKSEL].hbm, &allBMPs[CTRLIDX_RADCHECKSEL].bmpInfoHdr);
  // get bitmap info
  ciControls[CTRLIDX_RADCHECKSEL].bmpInfoHdr2.cbFix = sizeof(ciControls[CTRLIDX_RADCHECKSEL].bmpInfoHdr2);
  GpiQueryBitmapInfoHeader(allBMPs[CTRLIDX_RADCHECKSEL].hbm, &ciControls[CTRLIDX_RADCHECKSEL].bmpInfoHdr2);
  ciControls[CTRLIDX_RADCHECKSEL].hbmSource=allBMPs[CTRLIDX_RADCHECKSEL].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_RADCHECKSEL].rclSource.yBottom=0;
  ciControls[CTRLIDX_RADCHECKSEL].rclSource.yTop=allBMPs[CTRLIDX_RADCHECKSEL].bmpInfoHdr.cy;
  ciControls[CTRLIDX_RADCHECKSEL].rclSource.xLeft=0;
  ciControls[CTRLIDX_RADCHECKSEL].rclSource.xRight=allBMPs[CTRLIDX_RADCHECKSEL].bmpInfoHdr.cx;

  /* Radiobutton unchecked */
  allBMPs[CTRLIDX_RADUNCHECK].hbm=GpiLoadBitmap(hps, hSettingsResource, IDBMP_RADUNCHECK, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_RADUNCHECK].hbm, &allBMPs[CTRLIDX_RADUNCHECK].bmpInfoHdr);
  ciControls[CTRLIDX_RADUNCHECK].bmpInfoHdr2.cbFix = sizeof(ciControls[CTRLIDX_RADUNCHECK].bmpInfoHdr2);
  GpiQueryBitmapInfoHeader(allBMPs[CTRLIDX_RADUNCHECK].hbm, &ciControls[CTRLIDX_RADUNCHECK].bmpInfoHdr2);
  ciControls[CTRLIDX_RADUNCHECK].hbmSource=allBMPs[CTRLIDX_RADUNCHECK].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_RADUNCHECK].rclSource.yBottom=0;
  ciControls[CTRLIDX_RADUNCHECK].rclSource.yTop=allBMPs[CTRLIDX_RADUNCHECK].bmpInfoHdr.cy;
  ciControls[CTRLIDX_RADUNCHECK].rclSource.xLeft=0;
  ciControls[CTRLIDX_RADUNCHECK].rclSource.xRight=allBMPs[CTRLIDX_RADUNCHECK].bmpInfoHdr.cx;

  /* Radiobutton unchecked selected */
  allBMPs[CTRLIDX_RADUNCHECKSEL].hbm=GpiLoadBitmap(hps, hSettingsResource, IDBMP_RADUNCHECKSEL, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_RADUNCHECKSEL].hbm, &allBMPs[CTRLIDX_RADUNCHECKSEL].bmpInfoHdr);
  ciControls[CTRLIDX_RADUNCHECKSEL].bmpInfoHdr2.cbFix = sizeof(ciControls[CTRLIDX_RADUNCHECKSEL].bmpInfoHdr2);
  GpiQueryBitmapInfoHeader(allBMPs[CTRLIDX_RADUNCHECKSEL].hbm, &ciControls[CTRLIDX_RADUNCHECKSEL].bmpInfoHdr2);
  ciControls[CTRLIDX_RADUNCHECKSEL].hbmSource=allBMPs[CTRLIDX_RADUNCHECKSEL].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_RADUNCHECKSEL].rclSource.yBottom=0;
  ciControls[CTRLIDX_RADUNCHECKSEL].rclSource.yTop=allBMPs[CTRLIDX_RADUNCHECK].bmpInfoHdr.cy;
  ciControls[CTRLIDX_RADUNCHECKSEL].rclSource.xLeft=0;
  ciControls[CTRLIDX_RADUNCHECKSEL].rclSource.xRight=allBMPs[CTRLIDX_RADUNCHECK].bmpInfoHdr.cx;

  /* Radiobutton mask */
  allBMPs[CTRLIDX_RADMASK].hbm=GpiLoadBitmap(hps, hSettingsResource, IDBMP_RADMASK, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_RADMASK].hbm, &allBMPs[CTRLIDX_RADMASK].bmpInfoHdr);
  ciControls[CTRLIDX_RADMASK].bmpInfoHdr2.cbFix = sizeof(ciControls[CTRLIDX_RADMASK].bmpInfoHdr2);
  GpiQueryBitmapInfoHeader(allBMPs[CTRLIDX_RADMASK].hbm, &ciControls[CTRLIDX_RADMASK].bmpInfoHdr2);
  ciControls[CTRLIDX_RADMASK].hbmSource=allBMPs[CTRLIDX_RADMASK].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_RADMASK].rclSource.yBottom=0;
  ciControls[CTRLIDX_RADMASK].rclSource.yTop=allBMPs[CTRLIDX_RADMASK].bmpInfoHdr.cy;
  ciControls[CTRLIDX_RADMASK].rclSource.xLeft=0;
  ciControls[CTRLIDX_RADMASK].rclSource.xRight=allBMPs[CTRLIDX_RADMASK].bmpInfoHdr.cx;

  /* Radiobutton mask selected */
  allBMPs[CTRLIDX_RADMASKSEL].hbm=GpiLoadBitmap(hps, hSettingsResource, IDBMP_RADMASKSEL, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_RADMASKSEL].hbm, &allBMPs[CTRLIDX_RADMASKSEL].bmpInfoHdr);
  ciControls[CTRLIDX_RADMASKSEL].bmpInfoHdr2.cbFix = sizeof(ciControls[CTRLIDX_RADMASKSEL].bmpInfoHdr2);
  GpiQueryBitmapInfoHeader(allBMPs[CTRLIDX_RADMASKSEL].hbm, &ciControls[CTRLIDX_RADMASKSEL].bmpInfoHdr2);
  ciControls[CTRLIDX_RADMASKSEL].hbmSource=allBMPs[CTRLIDX_RADMASKSEL].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_RADMASKSEL].rclSource.yBottom=0;
  ciControls[CTRLIDX_RADMASKSEL].rclSource.yTop=allBMPs[CTRLIDX_RADMASKSEL].bmpInfoHdr.cy;
  ciControls[CTRLIDX_RADMASKSEL].rclSource.xLeft=0;
  ciControls[CTRLIDX_RADMASKSEL].rclSource.xRight=allBMPs[CTRLIDX_RADMASKSEL].bmpInfoHdr.cx;

  WinReleasePS(hps);
}

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
    hps=WinBeginPaint(hwnd, NULLHANDLE, &rcl);
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

