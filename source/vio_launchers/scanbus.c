/*
 * scanbus.exe (C) Chris Wohlgemuth 1999-2004
 *
 * This helper scans the bus for devices and sends them
 * to the creator settings.
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
#define INCL_DOSQUEUES   /* Queue values */
#define INCL_PM
 
#include <os2.h>
#include <process.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "audiofolder.h"
#include "audiofolderres.h"

#define LOGFILE_NAME "scanbus.log"

#define MAX_DEVICES 16

char chrInstallDir[CCHMAXPATH];
char chrDevices[MAX_DEVICES][20];

/*
  Printed if the program was started by hand
  */ 
void message()
{
  printf("This program is (C) Chris Wohlgemuth 2000\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creater WPS classes.\n");
}

/* This function removes the file 'logFile'
   from the creator logfile directory
   */
void removeLog(char* logFile)
{
  char logNameLocal[CCHMAXPATH];

  sprintf(logNameLocal,"%s\\Logfiles\\%s",chrInstallDir,logFile);
  remove(logNameLocal);
}

/* This function appends the text 'logText'
   to the file 'LOGFILE_NAME' in the creator installation
   directory.
   */
void writeLog(const char* chrFormat, ...)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;

  sprintf(logNameLocal,"%s\\Logfiles\\%s", chrInstallDir, LOGFILE_NAME);
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {
    va_list arg_ptr;

    va_start (arg_ptr, chrFormat);
    vfprintf(fHandle, chrFormat, arg_ptr);
    va_end (arg_ptr);
    fclose(fHandle);
  }
}


int main(int argc, char * argv[])
{
  int rc;
  HWND hwndNotify;
  CHAR szName[CCHMAXPATH];
  PVOID pvSharedMem;
  char *buf;

  BOOL bBus=FALSE;
  int bus,target,lun;
  char vendor[CCHMAXPATH];
  char type[CCHMAXPATH];
  int a;

  rc=-1;

  /* Check for proper params */
  if(argc<4) {
    message();
    exit(-1);
  }  

  /* We send our info to this window (usually the CD-creator settings page) */
  hwndNotify=atol(argv[1]);

  /* Save installdir */
  strcpy(chrInstallDir,argv[3]);

  /* Remoce logfile */
  removeLog(LOGFILE_NAME);  

  /* Log all the programs parameters */
  sprintf(szName, "argc:%d\n",argc);
  writeLog(szName);  
  printf(szName);
  for(a=0;a<argc;a++)
    {
      writeLog("%d: %s\n", a, argv[a]);  
      printf("%d: %s\n", a, argv[a]);
    }

  /* Build cdrecord command line */
  sprintf(szName,"%s -scanbus",argv[2]);
  printf("Command is: %s\n",szName);
  writeLog("\nCommand is: %s\n\n",  szName);  

  /* Get the shared mem for communication */
  if(!DosGetNamedSharedMem(&pvSharedMem, SCANBUSSHAREDMEM_NAME, PAG_READ|PAG_WRITE))
    {
      char *text;
      FILE *fStream;
      int curDevice=0;

      text=pvSharedMem;
      /* start scanning */
      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_LISTBOX),MPFROMLONG(0));
      writeLog("Drop down listbox cleared.\n\n");
      
      fStream=popen(szName,"r");
      if(fStream) {
        do {
          char achBuf[CCHMAXPATH];
         
          /* memset(achBuf, 0, sizeof(achBuf)); */
          buf=fgets(achBuf,sizeof(achBuf),fStream);
          if(buf) {
            if(!strchr(achBuf,'*')) { /* Skip line without devices */
              if(bBus && !strstr(achBuf,"scsibus")) {
                ULONG count;

                writeLog("Found device entry: %s", achBuf);
                count=sscanf(achBuf," %d,%d,%d %*[^']'%[^']' '%[^']",&bus,&target,&lun, vendor, type);
                if(count==5) {
                  sprintf(achBuf,"%d,%d,%d : '%s' '%s'",bus,target,lun,vendor,type);
                  sprintf(chrDevices[curDevice++], "%d,%d,%d", bus,target,lun );
                  strcpy(text,achBuf);
                  writeLog("Inserting following entry into listbox: %s\n\n", text);
                  /* Posting the message is asynchronous. To make sure the device name isn't
                     overriden before the settings notebook can read it, the pointer is advanced
                     for every new device. See the text++ statement below. Will break if names of
                     all devices are longer than the shared mem size... */
                  /* Send the settings page the device name */
                  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_LISTBOX),MPFROMP(text));
                  text=strchr(text,0);
                  text++; /* Advance pointer in the shared mem area */
                }/* if(count==5) */ 
              }/* if(bBus && !strstr(achBuf,"scsibus")) */
              if(strstr(achBuf,"scsibus")) /* Device section starts */
                bBus=TRUE;
            }/* if(!strchr(achBuf,'*')) */
          }
        }while(buf);
        fclose(fStream);
      }

      /* Found all devices on the bus. Now ask for their capabilities. */
      for(a=0;a <curDevice; a++) {
        writeLog("Getting capabilities of %s:\n", chrDevices[a]);

        /* Build cdrecord command line */
        sprintf(szName,"%s dev=%s -prcap",argv[2], chrDevices[a]);
        printf("Command is: %s\n",szName);
        writeLog("  Command is: %s\n",szName);
        fStream=popen(szName,"r");
        if(fStream) {
          do {
            char achBuf[CCHMAXPATH];
            
            buf=fgets(achBuf,sizeof(achBuf),fStream);
            if(buf) {
              /* Get maximum write speed */
              if(strstr(strupr(achBuf),"MAXIMUM WRITE SPEED")) {
                ULONG count;
                USHORT usSpeed;

                count=sscanf(achBuf," %*[^(](CD %hu",&usSpeed);
                if(count==1) {
                  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,
                             MPFROMLONG(ACKEY_MAXWRITESPEED),MPFROM2SHORT(a, usSpeed));
                  writeLog("  Max write speed is: %d\n", usSpeed);
                }/* count */
              }
            }/* If(buf) */
            /* Get burnproof feature */
            if(strstr(strupr(achBuf),"BUFFER-UNDERRUN-FREE")) {
              if(strstr(strupr(achBuf),"NOT SUPPORT")) {
                /* Seems not to support burnproof or alike */
                WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,
                           MPFROMLONG(ACKEY_BURNSAVE), MPFROM2SHORT(a, LBHANDLE_FEATURE_NOBURNPROOF));
                writeLog("  Drive seems NOT to support Buffer-Underrun-Free recording.\n");
              }
              else {
                writeLog("  Drive seems to support Buffer-Underrun-Free recording.\n");
              }
            }
        }while(buf);
          fclose(fStream);
        }/* if(fStream) */
      }

      writeLog("\nScanbus done.\n\n");
      DosFreeMem(pvSharedMem);
    }/* DoaAllocSharedMem() */
  else
    {
      writeLog("Can't allocate shared mem!\n");
    }
  /* Tell settings we are done */
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_SCANBUS),MPFROMLONG(0));
  return 0;
}










