/**********************************************************************
 *
 *      waveinfo.exe (c) Chris Wohlgemuth 1999-2002
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
	int i;
	HMMIO hmmio;
	ULONG result;
	MMAUDIOHEADER mmAudioHeader;
	LONG lBytesRead=0;
	LONG size;

	result=!MCIERR_SUCCESS;
    /* If argc==3 then called by mp3decode.res else by daoaudio.res.
       mp3decode wants to have some more info */

    if(argc<2) {
      message();
      exit(-1);
    }  
	
	// open the file
	do {
	  MMIOINFO mmioinfo;
      int iModulo=0;
      LONG lBytes;
      LONG lSectors;

      size=0;

      memset(&mmioinfo,0, sizeof(mmioinfo));
      mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
      mmioinfo.ulFlags=MMIO_READ|MMIO_DENYNONE;
      
      hmmio = mmioOpen(argv[1], &mmioinfo, MMIO_READ);
		if(!hmmio) {
			break;
		}
		memset(&mmAudioHeader,0,sizeof(MMAUDIOHEADER));
		result = mmioGetHeader(hmmio, &mmAudioHeader,sizeof(MMAUDIOHEADER),
														 &lBytesRead, 0, 0);
		if(result!=MMIO_SUCCESS) {
			mmioClose(hmmio, 0);
			break;
		}
				
		mmioClose(hmmio, 0);


        size=mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
					 mmAudioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec;
        printf("\nsize: %d\n",
               size);
        iModulo=mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes%2352;
        printf("iModulo: %d\n",
               iModulo);
        lSectors=(mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/2352);
        printf("lSectors: %d\n",
               lSectors);
        lBytes=mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes-iModulo;
        printf("lBytes: %d\n",
               lBytes);
        lBytes-=((mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/(44100*4*60)) * (44100*4)*60);
        printf("lBytes(2): %d\n",
               lBytes);
        lBytes-=((mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
                  (44100*4))%60)*(44100*4);
        printf("lBytes(3): %d\n",
               lBytes);
        lSectors=lBytes/2352;
        printf("lSectors: %d\n\n",
               lSectors);
        if(iModulo)
          lSectors++;

        printf("mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes: %d\n",
               mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes);
        printf("mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/2352: %d\n",
               mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/2352);
        printf("mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes%%2352 (iModulo): %d\n",
               mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes%2352);
        printf("mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes-iModulo: %d\n\n",
               mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes-iModulo);

        printf("%d:%02d:%02d  %d %d %d\n",mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
               mmAudioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec/60,
               mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes/
               mmAudioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec%60,
               lSectors, size, mmAudioHeader.mmXWAVHeader.WAVEHeader.usChannels,
               mmAudioHeader.mmXWAVHeader.WAVEHeader.ulSamplesPerSec);
        //		printf("%d",size);
        exit(0);
	}while (FALSE);
   
    printf("0");
    exit(1);
}






