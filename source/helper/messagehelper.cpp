/*
 * This file is (C) Chris Wohlgemuth 1999
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

#define INCL_WIN
#define INCL_DOS

#include <os2.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "audiofolderres.h"
#include "excptLogName.h"


void HlpSendCommandToObject(char* chrObject, char* command);
void errorResource2(char * chrTitle);

void errorResource()
{

  errorResource2("Problem with Audio/Data-CD-Creator installation");

}

void errorResource2(char * chrTitle)
{

  WinMessageBox(HWND_DESKTOP,0,
                "The resource DLL which contains all the dialogs, graphics and messages cannot be loaded. \
Please check your installation. There must be a file CDFLDxxx.DLL in the installation directory of the \
CD-Creator package. xxx is the country code of your system e.g. 049 for Germany. If there is no \
support for your language there must be at least the file CDFLD001.DLL. The CD creation features \
are not available!",
                chrTitle,12345,
                MB_OK|MB_MOVEABLE|MB_ERROR);

}

ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags)
{
  HWND hwndOwner=HWND_DESKTOP;

  if(WinIsWindow(WinQueryAnchorBlock(HWND_DESKTOP), hwnd))
    hwndOwner=hwnd;

  if(!WinLoadString(WinQueryAnchorBlock(hwndOwner),hResource,ulTextID,lSizeText,text)) {
    errorResource();
    return MBID_ERROR;
  }
  if(!WinLoadString(WinQueryAnchorBlock(hwndOwner),hResource,ulTitleID,lSizeTitle,title)) {
    errorResource();
    return MBID_ERROR;
  }
  return WinMessageBox(  HWND_DESKTOP, hwndOwner, text, title, 0UL, ulFlags );
}

/* Show a messagebox with the text ulIDText and title ulIDTitle loaded from
   the resource DLL */
ULONG showMsgBox2(HWND hwnd, ULONG ulIDTitle, ULONG ulIDText, HMODULE hModule, ULONG ulFlag)
{
  char text[256];
  char title[100];
  HWND hwndOwner=HWND_DESKTOP;

  if(WinIsWindow(WinQueryAnchorBlock(HWND_DESKTOP), hwnd))
    hwndOwner=hwnd;
  
  return messageBox(  text, ulIDText , sizeof(text),
               title, ulIDTitle, sizeof(title),
               hModule, hwndOwner, ulFlag);
};

void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd)
{
  if(!WinLoadString(WinQueryAnchorBlock(hwnd),hResource,ulID,lSizeText,text)) {
    sprintf(text,"");
  }
}

void mboxShowMissingProgMsgBox(HWND hwnd, char * chrWhichProg, HMODULE hResource, BOOL bOpenSettings)
{
  char name[CCHMAXPATH];
  char title [CCHMAXPATH+10];
  HWND hwndOwner=HWND_DESKTOP;
  
  if(WinIsWindow(WinQueryAnchorBlock(HWND_DESKTOP), hwnd))
    hwndOwner=hwnd;

 /* The path to XXXXX isn't valid */
  /* name: "...%s..." */
  getMessage(title, IDSTR_NOVALIDPATH, sizeof(name), hResource, hwnd);            
  sprintf(name, title, chrWhichProg);
  /* title: "Audio/Data-CD-Creator installation problem!" */
  getMessage(title, IDSTR_INSTALLERRORTITLE,sizeof(name), hResource, hwnd);                     
  WinMessageBox(  HWND_DESKTOP, hwndOwner,
                  name,
                  title,
                  0UL, MB_OK | MB_ERROR |MB_MOVEABLE);
  if(bOpenSettings)
    /* Open settings */
    HlpSendCommandToObject("<CWCREATOR_SETTINGS>","OPEN=DEFAULT");
}

#ifdef DEBUG_old
void writeDebugLog(char * text)
{
  FILE *file;

  file=fopen("creator.log","a");
  if(file) {
    fwrite(text,sizeof(char),strlen(text),file);
    fclose(file);
  }
}
#endif

char SysGetBootDriveLetter(void)
{
  ULONG ulSysValue;

  if(!DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,&ulSysValue,sizeof(ulSysValue)))
    return 'a'+ulSysValue-1;

  return 'c';
} 

void SysWriteToTrapLog(const char* chrFormat, ...)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;

  sprintf(logNameLocal,"%c:\\%s", SysGetBootDriveLetter(), EXCEPTION_LOGFILE_NAME);
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {
    va_list arg_ptr;
    void *tb;

    va_start (arg_ptr, chrFormat);
    vfprintf(fHandle, chrFormat, arg_ptr);
    va_end (arg_ptr);
    fclose(fHandle);
  }
}

