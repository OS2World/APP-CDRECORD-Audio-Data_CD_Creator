/*
 * This file is (C) Chris Wohlgemuth 1999
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
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSFILEMGR

#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "cddb.h"


#pragma pack(1)

typedef struct{
	UCHAR   ucFirstTrack;
	UCHAR 	ucLastTrack;
	ULONG   ulLeadOut;
}CDINFO;

typedef struct{
	ULONG   ulLeadOut;
	UCHAR   ucFirstTrack;
	UCHAR 	ucLastTrack;
}CDINFO2;

typedef struct
{
	ULONG ulTrackAddress;
	UCHAR ucTCInfo;
}TRACKINFO;

typedef struct
{
	UCHAR signature[4];
	UCHAR  ucTrackNum;
}TINFOPARAM;

typedef struct
{
	UCHAR ucFrames;
	UCHAR ucSeconds;
	UCHAR ucMinutes;
	UCHAR ucNotUsed;
}MSF;

typedef struct
{
  UCHAR signature[4];
  UCHAR  ucAddressMode;
  UCHAR ucStartSector[4];
  UCHAR ucEndSector[4];
}PLAYPARAM;

#pragma pack()


HFILE extern openDrive(char* drive);
void extern closeDrive(HFILE hfDrive);
LONG extern CDCalculateSize(MSF* msf1,MSF* msf2);
/******************************************/
/* Result:  
	 0:   Error
	 -1: CD is Data Disk
	 other: # Audio tracks */
/******************************************/
int extern CDQueryAudioCDTracks(HFILE hfDrive);
ULONG extern CDQueryUPC(UCHAR* ucUPC, char * drive);
/* Returns the size in bytes of a track on drive 'drive' (e.g. O:) */
LONG extern CDQueryTrackSize(ULONG numTrack, char * drive);
BOOL CDPlayTrack(ULONG numTrack, char * drive);
BOOL extern CDStop(char * drive);
LONG extern queryFreeDriveSpace(ULONG diskNum) ;
/**************************************************************/
/* This funktion returns the CD-Drives in the system          */
/*                                                            */
/* iNumCD (output): # of CD-Drives                            */
/* cFirstDrive (output): first cd-Drive letter                */
/* returns TRUE: We have cd-drives                            */
/**************************************************************/
BOOL extern CDQueryCDDrives(int *iNumCD, char * cFirstDrive);
/* Returns a CDDB disc-ID for quering the database */
/* Returns 0 if error.                             */
LONG extern CDDBDiscID(char * drive, CDDBINFO * cddbInfo); 


LONG extern CDCalculateSize(MSF* msf1,MSF* msf2)
{
 	return ((msf2->ucMinutes*60+msf2->ucSeconds)*75+msf2->ucFrames)*2352-
		((msf1->ucMinutes*60+msf1->ucSeconds)*75+msf1->ucFrames)*2352;	
}

LONG extern CDCalculateSector2(MSF* msf1)
{
 	return ((msf1->ucMinutes*60+msf1->ucSeconds));	
}

LONG extern CDCalculateSector(MSF* msf1)
{
 	return ((msf1->ucMinutes*60+msf1->ucSeconds)*75+msf1->ucFrames);	
}


/****************************************/
/* Input: drive (e.g. o:)               */
/****************************************/
HFILE extern openDrive(char* drive)
{
	HFILE hfDrive = 0;
	ULONG ulAction;
	ULONG rc;
	
	rc = DosOpen(drive, &hfDrive, &ulAction, 0,
					 FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
					 OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY |
					 OPEN_FLAGS_DASD|OPEN_FLAGS_FAIL_ON_ERROR, NULL);	

	if(rc)
		return NULL;//Error

	return hfDrive;
}

void extern closeDrive(HFILE hfDrive)
{
	DosClose(hfDrive);	
}

/******************************************/
/* Result:  
	 0:   Error
	 -1: CD is Data Disk
	 other: # Audio tracks */
/******************************************/
int extern CDQueryAudioCDTracks(HFILE hfDrive)
{
	ULONG ulParamLen;
	ULONG ulDataLen;
	ULONG rc;
	CDINFO cdInfo;
	TINFOPARAM tip;
	TRACKINFO trackInfo;
	
	ulDataLen=sizeof(cdInfo);
	ulParamLen=4;
		
	if(!hfDrive)
		return 0;
	
	rc = DosDevIOCtl(hfDrive, IOCTL_CDROMAUDIO, CDROMAUDIO_GETAUDIODISK,
						  "CD01", 4, &ulParamLen, &cdInfo,
						  sizeof(cdInfo), &ulDataLen);

	if(rc) {
		return 0;//Error
	}

	ulDataLen=sizeof(trackInfo);
	ulParamLen=sizeof(TINFOPARAM);
	tip.signature[0]='C';
	tip.signature[1]='D';	
	tip.signature[2]='0';
	tip.signature[3]='1';
	tip.ucTrackNum=1;
	
	/* We have a disc. Check if it's audio */
	rc = DosDevIOCtl(hfDrive, IOCTL_CDROMAUDIO, CDROMAUDIO_GETAUDIOTRACK,
						  &tip, sizeof(tip), &ulParamLen, &trackInfo,
						  sizeof(trackInfo), &ulDataLen);

	if(rc) {
		return 0;//Error
	}
		
	if(trackInfo.ucTCInfo & 0x40) {
		return -1;//It's a data track
	}
		
	return cdInfo.ucLastTrack-cdInfo.ucFirstTrack+1;
	
}

/* Returns the UPC of CD in drive 'drive' (e.g. O:) */
/* Size of ucUCP must be 10!                        */
ULONG extern CDQueryUPC(UCHAR* ucUPC, char * drive)
{
  HFILE hfDrive;
  ULONG ulParamLen;
  ULONG ulDataLen;
  ULONG rc;
  char setup[100]; 
 
  hfDrive=openDrive(drive);
  if(!hfDrive) return -1;

  do {
    DosBeep(1000,200);
    /* Get UPC info */
    ulDataLen=10;
    ulParamLen=4;
    rc = DosDevIOCtl(hfDrive, IOCTL_CDROMDISK, CDROMDISK_GETUPC,
                     "CD01", 4, &ulParamLen, ucUPC,
                     10, &ulDataLen);
    sprintf(setup,"rc is: %x",rc);

    WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP,  
                  setup, "GET_UPC", 0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );

    if(rc)
      break;
    DosBeep(2000,200);
    closeDrive(hfDrive);
    return 0;

  }while(TRUE);

  /* No error */
  closeDrive(hfDrive);
  return -1;
}


/* Returns the size in bytes of a track on drive 'drive' (e.g. O:) */
/* The track num starts with 1 */
LONG extern CDQueryTrackSize(ULONG numTrack, char * drive)
{
	HFILE hfDrive;
	ULONG ulParamLen;
	ULONG ulDataLen;
	ULONG rc;
	CDINFO cdInfo;
	TINFOPARAM tip;
	TRACKINFO trackInfo[2];

	hfDrive=openDrive(drive);
	if(!hfDrive) return 0;
	
	do {
		/* Get cd info */
		ulDataLen=sizeof(cdInfo);
		ulParamLen=4;
		rc = DosDevIOCtl(hfDrive, IOCTL_CDROMAUDIO, CDROMAUDIO_GETAUDIODISK,
										 "CD01", 4, &ulParamLen, &cdInfo,
										 sizeof(cdInfo), &ulDataLen);		
		if(rc)
			break;//Error
		ulDataLen=sizeof(trackInfo);
		ulParamLen=sizeof(TINFOPARAM);
		tip.signature[0]='C';
		tip.signature[1]='D';	
		tip.signature[2]='0';
		tip.signature[3]='1';
		/* Get information about our track */
		tip.ucTrackNum=numTrack;
		rc = DosDevIOCtl(hfDrive, IOCTL_CDROMAUDIO, CDROMAUDIO_GETAUDIOTRACK,
										 &tip, sizeof(tip), &ulParamLen, &trackInfo[0],
										 sizeof(trackInfo[0]), &ulDataLen);
		if(rc) 
			break;//Error

		/* Get information about next track */
		tip.ucTrackNum=numTrack+1;
		if(tip.ucTrackNum<=cdInfo.ucLastTrack) {
			rc = DosDevIOCtl(hfDrive, IOCTL_CDROMAUDIO, CDROMAUDIO_GETAUDIOTRACK,
											 &tip, sizeof(tip), &ulParamLen, &trackInfo[1],
											 sizeof(trackInfo[1]), &ulDataLen);
			if(rc) 
				break;//Error
		}
		else {
          trackInfo[1].ulTrackAddress=cdInfo.ulLeadOut;
          /* #pragma pack(1) eingebaut */

          /*BYTE *cdInfoBytes;
            cdInfoBytes=(BYTE*)&cdInfo;
            cdInfoBytes+=2;
            trackInfo[1].ulTrackAddress=*((ULONG*)cdInfoBytes);*/
        }
        closeDrive(hfDrive);
		return CDCalculateSize((MSF*)&trackInfo[0].ulTrackAddress, (MSF*)&trackInfo[1].ulTrackAddress);
	}while(TRUE);
	closeDrive(hfDrive);
	return 0;
}

/* Returns sector info of track #numTrack */
/* Starting with track 0 */
LONG extern CDQueryTrackStartSector(ULONG numTrack, HFILE hfDrive)
{
	
	ULONG ulParamLen;
	ULONG ulDataLen;
	ULONG rc;
	CDINFO cdInfo;
	TINFOPARAM tip;
	TRACKINFO trackInfo[2];

	do {
      /* Get cd info */
      ulDataLen=sizeof(cdInfo);
      ulParamLen=4;
      rc = DosDevIOCtl(hfDrive, IOCTL_CDROMAUDIO, CDROMAUDIO_GETAUDIODISK,
                       "CD01", 4, &ulParamLen, &cdInfo,
                       sizeof(cdInfo), &ulDataLen);		
      if(rc)
        break;//Error
      
      ulDataLen=sizeof(trackInfo);
		ulParamLen=sizeof(TINFOPARAM);
		tip.signature[0]='C';
		tip.signature[1]='D';	
		tip.signature[2]='0';
		tip.signature[3]='1';
		/* Get information about our track */
		tip.ucTrackNum=numTrack+1;
		if(tip.ucTrackNum<=cdInfo.ucLastTrack) {
          rc = DosDevIOCtl(hfDrive, IOCTL_CDROMAUDIO, CDROMAUDIO_GETAUDIOTRACK,
                           &tip, sizeof(tip), &ulParamLen, &trackInfo[0],
                           sizeof(trackInfo[0]), &ulDataLen);
          if(rc) 
            break;//Error
          }else 
            return CDCalculateSector((MSF*)&cdInfo.ulLeadOut);
        

        return CDCalculateSector((MSF*)&trackInfo[0].ulTrackAddress);

	}while(TRUE);
	
    /* error */
    printf("ERROR!!!!!!!!!\n");
	return 0;
}

/*****************************************/
/*                                       */
/* Plays track #numTrack on              */
/* CD-Drive 'drive'                      */
/*                                       */
/* Returns TRUE if successful            */
/*                                       */
/*****************************************/
BOOL CDPlayTrack(ULONG numTrack, char * drive)
{
	HFILE hfDrive;
	ULONG ulParamLen;
	ULONG ulDataLen;
	ULONG rc;
	CDINFO cdInfo;
	TINFOPARAM tip;
	TRACKINFO trackInfo[2];
    PLAYPARAM playParam;

	hfDrive=openDrive(drive);
	if(!hfDrive) return FALSE;
    do {
	/* Get cd info */
		ulDataLen=sizeof(cdInfo);
		ulParamLen=4;
		rc = DosDevIOCtl(hfDrive, IOCTL_CDROMAUDIO, CDROMAUDIO_GETAUDIODISK,
										 "CD01", 4, &ulParamLen, &cdInfo,
										 sizeof(cdInfo), &ulDataLen);		
		if(rc)
			break;//Error
		ulDataLen=sizeof(trackInfo);
		ulParamLen=sizeof(TINFOPARAM);
		tip.signature[0]='C';
		tip.signature[1]='D';	
		tip.signature[2]='0';
		tip.signature[3]='1';
		/* Get information about our track */
		tip.ucTrackNum=numTrack;
		rc = DosDevIOCtl(hfDrive, IOCTL_CDROMAUDIO, CDROMAUDIO_GETAUDIOTRACK,
										 &tip, sizeof(tip), &ulParamLen, &trackInfo[0],
										 sizeof(trackInfo[0]), &ulDataLen);
		if(rc) 
			break;//Error
		/* Get information about next track */
		tip.ucTrackNum=numTrack+1;
		if(tip.ucTrackNum<=cdInfo.ucLastTrack) {
			rc = DosDevIOCtl(hfDrive, IOCTL_CDROMAUDIO, CDROMAUDIO_GETAUDIOTRACK,
											 &tip, sizeof(tip), &ulParamLen, &trackInfo[1],
											 sizeof(trackInfo[1]), &ulDataLen);
			if(rc) 
				break;//Error
		}
		else {
          /* The following does not work on my system. Compilerbug ? 
             trackInfo[1].ulTrackAddress=cdInfo.ulLeadOut; */
          BYTE *cdInfoBytes;
          cdInfoBytes=(BYTE*)&cdInfo;
          cdInfoBytes+=2;
          trackInfo[1].ulTrackAddress=*((ULONG*)cdInfoBytes);
        }

              /* Play the Track... */
        ulParamLen=sizeof(PLAYPARAM);
		playParam.signature[0]='C';
		playParam.signature[1]='D';	
		playParam.signature[2]='0';
		playParam.signature[3]='1';
        playParam.ucAddressMode=01;
        memcpy(&playParam.ucStartSector,&trackInfo[0].ulTrackAddress,4);
        memcpy(&playParam.ucEndSector,&trackInfo[1].ulTrackAddress,4);
        
        rc = DosDevIOCtl(hfDrive, IOCTL_CDROMAUDIO, CDROMAUDIO_PLAYAUDIO,
                         &playParam, sizeof(playParam), &ulParamLen,0,
                         0, 0);
        if(rc)
          break;
        closeDrive(hfDrive);
        return TRUE;
    }while(TRUE);
	closeDrive(hfDrive);
    return FALSE;
}

BOOL extern CDStop(char * drive)
{
	HFILE hfDrive;
	ULONG ulParamLen;
    ULONG rc;

	hfDrive=openDrive(drive);
	if(!hfDrive) return FALSE;
    
    do {
      /* Stop CD */
      ulParamLen=4;
      rc = DosDevIOCtl(hfDrive, IOCTL_CDROMAUDIO, CDROMAUDIO_STOPAUDIO,
                       "CD01", 4, &ulParamLen,0,
                       0, 0);		
      if(rc)
        break;//Error
      closeDrive(hfDrive);
      return TRUE;
    }while(TRUE);
	closeDrive(hfDrive);
    return FALSE;
}

LONG extern queryFreeDriveSpace(ULONG diskNum) 
{
	FSALLOCATE infoBuf;
	
	if(DosQueryFSInfo(diskNum,FSIL_ALLOC,&infoBuf,sizeof(infoBuf)))
		return 0;
	
	return infoBuf.cUnitAvail*infoBuf.cbSector*infoBuf.cSectorUnit;
}

/**************************************************************/
/*                                                            */
/* This funktion returns the CD-Drives in the system          */
/*                                                            */
/* iNumCD (output): # of CD-Drives                            */
/* cFirstDrive (output): first cd-Drive letter                */
/* returns TRUE: We have cd-drives                            */
/*                                                            */
/**************************************************************/
BOOL extern CDQueryCDDrives(int *iNumCD, char * cFirstDrive)
{
  HFILE hfDevice;
  ULONG ulAction;
  ULONG ulLen;
  static char cFirst=0;
  static int iNumCDLocal=0;
  static BOOL haveCD=FALSE;
  static BOOL done=FALSE;			
  struct
  {
    USHORT usCountCD;
    USHORT usFirstCD;
  } CDInfo;
  
  if(!done){
    ulLen = sizeof(CDInfo);
    if(!DosOpen("\\DEV\\CD-ROM2$", &hfDevice, &ulAction, 0,
                FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, NULL))
      {
        if(!DosDevIOCtl(hfDevice, 0x82, 0x60, NULL, 0, NULL, &CDInfo, ulLen, &ulLen))
          {
            if(CDInfo.usCountCD) {
              haveCD=TRUE;
              iNumCDLocal=CDInfo.usCountCD;
              cFirst='A'+ CDInfo.usFirstCD;
            }										 
          }
        DosClose(hfDevice);
      }
    done=TRUE;
  }
  *iNumCD=iNumCDLocal;
  *cFirstDrive=cFirst;
  return haveCD;				
}


/*****************************************/
/*                                       */
/* Query if a CD is avaiable in          */
/* CD-Drive 'drive'                      */
/*                                       */
/* Input: drive num (starting with 0)    */
/* Returns TRUE if media is avaiable     */
/*                                       */
/*****************************************/
BOOL CDQueryMedia(ULONG  driveNum)
{
  return FALSE;
}


int cddb_sum(int n)
{
  int ret;
  //printf("n:  %d   ",n);
  for (ret = 0; n > 0; n /= 10) {
    ret += (n % 10);
  }
  //printf("ret:  %d   ",ret);
  return ret;
}

/* Returns a CDDB disc-ID for quering the database */
/* Returns 0 if error.                             */
LONG extern CDDBDiscID(char * drive, CDDBINFO * cddbInfo)
{
  int numTracks,a;
  HFILE hfDrive;
  LONG sum;
  int t;

#if 0
  cddbInfo->startSector[0]=150;
  cddbInfo->startSector[1]=14765;
  cddbInfo->startSector[2]=32512;
  cddbInfo->startSector[3]=69230;
  cddbInfo->startSector[4]=90632;
  cddbInfo->startSector[5]=103437;
  cddbInfo->startSector[6]=116932;
  cddbInfo->startSector[7]=134342;
  cddbInfo->startSector[8]=159705;
  cddbInfo->startSector[9]=180852;
  cddbInfo->startSector[10]=201460;
  cddbInfo->startSector[11]=219237;
  cddbInfo->startSector[12]=230510;
  cddbInfo->startSector[13]=246537;
  cddbInfo->startSector[14]=259305;
  cddbInfo->startSector[15]=275935;
  cddbInfo->startSector[16]=289800;

  cddbInfo->numTracks=17;
  cddbInfo->discLength=4067;
  cddbInfo->discid=0x0e0fe111;
    return cddbInfo->discid;
#endif

  do {
    hfDrive=openDrive(drive);
    if(!hfDrive)
      return 0;

    numTracks=CDQueryAudioCDTracks(hfDrive);
    if(numTracks==0||numTracks==-1)
      break;
    cddbInfo->numTracks=numTracks;

    sum=0;
    /* Calculating the id */
    for(a=0;a<numTracks;a++) {
      sum+= cddb_sum(CDQueryTrackStartSector(a, hfDrive)/75 );
      //printf("Track%d: %d\n",a,CDQueryTrackStartSector(a, hfDrive));
      cddbInfo->startSector[a]=CDQueryTrackStartSector(a, hfDrive);
    }
    t=CDQueryTrackStartSector(a, hfDrive)/75-CDQueryTrackStartSector(0, hfDrive)/75;
    /*printf("(a-1): %d  0: %d t:  %d\n",CDQueryTrackStartSector(a, hfDrive),
           CDQueryTrackStartSector(0, hfDrive),t);*/
    //printf("discid: %x\n",((sum % 0xff) << 24 | (t << 8) | numTracks));
    //printf("Disclength: %d",(CDQueryTrackStartSector(a, hfDrive)-CDQueryTrackStartSector(0, hfDrive))/75);
    cddbInfo->discLength=(CDQueryTrackStartSector(a, hfDrive)-CDQueryTrackStartSector(0, hfDrive))/75;
    cddbInfo->discid=(sum % 0xff) << 24 | (t << 8) | numTracks;
    closeDrive(hfDrive);
    return cddbInfo->discid;
  }while(TRUE);

  /* error */
  closeDrive(hfDrive);
  return 0;
}



