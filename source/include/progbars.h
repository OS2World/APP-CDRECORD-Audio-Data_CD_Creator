
/*
 * progbars.h:
 *      header file for progbars.c, which
 *      contains progress bar helper functions.
 *
 *      These can turn any static control into a working progress bar.
 *
 *      Function prefixes (new with V0.81):
 *      --  pbar*   progress bar helper functions
 *
 *      Required #include's before including this header:
 *      --  OS2.H with INCL_WIN.
 *
 *      Copyright (C) 1997-99 Ulrich M”ller.
 *      This file is part of the XFolder source package.
 *      XFolder is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XFolder main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef PROGBARS_HEADER_INCLUDED
    #define PROGBARS_HEADER_INCLUDED

    /* PROGRESSBARDATA:
       structure for progress bar data, saved at QWL_USER window ulong */
    typedef struct _PROGRESSBARDATA
    {
        ULONG      ulNow, ulMax, ulPaintX, ulOldPaintX;
        ULONG      ulAttr;
        PFNWP      OldStaticProc;
        RECTL      rtlBar;
    } PROGRESSBARDATA, *PPROGRESSBARDATA;

    #define WM_UPDATEPROGRESSBAR    WM_USER+1000

    /* status bar style attributes */
    #define PBA_NOPERCENTAGE        0x0000
    #define PBA_ALIGNLEFT           0x0001
    #define PBA_ALIGNRIGHT          0x0002
    #define PBA_ALIGNCENTER         0x0003
    #define PBA_PERCENTFLAGS        0x0003
    #define PBA_BUTTONSTYLE         0x0010

    BOOL pbarProgressBarFromStatic(HWND hwndStatic, ULONG ulAttr);

#endif

#if __cplusplus
}
#endif

