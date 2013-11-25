/*
 * This file is (C) Chris Wohlgemuth 1999-2001
 *
 * This helper handles the image writing
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
#define INCL_DOSERRORS
#define INCL_WIN

#include <os2.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#include "audiofolder.h" 

#define PIPESIZE 512
#define HF_STDOUT 1      /* Standard output handle */
#define HF_STDERR 2
#define HF_STDIN 0

PVOID pvSharedMem=NULL;

//#define DEBUG

void message()
{
  printf("This program is (C) Chris Wohlgemuth 1999-2000\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creater WPS classes.\n");
}


/*
  argv[0]: progname
  argv[1]: hwndNotify
  argv[2]: cdrecord path
  argv[3]: installdir
  argv[4]: Parameterfile
  argv[5]: logfile
 */

int main(int argc, char * argv[])
{
  char * text;
  char logText[CCHMAXPATH*5];
  char * chrPtr;	
  HWND hwndNotify;
  char logName[CCHMAXPATH];
  UCHAR LoadError[CCHMAXPATH]= {0};
  
  RESULTCODES ChildRC= {0};
  PID pidChild;
  ULONG ulAction;
  
  HFILE hfSave,hfNew;
  HPIPE hpR, hpW;
  FILE* fh;
  char achBuf[PIPESIZE];
  ULONG cbRead, cbWritten;
  char buffer[PIPESIZE];
  char buffer2[PIPESIZE];
  char buffer3[PIPESIZE];
  LONG written;
  LONG lOf;
  int fifo;
  int rc;
  HFILE hf=0;        /* Logfile handle */
  int a;

  char *ptrLocalMem;
  FILE *file;

  /* 
  argv[0]: progname
  argv[1]: hwndNotify
  argv[2]: cdrecord path
  argv[3]: installdir
  argv[4]: Parameterfile
  argv[5]: logfile
   */

  for(a=0;a<argc;a++)
    printf("%d: %s\n",a, argv[a]);
  printf("\n");

  if(argc==1) {
    message();
    exit(-1);
  }

  hwndNotify=atol(argv[1]);
  if(!hwndNotify) {
    message();
    exit(-1);
  }
  
  snprintf(logName, sizeof(logName),"%s\\%s",argv[3], argv[5] );
  printf(logName);
  printf("\n");

  if((ptrLocalMem=malloc(SHAREDMEM_SIZE))==NULL){
    WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PRINTSIZE),MPFROMLONG(0));
    exit(255);
  }
  /* Open file with parameters */
  if((file=fopen(argv[4],"rb"))==NULL){
    WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PRINTSIZE),MPFROMLONG(0));
    exit(255);
  }
  
  /* Copy command line to local memory */
  fread(ptrLocalMem, sizeof(char), SHAREDMEM_SIZE, file);
  fclose(file);

  text=ptrLocalMem;
  
  /* Open log file and write options */
  fh=fopen(logName,"a");
  if(fh) {
    fputs("\n---------------------\n",fh);
    fputs("writeimg.exe was started with the following parameters:\n",fh);
    for(a=0;a<argc;a++)
      {
        sprintf(logText,"%d: %s\n",a,argv[a]);
        fputs(logText,fh);
      }
    sprintf(logText,"argc: %d\n\n",argc);  
    fputs(logText,fh);
    sprintf(logText,"Launching now CDRecord/2 with the following parameters: %s\n\n",
            text);
    fputs(logText,fh);
  }
  
  /* Find end of cdrecord.exe path */
  chrPtr=strchr(text,'"');
  *chrPtr=0;
  chrPtr++;
      
#ifdef DEBUG    
  /* Print the options */
  printf("\n");
  printf(text);
  printf("\n");
  printf(chrPtr);
  free(ptrLocalMem);
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_WRITEONLY),MPFROMLONG(0));
  exit(0);
#endif
      
  /* Redirect stdout */ 
  hfSave = -1;// Get new handle
  hfNew = HF_STDOUT;
  
  if(!DosDupHandle(HF_STDOUT, &hfSave)){    /* Saves standard output handle */
    if(!DosCreatePipe(&hpR, &hpW, PIPESIZE)){ /* Creates pipe            */      
      if(!DosDupHandle(hpW, &hfNew)){           /* Duplicates standard output handle */
        /* start CDRecord/2 */
        rc=DosExecPgm(LoadError,sizeof(LoadError),EXEC_ASYNCRESULT, text,NULL,&ChildRC,text);
        fprintf(stderr,"DosExecPgm() for CDRecord/2 returned: %d\n",rc);
        /* Writing to log */
        sprintf(logText,"DosExecPgm() for CDRecord/2 returned: %d (should be 0).\n\n", rc);
        if(fh) {
          fputs(logText,fh);
          fputs("Output of CDRecord/2:\n\n\n\n",fh);
          fclose(fh);
        }
        
        DosClose(hpW);                       /* Closes write handle to ensure     */            
        DosDupHandle(hfSave, &hfNew);        /* Brings stdout back                */
        
        /*
         * Read from the pipe and write to the screen
         * as long as there are bytes to read.
         */
        /* Open logfile */
        rc=DosOpen(logName,&hf,&ulAction,0,FILE_NORMAL,OPEN_ACTION_CREATE_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
                   OPEN_ACCESS_WRITEONLY|OPEN_SHARE_DENYNONE,0);
        if(rc==NO_ERROR) {
          DosSetFilePtr(hf,0,FILE_END,&ulAction);            
        }          
        
        do {
          DosRead(hpR, achBuf, sizeof(achBuf), &cbRead);
          rc=sscanf(achBuf,"%[^:]:  %ld %[^f]f  %ld %[^o]o %d",buffer,&written,buffer2,lOf,buffer3,&fifo); 
          if(5==rc || 4==rc) {
            WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MBWRITTEN),MPFROMLONG(written));
          }
          // if(strstr(strlwr(achBuf),"fixating"))
          //  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(22),MPFROMLONG(0));
          //        fprintf(stderr,"*********** Fixating... found ***********\n");
          DosWrite(HF_STDERR, achBuf, cbRead, &cbWritten);
          /* Write output into logfile */ 
          if(hf) {
            DosWrite(hf, achBuf, cbRead, &cbWritten);
          }
        }while(cbRead);           
        DosClose(hpR);
        if(hf)
          DosClose(hf);
        DosWaitChild(0,DCWW_WAIT,&ChildRC,&pidChild,0);
        sprintf(logText,"\nCDRecord/2 returned: %d  (Should be 0).\n", ChildRC.codeResult);
        
        /* Open log file */
        fh=fopen(logName,"a");
        if(fh) {
          fputs(logText,fh);
          fclose(fh);
        }
      }
    }
  }/* if(!DosDupHandle(HF_STDOUT, &hfSave)) */      
  
  free(ptrLocalMem);
  /* Send msg. to the notification window with result code */
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_WRITEONLY),MPFROMLONG(ChildRC.codeResult));
  exit(rc);
}









