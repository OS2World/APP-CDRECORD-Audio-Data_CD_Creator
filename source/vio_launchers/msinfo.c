/* 
 * msinfo.exe (C) Chris Wohlgemuth 1999-2001
 *
 * This wrapper prog starts cdrecord/2 to retrieve multisession information
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

#define HF_STDOUT 1      /* Standard output handle */
#define HF_STDERR 2
#define PIPESIZE 2560

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
	char * chrPtr;	
    char * chrPtr2;
	HWND hwndNotify;
	char exeName[CCHMAXPATH];//="g:\\projects_working\\audiocpy\\cdrecord.exe";
	char chrError[CCHMAXPATH];
	RESULTCODES resultCodes;
	ULONG rc,ulAction;
    int a;

    char puffer[1024];

    LONG lValue1=0;
    LONG lValue2=0;

    /* Vars for check size */
  int iLeadin;
  int iLeadout;
  int size;
  HFILE hfNew;
  HFILE hfSave;
  HPIPE hpR, hpW;
  ULONG cbRead, cbWritten;
  BOOL bBreak=TRUE;
  char *text,*text2;
  CHAR achBuf[PIPESIZE]={0};
  CHAR szFailName[CCHMAXPATH];
  RESULTCODES ChildRC= {0};
  PID pidChild;
  /*
    argv[0]: progname
    argv[1]: hwnd
    argv[2]: cdrecord path
    argv[3]: dev=x,y,z
   */

    /* Have to check if argc is really 4 */
    if(argc<4) {
      message();
      exit(-1);
    }

    /* Print the parameters in VIO window */
    printf("argc:%d\n",argc);  
    for(a=0;a<argc;a++)
      {
        printf("%d: %s\n",a,argv[a]);
      }
    printf("\n");

    hwndNotify=atol(argv[1]);
    /* Error: no cdrecord options given */
    if(argc<4) {
      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(lValue1),MPFROMLONG(lValue2));
      return;
    }

	/* Get input */
    sprintf(exeName,"%s",argv[2]);// cdrecord/2 path

    for(i=3;i<argc;i++) {
      /* Find 'dev=' in option string */
      chrPtr=strstr(argv[i],"dev=");
      if(chrPtr!=NULL)i=argc;
    }


    /* Build cdrecord/2 cmd-line */
    sprintf(cmdLine,"%s",exeName);
    if((chrPtr=strrchr(cmdLine, 0))==NULL) {
      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(0),MPFROMLONG(0));
      return;
    }
	chrPtr++;
	sprintf(chrPtr,"%s -msinfo", argv[3]);

	if(DosCreatePipe(&hpR,&hpW,1024)) {
      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(0),MPFROMLONG(0));
      return;
    }
    if((chrPtr=strrchr(cmdLine, 0))==NULL) {
      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(0),MPFROMLONG(0));
      return;
    }

    /* Save stdout */
    hfSave=-1;
    if(!DosDupHandle(HF_STDOUT,&hfSave)){/* Saves standard  handle      *//* Saves standard  handle      */
      
      /* Redirect stdout */
      hfNew=HF_STDOUT;
      if(!DosDupHandle(hpW,&hfNew)) {
        
        /* start cdrecord/2 */
        rc=DosExecPgm(chrError,sizeof(chrError),EXEC_ASYNC,cmdLine,0,&resultCodes,exeName);
        fprintf(stderr,"DosExecPgm() for CDRecord/2 returned: %d\n",rc);

        DosClose(hpW);                       /* Closes write handle to ensure     */
        /* Notification at child termination */
        DosDupHandle(hfSave, &hfNew);        /* Brings stdout back                */

        /*
         * Read from the pipe and write to the screen
         * as long as there are bytes to read.
         */

        do {
          /* Retrieve output */
          rc=DosRead(hpR,puffer,sizeof(puffer),&cbRead);
          DosWrite(HF_STDERR, puffer, cbRead, &cbWritten);

          if(cbRead>2) {
            lValue1=atol(puffer);// Previous session start sector
            puffer[1023]=0; //Make sure we have a terminating zero!
            chrPtr=strchr(puffer,',');
            chrPtr++;
            lValue2=atol(chrPtr);
          }
        } while(cbRead);

        DosClose(hpR);
        bBreak=FALSE;
      }/* if(!DosDupHandle(hpW,&hfNew)) */
    }/* if(!DosDupHandle(HF_STDOUT,&hfSave)) */
    /* Get leadin and leadout */

    if(!bBreak) {
      /* Now query size */
      sprintf(achBuf,"%s",argv[2]);/* cdrecord path */
      text=strchr(achBuf,0);
      if(!text) {
        WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(0),MPFROMLONG(0));    
      }
      text++;
      sprintf(text,"%s -toc -v", argv[3]);
      printf("Parameters for size query: %s\n",text);
      text=achBuf; 
      printf("Cdrecord path is: %s\n",text);

      /* Redirect stdout */ 
      hfSave = -1;// Get new handle
      hfNew = HF_STDOUT;
      
      if(!DosDupHandle(HF_STDOUT, &hfSave)){    /* Saves standard  handle      */
        if(!DosCreatePipe(&hpR, &hpW, PIPESIZE)){ /* Creates pipe            */
          
          /* If the pipe size is smaller than the output of the child the system() call blocks.
             So create the pipe big enough. */
          
          if(!DosDupHandle(hpW, &hfNew)){           /* Duplicates standard output handle */
            
            /* start CDRecord/2 */
            rc=DosExecPgm(szFailName,sizeof(szFailName),EXEC_ASYNCRESULT, text,NULL,&ChildRC, text);
            fprintf(stderr,"DosExecPgm() for CDRecord/2 returned: %d\n",rc);
            /*
              rc=system(text);
              */
            
            DosClose(hpW);                       /* Closes write handle to ensure     */
            /* Notification at child termination */
            DosDupHandle(hfSave, &hfNew);        /* Brings stdout back                */
            
            /*
             * Read from the pipe and write to the screen
             * as long as there are bytes to read.
             */
            
            do {
              DosRead(hpR, achBuf, sizeof(achBuf), &cbRead);
              DosWrite(HF_STDERR, achBuf, cbRead, &cbWritten);
              
              text=strstr(achBuf,"ATIP");
              
              if(text){
                /* Check if we have a lead in section */
                text2=strstr(text,"in:");
                if(text2) {
                  /* Yes, we have a lead in. Seems to be a blank disk. Find lead out now */
                  iLeadin=0;
                  text2++;
                  text=strstr(text2,"ATIP");/* Second ATIP shows lead out of blank disk */
                  if(text)
                    sscanf(text,"%[^:]: %ld",szFailName,&iLeadout);
                }/* if(text2) */
                else {
                  /* No lead in section. It seems to be a disk with some tracks already written */
                  sscanf(text,"%[^:]: %ld",szFailName,&iLeadout); /* ATIP shows disk lead out */
                  text2=strstr(text,"lout");
                  if(!text2){/* Error */
                    iLeadin=iLeadout;/* Set disk size to zero as error */
                    break;
                  }
                  /* Lead out of track is lead in of next track */
                  sscanf(text2,"%[^:]: %ld",szFailName,&iLeadin);
                }
              }
            } while(cbRead);          
            printf("Leadin is  %d \n",iLeadin); /* We use stdout because stderr is redirected */
            printf("Leadout is  %d \n",iLeadout);
            DosClose(hpR);
          }
        }
      }/* if(!DosDupHandle(HF_STDERR, &hfSave)) */      

      DosWaitChild(DCWA_PROCESS, DCWW_WAIT,&ChildRC, &pidChild, ChildRC.codeTerminate);
      if(ChildRC.codeResult!=0) {
        WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(0),MPFROMLONG(0));
        exit(0);
      }

      /* Calculate free space */
      size=iLeadout-iLeadin;
      if(iLeadin) {
        /* Because iLeadin isn't zero, the disk seems to be a disk with already written session.
           11400 sectors must be substracted from the size as lead in for the next session */
        if(size<11400)
          size=0;
        else
          size-=11400;
      }
    }
    printf("1. sector: %d\n2. sector: %d\n", lValue1, lValue2);

    /* The following kombinations of leadin, out, 1. sector and 2. sector are possible:
       Open CD with previous session:  xxx,  xxx,  ???,  xxx
       Empty CD:                       0  ,  xxx,  0  ,  0
       Closed CD:                      0  ,  0  ,  0  ,  0 (???)
       Closed CD:                      xxx,  xxxx,  0  ,  0
       No CD:                          0  ,  0  ,  0  ,  0
       */
    /* Full or no CD -> Error */
    if(!lValue1 && !lValue2 && !iLeadin && !iLeadout) {
      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(0),MPFROMLONG(0));
      exit(0);
    }
    if(!lValue1 && !lValue2 && iLeadin && iLeadout) {
      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(0),MPFROMLONG(0));
      exit(0);
    }

    /* Empty CD. Mark it by sending 1 for the sectors */
    if(!lValue1 && !lValue2  && iLeadout) {
      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(1),MPFROMLONG(1));
      exit(0);
    }

	/* Send the two values to our notification window */
    WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(lValue1),MPFROMLONG(lValue2));
    exit(0);
}









