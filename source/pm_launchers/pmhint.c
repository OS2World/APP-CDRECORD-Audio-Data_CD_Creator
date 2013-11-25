/*
 * pmhint.c (C) Chris Wohlgemuth 2001
 *
 * This program shows a hint
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
//#include "audiofolderres.h"
#include "pmhint.h"

//#define DEBUG

typedef struct{
  PFNWP pfnwp;
  char  chrParameter[1024];
  char  chrProgram[CCHMAXPATH];
}ITEMDATA;

#define HINT_TIMER_ID 1
#define MY_DIALOGBORDER 10
#define MY_CBSPACE     12 /* space between checkbox and bottom clickable item */
#define MY_ITEMSPACE   5
#define SHADOWDELTA    10
#define MAXITEMS       5
 
char chrInstallDir[CCHMAXPATH];
char chrTutorialPath[CCHMAXPATH]="E:\\CD-Brenner\\ADC_048\\tutorial.inf";
char logName[]="pmhint.log";

extern SWP swpWindow;

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

RGB rgbForeground={0,0,0};
RGB rgbBackground={180,255,255};
char chrFontName[CCHMAXPATH];/* Font for fly over help */

  /* argv[0]: progname
   * argv[1]: installdir of Audio/Data-CD-Creator
   * argv[2]: 
   */

void pmUsage();
/*void removeLog();
 */
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
      if(item)
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
          WinPostMsg(WinQueryWindow(hwnd, QW_PARENT),WM_CLOSE,0,0);

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
ULONG calculateVerticalControlSize(HWND hwnd, char * ptrText, RECTL rectl, ULONG *ulCx)
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
  for(iTotalDrawn=0;iTotalDrawn!=ulWinTextLen; yTop-=lCy) {
    rectl=rcl;
    iDrawn=WinDrawText(hps, ulWinTextLen-iTotalDrawn,
                       ptrText+iTotalDrawn, &rectl, 0, 0, DT_QUERYEXTENT|DT_TEXTATTRS|DT_WORDBREAK|DT_TOP|DT_LEFT);
    if(iDrawn)
      iTotalDrawn+=iDrawn;
    else
      break; /* Nothing to draw */
  }
  WinReleasePS(hps);

  *ulCx=aptl[TXTBOX_TOPRIGHT].x-aptl[TXTBOX_TOPLEFT].x;

  /* Calculate the new height */
  return (rcl.yTop-yTop);
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
    }
    else
      item->chrParameter[0]=0;
  }
  else
    item->chrParameter[0]=0;
}

BOOL rebuildDialogFromIniData(HINI hini, HWND hwnd, char * chrApp, char *chrApp2)
{
  char *memPtr;
  ULONG ulSize;
  int a, numChoices;
  RECTL allRcl[2+5];/* Hinttext, checkbox and 5 clickable items */
  int numControls=2;
  ULONG ulY=0;
  HPS hps;
  POINTL aptlPoints[TXTBOX_COUNT];
  ULONG ulCy, ulCx;
  char text[CCHMAXPATH];
  HWND hwndCtrl;
  SWP swp;

  if(PrfQueryProfileSize(hini, chrApp,"hinttext",&ulSize)) {
    if(ulSize==0)
      chrApp=chrApp2;
  }
  else
    chrApp=chrApp2;

  /* Disable checkbutton */
  WinCheckButton(hwnd, IDCB_HINTENABLE, 0);
  /* This control is already added to numControls */
  hwndCtrl=WinWindowFromID(hwnd, IDCB_HINTENABLE);
  /* Calculate text size in pixel */
  WinQueryWindowText(hwndCtrl,CCHMAXPATH, text);
  hps=WinBeginPaint(hwndCtrl, (HPS)NULL,(PRECTL)NULL);
  GpiQueryTextBox(hps,strlen(text),text,TXTBOX_COUNT,aptlPoints);
  WinEndPaint(hps);
  ulCy=aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y;
  ulCx=aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x;
  WinQueryWindowPos(hwndCtrl,&swp);
  WinSetWindowPos(hwndCtrl,NULLHANDLE, swp.x , swp.y+5,
                  /*rcl.xRight-rcl.xLeft*/ ulCx+25, ulCy
                  ,SWP_SIZE|SWP_MOVE );
  ulY=MY_CBSPACE+ulCy+MY_DIALOGBORDER+5;

  /* Build the clickable items */
  numChoices=PrfQueryProfileInt(hini, chrApp, "numchoices", 0);
  if(numChoices>5)
    numChoices=5;
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
                              /*rcl.xRight-rcl.xLeft*/ ulCx+20, ulCy
                              ,SWP_SIZE|SWP_MOVE );

              /* Save the size for dialog calculating */
              ulY+=ulCy+MY_ITEMSPACE;

              /* Set the text */
              WinSetWindowText(hwndCtrl,memPtr);
              /* Free memory */
              free(memPtr);
              /* Subclass control */
              itemData[a-1].pfnwp=WinSubclassWindow(hwndCtrl, urlProc);
              WinSetWindowULong(hwndCtrl,QWL_USER,(ULONG)&itemData[a-1]);
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

        WinQueryWindowPos(hwndCtrl,&swp);
        WinSetWindowPos(hwndCtrl,NULLHANDLE, swp.x, ulY,
                        rcl.xRight-rcl.xLeft, ulCy
                        ,SWP_SIZE|SWP_MOVE);
        ulY+=ulCy+MY_DIALOGBORDER;

        /* Set the text */
        WinSetWindowText(hwndCtrl,memPtr);
        free(memPtr);
      }
  }
  else
    WinSetWindowText(WinWindowFromID(hwnd, IDST_MAINTEXT),"No hint available.");

  /* Size the dialog */
  WinQueryWindowPos(hwnd,&swpWindow);
  WinSetWindowPos(hwnd,NULLHANDLE, 0,0,
                  swpWindow.cx, ulY, SWP_SIZE);
  return TRUE;
}


void createShadow(HWND hwnd)
{
  HPS hpsMem;
  BITMAPINFOHEADER2 bmpIH2;
  PBITMAPINFO2 pbmp2;
  char * chrBuff;
  PBYTE ptr;
  HDC hdcMem;
  HBITMAP hbm;
  ULONG ulCx, ulCy;
  SWP swp;

  //  GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
  WinQueryWindowPos(hwnd, &swp);
  
  ulCx=swp.cx;
  ulCy=swp.cy;
  
  bmpIH2.cbFix=sizeof(BITMAPINFOHEADER2);
  bmpIH2.cx=ulCx;
  bmpIH2.cy=ulCy;
  bmpIH2.cPlanes=1;
  bmpIH2.cBitCount=8;
  bmpIH2.cbImage=(((ulCx*(1<<bmpIH2.cPlanes)*(1<<bmpIH2.cBitCount))+31)/32)*bmpIH2.cy;
  
  chrBuff=(char*)malloc(bmpIH2.cbImage+sizeof(BITMAPINFO2)+256*sizeof(RGB2));
  pbmp2=(PBITMAPINFO2)chrBuff;
  memset(pbmp2, 0, sizeof(BITMAPINFO2)+256*sizeof(RGB2));
  ptr=chrBuff+sizeof(BITMAPINFO2)+256*sizeof(RGB2);
  
  pbmp2->cbFix=sizeof(BITMAPINFO2);
  pbmp2->cx=ulCx;
  pbmp2->cy=ulCy;
  pbmp2->cPlanes=1;
  pbmp2->cBitCount=8;
  pbmp2->cbImage=((pbmp2->cx+31)/32)*pbmp2->cy;
  pbmp2->ulCompression=BCA_UNCOMP;
  pbmp2->ulColorEncoding=BCE_RGB;
  
  hdcMem=DevOpenDC(WinQueryAnchorBlock(hwnd),OD_MEMORY,"*", 0L/*4L*/, (PDEVOPENDATA)NULLHANDLE/*pszData*/, NULLHANDLE);
  if(hdcMem) {
    SIZEL sizel= {0,0};

    hpsMem=GpiCreatePS(WinQueryAnchorBlock(hwnd), hdcMem, &sizel, PU_PELS|GPIT_MICRO|GPIA_ASSOC);
    if(hpsMem)
      {                
        hbm=GpiCreateBitmap(hpsMem, &bmpIH2, FALSE, NULL, pbmp2);
        if(hbm) {
          HPS hpsDesktop;
          POINTL ptl[3]={0};
          RGB2 *prgb2;
          int a, r,g,b;

          hpsDesktop=WinGetScreenPS(HWND_DESKTOP);

          GpiSetBitmap(hpsMem, hbm);
          
          ptl[0].x=0;
          ptl[0].y=0;
          ptl[1].x=0+ulCx;
          ptl[1].y=0+ulCy;
          
          ptl[2].x=swp.x+SHADOWDELTA;
          ptl[2].y=swp.y-SHADOWDELTA;
          
          if(GpiBitBlt(hpsMem, hpsDesktop, 3, ptl , ROP_SRCCOPY, BBO_IGNORE)==GPI_ERROR)
            {

            }
          
          if(GpiQueryBitmapBits(hpsMem, 0, ulCy, ptr, pbmp2)==GPI_ALTERROR)
            {

            }
          
          prgb2=(RGB2*)(++pbmp2);
          for(a=0;a<256; a++, prgb2++) {
            r=-50;
            g=-50;
            b=-50;
            
            b+=prgb2->bBlue;
            g+=prgb2->bGreen;
            r+=prgb2->bRed;
            if(r>255)
              r=255;
            if(r<0)
              r=0;
            prgb2->bRed=r;
            
            if(g>255)
              g=255;
            if(g<0)
              g=0;
            prgb2->bGreen=g;
            
            if(b>255)
              b=255;
            if(b<0)
              b=0;
            prgb2->bBlue=b;        
          }

          if(GpiSetBitmapBits(hpsMem, 0, ulCy, ptr, --pbmp2)!=GPI_ALTERROR)
            {
              ptl[0].x=swp.x+SHADOWDELTA;
              ptl[0].y=swp.y-SHADOWDELTA;
              ptl[1].x=swp.x+SHADOWDELTA+ulCx;
              ptl[1].y=swp.y-SHADOWDELTA+ulCy;
              ptl[2].x=0;
              ptl[2].y=0;
              
              GpiBitBlt(hpsDesktop, hpsMem, 3, ptl , ROP_SRCCOPY, BBO_IGNORE);

            }
          WinReleasePS(hpsDesktop);

          GpiSetBitmap(hpsMem, NULLHANDLE);
          GpiDeleteBitmap(hbm);
        }/* hbm */
        GpiDestroyPS(hpsMem);
      }/* hpsMem */
    DevCloseDC(hdcMem);
  }/* if(hdcMem) */
  free(chrBuff);
}

#if 0
void createShadow(HWND hwnd)
{
  HPS hps;
  RECTL rcl;
  POINTL pointl;
  SWP swp;

  WinQueryWindowPos(hwnd, &swp);
  
  hps=WinGetScreenPS(HWND_DESKTOP);

  GpiSetPattern(hps,PATSYM_DENSE7);
  pointl.x = swp.x+SHADOWDELTA;
  pointl.y = swp.y-SHADOWDELTA;
  GpiMove(hps, &pointl);
  pointl.x = swp.x+swp.cx+SHADOWDELTA - 1;
  pointl.y = swp.y +swp.cy-SHADOWDELTA - 1;
  GpiBox(hps,DRO_FILL,&pointl,1,1);


  WinReleasePS(hps);
}
#endif

void removeShadow(HWND hwnd)
{
  HPS hps;
  RECTL rcl;
  POINTL pointl;
  SWP swp;

  WinQueryWindowPos(hwnd, &swp);

  rcl.yBottom=swp.y-SHADOWDELTA;
  rcl.yTop=swp.y+swp.cy-SHADOWDELTA;
  rcl.xLeft=swp.x+SHADOWDELTA;
  rcl.xRight=swp.x+swp.cx+SHADOWDELTA;

  WinInvalidateRect(HWND_DESKTOP,&rcl, TRUE);
  return;
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
#endif   

        /* Set colors */
        WinSetPresParam(hwnd,
                        PP_BACKGROUNDCOLOR,sizeof(rgbBackground) ,
                        &rgbBackground );
        WinSetPresParam(hwnd,
                        PP_FOREGROUNDCOLOR,sizeof(rgbForeground) ,
                        &rgbForeground );
        /* Set font */
        WinSetPresParam(hwnd,PP_FONTNAMESIZE,
                        sizeof(chrFontName),
                        chrFontName);
        /* Build the dialog */
        /* !!!!!!!!!!!!! Enable check is missing !!!!!!!!!!!!!!*/        
        if(numArgs==4) {
          a=3;
        }
        else {
          a=4;
        }

        if(rebuildDialogFromIniData(hini, hwnd, params[3], params[4])) {
          SWP swp;
          POINTL pointl;

          WinQueryWindowPos(hwnd,&swp);
          WinQueryPointerPos(HWND_DESKTOP,&pointl);
          if(swp.cy > rclDesktop.yTop-pointl.y)
            pointl.y=rclDesktop.yTop-swp.cy;
          else
            pointl.y-=10;

          if(swp.cx > rclDesktop.xRight-pointl.x)
            pointl.x=rclDesktop.xRight-swp.cx;
          else
            pointl.x-=10;

          WinSetWindowPos(hwnd,HWND_TOP,pointl.x,pointl.y,0,0,SWP_ZORDER|SWP_ACTIVATE|SWP_MOVE);
          createShadow(hwnd);
          WinSetWindowPos(hwnd,HWND_TOP,pointl.x,pointl.y,0,0,SWP_SHOW);
        }
        else
          WinPostMsg(hwnd,WM_CLOSE,0,0);
        ulTimer=WinStartTimer(hab, hwnd, HINT_TIMER_ID, 150);
        sprintf(text,"CDCHINTDAEMONHWND=%d",hwnd);
        /* HWND !!!!!!!!!!!!!! */
        //sendCommand(text);

        return (MRESULT) TRUE;
      }
#if 0
    case WM_APPTERMINATENOTIFY:
      {
        SWP swp;
        POINTL pointl;

      hwndFocus=WinQueryFocus(HWND_DESKTOP);
      WinQueryWindowPos(hwnd,&swp);
      WinQueryPointerPos(HWND_DESKTOP,&pointl);
      if(swp.cy > rclDesktop.yTop-pointl.y)
        pointl.y=rclDesktop.yTop-swp.cy;
      else
        pointl.y-=10;
      
      if(swp.cx > rclDesktop.xRight-pointl.x)
        pointl.x=rclDesktop.xRight-swp.cx;
      else
        pointl.x-=10;
      
      WinSetWindowPos(hwnd,HWND_TOP,pointl.x,pointl.y,0,0,SWP_ZORDER|SWP_ACTIVATE|SWP_MOVE|SWP_SHOW);

      WinSetFocus(HWND_DESKTOP, hwnd);
      if(!ulTimer)
        ulTimer=WinStartTimer(hab, hwnd, HINT_TIMER_ID, 150);
      //DosBeep(5000,200);

      }
      break;
#endif
    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return FALSE;
    case WM_DESTROY:
      if(ulTimer)
        WinStopTimer(hab, hwnd, ulTimer);
      removeShadow(hwnd);
      break;
    case WM_CONTROL:
      /* The window controls send WM_CONTROL messages */
      switch(SHORT1FROMMP(mp1))
        {
        case IDCB_HINTENABLE:
          {
            char chrSetup[CCHMAXPATH];
            if(WinQueryButtonCheckstate(hwnd,IDCB_HINTENABLE) & 1)
              sprintf(chrSetup,"%s=0",SETUP_FLDRHINTENABLE);
            else { 
              sprintf(chrSetup,"%s=1",SETUP_FLDRHINTENABLE);
            }
            sendCommand(chrSetup);
            break;
          }
        default:
          break;
        } // end switch(SHORT1FROMMP(mp1))
      break;
    case WM_TIMER:
      {
        if(SHORT1FROMMP(mp1)==ulTimer) {
          POINTL pointl;
          //       DosBeep(1000,10);       
          WinQueryWindowPos(hwnd,&swpWindow);
          WinQueryPointerPos(HWND_DESKTOP,&pointl);
          if(pointl.x<swpWindow.x || pointl.x >swpWindow.x+swpWindow.cx ||
             pointl.y<swpWindow.y || pointl.y >swpWindow.y+swpWindow.cy) {
            removeShadow(hwnd);   
            
            // WinPostMsg(hwnd,WM_APPTERMINATENOTIFY, 0,0);
            WinStopTimer(hab, hwnd, ulTimer);
            ulTimer=0;

            WinPostMsg(hwnd,WM_CLOSE,0,0);
          }
          //WinPostMsg(hwnd,WM_CLOSE,0,0);
          return (MRESULT) 0;
        }
      }
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

  /* Create a copy of the args */
  /* argv[0]: progname
     argv[1]: installdir
     argv[2]: "<CWCREATOR_SETTINGS>"
     argv[3]: Appkey in hint database
   */

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

  //  params[2]="<CWCREATOR_SETTINGS>";
  hab=WinInitialize(0);
  if(hab) {
    hmq=WinCreateMsgQueue(hab,0);
    if(hmq) {  
      /* Check if user started prog by hand */   
      if(argc<4) {
        pmUsage();
      }
      else {
        if (argc==4)
          params[4]=argv[3];

        /* Save installation directory */
        strncpy(chrInstallDir,params[1], sizeof(chrInstallDir));
        chrInstallDir[sizeof(chrInstallDir)-1]=0;
        /* Build the tutorial path */
        snprintf(chrTutorialPath, sizeof(chrTutorialPath), "%s\\Help\\tutorial.inf",chrInstallDir);
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
#if 0      /* Get our ressource dll */  
        RESSOURCEHANDLE=queryResModuleHandle2(chrInstallDir);
#endif
        /* Build the hint database path */
        HlpQueryHintDatabaseLocation(chrInstallDir, text, sizeof(text));
        if((hini=PrfOpenProfile(hab , text))!=NULLHANDLE) {
          if(!hptrHand)
            hptrHand=WinLoadPointer(HWND_DESKTOP, NULLHANDLE,IDPT_HAND);
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
          PrfCloseProfile(hini);
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









