/* 
 * (c) Chris Wohlgemuth 1999-2001
 *
 * This wrapper prog starts mkisofs to create an ISO-Image
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

#include "audiofolder.h" // For shared mem size and ACKEY_*

#define PIPESIZE 512
#define HF_STDOUT 1      /* Standard output handle */
#define HF_STDERR 2
#define HF_STDIN 0

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
	HWND hwndNotify;
    char * chrPtr;
	char exeName[CCHMAXPATH];
	char chrError[CCHMAXPATH];
	RESULTCODES ChildRC;
    PID pidChild;
	ULONG rc,ulAction;
    char *ptrLocalMem;
    char *text;
    FILE *file;
    HFILE hfSave,hfNew;
    HPIPE hpR, hpW;
    char achBuf[PIPESIZE];
    ULONG cbRead, cbWritten;
    int buffer;

    for(i=0;i<argc;i++)
      printf("%d: %s\n",i, argv[i]);
    printf("\n");

    if(argc<4) {
      message();
      exit(-1);
    }

    hwndNotify=atol(argv[1]);

    if((ptrLocalMem=malloc(SHAREDMEM_SIZE))==NULL){
      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PRINTSIZE),MPFROMLONG(0));
      exit(255);
    }
    /* Open file with parameters */
    if((file=fopen(argv[3],"rb"))==NULL){
      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PRINTSIZE),MPFROMLONG(0));
      exit(255);
    }
    /* Copy command line to local memory */
    fread(ptrLocalMem, sizeof(char), SHAREDMEM_SIZE, file);
    fclose(file);

    text=ptrLocalMem;   
    /* Build mkisofs cmd-line */
    /* Second zero to mark end */
    printf(text);
    printf("\n\n");
    *(strchr(text,0)+1)=0;
    /* Mark end of mkisofs path */
    *strchr(text,'"')=0;       


    /* Redirect stderr */ 
    hfSave = -1;// Get new handle
    hfNew = HF_STDERR;
    
    if(!DosDupHandle(HF_STDERR, &hfSave)){    /* Saves standard err handle */
      if(!DosCreatePipe(&hpR, &hpW, PIPESIZE)){ /* Creates pipe            */      
        if(!DosDupHandle(hpW, &hfNew)){           /* Duplicates standard err handle */
          
          /* start mkisofs */
          rc=DosExecPgm(chrError,sizeof(chrError),EXEC_ASYNCRESULT,text,NULL,&ChildRC,text);
          fprintf(stderr,"DosExecPgm() for mkisofs returned: %d\n",rc);

          DosClose(hpW);                       /* Closes write handle to ensure     */            
          DosDupHandle(hfSave, &hfNew);        /* Brings stdout back                */

        /*
         * Read from the pipe and write to the screen
         * as long as there are bytes to read.
         */
        do {
          DosRead(hpR, achBuf, sizeof(achBuf), &cbRead);
          rc=sscanf(achBuf,"%d",&buffer); 
          if(rc==1) {
            /* Use the following for status */
            //fprintf(stdout,"found: %d",buffer);
            //WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MBWRITTEN),MPFROMLONG(written*1024*1024));
          }
          DosWrite(HF_STDOUT, achBuf, cbRead, &cbWritten);
        }while(cbRead);           
        DosClose(hpR);
        DosWaitChild(0,DCWW_WAIT,&ChildRC,&pidChild,0);
        }
      }
    }/* if(!DosDupHandle(HF_STDERR, &hfSave)) */      

    free(ptrLocalMem);
    /* Send msg. to the notification window */
    WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_IMAGEONLY),MPFROMLONG(ChildRC.codeResult));
}









