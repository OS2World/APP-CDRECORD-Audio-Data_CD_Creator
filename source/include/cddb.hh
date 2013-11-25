/*
 * This file is (C) Chris Wohlgemuth 1999
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


/* The track class holds the information about
   every track of a CD. The tracks are linked
   and the whole list is put into the cddb class */
class clsTrack;

class clsTrack
{
public:
  clsTrack(char * title,int trackNo);
  char trackname[1000];
  int iTrackNo;
  clsTrack *nextTrack;
};

/*****************************************************/

/* This class holds the CDDB info of a CD. It
   allows linking to support several matches. After
   letting the user choose one match the missing
   tracknames are filled with queried information from
   the CDDB database*/ 

class cddb;
 
class cddb
{
public:
  char title[100];
  char artist[100];
  char category[20];
  LONG discid;

  cddb(char * chrTitle,char *chrArtist, char *category, int iMatch);
  ~cddb();
  int newTrack(char * trackTitle, int trackNo);
  clsTrack * cddbFirstTrack(){return firstTrack;};
  clsTrack * cddbNextTrack(clsTrack * track);
  void linkCddb(cddb * Cddb);
  cddb * getNextCddb(){return nextCddb;};
  int getFuzzyOrN() {return iFuzzy;};
  clsTrack* cddbFindTrack(int iTrack);
private:
  int iFuzzy;
  cddb* nextCddb;
  clsTrack* firstTrack;
  void addTrack(clsTrack * track);
};






