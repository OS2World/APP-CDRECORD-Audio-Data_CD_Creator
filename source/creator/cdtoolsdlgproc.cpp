#include "audiofolder.hh"
#include <stdio.h>

extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern int iBus,iTarget,iLun;
extern int iSpeed;

/* For custom BG */
extern LOADEDBITMAP allBMPs[NUM_CTRL_IDX];
extern BOOL bUseCustomPainting;
extern PFNWP  oldStaticTextProc;
/* New static text drawing proc */
extern MRESULT EXPENTRY staticTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void setupCheckBoxControl(  HWND hwnd, USHORT id);
void setupRadioButtonControl(HWND hwnd, USHORT id);
void setupGroupBoxControl(HWND hwnd, USHORT id);

ULONG cwQueryOSRelease();

/*********************************************************************/
/*                                                                   */
/* This procedure handles the CDRTools dialog                        */
/*                                                                   */
/*********************************************************************/
MRESULT EXPENTRY CDToolsDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  HAB hab;
  char chrLoadError[CCHMAXPATH];
  STARTDATA startData={0};
  PSZ pszTitle="CDRecord/2";
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrParams[CCHMAXPATH];
  char chrParams2[20];
  char chrDev[20];

  switch( msg )
    {
    case WM_PAINT:
      {
        HPS hps;
        RECTL rcl, rclSource;
        POINTL ptl;
        LONG lTemp;
        LONG lWidthX;
        LONG lWidthY;
        ULONG ulBG;

        if(bUseCustomPainting) {
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
            /* Border */
            lWidthX=WinQuerySysValue(HWND_DESKTOP, SV_CXBORDER);
            lWidthY=WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER);

            WinQueryWindowRect(hwnd, &rcl);
            WinDrawBorder(hps,&rcl,lWidthX, lWidthY,0,0,0x400);

            WinEndPaint(hps);
            return (MRESULT)0;
          }
        }
        break;
      }
    case WM_INITDLG:
      {      
        //   SWCNTRL swctl;
        
        WinCheckButton(hwnd,RBID_FAST,TRUE);
        WinCheckButton(hwnd,RBID_LOADDISK,TRUE);
        WinCheckButton(hwnd,CBID_DUMMY,TRUE);
        WinCheckButton(hwnd,CBID_FIXDUMMY,TRUE);
      
        /* Set dialog font to WarpSans for Warp 4 and above */
        if(cwQueryOSRelease()>=40) {
          WinSetPresParam(hwnd,
                          PP_FONTNAMESIZE,(ULONG)sizeof(DEFAULT_DIALOG_FONT),
                          DEFAULT_DIALOG_FONT );
        }

        /* subclass checkbox and find mnemonic */
        setupCheckBoxControl(  hwnd , CBID_DUMMY);
        setupCheckBoxControl(  hwnd , CBID_FIXDUMMY);
        /* subclass radio buttons */
        setupRadioButtonControl(hwnd, RBID_FAST);
        setupRadioButtonControl(hwnd, RBID_ALL);
        setupRadioButtonControl(hwnd, RBID_LASTSESSION);
        setupRadioButtonControl(hwnd, RBID_UNCLOSE);
        setupRadioButtonControl(hwnd, RBID_SHOWATIP);
        setupRadioButtonControl(hwnd, RBID_SHOWTOC);
        setupRadioButtonControl(hwnd, RBID_LOADDISK);
        setupRadioButtonControl(hwnd, RBID_UNLOADDISK);
        setupRadioButtonControl(hwnd, RBID_RESETDEVICE);
        /* subclass static text control for custom painting */
        oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_WARNINGTEXT1),staticTextProc);
        oldStaticTextProc=WinSubclassWindow(WinWindowFromID(hwnd, IDST_WARNINGTEXT2),staticTextProc);
        /* Subclass group boxes */
        setupGroupBoxControl(hwnd, IDGB_FIXCD);
        setupGroupBoxControl(hwnd, IDGB_CDRTOOLSBLANK);
        setupGroupBoxControl(hwnd, IDGB_CDRTOOLSMISC);

#if 0
        /* Add switch entry */
        memset(&swctl,0,sizeof(swctl));
        //WinQueryWindowProcess(hwnd,&pid,NULL);
        swctl.hwnd=hwnd;
        swctl.uchVisibility=SWL_VISIBLE;
        //  swctl.idProcess=pid;
        swctl.bProgType=PROG_DEFAULT;
        swctl.fbJump=SWL_JUMPABLE;
        swctl.szSwtitle="CDR-TOOLS";
        swctl.szSwtitle="CDR-TOOLS";
        WinAddSwitchEntry(&swctl);
#endif
        WinSetActiveWindow(HWND_DESKTOP,hwnd);
        return (MRESULT) TRUE;
      }
    case WM_DESTROY:
      
      break;
    case WM_CLOSE:
      WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
      return 0;
    case WM_COMMAND:
      {
	memset(&startData,0,sizeof(startData));
	startData.Length=sizeof(startData);
	startData.Related=SSF_RELATED_INDEPENDENT;
	startData.FgBg=SSF_FGBG_FORE;
	startData.TraceOpt=SSF_TRACEOPT_NONE;
	startData.PgmTitle=pszTitle;
	startData.PgmName=chrCDRecord;
	startData.InheritOpt=SSF_INHERTOPT_SHELL;
	startData.SessionType=SSF_TYPE_WINDOWABLEVIO;
	startData.PgmControl=SSF_CONTROL_VISIBLE|SSF_CONTROL_MAXIMIZE|SSF_CONTROL_NOAUTOCLOSE;
	startData.InitXPos=30;
	startData.InitYPos=30;
	startData.InitXSize=500;
	startData.InitYSize=400;
	startData.ObjectBuffer=chrLoadError;
	startData.ObjectBuffLen=(ULONG)sizeof(chrLoadError);
    sprintf(chrDev,"dev=%d,%d,%d -v",iBus,iTarget,iLun);
	switch( SHORT1FROMMP( mp1 ) )
	  {
	  case PBID_BLANK:
	    if(!WinQueryButtonCheckstate(hwnd,CBID_DUMMY)) {
	      if(WinMessageBox(  HWND_DESKTOP,
			       hwnd,
			       "Do you really want to perform a real write?",
			       "Dummy is unchecked!",
			       0UL,
			       MB_YESNO | MB_ICONEXCLAMATION|MB_MOVEABLE )==MBID_NO)break;
	      sprintf(chrParams2,"%s","");
	    }
	    else
	      sprintf(chrParams2,"%s","-dummy");
	    if(WinQueryButtonCheckstate(hwnd,RBID_FAST))
	      sprintf(chrParams,"%s %s -speed=%d %s",chrDev,chrParams2, iSpeed, "-blank=fast -eject");
	    else if(WinQueryButtonCheckstate(hwnd,RBID_ALL))
	      sprintf(chrParams,"%s %s -speed=%d %s",chrDev,chrParams2, iSpeed, "-blank=all -eject");
	    else if(WinQueryButtonCheckstate(hwnd,RBID_LASTSESSION))
	      sprintf(chrParams,"%s %s -speed=%d %s",chrDev,chrParams2, iSpeed, "-blank=session -eject");
	    else if(WinQueryButtonCheckstate(hwnd,RBID_UNCLOSE))
	      sprintf(chrParams,"%s %s -speed=%d %s",chrDev,chrParams2, iSpeed, "-blank=unclose -eject");
	    startData.PgmInputs=chrParams;
	    rc=DosStartSession(&startData,&ulSessionID,&pid);					
	    break;
	  case PBID_FIX:
	    if(!WinQueryButtonCheckstate(hwnd,CBID_FIXDUMMY)) {
	      if(WinMessageBox(  HWND_DESKTOP,
			       hwnd,
			       "Do you really want to perform a real write?",
			       "Dummy is unchecked!",
			       0UL,
			       MB_YESNO | MB_ICONEXCLAMATION|MB_MOVEABLE )==MBID_NO)break;
	      sprintf(chrParams,"%s -speed=%d %s",chrDev, iSpeed, " -fix");
	    }
	    else
	      sprintf(chrParams,"%s -speed=%d %s",chrDev, iSpeed, "-dummy -fix");			
	    startData.PgmInputs=chrParams;
	    rc=DosStartSession(&startData,&ulSessionID,&pid);
	    break;
	  case PBID_MISC:
	    if(WinQueryButtonCheckstate(hwnd,RBID_UNLOADDISK))
	      sprintf(chrParams,"%s %s",chrDev,"-eject");
	    else if(WinQueryButtonCheckstate(hwnd,RBID_LOADDISK))
	      sprintf(chrParams,"%s %s",chrDev,"-load");
	    else if(WinQueryButtonCheckstate(hwnd,RBID_RESETDEVICE))
	      sprintf(chrParams,"%s %s",chrDev,"-reset");
	    else if(WinQueryButtonCheckstate(hwnd,RBID_SHOWTOC))
	      sprintf(chrParams,"%s %s",chrDev,"-toc");
	    else if(WinQueryButtonCheckstate(hwnd,RBID_SHOWATIP))
	      sprintf(chrParams,"%s %s",chrDev,"-atip");

	    startData.PgmInputs=chrParams;
	    rc=DosStartSession(&startData,&ulSessionID,&pid);
	    break;
	  case PBID_EXIT:
	      WinPostMsg(hwnd,WM_CLOSE,(MPARAM)0,(MPARAM)0);
	    break;
	  default:
	    break;
	  }
      }
      return (MRESULT) TRUE;
      break;
    }	
  return( WinDefDlgProc( hwnd, msg, mp1, mp2 ) );	
}

