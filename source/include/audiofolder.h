/* This file is (C) Chris Wohlgemuth 1999-2004
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
#include "audiofolderres.h"

/* The creator homepage */
#define CD_CREATOR_HOME_URL  "http://www.os2world.com/cdwriting"

/* These IDs are used to know, which grabber we have */
#define IDGRABBER_CDDA2WAV   1 
#define IDGRABBER_LEECH      2
#define IDGRABBER_JCDREAD    3
#define IDGRABBER_UNKNOWN    99

/* ID keys for mp3 decoder to use. Used for playtime querying and writing. */
#define IDKEY_USEMPG123         1
#define IDKEY_USEZ              2
#define IDKEY_USEMMIOMP3        3

/* ID for MP3 encoding qualitiy settings */
#define IDQUALITY_VBRSTANDARD       1
#define IDQUALITY_VBREXTREME        2
#define IDQUALITY_AVERAGE           3
#define IDQUALITY_USERDEFINED       4
/* Default bitrate to be used when no setting in INI found */
#define DEFAULT_MP3_BITRATE         128
#define DEFAULT_MP3_ENCODEPRIORITY  1 /* LAME supports 0...4 */

/* ID for MP3 naming scheme */
#define IDMP3NAMING_NONE       0
#define IDMP3NAMING_ALBUM      1
#define IDMP3NAMING_ARTIST     2
#define IDMP3NAMING_TITLE      3
#define IDMP3NAMING_TRACK      4

#define NUM_MP3NAMES           5
#define MP3NAME_LEN           20
#define NUM_MP3NAMES           5
#define MP3NAME_FILLSTRING_LEN           10
#define NUM_MP3NAME_FILLSTRINGS 3

/* How many parts the MP3 name has. */
#define NUM_MP3NAMEPARTS      4

/* The driver we should preselect if the writer is unknown */
#define DEFAULT_CDRDAO_DRIVER " (driver) generic-mmc"

/* Audio shadow class name */
#define SHADOW_CLASS_NAME   "CWAudioShadow"

/* Folder IDs for the toolbars. (Remove eventually when using WPS-wizard) */
#define DATACD_TBNAME    "<DATACD_TOOLBAR>"
#define AUDIOCD_TBNAME    "<AUDIOCD_TOOLBAR>"

/* IDs of ISOFS programs */
#define ID_ISOFS_MOUNT    "<ISOFS_MOUNT>"
#define ID_ISOFS_UNMOUNT  "<ISOFS_UNMOUNT>"

/* IDs of the templates */
/* If changed the setup string in wpclsCreateDefaultTemplates() must also be changed! */
#define ID_AUDIOTEMPLATE   "<AUDIOCD_CREATOR>" 
#define ID_DATATEMPLATE   "<DATACD_CREATOR>"
#define ID_DVDTEMPLATE   "<DATADVD_CREATOR>"

#define AUDIO_TEMPLATE_NAME "Create Audio-CD"
#define DATA_TEMPLATE_NAME  "Create Data-CD"
#define DATADVD_TEMPLATE_NAME  "Create Data-DVD"

#define ID_CREATORSETTINGS "<CWCREATOR_SETTINGS>"

/* The name of the DRDialog runtime file */
#define DRDIALOG_RUNTIME "DRRexx.exe"

#define DEFAULT_WAVENAME  "Cddb.wav" /* Name if CDDB-Query fails and user didn't provide one */

/* The name of the PG>Pro command file which creates the audio cover */
#define COVER_NAME           "back.cwx"

#define CREATOR_LOGFILE      "creator.log"
#define LOGDIR_NAME          "logfiles"

/* The path to the hint database (installation directory will be prepended) */
#define HINTDATABASEPATH    "\\Help\\hint.ini"
#define DAYTIPDATABASEPATH    "\\Help\\daytip.ini"

/* Fonts to use in dialogs */
#define DEFAULT_DIALOG_FONT   "9.WarpSans"
#define DEFAULT_DIALOG_FONT_WARP3   "8.Helv"

/* Flyover default settings. To be removed later...  */
#define MAXDELAY            9999       /* Delay for toolbar flyover */
#define DEFAULTDELAY        250

/* Fly over window class. This class was registered by WPS-wizard and gives
   transparency and shadows. */
#define WC_FLYOVERCLIENT      "wizFlyOverClient"

/* Shared mem names */
#define AUDIOSHAREDMEM_SIZE    8192
#define SHAREDMEM_SIZE 8192 /* Max size of mkisofs cmd line */
#define SCANBUSSHAREDMEM_SIZE 4096
#define SCANBUSSHAREDMEM_NAME "\\SHAREMEM\\SCANBUS"

/* This is the max length of the only filename given on the cmd-line if
    deep shadows are used. The target name used on CD is included in
    the length. Used as size for char xxx[] */
#define CMDFILENAMELENGTH      CCHMAXPATH*3

/* The following is for use with WPS-Wizard if installed  */
/* The first user menuitem ID */
#define FIRSTUSERITEM        0x7200
/* Setup string to hide the class page */
#define SETUP_HIDECLASSPAGE  "HIDECLASSPAGE"
/* The ID for the data configuration folder */
#define USERMENUFOLDER_DATA "<USERMENU_DATAFOLDER>"
/* The ID for the audio configuration folder */
#define USERMENUFOLDER_AUDIO "<USERMENU_AUDIOFOLDER>"
/* The ID for the data DVD configuration folder */
#define USERMENUFOLDER_DATADVD "<USERMENU_DATADVDFOLDER>"

/* Flags used for global data */
#define GLOBALFLAG_DAYTIPSHOWN           0x00000001
#define GLOBALFLAG_EXPERTMODE            0x00000002
#define GLOBALFLAG_MMIOMP3INSTALLED     0x00000004

/* Global flags saved in cdrecord.ini as one ulong. App: CDWriter, Key: options */
#define IDCDR_HIDEWINDOW   0x0001
#define IDCDR_CLOSEWINDOW 0x0002
#define IDCDR_CREATESHADOWS 0x0004
#define IDCDR_NOEJECT       0x0008
#define IDCDR_SONYMULTISESSION     0x0010
#define IDCDR_BURNPROOF     0x0020

/* Key for save/restore of write flags (instance data) for audio folders */
#define IDKEY_FDRWRITEFLAGS        4000
/* write flags (Audio) saved in the instance data of the folder */
#define IDWF_PAD     0x0001
#define IDWF_NOFIX   0x0002
#define IDWF_PREEMP  0x0004
#define IDWF_DUMMY  0x0008
#define IDWF_DAO  0x0010
#define FLAG_ALREADYSIZED 0x1000

/* PREEMP is not supported anymore V0.54 */
//#define WF_ALLFLAGS (IDWF_PAD|IDWF_NOFIX|IDWF_DUMMY|IDWF_PREEMP|IDWF_DAO|FLAG_ALREADYSIZED)
#define WF_ALLFLAGS (IDWF_PAD|IDWF_NOFIX|IDWF_DUMMY|IDWF_DAO|FLAG_ALREADYSIZED)

/* Global flags saved in cdrecord.ini for helper windows */
/*#define IDMK_HIDEWINDOW   0x0001
#define IDMK_CLOSEWINDOW 0x0002
*/

/* Key for save/restore of mkisofs flags (instance data) for data folders */
#define IDKEY_MKISOFSFLAGS        5000
/* mkisofs flags (data) saved in the instance data of the folder */
#define IDMK_ALLOW32CHARS     0x0001 /* obsolete */
#define IDMK_LEADINGPERIODS   0x0002 /* obsolete */
#define IDMK_JOLIET  0x0004
#define IDMK_ROCKRIDGE  0x0008 /* obsolete, is default */
#define IDMK_TRANSTABLE  0x0010
#define IDMK_LOGFILE  0x0020
#define IDMK_ALLFILES  0x0040 /* obsolete */
#define IDMK_SHADOWSINROOTONLY  0x0080
#define IDMK_USEARCHIVEBIT      0x0100
#define IDMK_RESETARCHIVEBIT    0x0200
#define MK_ALLFLAGS (IDMK_ALLOW32CHARS|IDMK_LEADINGPERIODS|IDMK_JOLIET|IDMK_ROCKRIDGE|IDMK_TRANSTABLE|IDMK_LOGFILE | IDMK_ALLFILES | IDMK_SHADOWSINROOTONLY | IDMK_USEARCHIVEBIT | IDMK_RESETARCHIVEBIT)

/* Key for save/restore of create flags (instance data) */
#define IDKEY_CREATEFLAGS        5500
/* Create flags saved in the instance data */
#define IDCR_IMAGEONLY   0x0001
#define IDCR_WRITEIMAGE  0x0002
#define IDCR_ONTHEFLY    0x0004
#define IDCR_TESTONLY    0x0008 /* Same as IDWF_DUMMY for audio folders */

/* Key for save/restore of cdtype flags */
#define IDKEY_CDTYPEFLAGS        6000
/* CD-type flags */
#define IDCDT_MULTISESSION     0x0001
//#define IDCDT_FIRSTSESSION     0x0002
#define IDCDT_USEWRITER        0x0004
#define IDCDT_TRACKDATA        0x0008
#define IDCDT_TRACKMODE2       0x0010
#define IDCDT_TRACKXA1         0x0020
#define IDCDT_TRACKXA2         0x0040
#define IDCDT_TRACKCDI         0x0080
#define IDCDT_ALLTRACKTYPES        (IDCDT_TRACKDATA|IDCDT_TRACKMODE2|IDCDT_TRACKXA1|IDCDT_TRACKXA2|IDCDT_TRACKCDI)
//#define IDCDT_MERGESESSION     0x0100
#define IDCDT_FIXDISK          0x0200
#define IDCDT_BOOTCD           0x0400
#define IDCDT_USERDEFINED      0x0800
#define CDT_ALLFLAGS (IDCDT_MULTISESSION|IDCDT_USEWRITER|IDCDT_TRACKDATA|IDCDT_TRACKMODE2| \
                      IDCDT_TRACKXA1|IDCDT_TRACKXA2|IDCDT_TRACKCDI | IDCDT_FIXDISK|IDCDT_BOOTCD|IDCDT_USERDEFINED)
/*
//#define CDT_ALLFLAGS (IDCDT_MULTISESSION|IDCDT_FIRSTSESSION|IDCDT_USEWRITER|IDCDT_TRACKDATA|IDCDT_TRACKMODE2| \
//                      IDCDT_TRACKXA1|IDCDT_TRACKXA2|IDCDT_TRACKCDI | IDCDT_MERGESESSION | IDCDT_FIXDISK|IDCDT_BOOTCD|IDCDT_USERDEFINED)
*/
/* Keys for CD-Author */
#define IDKEY_APPLICATION       6100
#define IDKEY_PUBLISHER         6101
#define IDKEY_PREPARER          6102
#define IDKEY_COPYRIGHT         6103
#define IDKEY_VOLUMENAME       6104

/* Keys for boot CD */
#define IDKEY_BOOTIMAGE         6110
#define IDKEY_BOOTCATALOG       6111

/* Key for image name */
#define IDKEY_IMAGENAME         6120

/* Private window messages */
#define WM_STARTGRABMSG         "WM_STARTGRAB"
#define WM_UPDATESTATUSBARMSG   "WM_UPDATESTATUSBAR"
#define WM_STARTWRITEMSG        "WM_STARTWRITE"

#define NAMEFIELDLENGTH 100

/* Keys for CWMMAudio */
#define IDKEY_DISCTITLE          6200
#define IDKEY_TRACKTITLE         6201
#define IDKEY_ARTIST             6202
#define IDKEY_GENRE              6203
#define IDKEY_DISCID             6204

/* Action keys. Which helper should be started or */
/* Which helper sent the WM_APPTERMINATE msg */
#define ACKEY_IMAGEONLY         0
#define ACKEY_WRITEONLY         1
#define ACKEY_ONTHEFLY          2
#define ACKEY_PRINTSIZE         3
#define ACKEY_CDSIZE            4
#define ACKEY_WRITEAUDIO        5
#define ACKEY_PLAYTIME          6
#define ACKEY_MP3DECODE         7
#define ACKEY_SCANBUS           8
#define ACKEY_CREATECOVER       9
#define ACKEY_CDDBQUERY         10
#define ACKEY_MP3INFO           11
#define ACKEY_DVDSIZE           12

#define ACKEY_ABORT             20
#define ACKEY_MBWRITTEN         21
#define ACKEY_FIXATING          22
#define ACKEY_LEADIN            23
#define ACKEY_LEADOUT           24
#define ACKEY_LISTBOX           25
#define ACKEY_FREESHAREDMEM     26
#define ACKEY_PERCENTGRABBED    27
#define ACKEY_MAXWRITESPEED     28  /* Sent from scanbus.exe to set the write speed in the settings page. */
#define ACKEY_BURNSAVE          29  /* Sent from scanbus.exe to inform the settings page about burnproof. */

/* Feature bits in list box item handles of settings page */
#define LBHANDLE_FEATURE_NOBURNPROOF     0x8000

/* Struct which holds data for thread */
typedef struct {
  PVOID thisPtr;
  int iWhich;
}THREADINFO;

/* Struct which holds this ptr for Dialog */
typedef struct {
  SHORT usSize;
  PVOID thisPtr;
}THISPTR;

/* Setupstrings for settings object */
#define SETUP_DISPLAYHELPPANEL      "DISPLAYHELPPANEL"
#define SETUP_DISPLAYHELPPANELEXT   "DISPLAYHELPPANELEXT" /* Similar to previous but helplibrary may be given */

/* This one is also valid for data folders. Data folders will add their specific settings. */
#define SETUP_WRITECONFIGTOFILE     "WRITECONFIGTOFILE"

#define SETUP_CDRECORDPATH          "CDRECORDPATH"
#define SETUP_CDRECORDOPTIONSA      "CDRECORDOPTIONSA"
#define SETUP_CDRECORDOPTIONSD      "CDRECORDOPTIONSD"

#define SETUP_MKISOFSPATH           "MKISOFSPATH"
#define SETUP_MKISOFSOPTIONS        "MKISOFSOPTIONS"

#define SETUP_CDRDAOPATH            "CDRDAOPATH"
#define SETUP_CDRDAOOPTIONS         "CDRDAOOPTIONS"
#define SETUP_CDRDAOPATH2           "CDRDAOPATH2" /* Source for 1:1 */
#define SETUP_CDRDAOOPTIONS2        "CDRDAOOPTIONS2"
#define SETUP_CDRDAOPATH3           "CDRDAOPATH3" /* Target for 1:1 */
#define SETUP_CDRDAOOPTIONS3        "CDRDAOOPTIONS3"

#define SETUP_GRABBERPATH           "GRABBERPATH"
#define SETUP_GRABBEROPTIONS        "GRABBEROPTIONS"
#define SETUP_GRABBERSELECT         "GRABBERSELECT"

#define SETUP_MP3DECPATH            "MP3DECPATH"
#define SETUP_MP3SELECT             "MP3SELECT"
#define SETUP_MP3SWAPBYTES          "MP3SWAPBYTES"

#define SETUP_MP3ENCODERPATH         "MP3ENCODERPATH"
#define SETUP_MP3ENCODEROPTIONS     "MP3ENCODEROPTIONS"
#define SETUP_MP3ENCODERPRIORITY    "MP3ENCODERPRIORITY"
#define SETUP_MP3ENCODERQUALITY    "MP3ENCODERQUALITY"
#define SETUP_MP3ENCODERBITRATE    "MP3ENCODERBITRATE"

#define SETUP_MUSICLIBRARYPATH         "MUSICLIBRARYPATH"

#define SETUP_FLDRHINTENABLE        "FLDRHINTENABLE"
#define SETUP_DAYTIPENABLE          "DAYTIPENABLE"

#define SETUP_OPENCDRTOOLS          "OPENCDRTOOLS"

#define SETUP_FIFOSIZE              "FIFOSIZE"
#define SETUP_WRITESPEED                 "WRITESPEED"

#define SETUP_TOOLBARFLYOVERENABLE         "TBFLYOVERENABLE"
#define SETUP_TOOLBARFLYOVERDELAY         "TBFLYOVERDELAY"

#define SETUP_INSTLANGUAGEDLL                 "INSTLANGUAGEDLL"

/* Setup strings for Data-CD-folders */
#define DATAFLDRSETUP_FILENAMEOPTIONS       "FLDRFILENAMEOPTIONS"
#define DATAFLDRSETUP_CDTYPEOPTIONS         "FLDRCDTYPEOPTIONS"
#define DATAFLDRSETUP_FOLLOWALLSHADOWS      "FOLLOWALLSHADOWS"
#define DATAFLDRSETUP_FLDRCREATEOPTIONS     "FLDRCREATEOPTIONS"
#define DATAFLDRSETUP_VOLUMENAME            "VOLUMENAME"
#define DATAFLDRSETUP_APPLICATION           "APPLICATION"
#define DATAFLDRSETUP_PUBLISHER             "PUBLISHER"
#define DATAFLDRSETUP_PREPARER              "PREPARER"
#define DATAFLDRSETUP_IMAGENAME             "IMAGENAME"
/* This is sent when on the fly writing ended */
#define DATAFLDRSETUP_ONTHEFLYDONE          "ONTHEFLYDONE"
#define DATAFLDRSETUP_FREESHAREDMEM         "FREESHAREDMEM"
#define DATAFLDRSETUP_SETSTATUSTEXT         "SETSTATUSTEXT"

/* Setup strings for Audio-CD-folders */
#define AUDIOFLDRSETUP_SHOWABOUTDIALOG          "SHOWABOUTDIALOG"
#define AUDIOFLDRSETUP_SHOWABOUTDIALOG_COMPLETE "SHOWABOUTDIALOG=1"
#define AUDIOFLDRSETUP_FREESHAREDMEM             DATAFLDRSETUP_FREESHAREDMEM
#define AUDIOFLDRSETUP_SETSTATUSTEXT             DATAFLDRSETUP_SETSTATUSTEXT
#define AUDIOFLDRSETUP_FLDRWRITEFLAGS            "FLDRWRITEFLAGS"
/* This one is sent from the FreeDB helper to set the filenames in the grab listbox */
#define AUDIOFLDRSETUP_SETTRACKLBTEXT            "SETTRACKLBTEXT"

//#define AUDIOFLDSETUP_SHOWABOUTDIALOG_COMPLETE   AUDIOFLDRSETUP_SHOWABOUTDIALOG_COMPLETE
//#define AUDIOFLDSETUP_SHOWABOUTDIALOG            AUDIOFLDRSETUP_SHOWABOUTDIALOG

#ifdef INCL_GPIBITMAPS
/* Structs for custom bitmaps */
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

/* Indizes into array of custom bitmaps */
#define  CTRLIDX_BG                0
#define  CTRLIDX_CHECK             1
#define  CTRLIDX_UNCHECK           2
#define  CTRLIDX_RADCHECK          3
#define  CTRLIDX_RADUNCHECK        4
#define  CTRLIDX_RADMASK           5
#define  CTRLIDX_UNCHECKSEL        6
#define  CTRLIDX_RADUNCHECKSEL     7
#define  CTRLIDX_RADCHECKSEL       8
#define  CTRLIDX_RADMASKSEL        9

#define  NUM_CTRL_IDX              10

#define  CTRLIDX_POSSLIDER         1
#define  CTRLIDX_POSSLIDERARM      2
#define  CTRLIDX_VOLSLIDER         3
#define  CTRLIDX_VOLSLIDERARM      4
#define  CTRLIDX_VOLSLIDERARMSEL   5
#define  CTRLIDX_PLAYTIME          6
#define  CTRLIDX_TOPLEFT           7
#define  CTRLIDX_TOPRIGHT          8
#define  CTRLIDX_PLAYBUTTON        9
#define  CTRLIDX_PLAYBUTTONSEL     10
#define  CTRLIDX_STOPBUTTON        11
#define  CTRLIDX_STOPBUTTONSEL     12
#define  CTRLIDX_PAUSEBUTTON       13
#define  CTRLIDX_PAUSEBUTTONSEL    14
#define  CTRLIDX_SKIPBACKBUTTON    15
#define  CTRLIDX_SKIPBACKBUTTONSEL 16
#define  CTRLIDX_SKIPFWDBUTTON     17
#define  CTRLIDX_SKIPFWDBUTTONSEL  18
#define  CTRLIDX_BOTTOM            19



/* for cwmmQueryTrackInfo() */
/* ID tag defines */
#define IDINFO_NAME              1
#define IDINFO_ARTIST            2
#define IDINFO_ALBUM             3
#define IDINFO_YEAR              4
#define IDINFO_COMMENT           5
#define IDINFO_GENRE             6
#define IDINFO_PLAYTIME          7
#define IDINFO_BPS               8
#define IDINFO_SAMPLERATE        9
#define IDINFO_CHANNELS          10

#define IDINFO_LASTINFO          10
