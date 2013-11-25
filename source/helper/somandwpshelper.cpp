/*
 * This file is (C) Chris Wohlgemuth 2002
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
#define INCL_DOSERRORS
#define INCL_GPIBITMAPS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datafolder.h"
#include "audiofolder.hh"
#include "audiofolderhelp.h"


extern GLOBALDATA globalData;

/* For calling the method in the CWAudio class. Linking to the DLL would require the
   new audio classes. By dynamically querying the method the media folder works without
   installing the audio classes. */  
typedef ULONG   SOMLINK somTP_CWAudio_cwmmQueryTrackInfo(CWAudio *somSelf,
		char** chrString,
		ULONG ulSize,
		int iWhich);
typedef somTP_CWAudio_cwmmQueryTrackInfo *somTD_CWAudio_cwmmQueryTrackInfo;

//somTD_CWAudio_cwmmQueryTrackInfo methodPtr=NULL;

SOMClass* cwGetSomClass(char* chrClassName)
{
  somId    mySomId;
  SOMClass *somClass;

  if((mySomId=somIdFromString(chrClassName))==NULLHANDLE)
    return NULLHANDLE;

  //  somClass=SOMClassMgrObject->somClassFromId(mySomId);
  somClass=SOMClassMgrObject->somFindClass(mySomId, 1, 1);
  SOMFree(mySomId);

  return somClass;
}

BOOL somObjectIsA(WPObject* wpObject, char * chrClassName)
{
  return wpObject->somIsA( cwGetSomClass(chrClassName));
}

BOOL cwmmQueryCWAudioTrackInfoMethodPtr(/*CWMediaFolder *thisPtr*/)
{
  somId    mySomId;
  SOMClass *mmAudioClass;

  if(!globalData.cwmmQueryTrackInfoMthdPtr) {
    /* If we don't have the method pointer yet, get it. */
    /* Get class object of CWAudio. */
    mmAudioClass=cwGetSomClass("CWAudio");
    if(somIsObj(mmAudioClass)) {
      /* We have the class object */
      mySomId=somIdFromString("cwmmQueryTrackInfo");/* A new method in CWAudio (replacement for MMAudio) */ 
      globalData.cwmmQueryTrackInfoMthdPtr=(somTD_CWAudio_cwmmQueryTrackInfo) mmAudioClass->somFindSMethod(mySomId);
      SOMFree(mySomId);
      if(globalData.cwmmQueryTrackInfoMthdPtr)
        return TRUE;
    }
    mmAudioClass=cwGetSomClass("MMAudio");
    if(somIsObj(mmAudioClass)) {
      /* We have the class object */
      mySomId=somIdFromString("cwmmQueryTrackInfo");/* A new method in MMAudio since V0.2.4 of the MM classes */ 
      globalData.cwmmQueryTrackInfoMthdPtr=(somTD_CWAudio_cwmmQueryTrackInfo) mmAudioClass->somFindSMethod(mySomId);
      SOMFree(mySomId);
      if(globalData.cwmmQueryTrackInfoMthdPtr)
        return TRUE;
    }

    return FALSE;
  }
  else
    return TRUE;
}


SOMClass* queryMMAudioClass(void)
{
  somId mySomId;
  static SOMClass *mAudioClass=NULL;

  if(!mAudioClass) {
    if((mySomId=somIdFromString("MMAudio"))!=NULLHANDLE) {
      mAudioClass=(SOMClass*)SOMClassMgrObject->somClassFromId(mySomId);
      SOMFree(mySomId);
    }
  }
  return mAudioClass;
}

SOMClass* queryCWAudioClass(void)
{
  somId mySomId;
  static SOMClass *mAudioClass=NULL;

  if(!mAudioClass) {
    if((mySomId=somIdFromString("CWAudio"))!=NULLHANDLE) {
      mAudioClass=(SOMClass*)SOMClassMgrObject->somClassFromId(mySomId);
      SOMFree(mySomId);
    }
  }
  return mAudioClass;
}

