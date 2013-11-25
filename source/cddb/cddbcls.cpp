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

#include <os2.h>
#include <stdio.h> /* For stderr */
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

extern int h_errno;
extern int errno;
LONG extern CDDBDiscID(char * drive,CDDBINFO *cddbInfo);

extern int port;
extern char host[100];
extern char username[100];
extern char hostname[100];
extern char clientname[100];
extern char version[100];

/* You may set this in another module to a valid handle
   and the error text is written to this window (WinSetWindowText())
   This isn't threadsafe so be sure about what you're doing!! */

HWND hwndError=NULL;

/*****************************************************/

clsTrack::clsTrack(char * title,int trackNo)
{
  strncpy(trackname,title,sizeof(trackname));
  iTrackNo=trackNo;
  nextTrack=0;
}
/*****************************************************/




/* If iFuz==0 then we have one exact match. If iFuz==1
   we have one of n exact matches or a fuzzy match. */
cddb::cddb(char * chrTitle,char *chrArtist, char *category, int iFuz=0)
{
  firstTrack=0;
  nextCddb=0;
  iFuzzy=iFuz;
  strncpy(title,chrTitle,sizeof(title));
  strncpy(artist,chrArtist,sizeof(artist));

  return;
}

cddb::~cddb()
{
  clsTrack * tempTrack;

  while(!firstTrack) {
    tempTrack=firstTrack;
    firstTrack=firstTrack->nextTrack;
    delete(tempTrack);
  }
  
  if(nextCddb)delete nextCddb;

  return;
}

/* Method to link different matches together */
void cddb::linkCddb(cddb * Cddb)
{
  /* The new cddb goes to the end of the list */
  if(nextCddb){
    nextCddb->linkCddb(Cddb);
    return;
  }
  nextCddb=Cddb;
  return;
}

int cddb::newTrack(char * trackTitle, int trackNo)
{
  clsTrack * tempTrack;

  tempTrack=new clsTrack(trackTitle, trackNo);
  if(!tempTrack)return 1;
  addTrack(tempTrack);
  return 0;
}

void cddb::addTrack(clsTrack * track)
{
  clsTrack * tempTrack;

  if(!firstTrack) {
    firstTrack=track;
    return;
  }

  tempTrack=firstTrack;
  while(tempTrack->nextTrack) {
    tempTrack=tempTrack->nextTrack;  
  }
  tempTrack->nextTrack=track;
  return;  
}

clsTrack * cddb::cddbNextTrack(clsTrack * track)
{
  if(!track)return 0;
  return track->nextTrack;
}

/**********************************************************/
/* Some helper functions
 */
void printError(char* errorString)
{
  char buffer[CCHMAXPATH];
  char *ptr;

  strncpy(buffer, errorString, sizeof(buffer));
  fprintf(stderr,"%s",errorString);
  /* hwndError is global, so this isn't threadsafe */
  if(hwndError) {
    ptr=strchr(buffer,'\n');
    if(ptr) {
      *ptr=0;
    }
    WinSetWindowText(hwndError, buffer);
  }
}

void printErrorCR(char* errorString)
{
  fprintf(stderr,"%s\n",errorString);
}


void printData(char* outString)
{
  int fileHandle;
  int rc;

  fileHandle=open("cddb.out",O_APPEND|O_TEXT|O_WRONLY);
  if(fileHandle==-1) {
    printError("Cannot open cddb.out");
    return;
  }
  rc= write(fileHandle,outString,strlen(outString));
  // write(fileHandle,"\n",sizeof("\n"));
  close(fileHandle);
  return;
}


/**********************************************************/
/* CDDB Stuff */

int readResponse(int s, char * buffer,int size)
{ 
  int a;
  int rc;
  
  rc=1;
  for(a=0;a<size&&rc>0;a++) {
    rc=read(s,&buffer[a],1);
    if(buffer[a]=='\n') {
      buffer[a]=0;
      return CDDB_MORE;
    }
    if(rc==-1)
      return CDDB_ERROR;    
  }
   
  return CDDB_ERROR;
}

/* Returns the cddb code. */
int get_cddb_code( char * chrResponse)
{
  return atoi(chrResponse);
}

/* Check error codes coming from the server*/
int check_cddb_code(char * chrResponse)
{
  switch (chrResponse[0])
    {
    case '2': return CDDB_OK;
    case '3': return CDDB_MORE;
    case '4':
      printError("CDDB server error");
      return CDDB_ERROR;
    case '5':
      printError("CDDB client error");
      return CDDB_ERROR;
    default:
      printError("Unknown error");
      break;
    }
  return CDDB_ERROR;
}

/* Performing the handshake procedure with the CDDB server */
/* s: socket handle */
int cddb_handshaking(int s)
{
  char buffer[512];
  int rc;

  /* Handshaking */
  if(verbose)
    printError("Handshaking...    ");
  sprintf(buffer,"cddb hello %s %s %s %s\n",username,hostname,clientname,version);
  rc=write(s,buffer,strlen(buffer));
  if(rc==-1) {
    printError("Handshaking: cannot write to CDDB server");
    return -1;
  }
  if(verbose)
    printError("done\n");
  if(verbose==2)
    printError("Response: ");
  
  rc=readResponse(s,buffer,sizeof(buffer));
  if(rc==CDDB_ERROR) {
    printError("Handshaking: cannot get response");
    return -1;
  }
  if(verbose==2) 
    printErrorCR(buffer);
 
  if(check_cddb_code(buffer)==CDDB_ERROR)
    return -1;

  return 0;
}

/* s: socket handle */
int cddb_setting_proto(int s)
{
  char buffer[512];
  int rc;
  int iProtLevel;

  iProtLevel=4;/* Startlevel */
  /* Setting protocol level */
  do {
    if(verbose)
      printError("\nSetting protocol level...    ");
    sprintf(buffer,"proto %d\n",iProtLevel);
    printError(buffer);
    rc=write(s,buffer,strlen(buffer));
    if(rc==-1) {
      printError("Protocol: cannot write to CDDB server");
      return -1;
    }
    if(verbose)
      printError("done\n");
    if(verbose==2)
      printError("Response: ");
    
    rc=readResponse(s,buffer,sizeof(buffer));
    if(rc==CDDB_ERROR) {
      printError("Set protocol: cannot get response");
      return -1;
    }
    if(verbose==2)
      printErrorCR(buffer);
    
    /* Check cddb code */
    if(check_cddb_code(buffer)==CDDB_ERROR) {
      if(get_cddb_code(buffer)!=501)
        return -1;/* Unknown error */
    }
    else
      break;
    iProtLevel--;      
  }while(iProtLevel>0);
  
  return 0;
}

/* This is called if the CDDB database has got several matches for our
   discid. We take all matches and decide later, which to use. */
cddb * query_get_all_matches(int s)
{
  cddb * root=0;
  cddb * Cddb=0;
  int rc;
  char buffer[512];

  do {
    rc=readResponse(s,buffer,sizeof(buffer)); /* Get next line */
    
    if(rc==CDDB_ERROR)
      return root; /* return what we have so far */

    if(buffer[0]=='.') /* End of data */
      return root; /* return what we have */

    /* A new cddb instance for every match */
    Cddb=new cddb("","","",1); /* One fuzzy match or one of several matches */
    /* Get the data for this match */
    sscanf(buffer, " %20s %x %100[^/\r\n] / %100[^\r\n]", Cddb->category,&Cddb->discid,Cddb->artist,Cddb->title);
    /* Link the cddb instance into the list */
    if(!root)
      root=Cddb;
    else
      root->linkCddb(Cddb);
    
  }while(true);

}

/* Query some data */
/* With the data given we later query the
   tracknames */
cddb * cddb_query(int s, CDDBINFO *cddbInfo)
{
  char buffer[512]={0};
  char track[20];
  char title[100];
  char artist[100];
  char categ[20];
  int rc;
  int code;
  int a;
  cddb * Cddb;
  cddb * Cddb2;
  char * chrPtr;

  /* Sending query */
  if(verbose)
    printError("Sending query cmd...    ");
  /* Build cmd-line */
  sprintf(buffer,"cddb query %08x %d",cddbInfo->discid,cddbInfo->numTracks);
  for(a=0;a<cddbInfo->numTracks;a++) {
    sprintf(track," %d",cddbInfo->startSector[a]);
    strcat(buffer,track);
  }
  sprintf(track," %d\n",cddbInfo->discLength);
  strcat(buffer,track);
  if(verbose==2)
    printErrorCR(buffer);
  rc=write(s,buffer,strlen(buffer));
  if(rc==-1) {
    printError("Query: cannot write to CDDB server");
    return 0;
  }
  if(verbose)
    printError(" done\n");
  if(verbose==2)
    printError("Response: ");
  
  rc=readResponse(s,buffer,sizeof(buffer));
  //rc=read(s,buffer,sizeof(buffer)-1);
  if(rc==CDDB_ERROR) {
    printError("Query: cannot get response");
    return 0;
  }
  if(verbose==2) {
    printErrorCR(buffer);
  }
  if(check_cddb_code(buffer)==CDDB_ERROR)
    return 0;
  code=get_cddb_code(buffer);

  /* Check output */
  switch(code)
    {
    case 200:  /* exact match */
      Cddb=new cddb("","","");
      /*sscanf(buffer+4, "%20s %x %100[^ ] / %100[^\r\n]", Cddb->category,&Cddb->discid,Cddb->artist,Cddb->title);      
       */ 
      sscanf(buffer+4, "%20s %x %100[^/\r\n] / %100[^\r\n]", Cddb->category,&Cddb->discid,Cddb->artist,Cddb->title);
      /* printData(buffer);
         printData(buffer+4); */
      return Cddb;
      break;
    case 210: /* several exact matches */
    case 211: /* several inexact matches */
      return query_get_all_matches(s); /* if there's an error this call returns 0 */
      break;
    case 202:
    default:
      /* We shouldn't reach this */
      return 0;
    }
  /* We shouldn't reach this */
  return 0;
}  

/* Reads the CDDB banner from the server after connecting */
/* We don't really need it so it is only shown on stderr */
int cddb_banner(int s)
{
  char buffer[512];
  int rc;

  /* Get CDDB-Banner */
  if(verbose)
    printErrorCR("Querying CDDB-Banner...");
  rc=readResponse(s,buffer,sizeof(buffer));
  if(rc==CDDB_ERROR) {
    printError("Cannot read CDDB-Banner");
    return -1;
  }
  if(verbose==2)
    printErrorCR(buffer);

  if(check_cddb_code(buffer)==CDDB_ERROR)
    return -1;
  return 0;
}

/* Ask for the information about our CD. Parse it and
   save it in the data file. */
int read_and_parse(int s,cddb * Cddb)
{
  int rc;
  char buffer[512];
  char key[20];
  char trackName[100];
  int iTrack;
  
  do{
    rc=readResponse(s,buffer,sizeof(buffer));
    if(rc==CDDB_ERROR) {
      printError("Read request: cannot get response");
      return CDDB_ERROR;
    }
    /* Print the data to the outfile */
    printData(buffer);
    if(buffer[0]=='#') /* This is a comment */
      { }
      // printf("Found Comment: %s\n",buffer);
    else{
      if(buffer[0]=='.') /* End of data */
        return CDDB_OK; /* Done */
      //printf("Found data: %s\n",buffer);
      sscanf(buffer,"%20[^0-9=]",key);
      /* We put track names into a linked list in the cddb instance */
      if(!strcmp(strupr(key),"TTITLE")) {
        iTrack=0;
        /* We have a disc title */
        sscanf(buffer,"%20[^0-9]%d=%100[^\n\r]",key,iTrack,trackName);
        Cddb->newTrack(trackName,iTrack); /* New track entry in cddb instance */
      }
    }
  }while(rc==CDDB_MORE);

}

cddb_read(int s,cddb * Cddb)
{
  char buffer[512];
  int rc;

  /* Sending read */

  /* Build cmd-line */
  sprintf(buffer,"cddb read %s %08x\n",Cddb->category,Cddb->discid);  
  if(verbose) 
    printError("\nSending read cmd... ");
  if(verbose==2)
    printError(buffer);
  
  rc=write(s,buffer,strlen(buffer));
  if(rc==-1) {
    printError("Read request: cannot write to CDDB server");
    return -1;
  }
  if(verbose) 
    printError("done\n");
  if(verbose==2)
    printError("Response: ");
  
  rc=readResponse(s,buffer,sizeof(buffer));
  if(rc==CDDB_ERROR) {
    printError("Read request: cannot get response");
    return -1;
  }
  if(verbose==2) 
    printError(buffer);
  if(check_cddb_code(buffer)==CDDB_ERROR)
    return CDDB_ERROR;

  /* Get the data */  
  read_and_parse(s,Cddb);
  return CDDB_OK;  
}

/****************************************************/


int cddbConnectToHost(int * skt) {

  struct hostent *he;
  struct sockaddr_in sa_in;
  char buffer[512];
  int s;

  /* Creating socket */
  if(verbose)
    printError("Creating socket...     ");
  s=socket(AF_INET,SOCK_STREAM,0);
  if(s==-1) {
    printError("Cannot create socket!");
    return -1;
  }
  *skt=s;
  if(verbose)
    printError("Socket created\n");

  /* Querying host */
  if(verbose)
    printError("Querying host...     ");
  he = gethostbyname(host);
  if(he==NULL) {
    printError("Cannot get host address.\ngethostbyname() returned error:  ");
    switch (h_errno)
      {
      case HOST_NOT_FOUND:
        printError("Host not found\n");
        break;
      case TRY_AGAIN:
        printError("Temporary error. Try again later.\n");
        break;
      case NO_DATA:
        printError("Name valid but name server has no associated address.\n");
        break;
      case NO_RECOVERY:
        printError("Unrecoverable error.\n");
        break;
      default:
        printError("Unknown error\n");
      }
    /* Close the socket */
    close(s);
    return -1;
  }   

  if(verbose)
    printError("Host found.\n");


  /* Connecting to host */
  if(verbose)
    printError("Connecting to host...    ");
  memset(&sa_in,0, sizeof(struct sockaddr_in));

  sa_in.sin_family=AF_INET;
  sa_in.sin_port=htons((unsigned short)port);
  memcpy(&sa_in.sin_addr, he->h_addr, he->h_length);

  if(connect(s,(struct sockaddr*)&sa_in,sizeof(struct sockaddr_in))==-1) {
    sprintf(buffer,"errno: %d\n",errno);
    printError(buffer);
    switch (errno)
      {
      case EBADF:
        printError("No valid socket descriptor");
        break;
      case EISCONN:
        printError("The socket is already connected\n");
        break;
      case ETIMEDOUT:
        printError("No connection. Timed out.\n");
        break;
      case ECONNREFUSED:
        printError("Connection refused by  host.\n");
        break;
      case EADDRNOTAVAIL:
        printError("Address not avaiable.\n");
        break;

      default:
        printError("General failure: Cannot connect to host\n");
      }
    close(s);
    return -1;
  }
  if(verbose)
    printError("connected.\n");
  return 0;
}



