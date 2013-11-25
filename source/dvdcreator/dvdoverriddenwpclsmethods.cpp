/*
 * This file is (C) Chris Wohlgemuth 1999-2004
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
//extern HMODULE hDataResource;
extern char chrInstallDir[];
extern char chrLanguageDLL[];

#ifdef DEBUG
extern  char debugText[200];
#endif

HMODULE queryModuleHandle();
void _loadBmps();
void SysWriteToTrapLog(const char* chrFormat, ...);

/* Overriden function which returns our class name */
PSZ M_CWDVDFolder::wpclsQueryTitle()
{
  return "Data-DVD-Creator";
}

BOOL M_CWDVDFolder::wpclsCreateDefaultTemplates(WPObject * wpObject)
{
  HOBJECT hObject;
  WPObject * wpTemplate;
  char chrSetup[CCHMAXPATH*2];

  //SysWriteToTrapLog("%s...\n", __FUNCTION__);
  if((hObject=WinQueryObject(ID_DVDTEMPLATE))!=NULLHANDLE) {
    //SysWriteToTrapLog("%s: Found object with ID\n", __FUNCTION__);
    /* There's an object with the ID. Check if it's the right template */
    if((wpTemplate=wpclsQueryObject(hObject))==NULLHANDLE) {
      //SysWriteToTrapLog("%s: Can't get object from handle\n", __FUNCTION__);
      /* A handle but no object?! Shouldn't happen, fall back to default. */
      return FALSE;
    }
    if(wpTemplate->somIsInstanceOf(this)) {
      ULONG ulStyle;
#if 0
char name[255];
ulStyle=sizeof(name);
((WPFileSystem*)wpTemplate)->wpQueryRealName(name, &ulStyle,TRUE);
SysWriteToTrapLog("%s: Oobject is a datadvd folder. Path: %s\n", __FUNCTION__, name);
#endif
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
  //SysWriteToTrapLog("%s: Creating new template\n", __FUNCTION__);
  /* Create a new template */
  sprintf(chrSetup,
          "TEMPLATE=YES;FLDRFILENAMEOPTIONS=31;FLDRCREATEOPTIONS=4;FLDRCDTYPEOPTIONS=520;OBJECTID=<DATADVD_CREATOR>");
  if((wpTemplate=wpclsNew(DATADVD_TEMPLATE_NAME, chrSetup, (WPFolder*)wpObject, FALSE))!=NULLHANDLE) {
    //SysWriteToTrapLog("%s: Temmplate created.\n", __FUNCTION__);
   sprintf(chrSetup,"BACKGROUND=%s\\Res\\DATACD.jpg,S,1,I",chrInstallDir);
    wpTemplate->wpSetup(chrSetup);
    return TRUE;
  }

  /* Error */
  return FALSE;
}







