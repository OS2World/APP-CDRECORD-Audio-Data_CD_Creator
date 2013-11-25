/*
 * pmwrtimg.cpp (C) Chris Wohlgemuth 1999-2001
 *
 * This helper handles the GUI stuff for image writing
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

#include <os2.h>

#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#if 0
//#define DEBUG
extern char chrMkisofs[CCHMAXPATH];/* Path to mkisofs */
extern char chrMkisofsOptions[CCHMAXPATH];

extern char chrCDROptions[CCHMAXPATH];
extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern LONG  lCDROptions;
extern LONG lMKOptions;
extern char chrInstallDir[CCHMAXPATH];
extern int iSpeed;
extern int iBus;
extern int iTarget;
extern int iLun;
extern SWP swpWindow;
BOOL bHaveWindowPos=FALSE;
#endif
char   chrDev[20];
char * pipePtr;
char  logName[CCHMAXPATH]="pwrteimg.log";
int numArgs;
char *ptrLocalMem;

PVOID pvScanbusSharedMem;

char* params[10];
  /* argv[0]: progname
   * argv[1]: installdir of Audio/Data-CD-Creator
   * argv[2]: folder
   * argv[3]: imagename
   * argv[4]: Parameter file
   */

HMODULE RESSOURCEHANDLE=0;

void sendCommand(PSZ command);
void removeLog();
void pmUsage();
BOOL buildLogName( char * outBuf, char * logname,ULONG ulSize);
BOOL queryFreeCDSpace2(HWND hwnd, char * chrDev);


int main (int argc, char *argv[])
{
  char logText[CCHMAXPATH];
  short a;
  FILE *file;
  ULONG fl;
  HWND hwndClient;
  char text[CCHMAXPATH+10];
  char title[CCHMAXPATH];

  /* Create a copy of the args */
  /* argv[0]: progname
   * argv[1]: installdir of Audio-CD-Creator
   * argv[2]: foldername
   * argv[3]: imagename
   * argv[4]: Parameter file
   */
  ULONG ulDriveNum=0;
  ULONG ulDriveMap=0;
  APIRET rc;
  int i;

  rc=DosQueryCurrentDisk(&ulDriveNum, &ulDriveMap);
  if(rc!=NO_ERROR) {
    printf("Error\n");
    exit(1);
  }
  printf("Current disk: %c:\n",'A'+ulDriveNum-1);
  printf("Drive map:\n");
  printf("A B C D E F G H I J K L M N O P Q R S T U V W X Y Z\n");

  for(i=0;i<26;i++) {
    printf(( (ulDriveMap << (31-i)) >>31) ? "y ": "- ");
  }
  printf("\n\n");

  DosError(FERR_DISABLEHARDERR);
  for(i=2;i<26;i++) {
    if(( (ulDriveMap << (31-i)) >>31)) {
      char chrDrive[3]="A:";
      BYTE fsqBuf2[sizeof(FSQBUFFER2)+3*CCHMAXPATH]={0};
      PFSQBUFFER2 pfsqBuf2=(PFSQBUFFER2) &fsqBuf2;
      ULONG ulLength;
      /* Get FS */
      chrDrive[0]='A'+i;
      ulLength=sizeof(fsqBuf2);
      rc=DosQueryFSAttach(chrDrive,0L,FSAIL_QUERYNAME, (PFSQBUFFER2)&fsqBuf2, &ulLength);
      if(DosQueryFSAttach(chrDrive,0L,FSAIL_QUERYNAME, (PFSQBUFFER2)&fsqBuf2, &ulLength)==NO_ERROR) {
        if(!strcmp(pfsqBuf2->szName+pfsqBuf2->cbName+1,"ISOFS")) {
          FSINFO fsInfo;
          rc=DosQueryFSInfo(i+1, FSIL_VOLSER, &fsInfo,sizeof(fsInfo));
          printf("%s %s %d %s\n",chrDrive, pfsqBuf2->szName+pfsqBuf2->cbName+1, i+1, fsInfo.vol.szVolLabel); 
        }
      }
      else
        printf("%s %s\n",chrDrive, "---"); 
    }
  }
  DosError(FERR_ENABLEHARDERR);
    
  printf("\n\nDone");
  
  return 0;
}









