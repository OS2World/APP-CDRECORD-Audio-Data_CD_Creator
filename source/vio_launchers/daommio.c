/*
 * daommio.c (C) Chris Wohlgemuth 2002
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
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSQUEUES   /* Queue values */
#define INCL_OS2MM
#define INCL_MMIOOS2

#include <os2.h>
#include <os2me.h>
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

#define ZEROBUFFSIZE 2352

char tracks[99][CCHMAXPATH];
char buffer[500000]={0};
char zeroBuf[ZEROBUFFSIZE]= {0};

void message()
{
  fprintf(stderr, "This program is (C) Chris Wohlgemuth 1999\n");
  fprintf(stderr, "See the file COPYING for the license.\n\n");
  fprintf(stderr, "This program should only be called by the CD-Creater WPS classes.\n");
}

int main(int argc, char * argv[])
{
  char logText[CCHMAXPATH*5];
  int rc;
  HMMIO hmmio;
  MMIOINFO mmioinfo;
  MMAUDIOHEADER mmAudioHeader;
  LONG lBytesRead=0;
  LONG lBytesWritten=0;
  LONG lBytes;
  int numFiles=0;
  int iModulo=0;
  FILE * pipe;
  char cdrdaoCommand[CCHMAXPATH*10];

  int a;

  /* Set stdout to binary */
  _fsetmode(stdout,"b");
  fflush(stdout);

  if(argc<5) {
    message();
    fprintf(stderr,"DONE");
    exit(-1);
  }  
  
  fprintf(stderr,"\n%s started with the following parameters:\n",argv[0]);
  for(a=0;a<argc;a++)
    {
      fprintf(stderr,"%d: %s\n",a,argv[a]);
    }
      fprintf(stderr,"\n");

  if(1)
    {
      int ind=0;
      FILE *file;
      char * ptr;

      fprintf(stderr,"Cdrdao/2 path is:\n");
      fprintf(stderr,argv[1]);
      fprintf(stderr,"\n\n");

      /* Check validity of files */
      file=fopen(argv[4],"rt");
      while(fgets(tracks[ind], CCHMAXPATH, file)) {
        if((ptr=strrchr(tracks[ind], 0xa))!=NULLHANDLE)
          *ptr=0;
        if((ptr=strrchr(tracks[ind], 0xd))!=NULLHANDLE)
          *ptr=0;
        if(tracks[ind][0]!=0) {
          /*  fprintf(stderr, "%s\n", tracks[ind]); */
          ind++;
        }
      }
      numFiles=ind;
      fclose(file);
      
      fprintf(stderr, "%d files to write.\nChecking validity...\n\n", numFiles);
      for(a=0;a<numFiles; a++) {
        memset(&mmioinfo,0, sizeof(mmioinfo));
        mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
        mmioinfo.ulFlags=MMIO_READ|MMIO_DENYNONE;
        fprintf(stderr,tracks[a]);
        if((hmmio=mmioOpen(tracks[a], &mmioinfo,MMIO_READ))==NULLHANDLE)
          {
            fprintf(stderr,"mmioOpen error with file %s\n", tracks[a]);
            fprintf(stderr,"DONE");/* Signal end */
            exit(1);
          }
        mmioClose(hmmio,0);
        fprintf(stderr," -> OK\n");
      }/* for */
      fprintf(stderr,"\n");

      if(strlen(argv[1])+strlen(argv[2])+strlen(argv[3])>=sizeof(cdrdaoCommand))
        {
          fprintf(stderr,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
          fprintf(stderr,"Cdrdao command line is longer than internal command buffer!\n");
          fprintf(stderr,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
          fprintf(stderr,"DONE");
          exit(1);
        }
      sprintf(cdrdaoCommand,"\"%s\" %s \"%s\"", argv[1], argv[2], argv[3]);

      // sprintf(cdrdaoCommand,"\"D:\\Arbeitsoberflaeche\\Create Audio-CD!1\\devaudio.exe\"");
      fprintf(stderr,"Starting cdrdao/2 with the following parameters:\n");
      fprintf(stderr,cdrdaoCommand);
      fprintf(stderr,"\n\n");

      /* Set priority to timecritical 20. By doing so cdrdao will also run with TC 20 */
      DosSetPriority(PRTYS_PROCESS,PRTYC_TIMECRITICAL,20,0);
      
      /* Start cdrdao */
      pipe=popen(cdrdaoCommand,"wb");
      if(!pipe) {
        fprintf(stderr,"Can't start Cdrdao!\n");
        fprintf(stderr,"DONE");
        exit(1);
      }

      fprintf(stderr,"Cdrdao started...\n\n");

      /* Pipe audio data to cdrdao */
      fprintf(stderr,"Now piping audio data to cdrdao...\n\n");

      /* Increase priority for reader process */
      DosSetPriority(PRTYS_PROCESS, PRTYC_TIMECRITICAL,15, 0);

      for(a=0;a<numFiles; a++) {
        memset(&mmioinfo,0, sizeof(mmioinfo));
        mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
        mmioinfo.ulFlags=MMIO_READ|MMIO_DENYNONE;
      
        if((hmmio=mmioOpen(tracks[a], &mmioinfo,MMIO_READ))==NULLHANDLE)
          {
            fprintf(stderr,"mmioOpen error with file %s\n", tracks[a]);
            fprintf(stderr,"DONE");
            exit(1);
          }
        memset(&mmAudioHeader,0,sizeof(MMAUDIOHEADER));
        rc = mmioGetHeader(hmmio, &mmAudioHeader,sizeof(MMAUDIOHEADER),
                           &lBytesRead, 0, 0);
        if(rc!=MMIO_SUCCESS) {
          mmioClose(hmmio, 0);
          fprintf(stderr,"mmioGetHeader error!\n");
          fprintf(stderr,"DONE");
          exit(1);
        }
        fprintf(stderr,"WRITING: %s\n", tracks[a]);
 
        iModulo=mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes%2352;

        /* rc=mmioSeek(hmmio, mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes-(2352*575), SEEK_SET);*/
        /* rc=mmioSeek(hmmio, 2000000, SEEK_SET);*/
        lBytes=mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes;
        lBytesWritten=0;

       do{
        rc=mmioRead(hmmio, buffer, sizeof(buffer));
        if(rc!=0 && rc!=MMIO_ERROR) {
          if(fwrite(buffer, sizeof(char), rc, pipe)!=rc)
            fprintf(stderr,"Error while writing audio data to cdrdao!\n");
          lBytesWritten+=rc;
        }
      }while(rc!=0 && rc!=MMIO_ERROR);
      if(rc==MMIO_ERROR) {
        fprintf(stderr,"Error while reading audio data!\n");
        fprintf(stderr,"DONE");
      }
      else
        {
          /* All data written. when decoding MP3s using MMIOMP3 the actual audio data may be fewer
             than  previously reported by MMIOMP3. The TOC file was created with the reported value
             so pad the data with zero so the track marks are correct. */
          fprintf(stderr,"Bytes to read: %d, Bytes written: %d\n", lBytes, lBytesWritten);
          while(lBytesWritten<lBytes) {
            LONG lToWrite=lBytes-lBytesWritten;

            if(lToWrite<=ZEROBUFFSIZE) {
              if(fwrite(zeroBuf, sizeof(char), lToWrite, pipe)!=lToWrite)
                fprintf(stderr,"Error while writing audio data to cdrdao!\n");
              lBytesWritten+=lToWrite;
            }
            else {
              if(fwrite(zeroBuf, sizeof(char), ZEROBUFFSIZE, pipe)!=ZEROBUFFSIZE)
                fprintf(stderr,"Error while writing audio data to cdrdao!\n");
              lBytesWritten+=ZEROBUFFSIZE;
            }
            fprintf(stderr,"Padded with zeros. Bytes to read: %d, Bytes written: %d\n", lBytes, lBytesWritten);
          }/* while() */
          if(iModulo) {
            fprintf(stderr,"Padding sector with %d bytes...\n", 2352-iModulo);
            fwrite(zeroBuf, sizeof(char), 2352-iModulo, pipe);
          }
        }
      mmioClose(hmmio,0);
      /* fprintf(stderr,"Done Playing\n\n");*/
      }/* for */

      /* Wait for cdrdao to end */
      rc=pclose(pipe);
      fprintf(stderr,"Done Writing\n\n");
    }
  
  fprintf(stderr,"DONE");
  exit(0);
}
