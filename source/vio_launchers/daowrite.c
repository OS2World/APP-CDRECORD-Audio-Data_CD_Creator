/*
 * daowrite.c (C) Chris Wohlgemuth 1999-2002
 *
 * This helper starts a write process for writing an audio CD
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
#define INCL_DOSERRORS
#define  INCL_DOSQUEUES   /* Queue values */

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

//#define DEBUG

void message()
{
  printf("This program is (C) Chris Wohlgemuth 1999\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creater WPS classes.\n");
}

int main(int argc, char * argv[])
{
  char logName[CCHMAXPATH];
  char launchError[CCHMAXPATH];
  char logText[CCHMAXPATH*5];
  char parameters[4096]={0};
  char achBuf[PIPESIZE];
  char buffer[PIPESIZE];
  char buffer2[PIPESIZE];
  int rc;
  HFILE hfNew;
  HFILE hfSave;
  HPIPE hpR, hpW;

  ULONG cbRead, cbWritten;
  HWND hwndNotify;
  char *text;
  char *mem;

  int numArgs=1;
  int a;
  SHORT written;
  SHORT fifo;
  SHORT total;
  int prio=0;

  HFILE hf=0;        /* Logfile handle */
  ULONG ulAction;  /* for DosOpen()  */
  RESULTCODES ChildRC= {0};
  PID pidChild;
  FILE* fh;/* Logfile handle */

  ChildRC.codeResult=-1;

  if(argc<4) {
    message();
    printf("DONE");
    exit(-1);
  }  


  for(a=0;a<argc;a++)
    {
      printf("%d: %s\n",a,argv[a]);
    }


  if(1)
    {
      snprintf(logName, sizeof(logName),"%s",argv[0]);
      printf(logName);
      snprintf(parameters, sizeof(parameters),"%s",argv[1]);
      text=strrchr(parameters,0);
      text++;
      snprintf(text, sizeof(parameters)-strlen(parameters)-2,"%s \"%s\"",argv[2], argv[3]);

      printf("\nCdrdao/2 path is:\n");
      printf(argv[1]);
      printf("\n\n");
      printf("******************************************\n\n");
      printf("Starting cdrdao/2 with the following parameters:\n");
      printf(text);
      printf("\n\n");
      /*
printf("DONE");
exit(1);*/
#ifdef DEBUG
      DosFreeMem(pvSharedMem);
      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_WRITEAUDIO),MPFROMLONG(0));
      exit(0);
#endif
      /* Set priority to timecritical 0 */
      DosSetPriority(PRTYS_PROCESS,PRTYC_TIMECRITICAL,0,0);

      /* Redirect stderr */ 
      hfSave = -1;// Get new handle
      hfNew = HF_STDERR;
      
      if(!DosDupHandle(HF_STDERR, &hfSave)){    /* Saves standard error handle          */
        if(!DosCreatePipe(&hpR, &hpW, PIPESIZE)){ /* Creates pipe                       */      
          if(!DosDupHandle(hpW, &hfNew)){           /* Duplicates standard error handle */
            /* start cdrdao */
            rc=DosExecPgm(launchError,sizeof(launchError),EXEC_ASYNCRESULT,parameters,NULL,&ChildRC, parameters);
     
            fprintf(stdout,"DosExecPgm() for cdrdao returned: %d\n",rc);
   
            DosClose(hpW);                       /* Closes write handle to ensure     */
            
            DosDupHandle(hfSave, &hfNew);        /* Brings stdout back                */
            
            /*
             * Read from the pipe and write to the screen
             * as long as there are bytes to read.
             */    
            /* Open logfile */
            rc=DosOpen(logName,&hf,&ulAction,0,FILE_NORMAL,OPEN_ACTION_CREATE_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
                       OPEN_ACCESS_WRITEONLY|OPEN_SHARE_DENYWRITE,0);
            if(rc==NO_ERROR)
              DosSetFilePtr(hf,0,FILE_END,&ulAction);
            do {
              DosRead(hpR, achBuf, sizeof(achBuf), &cbRead);
              if( 3==sscanf(achBuf,"%*[^e]e %hd %*[^f]f %hd %*[^(](Buffer %hd",&written,&total,&fifo)) {
                if(fifo<85) {
                  /* Raise priority */
                  if(prio<31) {
                    DosSetPriority(PRTYS_PROCESSTREE,PRTYC_NOCHANGE,1,0);
                    prio++;
                    //fprintf(stdout,"Increasing priority to %d\n",prio);
                  }/* if(prio>=31) */
                }/* if(fifo<85) */
                else {
                  if(fifo>90) {/* Hysterese */
                    /* Lower priority */
                    if(prio>0) {
                      DosSetPriority(PRTYS_PROCESSTREE,PRTYC_NOCHANGE,-1,0);
                      prio--;
                      //fprintf(stdout,"Decreasing priority to %d\n",prio);
                    }
                  }
                }
              }
#if 0
              if( 2==sscanf(achBuf,"%*[^e]e %hd %*[^f]f %hd %*[^(](Buffer %hd",&written,&total,&fifo)) {  
                /* Leadin or Leadout */
                /* Leadin:   total = 25MB
                   Leadout:  total = 15MB */
                if(total==25)
                  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_LEADIN),MPFROMSHORT(written));
                else
                  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_LEADOUT),MPFROMSHORT(written));
              }
#endif
              DosWrite(HF_STDOUT, achBuf, cbRead, &cbWritten);
              /* Write output into logfile */
              if(hf) {
                DosWrite(hf, achBuf, cbRead, &cbWritten);
              }
            } while(cbRead);      
            if(hf)
              DosClose(hf);
            DosClose(hpR);
          }
        }
      }/* if(!DosDupHandle(HF_STDERR, &hfSave)) */      
      DosWaitChild(0,DCWW_WAIT,&ChildRC,&pidChild,0);  
      
      fh=fopen(logName,"a");
      if(fh) {
        fputs("\n",fh);
        sprintf(logText,"Returncode from cdrdao/2 is: %d\n\n",ChildRC.codeResult);
        fputs(logText,fh);
        fputs("******************************************\n\n",fh);
        fclose(fh);
      }
    }/* end of if(!DosGetNamedSharedMem(&pvSharedMem,"\\SHAREMEM\\AUDIOCMDLINE",PAG_READ|PAG_WRITE)) */

  printf("RETURN %d\n", ChildRC.codeResult);
  printf("DONE");
  exit(ChildRC.codeResult);
}






