/*
 * This file is (C) Chris Wohlgemuth 1999-2004
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

#define INCL_DOSERRORS
#define INCL_DOSNLS
#define INCL_WINWORKPLACE
#define INCL_GPIBITMAPS
#define INCL_PM
#define INCL_MMIOOS2

#include "audiofolder.hh"
#include "audiofolderhelp.h"

#include <stdlib.h>
#include <stdio.h>

#include "cddb.h"

GLOBALDATA globalData;

HMODULE hSettingsResource=NULLHANDLE;
HMODULE hBaseResource=NULLHANDLE;

extern int iNumCD;
extern char cFirstCD;
extern char chosenCD[3];
extern char chosenWriter[3];// Drive letter of CD-writer

extern int iMp3Decoder;

/* DVD settings */
char chrDVDDao[CCHMAXPATH];
char chrDVDDaoOptions[CCHMAXPATH];

/* MP3 encoder settings */
char chrMP3EncoderPath[CCHMAXPATH];
char chrMP3EncoderOptions[CCHMAXPATH];
ULONG ulMP3Bitrate=DEFAULT_MP3_BITRATE;
ULONG ulMP3Quality=IDQUALITY_VBRSTANDARD;
ULONG ulMP3EncoderPriority=1;

extern char chrAudioCDROptions[CCHMAXPATH];

extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern char chrDataCDROptions[CCHMAXPATH];
extern LONG  lCDROptions;

extern BOOL MkisofsSetupDone;
extern char chrMkisofs[CCHMAXPATH];/* Path to mkisofs */
extern char chrMkisofsOptions[CCHMAXPATH];
extern LONG lMKOptions;
extern char chrImage[CCHMAXPATH];/* Path to last image file */

int iCodePage;
BOOL bDisableCp;

extern char chrCdrdaoPath[CCHMAXPATH];
extern char chrCdrdaoDriver[100];
extern char chrDeviceName[CCHMAXPATH];
extern char chrWriterName[CCHMAXPATH];
char chrCdrdaoOptions[CCHMAXPATH]="";

extern int iBus;
extern int iTarget;
extern int iLun;
extern int iSpeed;
extern int iFifo;

extern BOOL setupDone;
extern BOOL GrabberSetupDone;

char chrCDDBServer[100]="";
char chrCDDBUser[100]="";
char chrCDDBUserHost[100]="";
BOOL bUseCDDB=FALSE;
char cddbServer[MAXSERVERS][100];
int NUMSERVERS=6;/* Predefined servers */

HPOINTER hPtrTBIcons[NUMTBICONS]={0};

char chrLBFontName[CCHMAXPATH]="";/* Font for grab listbox */

/* Fly-over help */
char chrTBFlyFontName[CCHMAXPATH];/* Font for toolbar fly over help */
RGB rgbTBFlyForeground;
RGB rgbTBFlyBackground;
BOOL bTBFlyOverEnabled=TRUE;
int iTBFlyOverDelay;

BOOL bHintEnabled;

//HWND hwndBubbleWindow=NULLHANDLE;

/* For links in formated text windows */
HPOINTER hptrHand=NULLHANDLE;

extern char chrInstallDir[CCHMAXPATH];
char chrLanguageDLL[CCHMAXPATH]="";
/* The dir with the config files. Currently it's the install dir but this may change.
   The dir is defined in querymodulehandle */
char chrConfigDir[CCHMAXPATH];

/* Paths to the ISOFS mount/unmount programs */
char chrMntIsoFSPath[CCHMAXPATH];
char chrUmntIsoPath[CCHMAXPATH];


char g_chrMP3Names[NUM_MP3NAMES][MP3NAME_LEN]= {"---","Album", "Artist","Title", "Track"};/* Strings for drop down box */
char g_chrMP3NameExample[NUM_MP3NAMES][MP3NAME_LEN]= {"","Album", "Artist","Title", "01"};/* Strings for building the example */
char g_cChosenMP3Name[NUM_MP3NAMEPARTS]={4,3,1,2};/* Array holding the indexes into the names array for each MP3 name part */
char g_chrMP3NameFillStrings[NUM_MP3NAME_FILLSTRINGS][MP3NAME_FILLSTRING_LEN]= {"-","-","-"};

HMTX hmtxFileName;


/* For custom BG */
BOOL bUseCustomPainting=TRUE;

LOADEDBITMAP allBMPs[NUM_CTRL_IDX]={0};
CONTROLINFO ciControls[NUM_CTRL_IDX]={
  {0,{0,0,590,70}, NULLHANDLE,{0}},/* main */
  {1, {0, 0, 13,13},NULLHANDLE, { 151, 18, 187, 54} }, /* Placeholder for checkbutton */
  {1, {0, 0, 13,13},NULLHANDLE, { 151, 18, 187, 54} }, /* Placeholder for checkbutton */
  {1, {0, 0, 13,13},NULLHANDLE, { 151, 18, 187, 54} }, /* Placeholder for radiobutton */
  {1, {0, 0, 13,13},NULLHANDLE, { 151, 18, 187, 54} }, /* Placeholder for radiobutton */
  {1, {0, 0, 13,13},NULLHANDLE, { 151, 18, 187, 54} } /* Placeholder for checkbutton sel */
};

#ifdef DEBUG
  char debugText[200];
#endif

BOOL extern CDQueryCDDrives(int *iNumCD, char * cFirstDrive);
ULONG cwQueryOSRelease();
void errorResource2(char *chrTitle);
ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);
extern void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);
extern void SysWriteToTrapLog(const char* chrFormat, ...);

ULONG cwCreateMutex(HMTX * hmtxBMP);
ULONG cwCloseMutex(HMTX  hmtxBMP);


static void _fillMP3NameArray(void)
{

  getMessage(g_chrMP3Names[IDMP3NAMING_ALBUM], IDSTR_MP3NAMEALBUM, sizeof(g_chrMP3Names[3]), hSettingsResource, HWND_DESKTOP);
  getMessage(g_chrMP3Names[IDMP3NAMING_ARTIST], IDSTR_MP3NAMEARTIST, sizeof(g_chrMP3Names[3]), hSettingsResource, HWND_DESKTOP);
  getMessage(g_chrMP3Names[IDMP3NAMING_TITLE], IDSTR_MP3NAMETITLE, sizeof(g_chrMP3Names[3]), hSettingsResource, HWND_DESKTOP);
  getMessage(g_chrMP3Names[IDMP3NAMING_TRACK], IDSTR_MP3NAMETRACK, sizeof(g_chrMP3Names[3]), hSettingsResource, HWND_DESKTOP);
  getMessage(g_chrMP3Names[IDMP3NAMING_NONE], IDSTR_MP3NAMENONE, sizeof(g_chrMP3Names[3]), hSettingsResource, HWND_DESKTOP);

  getMessage(g_chrMP3NameExample[IDMP3NAMING_ALBUM], IDSTR_MP3NAMEALBUM, sizeof(g_chrMP3NameExample[3]), hSettingsResource, HWND_DESKTOP);
  getMessage(g_chrMP3NameExample[IDMP3NAMING_ARTIST], IDSTR_MP3NAMEARTIST, sizeof(g_chrMP3NameExample[3]), hSettingsResource, HWND_DESKTOP);
  getMessage(g_chrMP3NameExample[IDMP3NAMING_TITLE], IDSTR_MP3NAMETITLE, sizeof(g_chrMP3NameExample[3]), hSettingsResource, HWND_DESKTOP);
  strncpy(g_chrMP3NameExample[IDMP3NAMING_TRACK], "01", sizeof(g_chrMP3NameExample[3]));
  g_chrMP3NameExample[IDMP3NAMING_TRACK][MP3NAME_LEN-1]=0;
  g_chrMP3NameExample[IDMP3NAMING_NONE][0]=0;

} 

void _loadBmps()
{
  HPS hps;
  BITMAPINFOHEADER bmpInfoHdr;

  if(allBMPs[CTRLIDX_BG].hbm)
    return; /* already loaded */

  hps=WinGetPS(HWND_DESKTOP);

  /* The main BMP */
  allBMPs[CTRLIDX_BG].hbm=GpiLoadBitmap(hps, hBaseResource, IDBMP_BACKGROUND, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_BG].hbm, &allBMPs[CTRLIDX_BG].bmpInfoHdr);

  /* Checkbutton checked */
  allBMPs[CTRLIDX_CHECK].hbm=GpiLoadBitmap(hps, hBaseResource, IDBMP_CHECK, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_CHECK].hbm, &allBMPs[CTRLIDX_CHECK].bmpInfoHdr);
  ciControls[CTRLIDX_CHECK].hbmSource=allBMPs[CTRLIDX_CHECK].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_CHECK].rclSource.yBottom=0;
  ciControls[CTRLIDX_CHECK].rclSource.yTop=allBMPs[CTRLIDX_CHECK].bmpInfoHdr.cy;
  ciControls[CTRLIDX_CHECK].rclSource.xLeft=0;
  ciControls[CTRLIDX_CHECK].rclSource.xRight=allBMPs[CTRLIDX_CHECK].bmpInfoHdr.cx;

  /* Checkbutton unchecked */
  allBMPs[CTRLIDX_UNCHECK].hbm=GpiLoadBitmap(hps, hBaseResource, IDBMP_UNCHECK, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_UNCHECK].hbm, &allBMPs[CTRLIDX_UNCHECK].bmpInfoHdr);
  ciControls[CTRLIDX_UNCHECK].hbmSource=allBMPs[CTRLIDX_UNCHECK].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_UNCHECK].rclSource.yBottom=0;
  ciControls[CTRLIDX_UNCHECK].rclSource.yTop=allBMPs[CTRLIDX_UNCHECK].bmpInfoHdr.cy;
  ciControls[CTRLIDX_UNCHECK].rclSource.xLeft=0;
  ciControls[CTRLIDX_UNCHECK].rclSource.xRight=allBMPs[CTRLIDX_UNCHECK].bmpInfoHdr.cx;

  /* Checkbutton unchecked selected */
  allBMPs[CTRLIDX_UNCHECKSEL].hbm=GpiLoadBitmap(hps, hBaseResource, IDBMP_UNCHECKSEL, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_UNCHECKSEL].hbm, &allBMPs[CTRLIDX_UNCHECKSEL].bmpInfoHdr);
  ciControls[CTRLIDX_UNCHECKSEL].hbmSource=allBMPs[CTRLIDX_UNCHECKSEL].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_UNCHECKSEL].rclSource.yBottom=0;
  ciControls[CTRLIDX_UNCHECKSEL].rclSource.yTop=allBMPs[CTRLIDX_UNCHECKSEL].bmpInfoHdr.cy;
  ciControls[CTRLIDX_UNCHECKSEL].rclSource.xLeft=0;
  ciControls[CTRLIDX_UNCHECKSEL].rclSource.xRight=allBMPs[CTRLIDX_UNCHECKSEL].bmpInfoHdr.cx;

  /* Radiobutton checked */
  allBMPs[CTRLIDX_RADCHECK].hbm=GpiLoadBitmap(hps, hBaseResource, IDBMP_RADCHECK, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_RADCHECK].hbm, &allBMPs[CTRLIDX_RADCHECK].bmpInfoHdr);
  // get bitmap info
  ciControls[CTRLIDX_RADCHECK].bmpInfoHdr2.cbFix = sizeof(ciControls[CTRLIDX_RADCHECK].bmpInfoHdr2);
  GpiQueryBitmapInfoHeader(allBMPs[CTRLIDX_RADCHECK].hbm, &ciControls[CTRLIDX_RADCHECK].bmpInfoHdr2);
  ciControls[CTRLIDX_RADCHECK].hbmSource=allBMPs[CTRLIDX_RADCHECK].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_RADCHECK].rclSource.yBottom=0;
  ciControls[CTRLIDX_RADCHECK].rclSource.yTop=allBMPs[CTRLIDX_RADCHECK].bmpInfoHdr.cy;
  ciControls[CTRLIDX_RADCHECK].rclSource.xLeft=0;
  ciControls[CTRLIDX_RADCHECK].rclSource.xRight=allBMPs[CTRLIDX_RADCHECK].bmpInfoHdr.cx;

  /* Radiobutton checked selected */
  allBMPs[CTRLIDX_RADCHECKSEL].hbm=GpiLoadBitmap(hps, hBaseResource, IDBMP_RADCHECKSEL, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_RADCHECKSEL].hbm, &allBMPs[CTRLIDX_RADCHECKSEL].bmpInfoHdr);
  // get bitmap info
  ciControls[CTRLIDX_RADCHECKSEL].bmpInfoHdr2.cbFix = sizeof(ciControls[CTRLIDX_RADCHECKSEL].bmpInfoHdr2);
  GpiQueryBitmapInfoHeader(allBMPs[CTRLIDX_RADCHECKSEL].hbm, &ciControls[CTRLIDX_RADCHECKSEL].bmpInfoHdr2);
  ciControls[CTRLIDX_RADCHECKSEL].hbmSource=allBMPs[CTRLIDX_RADCHECKSEL].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_RADCHECKSEL].rclSource.yBottom=0;
  ciControls[CTRLIDX_RADCHECKSEL].rclSource.yTop=allBMPs[CTRLIDX_RADCHECKSEL].bmpInfoHdr.cy;
  ciControls[CTRLIDX_RADCHECKSEL].rclSource.xLeft=0;
  ciControls[CTRLIDX_RADCHECKSEL].rclSource.xRight=allBMPs[CTRLIDX_RADCHECKSEL].bmpInfoHdr.cx;

  /* Radiobutton unchecked */
  allBMPs[CTRLIDX_RADUNCHECK].hbm=GpiLoadBitmap(hps, hBaseResource, IDBMP_RADUNCHECK, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_RADUNCHECK].hbm, &allBMPs[CTRLIDX_RADUNCHECK].bmpInfoHdr);
  ciControls[CTRLIDX_RADUNCHECK].bmpInfoHdr2.cbFix = sizeof(ciControls[CTRLIDX_RADUNCHECK].bmpInfoHdr2);
  GpiQueryBitmapInfoHeader(allBMPs[CTRLIDX_RADUNCHECK].hbm, &ciControls[CTRLIDX_RADUNCHECK].bmpInfoHdr2);
  ciControls[CTRLIDX_RADUNCHECK].hbmSource=allBMPs[CTRLIDX_RADUNCHECK].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_RADUNCHECK].rclSource.yBottom=0;
  ciControls[CTRLIDX_RADUNCHECK].rclSource.yTop=allBMPs[CTRLIDX_RADUNCHECK].bmpInfoHdr.cy;
  ciControls[CTRLIDX_RADUNCHECK].rclSource.xLeft=0;
  ciControls[CTRLIDX_RADUNCHECK].rclSource.xRight=allBMPs[CTRLIDX_RADUNCHECK].bmpInfoHdr.cx;

  /* Radiobutton unchecked selected */
  allBMPs[CTRLIDX_RADUNCHECKSEL].hbm=GpiLoadBitmap(hps, hBaseResource, IDBMP_RADUNCHECKSEL, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_RADUNCHECKSEL].hbm, &allBMPs[CTRLIDX_RADUNCHECKSEL].bmpInfoHdr);
  ciControls[CTRLIDX_RADUNCHECKSEL].bmpInfoHdr2.cbFix = sizeof(ciControls[CTRLIDX_RADUNCHECKSEL].bmpInfoHdr2);
  GpiQueryBitmapInfoHeader(allBMPs[CTRLIDX_RADUNCHECKSEL].hbm, &ciControls[CTRLIDX_RADUNCHECKSEL].bmpInfoHdr2);
  ciControls[CTRLIDX_RADUNCHECKSEL].hbmSource=allBMPs[CTRLIDX_RADUNCHECKSEL].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_RADUNCHECKSEL].rclSource.yBottom=0;
  ciControls[CTRLIDX_RADUNCHECKSEL].rclSource.yTop=allBMPs[CTRLIDX_RADUNCHECK].bmpInfoHdr.cy;
  ciControls[CTRLIDX_RADUNCHECKSEL].rclSource.xLeft=0;
  ciControls[CTRLIDX_RADUNCHECKSEL].rclSource.xRight=allBMPs[CTRLIDX_RADUNCHECK].bmpInfoHdr.cx;

  /* Radiobutton mask */
  allBMPs[CTRLIDX_RADMASK].hbm=GpiLoadBitmap(hps, hBaseResource, IDBMP_RADMASK, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_RADMASK].hbm, &allBMPs[CTRLIDX_RADMASK].bmpInfoHdr);
  ciControls[CTRLIDX_RADMASK].bmpInfoHdr2.cbFix = sizeof(ciControls[CTRLIDX_RADMASK].bmpInfoHdr2);
  GpiQueryBitmapInfoHeader(allBMPs[CTRLIDX_RADMASK].hbm, &ciControls[CTRLIDX_RADMASK].bmpInfoHdr2);
  ciControls[CTRLIDX_RADMASK].hbmSource=allBMPs[CTRLIDX_RADMASK].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_RADMASK].rclSource.yBottom=0;
  ciControls[CTRLIDX_RADMASK].rclSource.yTop=allBMPs[CTRLIDX_RADMASK].bmpInfoHdr.cy;
  ciControls[CTRLIDX_RADMASK].rclSource.xLeft=0;
  ciControls[CTRLIDX_RADMASK].rclSource.xRight=allBMPs[CTRLIDX_RADMASK].bmpInfoHdr.cx;

  /* Radiobutton mask selected */
  allBMPs[CTRLIDX_RADMASKSEL].hbm=GpiLoadBitmap(hps, hBaseResource, IDBMP_RADMASKSEL, 0, 0);
  GpiQueryBitmapParameters(allBMPs[CTRLIDX_RADMASKSEL].hbm, &allBMPs[CTRLIDX_RADMASKSEL].bmpInfoHdr);
  ciControls[CTRLIDX_RADMASKSEL].bmpInfoHdr2.cbFix = sizeof(ciControls[CTRLIDX_RADMASKSEL].bmpInfoHdr2);
  GpiQueryBitmapInfoHeader(allBMPs[CTRLIDX_RADMASKSEL].hbm, &ciControls[CTRLIDX_RADMASKSEL].bmpInfoHdr2);
  ciControls[CTRLIDX_RADMASKSEL].hbmSource=allBMPs[CTRLIDX_RADMASKSEL].hbm;
  /* Adjust source pos */
  ciControls[CTRLIDX_RADMASKSEL].rclSource.yBottom=0;
  ciControls[CTRLIDX_RADMASKSEL].rclSource.yTop=allBMPs[CTRLIDX_RADMASKSEL].bmpInfoHdr.cy;
  ciControls[CTRLIDX_RADMASKSEL].rclSource.xLeft=0;
  ciControls[CTRLIDX_RADMASKSEL].rclSource.xRight=allBMPs[CTRLIDX_RADMASKSEL].bmpInfoHdr.cx;

  WinReleasePS(hps);
}

/* This function returns the module handle of our class-dll */
HMODULE queryModuleHandle()
{
  char *found;

  static HMODULE hmod=0;

  if(!hmod) {
    PSZ pathname=SOMClassMgrObject  //Query Pathname of class file
      ->somLocateClassFile(somIdFromString("CWCreatorSettings"),1,2);
    /* The above function sometimes returns the class name instead of the dll name.
       I suspect it's because with DTS there's no dllname modifier to tell SOM
       the name. But why does it sometimes work??
       To circumvent the problem not having a module handle we try this function for
       the other classnames again hoping that one of the calls is succesful (until now
       it is). */
#ifdef DEBUG
    sprintf(debugText,"\n_queryModuleHandle() (CWCreatorSettings) returned: %s\n",pathname);
    writeDebugLog(debugText);
#endif

    if( DosQueryModuleHandle(pathname,&hmod)){  //Query module handle
      /* Second try: CWAudioFolder */
      pathname=SOMClassMgrObject  //Query Pathname of class file
        ->somLocateClassFile(somIdFromString("CWAudioFolder"),1,2);
      if( DosQueryModuleHandle(pathname,&hmod)){  //Query module handle
        /* Last resort: CWDataFolder */
        pathname=SOMClassMgrObject  //Query Pathname of class file
          ->somLocateClassFile(somIdFromString("CWDataFolder"),1,2);
        DosQueryModuleHandle(pathname,&hmod);
      }
    }
    /* At least now we should have the handle */
    /* Get install dir. Currently it's the dir the DLL are found */
    if(!DosQueryModuleName(hmod,sizeof(chrInstallDir),chrInstallDir)) {
      if((found=strrchr(chrInstallDir,'\\'))!=NULLHANDLE) {
        *found=0;
        /* Save the dir for config files. */
        strcpy( chrConfigDir, chrInstallDir);
      }
    }
  }/* end of if(!hmod) */
  return hmod;
}

BOOL checkCpSupport(int iCp)
{
  switch(iCp)
    {
    case 437:
    case 737:
    case 775:
    case 850:
    case 852:
    case 855:
    case 857:
    case 860:
    case 861:
    case 862:
    case 863:
    case 864:
    case 865:
    case 866:
    case 869:
    case 874:
      return TRUE;
    default:
      return FALSE;
    }
  return FALSE;
}

ULONG M_CWCreatorSettings::wpclsQueryStyle()
{
	return M_WPAbstract::wpclsQueryStyle()|CLSSTYLE_NEVERTEMPLATE;
}

PSZ M_CWCreatorSettings::wpclsQueryTitle()
{
	return "CD-Creator-Settings";
}

BOOL M_CWCreatorSettings::wpclsQueryDefaultHelp(PULONG HelpPanelId,PSZ HelpLibrary)
{
	if(HelpLibrary)
      /* The name of the library is defined in audiofolderhelp.h */
		strcpy(HelpLibrary,AFHELPLIBRARY);
	if(HelpPanelId)
		*HelpPanelId=IDHLP_CREATORSETTINGSMAIN;
	
	return TRUE;
}

ULONG M_CWCreatorSettings::wpclsQueryIconData(PICONINFO pIconInfo)
{
   
	if (pIconInfo)   {
		pIconInfo->fFormat = ICON_RESOURCE;
		pIconInfo->hmod    = hBaseResource;
		pIconInfo->resid   = ID_ICONCREATORSETTINGS;
	} /* endif */

	return ( sizeof(ICONINFO) );
}

static BOOL checkForMP3IOProc( void )
{
    CHAR          szBuffer[ sizeof( FOURCC ) + CCHMAXPATH + 4 ];
    MMFORMATINFO  mmFormatInfo;
    PMMFORMATINFO pmmFormatInfoArray;
    ULONG         ulReturnCode;
    LONG          lFormatsRead;
    LONG          index;
    LONG          lBytesRead;
    LONG          lNumIOProcs;
    void*         memPtr;

    memset( &mmFormatInfo,
            '\0',
            sizeof(MMFORMATINFO) );
    
    mmFormatInfo.ulMediaType |= MMIO_MEDIATYPE_AUDIO;
    mmFormatInfo.ulFlags|=MMIO_CANREADTRANSLATED;
    ulReturnCode = mmioQueryFormatCount ( &mmFormatInfo,
                                          &lNumIOProcs,
                                          0,
                                          0 );
    
    if( ulReturnCode != MMIO_SUCCESS )
      {
        /* Error - mmioQueryFormatCount failed. */
        return FALSE;
      }


    /*
     * Allocate enough memory for n number of FormatInfo blocks
     */

    memPtr=malloc (lNumIOProcs * sizeof( MMFORMATINFO ) );
    if( memPtr == NULLHANDLE )
      {
        /* Could not allocate enough memory for mmFormatInfo array. */
        return FALSE;
      }

    pmmFormatInfoArray = (PMMFORMATINFO)memPtr;

    /*
     * call mmioGetFormats to get info on the formats supported.
     */
    ulReturnCode = mmioGetFormats( &mmFormatInfo,
                                   lNumIOProcs,
                                   pmmFormatInfoArray,
                                   &lFormatsRead,
                                   0,
                                   0 );
    if( ulReturnCode != MMIO_SUCCESS )
      {
        /*
         *  mmioGetFormats failed.
         */
        free(pmmFormatInfoArray);
        return FALSE;
      }
    
    if( lFormatsRead != lNumIOProcs )
      {
        /*
         * Error in MMIO - number of formats read in by
         * mmioGetFormats is not equal to number of formats
         * found by mmioQueryFormatCount.
         */
        free(pmmFormatInfoArray);
        return FALSE;
      }

    for ( index = 0; index <lNumIOProcs; index++ )
      {
        mmioGetFormatName(pmmFormatInfoArray, szBuffer, &lBytesRead, 0L, 0L);
        
        /* Insert NULL string terminator */
        *( szBuffer + lBytesRead ) = (CHAR)NULL;

        /* Only read translated enabled IO procs
         */
        if(pmmFormatInfoArray->ulFlags & MMIO_CANREADTRANSLATED)
          {
            if(!stricmp(szBuffer, "MP3AUDIO")) {
              /* Found MP3 IO-Proc */
              //SysWriteToTrapLog("IO-Proc %d: %s\n", index, szBuffer);
              free(memPtr);
              //          return FALSE; /* For testing... */
              return TRUE;
            }
          }
        /*
         *  advance to next entry in mmFormatInfo array
         */
        pmmFormatInfoArray++;
      }
    return FALSE;
}


void M_CWCreatorSettings::wpclsInitData()
{
  COUNTRYCODE country= {0};
  COUNTRYINFO countryInfo= {0};
  ULONG ulInfoLen=0;
  APIRET rc;
  char path[CCHMAXPATH];  
  char* found;
  char buf[CCHMAXPATH];
  HINI hini=0;
  ULONG keyLength;
  ULONG ulSize;
  ULONG ulCp[4];
  int a;

  /* Call parent */
  M_WPAbstract::wpclsInitData();

  /* clear global data */
  memset(&globalData, 0, sizeof(GLOBALDATA));
  cwCreateMutex(&hmtxFileName);
  
  CDQueryCDDrives(&iNumCD,&cFirstCD);

  if(DosQueryCp(sizeof(ulCp),ulCp,&ulInfoLen)!=NO_ERROR) {
    iCodePage=0;
  }
  else{
    /* Check if mkisofs supports our CP */
   if(checkCpSupport((int)ulCp[0]))
     iCodePage=(int)ulCp[0];
    else
      iCodePage=0;
  };

  /* Get the handle of our class dll. This dll contains the graphics and icons.
     A language specific dll conatins all the NLS stuff.  */
  hBaseResource=queryModuleHandle();
  /* Get the country code of our system and load the  
     resource DLL with the right language */
  do {
    rc=DosQueryCtryInfo(sizeof(countryInfo),&country,&countryInfo,&ulInfoLen);
    if(rc!=NO_ERROR)
      break;
    
    rc=DosQueryModuleName(queryModuleHandle(),sizeof(path),path);
    if(rc!=NO_ERROR)
      break;

    found=strrchr(path,'\\');
    if(!found)
      break;

    *found=0;
    sprintf(buf,"\\cdfld%03d.dll",countryInfo.country);
    strcat(path,buf);

    rc=DosLoadModule(buf,sizeof(buf),path, &hSettingsResource);
    if(rc==NO_ERROR) {
      strcpy(chrLanguageDLL, path);
      break;
    }
    
    /* NLS DLL not found. Try to load default */
    found=strrchr(path,'\\');
    if(!found)
      break;
    
    *found=0;
    sprintf(buf,"\\cdfld001.dll");
    strcat(path,buf);

    rc=DosLoadModule(buf,sizeof(buf),path, &hSettingsResource);

    if(rc) {
      //writeLog2(chrInstallDir, CREATOR_LOGFILE, "M_CWCreatorSettings::wpclsInitData(): Can't get resource-DLL!\n");
    }
    else {
      for(a=0;a<NUMTBICONS;a++) {
        hPtrTBIcons[a]=WinLoadPointer(HWND_DESKTOP, hSettingsResource, ID_TBICONFIRST+a);
      }
      //  writeLog2(chrInstallDir, CREATOR_LOGFILE, "M_CWCreatorSettings::wpclsInitData(): Got resource handle.\n");
      strcpy(chrLanguageDLL, path);
    }
    break;
  }while(TRUE);
  

  /* Load ini values */
  /* Build full path for cdrecord.ini file */
  sprintf(path,"%s\\cdrecord.ini", chrConfigDir);       
  /* Open ini-file */
  hini=PrfOpenProfile(WinQueryAnchorBlock(HWND_DESKTOP),path);
  do{
    if(!hini) {
      /* path: "Warning! Cannot open Ini-file!"
         buf: "Audio-CD-Creator"
         */
      messageBox( path, IDSTR_INIFILEOPENWARNING , sizeof(path),
                  buf, IDSTR_AUDIOCDCREATOR, sizeof(buf),
                  hSettingsResource, HWND_DESKTOP, MB_OK | MB_ERROR | MB_MOVEABLE);
      break;
    }/* end of if(!hini) */
    
    keyLength=PrfQueryProfileString(hini,"CDWriter","cdrecord","",chrCDRecord,sizeof(chrCDRecord));
    if(keyLength!=1){
      PrfQueryProfileString(hini,"CDWriter","audiocdroptions","",chrAudioCDROptions,sizeof(chrAudioCDROptions));
      PrfQueryProfileString(hini,"CDWriter","datacdroptions","",chrDataCDROptions,sizeof(chrDataCDROptions));    
      ulSize=sizeof(lCDROptions);
      PrfQueryProfileData(hini,"CDWriter","options",&lCDROptions,&ulSize);
      PrfQueryProfileString(hini,"CDWriter","writedrive","",chosenWriter,sizeof(chosenWriter));
      setupDone=TRUE;
    }
    else {
      /* Default is close helper windows, start hidden, create shadows and eject. */
      lCDROptions=(IDCDR_HIDEWINDOW | IDCDR_CLOSEWINDOW | IDCDR_CREATESHADOWS);
    }
    
    /* Grabber options */
    keyLength=PrfQueryProfileString(hini,"CDGrabber","grabber","", globalData.chrGrabberPath,
                                    sizeof(globalData.chrGrabberPath));
    if(keyLength!=1){
      /* We have got some entries */    
      PrfQueryProfileString(hini,"CDGrabber","graboptions", "", globalData.chrGrabberOptions,
                            sizeof(globalData.chrGrabberOptions));
      PrfQueryProfileString(hini,"CDGrabber","grabdrive", "" , chosenCD,sizeof(chosenCD));
      PrfQueryProfileString(hini,"CDGrabber","grabbername", "", globalData.chrGrabberName,
                            sizeof(globalData.chrGrabberName));
      globalData.bTrackNumbers=PrfQueryProfileInt(hini,"CDGrabber","tracknumbers",1);
      globalData.iGrabberID=PrfQueryProfileInt(hini,"CDGrabber","ID", IDGRABBER_UNKNOWN);/* 99: unknown */
      GrabberSetupDone=TRUE;
    }

    /* DVD options */
    keyLength=PrfQueryProfileString(hini,"dvdwriting","writerpath","",
                                    chrDVDDao, sizeof(chrDVDDao));
    keyLength=PrfQueryProfileString(hini,"dvdwriting","options","",
                                    chrDVDDaoOptions, sizeof(chrDVDDaoOptions));

    /* MP3 decoder options */      
    keyLength=PrfQueryProfileString(hini,"mpg123","path","",
                                    globalData.chrMpg123Path,sizeof(globalData.chrMpg123Path));
    globalData.bMpg123SwabBytes=PrfQueryProfileInt(hini,"mpg123","swabbytes",1); /* Swab Bytes? */
    if(checkForMP3IOProc()) {
      globalData.ulGlobalFlags|=GLOBALFLAG_MMIOMP3INSTALLED;
      iMp3Decoder=PrfQueryProfileInt(hini,"mpg123","decoder",IDKEY_USEMMIOMP3);/* Which decoder to use */
    }
    else {
      iMp3Decoder=PrfQueryProfileInt(hini,"mpg123","decoder",IDKEY_USEMPG123);/* Which decoder to use */
      /* Check if MMIOMP3 is selected. If yes, reset it to mpg123 */
      if(iMp3Decoder==IDKEY_USEMMIOMP3)
        iMp3Decoder=IDKEY_USEMPG123;
    }
    /* MP3 encoder settings */
    keyLength=PrfQueryProfileString(hini,"mp3encode","encoderpath","",
                                    chrMP3EncoderPath, sizeof(chrMP3EncoderPath));
    keyLength=PrfQueryProfileString(hini,"mp3encode","options","",
                                    chrMP3EncoderOptions, sizeof(chrMP3EncoderOptions));
    ulMP3Quality=PrfQueryProfileInt(hini,"mp3encode","quality",IDQUALITY_VBRSTANDARD);
    ulMP3Bitrate=PrfQueryProfileInt(hini,"mp3encode","bitrate", DEFAULT_MP3_BITRATE);
    ulMP3EncoderPriority=PrfQueryProfileInt(hini,"mp3encode","priority", DEFAULT_MP3_ENCODEPRIORITY);

    keyLength=PrfQueryProfileString(hini,"mp3encode","librarypath","",
                                    globalData.chrMP3LibraryPath, sizeof(globalData.chrMP3LibraryPath));


    keyLength=sizeof(g_cChosenMP3Name);
    if(!PrfQueryProfileData(hini, "mp3naming", "order", g_cChosenMP3Name, &keyLength)||keyLength!=sizeof(g_cChosenMP3Name))
      {
        g_cChosenMP3Name[0]=4;
        g_cChosenMP3Name[1]=3;
        g_cChosenMP3Name[2]=1;
        g_cChosenMP3Name[3]=2;
      }
    keyLength=sizeof(g_chrMP3NameFillStrings);
    if(!PrfQueryProfileData(hini, "mp3naming", "fillstrings", g_chrMP3NameFillStrings, &keyLength) ||
       keyLength!=sizeof(g_chrMP3NameFillStrings))
      {
        strcpy(g_chrMP3NameFillStrings[0],"-");
        strcpy(g_chrMP3NameFillStrings[1],"-");
        strcpy(g_chrMP3NameFillStrings[2],"-");
      }
    
    /* Options for audio cdrdao */
    keyLength=PrfQueryProfileString(hini,"cdrdao","path","",chrCdrdaoPath,sizeof(chrCdrdaoPath));
    if(keyLength!=1){
      /* We have got entries yet */
    }
    keyLength=PrfQueryProfileString(hini,"cdrdao","driver","",chrCdrdaoDriver,sizeof(chrCdrdaoDriver));
    if(keyLength!=1){
      /* We have got entries yet */
    }
    keyLength=PrfQueryProfileString(hini,"cdrdao","writer","",chrWriterName,sizeof(chrWriterName));
    if(keyLength!=1){
      /* We have got entries yet */
    }
    keyLength=PrfQueryProfileString(hini,"cdrdao","options","",chrCdrdaoOptions,sizeof(chrCdrdaoOptions));
    if(keyLength!=1){
      /* We have got entries yet */
    }

    /* bus, target, lun... */
    iBus=PrfQueryProfileInt(hini,"device","bus",0);
    iTarget=PrfQueryProfileInt(hini,"device","target",0);
    iLun=PrfQueryProfileInt(hini,"device","lun",0);
    iSpeed=PrfQueryProfileInt(hini,"device","speed",1);
    iFifo=PrfQueryProfileInt(hini,"device","fifo",4);    
    keyLength=PrfQueryProfileString(hini,"device","devicename","", chrDeviceName, sizeof(chrDeviceName));
    
    /* Options for cdrdao 1:1 copy source */
    keyLength=PrfQueryProfileString(hini,"cdrdaosrc","path", chrCdrdaoPath , globalData.chrCdrdaoPath2,
                                    sizeof(globalData.chrCdrdaoPath2));
    keyLength=PrfQueryProfileString(hini,"cdrdaosrc","driver", chrCdrdaoDriver, globalData.chrCdrdaoDriver2,
                                    sizeof(globalData.chrCdrdaoDriver2));
    keyLength=PrfQueryProfileString(hini,"cdrdaosrc","writer", chrWriterName, globalData.chrWriterName2,
                                    sizeof(globalData.chrWriterName2));
    keyLength=PrfQueryProfileString(hini,"cdrdaosrc","options", chrCdrdaoOptions, globalData.chrCdrdaoOptions2,
                                    sizeof(globalData.chrCdrdaoOptions2));
    keyLength=PrfQueryProfileString(hini,"cdrdaosrc","devicename", chrDeviceName, globalData.chrDeviceName2,
                                    sizeof(globalData.chrDeviceName2));

    /* Options for cdrdao 1:1 copy target */
    keyLength=PrfQueryProfileString(hini,"cdrdaotarget","path", globalData.chrCdrdaoPath2 , globalData.chrCdrdaoPath3,
                                    sizeof(globalData.chrCdrdaoPath3));
    keyLength=PrfQueryProfileString(hini,"cdrdaotarget","driver", globalData.chrCdrdaoDriver2, globalData.chrCdrdaoDriver3,
                                    sizeof(globalData.chrCdrdaoDriver3));
    keyLength=PrfQueryProfileString(hini,"cdrdaotarget","writer", globalData.chrWriterName2, globalData.chrWriterName3,
                                    sizeof(globalData.chrWriterName3));
    keyLength=PrfQueryProfileString(hini,"cdrdaotarget","options", globalData.chrCdrdaoOptions2, globalData.chrCdrdaoOptions3,
                                    sizeof(globalData.chrCdrdaoOptions3));
    keyLength=PrfQueryProfileString(hini,"cdrdaotarget","devicename", globalData.chrDeviceName2, globalData.chrDeviceName3,
                                    sizeof(globalData.chrDeviceName3));

    /* Mkisofs options */
    keyLength=PrfQueryProfileString(hini,"Mkisofs","mkisofs","",chrMkisofs,sizeof(chrMkisofs));
    if(keyLength!=1){
      PrfQueryProfileString(hini,"Mkisofs","mkisofsoptions","",chrMkisofsOptions,sizeof(chrMkisofsOptions));
      ulSize=sizeof(lMKOptions);
      PrfQueryProfileData(hini,"Mkisofs","options",&lMKOptions,&ulSize);
      bDisableCp=PrfQueryProfileInt(hini,"Mkisofs","disablecp",0);
      /* Last image file path */
      PrfQueryProfileString(hini,"Mkisofs","imagepath","",chrImage,sizeof(chrImage));
      MkisofsSetupDone=TRUE;
    }

    PrfQueryProfileString(hini,"isofs","mountpath","",chrMntIsoFSPath,sizeof(chrMntIsoFSPath));
    PrfQueryProfileString(hini,"isofs","unmountpath","",chrUmntIsoPath, sizeof(chrUmntIsoPath));
    
    /* Presparameters */
    /* Font for track listbox in grab frame control */
    keyLength=PrfQueryProfileString(hini,"font","grablistbox","",chrLBFontName,sizeof(chrLBFontName));

    /* Font for toolbar flyover help */
    if(cwQueryOSRelease()>=40)
      keyLength=PrfQueryProfileString(hini,"toolbar","flyoverfont",DEFAULT_DIALOG_FONT,
                                      chrTBFlyFontName,sizeof(chrTBFlyFontName));
    else
      keyLength=PrfQueryProfileString(hini,"toolbar","flyoverfont",DEFAULT_DIALOG_FONT_WARP3,
                                      chrTBFlyFontName,sizeof(chrTBFlyFontName));

    /* Colors for toolbar flyover help */
    ulSize=sizeof(rgbTBFlyBackground);
    if(!PrfQueryProfileData(hini,"toolbar","flyoverbackcolor",(PBYTE)&rgbTBFlyBackground,&ulSize)) {
      rgbTBFlyBackground.bBlue=180;
      rgbTBFlyBackground.bGreen=255;
      rgbTBFlyBackground.bRed=255;
    }
    ulSize=sizeof(rgbTBFlyForeground);
    if(!PrfQueryProfileData(hini,"toolbar","flyoverforecolor",(PBYTE)&rgbTBFlyForeground,&ulSize)) {
      rgbTBFlyForeground.bRed=0;
      rgbTBFlyForeground.bGreen=0;
      rgbTBFlyForeground.bBlue=0;
    }
    /* Toolbar flyover enable */
    bTBFlyOverEnabled=PrfQueryProfileInt(hini,"toolbar","flyover",1);
    /* Toolbar flyover delay */
    iTBFlyOverDelay=PrfQueryProfileInt(hini,"toolbar","flyoverdelay", DEFAULTDELAY);

    /* Expert mofe */
    globalData.ulGlobalFlags|=(PrfQueryProfileInt(hini,"creator","ExpertMode", 0) ? GLOBALFLAG_EXPERTMODE : 0);

    /* Hints and user interface */
    bHintEnabled=PrfQueryProfileInt(hini,"hints","enable",1);
    bUseCustomPainting=PrfQueryProfileInt(hini,"newlook","enable",1);
    globalData.bTipsEnabled=PrfQueryProfileInt(hini,"tips","enable",1);

    /* CDDB */
    keyLength=PrfQueryProfileString(hini,"cddb","cddbserver","",chrCDDBServer,sizeof(chrCDDBServer));
    keyLength=PrfQueryProfileString(hini,"cddb","cddbuser","",chrCDDBUser,sizeof(chrCDDBUser));
    keyLength=PrfQueryProfileString(hini,"cddb","cddbuserhost","",chrCDDBUserHost,sizeof(chrCDDBUserHost));
    bUseCDDB=PrfQueryProfileInt(hini,"cddb","usecddb",0);
    NUMSERVERS=PrfQueryProfileInt(hini,"cddb","numservers",0);
    if(NUMSERVERS==0) {
      strncpy(cddbServer[0],"de.freedb.org:888",sizeof(cddbServer[0]));
      strncpy(cddbServer[1],"at.freedb.org:888",sizeof(cddbServer[0]));
      strncpy(cddbServer[2],"ca.freedb.org:888",sizeof(cddbServer[0]));
      strncpy(cddbServer[3],"uk.freedb.org:888",sizeof(cddbServer[0]));
      strncpy(cddbServer[4],"freedb.freedb.org:888",sizeof(cddbServer[0]));
      strncpy(cddbServer[5],"cz.freedb.org:888",sizeof(cddbServer[0]));
      NUMSERVERS=6;
    }
    else {
      if(NUMSERVERS>MAXSERVERS) 
        NUMSERVERS=MAXSERVERS;
      /* Read the servers from ini */
      for(a=0;a<NUMSERVERS;a++) {
        sprintf(buf,"cddbserver%d",a+1);
        PrfQueryProfileString(hini,"cddb",buf,"",cddbServer[a],sizeof(cddbServer[0]));
      }
    }
    /* Load hand ptr for html links */
    if(!hptrHand)
        hptrHand=WinLoadPointer(HWND_DESKTOP, hBaseResource,IDPT_HAND);

    break;
  } while(TRUE);
  if(hini)PrfCloseProfile(hini);

  /* Read in the custom backgrounds if not already done */
  _loadBmps();

  _fillMP3NameArray();

#ifdef DEBUG
    sprintf(debugText,"hSettingsResource: %x\n",hSettingsResource);
    writeDebugLog(debugText);
    writeDebugLog("Leaving M_CWDataFolder::wpclsInitData()\n\n");
#endif
  
}

void M_CWCreatorSettings::wpclsUnInitData()
{
  char profileName[CCHMAXPATH];
  char moduleName[CCHMAXPATH];
  char *chrPtr;
  char chrDev[10];
  HINI hini=0;
  BOOL bError=TRUE;
  int a;

  /* Save Settings to ini */

  /* Build the ini-file name */
  /* Build full path */
  sprintf(profileName,"%s\\cdrecord.ini",chrConfigDir);       

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
  for(a=0;a<NUMTBICONS;a++) {
    if(hPtrTBIcons[a])
      WinDestroyPointer(hPtrTBIcons[a]);
  }
  //  removeLog2( chrInstallDir, CREATOR_LOGFILE);
  DosFreeModule(hSettingsResource);
  cwCloseMutex( hmtxFileName);
  M_WPAbstract::wpclsUnInitData();
}






