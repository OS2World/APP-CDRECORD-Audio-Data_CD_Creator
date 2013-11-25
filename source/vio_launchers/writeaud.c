/*
 * writeaud.c (C) Chris Wohlgemuth 1999-2000
 *
 * This helper starts a write process for writing an audio track
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

char logFileName[CCHMAXPATH]="WriteTAO.log";

void message()
{
  printf("This program is (C) Chris Wohlgemuth 1999\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creater WPS classes.\n");
}

int main(int argc, char * argv[])
{
  char logName[CCHMAXPATH];
  char logText[CCHMAXPATH*5];
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
  char *mem2;
  PVOID pvSharedMem;
  int numArgs=1;
  int a;
  LONG written;
  int fifo;
  time_t    tme;

  HFILE hf=0;        /* Logfile handle */
  ULONG ulAction;  /* for DosOpen()  */
  RESULTCODES ChildRC= {0};
  PID pidChild;

  rc=-1;
  //DosSleep(5000);  
  if(argc<4) {
    message();
    exit(-1);
  }  


  printf("argc:%d\n",argc);  
  for(a=0;a<argc;a++)
    {
      printf("%d: %s\n",a,argv[a]);
    }

  
  hwndNotify=atol(argv[1]);
  /* Put the logfile into the installation folder */
  strcpy(logName,argv[0]);
  mem=strrchr(logName,'\\');
  *mem=0;
  mem=strrchr(logName,'\\');
  *mem=0;
  strcat(logName,"\\Logfiles\\");
  strcat(logName,logFileName);

#ifdef DEBUG
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_WRITEAUDIO),MPFROMLONG(0));
  exit(0);
#endif

  /* Open logfile */
  rc=DosOpen(logName,&hf,&ulAction,0,FILE_NORMAL,OPEN_ACTION_CREATE_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
	     OPEN_ACCESS_WRITEONLY|OPEN_SHARE_DENYWRITE,0);
  
  if(rc==NO_ERROR)
    DosSetFilePtr(hf,0,FILE_END,&ulAction);
  if(hf) {
    sprintf(logText,"******************************************\n\n");
    DosWrite(hf, logText, strlen(logText), &cbWritten);
    sprintf(logText,"Writeaud.exe started...\n\n");
    DosWrite(hf, logText, strlen(logText), &cbWritten);
    sprintf(logText,"Starting CDRecord/2 with the following parameters:\n");
    DosWrite(hf, logText, strlen(logText), &cbWritten); 
    for(a=0;a<argc;a++)
      {
	DosWrite(hf, argv[a], strlen(argv[a]), &cbWritten); 
	DosWrite(hf, "\n", strlen("\n"), &cbWritten); 
      }
  }        


  sprintf(logText,"%s",argv[2]);
  text = strchr(logText,'\0')+1;

  sprintf(text,"%s \"%s\"",argv[4],argv[5]);

  *(strchr(text,'\0')+1) = '\0';
  printf(text);
  printf("\n");

  DosWrite(hf, "\n", strlen("\n"), &cbWritten); 
  /*DosWrite(hf, logText, strlen(logText), &cbWritten);*/
  /* Redirect stdout */ 
  hfSave = -1;// Get new handle
    hfNew = HF_STDOUT;
  
    if(!DosDupHandle(HF_STDOUT, &hfSave)){    /* Saves standard output handle */
      if(!DosCreatePipe(&hpR, &hpW, PIPESIZE)){ /* Creates pipe            */      
        if(!DosDupHandle(hpW, &hfNew)){           /* Duplicates standard output handle */
          /* start CDRecord/2 */
          rc=DosExecPgm(logName,sizeof(logName),EXEC_ASYNCRESULT,logText,NULL,&ChildRC,logText);
     
          fprintf(stderr,"DosExecPgm returned: %d\n",rc);
          DosClose(hpW);                       /* Closes write handle to ensure     */
          
          DosDupHandle(hfSave, &hfNew);        /* Brings stdout back                */
          
          /*
           * Read from the pipe and write to the screen
           * as long as there are bytes to read.
           */          
          do {
            DosRead(hpR, achBuf, sizeof(achBuf), &cbRead);
            //            if( 3!=sscanf(achBuf,"%[^:]:  %ld %[^o]o %d",buffer,&written,buffer2,&fifo)) {       
            if( 2==sscanf(achBuf,"%*[^:]: %ld %*[^(](fifo %d",&written,&fifo)) {       
              WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MBWRITTEN),MPFROMLONG(written));
            }
            DosWrite(HF_STDERR, achBuf, cbRead, &cbWritten);
            /* Write output into logfile */
            if(hf) {
              DosWrite(hf, achBuf, cbRead, &cbWritten);
            }
          } while(cbRead);           
          DosClose(hpR);
        }
      }
    }/* if(!DosDupHandle(HF_STDERR, &hfSave)) */      
    DosWaitChild(0,DCWW_WAIT,&ChildRC,&pidChild,0);  
    if(hf) {
      sprintf(logText,"Returncode from CDRecord/2 is: %d\n\n",ChildRC.codeResult);
      DosWrite(hf, logText, strlen(logText), &cbWritten);
      sprintf(logText,"******************************************\n\n");
      DosWrite(hf, logText, strlen(logText), &cbWritten);
      DosClose(hf);
    }        

    /* Send msg. to the notification window with result code */
    WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_WRITEAUDIO),MPFROMSHORT(ChildRC.codeResult));
    exit(rc);
}




