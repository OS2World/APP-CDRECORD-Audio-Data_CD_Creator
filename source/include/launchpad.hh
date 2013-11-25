/*
 * This file is (C) Chris Wohlgemuth 2001
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

#ifndef LAUNCHPAD_H_INCLUDED
#define LAUNCHPAD_H_INCLUDED

#pragma SOMAsDefault(on)
#include <som.hh>
#include <somcls.hh>
#pragma SOMNoMangling(on)

#include "wpfolder.hh"

#pragma SOMAsDefault(off) 

class LPObject;
class launchPad;
class LPList;

class LPList
{
public:
  LPList *lplNext;
  launchPad * lpPad;
  void lplRefreshAllPads();
};

class LPObject
{
public:
  LPObject(HPOINTER hptr);
  ~LPObject(void);
  LPObject* lpoNext;
  WPObject *wpObject;
  HOBJECT  hObject;
  HPOINTER hPtr;
  char chrName[CCHMAXPATH];
  launchPad* lpParent;
};

class launchPad
{
public:
  launchPad(HWND hwndParent,HWND hwndOwner, BOOL bVisible, WPFolder *wpParent, LPList** lpList, HWND hwndPrev, ULONG ulFl);
  ~launchPad();
  BOOL lpFillPad();
  BOOL lpClearPad();
  BOOL lpAddButton(WPObject* wpObject, int iAfter);
  BOOL lpRemoveButton(LPObject *lpObject);
  void lpRefreshLaunchPad();
  // WPObject* lpQueryButtonObject(USHORT usButtonID);
  WPFolder* lpQueryParentFolder();
  //BOOL lpShowLaunchPad(BOOL bShow);
  BOOL lpSetLaunchPadPos(HWND hwndInsertBehind, LONG x, LONG y, LONG cx, LONG cy, ULONG fl);
  BOOL lpSetConfiguration(char * chrTarget, char * chrConfigID_);
  void lpFreeObjectList();
  virtual BOOL lpReBuildObjectList();
  ULONG lpQueryNumObjects(void) {return ulNumObjects;};
  BOOL  lpSetFlyOverText(char* chrText);
  PSZ  lpQueryFlyOverText(void);
private:
  HWND hwndLaunchPad; /* This is the frame window handle of the launchpad */
  WPFolder *wpParentFolder;
  LPObject* lpoObjectList;
  ULONG ulNumObjects;
  HWND  hwndPrevious;
  LPList** lpList;
  ULONG fl;
  char chrConfigTarget[CCHMAXPATH];
  char chrConfigID[CCHMAXPATH];
  char chrFlyOverText[250];/* Text for fly over help */
  virtual BOOL lpBuildObjectList(char * chrFolderID);
  virtual BOOL lpSaveObjectList();
};

#pragma SOMAsDefault(pop) 

#define LP_USEOBJECTASPARAM  0x00000001
#define LP_NODROPONPAD       0x00000002

#endif
