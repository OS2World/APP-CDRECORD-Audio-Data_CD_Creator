/*
 *
 * Include file with class definitions for class CWProgFolder
 *
 *             (C) Chris Wohlgenuth 1999-2002
 *
 * This class is intended for programmers and provides functions for some
 * common tasks like:
 *                     - Adding dialogs as frame controls to WPS folders
 *                     - Showing 'About'-dialogs
 *                     - Checking of object pointers
 *                     - ...
 *
 * Use IBM VAC++ V3.00 and Gnu-make 3.74.1 to compile
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
#define INCL_DOS
#define INCL_WIN
#include <os2.h>

/* Structure which holds information about our
   frame controls */
typedef struct {
    HWND hwndCtlLeft;
    BOOL bLeft;
    ULONG ulFlagsLeft;
    SIZEL sizelLeftFrame;

    HWND hwndCtlRight;
    BOOL bRight;
    ULONG ulFlagsRight;
    SIZEL sizelRightFrame;

    HWND hwndCtlTop;
    BOOL bTop;
    ULONG ulFlagsTop;
    SIZEL sizelTopFrame;

    HWND hwndCtlBottom;
    BOOL bBottom;
    ULONG ulFlagsBottom;
    SIZEL sizelBottomFrame;

  BOOL bCnrAdjusted;
}FRAMECTRLDATA;

/* Some stuff necessary for the compiler */
#pragma SOMAsDefault(on)
#include "som.hh"
#include "somcls.hh"
#pragma SOMNoMangling(on)
/* Include file with definition for WPFolder class */
#include "WPFOLDER.hh"
/* Include file with definition for WPShadow class */
#include "wpshadow.hh"



/* Class definition for the Metaclass of our new folder class */
class M_CWProgFolder:public M_WPFolder
{
#pragma SOMClassName(*,"M_CWProgFolder")
  /* Minimal SOM-Version we need. No change necessary */
#pragma SOMClassVersion(*,1,3)
#pragma SOMCallstyle(oidl)
public:
  /* The member functions we want to override */
  virtual PSZ  wpclsQueryTitle();
  virtual ULONG wpclsQueryStyle();
  virtual void wpclsInitData();
  /* This statement describes new memberfunctions if we introduce them */
#pragma SOMReleaseOrder()
};

/* The new folder class. It's a child of WPFolder-class */
class CWProgFolder:public WPFolder
{
  /* The class name */
#pragma SOMClassName(*,"CWProgFolder")
  /* The name of the class' Metaclass */
#pragma SOMMetaClass(CWProgFolder,"M_CWProgFolder")
  /* SOM must be Version 1.2 or above */
#pragma SOMClassVersion(CWProgFolder,1,3)
#pragma SOMCallstyle(oidl)
private:
  /* We have no private data or functions */
public:
  /* Overridden or new functions come here */
  /* This one is overridden */
    virtual HWND wpOpen(HWND hwndCnr,ULONG ulView,ULONG ulParam);
    virtual BOOL cwClose(HWND hwndFrame);
  /* This one is a new one */
    virtual ULONG cwNumAdditionalFrameControls(HWND hwndFolder);
    virtual BOOL cwCalculateClient(HWND hwndFolder, PRECTL pRectl, BOOL which);
    virtual ULONG cwFormatFrame(HWND hwndFolder, USHORT countSWP,PSWP pswp);
    virtual BOOL cwAddFrameCtl(HWND hwndFolder,HWND hwndNewCtl,SIZEL sizel, ULONG ulPos, ULONG ulFlags);
    virtual HWND cwQueryFrameCtl(HWND hwndFolder, SIZEL* sizel, ULONG ulPos, ULONG * ulFlags);
    virtual void cwUpdateFrame(HWND hwndFolder);
    virtual WPObject* cwGetFileSystemObject(WPObject* wpObject);
    virtual ULONG cwShowAboutDlg(HMODULE hModule, ULONG idDialog);
    virtual BOOL cwQueryCDDrives(int *iNumCD, char * cFirstDrive);
  virtual ULONG cwInsertUserMenuItems(HWND hwndMenu, char* chrMenuFolder, ULONG ulFirstId, BOOL bLeadingSeparator);
  virtual BOOL cwCheckUserMenuItems( char* chrMenuFolder, USHORT usItemId);

    //  virtual ULONG cwQueryAudioCDInfo(char * drive);
  /* Tell SOM the order of the new functions. NEVER change the order in
     future versions or you break binary compatibility! */
#pragma SOMReleaseOrder( \
"cwNumAdditionalFrameControls",\
"cwCalculateClient",\
"cwFormatFrame",\
"cwAddFrameCtl",\
"cwQueryFrameCtl",\
"cwUpdateFrame",\
"cwGetFileSystemObject",\
"cwShowAboutDlg",\
"cwQueryCDDrives",\
"cwClose",\
"cwInsertUserMenuItems",\
"cwCheckUserMenuItems")
};



