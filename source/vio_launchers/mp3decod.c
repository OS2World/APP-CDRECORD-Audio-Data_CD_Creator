/*
 * mp3dec.exe (C) Chris Wohlgemuth 1999-2002
 *
 * This helper starts mpg123 for decoding a mp3 file.
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
#define INCL_DOSQUEUES   /* Queue values */
#define INCL_OS2MM
#define INCL_MMIOOS2

#include <os2.h>
#include <os2me.h>
#include <process.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "audiofolder.h" // For ACKEY_*

#define PIPESIZE 512
#define HF_STDOUT 1      /* Standard output handle */
#define HF_STDERR 2

#undef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                         \
                  ( (ULONG)(BYTE)(ch0) | ( (ULONG)(BYTE)(ch1) << 8 ) |    \
                  ( (ULONG)(BYTE)(ch2) << 16 ) | ( (ULONG)(BYTE)(ch3) << 24 ) )

//#define DEBUG

char chrInstallDir[CCHMAXPATH];
//char buffer[5000000]={0};
char buffer[500000]={0};
/* This function removes the file 'logFile'
   from the creator inatallation directory
   */
void removeLog(char* logFile)
{
  char logNameLocal[CCHMAXPATH];

  sprintf(logNameLocal,"%s\\Logfiles\\%s",chrInstallDir,logFile);
  remove(logNameLocal);
}

/* This function appends the text 'logText'
   to the file 'logFile' in the creator installation
   directory.
   */
void writeLog(char* logFile, char* logText)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;

  sprintf(logNameLocal,"%s\\Logfiles\\%s",chrInstallDir,logFile);
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {
    fprintf(fHandle,logText);
    fclose(fHandle);
  }
}

void message()
{
  printf("This program is (C) Chris Wohlgemuth 1999\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creator WPS classes.\n");
}


int main(int argc, char * argv[])
{
  int rc;
  HWND hwndNotify;
  char *text,*text2;
  CHAR szCommand[CCHMAXPATH*4]={0};
  UCHAR loadError[CCHMAXPATH+4]= {0};
  int a;
  int min,sec,totalMin,totalSec;
  long done=1;
  int iMp3Decoder;

  HFILE hfNew2;
  HPIPE hpR2, hpW2;

  HFILE hfSaveError;

  char achBuf2[PIPESIZE];
  ULONG cbRead2, cbWritten;
  char chrCurrent;
 
  char achBuf[PIPESIZE];
  ULONG cbRead;
  RESULTCODES ChildRC= {0};
  PID pidChild;

  rc=-1;

  for(a=0;a<argc;a++) {
    printf(argv[a]);
    printf("\n");
    writeLog("mp3decod.log", argv[a]);
    writeLog("mp3decod.log", "\n");
  }
  writeLog("mp3decod.log", "\n");

  if(argc<8) {
    message();
    exit(-1);
  }
  
  hwndNotify=atol(argv[1]);
  iMp3Decoder=atoi(argv[7]);

  /* Save installdir */
  strcpy(chrInstallDir,argv[6]);
 
  /* Logfile entfernen */
  removeLog("mp3decod.log");  
  
  printf(argv[0]);
  printf(" started with the following parameters: \n\n");
  writeLog("mp3decod.log", argv[0]);
  writeLog("mp3decod.log", " started with the following parameters: \n\n");
  for(a=0;a<argc;a++) {
    printf(argv[a]);
    printf("\n");
    writeLog("mp3decod.log", argv[a]);
    writeLog("mp3decod.log", "\n");
  }
  writeLog("mp3decod.log", "\n");

  if(!iMp3Decoder) {
    printf("Don't know which decoder to use! Info couldn't be extracted from parameters. Aborting...\n");
    writeLog("mp3decod.log", "Don't know which decoder to use! Info couldn't be extracted from parameters. Aborting...\n");
    exit(-1);
  }

  sprintf(loadError,"%s","cmd.exe");
  text=strrchr(loadError,0);
  text++;
  if(argc==9) {
    /* An output filename is given. This means, the user decoded by hand. */
    if(iMp3Decoder==IDKEY_USEMPG123) {
      sprintf(szCommand,"/C \"\"%s\" -s \"%s\" 1> \"%s\\dec_mp3.raw\"\"",argv[4],argv[5],argv[3]);
    }
    else if(iMp3Decoder==IDKEY_USEZ)/* Z! builds wave files */
      sprintf(szCommand,"/C \"\"%s\" -w2 \"%s\" \"%s\"\"",argv[4],argv[5],argv[8]);
    else if(iMp3Decoder==IDKEY_USEMMIOMP3){
      /* Use MMIOMP3 for decoding */
      HMMIO hmmio, hmmioOut;
      MMIOINFO mmioinfo;
      MMAUDIOHEADER mmAudioHeader;
      LONG lBytesRead=0;
      LONG lBytes, lBytesWritten=0;
      FILE *file;
      FOURCC fourcc;

      if(!hwndNotify)
        fflush(stdout);

      memset(&mmioinfo,0, sizeof(mmioinfo));
      //      mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
      mmioinfo.fccIOProc=mmioFOURCC( 'W', 'A', 'V', 'E' );
      mmioinfo.aulInfo[3] = MMIO_MEDIATYPE_AUDIO;
      mmioinfo.ulFlags=MMIO_WRITE|MMIO_DENYWRITE|MMIO_CREATE;
      /* Wave name */
      sprintf(szCommand,"%s",argv[8]);//      sprintf(szCommand,"%s\\mp3.raw",argv[3],);

      if((hmmioOut=mmioOpen(szCommand, &mmioinfo,MMIO_WRITE|MMIO_CREATE))==NULLHANDLE)
        {
          fprintf(stderr,"mmioOpen error with file %s\n", szCommand);
          writeLog("mp3decod.log", "mmioOpen error with file ");
          writeLog("mp3decod.log", szCommand);
          writeLog("mp3decod.log", "\n\n");
          if(!hwndNotify)
            printf("DONE\n");
          else
            WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3DECODE),MPFROMLONG(1));
          exit(0);
        }

      /* Open source file */
      memset(&mmioinfo,0, sizeof(mmioinfo));
      mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
      mmioinfo.ulFlags=MMIO_READ|MMIO_DENYNONE;
      if((hmmio=mmioOpen(argv[5], &mmioinfo,MMIO_READ))==NULLHANDLE)
        {
          fprintf(stderr,"mmioOpen error with file %s\n", argv[5]);
          writeLog("mp3decod.log", "Can't open input file ");
          writeLog("mp3decod.log", argv[5]);
          writeLog("mp3decod.log", "\n");
          mmioClose(hmmioOut,0);
          if(!hwndNotify)
            printf("DONE\n");
          else
            WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3DECODE),MPFROMLONG(1));
          exit(0);
        }
      /* Get translated audio info from source file */
      memset(&mmAudioHeader,0,sizeof(MMAUDIOHEADER));
      rc = mmioGetHeader(hmmio, &mmAudioHeader,sizeof(MMAUDIOHEADER),
                         &lBytesRead, 0, 0);
      if(rc!=MMIO_SUCCESS) {
        mmioClose(hmmio, 0);
        mmioClose(hmmioOut,0);
        fprintf(stderr,"mmioGetHeader error for source!\n");
        writeLog("mp3decod.log", "Can't get header for input file\n");
        if(!hwndNotify)
          printf("DONE\n");
        else
          WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3DECODE),MPFROMLONG(1));  
        exit(0);
      }
      writeLog("mp3decod.log","Input file:\n" );
      sprintf(szCommand,"Channels: %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.usChannels);
      writeLog("mp3decod.log",szCommand );
      sprintf(szCommand,"Samplerate: %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.ulSamplesPerSec);
      writeLog("mp3decod.log",szCommand );
      sprintf(szCommand,"Bps: %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.usBitsPerSample);
      writeLog("mp3decod.log",szCommand );

      rc = mmioSetHeader(hmmioOut,&mmAudioHeader,sizeof(MMAUDIOHEADER),&lBytes,0,0);

      lBytes=mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes;

      do{
        rc=mmioRead(hmmio, buffer, sizeof(buffer));
        if(rc!=0 && rc!=MMIO_ERROR) {
          if(mmioWrite(hmmioOut,buffer,  rc)<rc) {
            fprintf(stderr,"Error while writing audio data!\n");
            writeLog("mp3decod.log", "Error while writing audio data!\n");
            mmioClose(hmmio,0);
            mmioClose(hmmioOut,0);
            if(!hwndNotify)
              printf("DONE\n");
            else
              WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3DECODE),MPFROMLONG(1));  
            exit(0);
          }
          lBytesWritten+=rc;
          done=(lBytesWritten/1000/(lBytes/100000));
          if(!hwndNotify) {
            printf("%d%%\n",done);
            fflush(stdout);
          }
          else
            WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MBWRITTEN),MPFROMLONG(done));
        }
      }while(rc!=0 && rc!=MMIO_ERROR);
      if(rc==MMIO_ERROR) {
        fprintf(stderr,"Error while reading MP3 data!\n");
        writeLog("mp3decod.log", "Error while reading MP3 data!\n");
      }
      else
        {
          fprintf(stderr,"All bytes read.\n");
        }
      sprintf(szCommand, "Bytes read: %d, Bytes written: %d\n\n", lBytes, lBytesWritten);
      writeLog("mp3decod.log", szCommand);
      mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes=lBytesWritten;
      mmioClose(hmmio,0);
      mmioClose(hmmioOut,0);
      // fclose(file);
      if(!hwndNotify)
        printf("DONE\n");
      else
        WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3DECODE),MPFROMLONG(0));
      exit(0);
    }
  }
  else {
    /* Decoding is done for writing TAO */
    if(iMp3Decoder==IDKEY_USEMPG123)
      sprintf(szCommand,"/C \"\"%s\" -s \"%s\" 1> \"%s\\dec_mp3.raw\"\"",argv[4],argv[5],argv[3]);
    else if(iMp3Decoder==IDKEY_USEZ)/* Z! builds wave files */
      sprintf(szCommand,"/C \"\"%s\" -w2 \"%s\" \"%s\\dec_mp3.wav\"\"",argv[4],argv[5],argv[3]);
    else {
      /* Use MMIOMP3 for decoding */
      HMMIO hmmio;
      MMIOINFO mmioinfo;
      MMAUDIOHEADER mmAudioHeader;
      LONG lBytesRead=0;
      LONG lBytes, lBytesWritten=0;
      FILE *file;
      
      memset(&mmioinfo,0, sizeof(mmioinfo));
      mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
      mmioinfo.ulFlags=MMIO_READ|MMIO_DENYNONE;
      sprintf(szCommand,"%s\\dec_mp3.raw",argv[3]);
      if((file=fopen(szCommand,"wb"))==NULLHANDLE) {
        fprintf(stderr,"Can't open output file \n");
        writeLog("mp3decod.log", "Can't open output file \n");
        fprintf(stderr, szCommand);
        writeLog("mp3decod.log", szCommand);
        writeLog("mp3decod.log", "\n");
        WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3DECODE),MPFROMLONG(1));
        exit(0);
      }
      
      if((hmmio=mmioOpen(argv[5], &mmioinfo,MMIO_READ))==NULLHANDLE)
        {
          fprintf(stderr,"mmioOpen error with file %s\n", argv[5]);
          WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3DECODE),MPFROMLONG(1));
          exit(0);
        }

      memset(&mmAudioHeader,0,sizeof(MMAUDIOHEADER));
      rc = mmioGetHeader(hmmio, &mmAudioHeader,sizeof(MMAUDIOHEADER),
                         &lBytesRead, 0, 0);
      if(rc!=MMIO_SUCCESS) {
        mmioClose(hmmio, 0);
        fprintf(stderr,"mmioGetHeader error!\n");
        WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3DECODE),MPFROMLONG(1));  
        exit(0);
      }
      sprintf(szCommand,"Channels: %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.usChannels);
      writeLog("mp3decod.log",szCommand );
      sprintf(szCommand,"Samplerate: %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.ulSamplesPerSec);
      writeLog("mp3decod.log",szCommand );
      sprintf(szCommand,"Bps: %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.usBitsPerSample);
      writeLog("mp3decod.log",szCommand );

      lBytes=mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes;
      do{
        rc=mmioRead(hmmio, buffer, sizeof(buffer));
        if(rc!=0 && rc!=MMIO_ERROR) {
          if(fwrite(buffer, sizeof(char), rc, file)<rc) {
            fprintf(stderr,"Error while reading audio data!\n");
            writeLog("mp3decod.log", "Error while writing audio data!\n");
            mmioClose(hmmio,0);
            fclose(file);
            WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3DECODE),MPFROMLONG(1));  
            exit(0);
          }
          lBytesWritten+=rc;
          done=(lBytesWritten/1000/(lBytes/100000));
          WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MBWRITTEN),MPFROMLONG(done));
        }
      }while(rc!=0 && rc!=MMIO_ERROR);
      if(rc==MMIO_ERROR) {
        fprintf(stderr,"Error while reading MP3 data!\n");
        writeLog("mp3decod.log", "Error while reading MP3 data!\n");
      }
      else
        {
          fprintf(stderr,"All bytes read.\n");
        }
      mmioClose(hmmio,0);
      fclose(file);

      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3DECODE),MPFROMLONG(0));
      exit(0);
    }
  }
  strcat(text,szCommand);
  printf("\nCommand is: %s\n",loadError);
  printf("\nParams: %s\n",szCommand);
  writeLog("mp3decod.log", "\nCommand is: ");
  writeLog("mp3decod.log", loadError);
  writeLog("mp3decod.log", "\nParams: ");
  writeLog("mp3decod.log", szCommand);
  writeLog("mp3decod.log", "\n");

  text=loadError;

  /* start  */
#ifdef DEBUG
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3DECODE),MPFROMLONG(0));
  exit(0);
#endif  
  
  fflush(stdout);
  fflush(stderr);

  hfSaveError = -1;// Get new handle
  if(iMp3Decoder==IDKEY_USEMPG123)
    hfNew2 = HF_STDERR;
  else  
    hfNew2 = HF_STDOUT;

  if(!DosDupHandle(hfNew2, &hfSaveError)) {
    if(!DosCreatePipe(&hpR2, &hpW2, PIPESIZE)) {
      if(!DosDupHandle(hpW2, &hfNew2)){/* Duplicates standard error handle */

        /* Start mpg123 */
        rc=DosExecPgm(loadError,sizeof(loadError),EXEC_ASYNCRESULT, text,NULL,&ChildRC,text);
        sprintf(achBuf,"DosExecPgm() for mp3 decoder returned: %d\n",rc);
        writeLog("mp3decod.log", achBuf);
                  
        DosClose(hpW2);                       /* Closes write handle to ensure     */
        /* Notification at child termination */

        DosDupHandle(hfSaveError, &hfNew2);        /* Brings std*** back                */   

        /*
         * Read from the pipe and write
         */          
        if(iMp3Decoder==IDKEY_USEMPG123) {
          do {
            /* Read from mpg123 */          
            text=achBuf2;
            do {
              DosRead(hpR2, &chrCurrent, sizeof(chrCurrent), &cbRead2);
              *text=chrCurrent;
              text++;
            }while((chrCurrent!='\n') &&(chrCurrent!='\r')&& cbRead2);
            *text=0;
            rc=sscanf(achBuf2,"[%d:%d]  Remaining: [%*d:%*d]  Total: [%d:%d]",&min,&sec,&totalMin,&totalSec);
            DosWrite(HF_STDERR, achBuf2, strlen(achBuf2), &cbWritten);
            /*writeLog("mp3decod.log",achBuf2);*/
            if(rc==4) {
              //sprintf(loadError,"tm: %d ts: %d : \n",totalMin,totalSec);
              //DosWrite(HF_STDERR, loadError, strlen(loadError), &cbWritten);
              done=(LONG)(min*60+sec)*100;
              if((totalMin*60+totalSec)>0)
                done/=(totalMin*60+totalSec);
              WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MBWRITTEN),MPFROMLONG(done));
            }          
          } while(cbRead2);
        } /* end of if(iMp3Decoder==IDKEY_USEMPG123) */
        else {
          do {
            /* Read from Z! */          
            DosRead(hpR2, &achBuf2, sizeof(achBuf2), &cbRead2);
            rc=sscanf(achBuf2,"%d",&done);
            DosWrite(HF_STDERR, achBuf2, strlen(achBuf2), &cbWritten);
            WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MBWRITTEN),MPFROMLONG(done));            
          } while(cbRead2);
        }/* end of else if(iMp3Decoder==IDKEY_USEMPG123) */
        DosClose(hpR2);
        DosWaitChild(0,DCWW_WAIT,&ChildRC,&pidChild,0);
        sprintf(achBuf,"\nmp3 decoder returned: %d\n",ChildRC.codeResult);
        writeLog("mp3decod.log", achBuf);
        fprintf(stderr,achBuf);
      }/* if(!DosDupHandle(hpW2, &hfNew2)) */
    }/* if(!DosCreatePipe(&hpR2, &hpW2, PIPESIZE)) */
  }/* if(!DosDupHandle(HF_STDERR, &hfSave)) */
  

  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3DECODE),MPFROMLONG(ChildRC.codeResult));
  exit(0);
}










