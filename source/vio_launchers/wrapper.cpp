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

#define HF_STDERR 2
#define HF_STDOUT 1      /* Standard output handle */

#define PIPESIZE 512
#define GRAB_LOG "grab.log"

  /* argv[0]: progname
   * argv[1]: hwnd 
   * argv[2]: exepath of grabber
   * argv[3]: folder into which to place the log
   * argv[4]: grabber options
   * argv[5]: trackname
   * argv[6]: Grabber ID
   */

char chrInstallDir[CCHMAXPATH];

/* This function removes the file 'logFile'
   from the creator inatallation directory
   */
void removeLog(char* logFile)
{
  char logNameLocal[CCHMAXPATH];

  //  sprintf(logNameLocal,"%s\\Logfiles\\%s",chrInstallDir,logFile);
  sprintf(logNameLocal,"%s\\%s",chrInstallDir,logFile);
  remove(logNameLocal);
}

/* This function appends the text 'logText'
   to the file 'logFile' in the creator installation
   directory.
   */
void writeLog(char* logFile, char* logText)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fStream;

  //sprintf(logNameLocal,"%s\\Logfiles\\%s",chrInstallDir,logFile);
  sprintf(logNameLocal,"%s\\%s",chrInstallDir,logFile);
  fStream=fopen(logNameLocal,"a");
  if(fStream) {
    fputs(logText, fStream);
    fclose(fStream);
  }
}

void message()
{
  printf("This program is (C) Chris Wohlgemuth 1999-2001\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creater WPS classes.\n");
}

void main(int argc, char * argv[])
{
  char * chrPtr;	
	HWND hwndNotify;
	RESULTCODES resultCodes={0};
    PID pidChild;
	ULONG rc, ulAction;
	HFILE hfNew, hf;
    HFILE hfSaveError;
    HPIPE hpR2, hpW2;
    char achBuf[PIPESIZE];
	int a;
	char cmdLine[CCHMAXPATH*4]={0};
    char tempText[CCHMAXPATH*2];
	char chrError[CCHMAXPATH];
	char exeName[CCHMAXPATH];
	char logName[CCHMAXPATH];
    ULONG cbRead, cbWritten;
    int iGrabberID;
    LONG lDone;

    if(argc<7) {
      message();
      exit(-1);
    }
	
	hwndNotify=atol(argv[1]);

    iGrabberID=atoi(argv[6]);

    strcpy(chrInstallDir,argv[3]);

	/* Build grabber input */
	sprintf(exeName,"%s",argv[2]);
	sprintf(logName,"%s\\grab.log",argv[3]);
	
	sprintf(cmdLine,exeName);
	chrPtr=strrchr(cmdLine,0);
	chrPtr++;

	/* Build parameters */
    sprintf(tempText,"%s \"%s\"",argv[4],argv[5]);
    strcat(chrPtr,tempText);

    /* Print some infos into the window */
	printf("HWND: %d",hwndNotify);
	printf("\n");	
	printf("Grabber-executable: %s",exeName);
	printf("\n");
	printf("Logname: %s\n",logName);
	printf("Grabber parameter: %s",chrPtr);
	printf("\n");

    /* Logfile entfernen */
    //removeLog(GRAB_LOG);  
    /* Write args to logfile */
    writeLog(GRAB_LOG, argv[0]);
    writeLog(GRAB_LOG, " started with the following parameters: \n\n");
    for(a=0;a<argc;a++) {
      printf(argv[a]);
      printf("\n");
      writeLog(GRAB_LOG, argv[a]);
      writeLog(GRAB_LOG, "\n");
    }
    writeLog(GRAB_LOG, "\n");

    if(iGrabberID!=IDGRABBER_UNKNOWN)
      {
        fflush(stdout);
        fflush(stderr);
        
        hfSaveError = -1;// Get new handle
        
        switch(iGrabberID)
          {
          case IDGRABBER_LEECH:
            hfNew=HF_STDOUT;/* stdout */
            break;
          default:
            hfNew=HF_STDERR;/* stderr */
            break;
          }
        
        if(!DosDupHandle(hfNew, &hfSaveError)) {
          if(!DosCreatePipe(&hpR2, &hpW2, PIPESIZE)) {
            if(!DosDupHandle(hpW2, &hfNew)){/* Duplicates standard error handle */
              /* Start grabber */
              DosExecPgm(chrError,sizeof(chrError),EXEC_ASYNCRESULT, cmdLine,0,&resultCodes,exeName);
              sprintf(achBuf,"DosExecPgm() for grabber returned: %d\n",rc);
              writeLog(GRAB_LOG, achBuf);
              
              DosClose(hpW2);                       /* Closes write handle to ensure     */
              /* Notification at child termination */
              
              DosDupHandle(hfSaveError, &hfNew);        /* Brings std*** back                */   
              
              /*
               * Read from the pipe
               */          
              do {
                DosRead(hpR2, &achBuf, sizeof(achBuf)-1, &cbRead);

                if(cbRead)
                  achBuf[cbRead]=0;
                else
                  achBuf[0]=0; 

                lDone=0;
                switch(iGrabberID)
                  {
                  case IDGRABBER_CDDA2WAV:
                    if(sscanf(achBuf," %d%1[%]",&lDone, &chrError)!=2)
                      lDone=0;
                    DosWrite(HF_STDOUT, achBuf, cbRead, &cbWritten);
                    break;
                  case IDGRABBER_JCDREAD:
                    if(sscanf(achBuf,"%40s %d",&chrError, &lDone )!=2)
                      lDone=0;
                    DosWrite(HF_STDOUT, achBuf, cbRead, &cbWritten);
                    break;
                  case IDGRABBER_LEECH:
                    if(sscanf(achBuf,"%200[^1234567890]%d%20s %d",&chrError, &lDone, &tempText, &a )!=4)
                      lDone=0;
                    else {                  
                      lDone*=100;
                      lDone/=a;
                    }

                    DosWrite(HF_STDERR, achBuf, cbRead, &cbWritten);
                    break;
                  default:
                    DosWrite(HF_STDOUT, achBuf, cbRead, &cbWritten);
                    break;
                  }
                WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PERCENTGRABBED),MPFROMLONG(lDone));
                

#if 0
                if((chrPtr=strchr(achBuf,'%'))!=NULL) {
                  chrPtr++;
                  *chrPtr='%';
                }
#endif
                writeLog(GRAB_LOG, achBuf);
              } while(cbRead);
              DosClose(hpR2);
              DosWaitChild(0,DCWW_WAIT,&resultCodes, &pidChild,0);
              sprintf(achBuf,"\nGrabber returned: %d\n",resultCodes.codeResult);
              writeLog(GRAB_LOG, achBuf);
              //fprintf(stderr,achBuf);
              
            }/* end of if(!DosDupHandle(hpW2, &hfNew2)) */
          }/* end of if(!DosCreatePipe(&hpR2, &hpW2, PIPESIZE)) */
        }/* end of  if(!DosDupHandle(hfNew, &hfSaveError)) */
        
        WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,0,MPFROMLONG(resultCodes.codeResult));
      }
    else
      {
        /* Unknown grabber. Do not get grab status */ 
       /* Redirect stderr */
        rc=DosOpen(logName,&hf,&ulAction,0,FILE_NORMAL,OPEN_ACTION_CREATE_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
                   OPEN_ACCESS_WRITEONLY|OPEN_SHARE_DENYWRITE,0);
        if(!rc) {
          DosSetFilePtr(hf,0,FILE_END,&ulAction);
          hfNew=HF_STDERR;/* stderr */
          DosDupHandle(hf,&hfNew);
          
          sprintf(logName,"---------------------------------------------------------------------\n");
          write(2,logName,strlen(logName));
          
          
          sprintf(logName,"\n");
          write(2,logName,strlen(logName));	
          
          sprintf(logName,"Starting to grab using %s\n",exeName);
          write(2,logName,strlen(logName));
          
          sprintf(logName,"with the following parameters: %s\n",chrPtr);
          write(2,logName,strlen(logName));
        }
        
        DosExecPgm(chrError,sizeof(chrError),EXEC_SYNC,cmdLine,0,&resultCodes,exeName);
        sprintf(logName,"Return code is: %d\n ",resultCodes.codeResult);
        write(2,logName,strlen(logName));
        /*	sprintf(logName,"---------------------------------------------------------------------\n");
            write(2,logName,strlen(logName));*/
        DosClose(hf);
        
        WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,0,MPFROMLONG(resultCodes.codeResult));
      }
    
}









