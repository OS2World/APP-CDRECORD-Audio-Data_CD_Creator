/*
 * Copyright (c) Chris Wohlgemuth 1999-2005 
 * All rights reserved.
 *
 * http://www.os2world.com/cdwriting
 * http://www.geocities.com/SiliconValley/Sector/5785/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The authors name may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#define INCL_GPIBITMAPS
#include "audiofolder.hh"
#include "audiofolderhelp.h"

#include <stdlib.h>
#include <stdio.h>

#include "cddb.h"

HWND hwndHintDaemon=0;

extern GLOBALDATA globalData;

extern char chrConfigDir[CCHMAXPATH];

extern HMODULE hSettingsResource;
extern BOOL bGotIniValues;

extern int iNumCD;
extern char cFirstCD;
extern char chosenCD[3];
extern char chosenWriter[3];// Drive letter of CD-writer

extern int iMp3Decoder;

extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern char chrDataCDROptions[CCHMAXPATH];
extern char chrAudioCDROptions[CCHMAXPATH];
extern LONG  lCDROptions;

extern char chrMkisofs[CCHMAXPATH];/* Path to mkisofs */
extern char chrMkisofsOptions[CCHMAXPATH];
extern LONG lMKOptions;
extern BOOL bDisableCp;
extern char chrImage[CCHMAXPATH];/* Path to last image file */

extern char chrCdrdaoPath[CCHMAXPATH];
extern char chrCdrdaoDriver[100];
extern char chrDeviceName[CCHMAXPATH];
extern char chrWriterName[CCHMAXPATH];
extern char chrCdrdaoOptions[CCHMAXPATH];

extern char chrDVDDao[CCHMAXPATH];/* Path to dvdao */
extern char chrDVDDaoOptions[CCHMAXPATH];

extern int iBus;
extern int iTarget;
extern int iLun;
extern int iSpeed;
extern int iFifo;

/* MP3 encoder settings */
extern char chrMP3EncoderPath[CCHMAXPATH];
extern char chrMP3EncoderOptions[CCHMAXPATH];
extern ULONG ulMP3Bitrate;
extern ULONG ulMP3Quality;
extern ULONG ulMP3EncoderPriority;

extern BOOL setupDone;
extern BOOL GrabberSetupDone;

extern char chrCDDBServer[100];
extern char chrCDDBUser[100];
extern char chrCDDBUserHost[100];
extern BOOL bUseCDDB;
extern char cddbServer[MAXSERVERS][100];
extern int NUMSERVERS;

extern char chrInstallDir[CCHMAXPATH];
extern char chrConfigDir[CCHMAXPATH];
extern char chrTBFlyFontName[CCHMAXPATH];/* Font for toolbar fly over help */
extern RGB rgbTBFlyForeground;
extern RGB rgbTBFlyBackground;
extern BOOL bTBFlyOverEnabled;
extern int iTBFlyOverDelay;

extern BOOL bHintEnabled;
extern BOOL bUseCustomPainting;

/* Paths to the ISOFS mount/unmount programs */
extern char chrMntIsoFSPath[CCHMAXPATH];
extern char chrUmntIsoPath[CCHMAXPATH];

extern char g_cChosenMP3Name[NUM_MP3NAMEPARTS];/* Array holding the indexes into the names array for each MP3 name part */
extern char g_chrMP3NameFillStrings[NUM_MP3NAME_FILLSTRINGS][MP3NAME_FILLSTRING_LEN];

void errorResource();
ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);
ULONG showMsgBox2(HWND hwnd, ULONG ulIDTitle, ULONG ulIDText, HMODULE hModule, ULONG ulFlag);
void sendCommand(char *chrObject, char* command);
void _Optlink toolsThreadFunc (void *arg);
void addSettingsToFile(FILE *fHandle);
extern BOOL checkFileExists(char* chrFileName);
BOOL installLanguageDLL(char *dllPath, HMODULE hModule);

BOOL CWCreatorSettings::wpSetup(PSZ pSetupString)
{
  /**************************************************
   *                                                *
   * Supported setupstrings:                        *
   *                                                *
   *                                                *
   **************************************************/
  char buffer[CCHMAXPATH+50];
  char text[CCHMAXPATH];
  ULONG bufferSize;
  ULONG ulWFlags;
  HINI hini;
  somId mySomId;
  BOOL rc;

#if 0
  /* HWND of PMHINT daemon */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString, "CDCHINTDAEMONHWND", buffer, &bufferSize))
    { 
      hwndHintDaemon=atol(buffer);
      //      DosBeep(1000,200);
    }
#endif

  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, SETUP_FLDRHINTENABLE, buffer, &bufferSize))
    { 
      ulWFlags=atol(buffer);
      if(ulWFlags==1)
        bHintEnabled=1;
      else if(ulWFlags==0)
        bHintEnabled=0;
    }

  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, SETUP_DAYTIPENABLE, buffer, &bufferSize))
    { 
      ulWFlags=atol(buffer);
      if(ulWFlags==1)
        globalData.bTipsEnabled=1;
      else if(ulWFlags==0)
        globalData.bTipsEnabled=0;
    }

  /* Toolbar settings */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString,SETUP_TOOLBARFLYOVERENABLE,buffer,&bufferSize))
    { 
      if(buffer[0]=='1')
        bTBFlyOverEnabled=1;
      else if(buffer[0]=='0')
        bTBFlyOverEnabled=0;
    }
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString,SETUP_TOOLBARFLYOVERDELAY,buffer,&bufferSize))
    { 
      iTBFlyOverDelay=atoi(buffer);
      if(iTBFlyOverDelay>MAXDELAY)
        iTBFlyOverDelay=MAXDELAY;
    }

  /* FIFO size */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString,SETUP_FIFOSIZE,buffer,&bufferSize))
    { 
      iFifo=atoi(buffer);
      if(iFifo<0)
        iFifo=0;
      else if(iFifo>64)
        iFifo=64;/* OS/2 only allows 64MB shared mem at all. Maybe 48MB or 32MB would be better. */
    }

  /* Write speed */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString,SETUP_WRITESPEED,buffer,&bufferSize))
    { 
      iSpeed=atoi(buffer);
      if(iSpeed<0)
        iSpeed=1;
    }

  /* An easy way for any PM helper or dialog to provide online help */
  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString,SETUP_DISPLAYHELPPANEL,buffer,&bufferSize))
    { 
      ulWFlags=atol(buffer);
      wpDisplayHelp(ulWFlags,AFHELPLIBRARY);
    }
  /* Same as above but the help library may be specified */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString,SETUP_DISPLAYHELPPANELEXT,buffer,&bufferSize))
    {
      char *chrPtr;

      /* Find end of help library path */
      if((chrPtr=strrchr(buffer,':'))!=NULLHANDLE) {
        *(chrPtr++)=0;
        wpDisplayHelp(atol(chrPtr), buffer);
      }
    }

  /* Path to cdrecord */
  bufferSize=sizeof(chrCDRecord); 
  wpScanSetupString(pSetupString, SETUP_CDRECORDPATH,chrCDRecord,&bufferSize);
  
  bufferSize=sizeof(chrAudioCDROptions); 
  if(wpScanSetupString(pSetupString, SETUP_CDRECORDOPTIONSA, chrAudioCDROptions,&bufferSize))
    { 
      if(!stricmp(chrAudioCDROptions, "CLEAR"))
        chrAudioCDROptions[0]=0;
    }

  bufferSize=sizeof(chrDataCDROptions); 
  if(wpScanSetupString(pSetupString, SETUP_CDRECORDOPTIONSD, chrDataCDROptions,&bufferSize))
    { 
      if(!stricmp(chrDataCDROptions, "CLEAR"))
        chrDataCDROptions[0]=0;     
    }
  /* Path to mkisofs */
  bufferSize=sizeof(chrMkisofs); 
  if(wpScanSetupString(pSetupString, SETUP_MKISOFSPATH, chrMkisofs,&bufferSize))
    { 
     
    }
  bufferSize=sizeof(chrMkisofsOptions); 
  if(wpScanSetupString(pSetupString, SETUP_MKISOFSOPTIONS, chrMkisofsOptions,&bufferSize))
    { 
      if(!stricmp(chrMkisofsOptions,"CLEAR"))
        chrMkisofsOptions[0]=0;
    }
  /* Path to cdrdao */
  bufferSize=sizeof(chrCdrdaoPath); 
  if(wpScanSetupString(pSetupString, SETUP_CDRDAOPATH, chrCdrdaoPath,&bufferSize))
    { 
     
    }

  bufferSize=sizeof(chrCdrdaoOptions); 
  if(wpScanSetupString(pSetupString, SETUP_CDRDAOOPTIONS, chrCdrdaoOptions,&bufferSize))
    { 
      if(!stricmp(chrCdrdaoOptions, "CLEAR"))
        chrCdrdaoOptions[0]=0;          
    }

  /* Path to cdrdao 2 (source) */
  bufferSize=sizeof(globalData.chrCdrdaoPath2); 
  wpScanSetupString(pSetupString, SETUP_CDRDAOPATH2, globalData.chrCdrdaoPath2,&bufferSize);

  bufferSize=sizeof(globalData.chrCdrdaoOptions2); 
  if(wpScanSetupString(pSetupString, SETUP_CDRDAOOPTIONS2, globalData.chrCdrdaoOptions2,&bufferSize)){ 
    if(!stricmp(globalData.chrCdrdaoOptions2, "CLEAR"))
      globalData.chrCdrdaoOptions2[0]=0;          
  }
    
  
  /* Path to cdrdao 3 (target) */
  bufferSize=sizeof(globalData.chrCdrdaoPath3); 
  wpScanSetupString(pSetupString, SETUP_CDRDAOPATH3, globalData.chrCdrdaoPath3,&bufferSize);

  bufferSize=sizeof(globalData.chrCdrdaoOptions3); 
  if(wpScanSetupString(pSetupString, SETUP_CDRDAOOPTIONS3, globalData.chrCdrdaoOptions3,&bufferSize)){ 
    if(!stricmp(globalData.chrCdrdaoOptions3, "CLEAR"))
      globalData.chrCdrdaoOptions3[0]=0;          
  }
  

#if 0
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString,"CDRDAOWRITER",buffer,&bufferSize))
    { 
     
    }
#endif
  /* Path to the grabber */
  bufferSize=sizeof(globalData.chrGrabberPath); 
  if(wpScanSetupString(pSetupString, SETUP_GRABBERPATH, globalData.chrGrabberPath,&bufferSize))
    { 
     
    }
  /* Grabber options */
  bufferSize=sizeof(globalData.chrGrabberOptions); 
  if(wpScanSetupString(pSetupString, SETUP_GRABBEROPTIONS, globalData.chrGrabberOptions,&bufferSize))
    { 
      if(!stricmp(globalData.chrGrabberOptions,"CLEAR"))
        globalData.chrGrabberOptions[0]=0;     
    }
  /* Select a grabber */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString, SETUP_GRABBERSELECT, buffer,&bufferSize))
    { 
      /* Predefined grabber? */
      sprintf(text,"%s\\cfgdata.ini", chrConfigDir);
      hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),text);
      if(hini)
        {
          if(PrfQueryProfileString(hini,buffer,"options", NULL, text, sizeof(text))){               
            /* Options found */   
            strcpy(globalData.chrGrabberOptions, text);
            globalData.bTrackNumbers=PrfQueryProfileInt(hini, buffer,"addnumbers",0);
            strcpy(globalData.chrGrabberName,buffer);
            globalData.iGrabberID=PrfQueryProfileInt(hini, globalData.chrGrabberName, "ID", IDGRABBER_UNKNOWN);
          }
          PrfCloseProfile(hini);
        }
    }

  /* Path to MP3 decoder */
  bufferSize=sizeof(globalData.chrMpg123Path); 
  if(wpScanSetupString(pSetupString, SETUP_MP3DECPATH, globalData.chrMpg123Path,&bufferSize))
    { 
     
    }
  /* Select MP3 decoder */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString, SETUP_MP3SELECT, buffer,&bufferSize))
    { 
      if(!stricmp(buffer,"z!"))
        iMp3Decoder=IDKEY_USEZ;
      else if(!stricmp(buffer,"mpg123"))
        iMp3Decoder=IDKEY_USEMPG123;
      else if(!stricmp(buffer,"MMIO"))
        iMp3Decoder=IDKEY_USEMMIOMP3;
    }

  /* Swap MP3 bytes */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString, SETUP_MP3SWAPBYTES, buffer, &bufferSize))
    { 
      ulWFlags=atol(buffer);
      if(ulWFlags==1)
        globalData.bMpg123SwabBytes=1;
      else if(ulWFlags==0)
        globalData.bMpg123SwabBytes=0;
    }

  /* Path to MP3 encoder */
  bufferSize=sizeof(chrMP3EncoderPath); 
  if(wpScanSetupString(pSetupString, SETUP_MP3ENCODERPATH, chrMP3EncoderPath, &bufferSize))
    { 
     
    }
  bufferSize=sizeof(chrMP3EncoderOptions); 
  if(wpScanSetupString(pSetupString, SETUP_MP3ENCODEROPTIONS, chrMP3EncoderOptions,&bufferSize))
    { 
      if(!stricmp(chrMP3EncoderOptions,"CLEAR"))
        chrMP3EncoderOptions[0]=0;
    }
  /* MP3 encoder priority */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString, SETUP_MP3ENCODERPRIORITY, buffer,&bufferSize))
    { 
      ULONG ulTemp=atol(buffer);
      if(ulTemp<=0)
        ulTemp=0;
      else if(ulTemp>4)
        ulTemp=4;
      ulMP3EncoderPriority=ulTemp;
    }

  /* Select MP3 encoding quality */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString, SETUP_MP3ENCODERQUALITY, buffer,&bufferSize))
    { 
      if(!stricmp(buffer,"STANDARD"))
        ulMP3Quality=IDQUALITY_VBRSTANDARD;
      else if(!stricmp(buffer,"EXTREME"))
        ulMP3Quality=IDQUALITY_VBREXTREME;
      else if(!stricmp(buffer,"BITRATE"))
        ulMP3Quality=IDQUALITY_AVERAGE;
      else if(!stricmp(buffer,"USERDEFINED"))
        ulMP3Quality=IDQUALITY_USERDEFINED;
    }

  /* MP3 encoder bitrate */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString, SETUP_MP3ENCODERBITRATE, buffer,&bufferSize))
    { 
      ULONG ulTemp=atol(buffer);
      if(ulTemp<=0)
        ulTemp=32;
      else if(ulTemp>320)
        ulTemp=320;
      ulMP3Bitrate=ulTemp;
    }

  /* Path to music library */
  bufferSize=sizeof(globalData.chrMP3LibraryPath); 
  if(wpScanSetupString(pSetupString, SETUP_MUSICLIBRARYPATH, globalData.chrMP3LibraryPath, &bufferSize))
    { 
     
    }

  /* Show the CDRTools dialog */
  bufferSize=sizeof(buffer); 
  if(wpScanSetupString(pSetupString, SETUP_OPENCDRTOOLS, buffer,&bufferSize))
    { 
      _beginthread(toolsThreadFunc,NULL,8192*8,this); //Fehlerbehandlung fehlt     
    }

  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, SETUP_WRITECONFIGTOFILE, buffer, &bufferSize))
    { 
      FILE *fHandle;
      /* Write the current settings to the given file */
      fHandle=fopen(buffer,"a");
      if(fHandle) {
        addSettingsToFile(fHandle);
        fclose(fHandle);
      }
    }

  /* Install a new language DLL */
  bufferSize=sizeof(buffer);
  if(wpScanSetupString(pSetupString, SETUP_INSTLANGUAGEDLL, buffer, &bufferSize))
    { 
      if(installLanguageDLL(buffer, hSettingsResource))
        showMsgBox2(HWND_DESKTOP, IDSTR_INSTLANGUAGETITLE, IDSTR_INSTLANGUAGEDLLDONE, hSettingsResource, MB_MOVEABLE|MB_OK|MB_INFORMATION);
      else
        showMsgBox2(HWND_DESKTOP, IDSTR_INSTLANGUAGETITLE, IDSTR_INSTLANGUAGEDLLFAILED, hSettingsResource, MB_MOVEABLE|MB_OK|MB_WARNING);
    }


  if((mySomId=somIdFromString("wpSetup"))!=NULLHANDLE) {
    rc=((somTD_WPObject_wpSetup)
        somParentNumResolve(__ClassObject->somGetPClsMtabs(),
                            1,
                            __ClassObject->    
                            somGetMethodToken(mySomId))                    
        )(this,pSetupString);
    SOMFree(mySomId);
    return rc;
  }
  else 
    return WPAbstract::wpSetup(pSetupString);
}

BOOL CWCreatorSettings::wpSaveState()
{
  char profileName[CCHMAXPATH];
  char moduleName[CCHMAXPATH];
  char *chrPtr;
  char chrDev[10];
  HINI hini=0;
  BOOL bError=TRUE;
  int a;
  somId mySomId;
  BOOL rc;

  /* Save Settings to ini */
  /* The ini file entries will be loaded in wpclsInitData() */

  /* Build the ini-file name */
  /* Build full path */
  sprintf(profileName,"%s\\cdrecord.ini",chrInstallDir);       
  /* Global settings are saved in an ini-file */
  do{  
    /* Open the ini-file */
    hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),profileName);
    if(!hini) {
      /* profileName: "Warning! Cannot open Ini-file!"
         moduleName: "Audio-CD-Creator"
         */
      messageBox( profileName, IDSTR_INIFILEOPENWARNING , sizeof(profileName),
                  moduleName, IDSTR_AUDIOCDCREATOR, sizeof(moduleName),
                  hSettingsResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
      break;
    }/* end of if(!hini) */

    /* Save cdrecord-path */
    if(!PrfWriteProfileString(hini,"CDWriter","cdrecord",chrCDRecord)) 
      break;    
    /* Save cdrecord options */
    if(!PrfWriteProfileString(hini,"CDWriter","audiocdroptions",chrAudioCDROptions))    
      break;
    if(!PrfWriteProfileString(hini,"CDWriter","writedrive",chosenWriter))
      break;

    if(!PrfWriteProfileData(hini,"CDWriter","options",&lCDROptions,sizeof(lCDROptions))) {
      break;
    };
    /* Save cdrecord options */
    if(!PrfWriteProfileString(hini,"CDWriter","datacdroptions",chrDataCDROptions))    
      break;

    /* Save grabber options */
    if(!PrfWriteProfileString(hini,"CDGrabber","grabber",globalData.chrGrabberPath))    
      break;
    if(!PrfWriteProfileString(hini,"CDGrabber","graboptions",globalData.chrGrabberOptions))
      break;
    if(!PrfWriteProfileString(hini,"CDGrabber","grabdrive",chosenCD))
      break;
    if(!PrfWriteProfileString(hini,"CDGrabber","grabbername",globalData.chrGrabberName))
      break;
    if(globalData.bTrackNumbers) {
      if(!PrfWriteProfileString(hini,"CDGrabber","tracknumbers","1"))
        break;
    }
    else
      if(!PrfWriteProfileString(hini,"CDGrabber","tracknumbers","0"))
        break;
    /* Save the grabber id */
    sprintf(chrDev,"%d", globalData.iGrabberID);
    if(!PrfWriteProfileString(hini,"CDGrabber","ID",chrDev))
      break;

    /* Save MP3 info */
    if(!PrfWriteProfileString(hini,"mpg123","path", globalData.chrMpg123Path))
      break;
    if(globalData.bMpg123SwabBytes) {
      if(!PrfWriteProfileString(hini,"mpg123","swabbytes","1"))
        break;
    }
    else
      if(!PrfWriteProfileString(hini,"mpg123","swabbytes","0"))
        break;
    sprintf(chrDev,"%d",iMp3Decoder); /* Make a string from int */
    if(!PrfWriteProfileString(hini,"mpg123","decoder",chrDev))
      break;

    /* DVD settings */
    if(!PrfWriteProfileString(hini,"dvdwriting","writerpath", chrDVDDao))
      break;
    if(!PrfWriteProfileString(hini,"dvdwriting","options", chrDVDDaoOptions))
      break;

    /* MP3 encoder settings */
    if(!PrfWriteProfileString(hini,"mp3encode","encoderpath",chrMP3EncoderPath))
      break;
    if(!PrfWriteProfileString(hini,"mp3encode","options",chrMP3EncoderOptions))
      break;
    sprintf(chrDev,"%d", ulMP3Quality);
    if(!PrfWriteProfileString(hini,"mp3encode","quality", chrDev))
      break;
    sprintf(chrDev,"%d", ulMP3Bitrate);
    if(!PrfWriteProfileString(hini,"mp3encode","bitrate", chrDev))
      break;
    sprintf(chrDev,"%d", ulMP3EncoderPriority);
    if(!PrfWriteProfileString(hini,"mp3encode","priority", chrDev))
      break;

    if(!PrfWriteProfileString(hini,"mp3encode", "librarypath", globalData.chrMP3LibraryPath))
      break;

    if(!PrfWriteProfileData(hini, "mp3naming", "order", g_cChosenMP3Name, sizeof(g_cChosenMP3Name)))
      break;
    if(!PrfWriteProfileData(hini, "mp3naming", "fillstrings", g_chrMP3NameFillStrings, sizeof(g_chrMP3NameFillStrings)))
      break;


    /* Cdrdao for audio */
    if(!PrfWriteProfileString(hini,"cdrdao","path",chrCdrdaoPath))
      break;
    if(!PrfWriteProfileString(hini,"cdrdao","driver",chrCdrdaoDriver))
      break;
    if(!PrfWriteProfileString(hini,"cdrdao","writer",chrWriterName))
      break;
    if(!PrfWriteProfileString(hini,"cdrdao","options", chrCdrdaoOptions))
      break;

    /* Cdrdao for 1:1 copy source */
    if(!PrfWriteProfileString(hini,"cdrdaosrc","path", globalData.chrCdrdaoPath2))
      break;
    if(!PrfWriteProfileString(hini,"cdrdaosrc","driver", globalData.chrCdrdaoDriver2))
      break;
    if(!PrfWriteProfileString(hini,"cdrdaosrc","writer", globalData.chrWriterName2))
      break;
    if(!PrfWriteProfileString(hini,"cdrdaosrc","options", globalData.chrCdrdaoOptions2))
      break;
    if(!PrfWriteProfileString(hini,"cdrdaosrc","devicename", globalData.chrDeviceName2))
      break;

    /* Cdrdao for 1:1 copy target */
    if(!PrfWriteProfileString(hini,"cdrdaotarget","path", globalData.chrCdrdaoPath3))
      break;
    if(!PrfWriteProfileString(hini,"cdrdaotarget","driver", globalData.chrCdrdaoDriver3))
      break;
    if(!PrfWriteProfileString(hini,"cdrdaotarget","writer", globalData.chrWriterName3))
      break;
    if(!PrfWriteProfileString(hini,"cdrdaotarget","options", globalData.chrCdrdaoOptions3))
      break;
    if(!PrfWriteProfileString(hini,"cdrdaotarget","devicename", globalData.chrDeviceName3))
      break;
   
    sprintf(chrDev,"%d",iBus);
    if(!PrfWriteProfileString(hini,"device","bus",chrDev))
      break;
    sprintf(chrDev,"%d",iTarget);
    if(!PrfWriteProfileString(hini,"device","target",chrDev))
      break;
    sprintf(chrDev,"%d",iLun);
    if(!PrfWriteProfileString(hini,"device","lun",chrDev))
      break;
    if(!PrfWriteProfileString(hini,"device","devicename",chrDeviceName))
      break;

    sprintf(chrDev,"%d",iSpeed);
    if(!PrfWriteProfileString(hini,"device","speed",chrDev))
      break;

    sprintf(chrDev,"%d",iFifo);
    if(!PrfWriteProfileString(hini,"device","fifo",chrDev))
      break;

    
    /* Save mkisofs options */
    if(!PrfWriteProfileString(hini,"Mkisofs","mkisofs",chrMkisofs)){
      break;
    }
    if(!PrfWriteProfileString(hini,"Mkisofs","mkisofsoptions",chrMkisofsOptions)){
      break;
    };
    if(!PrfWriteProfileData(hini,"Mkisofs","options",&lMKOptions,sizeof(lMKOptions))) {
      break;
    };
    if(bDisableCp) {
      if(!PrfWriteProfileString(hini,"Mkisofs","disablecp","1"))
        break;
    }
    else
      if(!PrfWriteProfileString(hini,"Mkisofs","disablecp","0"))
        break;

    if(!PrfWriteProfileString(hini,"Mkisofs","imagepath",chrImage)){
      break;
    };

    /* Save ISOFS stuff */
    if(!PrfWriteProfileString(hini,"isofs","mountpath",chrMntIsoFSPath))
      break;
    if(!PrfWriteProfileString(hini,"isofs","unmountpath",chrUmntIsoPath))
      break;

    /* Presparams for toolbar */
    if(!PrfWriteProfileString(hini, "toolbar", "flyoverfont", chrTBFlyFontName))
      break;
    if(!PrfWriteProfileData(hini,"toolbar","flyoverbackcolor",&rgbTBFlyBackground,sizeof(rgbTBFlyBackground)))
      break;
    if(!PrfWriteProfileData(hini,"toolbar","flyoverforecolor",&rgbTBFlyForeground,sizeof(rgbTBFlyForeground)))
      break;
    /* Toolbar flyover enable */
    if(bTBFlyOverEnabled) {
      if(!PrfWriteProfileString(hini,"toolbar","flyover","1"))
        break;
    }
    else
      if(!PrfWriteProfileString(hini,"toolbar","flyover","0"))
        break;
    /* Toolbar flyover delay */
    sprintf(chrDev,"%d",iTBFlyOverDelay);
    if(!PrfWriteProfileString(hini,"toolbar","flyoverdelay",chrDev))
      break;

    /* Expert mode */
    sprintf(chrDev,"%d", (globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE ? 1 : 0));
    if(!PrfWriteProfileString(hini,"creator","ExpertMode",chrDev))
      break;

    /* Hint enable */
    if(bHintEnabled) {
      if(!PrfWriteProfileString(hini,"hints","enable","1"))
        break;
    }
    else
      if(!PrfWriteProfileString(hini,"hints","enable","0"))
        break;

    if(bUseCustomPainting) {
      if(!PrfWriteProfileString(hini,"newlook","enable","1"))
        break;
    }
    else
      if(!PrfWriteProfileString(hini,"newlook","enable","0"))
        break;

    if(globalData.bTipsEnabled) {
      if(!PrfWriteProfileString(hini,"tips","enable","1"))
        break;
    }
    else
      if(!PrfWriteProfileString(hini,"tips","enable","0"))
        break;

    /* CDDB */
    if(NUMSERVERS!=0) {
      for(a=0;a<NUMSERVERS;a++) {
        sprintf(profileName,"cddbserver%d",a+1);
        PrfWriteProfileString(hini,"cddb",profileName,cddbServer[a]);
      }
    }
    else {
      /* Erase old entries */
      PrfWriteProfileString(hini,"cddb",NULL,"");
    }
    sprintf(profileName,"%d",NUMSERVERS);
    if(!PrfWriteProfileString(hini,"cddb","numservers",profileName))
      break;

    if(!PrfWriteProfileString(hini,"cddb","cddbserver",chrCDDBServer))
      break;
    if(!PrfWriteProfileString(hini,"cddb","cddbuser",chrCDDBUser))
      break;
    if(!PrfWriteProfileString(hini,"cddb","cddbuserhost",chrCDDBUserHost))
      break;

    if(bUseCDDB) {
      if(!PrfWriteProfileString(hini,"cddb","usecddb","1"))
        break;
    }
    else
      if(!PrfWriteProfileString(hini,"cddb","usecddb","0"))
        break;

    PrfCloseProfile(hini);
    bError=FALSE;
    break;
  }while(TRUE);
  if(bError) {
    /* profileName: "Warning! Cannot write to Ini-file!"
       moduleName: "Audio-CD-Creator"
       */
    messageBox( profileName, IDSTR_WRITEINIWARNING, sizeof(profileName),
                moduleName, IDSTR_AUDIOCDCREATOR, sizeof(moduleName),
                hSettingsResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
    if(hini)PrfCloseProfile(hini);
  }

  if((mySomId=somIdFromString("wpSaveState"))!=NULLHANDLE) {
    rc=((somTD_WPObject_wpSaveState)
        somParentNumResolve(__ClassObject->somGetPClsMtabs(),
                            1,
                            __ClassObject->    
                            somGetMethodToken(mySomId))                    
        )(this);
    SOMFree(mySomId);
    return rc;
  }
  else 
    return WPAbstract::wpSaveState();
}


BOOL CWCreatorSettings::wpAddSettingsPages(HWND hwndNotebook)
{
	ULONG rc;
    somId mySomId;
    
    if(!hSettingsResource) {
      errorResource();
      return FALSE;
    }

    if(globalData.ulGlobalFlags & GLOBALFLAG_EXPERTMODE) {
      if((mySomId=somIdFromString("wpAddSettingsPages"))!=NULLHANDLE) {
        rc=((somTD_WPObject_wpAddSettingsPages)
            somParentNumResolve(__ClassObject->somGetPClsMtabs(),
                                1,
                                __ClassObject->
                                somGetMethodToken(mySomId))                    
            )(this,hwndNotebook);  
        SOMFree(mySomId);
      }
      else 
        rc=WPAbstract::wpAddSettingsPages(hwndNotebook);
      
      /* Expert mode for avanced users with all settings */
      rc = rc /*&& cwAddIsoFSOptionPage(hwndNotebook) */
        && cwAddMkisofsOptionPage(hwndNotebook)
        && cwAddDVDDaoOptionPage(hwndNotebook)
        && cwAddMP3EncoderOptionPage2(hwndNotebook)
        && cwAddMP3EncoderOptionPage(hwndNotebook)
        && cwAddMpg123OptionPage(hwndNotebook)
        && cwAddGrabOptionPage(hwndNotebook)
        && cwAddCdrdaoOptionPage3(hwndNotebook)
        && cwAddCdrdaoOptionPage2(hwndNotebook)
        && cwAddCdrdaoOptionPage(hwndNotebook)
        && cwAddCdrecordOptionPage(hwndNotebook)
        && cwAddCDDBOptionPage2(hwndNotebook)  
        && cwAddCDDBOptionPage(hwndNotebook)
        && cwAddHintOptionPage(hwndNotebook)
        /*  && cwAddToolbarOptionPage(hwndNotebook) */
        && cwAddGeneralOptionPage2(hwndNotebook);
      return(rc && cwAddGeneralOptionPage(hwndNotebook)
             && cwAddAboutPage( hwndNotebook)
             );   
    }
    else {
      /* Beginners mode with only some relevant settings */
      if(!checkFileExists(chrMkisofs))
        rc = rc && cwAddMkisofsOptionPage(hwndNotebook); /* Insert page only if wrong path */ 
      if(!checkFileExists(chrDVDDao))
        rc = rc && cwAddDVDDaoOptionPage(hwndNotebook);
      rc = rc 
        && cwAddMP3EncoderOptionPage2(hwndNotebook)
        && cwAddMP3EncoderOptionPage(hwndNotebook)
        && cwAddMpg123OptionPage(hwndNotebook)
        && cwAddGrabOptionPage(hwndNotebook)
        && cwAddCdrdaoOptionPage3(hwndNotebook)
        && cwAddCdrdaoOptionPage2(hwndNotebook)
        && cwAddCdrdaoOptionPage(hwndNotebook);
      if(!checkFileExists(chrCDRecord))
        rc = rc && cwAddCdrecordOptionPage(hwndNotebook); /* Insert page only if wrong path */ 
      rc = rc
        /*   && cwAddCDDBOptionPage2(hwndNotebook)  */ /* No server page */
        && cwAddCDDBOptionPage(hwndNotebook);
        /* && cwAddHintOptionPage(hwndNotebook)
        && cwAddToolbarOptionPage(hwndNotebook) */
        /* && cwAddGeneralOptionPage2(hwndNotebook);*/
      return(rc && cwAddGeneralOptionPage(hwndNotebook)
             && cwAddAboutPage( hwndNotebook)
             );

    }
}

ULONG CWCreatorSettings::wpAddObjectWindowPage(HWND hwndNotebook)
{
  return SETTINGS_PAGE_REMOVED;
}

BOOL CWCreatorSettings::wpSetupOnce(PSZ pSetupString)
{
  /**************************************************
   *                                                *
   * Supported setupstrings:                        *
   *                                                *
   **************************************************/
  BOOL rcParent;
  char buffer[CCHMAXPATH];
  ULONG bufferSize;
  somId mySomId;

  if((mySomId=somIdFromString("wpSetupOnce"))!=NULLHANDLE) {
    rcParent=((somTD_WPObject_wpSetupOnce)
              somParentNumResolve(__ClassObject->somGetPClsMtabs(),
                                  1,
                                  __ClassObject->    
                                  somGetMethodToken(mySomId))                    
          )(this,pSetupString);
    SOMFree(mySomId);
  }
  else 
    rcParent=WPAbstract::wpSetupOnce(pSetupString);
  
  /* Doing our own setup here */
  wpSetup("DEFAULTVIEW=SETTINGS");
  return rcParent;
}


 
BOOL CWCreatorSettings::wpMenuItemSelected(HWND hwndFrame,ULONG ulMenuId)
{
  int tid;
  somId mySomId;
  BOOL rcParent;
  char chrCommand[100];

  switch(ulMenuId)
    {
    case ID_ABOUTITEM:
      /* 'About' from context menu was called. Tell the Audio-CD-Creator template
         to display the dialog. This is done using a setup string.  */
      sendCommand(ID_AUDIOTEMPLATE, AUDIOFLDRSETUP_SHOWABOUTDIALOG_COMPLETE);
      return TRUE;
    default:
      break;
    }

  /* Call parent */
  if((mySomId=somIdFromString("wpMenuItemSelected"))!=NULLHANDLE) {
    rcParent=((somTD_WPObject_wpMenuItemSelected)
            somParentNumResolve(__ClassObject->somGetPClsMtabs(),
                                1,
                                __ClassObject->    
                                somGetMethodToken(mySomId))                    
            )(this, hwndFrame, ulMenuId);
    SOMFree(mySomId);
  }
  else 
    rcParent=WPAbstract::wpMenuItemSelected(hwndFrame, ulMenuId);
  return rcParent;
}

BOOL CWCreatorSettings::wpModifyPopupMenu(HWND hwndMenu, HWND hwndCnr, ULONG ulPosition)
{
  somId mySomId;
  BOOL rcParent;

  /* Insert the 'About'-Item */
  if(hSettingsResource) {
    wpInsertPopupMenuItems(hwndMenu,-1,hSettingsResource,ID_MENUABOUT,WPMENUID_HELP);
  }
  /* Call parent */
  if((mySomId=somIdFromString("wpModifyPopupMenu"))!=NULLHANDLE) {
    rcParent=((somTD_WPObject_wpModifyPopupMenu)
            somParentNumResolve(__ClassObject->somGetPClsMtabs(),
                                1,
                                __ClassObject->    
                                somGetMethodToken(mySomId))                    
            )(this, hwndMenu, hwndCnr, ulPosition);
    SOMFree(mySomId);
    return rcParent;
  }
  else 
    return WPAbstract::wpModifyPopupMenu( hwndMenu, hwndCnr, ulPosition);
}







