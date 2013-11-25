/*
 * This file is (C) Chris Wohlgemuth 1999-2001
 * 
 * It's part of the Audio/Data-CD-Creator distribution
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

#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>

#include "audiofolder.hh"
#include "audiofolderhelp.h"
#include "datafolder.h"

//#define DEBUG
 
extern PVOID pvSharedMem;
extern PVOID pvScanbusSharedMem;

extern char chrImage[CCHMAXPATH];/* Path to iso-image */
extern char chrCDRecord[CCHMAXPATH];/* Path to cdrecord */
extern char chrDataCDROptions[CCHMAXPATH];
extern char chrAudioCDROptions[CCHMAXPATH];
extern char chrGrabberPath[CCHMAXPATH];
extern char chrGrabberOptions[CCHMAXPATH];
extern LONG lCDROptions;
extern char chrInstallDir[CCHMAXPATH];
extern LONG lMKOptions;
extern int iCodePage;
extern BOOL bDisableCp;
extern BOOL bMultiSessionDone;
extern char chosenMultiSessionCD[3];
extern char chrMkisofs[CCHMAXPATH];/* Path to mkisofs */
extern char chrMkisofsOptions[CCHMAXPATH];
extern char chrCdrdaoPath[CCHMAXPATH];
extern char chrCdrdaoDriver[100];
extern char chrCdrdaoOptions[CCHMAXPATH];

extern HMODULE hAudioResource;
extern HMODULE hDataResource;
extern HMODULE hSettingsResource;

extern int iBus;
extern int iTarget;
extern int iLun;
extern int iSpeed;
extern int iFifo;

extern char chosenCD[3];
extern int bTrackNumbers;

extern HMTX hmtxFileName;

/* extern */
BOOL checkBootImage(char * chrName);
BOOL checkFileExists(char* chrFileName);
ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags);

void changeBackslash(char* text);
void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd);


//PSZ buildWrapName(PSZ wrapperName);
PSZ buildWriteParam(CWAudioFolder* thisPtr, PSZ trackname);
ULONG launchWriter(PSZ parameter, PSZ folderPath,HWND hwnd, PSZ wrapperExe,  PSZ pszTitle);

ULONG cwRequestMutex(HMTX  hmtxBMP, ULONG ulTimeOut);
ULONG cwReleaseMutex(HMTX  hmtxBMP);

/* Local */
ULONG launchWrapper(PSZ , PSZ ,HWND , PSZ , PSZ );

#ifdef DEBUG
void debugLog(char * text,int depth) {
  FILE * file;
  char filename[CCHMAXPATH];
  int a;

  strcpy(filename,chrInstallDir);
  strcat(filename,"\\");
  strcat(filename, "debug.txt");
  file=fopen(filename,"a");
  
  if(!file)
    return;

  for(a=0;a<depth;a++) {
    fwrite(" ", sizeof(char), strlen(" "),file);
  }
  fwrite(text, sizeof(char), strlen(text),file);

  fclose(file);
}
#endif

BOOL startScanbusHelper(HWND hwnd) 
{
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];

  if(checkFileExists(chrCDRecord)) {
    /* Allocate shared mem for communication between helper and settings notebook */
    if(!pvScanbusSharedMem) {
      if(DosAllocSharedMem(&pvScanbusSharedMem,SCANBUSSHAREDMEM_NAME, SCANBUSSHAREDMEM_SIZE,
                           PAG_READ|PAG_WRITE|PAG_COMMIT)) {
        /*
          Text:   "Can't allocate shared memory. There's probably already a creation process running."
          Title: "Audio-CD-Creator"                       
          */             
        messageBox( text, IDSTR_ALLOCSHAREDMEMERROR , sizeof(text),
                    title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                    hSettingsResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
        return FALSE;
      }
    }
    else
      {
        /* The shared mem is already allocated. There's another write process running. */
        /*
          Text:   "Can't allocate shared memory. There's probably already a creation process running."
          Title: "Audio-CD-Creator"                       
          */             
        messageBox( text, IDSTR_ALLOCSHAREDMEMERROR , sizeof(text),
                    title, IDSTR_AUDIOCDCREATOR , sizeof(title),
                    hSettingsResource, hwnd, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
        return FALSE;
      }
    if(launchWrapper(chrInstallDir,"",hwnd,"scanbus.exe","Scanning SCSI bus...")) {
      if(pvScanbusSharedMem) {
        DosFreeMem(pvScanbusSharedMem);
        pvScanbusSharedMem=NULL;
      }
      return FALSE;
    }/* launchwrapper() */
  }
  else {
    /* The path to CDRecord/2 isn't valid */
    /* title: "...%s..." */
    getMessage(title, IDSTR_NOVALIDPATH,sizeof(title), hSettingsResource, hwnd);            
    sprintf(text, title, "CDRecord/2");
    /* title: "Audio-CD-Creator" */
    getMessage(title, IDSTR_AUDIOCDCREATOR,sizeof(title), hSettingsResource, hwnd);                     
    WinMessageBox(  HWND_DESKTOP, hwnd,
                    text,
                    title,
                    0UL, MB_OK | MB_ERROR |MB_MOVEABLE);
    return FALSE;
  }
  return TRUE;
}

void writeLogPrintf(char* logFile, char* format, ...);

#if 0
PSZ buildWrapName(PSZ wrapperName)
{
  char exename[CCHMAXPATH*2];

  /* Build full path */
  //  writeLogPrintf("trap.log","%s %s\n", chrInstallDir, wrapperName);
  sprintf(exename,"%s\\bin\\%s",chrInstallDir, wrapperName);
  return exename;
}
#endif

/* This function builds the audio params for CDRecord/2 */
PSZ buildAudioWriteParam(CWAudioFolder* thisPtr, PSZ pszDevice, int iSpeedLocal)
{	
  char chrParams[CCHMAXPATH*2];
  char *chrPtr;
  ULONG ulWriteFlagsLocal;

  if(somIsObj(thisPtr))
    ulWriteFlagsLocal=thisPtr->cwQueryWriteFlags();

  /* Put dev=.., speed and fs= into the cmd line */
  if((chrPtr=strchr(pszDevice,' '))==NULL)
    sprintf(chrParams,"dev=%d,%d,%d speed=%d  -audio fs=%dm %s ", iBus, iTarget, iLun, iSpeedLocal, iFifo, chrAudioCDROptions);
  else {
    *chrPtr=0;
    sprintf(chrParams,"dev=%s speed=%d  -audio fs=%dm %s ", pszDevice, iSpeedLocal, iFifo, chrAudioCDROptions);
    *chrPtr=' ';
  }

  chrPtr=chrParams;

  if(!(lCDROptions & IDCDR_NOEJECT))
    strcat(chrPtr,"-eject ");

  if((lCDROptions & IDCDR_BURNPROOF))
    strcat(chrPtr,"driveropts=burnproof ");

  if(ulWriteFlagsLocal & IDWF_DUMMY)
    chrPtr=strcat(chrPtr,"-dummy ");

  /* Make -pad the default for audio tracks */
  strcat(chrPtr,"-pad ");
                      
  /* if(thisPtr->bWrite+1 == thisPtr->aWrite) {*/
  if(ulWriteFlagsLocal & IDWF_NOFIX)
    strcat(chrPtr,"-nofix ");        
  
  if(ulWriteFlagsLocal & IDWF_PREEMP)
    strcat(chrPtr,"-preemp ");

  /* make sure we have a -v so write status works */
  if(!strstr(chrPtr,"-v"))
    strcat(chrPtr,"-v ");

  return chrParams;
}

/*                                                    */
/* This function builds the audio params for cdrdao/2 */
/*                                                    */
PSZ buildAudioWriteParamDAO(CWAudioFolder* thisPtr, PSZ pszDevice, int iSpeedLocal)
{	
  char chrParams[CCHMAXPATH*2];
  char *chrPtr;
  ULONG ulWriteFlagsLocal;
  int buffers;  
  char eject[10];

  if(somIsObj(thisPtr))
    ulWriteFlagsLocal=thisPtr->cwQueryWriteFlags();
  
  if(!(lCDROptions & IDCDR_NOEJECT))
    strcpy(eject,"--eject ");
  else
    strcpy(eject,"");

  buffers=iFifo*1024/176;

  /* Simulate or real write */
  if(ulWriteFlagsLocal & IDWF_DUMMY) {
    if((chrPtr=strchr(pszDevice,' '))==NULL)
      sprintf(chrParams,"%s| simulate --device %d,%d,%d --driver %s --speed %d --buffers %d %s %s", chrCdrdaoPath ,
              iBus, iTarget, iLun, chrCdrdaoDriver,iSpeedLocal, buffers, eject, chrCdrdaoOptions);/* Add '|' to mark end of path for helper */
    else{
      *chrPtr=0;
      sprintf(chrParams,"%s| simulate --device %s --driver %s --speed %d --buffers %d %s %s", chrCdrdaoPath ,
              pszDevice, chrCdrdaoDriver,iSpeedLocal, buffers, eject, chrCdrdaoOptions);/* Add '|' to mark end of path for helper */
      *chrPtr=' ';
    }   
  }
  else {
    if((chrPtr=strchr(pszDevice,' '))==NULL)
      sprintf(chrParams,"%s| write --device %d,%d,%d --driver %s --speed %d --buffers %d %s %s", chrCdrdaoPath,
              iBus,iTarget,iLun,chrCdrdaoDriver,iSpeedLocal, buffers, eject, chrCdrdaoOptions);/* Add '|' to mark end of path for helper */
    else {
      *chrPtr=0;
      sprintf(chrParams,"%s| write --device %s --driver %s --speed %d --buffers %d %s %s", chrCdrdaoPath ,
              pszDevice, chrCdrdaoDriver,iSpeedLocal, buffers, eject, chrCdrdaoOptions);/* Add '|' to mark end of path for helper */
      *chrPtr=' ';
    }
  }
  return chrParams;
}

/* This function builds the data parameters for cdrecord according to the settings */ 
void buildDataWriteParam(CWDataFolder* thisPtr, char * text ,int iSpeedChosen, char *chrDev)
{	

  ULONG ulWriteFlagsLocal;

  if(somIsObj(thisPtr))
    ulWriteFlagsLocal=thisPtr->cwQueryCDTypeFlags();
  /* else {
     WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, 
     "_buildMkisofsCommonParam(): The object pointer isn't valid. This is a bug, please contact the author.",
     "Error in _buildMkisofsCommonParam()",
     0UL, MB_OK | MB_ERROR|MB_MOVEABLE );
     return;
     } */

  /* Build cdrecord/2 cmd-line parameters */
  sprintf(text,"dev=%s speed=%d fs=%dm %s", chrDev, iSpeedChosen, iFifo, chrDataCDROptions);

  if(!(lCDROptions & IDCDR_NOEJECT))
    strcat(text," -eject");

  if((lCDROptions & IDCDR_BURNPROOF))
    strcat(text," driveropts=burnproof");
  
  /* Check if we write a multisession CD */
  if( ulWriteFlagsLocal & IDCDT_MULTISESSION) {
    strcat(text," -multi");//multisessionswitch   
    if(lCDROptions & IDCDR_SONYMULTISESSION)
      strcat(text," -data");//additional switch necessary for some SONY drives
  }
  else 
    if( ulWriteFlagsLocal & IDCDT_USERDEFINED){
      /* We write a singlesession disk so give the right track type */
      if(ulWriteFlagsLocal & IDCDT_TRACKDATA)
        strcat(text," -data");
      else if(ulWriteFlagsLocal & IDCDT_TRACKMODE2)
        strcat(text," -mode2");
      else if(ulWriteFlagsLocal & IDCDT_TRACKXA1)
        strcat(text," -xa1");
      else if(ulWriteFlagsLocal & IDCDT_TRACKXA2)
        strcat(text," -xa2");
      else if(ulWriteFlagsLocal & IDCDT_TRACKCDI)
        strcat(text," -cdi");
      if(!(ulWriteFlagsLocal & IDCDT_FIXDISK))
        strcat(text," -nofix");    
    }
    else // Singlesession
      strcat(text," -data");

  /* make sure we have a -v so write status works */
  if(!strstr(text,"-v"))
    strcat(text," -v");

  return;
}

/* This function builds the data parameters for dvdao according to the settings */ 
void buildDataDVDWriteParam(CWDataFolder* thisPtr, char * text ,int iSpeedChosen, char *chrDev)
{	

  ULONG ulWriteFlagsLocal;

#if 0
  if(somIsObj(thisPtr))
    ulWriteFlagsLocal=thisPtr->cwQueryCDTypeFlags();
  /* else {
     WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, 
     "_buildMkisofsCommonParam(): The object pointer isn't valid. This is a bug, please contact the author.",
     "Error in _buildMkisofsCommonParam()",
     0UL, MB_OK | MB_ERROR|MB_MOVEABLE );
     return;
     } */
#endif

  /* Build cmd-line parameters */
  sprintf(text,"--device %s --speed %d -b %d --priority 3,31 ", chrDev, iSpeedChosen, iFifo*4);

  if(!(lCDROptions & IDCDR_NOEJECT))
    strcat(text," --eject");
  /* Change priority */
  //  strcat(text," --eject");
#if 0
  if((lCDROptions & IDCDR_BURNPROOF))
    strcat(text," driveropts=burnproof");
  
  /* Check if we write a multisession CD */
  if( ulWriteFlagsLocal & IDCDT_MULTISESSION) {
    strcat(text," -multi");//multisessionswitch   
    if(lCDROptions & IDCDR_SONYMULTISESSION)
      strcat(text," -data");//additional switch necessary for some SONY drives
  }
  else 
    if( ulWriteFlagsLocal & IDCDT_USERDEFINED){
      /* We write a singlesession disk so give the right track type */
      if(ulWriteFlagsLocal & IDCDT_TRACKDATA)
        strcat(text," -data");
      else if(ulWriteFlagsLocal & IDCDT_TRACKMODE2)
        strcat(text," -mode2");
      else if(ulWriteFlagsLocal & IDCDT_TRACKXA1)
        strcat(text," -xa1");
      else if(ulWriteFlagsLocal & IDCDT_TRACKXA2)
        strcat(text," -xa2");
      else if(ulWriteFlagsLocal & IDCDT_TRACKCDI)
        strcat(text," -cdi");
      if(!(ulWriteFlagsLocal & IDCDT_FIXDISK))
        strcat(text," -nofix");    
    }
    else // Singlesession
      strcat(text," -data");

  /* make sure we have a -v so write status works */
  if(!strstr(text,"-v"))
    strcat(text," -v");
#endif
  return;
}

static BOOL _buildMkisofsCommonParam(CWDataFolder* thisPtr, char * text, char * outputName, char *chrChosenDev)
{	
  ULONG ulFlags;
  char filename[CCHMAXPATH];
  char chrDev[20];
  ULONG ulSize;

  LONG lValue1,lValue2;
  char text2[CCHMAXPATH];
  char title[CCHMAXPATH];

  if(!somIsObj(thisPtr)) {
    WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, "_buildMkisofsCommonParam(): The object pointer isn't valid. This is a bug, please contact the author.",
                    "Error in _buildMkisofsCommonParam()",
                    0UL, MB_OK | MB_ERROR|MB_MOVEABLE );
    return FALSE;/* There's an error */
  }

  /* Build cmd-line for mkisofs */
  ulFlags=thisPtr->cwQueryMkisofsFlags();
  /* Path to mkisofs */
  strcat(text,chrMkisofs);
  /* Mark end of mkisofs path */
  strcat(text,"\"");

  /* Additional options specified by the user on the settings page */
  strncat(text,chrMkisofsOptions,sizeof(chrMkisofsOptions));

  /* Filename options */
  if(ulFlags & IDMK_ALLOW32CHARS)
    strncat(text," -l",4);

  if(ulFlags & IDMK_JOLIET) {
    strncat(text," -J",4);
    /* Set codepage for unicode translation */
    if(iCodePage!=0 && !bDisableCp) {
      sprintf(text2," -jcharset cp%d",iCodePage);
      strcat(text,text2);
    }
  }

  /* Allow deep directories as default */
  strcat(text," -D");

  /* We always create additional rockridge extensions */ 
  strncat(text," -r",4);
  if(ulFlags & IDMK_TRANSTABLE) {
    /* We create TRANS.TBL files but hide them from the Joliet-tree */
    strncat(text," -T",4);
    strcat(text," -hide-joliet-trans-tbl");
  }

  /* Add the autor-stuff */
  if(strlen(thisPtr->chrApplication)) {
    strncat(text," \"-A",strlen(" \"-A"));
    strncat(text,thisPtr->chrApplication,strlen(thisPtr->chrApplication));
    strncat(text,"\"",strlen("\""));
  }
  if(strlen(thisPtr->chrPublisher)) {
    strncat(text," \"-P",strlen(" \"-P"));
    strncat(text,thisPtr->chrPublisher,strlen(thisPtr->chrPublisher));
    strncat(text,"\"",strlen("\""));
  }
  if(strlen(thisPtr->chrPreparer)) {
    strncat(text," \"-p",strlen(" \"-p"));
    strncat(text,thisPtr->chrPreparer,strlen(thisPtr->chrPreparer));
    strncat(text,"\"",strlen("\""));
  }
  if(strlen(thisPtr->chrVolumeName)) {
    strncat(text," \"-V",strlen(" \"-V"));
    strncat(text,thisPtr->chrVolumeName,strlen(thisPtr->chrVolumeName));
    strncat(text,"\"",strlen("\""));
  }

  ulFlags=thisPtr->cwQueryCDTypeFlags();
  /* Boot CD stuff */
  if(ulFlags & IDCDT_BOOTCD) {
    /* Check boot image */
    ulSize=sizeof(filename);
    /* Query name of this folder */
    if(!thisPtr->wpQueryRealName(filename,&ulSize,TRUE)) {
      sprintf(text,"Error while quering the foldername!");  
      WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, text,"Image file creation",
                      0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
      return FALSE;/* There's an error */
    }
    getMessage(text2,IDSTRD_BOOTIMAGENAME,sizeof(text2), hDataResource,HWND_DESKTOP);
    sprintf(title,"");
    strncat(title,filename,sizeof(title)-1);
    strncat(title,"\\",sizeof("\\")-strlen(title)-1);
    strncat(title,text2,sizeof(title)-strlen(title)-1);
    if(!checkBootImage(title))
      {
        /* No boot image in the folder or wrong size */
        messageBox( text2, IDSTRD_BOOTIMAGEERROR , sizeof(text2),
                    title, IDSTRD_DATACDCREATOR, sizeof(title),
                    hDataResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
        return FALSE;
      }
    strncat(text," \"-b",strlen(" \"-b"));
    strncat(text,text2,strlen(text2));
    strncat(text,"\"",strlen("\""));

    getMessage(text2,IDSTRD_BOOTCATALOGNAME,sizeof(text2), hDataResource,HWND_DESKTOP);
    sprintf(title,"");
    strncat(title,filename,sizeof(title)-1);
    strncat(title,"\\",sizeof("\\")-strlen(title)-1);
    strncat(title,text2,sizeof(title)-strlen(title)-1);

    if(checkFileExists(title))
      {
        /* There is a file with the name of the boot catalog in the folder. */
        messageBox( text2, IDSTRD_BOOTCATALOGERROR , sizeof(text2),
                    title, IDSTRD_DATACDCREATOR, sizeof(title),
                    hDataResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);
        return FALSE;
      }

    strncat(text," \"-c",strlen(" \"-c"));    
    strncat(text,text2,strlen(text2));
    strncat(text,"\"",strlen("\""));
    
  }/* end of if(ulFlags & IDCDT_BOOTCD) */

#if 0
  if((ulFlags & IDCDT_MULTISESSION) && !(ulFlags & IDCDT_FIRSTSESSION)) {
    /* We are building the second image of a multisession disk */
    lValue1=thisPtr->cwQueryPreviousStartSector();
    lValue2=thisPtr->cwQueryNextStartSector();
    /* Check if we have the necessary sectornumbers */
    if(lValue1==lValue2) {
      sprintf(text2,"There was an error while querying the previous session!");  
      WinMessageBox(  HWND_DESKTOP, HWND_DESKTOP,
                      text2,
                      "Imagefile creation error",
                      0UL, MB_OK | MB_ICONEXCLAMATION |MB_MOVEABLE);
      return FALSE;
    }/* if(lValue1==lValue2) */

    sprintf(filename," -C%d,%d",lValue1,lValue2);
    strncat(text,filename,strlen(filename));

    /* Check if we have a source drive for previous image */
    strncat(text," -M",4);
    /* We use the writer as source. Check if 'dev=x,y,z' is given as cdrecord options */
    /* Add target,lun statement to cmd-line */
    sprintf(chrDev,"%s ", chrChosenDev);
    strncat(text,chrDev,strlen(chrDev));
    /* We have all information. Build the rest of the cmd-line */
  }/* end of if((ulFlags & IDCDT_MULTISESSION) && !(ulFlags & IDCDT_FIRSTSESSION)) */
#endif  

  if(ulFlags & IDCDT_MULTISESSION) {
    /* We are building the image of a multisession disk */
    lValue1=thisPtr->cwQueryPreviousStartSector();
    lValue2=thisPtr->cwQueryNextStartSector();
    /* Check if we have the necessary sectornumbers */
    if(lValue1==lValue2 && lValue1==0) {/* There was an error while querying the sectors or the CD is full */
      sprintf(text2,"There was an error while querying the previous session!");  
      WinMessageBox(  HWND_DESKTOP, HWND_DESKTOP,
                      text2,
                      "Imagefile creation error",
                      0UL, MB_OK | MB_ICONEXCLAMATION |MB_MOVEABLE);
      return FALSE;
    }/* if(lValue1==lValue2) */
    
    if(lValue1!=lValue2 ) {
      sprintf(filename," -C%d,%d",lValue1,lValue2);
      strncat(text,filename,strlen(filename));

      /* Check if we have a source drive for previous image */
      strncat(text," -M",4);
      /* We use the writer as source. Check if 'dev=x,y,z' is given as cdrecord options */
      /* Add target,lun statement to cmd-line */
      sprintf(chrDev,"%s ", chrChosenDev);
      strncat(text,chrDev,strlen(chrDev));
      /* We have all information. Build the rest of the cmd-line */
    }
  }/* end of if((ulFlags & IDCDT_MULTISESSION) && !(ulFlags & IDCDT_FIRSTSESSION)) */
  
  if(outputName!=NULL) {
    /* Name of output file given */
    strncat(text," -o", 4);
    strncat(text,outputName,strlen(outputName));
  }
  
  return TRUE;
}


/* The outputname may be NULL. If so the -o switch will be omitted */
BOOL buildMkisofsParamShadowsRootOnly(CWDataFolder* thisPtr, char * text, char * outputName, char* chrChosenDev)
{	
  char *chrPtrParam;
  ULONG ulFlags;
  char filename[CCHMAXPATH];
  ULONG ulSize;
  HWND hwnd;
  PMINIRECORDCORE mrc;   
  WPObject * contentObject;
  char text2[CCHMAXPATH];
  char title[CCHMAXPATH];


  if(!_buildMkisofsCommonParam(thisPtr, text, outputName, chrChosenDev)) {
    DosFreeMem(pvSharedMem);
    pvSharedMem=NULL;
    return FALSE;
  }

  /*****************************/
  /* Query input files/folders */
  /* and build cmd line        */
  /*****************************/
  strncat(text," ",2);
  /* Find last space. It's the beginning of the file names */
  chrPtrParam=strrchr(text,' ');

  ulSize=sizeof(filename);
  /* Query name of this folder */
  if(!thisPtr->wpQueryRealName(filename,&ulSize,TRUE)) {
    sprintf(text,"Error while quering the foldername! %s, line %d", __FUNCTION__, __LINE__);  
    WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, text, __FUNCTION__,
                    0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
    DosFreeMem(pvSharedMem);
    pvSharedMem=NULL;
    return FALSE;/* There's an error */
  }

  /* Add the folder name enclosed in " " so spaces are allowed */
  strncat(text,"\"",sizeof("\""));
  strncat(text,filename,ulSize);
  strncat(text,"\"",sizeof("\""));

  hwnd=WinQueryWindow(thisPtr->hwndMkisofsMain,QW_PARENT); // Get folder HWND
  /* Get hwnd of folder container */ 
  hwnd=WinWindowFromID(hwnd,FID_CLIENT);
  if(!WinIsWindow(WinQueryAnchorBlock(HWND_DESKTOP),hwnd)) {
    sprintf(text,"Error while quering the container HWND! %s, line %d", __FUNCTION__, __LINE__);  
    WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, text, "In "__FILE__ " " __FUNCTION__,
                    0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
    DosFreeMem(pvSharedMem);
    pvSharedMem=NULL;
    return FALSE;
  }

  mrc=(PMINIRECORDCORE)WinSendMsg(hwnd,CM_QUERYRECORD,NULL,
                                  MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
  if(!mrc){ 
    /* Text: "The folder is empty. Please put some files into it before selecting this function."
       Title: "CD writing warning!"
       */
    messageBox( text2, IDSTR_EMPTYCDFOLDER , sizeof(text2),
                title, IDSTR_WRITEWARNINGTITLE, sizeof(title),
                hDataResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);

    DosFreeMem(pvSharedMem);
    pvSharedMem=NULL;
    return FALSE;
  }

  /* Enumerate folder contents. We only use filesystem objects. If we find a shadow we follow
     the link and use the linked object if it's a filesystem object */
  while(mrc) {
    /* Get wps-object-ptr. from container item */
    contentObject=(WPObject*)OBJECT_FROM_PREC(mrc);//Get object
    if(contentObject) {
      /* Check if it's a shadow */
      if(somResolveByName(contentObject,"wpQueryShadowedObject"))
        /* Yes, it's a shadow. Get file system object or NULL */
        contentObject=thisPtr->cwGetFileSystemObject(contentObject);
      else 
        /* It's no shadow, so do nothing. If it's a filesystem object it's included in our list
           because we yet provide this folder as input directory. If it's any other object (abstract)
           mkisofs will skip it because it uses the filesystem for querying directory contents. */
        contentObject=NULL;
    }
    if(contentObject){
      /* It's a file system object */
      strncat(text," \"",sizeof(" \""));
      /* Check if it's folder */
      if(somResolveByName(contentObject,"wpQueryContent")) {
        /* It's a folder. Query the name. */
        ulSize=sizeof(filename);
        ((WPFileSystem*)contentObject)->wpQueryRealName(filename,&ulSize,FALSE);
        /* Add the graft point to the command line. If not done, mkisofs will include the contents
           of the folder in the image, not the folder itself */ 
        if((strlen(text)+strlen(filename)+4)>=SHAREDMEM_SIZE) {
          sprintf(text,"Error! The length of the cmd-line exceeds %d Bytes!!\n",SHAREDMEM_SIZE);  
          WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, text, __FUNCTION__,
                          0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
          DosFreeMem(pvSharedMem);
          pvSharedMem=NULL;
          return FALSE;
        }
        /* Iclude the name in " " to allow spaces */
        // strncat(text,"\"",sizeof("\""));
        strncat(text,filename,ulSize);
        strncat(text,"/=",2);        
      }
      /* Query the full path */
      ulSize=sizeof(filename);
      ((WPFileSystem*)contentObject)->wpQueryRealName(filename,&ulSize,TRUE);
      /* Add the name to the command line */ 
      if((strlen(text)+strlen(filename)+2)>=SHAREDMEM_SIZE) {
        sprintf(text,"Error! The length of the cmd-line exceeds %d Bytes!!\n",SHAREDMEM_SIZE);  
        WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, text, __FUNCTION__,
                        0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
        DosFreeMem(pvSharedMem);
        pvSharedMem=NULL;
        return FALSE;
      }
      strncat(text,filename,ulSize);
      strncat(text,"\"",sizeof("\""));
    }/* end of if(contentObject) */
    if(thisPtr->bStopCreateThread) {
      DosFreeMem(pvSharedMem);
      pvSharedMem=NULL;
      return FALSE;
    }
    /* Get next container item */
    mrc=(PMINIRECORDCORE)WinSendMsg(hwnd,CM_QUERYRECORD,MPFROMP(mrc),
                                    MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
  }// end of while(mrc)
  /* Add additional 0 for DosExecPgm() */
  strncat(text,"\0",sizeof("\0"));
  /* Change backslashs in sourcefiles */
  changeBackslash(chrPtrParam);
  
  return TRUE;
}


/**************************************************************/
/*                                                            */
/* This function returns the pointer to a file system object  */
/* or NULL.                                                   */
/* It follows shadow objects and returns the object handle    */
/* of the linked object if it's a file system object          */
/*                                                            */
/**************************************************************/
static WPObject* _cwGetFileSystemObject(WPObject* wpObject)
{
  if(!somIsObj(wpObject)) return NULL;//No object given
  
  /* Check if it's a shadow */
  if(somResolveByName(wpObject,"wpQueryShadowedObject")){ 
    /* Yes, it's a shadow. Query the linked object. */
    wpObject=((WPShadow*)wpObject)->wpQueryShadowedObject(FALSE);
  }
  
  if(!somIsObj(wpObject))
    return NULL;//The link is broken
  
  /* Check if it's a file system object */
  if(somResolveByName(wpObject, "wpQueryRealName")){
    return wpObject;/* Yes */
  }
  else
    return NULL;
}


BOOL resetArchiveBit(WPFolder* thisPtr,  FILE *file ,char * parentDir, int iDepth, HWND hwndTest, CWDataFolder* thisPtrData)
{
  ULONG ulSize;
  ULONG ulMkisofsFlags;
  ULONG ulAttr;
  PMINIRECORDCORE mrc;   
  WPObject * contentObject;
  WPObject * contentObject2;
  char filename[CCHMAXPATH];
  char parentPath[CCHMAXPATH];
  char text[1000];

  if(!somIsObj(thisPtr))
    return FALSE;

  if(!somIsObj(thisPtrData))
    return FALSE;

  /* Get mkisofs flags. We need it for archive bit checking. */
  ulMkisofsFlags=thisPtrData->cwQueryMkisofsFlags();

  /* Make sure we don't have a circle */
  if(iDepth>=20) {
    WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP,
                    "Depth of folders exceeds 20. There's probably a cycle linking of shadows. Check your virtual tree. Aborting..."
                    ,"Create filelist",
                    0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );    
    return FALSE;
  }
  
  /* Awake the contents */ 
  if(!thisPtr->wpPopulate(NULLHANDLE, NULL , FALSE))
    return FALSE;/* Error */

  contentObject=thisPtr->wpQueryContent(NULLHANDLE, QC_FIRST);/* Get first object of the folder */

  if(!somIsObj(contentObject))
    return TRUE;/* The folder is empty so return */
  

  /* Check all the folder contents. */
  while(somIsObj(contentObject)) {
    contentObject2=_cwGetFileSystemObject(contentObject);/* Get the filesystem object. This resolves shadows. */    
    if(somIsObj(contentObject2)){
      /* Yes, it's a filesystem object. */
  
      /* Check if it's folder */
      if(somResolveByName(contentObject2,"wpQueryContent")) {
        /* It's a folder */
        if(!resetArchiveBit((WPFolder*) contentObject2, file, parentPath, iDepth+1, hwndTest, thisPtrData)){/* recurse into the folder */
          contentObject2->wpUnlockObject(); /* Unlock the object. It's locked because of the wpPopulate call of the parent folder */
          return FALSE;
        }
      }
      else {
        /* It's a file */
        /* Yes, only files with archive bit */
        ulAttr=((WPFileSystem*)contentObject2)->wpQueryAttr();/* Query attributes */
        if(ulAttr & FILE_ARCHIVED) {
          ((WPFileSystem*)contentObject2)->wpSetAttr(ulAttr & ~ FILE_ARCHIVED);/* Reset archive bit */
        }/* end of if(ulAttr & FILE_ARCHIVED) */        
      }/* else */
      contentObject2->wpUnlockObject(); /* Unlock the object. It's locked because of the wpPopulate call of the parent folder */
    }/* end of if(contentObject2) */           
    /* Save pointer */
    contentObject2=contentObject;
    /* Get next container item */   
    contentObject=thisPtr->wpQueryContent(contentObject, QC_NEXT);
    contentObject2->wpUnlockObject(); /* Unlock the object. It's locked because of the wpPopulate call of the parent folder */
  }// end of while(contentObject)
  return TRUE;
}

#ifdef USE_WPS_NAMES
static void _deleteReturns(char *wpsName)
{
  char *pBuchst;
  char *pRest;

  /* Delete 'Returns' in object title */
  pRest=wpsName;
  while((pBuchst=strchr(pRest,13))!=NULL) {
    *pBuchst=' ';
    pBuchst++;
    if(*pBuchst==10)
      *pBuchst=' ';
    pRest=pBuchst;
  }
  /* Delete '=' in file names */
  pRest=wpsName;
  while((pBuchst=strchr(pRest,'='))!=NULL) {
    *pBuchst='_';
    pBuchst++;
    pRest=pBuchst;
  }

}
#endif

static void _convertControlChars(char *wpsName, int iBufferSize)
{
  char *pBuchst, *pChr;
  char *pRest;
  int i;
  int iLen;

  iLen=strlen(wpsName);
  /* Escape '=' in file names */
  pRest=wpsName;
  while((pBuchst=strchr(pRest,'='))!=NULL) {
    /*  */
    i=strlen(pBuchst);
    /* Make sure the buffer is sufficient big */
    if(iLen+3 > iBufferSize)/* 2 chars for '\\' and one char for terminating 0 */
      i-2; /* This will cause a failing of image creation but at least it will not crash */

    pChr=pBuchst;
    pChr=(char*)memmove((pChr+2),pChr, i+1);/* Move the string by two chars */
    *pBuchst='\\';
    pBuchst++;
    *pBuchst='\\';
    pChr++;
    pRest=pChr;
  }

}

/* 
   thisPtr: Pointer to the folder object
   hwnd:    HWND of the folder container
   iDepth:  recursion counter for detecting cycles

   This function creates a pathlist file
   from the contents of a folder. It recurses into
   subdirs and follows shadows.

   */
#ifdef USE_WPS_NAMES
static BOOL createFileList(WPFolder* thisPtr,  FILE *file , char * parentDir, int iDepth, HWND hwndTest, CWDataFolder* thisPtrData, char * CmdFileName)
{
  ULONG ulSize;
  ULONG ulMkisofsFlags;
  ULONG ulAttr;
  PMINIRECORDCORE mrc;   
  WPObject * contentObject;
  WPObject * contentObject2;
  char filename[CCHMAXPATH];
  char parentPath[CCHMAXPATH];
  char text[1000];
  PSZ  wpsName;
  BOOL bGotCmdFileName=FALSE;

  if(!somIsObj(thisPtr))
    return FALSE;

  if(!somIsObj(thisPtrData))
    return FALSE;
  
  if(thisPtrData->bStopCreateThread)
    return FALSE; /* User pressed abort. Quit... */
  
  /* Get mkisofs flags. We need it for archive bit checking. */
  ulMkisofsFlags=thisPtrData->cwQueryMkisofsFlags();

  /* Make sure we don't have a circle */
  if(iDepth>=20) {
    WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP,
                    "Depth of folders exceeds 20. There's probably a cycle linking of shadows. Check your virtual tree. Aborting..."
                    ,"Create filelist",
                    0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );    
    return FALSE;
  }

  /* Awake the contents */ 
  if(!thisPtr->wpPopulate(NULLHANDLE, NULL , FALSE))
    return FALSE;/* Error */

#ifdef DEBUG
  sprintf(text,"Populating %s\n",filename);
  debugLog(text,iDepth);  
#endif

#if old
  ulSize=sizeof(filename);
  if(!((WPFileSystem*)thisPtr)->wpQueryRealName(filename,&ulSize,FALSE)) /* thisPtr: we're a folder */
    return FALSE;/* Error */
#endif

  /* We use the WPS title so we have long names on FAT, too */
  if((wpsName=((WPFileSystem*)thisPtr)->wpQueryTitle())==NULL) /* thisPtr: we're a folder */
    return FALSE;/* Error */
  /* Delete 'Returns' in object title */
  //  _deleteReturns( wpsName);
  
  contentObject=thisPtr->wpQueryContent(NULLHANDLE, QC_FIRST);/* Get first object of the folder */
  if(!somIsObj(contentObject)) {
    /* The folder is empty so put it explicit into the filelist or it gets lost */
    if(strlen(parentDir)) {
      fwrite(parentDir, sizeof(char), strlen(parentDir),file);
      fwrite("/", sizeof(char), strlen("/"),file);            
    }
    /* Add the WPS longname  */
    /* Write target path into filelist file */
    strncpy(filename,wpsName, sizeof(filename)-1);
    /* Delete 'Returns' in object title */
    _deleteReturns( filename);
    fwrite(filename, sizeof(char), strlen(filename),file);

    fwrite("=", sizeof(char), strlen("="),file);
    /* Get real name. On FAT it's 8.3 */
    ulSize=sizeof(filename);
    if(!((WPFileSystem*)thisPtr)->wpQueryRealName(filename,&ulSize,TRUE))
      return FALSE; /* Error */
    changeBackslash(filename); 
    fwrite(filename, sizeof(char), strlen(filename),file);
    fwrite("\n", sizeof(char), strlen("\n"),file);
    //thisPtr->wpUnlockObj();/* Unlock the object. It's locked because of wpPopulate() */
    return TRUE;
  }

  /* Check all the folder contents. */
  while(somIsObj(contentObject)) {
    contentObject2=_cwGetFileSystemObject(contentObject);/* Get the filesystem object. This resolves shadows. */    
    if(somIsObj(contentObject2)){
      /* Yes, it's a filesystem object. */
      strncpy(parentPath,parentDir, sizeof(parentPath)-1);
      
#if old
      /* Get it's real name */
      ulSize=sizeof(filename);
      if(!((WPFileSystem*)contentObject2)->wpQueryRealName(filename,&ulSize,FALSE))
        return FALSE; /* Error */
#endif

      /* We use the WPS title so we have long names on FAT, too */
      if((wpsName=((WPFileSystem*)contentObject2)->wpQueryTitle())==NULL)
        return FALSE;/* Error */
      strncpy(filename,wpsName, sizeof(filename)-1);
      /* Delete 'Returns' in object title */
      _deleteReturns(filename);
            
#ifdef DEBUG   
      sprintf(text," * contentObject2: %s\n",filename);
      debugLog(text,iDepth);  
#endif      

      /* Check if it's folder */
      if(somResolveByName(contentObject2,"wpQueryContent")) {
        /* It's a folder */
        if(strlen(parentDir))
          strncat(parentPath, "/" , strlen("/"));
        /* Add this folder to the parentpath */
#if old
        strncat(parentPath, filename , strlen(filename));
#endif
        /* Use the WPS name for the target path */
        strncat(parentPath, filename , strlen(filename));

        if(!createFileList((WPFolder*) contentObject2, file, parentPath, iDepth+1, hwndTest, thisPtrData, CmdFileName)){/* recurse into the folder */
          contentObject2->wpUnlockObject(); /* Unlock the object. It's locked because of the wpPopulate call of the parent folder */
          return FALSE;
        }
      } /* end of if(somResolveByName(contentObject2,"wpQueryContent")) */
      else {
        /* It's a file */
        /* Check if we should use the archive bit */
        if(ulMkisofsFlags & IDMK_USEARCHIVEBIT) {
          /* Yes, only files with archive bit */
          ulAttr=((WPFileSystem*)contentObject2)->wpQueryAttr();/* Query attributes */
          if(ulAttr & FILE_ARCHIVED) {
            /* Add the graft point to the command line. If not done, mkisofs will include the contents
               of the folder in the image, not the folder itself */
            
            if(strlen(parentDir)) /* We're not in the root */
              strncat(parentPath,"/",strlen("/"));

            strncat(parentPath, filename ,strlen(filename));/* Target name is WPS name */
            strncat(parentPath,"=",strlen("="));

            /* The first file found will be given as a parameter to mkisofs, because mkisofs need at least one
               filespec on the cmd line when used with a pathlist file */
            if(!strlen(CmdFileName)) {
              strncpy(CmdFileName, parentPath, CMDFILENAMELENGTH-1);
            }
            else {
              bGotCmdFileName=TRUE; /* Set flag that we already have a file for the command line */
              /* Write target path into file (path on CD) */
              fwrite(parentPath, sizeof(char), strlen(parentPath),file);
            }
            
#ifdef DEBUG
            sprintf(text," * ! Going to query the name of %d...\n",contentObject2);
            debugLog(text,iDepth);  
#endif        
            
            /* Query the full path */
            ulSize=sizeof(filename);
            if(!((WPFileSystem*)contentObject2)->wpQueryRealName(filename,&ulSize,TRUE))
              return FALSE;/* Error */

            /* Change backslashes to forwardslashes or mkisofs writes images unreadable with OS/2 */
            changeBackslash(filename);

            /* The first file found will be given as a parameter to mkisofs, because mkisofs need at least one
               filespec on the cmd line when used with a pathlist file */
            if(!bGotCmdFileName) {
                strncat(CmdFileName, filename, CMDFILENAMELENGTH-strlen(CmdFileName)-1);
                bGotCmdFileName=TRUE;
            }
            else {
              /* Write source path into file */
              fwrite(filename, sizeof(char), strlen(filename),file);
              fwrite("\n", sizeof(char), strlen("\n"),file);
            }

            /* Put the filename into the status line */       
            WinSetWindowText(WinWindowFromID(hwndTest,IDST_STATUSTOTALTIME),filename);
          }/* end of if(ulAttr & FILE_ARCHIVED) */
        }/* end of if(ulMkisofsFlags & IDMK_USEARCHIVEBIT) */
        else {
          /* Add the graft point to the command line. If not done, mkisofs will include the contents
             of the folder in the image, not the folder itself */ 
#if old
          if(strlen(parentDir))
            strncat(parentPath,"/=",strlen("/="));
#endif

          if(strlen(parentDir)) /* We're not in the root */
            strncat(parentPath,"/",strlen("/"));

          strncat(parentPath, filename ,strlen(filename));/* Target name is WPS name */
          strncat(parentPath,"=",strlen("="));

          /* The first file found will be given as a parameter to mkisofs, because mkisofs need at least one
             filespec on the cmd line when used with a pathlist file */
          if(!strlen(CmdFileName)) {
            strncpy(CmdFileName, parentPath, CMDFILENAMELENGTH-1);
          }
          else {
            bGotCmdFileName=TRUE; /* Set flag that we already have a file for the command line */
            /* Write target path into file (path on CD) */
            fwrite(parentPath, sizeof(char), strlen(parentPath),file);
          }
          
#ifdef DEBUG
          sprintf(text," * ! Going to query the name of %d...\n",contentObject2);
          debugLog(text,iDepth);  
#endif        
          
          /* Query the full path */
          ulSize=sizeof(filename);
          if(!((WPFileSystem*)contentObject2)->wpQueryRealName(filename,&ulSize,TRUE))
            return FALSE;/* Error */
          /* Change backslashes to forwardslashes or mkisofs writes images unreadable with OS/2 */
          changeBackslash(filename);

          /* The first file found will be given as a parameter to mkisofs, because mkisofs need at least one
             filespec on the cmd line when used with a pathlist file */
          if(!bGotCmdFileName) {
            strncat(CmdFileName, filename, CMDFILENAMELENGTH-strlen(CmdFileName)-1);
            bGotCmdFileName=TRUE;
          }
          else {
            /* Write source path into file */
            fwrite(filename, sizeof(char), strlen(filename),file);
            fwrite("\n", sizeof(char), strlen("\n"),file);
          }

          /* Put the filename into the status line */       
          WinSetWindowText(WinWindowFromID(hwndTest,IDST_STATUSTOTALTIME),filename);
        }/* end of else (ulMkisofsFlags & IDMK_USEARCHIVEBIT) */
      }/* else */

      contentObject2->wpUnlockObject(); /* Unlock the object. It's locked because of the wpPopulate call of the parent folder */

#ifdef DEBUG
      sprintf(text," * Unlocking contentObject2...\n");
      debugLog(text,iDepth);  
#endif

    }/* end of if(contentObject2) */           
    /* Save pointer */
    contentObject2=contentObject;
    /* Get next container item */   
    contentObject=thisPtr->wpQueryContent(contentObject, QC_NEXT);
    contentObject2->wpUnlockObject(); /* Unlock the object. It's locked because of the wpPopulate call of the parent folder */
  }// end of while(contentObject)
#ifdef DEBUG
  sprintf(text,"leaving...\n");
  debugLog(text,iDepth);  
#endif
  return TRUE;
}

#else

/*************************** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! **********************/
/* Use realnames */
static BOOL createFileList(WPFolder* thisPtr,  FILE *file ,char * parentDir, int iDepth, 
                           HWND hwndTest, CWDataFolder* thisPtrData, char * CmdFileName)
{
  ULONG ulSize;
  ULONG ulMkisofsFlags;
  ULONG ulAttr;
  PMINIRECORDCORE mrc;   
  WPObject * contentObject;
  WPObject * contentObject2;
  char filename[CCHMAXPATH];
  char parentPath[CCHMAXPATH];
  char text[1000];
  PSZ  wpsName;
  BOOL bGotCmdFileName=FALSE;
  BOOL bPopulated;

  if(!somIsObj(thisPtr))
    return FALSE;

  if(!somIsObj(thisPtrData))
    return FALSE;
  
  if(thisPtrData->bStopCreateThread)
    return FALSE; /* User pressed abort. Quit... */
  
  /* Get mkisofs flags. We need it for archive bit checking. */
  ulMkisofsFlags=thisPtrData->cwQueryMkisofsFlags();

  /* Make sure we don't have a circle */
  if(iDepth>=20) {
    WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP,
                    "Depth of folders exceeds 20. There's probably a cycle linking of shadows. Check your virtual tree. Aborting..."
                    ,"Create filelist",
                    0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );    
    return FALSE;
  }

  fflush(file);
#ifdef DEBUG
  debugLog("1. In createFileList(), populating folder... ",iDepth);  
#endif

  /* Awake the contents if necessary */
  bPopulated=FALSE;
  if(!(thisPtr->wpQueryFldrFlags() & FOI_POPULATEDWITHALL)) {
    if(!thisPtr->wpPopulate(NULLHANDLE, NULL , FALSE))
      return FALSE;/* Error */
    bPopulated=TRUE;
  }

#ifdef DEBUG
  sprintf(text,"2. Populated, Now trying to query realname...\n");
  debugLog(text,1);  
#endif

  ulSize=sizeof(filename);
  if(!((WPFileSystem*)thisPtr)->wpQueryRealName(filename,&ulSize,FALSE)) /* thisPtr: we're a folder */
    return FALSE;/* Error */
#ifdef DEBUG
  sprintf(text,"3. Realname is: %s\n",filename);
  debugLog(text,iDepth);  
#endif

  contentObject=thisPtr->wpQueryContent(NULLHANDLE, QC_FIRST);/* Get first object of the folder */
  if(!somIsObj(contentObject)) {
    int iLen;

#ifdef DEBUG
  debugLog("Empty folder.\n",iDepth);  
#endif

    /* The folder is empty so put it explicit into the filelist or it gets lost */
    if(strlen(parentDir)) {
      int iLength=strlen(parentDir);
      if(fwrite(parentDir, sizeof(char), iLength,file)!=iLength)
        return FALSE;
      if(fwrite("/", sizeof(char), strlen("/"),file)!=1)
        return FALSE;            
    }

    /* Escape '=' chars  */
    _convertControlChars(filename, CCHMAXPATH);
    iLen=strlen(filename);
#ifdef DEBUG
  debugLog("Before fwrite().\n",iDepth);  
#endif
    if(fwrite(filename, sizeof(char), iLen,file)!=iLen)
      return FALSE;
#ifdef DEBUG
  debugLog("After fwrite().\n",iDepth);  
  debugLog("Before fwrite().\n",iDepth);  
#endif
    if(fwrite("=", sizeof(char), strlen("="),file)!=1)
      return FALSE;;
#ifdef DEBUG
  debugLog("After fwrite().\n",iDepth);  
  debugLog("Get real name...\n",iDepth);  
#endif

    /* Get real name. On FAT it's 8.3 */
    ulSize=sizeof(filename);
    if(!((WPFileSystem*)thisPtr)->wpQueryRealName(filename,&ulSize,TRUE))
      return FALSE; /* Error */
#ifdef DEBUG
  debugLog("Got realname.\n",iDepth);  
#endif
    changeBackslash(filename); 
    _convertControlChars(filename, CCHMAXPATH);
    iLen=strlen(filename);

#ifdef DEBUG
  debugLog("Before fwrite().\n",iDepth);  
#endif

    if(fwrite(filename, sizeof(char), iLen,file)!=iLen)
      return FALSE;
#ifdef DEBUG
  debugLog("After fwrite().\n",iDepth);  
  debugLog("Before fwrite()... ",iDepth);  
#endif
    if(fwrite("\n", sizeof(char), strlen("\n"),file)!=1)
      return FALSE;
#ifdef DEBUG
  debugLog("After fwrite() and done...\n",iDepth);  
#endif
  //thisPtr->wpUnlockObj();/* Unlock the object. It's locked because of wpPopulate() */
  /* The folder will be unlocked if necessary in the parent function of the recursion */
    return TRUE;
  }/* if(!somIsObj(contentObject)) -> This is an empty folder */

  /* Check all the folder contents. */
  while(somIsObj(contentObject)) {
    contentObject2=_cwGetFileSystemObject(contentObject);/* Get the filesystem object. This resolves shadows. */    
    if(somIsObj(contentObject2)){
      /* Yes, it's a filesystem object. */
      strncpy(parentPath,parentDir, sizeof(parentPath)-1);

#ifdef DEBUG
  debugLog("Get realname...\n",iDepth);    
#endif
      
      /* Get it's real name */
      ulSize=sizeof(filename);
      if(!((WPFileSystem*)contentObject2)->wpQueryRealName(filename,&ulSize,FALSE))
        return FALSE; /* Error */

#ifdef DEBUG   
      sprintf(text," * contentObject2: %s\n",filename);
      debugLog(text,iDepth);  
#endif      

      /* Check if it's folder */
      if(somResolveByName(contentObject2,"wpQueryContent")) {
        /* It's a folder */
        if(strlen(parentDir))
          strncat(parentPath, "/" , strlen("/"));
        /* Escape '=' chars */
        _convertControlChars(filename, CCHMAXPATH);
        /* Add this folder to the parentpath */
        strncat(parentPath, filename , strlen(filename));

        if(!createFileList((WPFolder*) contentObject2, file, parentPath, iDepth+1, hwndTest, thisPtrData, CmdFileName)){/* recurse into the folder */
          contentObject2->wpUnlockObject(); /* Unlock the object. It's locked because of the wpPopulate call of the parent folder */
          return FALSE;
        }
      } /* end of if(somResolveByName(contentObject2,"wpQueryContent")) */
      else {
        /* It's a file */
        /* Check if we should use the archive bit */
        if(ulMkisofsFlags & IDMK_USEARCHIVEBIT) {
          /* Yes, only files with archive bit */
          ulAttr=((WPFileSystem*)contentObject2)->wpQueryAttr();/* Query attributes */
          if(ulAttr & FILE_ARCHIVED) {
            /* Add the graft point to the command line. If not done, mkisofs will include the contents
               of the folder in the image, not the folder itself */
            
            if(strlen(parentDir)) /* We're not in the root */
              strncat(parentPath,"/",strlen("/"));
            /* Escape '=' chars */
            _convertControlChars(filename, CCHMAXPATH);
            strncat(parentPath, filename ,strlen(filename));/* Target name */
            strncat(parentPath,"=",strlen("="));

            /* The first file found will be given as a parameter to mkisofs, because mkisofs need at least one
               filespec on the cmd line when used with a pathlist file */
            if(!strlen(CmdFileName)) {
              strncpy(CmdFileName, parentPath, CMDFILENAMELENGTH-1);
            }
            else {
              int iLength=strlen(parentPath);
              bGotCmdFileName=TRUE; /* Set flag that we already have a file for the command line */
              /* Write target path into file (path on CD) */
              if(fwrite(parentPath, sizeof(char), iLength,file)!=iLength)
                return FALSE;
            }
#ifdef DEBUG
            sprintf(text," * ! Going to query the name of %d...\n",contentObject2);
            debugLog(text,iDepth);  
#endif        
            /* Query the full path */
            ulSize=sizeof(filename);
            if(!((WPFileSystem*)contentObject2)->wpQueryRealName(filename,&ulSize,TRUE))
              return FALSE;/* Error */

            /* Change backslashes to forwardslashes or mkisofs writes images unreadable with OS/2 */
            changeBackslash(filename);
            /* Escape '=' chars */
            _convertControlChars(filename, CCHMAXPATH);

            /* The first file found will be given as a parameter to mkisofs, because mkisofs need at least one
               filespec on the cmd line when used with a pathlist file */
            if(!bGotCmdFileName) {
                strncat(CmdFileName, filename, CMDFILENAMELENGTH-strlen(CmdFileName)-1);
                bGotCmdFileName=TRUE;
            }
            else {
              int iLength=strlen(filename);
#ifdef DEBUG 
  debugLog("Before fwrite().\n",iDepth);  
#endif
              /* Write source path into file */
              if(fwrite(filename, sizeof(char), iLength,file)!=iLength)
                return FALSE;
#ifdef DEBUG
  debugLog("After fwrite().\n",iDepth);  
  debugLog("Before fwrite()... ",iDepth);  
#endif
              if(fwrite("\n", sizeof(char), strlen("\n"),file)!=1)
                return FALSE;
#ifdef DEBUG
  debugLog("After fwrite().\n",iDepth);    
#endif
            }
            /* Put the filename into the status line */       
            WinSetWindowText(WinWindowFromID(hwndTest,IDST_STATUSTOTALTIME),filename);
          }/* end of if(ulAttr & FILE_ARCHIVED) */
        }/* end of if(ulMkisofsFlags & IDMK_USEARCHIVEBIT) */
        else {
          /* Add the graft point to the command line. If not done, mkisofs will include the contents
             of the folder in the image, not the folder itself */ 

          if(strlen(parentDir)) /* We're not in the root */
            strncat(parentPath,"/",strlen("/"));

          /* Escape '=' */
          _convertControlChars(filename, CCHMAXPATH);
          strncat(parentPath, filename ,strlen(filename));/* Target name */
          strncat(parentPath,"=",strlen("="));

          /* The first file found will be given as a parameter to mkisofs, because mkisofs need at least one
             filespec on the cmd line when used with a pathlist file */
          if(!strlen(CmdFileName)) {
            strncpy(CmdFileName, parentPath, CMDFILENAMELENGTH-1);
          }
          else {
            int iLength=strlen(parentPath);
            bGotCmdFileName=TRUE; /* Set flag that we already have a file for the command line */
            /* Write target path into file (path on CD) */
            if(fwrite(parentPath, sizeof(char), iLength,file)!=iLength)
              return FALSE;
          }
          
#ifdef DEBUG
          sprintf(text," * ! Going to query the name of %d...\n",contentObject2);
          debugLog(text,iDepth);  
#endif        
          
          /* Query the full path */
          ulSize=sizeof(filename);
          if(!((WPFileSystem*)contentObject2)->wpQueryRealName(filename,&ulSize,TRUE))
            return FALSE;/* Error */

          /* Change backslashes to forwardslashes or mkisofs writes images unreadable with OS/2 */
          changeBackslash(filename);
          /* Escape '=' to '_' */
          _convertControlChars(filename,CCHMAXPATH);

          /* The first file found will be given as a parameter to mkisofs, because mkisofs need at least one
             filespec on the cmd line when used with a pathlist file */
          if(!bGotCmdFileName) {
            strncat(CmdFileName, filename, CMDFILENAMELENGTH-strlen(CmdFileName)-1);
            bGotCmdFileName=TRUE;
          }
          else {
            int iLength=strlen(filename);
#ifdef DEBUG
  debugLog("Before fwrite().\n",iDepth);  
#endif
            /* Write source path into file */
            if(fwrite(filename, sizeof(char), iLength,file)!=iLength)
              return FALSE;
#ifdef DEBUG
  debugLog("After fwrite().\n",iDepth);  
  debugLog("Before fwrite()... ",iDepth);  
#endif
            if(fwrite("\n", sizeof(char), strlen("\n"),file)!=1)
              return FALSE;
#ifdef DEBUG
  debugLog("After fwrite().\n",iDepth);    
#endif
          }

          /* Put the filename into the status line */       
          WinSetWindowText(WinWindowFromID(hwndTest,IDST_STATUSTOTALTIME),filename);
        }/* end of else (ulMkisofsFlags & IDMK_USEARCHIVEBIT) */
      }/* else */

#ifdef DEBUG
      sprintf(text," * Unlocking contentObject2... ");
      debugLog(text,iDepth);  
#endif      
      contentObject2->wpUnlockObject(); /* Unlock the object. It's locked because of the wpPopulate call of the parent folder */
#ifdef DEBUG
      debugLog("Done.\n",iDepth);  
#endif


    }/* end of if(contentObject2) */           
    /* Save pointer */
    contentObject2=contentObject;
    /* Get next container item */   
    contentObject=thisPtr->wpQueryContent(contentObject, QC_NEXT);
    if(bPopulated)
      contentObject2->wpUnlockObject(); /* Unlock the object. It's locked because of the wpPopulate call of the parent folder */
  }// end of while(contentObject)
#ifdef DEBUG
  sprintf(text,"leaving...\n");
  debugLog(text,iDepth);  
#endif
  return TRUE;
}
#endif

/* The outputname may be NULL. If so the -o switch will be omitted */
BOOL buildMkisofsParam(CWDataFolder* thisPtr, char * text, char * outputName, char * chrFileList, char * chrChosenDev)
{	
  char *chrPtrParam;
  char filename[CCHMAXPATH];
  char CmdFileName[CMDFILENAMELENGTH]={0};
  ULONG ulSize;
  WPObject * contentObject;
  FILE * file;  
  int iExtPriv;
  /* Extension counter */
  static int iExt=0;

  if(cwRequestMutex(hmtxFileName, 100000)==ERROR_TIMEOUT)
    return FALSE;
  iExt++;
  if(iExt==1000)
    iExt=1;
  /* The extension of our filelist file */
  iExtPriv=iExt;
  cwReleaseMutex(hmtxFileName);

  if(!_buildMkisofsCommonParam(thisPtr, text, outputName, chrChosenDev)) {
    DosFreeMem(pvSharedMem);
    pvSharedMem=NULL;
    return FALSE;
  }

  /*****************************/
  /* Query input files/folders */
  /* and build cmd line        */
  /*****************************/
  strncat(text," -path-list \"",strlen(" -path-list \""));
  sprintf(filename, "%s.%03d",PATHLISTFILE, iExtPriv);
  strncat(text,chrInstallDir,strlen(chrInstallDir));
  strncat(text,"\\",strlen("\\"));
  strncat(text,filename,strlen(filename));
  strncat(text,"\" ",strlen("\" "));
  /* Find last space. It's the beginning of the file names */
  chrPtrParam=strrchr(text,' ');

  ulSize=sizeof(filename);
  /* Query name of this folder */
  if(!thisPtr->wpQueryRealName(filename,&ulSize,TRUE)) {
    sprintf(filename,"Error while quering the foldername!");  
    WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, filename, "In "__FILE__ " " __FUNCTION__,
                    0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
    DosFreeMem(pvSharedMem);
    pvSharedMem=NULL;
    return FALSE;/* There's an error */
  }

  /* create pathlist filename */
  sprintf(filename,"%s\\%s.%03d", chrInstallDir, PATHLISTFILE, iExtPriv);
  //  strcpy(filename,chrInstallDir);
  //strncat(filename,"\\",strlen("\\"));
  //  strncat(filename,PATHLISTFILE,strlen(PATHLISTFILE));
  /* Open the file */
  file=fopen(filename,"wb");

  if(!file) {
    /* CmdFileName: "Error while opening the FILELIST.xxx file! Make sure the 'TEMP' directory exists in the Data-CD-Creator installation directory."
       filename: "CD writing error!"
       */
    messageBox( CmdFileName, IDSTR_NOFILELIST , sizeof(CmdFileName),
                filename, IDSTR_WRITEERRORTITLE, sizeof(filename),
                hDataResource, HWND_DESKTOP, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE);

    DosFreeMem(pvSharedMem);
    pvSharedMem=NULL;
    return FALSE;

  }
  strcpy(chrFileList, filename);

  /* static BOOL createFileList(WPFolder* thisPtr,  FILE *file ,char * parentDir, int iDepth, HWND hwndTest, CWDataFolder* thisPtrData) */
  if(!createFileList(thisPtr, file, "", 0 , thisPtr->hwndStatusCntrl, thisPtr, CmdFileName) || thisPtr->bStopCreateThread) {
    DosFreeMem(pvSharedMem);
    pvSharedMem=NULL;
    fclose(file);
    remove(filename);
    return FALSE;
  }

  fclose(file);

  strncat(text,"\"",sizeof("\""));
  strncat(text, CmdFileName,strlen(CmdFileName));
  strncat(text,"\"",sizeof("\""));

  /* Add additional 0 for DosExecPgm() */
  strncat(text,"\0",sizeof("\0"));
  /* Change backslashs in sourcefiles */
  changeBackslash(chrPtrParam);  
  return TRUE;
}

/* This function checks if a helper program is available */
BOOL checkHelper(PSZ helperPath) 
{
  struct stat statBuf;
  char text[CCHMAXPATH];
  char title[CCHMAXPATH];
  
  /* Check cdrdao path */
  if(stat(helperPath , &statBuf)==-1) {
    /* text: "Can't find the helper program %s. It should be in the installation directory of Audio/Data-CD-Creator."
       title: "Audio/Data-CD-Creator installation problem!"
       */
    getMessage(title, IDSTR_INSTALLNOVIO, sizeof(title), hAudioResource , HWND_DESKTOP);
    sprintf(text,title,helperPath);
    getMessage(title, IDSTR_INSTALLERRORTITLE, sizeof(title), hAudioResource , HWND_DESKTOP);
    
    WinMessageBox(  HWND_DESKTOP,   HWND_DESKTOP, text,title,
                    0UL, MB_OK | MB_ICONEXCLAMATION|MB_MOVEABLE );
    return FALSE; 
  }
  
  return TRUE;  
}


/* This function launches the wrapper <wrapperExe>         */
/*  with the params given as a PM-session */
/* in PSZ parameter. PSZ folderPath is the path to put the */
/* write.log. HWND hwnd is the window waiting for the      */
/* WM_APPTERMINATE message                                 */
ULONG launchDRDialog(PSZ parameter, PSZ resFile, PSZ pszTitle)
{
  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrLoadError[CCHMAXPATH];
  char startParams[CCHMAXPATH*4];
  char exename[CCHMAXPATH]={0};
  
  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_FORE;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;
    
  /*  sprintf(exename,"%s",buildWrapName(wrapperExe));*/
  sprintf(exename,"%s\\bin\\%s",chrInstallDir, DRDIALOG_RUNTIME);

  startData.PgmName=exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_PM;
  startData.PgmControl=0;
  startData.InitXPos=30;
  startData.InitYPos=30;
  startData.InitXSize=500;
  startData.InitYSize=400;
  startData.ObjectBuffer=chrLoadError;
  startData.ObjectBuffLen=(ULONG)sizeof(chrLoadError);

  /* Put the exe-path between " " to make sure, spaces are handled correctly */
  sprintf(startParams,"\"%s\\bin\\%s\" %s",
          chrInstallDir, resFile, parameter);
  startData.PgmInputs=startParams;

  rc=DosStartSession(&startData,&ulSessionID,&pid);   
  return 0;   
}

/* This function launches the wrapper <wrapperExe>         */
/*  with the params given as a PM-session */
/* in PSZ parameter. PSZ folderPath is the path to put the */
/* write.log. HWND hwnd is the window waiting for the      */
/* WM_APPTERMINATE message                                 */
ULONG launchPMWrapper(PSZ parameter, PSZ folderPath, PSZ wrapperExe, PSZ pszTitle="CDRecord/2")
{
  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrFolderPath[CCHMAXPATH+10];
  _launchData* ptrLaunchData;

 /* Keep data from the stack */
 if((ptrLaunchData=new(_launchData))==NULLHANDLE)
   return 1;
  
  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_FORE;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;
    
  //sprintf(exename,"%s",buildWrapName(wrapperExe));
  sprintf(ptrLaunchData->exename,"%s\\bin\\%s",chrInstallDir, wrapperExe);

  if(!checkHelper(ptrLaunchData->exename)) {
    delete ptrLaunchData;
    return -1;
  }

  startData.PgmName=ptrLaunchData->exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_PM;
  startData.PgmControl=0;
  startData.InitXPos=30;
  startData.InitYPos=30;
  startData.InitXSize=500;
  startData.InitYSize=400;
  startData.ObjectBuffer=ptrLaunchData->chrLoadError;
  startData.ObjectBuffLen=(ULONG)sizeof(ptrLaunchData->chrLoadError);

  /* Put the exe-path between " " to make sure, spaces are handled correctly */

  strcpy(chrFolderPath,folderPath);
  sprintf(ptrLaunchData->startParams,"\"%s\" \"%s\" %s",
          chrInstallDir,chrFolderPath, parameter);

  startData.PgmInputs=ptrLaunchData->startParams;

  rc=DosStartSession(&startData,&ulSessionID,&pid);
  delete ptrLaunchData;
   
  return 0;   
}


/* This function launches the wrapper <wrapperExe>         */
/*  with the params given                                  */
/* in PSZ parameter. PSZ folderPath is the path to put the */
/* write.log. HWND hwnd is the window waiting for the      */
/* WM_APPTERMINATE message                                 */
ULONG launchWrapper(PSZ parameter, PSZ folderPath,HWND hwnd, PSZ wrapperExe, PSZ title="CDRecord/2")
{
  return launchWriter(parameter, folderPath,hwnd, wrapperExe, title);
}


/* This function launches the writer with the params given */
/* in PSZ parameter. PSZ folderPath is the path to put the */
/* write.log. HWND hwnd is the window waiting for the      */
/* WM_APPTERMINATE message                                 */
ULONG launchWriter(PSZ parameter, PSZ folderPath,HWND hwnd, PSZ wrapperExe, PSZ pszTitle="CDRecord/2")
{
  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrCDRexe[CCHMAXPATH+10];
  struct stat statBuf;
  _launchData* ptrLaunchData;

 /* Keep data from the stack */
 if((ptrLaunchData=new(_launchData))==NULLHANDLE)
   return 1;

  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_BACK;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;

  //sprintf(ptrLaunchData->exename,"%s", buildWrapName(wrapperExe));
  sprintf(ptrLaunchData->exename,"%s\\bin\\%s",chrInstallDir, wrapperExe);
  /* Check if we have a helper program */

  if(stat(ptrLaunchData->exename , &statBuf)==-1) {
    /* text: "Can't find the helper program %s. It should be in the installation directory of Audio/Data-CD-Creator."
       title: "Audio/Data-CD-Creator installation problem!"
       */
    getMessage(ptrLaunchData->chrLoadError, IDSTR_INSTALLNOVIO, sizeof(ptrLaunchData->chrLoadError),
               hDataResource , hwnd);
    sprintf(ptrLaunchData->startParams, ptrLaunchData->chrLoadError,wrapperExe);
    getMessage(ptrLaunchData->chrLoadError, IDSTR_INSTALLERRORTITLE, 
               sizeof(ptrLaunchData->chrLoadError), hDataResource , hwnd);
    WinMessageBox(HWND_DESKTOP,0,
                  ptrLaunchData->startParams,
                  ptrLaunchData->chrLoadError,12345,
                  MB_OK|MB_MOVEABLE|MB_ERROR);
    delete ptrLaunchData;
    return -1;
  }

  startData.PgmName=ptrLaunchData->exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_WINDOWABLEVIO;
  startData.PgmControl=0;
  if(lCDROptions&IDCDR_HIDEWINDOW)
    startData.PgmControl|=SSF_CONTROL_INVISIBLE;//|SSF_CONTROL_MAXIMIZE|SSF_CONTROL_NOAUTOCLOSE;
  if(!(lCDROptions&IDCDR_CLOSEWINDOW))
    startData.PgmControl|=SSF_CONTROL_NOAUTOCLOSE;
  startData.InitXPos=30;
  startData.InitYPos=30;
  startData.InitXSize=500;
  startData.InitYSize=400;
  startData.ObjectBuffer=ptrLaunchData->chrLoadError;
  startData.ObjectBuffLen=(ULONG)sizeof(ptrLaunchData->chrLoadError);

  /* Put the exe-path between " " to make sure, spaces are handled correctly */
  sprintf(chrCDRexe,"\"%s\"",chrCDRecord);
  sprintf(ptrLaunchData->startParams,"%d %s %s %s",hwnd, chrCDRexe,folderPath ,parameter);

  startData.PgmInputs=ptrLaunchData->startParams;

  rc=DosStartSession(&startData,&ulSessionID,&pid);   
  delete ptrLaunchData;
  return rc;   
}
#if 0
ULONG launchWriter(PSZ parameter, PSZ folderPath,HWND hwnd, PSZ wrapperExe, PSZ pszTitle="CDRecord/2")
{
  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  char chrLoadError[CCHMAXPATH];
  char startParams[CCHMAXPATH*4];
  //  char exename[CCHMAXPATH]={0};
  char chrCDRexe[CCHMAXPATH+10];
 struct stat statBuf;
 _launchData* ptrLaunchData;

 if((ptrLaunchData=new(_launchData))==NULLHANDLE)
   return 1;

  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_BACK;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;

  sprintf(ptrLaunchData->exename,"%s",buildWrapName(wrapperExe));
  /* Check if we have a helper program */

  if(stat(ptrLaunchData->exename , &statBuf)==-1) {
    /* text: "Can't find the helper program %s. It should be in the installation directory of Audio/Data-CD-Creator."
       title: "Audio/Data-CD-Creator installation problem!"
       */
    getMessage(chrLoadError, IDSTR_INSTALLNOVIO, sizeof(chrLoadError), hDataResource , hwnd);
    sprintf(startParams,chrLoadError,wrapperExe);
    getMessage(chrLoadError, IDSTR_INSTALLERRORTITLE, sizeof(chrLoadError), hDataResource , hwnd);
    WinMessageBox(HWND_DESKTOP,0,
                  startParams,
                  chrLoadError,12345,
                  MB_OK|MB_MOVEABLE|MB_ERROR);
    return -1;
  }


  startData.PgmName=ptrLaunchData->exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_WINDOWABLEVIO;
  startData.PgmControl=0;
  if(lCDROptions&IDCDR_HIDEWINDOW)
    startData.PgmControl|=SSF_CONTROL_INVISIBLE;//|SSF_CONTROL_MAXIMIZE|SSF_CONTROL_NOAUTOCLOSE;
  if(!(lCDROptions&IDCDR_CLOSEWINDOW))
    startData.PgmControl|=SSF_CONTROL_NOAUTOCLOSE;
  startData.InitXPos=30;
  startData.InitYPos=30;
  startData.InitXSize=500;
  startData.InitYSize=400;
  startData.ObjectBuffer=chrLoadError;
  startData.ObjectBuffLen=(ULONG)sizeof(chrLoadError);

  /* Put the exe-path between " " to make sure, spaces are handled correctly */
  sprintf(chrCDRexe,"\"%s\"",chrCDRecord);
  sprintf(startParams,"%d %s %s %s",hwnd,chrCDRexe,folderPath ,parameter);

  startData.PgmInputs=startParams;

  rc=DosStartSession(&startData,&ulSessionID,&pid);   
  return rc;   
}
#endif

ULONG launchMsInfo(HWND hwnd)
{
  /* hwnd:    notification window
     msinfo.exe sends a WM_APPTERMINATIONNOTIFY msg to that window which contains in mp1/mp2 the
     two numbers necessary for creating of the second session.
     The correct cmd-line for calling cdrecord/2 is build by msinfo. So we supply only the generic
     cdrecord/2 information.
   */

  STARTDATA startData={0};
  APIRET rc;
  PID pid;
  ULONG ulSessionID=0;
  PSZ pszTitle="Query multisession info";
  //char chrLoadError[CCHMAXPATH];
  //char startParams[CCHMAXPATH*4];
  //char exename[CCHMAXPATH]={0};
  _launchData* ptrLaunchData;
  
  /* Keep data from the stack */
  if((ptrLaunchData=new(_launchData))==NULLHANDLE)
    return 1;
    
  memset(&startData,0,sizeof(startData));
  startData.Length=sizeof(startData);
  startData.Related=SSF_RELATED_INDEPENDENT;
  startData.FgBg=SSF_FGBG_BACK;
  startData.TraceOpt=SSF_TRACEOPT_NONE;
  startData.PgmTitle=pszTitle;
    
  //  sprintf(exename,"%s",buildWrapName("msinfo.exe"));
  sprintf(ptrLaunchData->exename,"%s\\bin\\%s",chrInstallDir, "msinfo.exe");
  startData.PgmName=ptrLaunchData->exename;
  startData.InheritOpt=SSF_INHERTOPT_SHELL;
  startData.SessionType=SSF_TYPE_WINDOWABLEVIO;
  startData.PgmControl=SSF_CONTROL_INVISIBLE;//|SSF_CONTROL_MAXIMIZE|SSF_CONTROL_NOAUTOCLOSE;
  startData.InitXPos=30;
  startData.InitYPos=30;
  startData.InitXSize=500;
  startData.InitYSize=400;
  startData.ObjectBuffer=ptrLaunchData->chrLoadError;
  startData.ObjectBuffLen=(ULONG)sizeof(ptrLaunchData->chrLoadError);
  //  sprintf(startParams,"%d %s %s",hwnd, chrCDRecord, chrCDROptions);
  sprintf(ptrLaunchData->startParams,"%d %s dev=%d,%d,%d %s",hwnd, chrCDRecord, iBus, iTarget, iLun, chrDataCDROptions);
  startData.PgmInputs=ptrLaunchData->startParams;
  rc=DosStartSession(&startData,&ulSessionID,&pid);
  delete ptrLaunchData;
  return 0;   
}

















