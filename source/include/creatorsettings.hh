/*
 * This file is (C) Chris Wohlgemuth 1999-2002
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

#include "wppgm.hh"

class M_CWCreatorSettings:public M_WPAbstract
{
#pragma SOMClassName(*,"M_CWCreatorSettings")
#pragma SOMClassVersion(*,1,2)
#pragma SOMCallstyle(oidl)
#pragma SOMIDLPass(*,"Implementation-End","    dllname = aucdfldr;")
public:
  virtual BOOL wpclsQueryDefaultHelp(PULONG HelpPanelId,PSZ HelpLibrary);
  virtual ULONG wpclsQueryStyle();
  virtual PSZ  wpclsQueryTitle();
  virtual ULONG wpclsQueryIconData(PICONINFO pIconInfo);
  virtual void wpclsInitData();
  virtual void wpclsUnInitData();
#pragma SOMReleaseOrder()
#pragma SOMIDLPass(*,"Implementation-End","dllname = aucdfldr;")
};

class CWCreatorSettings:public WPAbstract
{
#pragma SOMClassName(*,"CWCreatorSettings")
#pragma SOMMetaClass(*,"M_CWCreatorSettings")
#pragma SOMClassVersion(*,1,2)
#pragma SOMCallstyle(oidl)


private:

public:
  virtual BOOL wpAddSettingsPages(HWND hwndNotebook);
  virtual ULONG wpAddObjectWindowPage(HWND hwndNotebook);
  virtual BOOL wpSetupOnce(PSZ pSetupString);
  virtual BOOL wpSaveState();	
  virtual BOOL wpMenuItemSelected(HWND hwndFrame,ULONG ulMenuId);
  virtual BOOL wpModifyPopupMenu(HWND hwndMenu, HWND hwndCnr, ULONG ulPosition);
  virtual BOOL cwAddGeneralOptionPage(HWND hwndNotebook);
  virtual BOOL cwAddCdrecordOptionPage(HWND hwndNotebook);
  virtual BOOL cwAddCdrdaoOptionPage(HWND hwndNotebook);
  virtual BOOL cwAddGrabOptionPage(HWND hwndNotebook);
  virtual BOOL cwAddMpg123OptionPage(HWND hwndNotebook);
  virtual BOOL cwAddMkisofsOptionPage(HWND hwndNotebook);
  virtual BOOL cwAddCDDBOptionPage(HWND hwndNotebook);
  virtual BOOL cwAddCDDBOptionPage2(HWND hwndNotebook);
  virtual BOOL wpSetup(PSZ pSetupString);
  virtual BOOL cwAddGeneralOptionPage2(HWND hwndNotebook);
  virtual BOOL cwAddToolbarOptionPage(HWND hwndNotebook);
  virtual BOOL cwAddHintOptionPage(HWND hwndNotebook);
  virtual BOOL cwAddCdrdaoOptionPage2(HWND hwndNotebook);
  virtual BOOL cwAddCdrdaoOptionPage3(HWND hwndNotebook);
  virtual BOOL cwAddMP3EncoderOptionPage(HWND hwndNotebook);
  virtual BOOL cwAddMP3EncoderOptionPage2(HWND hwndNotebook);
  virtual BOOL cwAddAboutPage(HWND hwndNotebook);
  virtual BOOL cwAddDVDDaoOptionPage(HWND hwndNotebook);
  //  virtual BOOL cwAddIsoFSOptionPage(HWND hwndNotebook);
#pragma SOMReleaseOrder(\
  "cwAddGeneralOptionPage",\
  "cwAddCdrecordOptionPage",\
"cwAddCdrdaoOptionPage",\
"cwAddGrabOptionPage",\
"cwAddMpg123OptionPage",\
"cwAddMkisofsOptionPage",\
"cwAddCDDBOptionPage",\
"cwAddCDDBOptionPage2",\
"cwAddGeneralOptionPage2",\
"cwAddToolbarOptionPage",\
"cwAddHintOptionPage",\
"cwAddCdrdaoOptionPage2",\
"cwAddCdrdaoOptionPage3",\
"cwAddMP3EncoderOptionPage",\
"cwAddMP3EncoderOptionPage2",\
"cwAddAboutPage", \
"cwAddDVDDaoOptionPage")
/*,\
  "cwAddIsoFSOptionPage")*/
  /*
    AddFileOptionPage(,\
    AddDiskOptionPage(HWND hwndNotebook))*/
#pragma SOMIDLPass(CWCreatorSettings,"Implementation-End","dllname = aucdfldr;")
};






