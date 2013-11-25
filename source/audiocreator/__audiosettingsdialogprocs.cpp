/*
 * This file is (C) Chris Wohlgemuth 1999
 * It is part of the Audio/Date-CD-Creator package
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
#include "audiofolder.hh"
#include "audiofolderhelp.h"
#include <stdio.h>
#include <stdlib.h>
#include "cddb.h"

extern PVOID pvAudioSharedMem;

extern BOOL GrabberSetupDone;

extern char chrMpg123Path[CCHMAXPATH];
extern BOOL bMpg123SwabBytes;

extern int iNumCD;
extern char cFirstCD;
extern char chosenCD[3];
extern char chrGrabberPath[CCHMAXPATH];
extern char chrGrabberOptions[CCHMAXPATH];
extern int bTrackNumbers;
extern BOOL bGrabberFirst;
extern char chrInstallDir[CCHMAXPATH];
extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern char chrDataCDROptions[CCHMAXPATH];
extern char chrAudioCDROptions[CCHMAXPATH];
extern LONG lCDROptions;
extern char chosenWriter[3];

extern char chrCdrdaoPath[CCHMAXPATH];
extern char chrCdrdaoDriver[100];
extern char chrDeviceName[CCHMAXPATH];
extern char chrWriterName[CCHMAXPATH];

extern int iBus;
extern int iTarget;
extern int iLun;
extern int iSpeed;
extern int iFifo;

extern BOOL setupDone;
extern ATOM atomUpdateStatusbar;

extern HMODULE hAudioResource;

/* Extern */
HMODULE queryModuleHandle(void);
void sendConfig();
ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);
void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
ULONG launchWrapper(PSZ parameter, PSZ folderPath,HWND hwnd, PSZ wrapperExe, PSZ title="CDRecord/2");
BOOL checkFileExists(char* chrFileName);


/* Local */




