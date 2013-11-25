/*
 * mp3size.exe (C) Chris Wohlgemuth 1999-2002
 *
 * This helper starts mp3info to query the playtime of the mp3 and sends it
 * to the audio folder class
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

#define PIPESIZE 2560
#define HF_STDOUT 1      /* Standard output handle */
#define HF_STDERR 2

//#define DEBUG

void message()
{
  printf("This program is (C) Chris Wohlgemuth 1999-2000\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creator WPS classes.\n");
}

int main(int argc, char * argv[])
{
  int rc;
  HFILE hfNew;
  HWND hwndNotify;
  char *text,*text2;
  HPIPE hpR, hpW;
  ULONG cbRead, cbWritten;
  CHAR achBuf[PIPESIZE], szFailName[CCHMAXPATH*4];
  HFILE hfSave;
  long size=0;
  int rate=0;
  int iBitRate=0;
  int iChannels=0;
  SHORT stereo=0;
  int iWhichMp3Dec;

  rc=-1;

  /* Print the command line to screen for debugging */
  printf("\n");
  printf(argv[0]);
  printf(" started with the following parameters: \n\n");
  for(size=0;size<argc;size++) {
    printf(argv[size]);
    printf("\n");
  }
  
  if(argc<6) {
    message();
    exit(-1);
  }  
  
  hwndNotify=atol(argv[1]);

  /* Which decoder do we have */
  iWhichMp3Dec=atoi(argv[5]);

#ifdef DEBUG
  size=0;
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PLAYTIME),MPFROMLONG(size));
  exit(0);
#endif  

  switch(iWhichMp3Dec)
    {
    case IDKEY_USEMPG123:
      sprintf(szFailName,"\"%s\" \"-f%%S %%F %%b %%M\" \"%s\"",argv[3],argv[4]);
      break;
    case IDKEY_USEZ:
      sprintf(szFailName,"\"%s\" -n \"%s\"",argv[3],argv[4]);
      break;

    case IDKEY_USEMMIOMP3:
      {
        HMMIO hmmio;
        MMIOINFO mmioinfo;
        MMAUDIOHEADER mmAudioHeader;
        LONG lBytesRead=0;

        memset(&mmioinfo,0, sizeof(mmioinfo));
        mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
        mmioinfo.ulFlags=MMIO_READ|MMIO_DENYNONE;

        do {
          int size=0;
          if((hmmio=mmioOpen(argv[4], &mmioinfo,MMIO_READ))==NULLHANDLE)
            {
              fprintf(stderr,"mmioOpen error with file %s\n", argv[4]);
              break;
            }
          memset(&mmAudioHeader,0,sizeof(MMAUDIOHEADER));
          rc = mmioGetHeader(hmmio, &mmAudioHeader,sizeof(MMAUDIOHEADER),
                             &lBytesRead, 0, 0);
          mmioClose(hmmio, 0);
          if(rc!=MMIO_SUCCESS) {
            fprintf(stderr,"mmioGetHeader error!\n");
            break;
          }

          fprintf(stderr,"Channels: %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.usChannels);
          fprintf(stderr,"Samplerate: %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.ulSamplesPerSec);
          fprintf(stderr,"Bps: %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.usBitsPerSample);
          fprintf(stderr,"Playtime: %d:%02d\n",mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
                  mmAudioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec/60,
                  mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
                  mmAudioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec%60);
          fprintf(stderr,"Bytes: %d\n", mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes);

          if(mmAudioHeader.mmXWAVHeader.WAVEHeader.usChannels!=2)
            break;
          if(mmAudioHeader.mmXWAVHeader.WAVEHeader.ulSamplesPerSec!=44100)
            break;
          size=mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
                  mmAudioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec;

#if 0
          /* Send mp3 info message */
          iBitRate<<=2;
          stereo+=iBitRate;
          printf("%d",stereo);
#endif
          WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3INFO),
                     MPFROM2SHORT(mmAudioHeader.mmXWAVHeader.WAVEHeader.usChannels,
                                  (SHORT)mmAudioHeader.mmXWAVHeader.WAVEHeader.ulSamplesPerSec));

          size*=4*44100;
          /* Send msg. to the notification window */
          WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PLAYTIME),MPFROMLONG(size));
          exit(0);
        }while(TRUE);
        WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PLAYTIME),MPFROMLONG(0));
        exit(-1);
      }

    default:
      /* Send msg. to the notification window */
      WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PLAYTIME),MPFROMLONG(0));
      exit(-1);
    }

  text=szFailName; 
  printf("\nmpinfo command is: %s\n",text);
  /* start  */

  /* Redirect stdout */ 
  hfSave = -1;// Get new handle
  hfNew = HF_STDOUT;
 
  if(!DosDupHandle(HF_STDOUT, &hfSave)){    /* Saves standard  handle      */
    if(!DosCreatePipe(&hpR, &hpW, PIPESIZE)){ /* Creates pipe            */

      /* If the pipe size is smaller than the output of the child the system() call blocks.
         So create the pipe big enough. */
      
      if(!DosDupHandle(hpW, &hfNew)){           /* Duplicates standard error handle */
        
        /* Start mp3info or z!. The call returns after end of the decoder */
        rc=system(text);
        printf("Errorcode from mp3info/z! is: %d\n",rc);

        if(rc!=-1) {
          DosClose(hpW);                       /* Closes write handle to ensure     */
          /* Notification at child termination */
          DosDupHandle(hfSave, &hfNew);        /* Brings stderr back                */
          
          /*
           * Read from the pipe and write to the screen
           * as long as there are bytes to read.
           */
          
          do {
            DosRead(hpR, achBuf, sizeof(achBuf), &cbRead);
            DosWrite(HF_STDERR, achBuf, cbRead, &cbWritten);

            switch(iWhichMp3Dec)
              {
              case IDKEY_USEMPG123:
                if(sscanf(achBuf,"%li %i %i %s",&size, &rate, &iBitRate, &szFailName)!=4) {
                  rc=-1;
                }
                break;
              case IDKEY_USEZ:
                if(sscanf(achBuf,"%i %i %i %i", &rate, &iChannels, &size, &iBitRate)!=4) {
                  rc=-1;
                }
                if(iChannels==2)
                  sprintf(szFailName,"stereo");
                else
                  sprintf(szFailName,"mono");
                break;
              default:
                break;
              }
                                    
          } while(cbRead);
          DosClose(hpR);

          printf("Size in secs: %d\n",size);
          printf("Samplerate:   %d\n",rate);
          printf("Mode:         %s\n",szFailName);
          size*=4*44100;/* Bytes */
          if(rate!=44100)
            size=0;/* Indicate error */
          stereo=0;
          if(!stricmp(szFailName,"stereo"))
            stereo=1;/* stereo */
          else if(!stricmp(szFailName, "j-stereo"))
            stereo=2;
          if(!stereo)
            size=0;
        }/* End of if(rc!=-1) */
        else
          size=0;/* Indicate error */       
      }/* if(!DosDupHandle(HF_STDERR, &hfSave)) */      
    }
  }
 
  /* Send mp3 info message */
  iBitRate<<=2;
  stereo+=iBitRate;
  printf("%d",stereo);
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_MP3INFO),MPFROM2SHORT(stereo,(SHORT)rate));
  /* Send msg. to the notification window */
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PLAYTIME),MPFROMLONG(size));
}










