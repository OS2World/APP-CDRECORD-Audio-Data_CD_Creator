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
#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#include "audiofolder.h" 

void changeChar(char* text, char which, char with)
{
  char * chrPtr;
  
  chrPtr=strchr(text,which);

  while(chrPtr!=NULL) {
    *chrPtr=with;
    chrPtr=strchr(text,which);
  }
}

void message()
{
  printf("This program is (C) Chris Wohlgemuth 1999-2001\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creater WPS classes.\n");
}

int main(int argc, char * argv[])
{
	int i;
	char cmdLine[CCHMAXPATH*4]={0};
	char cmdLineBuff[CCHMAXPATH*4];
	char * chrPtr;
	HWND hwndNotify;
	char exeName[CCHMAXPATH];//"g:\\projects_working\\audiocpy\\show.exe";
	char chrError[CCHMAXPATH];
	char logName[CCHMAXPATH];
	RESULTCODES resultCodes;
	ULONG rc,ulAction;
	HFILE hf,hfNew;

    if(argc<3) {
      message();
      exit(-1);
    }
	
    hwndNotify=atol(argv[1]);

	sprintf(exeName,"%s",argv[2]);
	sprintf(logName,"%s\\Logfiles\\write.log",argv[3]);
	
	sprintf(cmdLine,exeName);
	chrPtr=strrchr(cmdLine,0);
	chrPtr++;

	/* Build parameters */
    for(i=4;i<argc;i++) {
      sprintf(cmdLineBuff,"%s",chrPtr);
      sprintf(chrPtr,"%s %s",cmdLineBuff,argv[i]);
    }
    /* Replace ' with " */
    changeChar(chrPtr,'\'','\"');

    /* Replace ^ with space */
    changeChar(chrPtr,'^',' ');

	printf("HWND: %ld",hwndNotify);
	printf("\n");	
	printf("cdrecord-executable: %s",exeName);
	printf("\n");
	printf("Logname: %s\n",logName);
	printf("cdrecord parameter: %s",chrPtr);
	printf("\n");

	/* Redirect stderr */
    rc=DosOpen(logName,&hf,&ulAction,0,FILE_NORMAL,OPEN_ACTION_CREATE_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_ACCESS_WRITEONLY|OPEN_SHARE_DENYWRITE,0);
	if(!rc) {
		DosSetFilePtr(hf,0,FILE_END,&ulAction);
		hfNew=2;
		DosDupHandle(hf,&hfNew);
		
		sprintf(logName,"---------------------------------------------------------------------\n");
		write(2,logName,strlen(logName));

		sprintf(logName,"\n");
		write(2,logName,strlen(logName));	
        
		/*	time(&ltime);		
		sprintf(logName,"%s",ctime(&ltime));
		write(2,logName,strlen(logName));	
		sprintf(logName,"\n");
		write(2,logName,strlen(logName));	*/
    
		sprintf(logName,"Starting to write using %s\n",exeName);
		write(2,logName,strlen(logName));
		
		sprintf(logName,"with the following parameters: %s\n",chrPtr);
		write(2,logName,strlen(logName));
	}
	
	DosExecPgm(chrError,sizeof(chrError),EXEC_SYNC,cmdLine,0,&resultCodes,exeName);
	sprintf(logName,"Return code is: %ld\n ",resultCodes.codeResult);
	write(2,logName,strlen(logName));

    DosClose(hf);
    WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_WRITEONLY),MPFROMLONG(resultCodes.codeResult));
    return 0;
}









