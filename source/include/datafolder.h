/* This file is (C) Chris Wohlgemuth 2000
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

#ifndef __DATAFOLDER_H
#define __DATAFOLDER_H

#define PATHLISTFILE "temp\\filelist" /* Extension will be added */
#define SHAREDMEMFILE "temp\\data" /* Extension will be added */

/* Struct which holds device data */
typedef struct {
  char chrDev[20];
  int iSpeed;
}DEVICEINFO;


#endif 
