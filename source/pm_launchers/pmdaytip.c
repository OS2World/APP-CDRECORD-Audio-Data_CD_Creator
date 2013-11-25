/*
 * pmdaytip.c (C) Chris Wohlgemuth 2001-2002
 *
 * This program shows the tip of the day
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
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "audiofolder.h"

#include "pmdaytip.h"

//#define DEBUG

#define ACTION_STARTPROGRAM  1
#define ACTION_OPENOBJECT    2
#define ACTION_OPENWEBPAGE   3

typedef struct{
  int   iAction;
  PFNWP pfnwp;
  char  chrParameter[1024];
  char  chrProgram[CCHMAXPATH];
}ITEMDATA;

#define HINT_TIMER_ID 1
#define MY_DIALOGBORDER 10
#define MY_CBSPACE     12 /* space between checkbox and bottom clickable item */
#define MY_ITEMSPACE   5
#define SHADOWDELTA    10
#define SPACE_BTN_ITEM   24 /* Space between chckbox and bottom item */
/* The max number of choices */
#define MAXITEMS       5

char chrInstallDir[CCHMAXPATH];
char chrTutorialPath[CCHMAXPATH]="";
char logName[]="TipOfDay.log";

/* See helpers.c */

extern SWP swpWindow;
extern BOOL bTipsEnabled;
int numArgs;
char* params[10];
HMODULE RESSOURCEHANDLE=0;
HMODULE CLASSDLLHANDLE=0;

HAB  hab;
ULONG ulTimer=0;
HINI hini;
HPOINTER hptrHand;
RECTL rclDesktop;
ITEMDATA itemData[MAXITEMS];
HWND hwndFocus;

int iNumTips;
int iCurrentTip;
char chrCurrentTip[10]="1";

RGB rgbForeground={0,0,0};
RGB rgbBackground={180,255,255};
char chrFontName[CCHMAXPATH];/* Font for fly over help */

  /* argv[0]: progname
   * argv[1]: installdir of Audio/Data-CD-Creator
   * argv[2]: 
   */

void pmUsage();
void HlpSendCommandToObject(char* chrObject, char* command);
void HlpOpenWebPage(char* chrUrl);

void paintURL(HPS hps, HWND hwnd, LONG lColor)
{
  RECTL rectl;
  char chrURL[500];

  WinQueryWindowText(hwnd, sizeof(chrURL),chrURL);  
  WinQueryWindowRect(hwnd, &rectl);
  WinDrawText(hps, -1L, chrURL,  &rectl, lColor , CLR_BACKGROUND, /*DT_TEXTATTRS|*/ DT_LEFT|DT_UNDERSCORE |DT_WORDBREAK );
  return;
}

MRESULT EXPENTRY urlProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  ITEMDATA * item;
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
      item=(ITEMDATA*)WinQueryWindowULong(hwnd,QWL_USER);
      if(item) {
        switch(item->iAction) {
        case ACTION_STARTPROGRAM:
          if(strlen(item->chrParameter)) {
            memset(&startData,0,sizeof(startData));
            startData.Length=sizeof(startData);
            startData.Related=SSF_RELATED_INDEPENDENT;
            startData.FgBg=SSF_FGBG_FORE;
            startData.TraceOpt=SSF_TRACEOPT_NONE;
            startData.PgmTitle="";
            startData.PgmName=item->chrProgram;
            startData.InheritOpt=SSF_INHERTOPT_SHELL;
            startData.SessionType=SSF_TYPE_DEFAULT;
            startData.PgmControl=SSF_CONTROL_VISIBLE;
            startData.InitXPos=30;
            startData.InitYPos=30;
            startData.InitXSize=500;
            startData.InitYSize=400;
            startData.ObjectBuffer=chrLoadError;
            startData.ObjectBuffLen=(ULONG)sizeof(chrLoadError);
            startData.PgmInputs=item->chrParameter;
            rc=DosStartSession(&startData,&ulSessionID,&pid);
            /* close daytip dialog */
            //  WinPostMsg(WinQueryWindow(hwnd, QW_PARENT),WM_CLOSE,0,0);
          }
          break;
        case ACTION_OPENOBJECT:
#if 0
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,item->chrProgram ,
                       item->chrParameter,
                       0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
#endif
          HlpSendCommandToObject(item->chrProgram, item->chrParameter);
          /* close daytip dialog */
          //    WinPostMsg(WinQueryWindow(hwnd, QW_PARENT),WM_CLOSE,0,0);
          break;
        case ACTION_OPENWEBPAGE:
          HlpOpenWebPage(item->chrParameter);
          break;
        default:
          break;
        }/* switch() */
      }
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
  item=(ITEMDATA*)WinQueryWindowULong(hwnd,QWL_USER);
  if(item)
    if(item->pfnwp)
      return item->pfnwp( hwnd, msg, mp1, mp2 );

  return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

void buildItemCommand(ITEMDATA * item)
{
  char chrCommand[1024];
  char * chrPtr;

  if(strstr(item->chrParameter,"TUTORIAL:")) {
    if((chrPtr=strchr(item->chrParameter,':'))!=NULLHANDLE) {
     chrPtr++;
     snprintf(chrCommand, sizeof(item->chrParameter) ,"\"%s\" \"%s\"",chrTutorialPath, chrPtr);
     strncpy(item->chrParameter,chrCommand,sizeof(item->chrParameter));
     strcpy(item->chrProgram, "view.exe");
     item->iAction=ACTION_STARTPROGRAM;
    }
    else
      item->chrParameter[0]=0;
  }
  else if(strstr(item->chrParameter,"OBJECT:")) {
    if((chrPtr=strchr(item->chrParameter,':'))!=NULLHANDLE) {
      chrPtr++;

      strncpy(item->chrProgram, chrPtr, CCHMAXPATH);
      strncpy(item->chrParameter,"OPEN=DEFAULT;", sizeof(item->chrParameter));
      item->iAction=ACTION_OPENOBJECT;
    }
  }
  else if(strstr(item->chrParameter,"WEB:")) {
    if((chrPtr=strchr(item->chrParameter,':'))!=NULLHANDLE) {
      chrPtr++;
      strncpy(item->chrParameter, chrPtr, sizeof(item->chrParameter));
      item->iAction=ACTION_OPENWEBPAGE;
    }
  }
  else
    item->chrParameter[0]=0;
}

/***********************************************/
/*                                             */
/* This function calculates the vertical size  */
/* of a static textcontrol with the wordbreak  */
/* style.                                      */
/*                                             */
/* Input:                                      */
/*       hwnd - handle of the  text control    */
/*       ptrText - the text (terminated with 0)*/
/*       rectl - size of the control           */
/*                                             */
/* Returns:                                    */
/*       needed vertical size                  */
/*                                             */
/***********************************************/
ULONG _calculateVerticalControlSize(HWND hwnd, char * ptrText, RECTL rectl, ULONG *ulCx)
{
  ULONG ulWinTextLen, ulLen;
  HPS hps;
  RECTL rcl;
  char *tempPtr;
  LONG lCy;
  LONG yTop;
  int iDrawn;
  int iTotalDrawn;
  POINTL aptl[TXTBOX_COUNT];
  //char text[200];
  hps=WinGetPS(hwnd);
  ulLen = ulWinTextLen=(ULONG)strlen(ptrText); /* Query text length */

  rcl=rectl;

  tempPtr=ptrText;
  /* Get the height of one line of text */
  if(ulLen>512)
    ulLen=512;
  GpiQueryTextBox(hps, ulLen, ptrText, TXTBOX_COUNT, aptl);

  /* Save the line height */
  lCy=aptl[TXTBOX_TOPRIGHT].y-aptl[TXTBOX_BOTTOMRIGHT].y;
  yTop=rectl.yTop;
  /* Calculate the line position for every line of text. No drawing takes place. */
  for(iTotalDrawn=0, yTop=0/*-lCy/4*/ ;iTotalDrawn!=ulWinTextLen; yTop+=20) {
    iDrawn=WinDrawText(hps, ulWinTextLen-iTotalDrawn,
                       ptrText+iTotalDrawn, &rectl, 0, 0, DT_QUERYEXTENT|DT_TEXTATTRS|DT_WORDBREAK|DT_TOP|DT_LEFT);

    if(iDrawn) {
#if 0
      if(iDrawn>25)
        yTop+=20;
      else if(strlen(ptrText+iTotalDrawn) <= 25)
        {
          yTop+=20;
          break;
        }
#endif
      iTotalDrawn+=iDrawn;
      //  DosBeep(1000, 500);
      //   DosSleep(500);
      //      sprintf(text, "%d, %d, %d", iTotalDrawn, iDrawn, strlen(ptrText));
      //  WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, text, "", 1234, MB_OK|MB_MOVEABLE);
    }
    else
      break; /* Nothing to draw */
  }
  WinReleasePS(hps);

  *ulCx=aptl[TXTBOX_TOPRIGHT].x-aptl[TXTBOX_TOPLEFT].x;
  //  sprintf(text, "%d", yTop);
  //  WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, text, "", 1234, MB_OK|MB_MOVEABLE);
  if(yTop>140)
    yTop=140;
  return (yTop);
  /* Calculate the new height */
  //return (rectl.yTop-yTop);
  return (rcl.yTop-yTop);
}

#define MAXCHARS 511
ULONG calculateVerticalControlSize(HWND hwnd, char * ptrText, RECTL rcl, ULONG *ulCx)
{
  POINTL aptl[TXTBOX_COUNT];
  POINTL ptl[2];
  POINTL charPtl[MAXCHARS+1];
  int iLen;
  RECTL rectl;
  int a;
  LONG lSaveY;
  HPS hps;
  LONG lCy, lCyGes;

  hps=WinGetPS(hwnd);
  WinQueryWindowRect(hwnd, &rectl);
  iLen=strlen(ptrText);
  /* Always use only the first 15 chars to speed up */
  a=(iLen>15 ? 15 : iLen);
  GpiQueryTextBox(hps, a, ptrText, TXTBOX_COUNT, aptl);
  /* The height of the string */
  lCy=aptl[TXTBOX_TOPLEFT].y-aptl[TXTBOX_BOTTOMLEFT].y;

  a=0;
  lCyGes=0;

  while(a < iLen) {/* While chars in string */ 
    int iLen2,b;
    LONG lTemp;

    iLen2=strlen(&ptrText[a]);
    b=(iLen2>MAXCHARS ? MAXCHARS: iLen2);
    /* Get positions of chars */
    GpiQueryCharStringPos(hps, 0, iLen2,  &ptrText[a], 0, charPtl);
      
    lTemp=charPtl[b].x/rectl.xRight*lCy;
    lCyGes+=lTemp;
    if(charPtl[b].x%rectl.xRight)
      lCyGes+=20;
    //      lCyGes+=lCy+2;
    a+=b;
  }

  WinReleasePS(hps);
  lCyGes+=lCy;  
  return lCyGes;
}

#if 0
#define MAXCHARS 511
ULONG calculateVerticalControlSize(HWND hwnd, char * ptrText, RECTL rcl, ULONG *ulCx)
{
  POINTL aptl[TXTBOX_COUNT];
  POINTL ptl[2];
  POINTL charPtl[MAXCHARS+1];
  int iLen;
  RECTL rectl;
  int a;
  LONG lSaveY;
  HPS hps;
  LONG lCy;
char text[100];

  hps=WinGetPS(hwnd);
  WinQueryWindowRect(hwnd, &rectl);

  /* Always use only the first 15 chars to speed up */
  a=(iLen>15 ? 15 : iLen);
  GpiQueryTextBox(hps, a, ptrText, TXTBOX_COUNT, aptl);
  /* The height of the string */
  lCy=aptl[TXTBOX_TOPLEFT].y-aptl[TXTBOX_BOTTOMLEFT].y;

  a=0;
  while(a <= iLen) {/* While chars in string */ 
    int b, c;
    iLen=strlen(ptrText[a]);
    b=(iLen>MAXCHARS ? MAXCHARS: iLen);
    /* Get positions of chars */
    GpiQueryCharStringPos(hps, 0, iLen,  &ptrText[a], 0, charPtl);
    /* Check if any passes right border */
    for(c=1 ; c<=b ; c++) {
      if(charPtl[c].x>rectl.xRight)
        {
          char chrTemp;
          char *chrPtr;
          /* The string goes past the right border. Find space. */
          chrTemp=*ptrText[a+c-1];
          ptrText[a+c-1]=0;
          if((chrPtr=strrchr(ptrText[a],' '))!=NULLHANDLE)
            {
              /* space found */
              /* Restore char */
              ptrText[a+c-1]=chrTemp;

            }
          else
            {
              /* No space -> don't split this word. Find next space instead. */
              /* Restore char */
              ptrText[a+c-1]=chrTemp;
              if((chrPtr=strrchr(ptrText[a],' '))!=NULLHANDLE)
                {
                  /* Next space found */
                }
              else
                {
                  /* No more spaces -> last word of string */

                }
            }
        }
    }
  }
    /* Get draw position of next char */
    GpiQueryCharStringPos(hps, 0, 1,  &ptrText[a], 0, ptl);
    sprintf(text, "%d, %d", ptl[0].x, ptl[1].x);
    WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, text, "", 1234, MB_OK|MB_MOVEABLE);

    if(ptl[1].x  > rectl.xRight) {/* ptl[1] is position of next char */ 
      /* Next line */
      ptl[0].x=0;
      lSaveY-=aptl[TXTBOX_TOPLEFT].y+2;
      ptl[0].y=lSaveY;
      GpiMove(hps, ptl);
    }
    
    /* Draw char */
    //    GpiCharStringPos(hps,NULLHANDLE,CHS_UNDERSCORE,1,&chrURL[a],0);
    a++;
  };
  WinReleasePS(hps);  
}
#endif

BOOL rebuildDialogFromIniData(HINI hini, HWND hwnd, char * chrApp, char *chrApp2)
{
  char *memPtr;
  ULONG ulSize;
  int a, numChoices;
  RECTL allRcl[2+5+1];/* Hinttext, checkbox, 5 clickable items and the pushbutton row*/
  int numControls=2;
  ULONG ulYGroupBox, ulY=0;
  HPS hps;
  POINTL aptlPoints[TXTBOX_COUNT];
  ULONG ulCy, ulCx, ulCyGroupBox;
  char text[CCHMAXPATH];
  HWND hwndCtrl;
  SWP swp, swp2;

  if(PrfQueryProfileSize(hini, chrApp,"hinttext",&ulSize)) {
    if(ulSize==0)
      chrApp=chrApp2;
  }
  else
    chrApp=chrApp2;

  /* Push button size and position */
  /* This control is already added to numControls */
  hwndCtrl=WinWindowFromID(hwnd, DID_OK );
  WinQueryWindowPos(hwndCtrl,&swp);
  ulY=swp.y+swp.cy+MY_CBSPACE;

  /* Disable checkbutton */
/*  WinCheckButton(hwnd, IDCB_HINTENABLE, 0); */

  /* Checkbox size and position */
  /* This control is already added to numControls */
  hwndCtrl=WinWindowFromID(hwnd, IDCB_HINTENABLE);
  /* Calculate text size in pixel */
  WinQueryWindowText(hwndCtrl,CCHMAXPATH, text);
  hps=WinBeginPaint(hwndCtrl, (HPS)NULL,(PRECTL)NULL);
  GpiQueryTextBox(hps,strlen(text),text,TXTBOX_COUNT,aptlPoints);
  WinEndPaint(hps);
  ulCy=aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y;
  ulCx=aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x;
  ulY+=SPACE_BTN_ITEM+ulCy;
  ulYGroupBox=ulY-SPACE_BTN_ITEM/2;

  /* Build the clickable items */
  numChoices=PrfQueryProfileInt(hini, chrApp, "numchoices", 0);
  if(numChoices>MAXITEMS)
    numChoices=MAXITEMS;
  /* First hide all choices to have a clean start */
  for(a=MAXITEMS;a>0;a--)
    {
      WinShowWindow(WinWindowFromID(hwnd, IDST_ITEM0 +a-1),FALSE);
    }
  for(a=numChoices;a>0;a--)
    {
      char chrText[10];
      PFNWP pfnwp;

      /* prepare the clickable items */      
      sprintf(chrText,"choice%d", a);
      if(PrfQueryProfileSize(hini, chrApp, chrText,&ulSize)) {
        if(ulSize) {
          if((memPtr=(char*)malloc(ulSize))!=NULLHANDLE)
            {
              RECTL rcl;
              SWP swp;

              /* Get the text for the item */
              PrfQueryProfileString(hini, chrApp, chrText, "", memPtr, ulSize);
              /* The item handle */
              hwndCtrl=WinWindowFromID(hwnd, IDST_ITEM0 +a-1);
              WinSendMsg(hwndCtrl,EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
              /* Query bounding RECT */
              WinQueryWindowRect(hwndCtrl, &rcl);

              /* Calculate text size in pixel */
              hps=WinBeginPaint(hwndCtrl,(HPS)NULL,(PRECTL)NULL);
              GpiQueryTextBox(hps,strlen(memPtr),memPtr,TXTBOX_COUNT,aptlPoints);
              WinEndPaint(hps);
              ulCy=aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y;
              ulCx=aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x;

              WinQueryWindowPos(hwndCtrl,&swp);
              WinSetWindowPos(hwndCtrl,NULLHANDLE, swp.x , ulY,
                              ulCx+20, ulCy
                              ,SWP_SIZE|SWP_MOVE );

              /* Add the size of the item for dialog calculating */
              ulY+=ulCy+MY_ITEMSPACE;

              /* Set the text */
              WinSetWindowText(hwndCtrl,memPtr);
              /* Free memory */
              free(memPtr);
              WinShowWindow( hwndCtrl, TRUE);
              /* Build command */
              sprintf(chrText,"open%d", a);
              PrfQueryProfileString(hini, chrApp, chrText, "", &itemData[a-1].chrParameter, sizeof(itemData[a-1].chrParameter));
              buildItemCommand(&itemData[a-1]);
              numControls++;
            }
        }/* end of if(ulSize) */
      }/* end of if(PrfQueryProfileSize(hini, chrApp, chrText,&ulSize)) */
    }/* for() */
  if(numChoices)
    ulY+=MY_ITEMSPACE;

  /* Get hint text */
  if(PrfQueryProfileSize(hini, chrApp,"hinttext",&ulSize)) {
    if((memPtr=(char*)malloc(ulSize))!=NULLHANDLE)
      {
        RECTL rcl;
        ULONG ulCy, ulCx;
        HWND hwndCtrl;
        SWP swp;

        PrfQueryProfileString(hini, chrApp,"hinttext","No hint available", memPtr, ulSize);
        hwndCtrl=WinWindowFromID(hwnd, IDST_MAINTEXT);
        /* Query bounding RECT */
        WinQueryWindowRect(hwndCtrl, &rcl);
        /* Size the text control */
        ulCy=calculateVerticalControlSize(hwndCtrl,  memPtr, rcl, &ulCx);


        WinSetWindowText(hwndCtrl,memPtr);
        WinQueryWindowPos(hwndCtrl,&swp);
        WinSetWindowPos(hwndCtrl,NULLHANDLE, swp.x, ulY,
                        rcl.xRight-rcl.xLeft, ulCy
//                        ,SWP_MOVE);
                        ,SWP_SIZE|SWP_MOVE);
//        ulY+=SPACE_BTN_ITEM+swp.cy;

        ulY+=SPACE_BTN_ITEM+ulCy;
        /* Set the text */


        free(memPtr);
      }
  }
  else 
    WinSetWindowText(WinWindowFromID(hwnd, IDST_MAINTEXT),"No hint available.");

  /* Size and move group box */
  ulCyGroupBox=ulY-ulYGroupBox;
  hwndCtrl=WinWindowFromID(hwnd, IDGB_GROUP );
  WinQueryWindowPos(hwndCtrl,&swp2);
  WinSetWindowPos(hwndCtrl,NULLHANDLE, swp2.x , ulYGroupBox,
                  swp2.cx, ulCyGroupBox
                  ,SWP_SIZE|SWP_MOVE );

  /* Move icon */
#if 0
  hwndCtrl=WinWindowFromID(hwnd, IDST_MAINTEXT );
  WinQueryWindowPos(hwndCtrl,&swp2);
#endif
  hwndCtrl=WinWindowFromID(hwnd, IDPB_ICON );
  WinQueryWindowPos(hwndCtrl,&swp);
  WinSetWindowPos(hwndCtrl,NULLHANDLE, swp.x , ulYGroupBox + (ulCyGroupBox-swp.cy)/2,
                  0, 0
                  , SWP_MOVE );

#if 0
  WinSetWindowPos(hwndCtrl,NULLHANDLE, swp.x , swp2.y + (swp2.cy-swp.cy)/2,
                  0, 0
                  , SWP_MOVE );
#endif
  /* Titlebar size */
  hwndCtrl=WinWindowFromID(hwnd, FID_TITLEBAR );
  WinQueryWindowPos(hwndCtrl,&swp);
  //  ulY+=swp.cy+MY_CBSPACE;
  ulY+=swp.cy;
  /* Size the dialog */
  WinQueryWindowPos(hwnd,&swpWindow);
  WinSetWindowPos(hwnd,NULLHANDLE, 0,0,
                  swpWindow.cx, ulY, SWP_SIZE);
  return TRUE;
}

PFNWP oldButtonProc;
MRESULT EXPENTRY newButtonProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg)
    {
    case WM_BUTTON1DOWN:
    case WM_BUTTON2DOWN:
    case WM_BUTTON3DOWN:
    case WM_BUTTON1CLICK:
    case WM_BUTTON2CLICK:
    case WM_BUTTON3CLICK:
    case WM_BUTTON1DBLCLK:
    case WM_BUTTON2DBLCLK:
    case WM_BUTTON3DBLCLK:
      return (MRESULT) FALSE;
    default:
      break;
    }
  return oldButtonProc( hwnd, msg, mp1, mp2);
}

/* This Proc handles the on-the-fly data CD writing */
MRESULT EXPENTRY hintDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  char text[CCHMAXPATH*2 +10];
  char title[CCHMAXPATH];
  SWCNTRL swctl;
  PID pid;
  int a;
  ULONG ulSize;
  char *memPtr;
  
  switch (msg)
    {      
    case WM_INITDLG:
      {
        POINTL pointl;

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
        sprintf(text,"1: %s, 2: %s, 3: %s 4: %s",params[1],params[2], params[3], params[4]);
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text,
                       params[3],
                       0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
        /* Set colors */
        WinSetPresParam(WinWindowFromID(hwnd,IDST_MAINTEXT),
                        PP_BACKGROUNDCOLOR,sizeof(rgbBackground) ,
                        &rgbBackground );
        WinSetPresParam(WinWindowFromID(hwnd,IDST_MAINTEXT),
                        PP_FOREGROUNDCOLOR,sizeof(rgbForeground) ,
                        &rgbForeground );
        /* Set font */
        WinSetPresParam(WinWindowFromID(hwnd,IDST_MAINTEXT),PP_FONTNAMESIZE,
                        sizeof(chrFontName),
                        chrFontName);
#endif   


        /* Set dialog font to WarpSans for Warp 4 and above */
        if(cwQueryOSRelease()>=40) {
          WinSetPresParam(hwnd,
                          PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                          DEFAULT_DIALOG_FONT );
        }
        oldButtonProc=WinSubclassWindow(WinWindowFromID(hwnd,IDPB_ICON), newButtonProc);

        for(a=MAXITEMS; a>0; a--)
          {
            HWND hwndCtrl;

            /* The item handle */
            hwndCtrl=WinWindowFromID(hwnd, IDST_ITEM0 +a-1);
            /* Subclass control */
            itemData[a-1].pfnwp=WinSubclassWindow(hwndCtrl, urlProc);
            WinSetWindowULong(hwndCtrl,QWL_USER,(ULONG)&itemData[a-1]);
          }
        if(bTipsEnabled)
          WinCheckButton(hwnd, IDCB_HINTENABLE, 0);
        else
          WinCheckButton(hwnd, IDCB_HINTENABLE, 1);

        
        /* Build the dialog */
        //        if(rebuildDialogFromIniData(hini, hwnd, params[3], params[4])) {
        if(rebuildDialogFromIniData(hini, hwnd, chrCurrentTip, chrCurrentTip)) {
          SWP swp;

          WinQueryWindowPos(hwnd, &swp);
          WinSetWindowPos(hwnd,HWND_TOP,(rclDesktop.xRight-swp.cx)/2, (rclDesktop.yTop-swp.cy)/2,
                          0,0,SWP_ZORDER|SWP_ACTIVATE|SWP_MOVE);
          //  WinSetWindowPos(hwnd,HWND_TOP,pointl.x,pointl.y,0,0,SWP_ZORDER|SWP_ACTIVATE|SWP_MOVE);
          WinSetWindowPos(hwnd,HWND_TOP, 0, 0,0,0,SWP_SHOW);
        }
        else
          WinPostMsg(hwnd,WM_CLOSE,0,0);

        return (MRESULT) TRUE;
      }
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_DESTROY:
      break;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
        {
        case DID_OK:
          WinDismissDlg(hwnd,0);
          break;
        case IDPB_PREVIOUS:
          iCurrentTip--;
          if(iCurrentTip<=0)
            iCurrentTip=iNumTips;
          sprintf(chrCurrentTip,"%d",iCurrentTip);
          rebuildDialogFromIniData(hini, hwnd, chrCurrentTip, chrCurrentTip);
          break;
        case IDPB_NEXT:
          iCurrentTip++;
          if(iCurrentTip>iNumTips)
            iCurrentTip=1;
          sprintf(chrCurrentTip,"%d",iCurrentTip);
          rebuildDialogFromIniData(hini, hwnd, chrCurrentTip, chrCurrentTip);
          break;
        default:
          break;
        }
      return (MRESULT) FALSE;
    case WM_CONTROL:
      /* The window controls send WM_CONTROL messages */
      switch(SHORT1FROMMP(mp1))
        {
        case IDCB_HINTENABLE:
          {
            char chrSetup[CCHMAXPATH];
            if(WinQueryButtonCheckstate(hwnd,IDCB_HINTENABLE) & 1)
              sprintf(chrSetup,"%s=0", SETUP_DAYTIPENABLE);
            else 
              sprintf(chrSetup,"%s=1",SETUP_DAYTIPENABLE);
            
            sendCommand(chrSetup);
            break;
          }
        default:
          break;
        } // end switch(SHORT1FROMMP(mp1))
      break;
    default:
      break;
    }/* switch */
  
  return WinDefDlgProc( hwnd, msg, mp1, mp2);
}

int main (int argc, char *argv[])
{
  HMQ  hmq;
  QMSG qmsg;
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  short a;
  HWND hwndClient;
  ULONG result;
  ULONG rc;
  HINI hIni;
  char *chrPtr;

  /* Create a copy of the args */
  /* argv[0]: progname
   */

  params[2]="<CWCREATOR_SETTINGS>";/* This is required for sendCommand() */
  params[3]="1";
  params[4]="1";
  numArgs=argc;

  sprintf(text,"");

  for(a=0;a<argc;a++)
    {
      params[a]=argv[a];
      writeLog(argv[a]);
      writeLog("\n");
    }
  removeLog();
  for(a=0;a<argc;a++)
    {
      writeLog(argv[a]);
      writeLog("\n");
    }

  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {  
      /* Check if user started prog by hand */   
      if(argc<1) {
        pmUsage();
      }
      else {

        /* Save installation directory */

        strncpy(chrInstallDir,params[0], sizeof(chrInstallDir));
        if((chrPtr=strrchr(chrInstallDir,'\\'))!=NULLHANDLE) {
          *chrPtr=0;
          if((chrPtr=strrchr(chrInstallDir,'\\'))!=NULLHANDLE)
            *chrPtr=0;
        }
        /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
        // strcpy(chrInstallDir,"E:\\CD-Brenner\\ADC_050");

        chrInstallDir[sizeof(chrInstallDir)-1]=0;
        /* Build the tutorial path */
        snprintf(chrTutorialPath, sizeof(chrTutorialPath), "%s\\Help\\tutorial.inf",chrInstallDir);
#if 0
        /* Get some ini settings */
        if((hIni=HlpOpenIni())!=NULLHANDLE)
          {
            ULONG ulSize, keyLength;
            
            /* Colors for toolbar flyover help */
            ulSize=sizeof(rgbBackground);
            if(!PrfQueryProfileData(hIni,"toolbar","flyoverbackcolor",(PBYTE)&rgbBackground,&ulSize)) {
            }
            ulSize=sizeof(rgbForeground);
            if(!PrfQueryProfileData(hIni,"toolbar","flyoverforecolor",(PBYTE)&rgbForeground,&ulSize)) {
            }
            /* Font for toolbar flyover help */
            if(cwQueryOSRelease()>=40)
              keyLength=PrfQueryProfileString(hIni,"toolbar","flyoverfont",DEFAULT_DIALOG_FONT,
                                              chrFontName,sizeof(chrFontName));
            else
              keyLength=PrfQueryProfileString(hIni,"toolbar","flyoverfont",DEFAULT_DIALOG_FONT_WARP3,
                                              chrFontName,sizeof(chrFontName));                      
            HlpCloseIni(hIni);
          }
#endif
        readIni2(chrInstallDir);
        /* Build the hint database path */
        HlpQueryDaytipDatabaseLocation(chrInstallDir, text, sizeof(text));
        //strcpy(text,"G:\\Projects_working\\Audio_cdcreator_051\\source\\res\\001\\DAYTIP.INI");
        if((hini=PrfOpenProfile(hab , text))!=NULLHANDLE) {
          if(!hptrHand)
            hptrHand=WinLoadPointer(HWND_DESKTOP, NULLHANDLE,IDPT_HAND);
          iNumTips=PrfQueryProfileInt(hini,"daytip" , "numtips",1);
          iCurrentTip=PrfQueryProfileInt(hini,"daytip" , "currenttip",1);
          sprintf(chrCurrentTip,"%d",iCurrentTip);
          WinQueryWindowRect(HWND_DESKTOP, &rclDesktop);//Query desktop size        
          hwndFocus=WinQueryFocus(HWND_DESKTOP);
          if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, hintDialogProc, NULLHANDLE, 100, 0) == DID_ERROR )
            {
              PrfCloseProfile(hini);
              if(hptrHand)
                WinDestroyPointer(hptrHand);
              return( 1 );    
              WinDestroyMsgQueue( hmq );
              WinTerminate( hab );
              DosBeep(100,600);
            }
          iCurrentTip++;
          if(iCurrentTip>iNumTips)
            iCurrentTip=1;
          sprintf(chrCurrentTip,"%d",iCurrentTip);
          PrfWriteProfileString(hini, "daytip" , "currenttip", chrCurrentTip);
          PrfCloseProfile(hini);
        }/* hini */
        else {
          sprintf(text, "Tip database can't be opened. Check your installation. There must be a file %s%s.", 
                  chrInstallDir, DAYTIPDATABASEPATH);
          WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text,
                         "Installation problem",
                         0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
        }
      }
      if(hptrHand)
        WinDestroyPointer(hptrHand);
      WinSetFocus(HWND_DESKTOP,hwndFocus);
    }
    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);
  }
  return 0;
}









