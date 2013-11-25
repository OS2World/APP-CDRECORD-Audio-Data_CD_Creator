/*
 *    storewindow.cpp: creating of an invisible child window of
 *                     the WPS folder which holds important data
 *                     like the object handle and frame control
 *                     information.
 *
 *             (C) Chris Wohlgenuth 1999
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
#include <stdlib.h>
#include "progfolder.h"
#include "progfolder.hh"


MRESULT EXPENTRY storeWindowProc (HWND hwnd,ULONG msg, MPARAM mp1,MPARAM mp2)
{
  if( msg==WM_DESTROY)
    free(WinQueryWindowPtr(hwnd,QWP_FCTRLDATA));
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

HWND createStoreWindow(HWND hwndCLF)
{
  static BOOL bStoreWindowRegistered=FALSE;
  
  if(!bStoreWindowRegistered) {
    if(WinRegisterClass(WinQueryAnchorBlock(HWND_DESKTOP),"FolderStoreWindow",&storeWindowProc,0,NUM_STOREWINDOWBYTES))
      {
        bStoreWindowRegistered=TRUE;
      }else
        return NULL;
  }
  return WinCreateWindow(hwndCLF,"FolderStoreWindow","",0,
                         0,0,0,0,0,HWND_TOP,ID_FOLDERSTOREWINDOW,NULL,0);
}




