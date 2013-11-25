/*
 * cover.exe (C) Chris Wohlgemuth 2000
 *
 * 
 * 
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
#define  INCL_DOSERRORS
#define INCL_DOSQUEUES   /* Queue values */
 
#include <os2.h>
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
  printf("This program should only be called by the CD-Creater WPS classes.\n");
}

ULONG queryFileSize(char * fileName)
{
  HFILE hf;
  ULONG ulAction=0;
  APIRET rc;
  FILESTATUS3 fs3={{0}};  
  char text[CCHMAXPATH];
  
  
  rc=DosOpen(fileName,&hf,&ulAction,0L,0L,OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
             OPEN_FLAGS_NOINHERIT|OPEN_ACCESS_READONLY|OPEN_SHARE_DENYNONE,0L);
  if(rc!=NO_ERROR)
    return 0;
 
  rc=DosQueryFileInfo(hf,FIL_STANDARD,&fs3,sizeof(FILESTATUS3));
  if(rc!=NO_ERROR) {
    DosClose(hf);
    return 0;
  }
 
  DosClose(hf);
  return fs3.cbFile;
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
  LONG size=0;
  char* chrExtension;
  char artist[50]="";
  char title[CCHMAXPATH]="";
  char genre[50]="";
  char year[10]="";
  char album[50]="";
  char time[10]="";
  FILE *file;
  int iTrackNum;

  rc=-1;

  /*
    arg[0]: program name
    arg[1]: HWND
    arg[2]: cdrecord.exe path
    arg[3]: install dir
    arg[4]: MP3/Wave path
    arg[5]: folder path
    arg[6]: Track num
   */

  if(argc<7) {
    message();
    exit(-1);
  }  

  hwndNotify=atol(argv[1]);
  printf("\n");
  printf(argv[0]);
  printf(" started with the following parameters: \n\n");
  for(size=0;size<argc;size++) {
    printf(argv[size]);
    printf("\n");
  }

  iTrackNum=atoi(argv[6]);

#ifdef DEBUG
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_CREATECOVER),MPFROMLONG(size));
  exit(0);
#endif  

  if(iTrackNum==1) {
    /* Copy the PG>Pro script template to the folder */
    sprintf(achBuf,"%s\\bin\\%s",argv[3],COVER_NAME);
    printf("\nCopying %s to audio folder.\n",achBuf);
    sprintf(szFailName,"%s\\%s",argv[5],COVER_NAME);
    printf("%s\n",szFailName);
    printf("DosCopy() returned: %d\n",DosCopy(achBuf,szFailName,DCPY_EXISTING));
  }

  sprintf(szFailName,"%s\\%s",argv[5],COVER_NAME);
  file=fopen(szFailName,"a+");
  if(!file) {
    WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_CREATECOVER),MPFROMLONG(0));
    exit(-1);
  }

  /* All track info written to cover skript. Close the sub function. */
  if(iTrackNum==0) {
    snprintf(achBuf,sizeof(achBuf),"return\n");
    fwrite(achBuf,strlen(achBuf),1,file);
    WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_CREATECOVER),MPFROMLONG(0));
    fclose(file);
    exit(0);
  }

  /* check if it's a MP3. The Check if it's a supported file and if the extension is valid was done in the class.
     We have just to check if it's .mp3 or .wav */
  chrExtension=strrchr(argv[4],'.');
  if(strstr(strlwr(chrExtension),".mp3")) {

    /* It's a MP3 */
    sprintf(szFailName,"\"%s\\bin\\mp3info.exe\" -f \"%%IH|%%l |%%a |%%t |%%g |%%y |%%m:%%02s |%%IE\" \"%s\"",argv[3],argv[4]);
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
          
          /* Start mp3info. The call returns after mp3info ended */
          rc=system(text);
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
              if(cbRead) {
                sscanf(achBuf,"|%50[^|]|%50[^|]|%50[^|]|%50[^|]|%10[^|]|%10[^|]|",&album,&artist,&title,&genre,&year,&time);
                chrExtension=strchr(album,0);
                if(chrExtension)
                  *(--chrExtension)=0;
                chrExtension=strchr(artist,0);
                if(chrExtension)
                  *(--chrExtension)=0;
                chrExtension=strchr(title,0);
                if(chrExtension)
                  *(--chrExtension)=0;
                chrExtension=strchr(genre,0);
                if(chrExtension)
                  *(--chrExtension)=0;
                chrExtension=strchr(year,0);
                if(chrExtension)
                  *(--chrExtension)=0;
                chrExtension=strchr(time,0);
                if(chrExtension)
                  *(--chrExtension)=0;
      
                printf(album);
                printf("\n");
                printf(artist);
                printf("\n");
                printf(title);
                printf("\n");
                printf(genre);
                printf("\n");
                printf(year);
                printf("\n");
                printf(time);
                printf("\n");
              }
            } while(cbRead);          
            DosClose(hpR);
          }
        }
      }/* if(!DosDupHandle(HF_STDERR, &hfSave)) */      
    }/* End of if(rc!=-1) */
    
    printf("Errorcode from mp3info is: %d\n",rc);
    if(rc)
      size=0;/* Indicate error */
  }/* end of if(strstr(strlwr(chrExtension),".mp3")) */
  else {
    printf("\nIt's a '.wav'.\n");
    cbRead=queryFileSize(argv[4]);
    if(cbRead) {
      cbRead-=44;
      printf("Filesize in Bytes (without header): %d\n",cbRead);
      cbRead/=(44100*4);
      snprintf(time,sizeof(time),"%d:%02d",cbRead/60,cbRead%60);
      printf("Time: %s\n",time);
    }
  }

  if(!strlen(title))
    snprintf(title,sizeof(title),"%s",argv[4]);

  if(strlen(artist))
    snprintf(achBuf,sizeof(achBuf),"info._name.%d=\"%s - %s\"\n",iTrackNum,artist,title);
  else
    snprintf(achBuf,sizeof(achBuf),"info._name.%d=\"%s\"\n",iTrackNum,title);
  fwrite(achBuf,strlen(achBuf),1,file);
  snprintf(achBuf,sizeof(achBuf),"info._album.%d=\"%s\"\n",iTrackNum,album);
  fwrite(achBuf,strlen(achBuf),1,file);
  snprintf(achBuf,sizeof(achBuf),"info._artist.%d=\"%s\"\n",iTrackNum,artist);
  fwrite(achBuf,strlen(achBuf),1,file);
  snprintf(achBuf,sizeof(achBuf),"info._playtime.%d=\"%s\"\n",iTrackNum,time);
  fwrite(achBuf,strlen(achBuf),1,file);
  snprintf(achBuf,sizeof(achBuf),"info._genre.%d=\"%s\"\n",iTrackNum,genre);
  fwrite(achBuf,strlen(achBuf),1,file);
  snprintf(achBuf,sizeof(achBuf),"info._year.%d=\"%s\"\n",iTrackNum,year);
  fwrite(achBuf,strlen(achBuf),1,file);

  snprintf(achBuf,sizeof(achBuf),"info._name.0=info._name.0+1\n\n");
  fwrite(achBuf,strlen(achBuf),1,file);


  /* Send msg. to the notification window */
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_CREATECOVER),MPFROMLONG(size));
  
  if(file)
    fclose(file);
}










