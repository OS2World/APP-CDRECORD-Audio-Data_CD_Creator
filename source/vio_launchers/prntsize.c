/*
 * prntsize.cpp (C) Chris Wohlgemuth 1999-2001
 *
 * This helper starts mkisofs to calculate the size of an image.
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
#include <process.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "audiofolder.h" // For shared mem size and ACKEY_*

#define PIPESIZE 8192
#define HF_STDOUT 1      /* Standard output handle */
#define HF_STDERR 2

//#define DEBUG

void message()
{
  printf("This program is (C) Chris Wohlgemuth 1999-2001\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creater WPS classes.\n");
}

int main(int argc, char * argv[])
{
  int rc=0;
  int i;
  HFILE hfNew;
  HWND hwndNotify;
  char *text;
  char *mem;
  char *ptrLocalMem;
  HPIPE hpR, hpW;
  ULONG cbRead, cbWritten;
  CHAR achBuf[PIPESIZE], szFailName[CCHMAXPATH];
  HFILE hfSave;
  long size=0;
  FILE *file;
  UCHAR LoadError[CCHMAXPATH]= {0};
  RESULTCODES ChildRC= {0};

  PID pid;
  FILE* fh;

  /*
    argv[0]=progname
    argv[1]=HWND 
    argv[2]=cdrecord path
    argv[3]=installation dir
    argv[4]=command line datafile
    If launched by pmthefly.exe:
    argv[5]=logfilename
   */

  for(i=0;i<argc;i++)
    printf("%d: %s\n",i, argv[i]);

  if(argc<5) {
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
  
  /* Open logfile */
  if(argc==5) {
    snprintf(LoadError, sizeof(LoadError), argv[0]);
    mem=strrchr(LoadError,'\\');
    *mem=0;
    mem=strrchr(LoadError,'\\');
    *mem=0;
    mem=strcat(LoadError,"\\Logfiles\\imgsize.log");
    fh=fopen(mem,"w");
  }
  else {
    snprintf(LoadError, sizeof(LoadError),"%s\\%s",argv[3], argv[5]);
    fh=fopen(LoadError,"a");
  }
  text=ptrLocalMem;

  printf(text);
  printf("\n\n");
  
  if(fh) {
    fputs("---------------------------------------------------------------\n",fh);
    fputs("This helper calculates the size of the image file.\n",fh);
    fputs("Starting the helper \"",fh);
    fputs(argv[0],fh);
    fputs("\" with the following parameters:\n",fh);
    fputs(text,fh);
    fputs("\n\n",fh);    
  }
  /* Find end of mkisofs path */
  mem=strchr(text,'"');
  *mem=0;
  printf(text);
  printf("\n\n");
  
  
#ifdef DEBUG    
  free(ptrLocalMem);
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(3),MPFROMLONG(size));
  exit(0);
#endif
  
      /* Redirect stderr */ 
      hfSave = -1;// Get new handle
      hfNew = HF_STDERR;
 
      if(!DosDupHandle(HF_STDERR, &hfSave)){    /* Saves standard error handle      */
        if(!DosCreatePipe(&hpR, &hpW, strlen(text)+CCHMAXPATH*2)){ /* Creates pipe            */

          /* If the pipe size is smaller than the command length the system() call blocks, because
             the child process waits for more input which isn't coming through the pipe.
             So the pipe is as big as the command. The same is with the output of the child.
             This all happens because system() blocks until the child ends and so the pipe isn't
             emptied until then. */
      
          if(!DosDupHandle(hpW, &hfNew)){           /* Duplicates standard error handle */
            //rc=system(text);
            //rc=spawnv(P_NOWAIT|P_DEFAULT|P_MAXIMIZE,(char*) *((LONG*)mem2),(LONG*)mem2);
            rc=DosExecPgm(LoadError,sizeof(LoadError),EXEC_ASYNCRESULT, text,NULL,&ChildRC,text);
            printf("DosExecPgm() returned: %d\n",rc);
            if(fh) {        
              if(rc) {
                if(fh) {
                  fputs("DosExecPgm() failed because of module: ",fh);
                  fputs(LoadError,fh);
                  snprintf(LoadError, sizeof(LoadError),"\nDosExecPgm() returned: %d (should be 0)\n",rc);
                  fputs(LoadError,fh);
                }
              }
              else {
                snprintf(LoadError, sizeof(LoadError), "\nDosExecPgm() returned: %d (should be 0)\n",rc);
                if(fh)
                  fputs(LoadError,fh);
              } 
              if(fh)
                fputs("\n\nOutput of mkisofs on stderr:\n\n",fh);
            }
            DosClose(hpW);                       /* Closes write handle to ensure     */
                                                 /* Notification at child termination */
            DosDupHandle(hfSave, &hfNew);        /* Brings stderr back                */
            
            /*
             * Read from the pipe and write to the screen
             * as long as there are bytes to read.
             */          
            
            do {
              DosRead(hpR, achBuf, sizeof(achBuf), &cbRead);
              sscanf(achBuf,"%[^=]= %ld",szFailName,&size);       
              DosWrite(HF_STDOUT, achBuf, cbRead, &cbWritten);
              if(fh)
                fwrite(achBuf,sizeof(char),cbRead,fh);
            } while(cbRead);          
            if(size<=2097152)
              snprintf(LoadError,sizeof(LoadError),"\nEstimated size is %d extents = %d.%0.3d.%0.3d bytes\n",
                       size,size*2048/1000000,(size*2048%1000000)/1000,size*2048%1000);
            else
              snprintf(LoadError,sizeof(LoadError),"\nEstimated size is %d extents = %d Mb\n",
                       size,size*2/1024);

            if(fh)
              fputs(LoadError,fh);
            printf(LoadError);

            DosClose(hpR);
            DosWaitChild(DCWA_PROCESS,DCWW_WAIT,&ChildRC,&pid,0);
          }
        }
      }/* if(!DosDupHandle(HF_STDERR, &hfSave)) */      
      *mem='\"';
      //DosFreeMem(pvSharedMem);
      free(ptrLocalMem);
      if(fh) {
        fputs("---------------------------------------------------------------\n",fh);
        fclose(fh);
      }
   

  //printf("Errorcode from mkisofs is: %d\n",rc);
  //  free(mem2);
  /* Send msg. to the notification window */
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PRINTSIZE),MPFROMLONG(size));
  return 0;
}










