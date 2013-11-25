/**********************************************************************
*                                                                     *
*       SysIni              -- Reads and/or updates entries in .INI   *
*                              files.                                 *
*                                                                     *
**********************************************************************/
/* Include files */

#define  INCL_PM
#define  INCL_WINSHELLDATA
#define  INCL_WINSYS
#define  INCL_VIO
#define  INCL_DOS
#define  INCL_DOSFILEMGR
#define  INCL_DOSMEMMGR
#define  INCL_DOSMISC
#define  INCL_DOSNMPIPES
#define  INCL_ERRORS
#define  INCL_REXXSAA
#define  _DLL
#define  _MT
#include <os2.h>
#include <rexxsaa.h>
#include <memory.h>
#include <malloc.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

#define QWL_PERCENT 4 /* The location in the window words to store the percent value */
#define QWL_TEXTPTR 8 /* The ptr to our text */

/*********************************************************************/
/*  Declare all exported functions as REXX functions.                */
/*********************************************************************/

RexxFunctionHandler DRCtrlDropFuncs;
RexxFunctionHandler DRCtrlRegister;
RexxFunctionHandler DRCtrlLoadFuncs;
RexxFunctionHandler CreatorGetResString;
RexxFunctionHandler CreatorMessageBox;
RexxFunctionHandler CreatorGetHWND;

/*********************************************************************/
/*  Various definitions used by various functions.                   */
/*********************************************************************/

#define  MAX_DIGITS     9          /* maximum digits in numeric arg  */
#define  MAX            256        /* temporary buffer length        */
#define  IBUF_LEN       4096       /* Input buffer length            */
#define  AllocFlag      PAG_COMMIT | PAG_WRITE  /* for DosAllocMem   */

/*********************************************************************/
/* Structures used throughout REXXUTIL.C                              */
/*********************************************************************/

/*********************************************************************/
/* RxStemData                                                        */
/*   Structure which describes as generic                            */
/*   stem variable.                                                  */
/*********************************************************************/

typedef struct RxStemData {
    SHVBLOCK shvb;                     /* Request block for RxVar    */
    CHAR ibuf[IBUF_LEN];               /* Input buffer               */
    CHAR varname[MAX];                 /* Buffer for the variable    */
                                       /* name                       */
    CHAR stemname[MAX];                /* Buffer for the variable    */
                                       /* name                       */
    ULONG stemlen;                     /* Length of stem.            */
    ULONG vlen;                        /* Length of variable value   */
    ULONG j;                           /* Temp counter               */
    ULONG tlong;                       /* Temp counter               */
    ULONG count;                       /* Number of elements         */
                                       /* processed                  */
} RXSTEMDATA;

/*********************************************************************/
/* RxFncTable                                                        */
/*   Array of names of the REXXUTIL functions.                       */
/*   This list is used for registration and deregistration.          */
/*********************************************************************/

static PSZ  RxFncTable[] =
   {
      "DRCtrlRegister",
      "DRCtrlLoadFuncs",
      "DRCtrlDropFuncs",
      "CreatorGetResString",
      "CreatorMessageBox",
      "CreatorGetHWND",
   };

/*********************************************************************/
/* Numeric Error Return Strings                                      */
/*********************************************************************/

#define  NO_UTIL_ERROR    "0"          /* No error whatsoever        */
#define  ERROR_NOMEM      "2"          /* Insufficient memory        */
#define  ERROR_FILEOPEN   "3"          /* Error opening text file    */

/*********************************************************************/
/* Alpha Numeric Return Strings                                      */
/*********************************************************************/

#define  ERROR_RETSTR   "ERROR:"
#define  EMPTY_RETSTR   ""

/*********************************************************************/
/* Numeric Return calls                                              */
/*********************************************************************/

#define  INVALID_ROUTINE 40            /* Raise Rexx error           */
#define  VALID_ROUTINE    0            /* Successful completion      */

/*********************************************************************/
/* Some useful macros                                                */
/*********************************************************************/

#define BUILDRXSTRING(t, s) { \
  strcpy((t)->strptr,(s));\
  (t)->strlength = strlen((s)); \
}

/*
 * PaintProgress:
 *      this does the actual painting. It is called
 *      both by WM_PAINT and by WM_UPDATEPROGRESSBAR
 *      with different HPS's then.
 */

VOID PaintProgress(int iPercent, HWND hwnd, HPS hps)
{
    POINTL  ptl1, ptlText, aptlText[TXTBOX_COUNT];
    RECTL   rcl, rcl2;
    BOOL    bVertical=FALSE;
    CHAR    szPercent[10] = "";
    CHAR  * ptrChr=NULL;

    WinQueryWindowRect(hwnd, &rcl);
    /* Check if it's a vertical percent bar */
    if(rcl.xRight<rcl.yTop)
      bVertical=TRUE;
    
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
#if 0
        sprintf(szPercent, "%d %%", iPercent );
        GpiQueryTextBox(hps, strlen(szPercent), szPercent,
                        TXTBOX_COUNT, (PPOINTL)&aptlText);
#endif
        GpiQueryTextBox(hps, strlen(ptrChr), ptrChr,
                        TXTBOX_COUNT, (PPOINTL)&aptlText);
   
        ptlText.x = rcl.xLeft +
          (   (   (rcl.xRight-rcl.xLeft)
                  - (aptlText[TXTBOX_BOTTOMRIGHT].x-aptlText[TXTBOX_BOTTOMLEFT].x)
                  )
              / 2);
        ptlText.y = 3 + rcl.yBottom +
          (   (   (rcl.yTop-rcl.yBottom)
                  - (aptlText[TXTBOX_TOPLEFT].y-aptlText[TXTBOX_BOTTOMLEFT].y)
                  )
              / 2);
      }

    if(!bVertical) {
      rcl2.xLeft = rcl.xLeft;
      rcl2.xRight = (rcl.xRight-rcl.xLeft)*iPercent/100; 
      rcl2.yBottom = rcl.yBottom;
      rcl2.yTop = rcl.yTop-1;
      rcl.xLeft=rcl2.xRight+1;
    }
    else {
      rcl2.xLeft = rcl.xLeft;
      rcl2.xRight = rcl.xRight-1;
      rcl2.yBottom = rcl.yBottom;
      rcl2.yTop = (rcl.yTop-rcl.yBottom)*iPercent/100; 
      rcl.yBottom=rcl2.yTop+1;
    }
    /* Background */
    WinFillRect(hps, &rcl,
                WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0));

    /* Percentbar */
    if ((rcl2.xRight > rcl2.xLeft && !bVertical)||(rcl2.yTop > rcl2.yBottom && bVertical)) {
      ULONG ulBG;

      /* Find color. This color is the background color set within DrDialog */
      if(!WinQueryPresParam(hwnd, PP_BACKGROUNDCOLOR, PP_BACKGROUNDCOLORINDEX, NULL, sizeof(ulBG),
                        &ulBG, QPF_ID2COLORINDEX|QPF_NOINHERIT ))
        ulBG=0x002020ff;
      GpiSetColor(hps,ulBG );

      ptl1.x = rcl2.xLeft+1;
      ptl1.y = rcl2.yBottom+1;
      GpiMove(hps, &ptl1);
      ptl1.x = rcl2.xRight-1;
      ptl1.y = rcl2.yTop-1;
      GpiBox(hps, DRO_FILL | DRO_OUTLINE,
             &ptl1,
             0,
             0);
      rcl2.yBottom+=1;
      rcl2.xLeft+=1;
      WinDrawBorder(hps, &rcl2, 1,1,0,0, 0x400);
    }

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
#if 0
        GpiCharString(hps, strlen(szPercent), szPercent);
#endif
        GpiCharString(hps, strlen(ptrChr), ptrChr);
      }
}


/*
 *@@ fnwpProgressBar:
 *      this is the window procedure for the progress bar control,
 *      which is a static rectangle control subclassed by
 *      pbarProgressBarFromStatic.
 *
 *      We need to capture WM_PAINT to draw the progress bar according
 *      to the current progress, and we also update the static text field
 *      (percentage) next to it.
 *
 *      This also evaluates the WM_UPDATEPROGRESSBAR message.
 */

MRESULT EXPENTRY fnwpProgressBar(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
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
        char *ptr;
        char *ptr2;

        iPercent=atol(pwp->pszText);
        if(iPercent>100)
          iPercent=100;
        if(iPercent<0)
          iPercent=0;

        if((ptr=strchr(pwp->pszText, '#'))!=NULLHANDLE) {
          if((ptr2=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
            free(ptr2);
          WinSetWindowPtr(hwnd,QWL_TEXTPTR, NULLHANDLE);
          if(*(ptr++)!=0) {
            /* There's additional text to print */
            if((ptr2=malloc(strlen(ptr)))!=NULLHANDLE) {
              strcpy(ptr2,ptr);
              WinSetWindowPtr(hwnd,QWL_TEXTPTR,ptr2);
            }
          }
        }
        
        /*        mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);*/
        WinSetWindowULong(hwnd, QWL_PERCENT,iPercent);
        WinQueryWindowRect(hwnd, &rcl);
        WinInvalidateRect(hwnd, &rcl, FALSE);
        /*        return mrc;*/
      }
      break;
    }
  case WM_DESTROY:
    {
      char *ptrText;
      if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
        free(ptrText);
      break;
    }
  case WM_PRESPARAMCHANGED:
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
      PaintProgress(WinQueryWindowULong(hwnd,QWL_PERCENT), hwnd, hps);
      WinEndPaint(hps);
    return (MRESULT) FALSE;
    }
  
  default:
    break;
  }
  /*  return (MRESULT) FALSE;*/
  mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
  return (mrc);
}

static HWND _getHWNDFromID(HWND hwndParent, USHORT id)
{
  PTIB ptib;
  PPIB ppip;
  /*  ULONG rc;*/
  HENUM henum;
  PID pid;
  TID tid;
  HWND hwnd=NULLHANDLE;


  if(NO_ERROR!=DosGetInfoBlocks(&ptib, &ppip))
      return NULLHANDLE;

  henum=WinBeginEnumWindows(hwndParent);
  while((hwnd=WinGetNextWindow(henum))!=NULLHANDLE)
    {
      if(WinQueryWindowUShort(hwnd, QWS_ID)==id) {
        WinQueryWindowProcess(hwnd,&pid, &tid);
        /* There's only one control with a given ID and a given parent in a DRDialog process */
        if(pid==ppip->pib_ulpid)
          break;
      }
    }
  WinEndEnumWindows(henum);

  return hwnd;
}

/*************************************************************************
* Function:  CreatorGetHWND                                              *
*                                                                        *
*            Gets the HWND of a dialog or control from the ID.           *
*                                                                        *
*                                                                        *
* Syntax:    call DRCtrlGetHWND hwnd, ID                                 *
*                                                                        *
* Params:    hwnd, ID, 1/0                                               *
*                                                                        *
* Return:    0: failure, 1: success                                      *
*                                                                        *
* Remarks:   If hwnd==0 it's assumed the dialog is a child of the        *
*            desktop.                                                    *
*                                                                        *
*************************************************************************/
ULONG CreatorGetHWND(CHAR *name, ULONG numargs, RXSTRING args[],
                     CHAR *queuename, RXSTRING *retstr)
{
  HWND hwndParent;

    retstr->strlength = 0;               /* set return value           */
  /* check arguments            */
  if (numargs != 2)
    return INVALID_ROUTINE;

  hwndParent=atol(args[0].strptr);
  if(!hwndParent)
    hwndParent=HWND_DESKTOP;
  
  sprintf(retstr->strptr, "%d", _getHWNDFromID(hwndParent, (USHORT)atol(args[1].strptr)));
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;
}

ULONG CreatorMessageBox(CHAR *name, ULONG numargs, RXSTRING args[],
                    CHAR *queuename, RXSTRING *retstr)
{
  HWND hwnd=HWND_DESKTOP;
  ULONG flStyle=0;

  /*
    args[0]:  text
    args[1]:  title
    args[2]:  button
    args[3]:  icon
    args[4]:  misc style
    args[5]:  parent/owner hwnd (optional)
   */

  retstr->strlength = 0;               /* set return value           */

  /* check arguments */
  if (numargs <5 && numargs >6)
    return INVALID_ROUTINE;

  if(numargs==6)
    hwnd=atol(args[5].strptr);

  /* Which button to use */
  if(!stricmp(args[2].strptr, "OK"))
    flStyle|=MB_OK;
  else if(!stricmp(args[2].strptr, "OKCANCEL"))
    flStyle|=MB_OKCANCEL;
  else if(!stricmp(args[2].strptr, "CANCEL"))
    flStyle|=MB_CANCEL;
  else if(!stricmp(args[2].strptr, "ENTER"))
    flStyle|=MB_ENTER;
  else if(!stricmp(args[2].strptr, "ENTERCANCEL"))
    flStyle|=MB_ENTERCANCEL;
  else if(!stricmp(args[2].strptr, "RETRYCANCEL"))
    flStyle|=MB_RETRYCANCEL;
  else if(!stricmp(args[2].strptr, "ABORTRETRYIGNORE"))
    flStyle|=MB_ABORTRETRYIGNORE;
  else if(!stricmp(args[2].strptr, "YESNO"))
    flStyle|=MB_YESNO;
  else if(!stricmp(args[2].strptr, "YESNOCANCEL"))
    flStyle|=MB_YESNOCANCEL;
  else
    return INVALID_ROUTINE;

  /* Which icon to use */
  if(!stricmp(args[3].strptr, "NONE"))
    flStyle|=MB_NOICON;
  else if(!stricmp(args[3].strptr, "HAND"))
    flStyle|=MB_ICONHAND;
  else if(!stricmp(args[3].strptr, "QUESTION"))
    flStyle|=MB_ICONQUESTION;
  else if(!stricmp(args[3].strptr, "EXCLAMATION"))
    flStyle|=MB_ICONEXCLAMATION;
  else if(!stricmp(args[3].strptr, "ASTERISK"))
    flStyle|=MB_ICONASTERISK;
  else if(!stricmp(args[3].strptr, "INFORMATION"))
    flStyle|=MB_INFORMATION;
  else if(!stricmp(args[3].strptr, "QUERY"))
    flStyle|=MB_QUERY;
  else if(!stricmp(args[3].strptr, "WARNING"))
    flStyle|=MB_WARNING;
  else if(!stricmp(args[3].strptr, "ERROR"))
    flStyle|=MB_ERROR;
  else
    return INVALID_ROUTINE;

  /* Additional styles */
  if(!stricmp(args[4].strptr, "MOVEABLE"))
    flStyle|=MB_MOVEABLE;
  else if(!stricmp(args[4].strptr, "SYSTEMMODAL"))
    flStyle|=MB_SYSTEMMODAL;
  else
    return INVALID_ROUTINE;

  sprintf(retstr->strptr, "%d", WinMessageBox(HWND_DESKTOP, hwnd, args[0].strptr, args[1].strptr,1234, flStyle));
  retstr->strlength=strlen(retstr->strptr);
  return VALID_ROUTINE;
}

ULONG CreatorGetResString(CHAR *name, ULONG numargs, RXSTRING args[],
                          CHAR *queuename, RXSTRING *retstr)
{
  ULONG ulString;  
  ULONG rc;
  HMODULE hModule;
  char error[CCHMAXPATH];

  /*
    args[0]:  DLL path
    args[1]:  ID of string
   */

  retstr->strlength = 0;               /* set return value           */

  /* check arguments */
  if (numargs !=2)
    return INVALID_ROUTINE;

  ulString=atol(args[1].strptr);
  if(ulString>0xffff)
    return INVALID_ROUTINE;

  strcpy(retstr->strptr, "ERROR:");
  retstr->strlength=strlen(retstr->strptr);
  /* Load Module */
  rc=DosLoadModule(error,sizeof(error), args[0].strptr, &hModule);
  if(rc!=NO_ERROR)
    return VALID_ROUTINE;;

  /* Now load the string */
  if(!WinLoadString(WinQueryAnchorBlock(HWND_DESKTOP),hModule, ulString, 256, retstr->strptr))
    return VALID_ROUTINE;
  
  retstr->strlength=strlen(retstr->strptr);
  DosFreeModule(hModule);
  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  DRCtrlRegister                                              *
*                                                                        *
* Syntax:    call DRCtrlRegister                                         *
*                                                                        *
*            Registers the DrDialog window classes in the process        *
*                                                                        *
*************************************************************************/
ULONG DRCtrlRegister(CHAR *name, ULONG numargs, RXSTRING args[],
                     CHAR *queuename, RXSTRING *retstr)
{

  WinRegisterClass(WinQueryAnchorBlock(HWND_DESKTOP),"DRD_PERCENTBAR",fnwpProgressBar,0L,12);

  return VALID_ROUTINE;                /* no error on call           */
}

ULONG DRCtrlDropFuncs(CHAR *name, ULONG numargs, RXSTRING args[],
                          CHAR *queuename, RXSTRING *retstr)
{
  INT     entries;                     /* Num of entries             */
  INT     j;                           /* Counter                    */

  if (numargs != 0)                    /* no arguments for this      */
    return INVALID_ROUTINE;            /* raise an error             */

  retstr->strlength = 0;               /* return a null string result*/

  entries = sizeof(RxFncTable)/sizeof(PSZ);

  for (j = 0; j < entries; j++) {

    printf(RxFncTable[j]);
    RexxDeregisterFunction(RxFncTable[j]);
  }
  return VALID_ROUTINE;                /* no error on call           */
}


/*************************************************************************
* Function:  SysLoadFuncs                                                *
*                                                                        *
* Syntax:    call SysLoadFuncs [option]                                  *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/

ULONG DRCtrlLoadFuncs(CHAR *name, ULONG numargs, RXSTRING args[],
                           CHAR *queuename, RXSTRING *retstr)
{
  INT    entries;                      /* Num of entries             */
  INT    j;                            /* Counter                    */

  retstr->strlength = 0;               /* set return value           */
                                       /* check arguments            */

  if (numargs > 0)
    return INVALID_ROUTINE;

  entries = sizeof(RxFncTable)/sizeof(PSZ);

  for (j = 0; j < entries; j++) {
 
    printf(RxFncTable[j]);
    printf(": ");
    RexxRegisterFunctionDll(RxFncTable[j],
          "DRUSRCTL", RxFncTable[j]);
  }

  return VALID_ROUTINE;
}


