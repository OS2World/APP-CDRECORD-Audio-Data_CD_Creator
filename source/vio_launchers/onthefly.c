/*
 * onthefly.cpp (C) Chris Wohlgemuth 1999-2002
 *
 * This helper starts a write process for onthefly CD creation
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
#define  INCL_DOS
#define INCL_DOSQUEUES   /* Queue values */

#include <os2.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <process.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "audiofolder.h" // For ACKEY_*

#define PIPESIZE 512
#define HF_STDOUT 1      /* Standard output handle */
#define HF_STDERR 2
#define HF_STDIN 0

//#define MKISOFS_PRIO 30

//#define DEBUG

void message()
{
  printf("This program is (C) Chris Wohlgemuth 1999-2002\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creater WPS classes.\n");
}

int main(int argc, char * argv[])
{
  char achBuf[PIPESIZE];
  char achBuf2[PIPESIZE];
  char szFailName[CCHMAXPATH];

  int rc;
  HFILE hfNew;
  HFILE hfSave;
  HPIPE hpR, hpW;

  HFILE hfNew2;
  HFILE hfSaveOutput;
  HPIPE hpR2, hpW2;


  ULONG cbRead, cbRead2, cbWritten;
  HWND hwndNotify;
  char *text;
  char *mem;
  char *ptrLocalMem;
  int a;


  UCHAR LoadError[CCHMAXPATH]= {0};
  RESULTCODES ChildRC= {0};
  PID pidChild;
  FILE* fh;
  char* pipePtr;
  char* fifoPtr;
  FILE * pipe;
  LONG writtenB=0;
  LONG delta=0;
  LONG lNoFix=0;
  int iFifo=0;
  BOOL bPriority=FALSE;
  FILE *file;

  rc=-1;

  /*
    argv[0]=progname
    argv[1]=HWND 
    argv[2]=cdrecord path
    argv[3]=installation dir
    argv[4]=Parameter filename
    argv[5]=logfilename
   */

  for(a=0;a<argc;a++)
    printf("%d:  %s\n", a, argv[a]);
    
  if(argc<6) {
    message();
    exit(-1);
  }  

  hwndNotify=atol(argv[1]);

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

  /* Open log file */
  snprintf(LoadError, sizeof(LoadError), "%s\\%s",argv[3], argv[5]);
  fh=fopen(LoadError,"a");

  /* Get cmd line */
  text=ptrLocalMem;
  
  /* find pipe char */
  pipePtr=strchr(text,'|');
  
  mem=strstr(text,"-print-size");
  for(a=0;a<strlen("-print-size");a++) {
    *mem=' ';
    mem++;
  }
  
  if(fh) {
    fputs("\n---------------------------------------------------------------\n",fh);
    fputs("Starting the helper \"",fh);
    fputs(argv[0],fh);
    fputs("\" with the following parameters:\n",fh);
    fputs(text,fh);
    fputs("\n\n",fh);
  }
  
  /* Find end of mkisofs path */
  mem=strchr(text,'"');
  *mem=0;
  
  if(pipePtr) {
    *pipePtr=0;
    pipePtr++;
    *pipePtr=0;
    pipePtr++;
  }
  /* pipePtr points to begin of cdrecord path */
  if(strstr(pipePtr,"-nofix"))
    lNoFix=1;
  
  /* FIFO size. Needing it to know when FIFO is filled. We raise the priority of the mkisofs
     process then.
     */
  if((fifoPtr=strstr(pipePtr,"fs="))!=NULLHANDLE) {
    fifoPtr+=3;
    iFifo=atoi(fifoPtr);
    iFifo*=1024*1024;
  }        
  
#ifdef DEBUG    
  //      DosFreeMem(pvSharedMem);
  free(ptrLocalMem);
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_ONTHEFLY),MPFROMSHORT(rc));
  exit(0);
#endif

  fflush(stdout);
  fflush(stderr);
  
  hfSaveOutput = -1;// Get new handle
  hfNew2 = HF_STDOUT;
  
  if(!DosDupHandle(HF_STDOUT, &hfSaveOutput)) {
        if(!DosCreatePipe(&hpR2, &hpW2, PIPESIZE)) {
          if(!DosDupHandle(hpW2, &hfNew2)){/* Duplicates standard out handle */

            /* Start mkisofs */
            rc=DosExecPgm(LoadError,sizeof(LoadError),EXEC_ASYNCRESULT, text,NULL,&ChildRC,text);
            sprintf(achBuf,"DosExecPgm() for mkisofs returned: %d\n",rc);
            if(fh)
              fputs(achBuf,fh);
            
            DosClose(hpW2);                       /* Closes write handle to ensure     */
            /* Notification at child termination */
            
            DosDupHandle(hfSaveOutput, &hfNew2);        /* Brings stdout back                */   
            /* pipePtr points to begin of cdrecord path */
            pipe=popen(pipePtr,"wb");

            /*
             * Read from the pipe and write to the screen
             * as long as there are bytes to read.
             */          
            do {
              /* Read from mkisofs */
              DosRead(hpR2, achBuf2, sizeof(achBuf2), &cbRead2);

              /* Read output of cdrecord */
              //DosRead(hpR, achBuf, sizeof(achBuf), &cbRead);
              //DosWrite(HF_STDERR, achBuf, cbRead, &cbWritten);
              //   sscanf(achBuf,"%[^=]= %ld",szFailName,&size);       

              /* Write output of mkisofs */
              if(cbRead2)
                fwrite(achBuf2,sizeof(BYTE),cbRead2,pipe);
              
              delta+=cbRead2;
              writtenB+=cbRead2;
              if(delta>=PIPESIZE*200) {
                delta=0;
                WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MBWRITTEN),MPFROMLONG(writtenB));
              }
              if(writtenB>=iFifo && !bPriority) {
                fputs("FIFO filled, increasing priority of mkisofs process.\n", fh);
                bPriority=TRUE;
                rc=DosSetPriority(PRTYS_PROCESS,PRTYC_TIMECRITICAL,30,ChildRC.codeTerminate);
                sprintf(LoadError,"DosSetPriority() returned: %d (should be 0)\n", rc);
                fputs(LoadError,fh);
                fputs("Increasing priority of the on-the-fly helper process.\n",fh);
                // rc=DosSetPriority(PRTYS_PROCESS,PRTYC_TIMECRITICAL,20,0);
                /* Is this kind of priority setting ok? */
                rc=DosSetPriority(PRTYS_PROCESS,PRTYC_TIMECRITICAL,30,0);
                sprintf(LoadError,"DosSetPriority() returned: %d (should be 0)\n", rc);
                fputs(LoadError,fh);
              }

            } while(cbRead2);

            WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MBWRITTEN),MPFROMLONG(writtenB));
            /*  mark EOF in pipe */        
            ungetc(EOF,pipe);
            /* Tell the PM wrapper that all the data is transfered to cdrecord */
            WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_FIXATING),MPFROMLONG(lNoFix));            
            /* Wait for cdrecord process to end */
            rc=pclose(pipe);
          
            DosClose(hpR2);
        
          }
        }
      }/* if(!DosDupHandle(HF_STDERR, &hfSave)) */
      DosWaitChild(0,DCWW_WAIT,&ChildRC,&pidChild,0);

      free(ptrLocalMem);
      //  DosFreeMem(pvSharedMem);
      if(fh) {
        sprintf(achBuf,"CDRecord returned: %d\n",rc);
        fputs(achBuf,fh);
        fclose(fh);
      }
   
  /* Send msg. to the notification window */
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_ONTHEFLY),MPFROMSHORT(rc));
}


/*****************************************/








