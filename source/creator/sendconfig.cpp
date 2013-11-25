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
#include "audiofolder.hh"
#include "stdio.h"

extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern char chrDataCDROptions[CCHMAXPATH];
extern LONG lCDROptions;
extern char chrInstallDir[CCHMAXPATH];/* Dir with all the helpers */

extern int iNumCD;
extern char cFirstCD;
extern char chosenWriter[3];

/*
 * This function sends the cdrecord config info
 * to the CWDisk class.
 */
void sendConfig()
{
  int a,b;
  HOBJECT hObject;
  char chrCD[4];
  char driveID[40];
  char chrOptions[2*CCHMAXPATH];

  b=cFirstCD-'A'+iNumCD;
  /* Unset writeflag of diskobjects */
  for( a=0; a<b ;a++) {
    chrCD[0]=a+'A';
    chrCD[1]=0;
    /* Build ID of drive object */
    sprintf(driveID,"<WP_DRIVE_%s>",chrCD);
    /* Clear writer flag of disk objects. Clear CD-Rom flag */
    if((hObject=WinQueryObject(driveID))!=NULL) {
      WinSetObjectData(hObject,"ISWRITER=0;ISCDROM=0");
    }
  }

  for( a=0; a<iNumCD ;a++) {
    chrCD[0]=cFirstCD+a;
    chrCD[1]=0;
    /* Build ID of CD-Drive object */
    sprintf(driveID,"<WP_DRIVE_%s>",chrCD);
    /* Clear writer flag of disk objects. Set CD-Rom flag */
    if((hObject=WinQueryObject(driveID))!=NULL) {
      WinSetObjectData(hObject,"ISCDROM=1");
    }
  }               
  /* Query the writer-drive */
  sprintf(chrCD,"%s",chosenWriter);
  chrCD[1]=0;
  /* Build ID of chosenCD-Drive object */
  sprintf(driveID,"<WP_DRIVE_%s>",chrCD);
  /* Set writer flag of this disk object */
  if((hObject=WinQueryObject(driveID))!=NULL) {
    WinSetObjectData(hObject,"ISWRITER=1");
    sprintf(chrOptions,"CDRPATH=%s;CDROPTIONS=%s;CDREXECUTION=%d",chrCDRecord,chrDataCDROptions,lCDROptions);// Path and cdrecord options
    WinSetObjectData(hObject,chrOptions);
    
    sprintf(chrOptions,"INSTALLDIR=%s",chrInstallDir);
    WinSetObjectData(hObject,chrOptions);
    /*    WinMessageBox(  HWND_DESKTOP,
          HWND_DESKTOP,
          chrInstallDir,
          chrOptions,
          0UL,
          MB_YESNO | MB_ICONQUESTION );*/
  }
}

