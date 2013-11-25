/*
 * pmwvinfo.c (C) Chris Wohlgemuth 2001
 *
 * This helper shows the format of a wavefile in a dialog box
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
#define INCL_DOS
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_OS2MM
#define INCL_MMIOOS2
#define INCL_MCIOS2
#define INCL_REXXSAA
#include <os2.h>
//#include <rexxsaa.h>                   /* needed for RexxStart()     */
#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <string.h>
#include "os2me.h"
#include "pmtst.h"


//#define DEBUG

#define CLASS_NAME "MY_WIN"
#define SLIDERID   200

char chrInstallDir[CCHMAXPATH];
char logName[]="WaveInfo.log";

int numArgs;
char* params[4];

  /* argv[0]: progname
   * argv[1]: installdir of Audio/Data-CD-Creator
   * argv[2]: wavefile
   */

HWND hwndNew;
PFNWP oldProc;

/*
Sliderdata:

USHORT  sliderarmwidth
USHORT  sliderarmheight
USHORT  ptr position in sliderarm

*/


ULONG rexxFunc1(PSZ name, PLONG argc, RXSTRING argv[], PSZ queuename,PRXSTRING retstring, LONG next)
{
  PRXSTRING prx;
char text[200];
  DosBeep(1000, 100);
  DosBeep(2000, 100);
  DosBeep(3000, 100);
  DosBeep(2000, 100);
  DosBeep(1000, 100);
  prx=&argv[0];
  //  prx=(RXSTRING*)(*prx);

    sprintf(text, "%x %x %s %d  ", argc, prx->strlength, prx->strptr,123);
    WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, text, "", 1234, MB_OK|MB_MOVEABLE);
    prx++;
    sprintf(text, "%x %x %s %d  ", argc, prx->strlength, prx->strptr,123);
    WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, text, "", 1234, MB_OK|MB_MOVEABLE);

#if 0
    if(argc==1)
    {

      sprintf(text, "%s %x",argv[0].strptr, atol(argv[0].strptr));
      WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, text, "", 1234, MB_OK|MB_MOVEABLE);
    }
  else
    {
#endif

#if 0
    }
   #endif
  return 0;
}

#define SLIDERARMWITH    0
#define SLIDERARMHEIGHT  2
#define PTRPOSINSLARM    4
#define USRESERVED1      6
#define SLIDERCX         8   /* ULONG */
#define SLIDERCY         12  /* ULONG */
#define SLIDERX          16   /* ULONG */
#define SLIDERY          20  /* ULONG */
#define SLIDERARMPOS     24  /* LONG  */
#define SLDRAGGING       28  /* BOOL  */

#define SLADDBYTES       32

//USHORT usWidth=10;

//USHORT usHeight=10;
//USHORT usPosInSl=0; 
//USHORT usReserved1;
//ULONG  ulSlidercx;
//ULONG  ulSlidercy;
//LONG lPos=0;
//BOOL bDragging=FALSE;

//SWP swpSl;

char text[200];

#if 0
MRESULT EXPENTRY newProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

  switch (msg)
    {      
    case WM_CREATE:
      {
        PCREATESTRUCT pCreate=PVOIDFROMMP(mp2);

        /* Initialize the slider data */
        WinSetWindowUShort(hwnd, SLIDERARMWITH,20);
        WinSetWindowUShort(hwnd, SLIDERARMHEIGHT,10);
        /* Dimensions of slider. The slider active area is smaller than the window to
           allow drawing of active state. */
        WinSetWindowULong(hwnd, SLIDERCX,pCreate->cx-4);
        WinSetWindowULong(hwnd, SLIDERCY,pCreate->cy-4);
        WinSetWindowULong(hwnd, SLIDERX,2);
        WinSetWindowULong(hwnd, SLIDERY,2);
        WinSetWindowULong(hwnd, SLIDERARMPOS, 0);
        WinSetWindowULong(hwnd, SLDRAGGING,FALSE);
        return (MRESULT)0;
      }
    case WM_SIZE:
      WinSetWindowULong(hwnd, SLIDERCX, SHORT1FROMMP(mp2)-4);
      WinSetWindowULong(hwnd, SLIDERCY, SHORT2FROMMP(mp2)-4);
      WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
      return (MRESULT)0;
    case WM_BUTTON1DOWN:
      {
        SHORT x=SHORT1FROMMP( mp1);
        SHORT y=SHORT2FROMMP( mp1);
        LONG  lPos=WinQueryWindowULong(hwnd, SLIDERARMPOS);
        USHORT usWidth=WinQueryWindowUShort(hwnd, SLIDERARMWITH);
#if 0
        sprintf(text,"%d %d %d %d", WinQueryWindowUShort(hwnd, SLIDERY) ,WinQueryWindowUShort(hwnd, SLIDERCY)
,x, y);
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text,
                       "",
                       0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
#endif   
        if(x<lPos+usWidth && y<WinQueryWindowUShort(hwnd, SLIDERY)+WinQueryWindowUShort(hwnd, SLIDERCY)
           && y>WinQueryWindowUShort(hwnd, SLIDERY)) {
          WinSetWindowUShort(hwnd, PTRPOSINSLARM, x-lPos);
        }
        WinSetFocus(HWND_DESKTOP, hwnd);
      break;
      }
    case WM_FOCUSCHANGE:
      {
        HPS hps;
        RECTL rcl;
        POINTL ptl;

        if(SHORT1FROMMP(mp2)) {
          hps=WinGetPS(hwnd);
          WinQueryWindowRect(hwnd, &rcl);        
          GpiSetLineType(hps, LINETYPE_DOT);
          ptl.x=rcl.xLeft;
          ptl.y=rcl.yBottom;
          GpiMove(hps,&ptl);
          ptl.x=rcl.xRight-1;
          GpiLine(hps,&ptl);
          ptl.y=rcl.yTop-1;
          GpiLine(hps,&ptl);
          ptl.x=rcl.xLeft;
          GpiLine(hps,&ptl);
          ptl.y=rcl.yBottom;
          GpiLine(hps,&ptl);
          WinReleasePS(hps);
        }
        else {
          WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
        }

        break;
      }
    case WM_CHAR:
      if(WinQueryFocus(HWND_DESKTOP)==hwnd) {
        /* We have the focus */
        if((SHORT1FROMMP(mp1) & (KC_VIRTUALKEY))==(KC_VIRTUALKEY)) {
          LONG  lPos=WinQueryWindowULong(hwnd, SLIDERARMPOS);
          USHORT usWidth=WinQueryWindowUShort(hwnd, SLIDERARMWITH);
          ULONG ulCx=WinQueryWindowULong(hwnd, SLIDERCX);

          /*(KC_KEYUP|KC_PREVDOWN|KC_VIRTUALKEY)*/
          switch(SHORT2FROMMP(mp2))
            {
            case VK_RIGHT:
              if(SHORT1FROMMP(mp1) & (KC_KEYUP|KC_PREVDOWN)) {
                lPos+=2;
                if(lPos>ulCx-usWidth)
                  lPos=ulCx-usWidth;
                else {
                  WinPostMsg( hwnd,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_RANGEVALUE),
                              MPFROMLONG( (LONG) lPos ));
                  /* Post SLN_CHANGE notification */
                  WinPostMsg( WinQueryWindow(hwnd, QW_PARENT),WM_CONTROL,
                             MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), SLN_CHANGE),
                             MPFROMLONG(WinQueryWindowULong(hwnd, SLIDERARMPOS)));
                }
                WinSetWindowULong(hwnd, SLIDERARMPOS, lPos);
              }
              return (MRESULT)TRUE;
            case VK_LEFT:
              if(SHORT1FROMMP(mp1) & (KC_KEYUP|KC_PREVDOWN)) {
                lPos-=2;
                if(lPos<0) {
                  lPos=0;
                }
                else {
                  WinPostMsg( hwnd,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_RANGEVALUE),
                              MPFROMLONG( (LONG) lPos ));
                  /* Post SLN_CHANGE notification */
                  WinPostMsg(WinQueryWindow(hwnd, QW_PARENT), WM_CONTROL,
                             MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), SLN_CHANGE),
                   MPFROMLONG(WinQueryWindowULong(hwnd, SLIDERARMPOS)));

                }
                WinSetWindowULong(hwnd, SLIDERARMPOS, lPos);
              }
              return (MRESULT)TRUE;
            default:
              break;
            }
        }
      }
      break;
    case WM_BUTTON1MOTIONSTART:
      {
        SHORT x=SHORT1FROMMP( mp1);
        SHORT y=SHORT2FROMMP( mp1);
        LONG  lPos=WinQueryWindowULong(hwnd, SLIDERARMPOS);
        USHORT usWidth=WinQueryWindowUShort(hwnd, SLIDERARMWITH);

        if(x<lPos+usWidth && y<WinQueryWindowUShort(hwnd, SLIDERY)+WinQueryWindowUShort(hwnd, SLIDERCY)
           && y>WinQueryWindowUShort(hwnd, SLIDERY)) {
          WinSetWindowULong(hwnd, SLDRAGGING, TRUE);
          WinSetCapture(HWND_DESKTOP, hwnd);
        }
        break;
      }
    case WM_BUTTON1MOTIONEND:
      if(WinQueryWindowULong(hwnd, SLDRAGGING)) {
        WinSetWindowULong(hwnd, SLDRAGGING,FALSE);
        WinSetCapture(HWND_DESKTOP, NULLHANDLE);
        /* Post SLN_CHANGE notification */
        WinPostMsg(WinQueryWindow(hwnd, QW_PARENT),WM_CONTROL, MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), SLN_CHANGE),
                   MPFROMLONG(WinQueryWindowULong(hwnd, SLIDERARMPOS)));
      }
      break;
    case SLM_SETSLIDERINFO:
      switch(SHORT1FROMMP(mp1))
        {
          case SMA_SLIDERARMPOSITION:
            /*  SMA_RANGEVALUE only for now !! */
            if(SHORT2FROMMP(mp1)==SMA_RANGEVALUE) {
              WinSetWindowULong(hwnd, SLIDERARMPOS, LONGFROMMP(mp2));
              WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
              return (MRESULT)0;
            }
            break;
        default:
          break;
        }
      break;
    case WM_MOUSEMOVE:
      if(WinQueryWindowULong(hwnd, SLDRAGGING)) {
        HPS hps;
        RECTL rcl, rcl2, rcl3;
        LONG lTemp;
        SHORT x=SHORT1FROMMP(mp1);
        LONG  lPos=WinQueryWindowULong(hwnd, SLIDERARMPOS);
        USHORT usWidth=WinQueryWindowUShort(hwnd, SLIDERARMWITH);

        lTemp=lPos;
     
        rcl.xLeft=WinQueryWindowULong(hwnd, SLIDERX);
        rcl.yBottom=WinQueryWindowULong(hwnd, SLIDERY);
        rcl.xRight=rcl.xLeft+WinQueryWindowULong(hwnd, SLIDERCX);
        rcl.yTop=rcl.yBottom+WinQueryWindowULong(hwnd, SLIDERCY);

        rcl2=rcl3=rcl;
        rcl.xLeft=x-WinQueryWindowUShort(hwnd, PTRPOSINSLARM);
        if(rcl.xLeft<rcl2.xLeft)/* Make sure we stop at the left border */
          rcl.xLeft=rcl2.xLeft;

        rcl.xRight=rcl.xLeft+usWidth;
        if(rcl.xRight>rcl2.xRight)
          {/* Make sure we stop at the right border */
            rcl.xRight=rcl2.xRight;
            rcl.xLeft=rcl.xRight-usWidth;
          }
        lPos=rcl.xLeft-WinQueryWindowULong(hwnd, SLIDERX);/* Save position zero based */
        WinSetWindowULong(hwnd, SLIDERARMPOS, lPos);
        if(lPos!=lTemp) {
          hps=WinGetPS(hwnd);
          /* Paint Background not necessary here */
          /* Shaft */
          /* Left part */
          rcl3.xRight=rcl.xLeft;
          WinFillRect(hps, &rcl3, CLR_GREEN);
          /* Right part */
          rcl3.xRight=rcl2.xRight;
          rcl3.xLeft=rcl.xRight;
          WinFillRect(hps, &rcl3, CLR_WHITE);

          /* Paint Slider */
          WinFillRect(hps,&rcl, CLR_BLUE);
          WinDrawBorder(hps, &rcl, 2, 2, 0, 0 ,0x0400);
          
          WinReleasePS(hps);
          /* Post SLN_SLIDERTRACK notification */
          WinPostMsg(WinQueryWindow(hwnd, QW_PARENT),WM_CONTROL, MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), SLN_SLIDERTRACK),
                     MPFROMLONG(lPos));
        }
      }
      break;
    case WM_PAINT:
      {
        HPS hps, hps2;
        RECTL rcl, rcl2, rcl3;
        POINTL ptl;
        LONG  lPos=WinQueryWindowULong(hwnd, SLIDERARMPOS);
        USHORT usWidth=WinQueryWindowUShort(hwnd, SLIDERARMWITH);

        WinQueryWindowRect(hwnd, &rcl);

        /* Shaft */
        rcl2.xLeft=WinQueryWindowULong(hwnd, SLIDERX);
        rcl2.yBottom=WinQueryWindowULong(hwnd, SLIDERY);
        rcl2.xRight=rcl2.xLeft+WinQueryWindowULong(hwnd, SLIDERCX)-1;
        rcl2.yTop=rcl2.yBottom+WinQueryWindowULong(hwnd, SLIDERCY)-1;

        /* Background */
        hps2=WinGetPS(hwnd);
        GpiExcludeClipRectangle(hps2,&rcl2);
        WinFillRect(hps2, &rcl, CLR_PALEGRAY);
        WinReleasePS(hps2);
        rcl2.yTop+=1;
        rcl2.xRight+=1;

        hps=WinBeginPaint(hwnd, NULLHANDLE, NULLHANDLE);
        /* Focus */
        if(WinQueryFocus(HWND_DESKTOP)==hwnd) {
          GpiSetLineType(hps, LINETYPE_DOT);
          ptl.x=rcl.xLeft;
          ptl.y=rcl.yBottom;
          GpiMove(hps,&ptl);
          ptl.x=rcl.xRight-1;
          GpiLine(hps,&ptl);
          ptl.y=rcl.yTop-1;
          GpiLine(hps,&ptl);
          ptl.x=rcl.xLeft;
          GpiLine(hps,&ptl);
          ptl.y=rcl.yBottom;
          GpiLine(hps,&ptl);
        }

        rcl3=rcl=rcl2;

        /* Arm pos */        
        rcl2.xLeft+=lPos;
        /* Arm size */        
        rcl2.xRight=rcl2.xLeft+usWidth;
        
        /* Shaft */
        /* Left part */
        rcl3.xRight=rcl2.xLeft;
        WinFillRect(hps, &rcl3, CLR_GREEN);

        /* Right part */
        rcl3.xRight=rcl.xRight;
        rcl3.xLeft=rcl2.xRight;
        WinFillRect(hps, &rcl3, CLR_WHITE);

        WinFillRect(hps,&rcl2, CLR_BLUE);        
        WinDrawBorder(hps, &rcl2, 2, 2, 0, 0 ,0x0400);

        WinEndPaint(hps);
        return (MRESULT)0;
      }
      break;
    default:
      break;
    }

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}
#endif










#define USERSLIDER 1

MRESULT EXPENTRY newProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

  switch (msg)
    {      
    case WM_CREATE:
      {
        PCREATESTRUCT pCreate=PVOIDFROMMP(mp2);

        /* Initialize the slider data */
        WinSetWindowUShort(hwnd, SLIDERARMWITH,20);
        WinSetWindowUShort(hwnd, SLIDERARMHEIGHT,10);
        /* Dimensions of slider. The slider active area is smaller than the window to
           allow drawing of active state. */
        WinSetWindowULong(hwnd, SLIDERCX,pCreate->cx-4);
        WinSetWindowULong(hwnd, SLIDERCY,pCreate->cy-4);
        WinSetWindowULong(hwnd, SLIDERX,2);
        WinSetWindowULong(hwnd, SLIDERY,2);
        WinSetWindowULong(hwnd, SLIDERARMPOS, 0);
        WinSetWindowULong(hwnd, SLDRAGGING,FALSE);
        return (MRESULT)0;
      }
    case WM_SIZE:
      WinSetWindowULong(hwnd, SLIDERCX, SHORT1FROMMP(mp2)-4);
      WinSetWindowULong(hwnd, SLIDERCY, SHORT2FROMMP(mp2)-4);
      WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
      return (MRESULT)0;
    case WM_BUTTON1DOWN:
      {
        SHORT x=SHORT1FROMMP( mp1);
        SHORT y=SHORT2FROMMP( mp1);
        LONG  lPos=WinQueryWindowULong(hwnd, SLIDERARMPOS);
        USHORT usWidth=WinQueryWindowUShort(hwnd, SLIDERARMWITH);

        if(x<lPos+usWidth && y<WinQueryWindowUShort(hwnd, SLIDERY)+WinQueryWindowUShort(hwnd, SLIDERCY)
           && y>WinQueryWindowUShort(hwnd, SLIDERY)) {
          WinSetWindowUShort(hwnd, PTRPOSINSLARM, x-lPos);
        }
        WinSetFocus(HWND_DESKTOP, hwnd);
      break;
      }
    case WM_FOCUSCHANGE:
      {
        HPS hps;
        RECTL rcl;
        POINTL ptl;

        if(SHORT1FROMMP(mp2)) {
          hps=WinGetPS(hwnd);
          WinQueryWindowRect(hwnd, &rcl);        
          GpiSetLineType(hps, LINETYPE_DOT);
          ptl.x=rcl.xLeft;
          ptl.y=rcl.yBottom;
          GpiMove(hps,&ptl);
          ptl.x=rcl.xRight-1;
          GpiLine(hps,&ptl);
          ptl.y=rcl.yTop-1;
          GpiLine(hps,&ptl);
          ptl.x=rcl.xLeft;
          GpiLine(hps,&ptl);
          ptl.y=rcl.yBottom;
          GpiLine(hps,&ptl);
          WinReleasePS(hps);
        }
        else {
          WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
        }

        break;
      }
    case WM_CHAR:
      if(WinQueryFocus(HWND_DESKTOP)==hwnd) {
        /* We have the focus */
        if((SHORT1FROMMP(mp1) & (KC_VIRTUALKEY))==(KC_VIRTUALKEY)) {
          LONG  lPos=WinQueryWindowULong(hwnd, SLIDERARMPOS);
          USHORT usWidth=WinQueryWindowUShort(hwnd, SLIDERARMWITH);
          ULONG ulCx=WinQueryWindowULong(hwnd, SLIDERCX);
#if 0
          FILE* file;
          file=fopen("d:\\md.log","a");
          fprintf(file,"0x%x 0x%x \n",SHORT1FROMMP(mp1),SHORT2FROMMP(mp1) );
          fclose(file);
#endif
          /*(KC_KEYUP|KC_PREVDOWN|KC_VIRTUALKEY)*/
          switch(SHORT2FROMMP(mp2))
            {
            case VK_RIGHT:

              if(SHORT1FROMMP(mp1) & (KC_KEYUP|KC_PREVDOWN)) {
                lPos+=2;
                if(lPos>ulCx-usWidth)
                  lPos=ulCx-usWidth;
                else {
                  WinPostMsg( hwnd,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_RANGEVALUE),
                              MPFROMLONG( (LONG) lPos ));
                  if(SHORT1FROMMP(mp1) & KC_LONEKEY)
                    /* Post SLN_CHANGE notification */
                    WinPostMsg( WinQueryWindow(hwnd, QW_PARENT),WM_CONTROL,
                                MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), SLN_CHANGE),
                                MPFROMLONG(lPos));
                  else
                    /* Post SLN_SLIDERTRACK notification */
                    WinPostMsg(WinQueryWindow(hwnd, QW_PARENT),WM_CONTROL,
                               MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), SLN_SLIDERTRACK),
                               MPFROMLONG(lPos));
                  
                }
                WinSetWindowULong(hwnd, SLIDERARMPOS, lPos);
              }
              return (MRESULT)TRUE;
            case VK_LEFT:
              if(SHORT1FROMMP(mp1) & (KC_KEYUP|KC_PREVDOWN)) {
                lPos-=2;
                if(lPos<0) {
                  lPos=0;
                }
                else {
                  WinPostMsg( hwnd,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_RANGEVALUE),
                              MPFROMLONG( (LONG) lPos ));
                  /* Post SLN_CHANGE notification */
                  WinPostMsg(WinQueryWindow(hwnd, QW_PARENT), WM_CONTROL,
                             MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), SLN_CHANGE),
                   MPFROMLONG(WinQueryWindowULong(hwnd, SLIDERARMPOS)));

                }
                WinSetWindowULong(hwnd, SLIDERARMPOS, lPos);
              }
              return (MRESULT)TRUE;
            default:
              break;
            }
        }
      }
      break;
    case WM_BUTTON1MOTIONSTART:
      {
        SHORT x=SHORT1FROMMP( mp1);
        SHORT y=SHORT2FROMMP( mp1);
        LONG  lPos=WinQueryWindowULong(hwnd, SLIDERARMPOS);
        USHORT usWidth=WinQueryWindowUShort(hwnd, SLIDERARMWITH);

        if(x<lPos+usWidth && y<WinQueryWindowUShort(hwnd, SLIDERY)+WinQueryWindowUShort(hwnd, SLIDERCY)
           && y>WinQueryWindowUShort(hwnd, SLIDERY)) {
          WinSetWindowULong(hwnd, SLDRAGGING, TRUE);
          WinSetCapture(HWND_DESKTOP, hwnd);
        }
        break;
      }
    case WM_BUTTON1MOTIONEND:
      if(WinQueryWindowULong(hwnd, SLDRAGGING)) {
        WinSetWindowULong(hwnd, SLDRAGGING,FALSE);
        WinSetCapture(HWND_DESKTOP, NULLHANDLE);
        /* Post SLN_CHANGE notification */
        WinPostMsg(WinQueryWindow(hwnd, QW_PARENT),WM_CONTROL, MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), SLN_CHANGE),
                   MPFROMLONG(WinQueryWindowULong(hwnd, SLIDERARMPOS)));
      }
      break;
    case SLM_SETSLIDERINFO:
      switch(SHORT1FROMMP(mp1))
        {
          case SMA_SLIDERARMPOSITION:
            /*  SMA_RANGEVALUE only for now !! */
            if(SHORT2FROMMP(mp1)==SMA_RANGEVALUE) {
              WinSetWindowULong(hwnd, SLIDERARMPOS, LONGFROMMP(mp2));
              WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
              return (MRESULT)0;
            }
            break;
        default:
          break;
        }
      break;
    case WM_MOUSEMOVE:
      if(WinQueryWindowULong(hwnd, SLDRAGGING)) {
        HPS hps;
        RECTL rcl, rcl2, rcl3;
        LONG lTemp;
        SHORT x=SHORT1FROMMP(mp1);
        LONG  lPos=WinQueryWindowULong(hwnd, SLIDERARMPOS);
        USHORT usWidth=WinQueryWindowUShort(hwnd, SLIDERARMWITH);
       
        lTemp=lPos;
     
        rcl.xLeft=WinQueryWindowULong(hwnd, SLIDERX);
        rcl.yBottom=WinQueryWindowULong(hwnd, SLIDERY);
        rcl.xRight=rcl.xLeft+WinQueryWindowULong(hwnd, SLIDERCX);
        rcl.yTop=rcl.yBottom+WinQueryWindowULong(hwnd, SLIDERCY);

        rcl2=rcl3=rcl;
        rcl.xLeft=x-WinQueryWindowUShort(hwnd, PTRPOSINSLARM);
        if(rcl.xLeft<rcl2.xLeft)/* Make sure we stop at the left border */
          rcl.xLeft=rcl2.xLeft;

        rcl.xRight=rcl.xLeft+usWidth;
        if(rcl.xRight>rcl2.xRight)
          {/* Make sure we stop at the right border */
            rcl.xRight=rcl2.xRight;
            rcl.xLeft=rcl.xRight-usWidth;
          }
        lPos=rcl.xLeft-WinQueryWindowULong(hwnd, SLIDERX);/* Save position zero based */
        WinSetWindowULong(hwnd, SLIDERARMPOS, lPos);
        if(lPos!=lTemp) {
          BOOL rc;

          hps=WinGetPS(hwnd);
          /* Paint Background not necessary here */

          /* Shaft */
          /* Left part */
          rcl3.xRight=rcl.xLeft;
          rc=FALSE;
          if(USERSLIDER) {
            OWNERITEM oi={0};
            oi.hwnd=hwnd;
            oi.hps=hps;
            oi.fsState=SLS_OWNERDRAW;/* More to come */
            oi.rclItem=rcl3;
            oi.idItem=SDA_SLIDERSHAFT;
            rc=(BOOL)WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), WM_DRAWITEM, MPFROMSHORT(WinQueryWindowUShort(hwnd, QWS_ID)),
                                MPFROMP(&oi) );
          }
          if(!rc)
            WinFillRect(hps, &rcl3, CLR_GREEN);

          /* Right part */
          rcl3.xRight=rcl2.xRight;
          rcl3.xLeft=rcl.xRight;
          rc=FALSE;
          if(USERSLIDER) {
            OWNERITEM oi={0};
            oi.hwnd=hwnd;
            oi.hps=hps;
            oi.fsState=SLS_OWNERDRAW;/* More to come */
            oi.rclItem=rcl3;
            oi.idItem=SDA_SLIDERSHAFT;
            rc=(BOOL)WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), WM_DRAWITEM, MPFROMSHORT(WinQueryWindowUShort(hwnd, QWS_ID)),
                                MPFROMP(&oi) );
          }
          if(!rc)
            WinFillRect(hps, &rcl3, CLR_WHITE);

          /* Paint Slider */
          rc=FALSE;
          if(USERSLIDER) {
            OWNERITEM oi={0};
            oi.hwnd=hwnd;
            oi.hps=hps;
            oi.fsState=SLS_OWNERDRAW;/* More to come */
            oi.rclItem=rcl;
            oi.idItem=SDA_SLIDERARM;
            rc=(BOOL)WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), WM_DRAWITEM, MPFROMSHORT(WinQueryWindowUShort(hwnd, QWS_ID)),
                                MPFROMP(&oi) );
          }
          if(!rc)
            {
              WinFillRect(hps,&rcl, CLR_BLUE);
              WinDrawBorder(hps, &rcl, 2, 2, 0, 0 ,0x0400);
            }

          WinReleasePS(hps);

          /* Post SLN_SLIDERTRACK notification */
          WinPostMsg(WinQueryWindow(hwnd, QW_PARENT),WM_CONTROL, MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), SLN_SLIDERTRACK),
                     MPFROMLONG(lPos));
        }
      }
      break;
    case WM_PAINT:
      {
        HPS hps, hps2;
        RECTL rcl, rcl2, rcl3;
        POINTL ptl;
        LONG  lPos=WinQueryWindowULong(hwnd, SLIDERARMPOS);
        USHORT usWidth=WinQueryWindowUShort(hwnd, SLIDERARMWITH);
        BOOL rc;

        WinQueryWindowRect(hwnd, &rcl);

        /* Shaft */
        rcl2.xLeft=WinQueryWindowULong(hwnd, SLIDERX);
        rcl2.yBottom=WinQueryWindowULong(hwnd, SLIDERY);
        rcl2.xRight=rcl2.xLeft+WinQueryWindowULong(hwnd, SLIDERCX)-1;
        rcl2.yTop=rcl2.yBottom+WinQueryWindowULong(hwnd, SLIDERCY)-1;

        /* Background */
        hps2=WinGetPS(hwnd);
        GpiExcludeClipRectangle(hps2,&rcl2);
        WinFillRect(hps2, &rcl, CLR_PALEGRAY);
        WinReleasePS(hps2);
        rcl2.yTop+=1;
        rcl2.xRight+=1;

        hps=WinBeginPaint(hwnd, NULLHANDLE, NULLHANDLE);
        /* Focus */
        if(WinQueryFocus(HWND_DESKTOP)==hwnd) {
          GpiSetLineType(hps, LINETYPE_DOT);
          ptl.x=rcl.xLeft;
          ptl.y=rcl.yBottom;
          GpiMove(hps,&ptl);
          ptl.x=rcl.xRight-1;
          GpiLine(hps,&ptl);
          ptl.y=rcl.yTop-1;
          GpiLine(hps,&ptl);
          ptl.x=rcl.xLeft;
          GpiLine(hps,&ptl);
          ptl.y=rcl.yBottom;
          GpiLine(hps,&ptl);
        }

        rcl3=rcl=rcl2;

        /* Arm pos */        
        rcl2.xLeft+=lPos;
        /* Arm size */        
        rcl2.xRight=rcl2.xLeft+usWidth;
        
        /* Shaft */
        /* Left part */
        rcl3.xRight=rcl2.xLeft;
        rc=FALSE;
        if(USERSLIDER) {
          OWNERITEM oi={0};
          oi.hwnd=hwnd;
          oi.hps=hps;
          oi.fsState=SLS_OWNERDRAW;/* More to come */
          oi.rclItem=rcl3;
          oi.idItem=SDA_SLIDERSHAFT;
          rc=(BOOL)WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), WM_DRAWITEM, MPFROMSHORT(WinQueryWindowUShort(hwnd, QWS_ID)),
                    MPFROMP(&oi) );
        }
        if(!rc)
          WinFillRect(hps, &rcl3, CLR_GREEN);

        /* Right part */
        rcl3.xRight=rcl.xRight;
        rcl3.xLeft=rcl2.xRight;
        rc=FALSE;
        if(USERSLIDER) {
          OWNERITEM oi={0};
          oi.hwnd=hwnd;
          oi.hps=hps;
          oi.fsState=SLS_OWNERDRAW;/* More to come */
          oi.rclItem=rcl3;
          oi.idItem=SDA_SLIDERSHAFT;
          rc=(BOOL)WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), WM_DRAWITEM, MPFROMSHORT(WinQueryWindowUShort(hwnd, QWS_ID)),
                              MPFROMP(&oi) );
        }
        if(!rc)
          WinFillRect(hps, &rcl3, CLR_WHITE);

        rc=FALSE;
        if(USERSLIDER) {
          OWNERITEM oi={0};
          oi.hwnd=hwnd;
          oi.hps=hps;
          oi.fsState=SLS_OWNERDRAW;/* More to come */
          oi.rclItem=rcl2;
          oi.idItem=SDA_SLIDERARM;
          rc=(BOOL)WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), WM_DRAWITEM, MPFROMSHORT(WinQueryWindowUShort(hwnd, QWS_ID)),
                              MPFROMP(&oi) );
        }
        if(!rc)
          {
            WinFillRect(hps,&rcl2, CLR_BLUE);        
            WinDrawBorder(hps, &rcl2, 2, 2, 0, 0 ,0x0400);
          }

        WinEndPaint(hps);
        return (MRESULT)0;
      }
      break;
    default:
      break;
    }

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


MRESULT drawSlider(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2, USHORT id)
{
  POWNERITEM poi;

  RECTL rcl;
  POINTL ptl= {0};

  poi=(POWNERITEM)PVOIDFROMMP(mp2);

  switch(poi->idItem)
    {
    case SDA_SLIDERARM:

      WinFillRect(poi->hps,&poi->rclItem, CLR_RED);
      WinDrawBorder(poi->hps,&poi->rclItem, 2, 2, 0, 0 ,0x0400);

      return (MRESULT) TRUE;
      break;
    case SDA_BACKGROUND:
      {
        HPS hps;

        WinFillRect(poi->hps,&poi->rclItem, CLR_WHITE);

        return (MRESULT) TRUE;
      }
    case SDA_SLIDERSHAFT:
      {
        HPS hps;
        RECTL rcl;
        MRESULT mr;

        WinFillRect(poi->hps,&poi->rclItem, CLR_BLUE);

        return (MRESULT)TRUE;
      }
    default:
      return (MRESULT)TRUE;
    }

  /* Done */
  return (MRESULT)TRUE;
}


/* This Proc handles the on-the-fly data CD writing */
MRESULT EXPENTRY waveinfoStatusDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH*2 +10];
  char title[CCHMAXPATH];
  SWCNTRL swctl;
  PID pid;
  int a;


  switch (msg)
    {
    case WM_PAINT:
      {
        HPS hps;
        RECTL rcl, rclSource;
        POINTL ptl;
        LONG lTemp;
        BOOL bUseCustomPainting=TRUE;
        LONG lWidthX;
        LONG lWidthY;
        LONG  lColor;
        ULONG attrFound;


        if(bUseCustomPainting) {
          //          if(allBMPs[CTRLIDX_BG].hbm) {
          if(1) {
            hps=WinBeginPaint(hwnd, NULLHANDLE, &rcl);
            rclSource.xLeft=0;
            rclSource.yBottom=0;
#if 0
            rclSource.yTop=allBMPs[CTRLIDX_BG].bmpInfoHdr.cy;
            rclSource.xRight=allBMPs[CTRLIDX_BG].bmpInfoHdr.cx;
            lTemp=rcl.xLeft/rclSource.xRight;
            ptl.x=lTemp*rclSource.xRight;
            lTemp=rcl.yBottom/rclSource.yTop;
            lTemp*=rclSource.yTop;
#endif

            //    rcl.yBottom+=4;
            rclSource=rcl;
            rcl.yTop+=4;
            WinFillRect(hps, &rclSource, CLR_RED);

            /* Border */
            lWidthX=WinQuerySysValue(HWND_DESKTOP, SV_CXBORDER);
            lWidthY=WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER);

            GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
#if 0
        if ( (WinQueryPresParam(hwnd, PP_BACKGROUNDCOLOR, 0, &attrFound, sizeof(attrFound),
                                                &lColor, QPF_PURERGBCOLOR)) == 0 )
           lColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0);
#endif

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

#if 0
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
#endif
            WinEndPaint(hps);
            return (MRESULT)0;
          }
        }
        break;
      }

    case WM_DRAWITEM:
      switch(SHORT1FROMMP(mp1))
        {
        case SLIDERID:
          return drawSlider(hwnd, msg, mp1, mp2, SHORT1FROMMP(mp1));
        default:
          break;
        }              
      break;
    case WM_INITDLG:
      /* Add switch entry */
      memset(&swctl,0,sizeof(swctl));
      WinQueryWindowProcess(hwnd,&pid,NULL);
      swctl.hwnd=hwnd;
      swctl.uchVisibility=SWL_VISIBLE;
      swctl.idProcess=pid;
      swctl.bProgType=PROG_DEFAULT;
      swctl.fbJump=SWL_JUMPABLE;
      WinAddSwitchEntry(&swctl);

#if 0      
      WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, "",
                     "",
                     0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
#endif
      //      oldProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_TEXT), newProc);
      hwndNew=WinCreateWindow(hwnd, CLASS_NAME, "Title", WS_VISIBLE |WS_TABSTOP, 10,50, SLIDERID, 40, 
                              hwnd, HWND_TOP, SLIDERID, NULLHANDLE, NULLHANDLE);
      WinShowWindow(hwnd, TRUE);
      //      WinQueryWindowPos(hwndNew,&swpSl);
      return (MRESULT) TRUE;
      /* WM_APPTERMINATENOTIFY messages are sent from the helper programs e.g. format checker. */
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_CONTROL:
      {
        switch(SHORT2FROMMP(mp1))
          {
          case SLN_SLIDERTRACK:
            {
              switch(SHORT1FROMMP(mp1))
                {
                case SLIDERID:
                  DosBeep(5000, 10);
                  break;
                default:
                  break;
                }/* switch */
              return (MRESULT) 0;
              break;
            }
          case SLN_CHANGE:
            {
              switch(SHORT1FROMMP(mp1))
                {
                case SLIDERID:
                  DosBeep(500, 100);
                  break;
                default:
                  break;
                }/* switch */
              return (MRESULT)TRUE;
            }
          default:
            break;
          }/* switch */
        break;
      }/* WM_CONTROL */
      
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case DID_OK:
          /* User pressed the OK button */
          WinPostMsg(hwnd,WM_CLOSE,0,0);
          break;
        case IDPB_RIGHT:
          WinPostMsg( WinWindowFromID(hwnd, SLIDERID),
                      SLM_SETSLIDERINFO,
                      MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_RANGEVALUE),
                      MPFROMLONG( (LONG) 70 ));
          
          break;
        case IDPB_LEFT:
          WinPostMsg( WinWindowFromID(hwnd, SLIDERID),
                      SLM_SETSLIDERINFO,
                      MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_RANGEVALUE),
                      MPFROMLONG( (LONG) 20 ));
          
          break;
        case IDPB_SIZE:
          {
            SWP swp;
              RXSTRING arg[2];                       /* argument string for REXX  */
              RXSTRING rexxretval;                /* return value from REXX    */
              APIRET   rc;                        /* return code from REXX     */
              SHORT    rexxrc = 0;                /* return code from function */
              char chrThis[20];
              char chrHwnd[20];

              char    *str = "These words will be swapped"; /* text to swap   */

#if 0
              RexxRegisterFunctionExe("CWRXFunc1", (PFN)rexxFunc1);
              

              sprintf(chrThis, "%d",123);
              sprintf(chrHwnd, "%d", WinQueryWindow(hwnd,QW_PARENT));

              /* By setting the strlength of the output RXSTRING to zero, we   */
              /* force the interpreter to allocate memory and return it to us. */
              /* We could provide a buffer for the interpreter to use instead. */
              rexxretval.strlength = 0L;          /* initialize return to empty*/
              
              MAKERXSTRING(arg[0], chrHwnd, strlen(chrHwnd));/* create input argument     */
              MAKERXSTRING(arg[1], chrThis, strlen(chrThis));/* create input argument     */

              /* Here we call the interpreter.  We don't really need to use    */
              /* all the casts in this call; they just help illustrate         */
              /* the data types used.                                          */
              rc=RexxStart((LONG)      2,             /* number of arguments   */
                           (PRXSTRING)  &arg,          /* array of arguments    */
                           (PSZ)        "G:\\Projects_working\\mmclasses-0.3.0\\testfunc.cwr",/* name of REXX file     */
                           (PRXSTRING)  0,             /* No INSTORE used       */
                           (PSZ)        "CWRXX",         /* Command env. name     */
                           (LONG)       RXSUBROUTINE,  /* Code for how invoked  */
                           (PRXSYSEXIT) 0,             /* No EXITs on this call */
                           (PSHORT)     &rexxrc,       /* Rexx program output   */
                           (PRXSTRING)  &rexxretval ); /* Rexx program output   */

              sprintf(text,"rc: %d, function return code: %d, %s ", 
                      rc, (int) rexxrc, rexxretval.strptr);

              WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, text, "", 1234, MB_OK|MB_MOVEABLE);
              
              DosFreeMem(rexxretval.strptr);          /* Release storage       
                                                         given to us by REXX.  */
              
#endif

              //            WinQueryWindowPos(hwndNew,&swp);
              //    WinSetWindowPos(hwndNew, NULLHANDLE, 0, 0, swp.cx-30, swp.cy+10,SWP_SIZE);
          break;
          }
        default:
          break;
        }
      return (MRESULT) FALSE;
    default:
      break;
    }/* switch */
  
  return WinDefDlgProc( hwnd, msg, mp1, mp2);
}

int main (int argc, char *argv[])
{
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  short a;
  HWND hwndClient;
  ULONG result;
  ULONG rc;

  /* Create a copy of the args */
  /* argv[0]: progname
     argv[1]: installdir
     argv[2]: 0: query wave, 1: query mp3
     argv[3]: wavefile
   */
  numArgs=argc;
  
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {  
      /* Save installation directory */

      /* Get our ressource dll */  

      WinRegisterClass(hab, CLASS_NAME, newProc,0, SLADDBYTES);
      if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, waveinfoStatusDialogProc, NULLHANDLE, IDDL_PMTST, 0) == DID_ERROR )
        {
          
          WinDestroyMsgQueue( hmq );
          WinTerminate( hab );
          DosBeep(100,600);
          return( 1 );
        }
      
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}









