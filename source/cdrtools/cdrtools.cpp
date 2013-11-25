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
#define INCL_DOSSESMGR
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_WINDIALOGS

#include <os2.h>
#include <stdio.h>
#include <string.h>

#include "audiodialog.h"

MRESULT EXPENTRY CDToolsDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY CDSettingsDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );


HINI hini;
char chrCDRecord[CCHMAXPATH];
char chrCDROptions[CCHMAXPATH];

main()
{
  HAB hab;
  HMQ hmq;
  ULONG keyLength;
  char profileName[20];
  
  hab = WinInitialize( 0 );
  if(hab) {
    hmq = WinCreateMsgQueue( hab, 100UL );
    if(hmq) {
      sprintf(profileName,"cdrecord.ini");		
      hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),profileName);
      if(!hini) {
        WinMessageBox(  HWND_DESKTOP,
                        HWND_DESKTOP,
                        "Warning! Cannot open Ini-file!",
                        "",
                        0UL,
                        MB_OK | MB_ICONEXCLAMATION );
        WinDestroyMsgQueue( hmq );
        WinTerminate( hab );
        return( 1 );
      }/* end of if(!hini) */
      
      keyLength=PrfQueryProfileString(hini,"CDWriter","cdrecord","",chrCDRecord,sizeof(chrCDRecord));
      if(keyLength==1){
        if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, CDSettingsDlgProc, NULLHANDLE,DLGID_SETUP, 0 ) == DID_ERROR )
          {
            WinMessageBox(  HWND_DESKTOP,
                            HWND_DESKTOP,
                            "Warning! Cannot open Settings dialog!",
                            "",
                            0UL,
                            MB_OK | MB_ICONEXCLAMATION );
            WinDestroyMsgQueue( hmq );
            WinTerminate( hab );
            if(hini)PrfCloseProfile(hini);
            return( 1 );
          }
        if(!PrfWriteProfileString(hini,"CDWriter","cdrecord",chrCDRecord)){
          WinMessageBox(  HWND_DESKTOP,
                          HWND_DESKTOP,
                          "Warning! Cannot write to Ini-file!",
                          "",
                          0UL,
                          MB_OK | MB_ICONEXCLAMATION );
          WinDestroyMsgQueue( hmq );
          WinTerminate( hab );
          if(hini)PrfCloseProfile(hini);
          return( 1 );
        }
        
        if(!PrfWriteProfileString(hini,"CDWriter","cdroptions",chrCDROptions)){
          WinMessageBox(  HWND_DESKTOP,
                          HWND_DESKTOP,
                          "Warning! Cannot write to Ini-file!",
                          "",
                          0UL,
                          MB_OK | MB_ICONEXCLAMATION );
          WinDestroyMsgQueue( hmq );
          WinTerminate( hab );
          if(hini)PrfCloseProfile(hini);
          return( 1 );
        };
      }
      PrfQueryProfileString(hini,"CDWriter","cdroptions","",chrCDROptions,sizeof(chrCDROptions));
      
      if( WinDlgBox( HWND_DESKTOP, NULLHANDLE, CDToolsDlgProc, NULLHANDLE,DLGID_CDTOOLS, 0 ) == DID_ERROR )
        {
          WinDestroyMsgQueue( hmq );
          WinTerminate( hab );
          if(hini)PrfCloseProfile(hini);
          DosBeep(100,600);
          return( 1 );
        }
      if(hini)PrfCloseProfile(hini);
      WinDestroyMsgQueue( hmq );
    }
    WinTerminate( hab );
  }
  return( 0 );	
}

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
  
  switch( msg )
    {
    case WM_INITDLG:
      {
	WinCheckButton(hwnd,RBID_FAST,TRUE);
	WinCheckButton(hwnd,RBID_LOADDISK,TRUE);
	WinCheckButton(hwnd,CBID_DUMMY,TRUE);
	WinCheckButton(hwnd,CBID_FIXDUMMY,TRUE);
      }
      return (MRESULT) TRUE;
    case WM_DESTROY:
      
      break;
    case WM_CLOSE:
      if(WinMessageBox(  HWND_DESKTOP,
		       hwnd,
		       "Do you really want to quit?",
		       "Question",
		       0UL,
		       MB_OKCANCEL | MB_ICONQUESTION )==MBID_OK)
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
    /* 13.Apr.2000: changed to SSF_INHERTOPT_PARENT so modified environment by a skript is inherited */ 
	startData.InheritOpt=SSF_INHERTOPT_PARENT;
	startData.SessionType=SSF_TYPE_WINDOWABLEVIO;
	startData.PgmControl=SSF_CONTROL_VISIBLE|SSF_CONTROL_MAXIMIZE|SSF_CONTROL_NOAUTOCLOSE;
	startData.InitXPos=30;
	startData.InitYPos=30;
	startData.InitXSize=500;
	startData.InitYSize=400;
	startData.ObjectBuffer=chrLoadError;
	startData.ObjectBuffLen=(ULONG)sizeof(chrLoadError);
	switch( SHORT1FROMMP( mp1 ) )
	  {
	  case PBID_BLANK:
	    if(!WinQueryButtonCheckstate(hwnd,CBID_DUMMY)) {
	      if(WinMessageBox(  HWND_DESKTOP,
			       hwnd,
			       "Do you really want to perform a real write?",
			       "Dummy is unchecked!",
			       0UL,
			       MB_OKCANCEL | MB_ICONEXCLAMATION )==MBID_CANCEL)break;
	      sprintf(chrParams2,"%s","");
	    }
	    else
	      sprintf(chrParams2,"%s","-dummy");
	    if(WinQueryButtonCheckstate(hwnd,RBID_FAST))
	      sprintf(chrParams,"%s %s %s",chrCDROptions,chrParams2,"-blank=fast -eject");
	    if(WinQueryButtonCheckstate(hwnd,RBID_ALL))
	      sprintf(chrParams,"%s %s %s",chrCDROptions,chrParams2,"-blank=all -eject");
	    if(WinQueryButtonCheckstate(hwnd,RBID_LASTSESSION))
	      sprintf(chrParams,"%s %s %s",chrCDROptions,chrParams2,"-blank=session -eject");
	    if(WinQueryButtonCheckstate(hwnd,RBID_UNCLOSE))
	      sprintf(chrParams,"%s %s %s",chrCDROptions,chrParams2,"-blank=unclose -eject");
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
			       MB_OKCANCEL | MB_ICONEXCLAMATION )==MBID_CANCEL)break;
	      sprintf(chrParams,"%s %s",chrCDROptions," -fix");
	    }
	    else
	      sprintf(chrParams,"%s %s",chrCDROptions,"-dummy -fix");			
	    startData.PgmInputs=chrParams;
	    rc=DosStartSession(&startData,&ulSessionID,&pid);
	    break;
	  case PBID_MISC:
	    if(WinQueryButtonCheckstate(hwnd,RBID_UNLOADDISK))
	      sprintf(chrParams,"%s %s",chrCDROptions,"-eject");
	    else if(WinQueryButtonCheckstate(hwnd,RBID_LOADDISK))
	      sprintf(chrParams,"%s %s",chrCDROptions,"-load");
	    else if(WinQueryButtonCheckstate(hwnd,RBID_RESETDEVICE))
	      sprintf(chrParams,"%s %s",chrCDROptions,"-reset");
	    else if(WinQueryButtonCheckstate(hwnd,RBID_SHOWTOC))
	      sprintf(chrParams,"%s %s",chrCDROptions,"-toc");
	    else if(WinQueryButtonCheckstate(hwnd,RBID_SHOWATIP))
	      sprintf(chrParams,"%s %s",chrCDROptions,"-atip");


	    startData.PgmInputs=chrParams;
	    rc=DosStartSession(&startData,&ulSessionID,&pid);
	    break;
	  case PBID_EXIT:
	    WinPostMsg(hwnd,WM_CLOSE,(MPARAM)0,(MPARAM)0);
	    break;
	  case PBID_SETTINGS:
	    if( WinDlgBox( HWND_DESKTOP, hwnd, CDSettingsDlgProc, NULLHANDLE,DLGID_SETUP, 0 ) == DID_ERROR )
	      {
		WinMessageBox(  HWND_DESKTOP,
			      HWND_DESKTOP,
			      "Warning! Cannot open Settings dialog!",
			      "",
			      0UL,
			      MB_OK | MB_ICONEXCLAMATION );
	      }
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

MRESULT EXPENTRY CDSettingsDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  FILEDLG fd = { 0 };
  char profileName[20];
  static HINI hini;
  ULONG keyLength;
  static  char chrPath[CCHMAXPATH];
  static  char chrOptions[CCHMAXPATH];

  switch( msg )
    {
    case WM_INITDLG:
      {
	WinSendMsg(WinWindowFromID(hwnd,EFID_CDRECORDPATH),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
	WinSendMsg(WinWindowFromID(hwnd,EFID_CDRECORDOPTIONS),EM_SETTEXTLIMIT,MPFROMSHORT((SHORT)CCHMAXPATH),0);
	WinSetWindowText( WinWindowFromID(hwnd,EFID_CDRECORDPATH),chrCDRecord);
	WinSetWindowText( WinWindowFromID(hwnd,EFID_CDRECORDOPTIONS),chrCDROptions);
	sprintf(chrPath,"%s",chrCDRecord);
	sprintf(chrOptions,"%s",chrCDROptions);
	sprintf(profileName,"cdrecord.ini");		
	hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),profileName);
	if(!hini) {
	  WinMessageBox(  HWND_DESKTOP,
			HWND_DESKTOP,
			"Warning! Cannot open Ini-file!",
			"",
			0UL,
			MB_OK | MB_ICONEXCLAMATION );
	  WinDismissDlg(hwnd,DID_ERROR);
	}/* end of if(!hini) */				
      }
      return (MRESULT) TRUE;
    case WM_DESTROY:
      if(hini)PrfCloseProfile(hini);
      hini=NULLHANDLE;
      break;
    case WM_CLOSE:
      WinDismissDlg(hwnd,DID_ERROR);
      break;
    case WM_COMMAND:
      {			
	switch( SHORT1FROMMP( mp1 ) )
	  {
	  case PBID_CDRECORDBROWSE:
	    fd.cbSize = sizeof( fd );
	    fd.fl = FDS_OPEN_DIALOG|FDS_CENTER;
	    fd.pszTitle = "Search CDRecord/2";
	    sprintf(fd.szFullFile,"%s","*.exe");
	    
	    if( WinFileDlg( HWND_DESKTOP, hwnd, &fd ) == NULLHANDLE )
	      {
		break;
	      }
	    if( fd.lReturn == DID_OK )
	      {
		WinSetWindowText( WinWindowFromID(hwnd,EFID_CDRECORDPATH), fd.szFullFile );
		sprintf(chrCDRecord,"%s",fd.szFullFile);
	      }
	    break;
	  case DID_OK:
	    WinQueryWindowText(WinWindowFromID(hwnd,EFID_CDRECORDOPTIONS),sizeof(chrCDROptions),chrCDROptions);
	    if(!PrfWriteProfileString(hini,"CDWriter","cdrecord",chrCDRecord)){
	      WinMessageBox(  HWND_DESKTOP,
			    HWND_DESKTOP,
			    "Warning! Cannot write to Ini-file!",
			    "",
			    0UL,
			    MB_OK | MB_ICONEXCLAMATION );
	      WinDismissDlg(hwnd,DID_ERROR);
	    }
	    
	    if(!PrfWriteProfileString(hini,"CDWriter","cdroptions",chrCDROptions)){
	      WinMessageBox(  HWND_DESKTOP,
			    HWND_DESKTOP,
			    "Warning! Cannot write to Ini-file!",
			    "",
			    0UL,
			    MB_OK | MB_ICONEXCLAMATION );
	      WinDismissDlg(hwnd,DID_ERROR);
	    };
	    WinDismissDlg(hwnd,0);
	    break;
	  case DID_CANCEL:
	    sprintf(chrCDRecord,"%s",chrPath);
	    sprintf(chrCDROptions,"%s",chrOptions);
	    WinDismissDlg(hwnd,1);
	    break;
	    
	  default:
	    break;
	  }
      }
      return (MRESULT) TRUE;      
    }	
  return( WinDefDlgProc( hwnd, msg, mp1, mp2 ) );	
}







