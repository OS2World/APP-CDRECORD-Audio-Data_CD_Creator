/*
 * cdsize.exe (C) Chris Wohlgemuth 1999-2004
 *
 * This helper calculates the free space on a CDR and sends it
 * to the creator folder class
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

#define PIPESIZE 2560
#define HF_STDOUT 1      /* Standard output handle */
#define HF_STDERR 2

/*
arg[0]: progname
arg[1]: hwndNotify
arg[2]: cdrecord path
arg[3]: dvddao path
arg[4]: -d x,y,z
 */

void message()
{
  printf("This program is (C) Chris Wohlgemuth 1999-2001\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creater WPS classes.\n");
}

int main(int argc, char * argv[])
{
  int rc;
  HFILE hfNew;
  HWND hwndNotify;
  char *text;
  HPIPE hpR, hpW;
  ULONG cbRead, cbWritten;
  CHAR achBuf[PIPESIZE]={0};
  CHAR szFailName[CCHMAXPATH];
  HFILE hfSave;
  RESULTCODES ChildRC= {0};
  int a;
  int iBlocks;

  rc=-1;

  if(argc<4) {
    message();
    exit(-1);
  }  

  hwndNotify=atol(argv[1]);

  /* Print the parameters in VIO window */
  printf("argc:%d\n",argc);  
  for(a=0;a<argc;a++)
    {
      printf("%d: %s\n",a,argv[a]);
    }
  printf("\n");
  memset(achBuf, 0, sizeof(achBuf));

  /*  sprintf(achBuf,"%s %s -toc -v",argv[2],argv[3]); */
  sprintf(achBuf,"%s",argv[3]);
  text=strchr(achBuf,0);
  if(!text) {
    WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(4),MPFROMLONG(0));    
  }
  text++;
  sprintf(text,"%s", argv[4]);
  printf("Parameters for size query: %s\n",text);
  text=achBuf; 
  printf("Path is: %s\n",text);

  /**** start  ****/
  
  /* Redirect stdout */ 
  hfSave = -1;// Get new handle
  hfNew = HF_STDOUT;
 
  if(!DosDupHandle(HF_STDOUT, &hfSave)){    /* Saves standard  handle      */
    if(!DosCreatePipe(&hpR, &hpW, PIPESIZE)){ /* Creates pipe            */

      /* If the pipe size is smaller than the output of the child the system() call blocks.
         So create the pipe big enough. */
      
      if(!DosDupHandle(hpW, &hfNew)){           /* Duplicates standard output handle */
  
        /* start CDRecord/2 */
        rc=DosExecPgm(szFailName,sizeof(szFailName),EXEC_ASYNC, text,NULL,&ChildRC, text);
        fprintf(stderr,"DosExecPgm() for dvddao returned: %d\n",rc);
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
          BOOL bNoMedia=FALSE;

          DosRead(hpR, achBuf, sizeof(achBuf), &cbRead);
          DosWrite(HF_STDERR, achBuf, cbRead, &cbWritten);

          /* Uppercase string */
          strupr(achBuf);
          text=strstr(achBuf,"NO MEDIA");
          if(text){
            /* No DVD/CD inserted */
            iBlocks=0;
            //WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(4),MPFROMLONG(0));
            bNoMedia=TRUE;
            break;
          }

          text=strstr(achBuf,"MEDIA SIZE");
          if(text){
            char dummy[200];
            /* We have the size */
            sscanf(text, "%s %s %d", dummy, dummy, &iBlocks);
            break;
          }

        } while(cbRead);          
        printf("Num blocks found %d \n",iBlocks);
        DosClose(hpR);
      }
    }
  }/* if(!DosDupHandle(HF_STDERR, &hfSave)) */      

  printf("Errorcode from dvddao is: %d\n",rc);
  /* Send msg. to the notification window */
  WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(4),MPFROMLONG(iBlocks));
  exit(0);
}










