/*
 * This file is (C) Chris Wohlgemuth 2000-2001
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

#include <os2.h>
#include <stdio.h>
#include <stdarg.h>
#include "audiofolder.h"

/* *-CD-Creator installation directory */
extern char chrInstallDir[CCHMAXPATH];

/* This function removes the file 'logFile'
   from the creator installation directory
   */
void removeLog(char* logFile)
{
  char logNameLocal[CCHMAXPATH];

  sprintf(logNameLocal,"%s\\%s\\%s",chrInstallDir, LOGDIR_NAME, logFile);
  remove(logNameLocal);
}

/* This function removes the file 'logFile'
   */
void removeLog2( char * installDir, char* logFile)
{
  char logNameLocal[CCHMAXPATH];

  sprintf(logNameLocal,"%s\\%s\\%s",installDir, LOGDIR_NAME, logFile);
  remove(logNameLocal);
}

/* This function appends the text 'logText'
   to the file 'logFile' in the creator installation
   directory.
   */
void writeLog(char* logFile, char* logText)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;

  sprintf(logNameLocal,"%s\\%s\\%s",chrInstallDir, LOGDIR_NAME, logFile);
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {
    fprintf(fHandle,logText);
    fclose(fHandle);
  }
}

void writeLogPrintf(char* logFile, char* format, ...)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;
  va_list vl;
  int args=0;

  sprintf(logNameLocal,"%s\\%s\\%s",chrInstallDir, LOGDIR_NAME, logFile);
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {
    va_start(vl, format);
    vfprintf(fHandle, format, vl);
    va_end(vl);
    fclose(fHandle);
  }
}

/* This function appends the text 'logText'
   to the file 'logFile'
   */
void writeLog2( char * installDir, char* logFile, char* logText)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;

  sprintf(logNameLocal,"%s\\%s\\%s", installDir, LOGDIR_NAME, logFile);
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {
    fprintf(fHandle,logText);
    fclose(fHandle);
  }
}




