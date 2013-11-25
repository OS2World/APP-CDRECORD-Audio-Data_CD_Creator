/* OS/2 RealTime DART Engine for mpg123 (C) 1998 Samuel Audet <guardia@cam.org> */

#define INCL_OS2MM
#define INCL_DOS
#define INCL_VIO
#define INCL_KBD
#include <os2.h>
#include <os2me.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef MPG123_INCLUDED
#include "mpg123.h"
#endif

/* complementary audio parameters */
int numbuffers = 32;     /* total audio buffers, _bare_ minimum = 4 (cuz of prio boost check) */
char *boostprio = NULL;
char *normalprio = NULL;
int lockdevice = FALSE;
ULONG volume = 100;

/* audio buffers */
static ULONG ulMCIBuffers;

static MCI_AMP_OPEN_PARMS  maop = {0};
static MCI_MIXSETUP_PARMS  mmp = {0};
static MCI_BUFFER_PARMS    mbp = {0};
static MCI_GENERIC_PARMS   mgp = {0};
static MCI_SET_PARMS       msp = {0};
static MCI_MIX_BUFFER      *MixBuffers = NULL;

static HEV dataplayed = 0;
static ULONG resetcount;
static BOOL paused = FALSE;

static MCI_MIX_BUFFER *tobefilled,
                      *playingbuffer;

static BOOL nomoredata;

static TIB *mainthread; /* thread info to set thread priority */
static unsigned char boostclass = 3, normalclass = 2;
static signed char   boostdelta = 0, normaldelta = 31;

ULONG keyboardtid;


static LONG APIENTRY DARTEvent(ULONG ulStatus, MCI_MIX_BUFFER *PlayedBuffer, ULONG ulFlags)
{
   switch(ulFlags)
   {
      case MIX_STREAM_ERROR | MIX_WRITE_COMPLETE:  /* error occur in device */

      if ( ulStatus == ERROR_DEVICE_UNDERRUN)
         /* Write buffers to rekick off the amp mixer. */
         mmp.pmixWrite( mmp.ulMixHandle,
                        MixBuffers,
                        ulMCIBuffers );
      break;

   case MIX_WRITE_COMPLETE:                     /* for playback  */

      playingbuffer = (MCI_MIX_BUFFER *) PlayedBuffer->ulUserParm;

      /* if we're about to be short of decoder's data
         (2nd ahead buffer not filled), let's boost its priority! */
      if(tobefilled == (MCI_MIX_BUFFER *)((MCI_MIX_BUFFER *) playingbuffer->ulUserParm)->ulUserParm)
         DosSetPriority(PRTYS_THREAD,boostclass,boostdelta,mainthread->tib_ptib2->tib2_ultid);

      /* just too bad, the decoder fell behind... here we just keep the
         buffer to be filled in front of the playing one so that when the
         decoder kicks back in, we'll hear it in at the right time */
      else if(tobefilled == playingbuffer)
      {
         tobefilled = (MCI_MIX_BUFFER *) playingbuffer->ulUserParm;
         nomoredata = TRUE;
      }

      /* empty the played buffer in case it doesn't get filled back */
      memset(PlayedBuffer->pBuffer,0,PlayedBuffer->ulBufferLength);

      DosPostEventSem(dataplayed);

      mmp.pmixWrite( mmp.ulMixHandle,
                     PlayedBuffer /* will contain new data */,
                     1 );
      break;

   } /* end switch */

   return( TRUE );

} /* end DARTEvent */


static void MciError(ULONG ulError)
{
   unsigned char buffer[128];
   ULONG rc;

   rc = mciGetErrorString(ulError, buffer, sizeof(buffer));

   if (rc == MCIERR_SUCCESS)
      fprintf(stderr,"MCI Error %d: %s\n",ULONG_LOWD(ulError),buffer);
   else
      fprintf(stderr,"MCI Error %d: Cannot query error message.\n",ULONG_LOWD(rc));
}

int audio_set_volume(struct audio_info_struct *ai, ULONG volume)
{
   if(volume > 100) volume = 100;
   if(maop.usDeviceID)
   {
     memset(&msp,0,sizeof(msp));
     msp.ulAudio = MCI_SET_AUDIO_ALL;
     msp.ulLevel = volume;

     mciSendCommand(maop.usDeviceID, MCI_SET,
                    MCI_WAIT | MCI_SET_AUDIO | MCI_SET_VOLUME,
                    &msp, 0);
   }
   return volume;
}

int audio_pause(struct audio_info_struct *ai, int pause)
{
   if(maop.usDeviceID)
   {
      if(pause)
         mciSendCommand(maop.usDeviceID, MCI_PAUSE,
                        MCI_WAIT,
                        &mgp, 0);
      else
         mciSendCommand(maop.usDeviceID, MCI_RESUME,
                        MCI_WAIT,
                        &mgp, 0);
   }
   return pause;
}

int audio_open(struct audio_info_struct *ai)
{
   ULONG rc,i;
   char *temp;
   ULONG openflags;
   PPIB ppib;

   if(maop.usDeviceID) return (maop.usDeviceID);

   if(!ai) return -1;

   if(!ai->device) ai->device = "0";

   if(ai->rate < 0) ai->rate = 44100;
   if(ai->channels < 0) ai->channels = 2;

   /* open the mixer device */
   memset (&maop, 0, sizeof(maop));
   maop.usDeviceID = 0;
   maop.pszDeviceType = (PSZ) MAKEULONG(MCI_DEVTYPE_AUDIO_AMPMIX, atoi(ai->device));

   openflags = MCI_WAIT | MCI_OPEN_TYPE_ID;
   if(!lockdevice) openflags |=  MCI_OPEN_SHAREABLE;

   rc = mciSendCommand(0,
                       MCI_OPEN,
                       openflags,
                       &maop,
                       0);

   if (ULONG_LOWD(rc) != MCIERR_SUCCESS)
   {
      MciError(rc);
      return(-1);
   }

  /* volume in ai->gain ?? */

   /* Set the MCI_MIXSETUP_PARMS data structure to match the audio stream. */

   memset(&mmp, 0, sizeof(mmp));

   ai->format = AUDIO_FORMAT_SIGNED_16;
   mmp.ulBitsPerSample = 16;
   mmp.ulFormatTag = MCI_WAVE_FORMAT_PCM;
   mmp.ulSamplesPerSec = ai->rate;
   mmp.ulChannels = ai->channels;

   /* Setup the mixer for playback of wave data */
   mmp.ulFormatMode = MCI_PLAY;
   mmp.ulDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
   mmp.pmixEvent    = DARTEvent;

   rc = mciSendCommand( maop.usDeviceID,
                        MCI_MIXSETUP,
                        MCI_WAIT | MCI_MIXSETUP_INIT,
                        &mmp,
                        0 );

   if ( ULONG_LOWD(rc) != MCIERR_SUCCESS )
   {
      MciError(rc);
      return(-1);
   }

   volume = audio_set_volume(ai,volume);

   /* Set up the BufferParms data structure and allocate
    * device buffers from the Amp-Mixer  */

   memset(&mbp, 0, sizeof(mbp));
   free(MixBuffers);
   if(numbuffers < 4) numbuffers = 4;
   MixBuffers = calloc(numbuffers, sizeof(*MixBuffers));

   ulMCIBuffers = numbuffers;
   mbp.ulNumBuffers = ulMCIBuffers;
/*   mbp.ulBufferSize = mmp.ulBufferSize; */
   /* I don't like this... they must be smaller than 64KB or else the
      engine needs major rewrite */
   mbp.ulBufferSize = AUDIOBUFSIZE;
   mbp.pBufList = MixBuffers;

   rc = mciSendCommand( maop.usDeviceID,
                        MCI_BUFFER,
                        MCI_WAIT | MCI_ALLOCATE_MEMORY,
                        (PVOID) &mbp,
                        0 );

   if ( ULONG_LOWD(rc) != MCIERR_SUCCESS )
   {
      MciError(rc);
      return(-1);
   }

   ulMCIBuffers = mbp.ulNumBuffers; /* never know! */

   /* Fill all device buffers with zeros and set linked list */

   for(i = 0; i < ulMCIBuffers; i++)
   {
      MixBuffers[i].ulFlags = 0;
      MixBuffers[i].ulUserParm = (ULONG) &MixBuffers[i+1];
      MixBuffers[i].ulBufferLength = mbp.ulBufferSize;
      memset(MixBuffers[i].pBuffer, 0, MixBuffers[i].ulBufferLength);
   }

   MixBuffers[i-1].ulUserParm = (ULONG) &MixBuffers[0];

   /* Create a semaphore to know when data has been played by the DART thread */
   DosCreateEventSem(NULL,&dataplayed,0,FALSE);

   playingbuffer = &MixBuffers[0];
   tobefilled = &MixBuffers[1];
   nomoredata = TRUE;

   if(boostprio)
   {
      temp = alloca(strlen(boostprio)+1);
      strcpy(temp,boostprio);

      boostdelta = atoi(temp+1);
      *(temp+1) = 0;
      boostclass = atoi(temp);
      if(boostclass > 4) boostdelta = 3;
      if(boostdelta > 31) boostdelta = 31;
      if(boostdelta < -31) boostdelta = -31;
   }

   if(normalprio)
   {
      temp = alloca(strlen(normalprio)+1);
      strcpy(temp,normalprio);

      normaldelta = atoi(temp+1);
      *(temp+1) = 0;
      normalclass = atoi(temp);
      if(normalclass > 4) normaldelta = 3;
      if(normaldelta > 31) normaldelta = 31;
      if(normaldelta < -31) normaldelta = -31;
   }

   DosGetInfoBlocks(&mainthread,&ppib); /* ppib not needed, but makes some DOSCALLS.DLL crash */
   DosSetPriority(PRTYS_THREAD,boostclass,boostdelta,mainthread->tib_ptib2->tib2_ultid);

   /* Write buffers to kick off the amp mixer. see DARTEvent() */
   rc = mmp.pmixWrite( mmp.ulMixHandle,
                       MixBuffers,
                       ulMCIBuffers );

   return maop.usDeviceID;
}

int audio_play_samples(struct audio_info_struct *ai,unsigned char *buf,int len)
{
   /* if we're too quick, let's wait */
   while(tobefilled == playingbuffer)
   {
      DosResetEventSem(dataplayed,&resetcount);
      DosWaitEventSem(dataplayed, -1);
   }

   nomoredata = FALSE;

   memcpy(tobefilled->pBuffer, buf, len);
   tobefilled->ulBufferLength = len;

   /* if we're out of the water (3rd ahead buffer filled),
      let's reduce our priority */
   if(tobefilled == (MCI_MIX_BUFFER *)((MCI_MIX_BUFFER *)((MCI_MIX_BUFFER *) playingbuffer->ulUserParm)->ulUserParm)->ulUserParm)
      DosSetPriority(PRTYS_THREAD,normalclass,normaldelta,mainthread->tib_ptib2->tib2_ultid);

   tobefilled = (MCI_MIX_BUFFER *) tobefilled->ulUserParm;

   return len;
}

int audio_close(struct audio_info_struct *ai)
{
   ULONG rc;

   if(!maop.usDeviceID)
      return 0;

   while(!nomoredata)
   {
      DosResetEventSem(dataplayed,&resetcount);
      DosWaitEventSem(dataplayed, -1);
   }
   DosCloseEventSem(dataplayed);
   dataplayed = 0;

   rc = mciSendCommand( maop.usDeviceID,
                        MCI_BUFFER,
                        MCI_WAIT | MCI_DEALLOCATE_MEMORY,
                        &mbp,
                        0 );

   if ( ULONG_LOWD(rc) != MCIERR_SUCCESS )
   {
      MciError(rc);
      return(-1);
   }

   free(MixBuffers);
   MixBuffers = NULL;
   memset(&mbp, 0, sizeof(mbp));

   rc = mciSendCommand( maop.usDeviceID,
                        MCI_CLOSE,
                        MCI_WAIT ,
                        &mgp,
                        0 );

   if ( ULONG_LOWD(rc) != MCIERR_SUCCESS )
   {
      MciError(rc);
      return(-1);
   }

   memset(&mgp, 0, sizeof(mgp));

   memset(&maop, 0, sizeof(maop));

   return 0;
}

/*
 * get formats for specific channel/rate parameters
 */
int audio_get_formats(struct audio_info_struct *ai)
{
   int fmts = 0;
   ULONG rc;
   MCI_MIXSETUP_PARMS  mmptemp = {0};

   mmp.ulDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
   mmp.pmixEvent    = DARTEvent;

   mmptemp.ulFormatMode = MCI_PLAY;
   mmptemp.ulSamplesPerSec = ai->rate;
   mmptemp.ulChannels = ai->channels;

   mmptemp.ulFormatTag = MCI_WAVE_FORMAT_PCM;
   mmptemp.ulBitsPerSample = 16;
   rc = mciSendCommand( maop.usDeviceID,
                        MCI_MIXSETUP,
                        MCI_WAIT | MCI_MIXSETUP_QUERYMODE,
                        &mmptemp,
                        0 );
   if((ULONG_LOWD(rc) == MCIERR_SUCCESS) && (rc != 0x4000)) /* undocumented */
      fmts = fmts | AUDIO_FORMAT_SIGNED_16;

   mmptemp.ulFormatTag = MCI_WAVE_FORMAT_PCM;
   mmptemp.ulBitsPerSample = 8;
   rc = mciSendCommand( maop.usDeviceID,
                        MCI_MIXSETUP,
                        MCI_WAIT | MCI_MIXSETUP_QUERYMODE,
                        &mmptemp,
                        0 );
   if((ULONG_LOWD(rc) == MCIERR_SUCCESS) && (rc != 0x4000)) /* undocumented */
      fmts = fmts | AUDIO_FORMAT_UNSIGNED_8;

   mmptemp.ulFormatTag = MCI_WAVE_FORMAT_ALAW;
   mmptemp.ulBitsPerSample = 8;
   rc = mciSendCommand( maop.usDeviceID,
                        MCI_MIXSETUP,
                        MCI_WAIT | MCI_MIXSETUP_QUERYMODE,
                        &mmptemp,
                        0 );
   if((ULONG_LOWD(rc) == MCIERR_SUCCESS) && (rc != 0x4000)) /* undocumented */
      fmts = fmts | AUDIO_FORMAT_ALAW_8;

   mmptemp.ulFormatTag = MCI_WAVE_FORMAT_MULAW;
   mmptemp.ulBitsPerSample = 8;
   rc = mciSendCommand( maop.usDeviceID,
                        MCI_MIXSETUP,
                        MCI_WAIT | MCI_MIXSETUP_QUERYMODE,
                        &mmptemp,
                        0 );
   if((ULONG_LOWD(rc) == MCIERR_SUCCESS) && (rc != 0x4000)) /* undocumented */
      fmts = fmts | AUDIO_FORMAT_ULAW_8;

   return fmts;
}

int audio_rate_best_match(struct audio_info_struct *ai)
{
   return 0;
}

int audio_get_devices(char *info, int deviceid)
{
   char buffer[128];
   MCI_SYSINFO_PARMS mip;

   if(deviceid && info)
   {
      MCI_SYSINFO_LOGDEVICE mid;

      mip.pszReturn = buffer;
      mip.ulRetSize = sizeof(buffer);
      mip.usDeviceType = MCI_DEVTYPE_AUDIO_AMPMIX;
      mip.ulNumber = deviceid;

      mciSendCommand(0,
                     MCI_SYSINFO,
                     MCI_WAIT | MCI_SYSINFO_INSTALLNAME,
                     &mip,
                     0);

      mip.ulItem = MCI_SYSINFO_QUERY_DRIVER;
      mip.pSysInfoParm = &mid;
      strcpy(mid.szInstallName,buffer);

      mciSendCommand(0,
                     MCI_SYSINFO,
                     MCI_WAIT | MCI_SYSINFO_ITEM,
                     &mip,
                     0);

      strcpy(info,mid.szProductInfo);
      return deviceid;
   }
   else
   {
      int number;

      mip.pszReturn = buffer;
      mip.ulRetSize = sizeof(buffer);
      mip.usDeviceType = MCI_DEVTYPE_AUDIO_AMPMIX;

      mciSendCommand(0,
                     MCI_SYSINFO,
                     MCI_WAIT | MCI_SYSINFO_QUANTITY,
                     &mip,
                     0);

      number = atoi(mip.pszReturn);
      return number;
   }
}

int rew = FALSE,
    fwd = FALSE,
    gotosecs = -1,
    previous = FALSE;

extern int intflag;

static void KeyboardThread(void *arg)
{
/*   struct frame *fr = (struct frame *) arg; */
	KBDKEYINFO key;

   while(key.chChar != 27)
   {
		KbdCharIn(&key,IO_WAIT,0);

      switch(tolower(key.chChar))
		{
         case '-': volume = audio_set_volume(NULL,--volume); break;
         case '+': volume = audio_set_volume(NULL,++volume); break;
         case 'p': paused = audio_pause(NULL,!paused); break;
         case 'j':
            {
               char *scrstr = "Jump to:";
               ULONG row; USHORT column;
               char buffer[128] = {0};
               char zero = 0;

               VioGetCurPos(&row,&column,0);
               column = 50;
               VioWrtCharStr(scrstr, strlen(scrstr), row, column, 0);
               column += strlen(scrstr)+1;
               do
               {
                  KbdCharIn(&key,IO_WAIT,0);
                  switch(key.chChar)
                  {
                     case 13:
                        {
                           char *seperator = strchr(buffer,':');
                           int temp;
                           if(seperator)
                           {
                              temp = atoi(seperator+1);
                              *seperator = '\0';
                              gotosecs = atoi(buffer)*60 + temp;
                           }
                           else
                              gotosecs = atoi(buffer);
                        }
                     case 27:
                        VioWrtNChar(&zero,100,row,50, 0);
                        break;

                     case 8:
                        if(*buffer)
                        {
                           *(strchr(buffer,'\0')-1) = '\0';
                           VioWrtNChar(&zero,1,row,--column, 0);
                        }
                        break;

                     default:
                        *strchr(buffer,'\0') = key.chChar;
                        VioWrtNChar(&key.chChar,1,row,column++, 0);
                        break;
                  }
               }
               while((key.chChar != 13) && (key.chChar != 27));
               key.chChar = 0;
            }
            break;
      }

      switch(key.chScan)
		{
/* left */ case 75: rew = TRUE; break;
/* right */case 77: fwd = TRUE; break;
/* up */   case 72: intflag = TRUE; previous = TRUE; break;
/* down */ case 80: intflag = TRUE; previous = FALSE; break;
      }
	}

   DosExit(EXIT_PROCESS,0);
}


void start_keyboard_thread(struct frame *fr)
{
   fprintf(stderr,"\nEsc: Unconditional termination  Up: Previous Song      Down: Next Song\n<-: Rewind   ->: Fast Forward   J: Jump    -/+: Volume   P: Pause\n");
   keyboardtid = _beginthread(KeyboardThread,0,16384,(void *) fr);
}

