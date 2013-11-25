/*
 * This file is (C) Chris Wohlgemuth 1999-2003
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
#define INCL_DOSERRORS

#include "audiofolder.hh"
#include "audiofolderhelp.h"
#include <stdio.h>
#include <string.h>

extern HMODULE hBaseResource;
extern HMODULE hAudioResource;
extern ATOM atomUpdateStatusbar;
extern ATOM atomStartGrab;
extern ATOM atomStartWrite;
extern char chrInstallDir[];
extern char chrLanguageDLL[];

HMODULE queryModuleHandle(void);
BOOL registerFillBarClass(void);
void _loadBmps();

/* Overriden function which returns our class name */
PSZ M_CWAudioFolder::wpclsQueryTitle()
{
  return "Audio-CD-Creator";
}

/* Overriden function which returns the custom icon */
ULONG M_CWAudioFolder::wpclsQueryIconData(PICONINFO pIconInfo)
{
  if (pIconInfo)   {
    pIconInfo->fFormat = ICON_RESOURCE;
    /* The function queryModuleHandle() returns the module handle
       of the DLL the class resides in. The resource compiler put
       the icon in the DLL after compiling the class */ 
    //pIconInfo->hmod    = queryModuleHandle();
    pIconInfo->hmod    = hBaseResource;
    pIconInfo->resid   = ID_ICONAUDIOFOLDER;
  } /* endif */

  return ( sizeof(ICONINFO) );
}

ULONG M_CWAudioFolder::wpclsQueryIconDataN(PICONINFO pIconInfo, ULONG ulIconIndex)
{

  if (pIconInfo)   {
    pIconInfo->fFormat = ICON_RESOURCE;
    /* The function queryModuleHandle() returns the module handle
       of the DLL the class resides in. The resource compiler put
       the icon in the DLL after compiling the class */ 
    //pIconInfo->hmod    = queryModuleHandle();
    pIconInfo->hmod    = hBaseResource;
    pIconInfo->resid   = ID_ICONAUDIOFOLDER2;
  } /* endif */

  return ( sizeof(ICONINFO) );
}

ULONG M_CWAudioFolder::wpclsQueryStyle()
{
  /* CWProgFolder returns *_NEVERTEMPLATE but we want a template */
  //  return M_WPFolder::wpclsQueryStyle();

  /* We don't want an automatic template because we create it during
     installation because we need one with an ID so we can create a shadow
     without problems. So we return CLSSTYLE_DONTTEMPLATE. 
     Another way would be an override of wpclsCreateDefaultTemplate()
     but this works, too, and we have to override this method anyway. We must
     disable CLSSTYLE_NEVERTEMPLATE or 'Create another' and the template checkbox
     will be lost. */
  return (M_CWProgFolder::wpclsQueryStyle()&~CLSSTYLE_NEVERTEMPLATE)|CLSSTYLE_DONTTEMPLATE;
}

BOOL M_CWAudioFolder::wpclsQueryDefaultHelp(PULONG HelpPanelId,PSZ HelpLibrary)
{
	if(HelpLibrary)
      /* The name of the library is defined in audiofolderhelp.h */
		strcpy(HelpLibrary,AFHELPLIBRARY);
	if(HelpPanelId)
		*HelpPanelId= IDHLP_MAIN;
	
	return TRUE;
}

void M_CWAudioFolder::wpclsInitData()
{
  COUNTRYCODE country= {0};
  COUNTRYINFO countryInfo= {0};
  ULONG ulInfoLen=0;
  APIRET rc;
  char path[CCHMAXPATH];  
  char* found;
  char buf[CCHMAXPATH];
  HATOMTBL hatSystem;

  /* Call parent */
  M_CWProgFolder::wpclsInitData();

  /* Get unique window messages for our internal communication */
  hatSystem=WinQuerySystemAtomTable();
  /* Window message for updating the statusbar */
  atomUpdateStatusbar=WinAddAtom(hatSystem,WM_UPDATESTATUSBARMSG);
  /* Window message for starting a grab  */
  atomStartGrab=WinAddAtom(hatSystem,WM_STARTGRABMSG);
  /* Window message for starting a write */
  atomStartWrite=WinAddAtom(hatSystem,WM_STARTWRITEMSG);

  /* Get the handle of our class dll. This dll contains the graphics and icons.
     A language specific dll conatins all the NLS stuff.  */
  hBaseResource=queryModuleHandle();

  /* Get the country code of our system and load the  
     resource DLL with the right language */
  do {
    rc=DosQueryCtryInfo(sizeof(countryInfo),&country,&countryInfo,&ulInfoLen);
    if(rc!=NO_ERROR)
      break;
    
    rc=DosQueryModuleName(queryModuleHandle(),sizeof(path),path);
    if(rc!=NO_ERROR)
      break;

    found=strrchr(path,'\\');
    if(!found)
      break;

    *found=0;
    sprintf(buf,"\\cdfld%03d.dll",countryInfo.country);
    strcat(path,buf);

    rc=DosLoadModule(buf,sizeof(buf),path, &hAudioResource);
    if(rc==NO_ERROR) {
      strcpy(chrLanguageDLL, path);
      break;
    }

    /* NLS DLL not found. Try to load default */
    found=strrchr(path,'\\');
    if(!found)
      break;
    
    *found=0;
    sprintf(buf,"\\cdfld001.dll");
    strcat(path,buf);

    rc=DosLoadModule(buf,sizeof(buf),path, &hAudioResource);
    if(rc==NO_ERROR)
      strcpy(chrLanguageDLL, path);

    break;
  }while(TRUE);

  //wpclsIncUsage();
  /*
    For some strange reason the WPS may unload the class even if still in use. Seen on
    eCS V1.01 after deletion of an audio folder while a shadow to the CWAudioFolder template
    was still awake. This shouldn't happen at all. The template should be locked preventing
    unloading of the class. The reason is not clear. As a workaround the CW-Creator classes
    are added to "PM_Workplace:IplLoad". That will cause a load right on WPS start even if no
    object is awake but will also prevent an unloading of the class.
    Here we make sure the key is always present in the INI file.
   */
  PrfWriteProfileString(HINI_PROFILE, "PM_Workplace:IplLoad", "CWAudioFolder", " ");

  registerFillBarClass();

  /* Read in the custom backgrounds if not already done */
  _loadBmps();

  /* Call parent */
  //M_CWProgFolder::wpclsInitData();
}

void M_CWAudioFolder::wpclsUnInitData()
{
  //DosBeep(500, 1000);
  //  DosFreeModule(hAudioResource);
  M_CWProgFolder::wpclsUnInitData();
}


BOOL M_CWAudioFolder::wpclsCreateDefaultTemplates(WPObject * wpObject)
{
  HOBJECT hObject;
  WPObject * wpTemplate;
  char chrSetup[CCHMAXPATH*2];



  if((hObject=WinQueryObject(ID_AUDIOTEMPLATE))!=NULLHANDLE) {
    /* There's an object with the ID. Check if it's the right template */
    if((wpTemplate=wpclsQueryObject(hObject))==NULLHANDLE) {
      /* A handle but no object?! Shouldn't happen, fall back to default. */
      return FALSE;
    }
    if(wpTemplate->somIsInstanceOf(this)) {
      ULONG ulStyle;
      /* It's an instance of our class. Make sure it's a template. */
      ulStyle=wpTemplate->wpQueryStyle();
      wpTemplate->wpSetStyle(ulStyle|OBJSTYLE_TEMPLATE);
      return TRUE; /* Ok we have a template */
    }
    /* The object isn't our template. Probably left over from previous installation
       so remove it. */
    ((WPFileSystem*)wpTemplate)->wpSetAttr(FILE_NORMAL); /* Remove system, readonly, hidden flags */
    wpTemplate->wpSetStyle(0);/* Remove styles so deleting is possible */
    wpTemplate->wpDelete(NULL);
  }/* if((hObject=WinQueryObject(ID_AUDIOTEMPLATE))!=NULLHANDLE) */
  /* Create a new template */
  sprintf(chrSetup,
          "TEMPLATE=YES;FLDRWRITEFLAGS=11;OBJECTID=<AUDIOCD_CREATOR>");
  if((wpTemplate=wpclsNew(AUDIO_TEMPLATE_NAME,chrSetup, (WPFolder*)wpObject, FALSE))!=NULLHANDLE) {
    sprintf(chrSetup,"BACKGROUND=%s\\Res\\music.jpg,T,,I",chrInstallDir);
    wpTemplate->wpSetup(chrSetup);
    return TRUE;
  }

  /* Error */
  return FALSE;
}

