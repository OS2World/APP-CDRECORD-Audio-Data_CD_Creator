/**********************************************************************
 *
 *      waveinfo.exe (c) Chris Wohlgemuth 1999-2002
 *
 *      Gets size of any audio file OS/2 has an IO procedure for
 *
 *      http://www.geocities.com/SiliconValley/Sector/5785/
 *
 *
 *********************************************************************/

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
#define INCL_OS2MM
#define INCL_MMIOOS2
#define INCL_MCIOS2

#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "os2me.h"
#include "audiofolder.h" // For ACKEY_*


void message()
{
  printf("This program is (C) Chris Wohlgemuth 1999-2001\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creator WPS classes.\n");
}


int main(int argc, char * argv[])
{
	HMMIO hmmio;
	MMAUDIOHEADER mmAudioHeader;
    MMIOINFO mmioinfo;
	LONG lBytesRead=0;
	LONG size;
    HWND hwndNotify;
	int i;
	ULONG result;

	result=!MCIERR_SUCCESS;
   

	printf("\nWavefile info (c) Chris Wohlgemuth 2000\n");
	printf("This software is free software. The Gnu Public Licence applies.\n");
	printf("See the file COPYING for further information.\n\n");
	

    printf("\n");
    printf(argv[0]);
    printf(" started with the following parameters: \n\n");
    for(size=0;size<argc;size++) {
      printf(argv[size]);
      printf("\n");
    }
  
    if(argc<4) {
      message();
      exit(-1);
    }  
	
    hwndNotify=atol(argv[1]);
	
	// open the file
	do {
      
      size=0;
      memset(&mmioinfo,0, sizeof(mmioinfo));
      mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
      mmioinfo.ulFlags=MMIO_READ|MMIO_DENYNONE;
      
      hmmio = mmioOpen(argv[3], &mmioinfo, MMIO_READ);
      if(!hmmio) {
        printf("MMIO: Can't open file %s\n\n",argv[3]);
        break;
      }
      memset(&mmAudioHeader,0,sizeof(MMAUDIOHEADER));
      result = mmioGetHeader(hmmio, &mmAudioHeader,sizeof(MMAUDIOHEADER),
                             &lBytesRead, 0, 0);
		if(result!=MMIO_SUCCESS) {
			mmioClose(hmmio, 0);
			printf("Can't get MMAUDIOHEADER\n\n");
			break;
		}
		printf("Information for %s:\n\n",argv[3]);
		printf("Format:                ");
		
		switch (mmAudioHeader.mmXWAVHeader.WAVEHeader.usFormatTag)
			{
			case DATATYPE_WAVEFORM:
				printf("Waveform audio (PCM)\n");				
				break;
			case DATATYPE_ALAW:
				printf("A-Law\n");				
				break;
			case DATATYPE_MULAW:
				printf("Mu-Law\n");				
				break;
			case DATATYPE_ADPCM_AVC:
				printf("ADPCM-audio\n");				
				break;
			default:
				printf("Unknown wave format\n");				
				break;
			}
		
		printf("Time:                  %d:%02d\n",mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
					 mmAudioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec/60,
					 mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
					 mmAudioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec%60);
		printf("Channels:              %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.usChannels);
		printf("Samples per second:    %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.ulSamplesPerSec);
		printf("Bits per sample:       %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.usBitsPerSample);
		printf("Average bytes p. sec.: %d\n",mmAudioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec);
		printf("Audio length in bytes: %d\n",mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes);
		
		mmioClose(hmmio, 0);
        if(mmAudioHeader.mmXWAVHeader.WAVEHeader.usFormatTag!=DATATYPE_WAVEFORM ||
           mmAudioHeader.mmXWAVHeader.WAVEHeader.usChannels!=2 ||
           mmAudioHeader.mmXWAVHeader.WAVEHeader.ulSamplesPerSec!=44100 ||
           mmAudioHeader.mmXWAVHeader.WAVEHeader.usBitsPerSample !=16)
          break;

        size=mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes;
        /* Send msg. to the notification window */
        WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PLAYTIME),MPFROMLONG(size));
		return 0;
	}while (FALSE);
   
    /* Send msg. to the notification window */
    WinPostMsg(hwndNotify,WM_APPTERMINATENOTIFY,MPFROMLONG(ACKEY_PLAYTIME),MPFROMLONG(0));
    return result;
}





