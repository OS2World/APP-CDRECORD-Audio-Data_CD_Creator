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
#define INCL_DOS
#define INCL_WIN
#define INCL_GPIBITMAPS
#define INCL_PM

#include <os2.h>
#include "os2me.h"
#include "audiofolder.h"

#include "debug.h"
#include "except.h"


#pragma SOMAsDefault(off)
#include "cwaudio.hh"
#pragma SOMAsDefault(pop)

/* For calling the method in the CWAudio class. Linking to the DLL would require the
   new audio classes. By dynamically querying the method the media folder works without
   installing the audio classes. */  
typedef ULONG   SOMLINK somTP_CWAudio_cwmmQueryTrackInfo(CWAudio *somSelf,
		char** chrString,
		ULONG ulSize,
		int iWhich);
typedef somTP_CWAudio_cwmmQueryTrackInfo *somTD_CWAudio_cwmmQueryTrackInfo;

typedef struct
{
  char chrCdrdaoPath2[CCHMAXPATH];/* Source for 1:1 copy */
  char chrCdrdaoDriver2[100];
  char chrWriterName2[100];
  char chrDeviceName2[100];
  char chrCdrdaoOptions2[CCHMAXPATH];
  char chrCdrdaoPath3[CCHMAXPATH];/* Target for 1:1 copy */
  char chrCdrdaoDriver3[100];
  char chrWriterName3[100];
  char chrDeviceName3[100];
  char chrCdrdaoOptions3[CCHMAXPATH];
  /* Grabber options */
  char chrGrabberPath[CCHMAXPATH];
  char chrGrabberOptions[CCHMAXPATH];
  char chrGrabberName[100];
  int iGrabberID;
  int bTrackNumbers;
  /* MP3 decoder */
  char chrMpg123Path[CCHMAXPATH];
  BOOL bMpg123SwabBytes;
  //  int iMp3Decoder;

  char chrMP3LibraryPath[CCHMAXPATH];
  /* The method ptr */
  somTD_CWAudio_cwmmQueryTrackInfo cwmmQueryTrackInfoMthdPtr; 
  BOOL bTipsEnabled;
  ULONG ulGlobalFlags;  /* see audiofolder.h */

}GLOBALDATA;

typedef struct
{
  LONG lCDROptions;
  char chrDeviceName[CCHMAXPATH];
}UNDODATA;

#if 0
/* now in audiofolder.h*/
typedef struct
{
  BITMAPINFOHEADER bmpInfoHdr;
  HBITMAP hbm;
}LOADEDBITMAP;
typedef struct
{
  INT id;
  RECTL rclSource;
  HBITMAP hbmSource;
  RECTL rclDest;
  BITMAPINFOHEADER2 bmpInfoHdr2;
}CONTROLINFO;
#endif

class CWAudioFolder;

typedef struct _DROPTHREADPARAMS
{
  CWAudioFolder *thisPtr;
  HWND hwndCnr;
  HWND hwndSource;
  ULONG ulNumObjects;
  POINTL ptl;
  WPObject* wpObject[1];
}DROPTHREADPARAMS;

typedef DROPTHREADPARAMS* PDROPTHREADPARAMS;

class launchPad;

class _launchData
{
public:
  char chrLoadError[CCHMAXPATH];
  char startParams[CCHMAXPATH*4];
  char exename[CCHMAXPATH];
  char chrCDRexe[CCHMAXPATH+10];
};

#define STATUS_LENGTH 200

#ifdef DEBUG
    void writeDebugLog(char *text);
#endif


#pragma SOMAsDefault(on)
#include <som.hh>
#include <somcls.hh>
#pragma SOMNoMangling(on)
#include "progfolder.h"
#include "progfolder.hh"
//For WPAbstract:
#include <wpabs.hh>

#include "creatorsettings.hh"


class M_CWAudioFolder:public M_CWProgFolder
{
/* The name of of the new class */
#pragma SOMClassName(*,"M_CWAudioFolder")
/* Required SOM version */
#pragma SOMClassVersion(*,1,2)
#pragma SOMCallstyle(oidl)
public:
	virtual PSZ  wpclsQueryTitle();
	virtual	ULONG wpclsQueryIconData(PICONINFO pIconInfo);
	virtual	ULONG wpclsQueryIconDataN(PICONINFO pIconInfo, ULONG ulIconIndex);
	virtual	ULONG wpclsQueryStyle();
	virtual BOOL wpclsQueryDefaultHelp(PULONG HelpPanelId,PSZ HelpLibrary);
	virtual void wpclsInitData();
  virtual void wpclsUnInitData();
  virtual BOOL wpclsCreateDefaultTemplates(WPObject * wpObject);
#pragma SOMReleaseOrder()
#pragma SOMIDLPass(*,"Implementation-End","dllname = aucdfldr;")
};

class CWAudioFolder:public CWProgFolder
{
#pragma SOMClassName(*,"CWAudioFolder")
#pragma SOMMetaClass(CWAudioFolder,"M_CWAudioFolder")
#pragma SOMClassVersion(CWAudioFolder,1,2)
#pragma SOMCallstyle(oidl)
private:
  ULONG ulWriteFlags;
  BOOL bGrabEnabled;
  BOOL bWriteEnabled;
public:		
  ULONG ulPageID;
  BOOL bShowGrab;
  HWND hwndGrab;/* Grab frame control (left) */
  HWND hwndWrite;/* Write frame control (left) */
  HWND hwndStatus;
  HWND hwndSelect;/* Select frame control (top) */
  HWND hwndStatusFrameCtl;/* Status line frame control (bottom) */
  //HWND hwndWriteStatus;/* Status line frame control (bottom) */
  HWND hwndWriteLB;
  PFNWP pfnwpGrabLB;/* Track listbox in the grab frame control */
  PFNWP pfnwpContainer;/* Folder container */
  PFNWP pfnwpFrame;
  ULONG ulTrackSize;
  USHORT usLastSelMenuItem;
  launchPad* lPad;	
  SHORT a;
  int b;
  int numTracks;
  int tid;
  int tidWrite;
  int aWrite;
  int bWrite;
  BOOL bPopulated;

  virtual BOOL wpMenuItemHelpSelected(ULONG MenuId);
  virtual ULONG wpFilterPopupMenu(ULONG ulFlags, HWND hwndCnr, BOOL fMultiSelect);
  virtual BOOL wpModifyPopupMenu(HWND hwndMenu, HWND hwndCnr, ULONG ulPosition);
  virtual BOOL wpMenuItemSelected(HWND hwndFrame,ULONG ulMenuId);
  virtual ULONG wpAddFolderView2Page(HWND hwndNotebook);
  virtual MRESULT wpDragOver(HWND hwndCnr,PDRAGINFO pDragInfo);
  virtual BOOL wpAddSettingsPages(HWND hwndNotebook);
  
  virtual BOOL wpPopulate(ULONG ulReserved,PSZ Folder,BOOL fFoldersOnly);
  virtual BOOL wpAddToContent(WPObject* Object);
  virtual BOOL wpDeleteFromContent(WPObject* Object);
  virtual BOOL cwIsWaveFile(WPObject* wpObject, BOOL bWithoutExt);
  virtual ULONG cwSetWriteFlags(ULONG ulNewWriteFlags,ULONG ulMask);
  virtual ULONG cwQueryWriteFlags();
  virtual BOOL wpRestoreState(ULONG ulReserved);
  virtual BOOL wpSaveState();	
  virtual HWND wpOpen(HWND hwndCnr,ULONG ulView,ULONG ulParam);
  virtual MRESULT wpDrop(HWND hwndCnr,PDRAGINFO pDragInfo,PDRAGITEM pDragItem);
  virtual BOOL cwClose(HWND hwndFrame);	
  virtual BOOL wpSetupOnce(PSZ pSetupString);
  virtual BOOL wpSetup(PSZ pSetupString);
  virtual void cwEnableGrab(BOOL bEnable);
  virtual BOOL cwQueryGrabEnabled(void);
  virtual void cwEnableGrabControls(BOOL bEnable);
  virtual void cwEnableSelectControls(BOOL bEnable);
  virtual ULONG cwFillTrackList(HWND hwndFrame, HWND hwndListBox);
  virtual void cwSetStatusTime(ULONG ulTrackSize);
  virtual void cwSetStatusText(char * text);
  virtual void cwForceStatusUpdate(void);
  virtual void cwEnableWrite(BOOL bEnable);
  virtual BOOL cwQueryWriteEnabled(void);
  virtual void cwEnableWriteControls(BOOL bEnable);
  virtual void cwWriteAudioTracks(HWND hwnd, char* trackName, char* folderName);
  virtual BOOL cwIsMp3File(WPObject* wpObject);
  virtual BOOL cwCreateContentsFile(char * fileName, HWND hwndFrame);
  virtual BOOL cwIsAudioFile(WPObject* wpObject);
  virtual BOOL cwCreateContentsFileForDec(char * fileName, HWND hwndFrame);
  virtual BOOL cwCreateContentsFileForEncoding(char * fileName, HWND hwndFrame);
#pragma SOMReleaseOrder(\
"cwIsWaveFile",\
"cwSetWriteFlags",\
"cwQueryWriteFlags",\
"cwEnableGrab",\
"cwQueryGrabEnabled",\
"cwEnableGrabControls",\
"cwEnableSelectControls",\
"cwFillTrackList",\
"cwSetStatusTime",\
"cwForceStatusUpdate",\
"cwEnableWrite",\
"cwQueryWriteEnabled",\
"cwEnableWriteControls",\
"cwSetStatusText",\
"cwWriteAudioTracks",\
"cwIsMp3File",\
"cwCreateContentsFile",\
"cwIsAudioFile",\
"cwCreateContentsFileForDec",\
"cwCreateContentsFileForEncoding")
#pragma SOMIDLPass(CWAudioFolder,"Implementation-End","dllname = aucdfldr;")
};

class M_CWDataFolder:public M_CWProgFolder
{
#pragma SOMClassName(*,"M_CWDataFolder")
#pragma SOMClassVersion(*,1,2)
#pragma SOMCallstyle(oidl)
public:
  virtual PSZ  wpclsQueryTitle();
  virtual	ULONG wpclsQueryIconData(PICONINFO pIconInfo);
  virtual	ULONG wpclsQueryIconDataN(PICONINFO pIconInfo, ULONG ulIconIndex);
  virtual	ULONG wpclsQueryStyle();
  virtual BOOL wpclsQueryDefaultHelp(PULONG HelpPanelId,PSZ HelpLibrary);
  virtual void wpclsInitData();
  virtual void wpclsUnInitData();
  virtual BOOL wpclsCreateDefaultTemplates(WPObject * wpObject);
#pragma SOMReleaseOrder()
#pragma SOMIDLPass(*,"Implementation-End","dllname = aucdfldr;")
};

class CWDataFolder:public CWProgFolder
{
#pragma SOMClassName(*,"CWDataFolder")
#pragma SOMMetaClass(CWDataFolder,"M_CWDataFolder")
#pragma SOMClassVersion(CWDataFolder,1,2)
#pragma SOMCallstyle(oidl)
private:
  ULONG ulMkisofsFlags;
  ULONG ulCDTypeFlags;
  ULONG ulCreateFlags;
  LONG lPreviousStart;
  LONG lNextStart;

public:		
  SHORT sDummy;
  HWND hwndImageName;
  HWND hwndMkisofsMain;
  HWND hwndStatusCntrl;
  PFNWP pfnwpContainer;
  PFNWP pfnwpFrame;
  USHORT usLastSelMenuItem;
  int iCreate;
  BOOL bStopCreateThread;
  launchPad* lPad;	

  char chrApplication[128+1];
  char chrPublisher[128+1];
  char chrPreparer[128+1];
  char chrCopyright[37+1];
  char chrVolumeName[37+1];
  char chrStatusText[STATUS_LENGTH];
  char chrBootImage[CCHMAXPATH];
  char chrBootCatalog[CCHMAXPATH];
  char chrImageName[CCHMAXPATH];

  virtual BOOL wpMenuItemHelpSelected(ULONG MenuId);
  virtual MRESULT wpDragOver(HWND hwndCnr,PDRAGINFO pDragInfo);
  virtual BOOL wpModifyPopupMenu(HWND hwndMenu, HWND hwndCnr, ULONG ulPosition);
  virtual BOOL wpMenuItemSelected(HWND hwndFrame,ULONG ulMenuId);
  virtual BOOL wpSetup(PSZ pSetupString);
  virtual BOOL wpSetupOnce(PSZ pSetupString);
  virtual BOOL wpAddSettingsPages(HWND hwndNotebook);
  virtual BOOL AddFileNameOptionPage(HWND hwndNotebook);
  virtual ULONG cwSetMkisofsFlags(ULONG ulNewMkisofsFlags,ULONG ulMask);
  virtual ULONG cwQueryMkisofsFlags();
  virtual BOOL wpRestoreState(ULONG ulReserved);
  virtual BOOL wpSaveState();	
  virtual HWND wpOpen(HWND hwndCnr,ULONG ulView,ULONG ulParam);
  virtual BOOL cwClose(HWND hwndFrame);	
  virtual void cwCreateImage();
  virtual ULONG AddCDTypeOptionPage(HWND hwndNotebook);
  virtual LONG cwQueryPreviousStartSector();
  virtual LONG cwQueryNextStartSector();
  virtual void cwSetPreviousStartSector(LONG lSector);
  virtual void cwSetNextStartSector(LONG lSector);
  virtual ULONG cwSetCDTypeFlags(ULONG ulNewCDTypeFlags,ULONG ulMask);
  virtual ULONG cwQueryCDTypeFlags();
  virtual BOOL AddAuthorOptionPage(HWND);
  virtual void cwWriteImage();
  virtual void cwSetStatusText(PSZ pszText);
  virtual ULONG cwSetCreateFlags(ULONG ulNewCreateFlags,ULONG ulMask);
  virtual ULONG cwQueryCreateFlags();
  virtual void cwWriteOnTheFly();
  /*   virtual MRESULT wpDrop(HWND hwndCnr,PDRAGINFO pDragInfo,PDRAGITEM pDragItem);
      virtual BOOL wpSetupOnce(PSZ pSetupString);
      virtual void wpCopiedFromTemplate();*/
  virtual void cwEnableWriteControls(BOOL bEnable);
  virtual void cwGetImageSize();
  virtual void cwShowImageName(BOOL bShow);
  virtual BOOL AddSpecialOptionPage(HWND hwndNotebook);
  virtual void cwResetArchiveBit();
#pragma SOMReleaseOrder(\
"AddFileNameOptionPage",\
"cwSetMkisofsFlags",\
"cwQueryMkisofsFlags",\
"cwCreateImage",\
"AddCDTypeOptionPage",\
"cwQueryPreviousStartSector",\
"cwQueryNextStartSector",\
"cwSetPreviousStartSector",\
"cwSetNextStartSector",\
"cwSetCDTypeFlags",\
"cwQueryCDTypeFlags",\
"AddAuthorOptionPage",\
"cwWriteImage",\
"cwSetStatusText",\
"cwSetCreateFlags",\
"cwQueryCreateFlags",\
"cwWriteOnTheFly",\
"cwEnableWriteControls",\
"cwGetImageSize",\
"cwShowImageName",\
"AddSpecialOptionPage",\
"cwResetArchiveBit")
#pragma SOMIDLPass(CWDataFolder,"Implementation-End","dllname = aucdfldr;")
};

class M_CWDVDFolder:public M_CWDataFolder
{
#pragma SOMClassName(*,"M_CWDVDFolder")
#pragma SOMClassVersion(*,1,2)
#pragma SOMCallstyle(oidl)
public:
  virtual PSZ  wpclsQueryTitle();
  //  virtual	ULONG wpclsQueryIconData(PICONINFO pIconInfo);
  //  virtual	ULONG wpclsQueryIconDataN(PICONINFO pIconInfo, ULONG ulIconIndex);
  //  virtual	ULONG wpclsQueryStyle();
  //  virtual BOOL wpclsQueryDefaultHelp(PULONG HelpPanelId,PSZ HelpLibrary);
  //  virtual void wpclsInitData();
  //  virtual void wpclsUnInitData();
  virtual BOOL wpclsCreateDefaultTemplates(WPObject * wpObject);
#pragma SOMReleaseOrder()
#pragma SOMIDLPass(*,"Implementation-End","dllname = aucdfldr;")
};

class CWDVDFolder:public CWDataFolder
{
#pragma SOMClassName(*,"CWDVDFolder")
#pragma SOMMetaClass(CWDVDFolder,"M_CWDVDFolder")
#pragma SOMClassVersion(CWDVDFolder,1,2)
#pragma SOMCallstyle(oidl)
private:

public:		
  /*  HWND hwndImageName;
  HWND hwndMkisofsMain;
  HWND hwndStatusCntrl;
  PFNWP pfnwpContainer;
  PFNWP pfnwpFrame;
  USHORT usLastSelMenuItem;
  int iCreate;
  BOOL bStopCreateThread;

  */
  //  virtual BOOL wpRestoreState(ULONG ulReserved);
  //virtual BOOL wpSaveState();	
  virtual HWND wpOpen(HWND hwndCnr,ULONG ulView,ULONG ulParam);
  virtual BOOL wpMenuItemSelected(HWND hwndFrame,ULONG ulMenuId);
  virtual ULONG AddCDTypeOptionPage(HWND hwndNotebook);
  virtual void cwWriteOnTheFly();
#pragma SOMReleaseOrder()
#pragma SOMIDLPass(CWDVDFolder,"Implementation-End","dllname = aucdfldr;")
};














