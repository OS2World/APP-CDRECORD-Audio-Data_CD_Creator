#include <stdlib.h>
#include "progfolder.h"
#include "progfolder.hh"

MRESULT EXPENTRY separatorProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg)
    {
    case WM_PAINT:
      {
        MRESULT mr;
        HPS hps;
        RECTL rcl;

        WinQueryWindowRect(hwnd, &rcl);
        if(rcl.yTop-rcl.yBottom<2)
          break;

        mr=WinDefWindowProc(hwnd, msg, mp1, mp2);
        if((hps=WinGetPS(hwnd))!=NULLHANDLE) {
          POINTL ptl;

          ptl.y=(rcl.yTop-rcl.yBottom)/2;
          ptl.x=0;
          GpiMove(hps, &ptl);
          ptl.x=rcl.xRight;
          GpiSetColor(hps, CLR_DARKGRAY);
          GpiLine(hps,&ptl);
          GpiSetColor(hps, CLR_WHITE);
          ptl.y-=1;
          GpiMove(hps, &ptl);
          ptl.x=0;
          GpiLine(hps,&ptl);
          WinReleasePS(hps);
        }
        return mr;
        break;
      }
    default:
      break;
    }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}



BOOL cwRegisterSeparatorWindowClass(void)
{
  CLASSINFO ci;
 
  if(WinQueryClassInfo(WinQueryAnchorBlock(HWND_DESKTOP),
                       WC_STATIC,
                       &ci))
    {
      if(WinRegisterClass(WinQueryAnchorBlock(HWND_DESKTOP),
                          (PSZ)"CW_SEPARATOR",
                          separatorProc,
                          (ci.flClassStyle) &~(CS_PUBLIC | CS_PARENTCLIP),
                          0))
        return TRUE;
    }
  return FALSE;
}
