/*
 * progfolder.h: Definitions for class CWProgFolder
 *
 *             (C) Chris Wohlgenuth 1999
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
#define NUM_STOREWINDOWBYTES 12
#define QWP_ORGFRAMEPROC 4 /* address of the original frame proc */
#define QWP_FCTRLDATA 8 /* ptr to the private framectrl data */
/* 0: this-ptr.
   4: original frame proc
   8: ptr. to framectrl data */

#define ID_FOLDERSTOREWINDOW         1200

#define ID_FRAMELEFT         1000
#define ID_FRAMERIGHT         1001
#define ID_FRAMETOP         1002
#define ID_FRAMEBOTTOM         1003

#define FCTL_LEFT   1
#define FCTL_RIGHT  2
#define FCTL_TOP    4
#define FCTL_BOTTOM 8

#define FCTL_POSABOVE  1
#define FCTL_POSBELOW  2

#define ID_FOLDERCLASSNAME 100

#define IDPT_HAND       6000
