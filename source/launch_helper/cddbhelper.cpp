/*
 * This file is (C) Chris Wohlgemuth 1999/2000
 */
/* These functions are used by PMCDDB to output error messages and
   CDDB data to the appropriate log and data files.
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
#include <stdio.h> /* For stderr */
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

extern HWND hwndError;
extern char cddbDataFile[];

/**********************************************************/
/* Some helper functions
 */

void extern printError(char* errorString)
{
  char buffer[CCHMAXPATH];
  char *ptr;

  fprintf(stderr,"%s",errorString);

  writeLog(errorString);
  if(hwndError) {
    strncpy(buffer, errorString, sizeof(buffer));
    ptr=strchr(buffer,'\n');
    if(ptr) {
      *ptr=0;
    }
  }
}

void extern printErrorCR(char* errorString)
{
  fprintf(stderr,"%s\n",errorString);
  writeLog(errorString);
  writeLog("\n");
}


void extern printData(char* outString)
{
  int fileHandle;
  int rc;

  /* The name cddbDataFile is defined in the main pmcddb.cpp file. It is 
     the discid (this is the native format of the data hold by a cddb server) */
  fileHandle=open(cddbDataFile,O_APPEND|O_TEXT|O_WRONLY|O_CREAT,S_IWRITE);
  if(fileHandle==-1) {
    printError("Cannot open cddb outfile");/* Disk full? */
    return;
  }
  rc= write(fileHandle,outString,strlen(outString));
  // write(fileHandle,"\n",sizeof("\n"));
  close(fileHandle);
  return;
}











