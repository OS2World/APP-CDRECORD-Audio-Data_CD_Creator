/*
 * This file is (C) Chris Wohlgemuth 1999/2000
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

#include <os2.h>
#include <stdio.h> /* For stderr */
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>

#include "cddb.h"
#include "cddb.hh"

#define CDDB_OK   0
#define CDDB_MORE 1
#define CDDB_ERROR -1

#define verbose 1

#define QUERYCDDB_VERSION "0.12"
#define QUERYCDDB_NAME "QueryCddb-OS2"

extern int h_errno;
extern int errno;
LONG extern CDDBDiscID(char * drive,CDDBINFO *cddbInfo);

int port=888;
char host[100]="www.freedb.org";
char username[100]={0};
char hostname[100]={0};
char clientname[100]={0};
char version[100]= {0};


/**********************************************************/
/* CDDB Stuff */

extern int readResponse(int s, char * buffer,int size);
/* Returns the cddb code. */
extern int get_cddb_code( char * chrResponse);
/* Check error codes coming from the server*/
extern int check_cddb_code(char * chrResponse);
/* Performing the handshake procedure with the CDDB server */
/* s: socket handle */
extern int cddb_handshaking(int s);
/* s: socket handle */
extern int cddb_setting_proto(int s);
/* This is called if the CDDB database has got several matches for our
   discid. We take all matches and decide later, which to use. */
extern cddb * query_get_all_matches(int s);
/* Query some data */
/* With the data given we later query the
   tracknames */
extern cddb * cddb_query(int s, CDDBINFO *cddbInfo);
/* Reads the CDDB banner from the server after connecting */
/* We don't really need it so it is only shown on stderr */
extern int cddb_banner(int s);
/* Ask for the information about our CD. Parse it and
   save it in the data file. */
extern int read_and_parse(int s,cddb * Cddb);
extern cddb_read(int s,cddb * Cddb);
extern int cddbConnectToHost(int * skt);

/**********************************************************/
/* Some helper functions
 */

void extern printError(char* errorString);
void extern printErrorCR(char* errorString);
void extern printData(char* outString);

void banner()
{
  fprintf(stderr,"CDDB-Query (C) Chris Wohlgemuth 2000-2001\n");
  fprintf(stderr,"This program is released under the GPL (Gnu Public Licence)\n");
  fprintf(stderr,"See file COPYING for further information\n\n");
}

void usage(char * progName)
{
  char buffer[200];

  snprintf(buffer, sizeof(buffer), "usage:  %s <drive> <user> <userhost> <cddb-host> <port>\n\n", progName);
  printError(buffer);
  snprintf(buffer, sizeof(buffer), "e.g.    %s o: dilbert company.com www.freedb.org 888\n",progName);
  printError(buffer);
}

printCddbList(cddb * Cddb)
{
  cddb * tmpCddb;
  char text[400];
  int iNum;

  tmpCddb=Cddb;
  iNum=0;
  if(verbose) {
    printError("\n-------------------\n");
    printError("CDDB database returned the following match(es):\n");
  }
  while (tmpCddb) {/* Print data of every match */
    /* Print our data */
    iNum++;
    if(verbose) {
      sprintf(text,"%d.:\n   Title: %s\n   Artist: %s\n"/*Category: %s\nDiscid: %x\n"*/
              ,iNum,tmpCddb->title,tmpCddb->artist/*,tmpCddb->category,tmpCddb->discid*/);
      printError(text);
    }
    tmpCddb=tmpCddb->getNextCddb();
  }/* while */
  if(verbose) {
    printError("-------------------\n");
  }
}

cddb* choseCddb(cddb* Cddb)
{
  int iNum;
  int choosen;
  cddb * tmpCddb;
  char buffer[10]={0};
  char * rc;
  /* Temp!!!!!*/
  char text[400];

  iNum=0;
  tmpCddb=Cddb;
  /* Count the matches */
  while (tmpCddb) {
    iNum++;
    tmpCddb=tmpCddb->getNextCddb();
  }/* while */
  if(iNum==1) {
    printError("If you want to choose this unexact match enter '1' otherwise enter '-1'\n");
  }
  else {
    printError("Enter the Number of the match to choose or '-1' to abort\n");
  }

  rc=NULL;
  while(rc==NULL) {
    fprintf(stderr,"> ");
    rc=fgets(buffer,10,stdin);
    choosen=atoi(buffer);
    if(choosen>iNum||choosen==0||choosen<-1) {
      rc=NULL;
      sprintf(buffer,"");
    }
  }/* while */

  if(choosen==-1)
    return NULL;

  iNum=1;
  tmpCddb=Cddb;
  /* Get the cddb */
  while (iNum!=choosen) {
    iNum++;
    tmpCddb=tmpCddb->getNextCddb();
  }/* while */

  return tmpCddb;
}

void printQueriedData(cddb * Cddb,CDDBINFO * cddbInfo)
{
  clsTrack * tempTrack;
  int  trackNr;

    /* Print Tracks to screen or log */
    /* The data is already written to the data
       file. This was done in cddb_read()
       ( to be accurate: in read_and_parse()
       called by cddb_read()). */
    tempTrack=Cddb->cddbFirstTrack();
    if(Cddb->getFuzzyOrN()) {
      printf("DiscID of this CD: %x\n", cddbInfo->discid);
      printf("Choosen DiscID: %x\n", Cddb->discid);
    }
    else
      printf("DiscID: %x\n", Cddb->discid);
    printf("Artist: %s\n", Cddb->artist);
    printf("Title: %s\n", Cddb->title);
    printf("Category: %s\n\n", Cddb->category);
    if(!tempTrack)
      return;/* Hey, where's the track item?! */
    trackNr=0;
    while(tempTrack) {
      trackNr++;
      printf("Track%02d: %s\n",trackNr, tempTrack->trackname);
      tempTrack=Cddb->cddbNextTrack(tempTrack);
    }
}

/******************************************************/

int main(int argc,char* argv[])
{
 
  int s,a;
  char buffer[512];
  int rc;
  CDDBINFO cddbInfo;
  cddb * Cddb;
  clsTrack * tempTrack;


  /* Show our copyright */
  banner();

  if(argc!=6) {
    usage(argv[0]);
    return -1;
  }  

  strncpy(clientname,QUERYCDDB_NAME ,sizeof(clientname));
  strncpy(version,QUERYCDDB_VERSION,sizeof(version));


  strncpy(username, argv[2], sizeof(username));
  strncpy(hostname, argv[3], sizeof(hostname));
  strncpy(host,argv[4], sizeof(host));
  port=atoi(argv[5]);

  /* Get track data for CDDB query.
     This function calculates the diskID and the
     length of the tracks of the inserted CD.
     We use the data to perform the database query. */
  if(!CDDBDiscID(argv[1],&cddbInfo)) {
    printError("Cannot query track data for calculating diskID.\nMake sure an audio CD is inserted." );
    return -1;
  }


  /* Insert here: query different hosts */

  /* Connect to cddb-host */
  if(cddbConnectToHost(&s))
    return -1;
      
  /* Query the database */
  do {
    /* Getting CDDB banner */
    if(cddb_banner(s))
      break; /* Error */

    /* Setting protocol level */
    if(cddb_setting_proto(s))
      break;
    
    /* Handshaking */
    if(cddb_handshaking(s))
      break;
    
    /* Query disc */
    /* If there're several matches this call returns 
       a linked list of all matches. We let the user
       decide, which match to use. */ 
    Cddb=cddb_query(s, &cddbInfo);
    if(!Cddb)
      break; /* no list, so break */

    /* Print disc */
    /* We print all matches, we found */
    if(Cddb)
      printCddbList(Cddb);     
    
    /* Let the user decide which match to use */
    /* If the head of the list has iFuzzy set
       to one we have to choose. */
    if(Cddb->getFuzzyOrN())
      Cddb=choseCddb(Cddb);

    if(!Cddb)
      break; /* no choice, so break */

    /* Query tracks of match 'Cddb' */
    /* The track names are put into a linked
       list in Cddb. */
    if(cddb_read(s,Cddb)==CDDB_ERROR)
      break;
      
    /* Print the queried data */
    printQueriedData(Cddb, &cddbInfo);
    
    /* everything's ok */
    write(s,"QUIT\r\n",6);
    close(s);
    delete(Cddb);
    return 0;  
  } while(TRUE);
  if(Cddb)
    delete(Cddb);
    /* Error */
  write(s,"QUIT\r\n",6);
  close(s);
  return -1;
}



