/*
 * This file is (C) Chris Wohlgemuth 1999-2001
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

#include <stdlib.h>
#include <stdio.h>

extern HMODULE hBaseResource;
extern HMODULE hDataResource;
extern char chrInstallDir[];
extern char chrLanguageDLL[];

#ifdef DEBUG
extern  char debugText[200];
#endif

HMODULE queryModuleHandle();
void _loadBmps();

#if 0 /* This function is in settingsoverrridenwpcls... */

/* This function returns the module handle of our class-dll */
static HMODULE _queryModuleHandle()
{
  static HMODULE hmod=0;
    
  if(!hmod) {
    PSZ pathname=SOMClassMgrObject  //Query Pathname of class file
      ->somLocateClassFile(somIdFromString("CWDataFolder"),1,2);
    if( DosQueryModuleHandle(pathname,&hmod)){  //Query module handle
      /* Second try: CWAudioFolder */
      pathname=SOMClassMgrObject  //Query Pathname of class file
        ->somLocateClassFile(somIdFromString("CWAudioFolder"),1,2);
      if( DosQueryModuleHandle(pathname,&hmod)){  //Query module handle
        /* Last resort: CWCreatorSettings */
        pathname=SOMClassMgrObject  //Query Pathname of class file
          ->somLocateClassFile(somIdFromString("CWCreatorSettings"),1,2);
        DosQueryModuleHandle(pathname,&hmod);
      }
    }
  }/* if(!hmod) */

  return hmod;
}
#endif


/* Overriden function which returns our class name */
PSZ M_CWDataFolder::wpclsQueryTitle()
{
  return "Data-CD-Creator";
}

ULONG M_CWDataFolder::wpclsQueryStyle()
{
  /* CWProgFolder returns *_NEVERTEMPLATE but we want a template */
  //return M_WPFolder::wpclsQueryStyle();

  /* We don't want an automatic template because we create it during
     installation because we need one with an ID so we can create a shadow
     without problems. So we return CLSSTYLE_DONTTEMPLATE. 
     Another way would be an override of wpclsCreateDefaultTemplate()
     but this works, too, and we have to override this method anyway. We must
     disable CLSSTYLE_NEVERTEMPLATE or 'Create another' and the template checkbox
     will be lost. */
  return (M_CWProgFolder::wpclsQueryStyle()&~CLSSTYLE_NEVERTEMPLATE)|CLSSTYLE_DONTTEMPLATE;
}

BOOL M_CWDataFolder::wpclsQueryDefaultHelp(PULONG HelpPanelId,PSZ HelpLibrary)
{
	if(HelpLibrary)
		strcpy(HelpLibrary,AFHELPLIBRARY);
	if(HelpPanelId)
		*HelpPanelId= IDHLP_DATACDMAIN;
	
	return TRUE;
}

/* Overriden function which returns the custom icon */
ULONG M_CWDataFolder::wpclsQueryIconData(PICONINFO pIconInfo)
{  
  if (pIconInfo)   {
#ifdef DEBUG
    writeDebugLog("Entering M_CWDataFolder::wpclsQueryIconData(PICONINFO pIconInfo)\n");
    sprintf(debugText,"hDataResource: %x\n",hDataResource);
    writeDebugLog(debugText);
#endif

    pIconInfo->fFormat = ICON_RESOURCE;
    /* The function queryModuleHandle() returns the module handle
       of the DLL the class resides in. The resource compiler put
       the icon in the DLL after compiling the class */ 
    //pIconInfo->hmod    = queryModuleHandle();
    pIconInfo->hmod    = hBaseResource;
    pIconInfo->resid   = ID_ICONDATAFOLDER;
  } /* endif */
#ifdef DEBUG
    writeDebugLog("Leaving M_CWDataFolder::wpclsQueryIconData(PICONINFO pIconInfo)\n\n");
#endif

  return ( sizeof(ICONINFO) );
}


/* Overriden function which returns the custom icon */
ULONG M_CWDataFolder::wpclsQueryIconDataN(PICONINFO pIconInfo, ULONG ulIconIndex)
{
  if (pIconInfo)   {
#ifdef DEBUG
    writeDebugLog("Entering M_CWDataFolder::wpclsQueryIconData(PICONINFO pIconInfo)\n");
    sprintf(debugText,"hDataResource: %x\n",hDataResource);
    writeDebugLog(debugText);
#endif
    pIconInfo->fFormat = ICON_RESOURCE;
    /* The function queryModuleHandle() returns the module handle
       of the DLL the class resides in. The resource compiler put
       the icon in the DLL after compiling the class */ 
    //pIconInfo->hmod    = queryModuleHandle();
    pIconInfo->hmod    = hBaseResource;
    pIconInfo->resid   = ID_ICONDATAFOLDER2;
  } /* endif */
#ifdef DEBUG
    writeDebugLog("Leaving M_CWDataFolder::wpclsQueryIconDataN(PICONINFO pIconInfo)\n\n");
#endif

  return ( sizeof(ICONINFO) );
}

void M_CWDataFolder::wpclsInitData()
{
  COUNTRYCODE country= {0};
  COUNTRYINFO countryInfo= {0};
  ULONG ulInfoLen=0;
  APIRET rc;
  char path[CCHMAXPATH];  
  char* found;
  char buf[CCHMAXPATH];

  /* Call parent */
  M_CWProgFolder::wpclsInitData();

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

    rc=DosLoadModule(buf,sizeof(buf),path, &hDataResource);
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

    rc=DosLoadModule(buf,sizeof(buf),path, &hDataResource);
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
  PrfWriteProfileString(HINI_PROFILE, "PM_Workplace:IplLoad", "CWDataFolder", " ");

  /* Read in the custom backgrounds if not already done */
  _loadBmps();

#ifdef DEBUG
    sprintf(debugText,"hDataResource: %x\n",hDataResource);
    writeDebugLog(debugText);
    writeDebugLog("Leaving M_CWDataFolder::wpclsInitData()\n\n");
#endif
}

void M_CWDataFolder::wpclsUnInitData()
{
  //DosBeep(5000, 1000);
  //  DosFreeModule(hDataResource);
  M_CWProgFolder::wpclsUnInitData();
}


BOOL M_CWDataFolder::wpclsCreateDefaultTemplates(WPObject * wpObject)
{
  HOBJECT hObject;
  WPObject * wpTemplate;
  char chrSetup[CCHMAXPATH*2];


  if((hObject=WinQueryObject(ID_DATATEMPLATE))!=NULLHANDLE) {
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
  }/* if((hObject=WinQueryObject(ID_DATATEMPLATE))!=NULLHANDLE) */
  /* Create a new template */
  sprintf(chrSetup,
          "TEMPLATE=YES;FLDRFILENAMEOPTIONS=31;FLDRCREATEOPTIONS=1;FLDRCDTYPEOPTIONS=520;OBJECTID=<DATACD_CREATOR>");
  if((wpTemplate=wpclsNew(DATA_TEMPLATE_NAME,chrSetup, (WPFolder*)wpObject, FALSE))!=NULLHANDLE) {
    sprintf(chrSetup,"BACKGROUND=%s\\Res\\DATACD.jpg,S,1,I",chrInstallDir);
    wpTemplate->wpSetup(chrSetup);
    return TRUE;
  }

  /* Error */
  return FALSE;
}







