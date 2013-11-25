:h2 res=2800.Cdrdao/2 for audio writing setup
:p.
Provide the path to cdrdao/2 on this page if you want to create audio CDs in 
:link reftype=hd res=2802.DAO:elink. mode.

:nt.
On the page 2 you can specify the necessary cdrdao/2 settings for the CD1&colon.1-copy 
source, and on page 3 the cdrdao/2 settings for the CD1&colon.1-copy target, so you are 
able to use different drivers and options, you may even use different cdrdao/2 builds.
:ent.
:p.
Select your writer using the drop down listbox. If your writer isn't listed you may choose a generic driver.
Many writers are using the same driver so chances are good that your device will work with one
of these generic drivers. If your Recorder is not listed try one of the following according to the descriptions.

:sl.
:li.:hp2.plextor:ehp2. Supports CD structure analysis (command read-toc) for Plextor CD-ROM readers. Pre-gaps and index marks
are detected by performing a binary search over the Q sub-channel for each track. This method is fast and gives
very exact results on Plextor drives. The driver uses generic SCSI commands (PLAY AUDIO and READ
SUB-CHANNEL) so it may work on other models, too, but result and speed is directly correlated to the digital
audio extraction capabilities. 

:li.:hp2.plextor-scan:ehp2. This is an alternate driver for CD structure analysis with Plextor drives. It scans the Q sub-channels of a track
linearly like the generic-mmc driver but is faster on Plextor drives. 

:li.:hp2.cdd2600:ehp2. This is a native driver for the Philips CDD2000/CDD2600 drive family. read-toc is implemented like in the
plextor driver but it is slow and not very exact due to poor digital audio extraction capabilities of these drives.
Anyway, I don't recommend doing CD structure analysis with the CDD2x00 because it stresses the mechanism too
much. 

:li.:hp2.generic-mmc:ehp2. This is a driver for SCSI-3/mmc compatible CD-recorders that support session-at-once (cue sheet based) writing.
Most of the recent drives should be compatible with this or with the generic-mmc-raw driver described below.
CD-TEXT writing is supported with CD-TEXT capable recorders. read-toc scans linearly the Q sub-channel of
each track to retrieve the pre-gap length and index marks. This method is very accurate but takes the same time
like digital audio extraction. It should also work with recent CD-ROM readers. 

:li.:hp2.generic-mmc-raw:ehp2. This is an alternate driver for SCSI-3/mmc compatible CD-recorders. It uses the raw writing interface where the 
driver has to provide the PQ sub-channel data in addition to the audio data. This writing mode allows using part of
the lead-out area for audio data since the drive's firmware has no chance to prevent this. Of course, you will get
some error message at the end of writing when the CD-R capacity is exhausted.
CD structure analysis is done like in the generic-mmc driver. 

:li.:hp2.ricoh-mp6200:ehp2. Supports writing with the Ricoh MP6200S CD recorder. It cannot write the Catalog Number, yet.
CD structure analysis is done like in the generic-mmc driver. 

:li.:hp2.sony-cdu920:ehp2. Driver for the Sony CDU920 recorder. Audio and data tracks are supported. The Sony CDU948 recorder will 
work with this driver, too. Use option --speed 0 to get the full writing speed.
read-toc uses the Q sub-channel scanning method. 

:li.:hp2.sony-cdu948:ehp2. Driver for the Sony CDU948 recorder. It extends the sony-cdu920 driver by CD-TEXT writing and multi
session mode. 

:li.:hp2.taiyo-yuden:ehp2. This is an adapted cdd2600 driver for Taiyo-Yuden recorders. 'read-toc' is done with the plextor method. 

:li.:hp2.teac-cdr55:ehp2. Driver for the Teac CD-R55 recorder. Audio and data tracks are supported. read-toc uses the Q sub-channel 
scanning method. 

:li.:hp2.yamaha-cdr10x:ehp2. Driver for the Yamaha CDR10X recorder series that supports audio and data tracks. read-toc uses the Q 
sub-channel scanning method. 
:esl.

:note.If you're uncertain about the driver to use do some tests on the command line first. It's no good idea to
experiment using a front end when something isn't working at all. If you've found a working driver you may send
a mail to me describing your writer and the driver so I can update the list. A complete list of supported drives 
is in tutorial.inf chapter 'List of supported CD-Writers'.
:sl.
:li.:link reftype=hd res=2801.Options:elink.
:esl.

:h3 res=2801.Options
:p.
Enter arbitrary options into these entryfields to customize your 
CDRDAO/2 commandline. Options supported by your drive can be found in tutorial.inf 
chapter 'List of supported CD-Writers'.
:p.


.********** Normal mode without path selection ************
:h2 res=2850.Cdrdao/2 for audio writing setup
:p.
Cdrdao/2 is used to create audio CDs in 
:link reftype=hd res=2802.DAO:elink. mode.

:nt.
On the page 2 you can specify the necessary cdrdao/2 settings for the CD1&colon.1-copy 
source, and on page 3 the cdrdao/2 settings for the CD1&colon.1-copy target, so you are 
able to use different drivers and options, you may even use different cdrdao/2 builds.
:ent.
:p.
Select your writer using the drop down listbox. If your writer isn't listed you may choose a generic driver.
Many writers are using the same driver so chances are good that your device will work with one
of these generic drivers. If your Recorder is not listed try one of the following according to the descriptions.

:sl.
:li.:hp2.plextor:ehp2. Supports CD structure analysis (command read-toc) for Plextor CD-ROM readers. Pre-gaps and index marks
are detected by performing a binary search over the Q sub-channel for each track. This method is fast and gives
very exact results on Plextor drives. The driver uses generic SCSI commands (PLAY AUDIO and READ
SUB-CHANNEL) so it may work on other models, too, but result and speed is directly correlated to the digital
audio extraction capabilities. 

:li.:hp2.plextor-scan:ehp2. This is an alternate driver for CD structure analysis with Plextor drives. It scans the Q sub-channels of a track
linearly like the generic-mmc driver but is faster on Plextor drives. 

:li.:hp2.cdd2600:ehp2. This is a native driver for the Philips CDD2000/CDD2600 drive family. read-toc is implemented like in the
plextor driver but it is slow and not very exact due to poor digital audio extraction capabilities of these drives.
Anyway, I don't recommend doing CD structure analysis with the CDD2x00 because it stresses the mechanism too
much. 

:li.:hp2.generic-mmc:ehp2. This is a driver for SCSI-3/mmc compatible CD-recorders that support session-at-once (cue sheet based) writing.
Most of the recent drives should be compatible with this or with the generic-mmc-raw driver described below.
CD-TEXT writing is supported with CD-TEXT capable recorders. read-toc scans linearly the Q sub-channel of
each track to retrieve the pre-gap length and index marks. This method is very accurate but takes the same time
like digital audio extraction. It should also work with recent CD-ROM readers. 

:li.:hp2.generic-mmc-raw:ehp2. This is an alternate driver for SCSI-3/mmc compatible CD-recorders. It uses the raw writing interface where the 
driver has to provide the PQ sub-channel data in addition to the audio data. This writing mode allows using part of
the lead-out area for audio data since the drive's firmware has no chance to prevent this. Of course, you will get
some error message at the end of writing when the CD-R capacity is exhausted.
CD structure analysis is done like in the generic-mmc driver. 

:li.:hp2.ricoh-mp6200:ehp2. Supports writing with the Ricoh MP6200S CD recorder. It cannot write the Catalog Number, yet.
CD structure analysis is done like in the generic-mmc driver. 

:li.:hp2.sony-cdu920:ehp2. Driver for the Sony CDU920 recorder. Audio and data tracks are supported. The Sony CDU948 recorder will 
work with this driver, too. Use option --speed 0 to get the full writing speed.
read-toc uses the Q sub-channel scanning method. 

:li.:hp2.sony-cdu948:ehp2. Driver for the Sony CDU948 recorder. It extends the sony-cdu920 driver by CD-TEXT writing and multi
session mode. 

:li.:hp2.taiyo-yuden:ehp2. This is an adapted cdd2600 driver for Taiyo-Yuden recorders. 'read-toc' is done with the plextor method. 

:li.:hp2.teac-cdr55:ehp2. Driver for the Teac CD-R55 recorder. Audio and data tracks are supported. read-toc uses the Q sub-channel 
scanning method. 

:li.:hp2.yamaha-cdr10x:ehp2. Driver for the Yamaha CDR10X recorder series that supports audio and data tracks. read-toc uses the Q 
sub-channel scanning method. 
:esl.

:note.If you're uncertain about the driver to use do some tests on the command line first. It's no good idea to
experiment using a front end when something isn't working at all. If you've found a working driver you may send
a mail to me describing your writer and the driver so I can update the list. A complete list of supported drives 
is in tutorial.inf chapter 'List of supported CD-Writers'.
:sl.
:li.:link reftype=hd res=2801.Options:elink.
:esl.




:h3 res=2802.DAO
:p.
DAO is an abbreviation for disk at once. This means the CD is written from the beginning to the end
without any interruption. You have to provide all the data at once. The great advantage is that you've full
control over track markers, indices, pregaps and so on. Only using DAO it's possible to write audio CDs
without a pause between every track.
:p.
TAO which means track at once is the other mode when writing CDs. In TAO mode you write
one track containing some data (e.g. an audio track) at a time. You may stop writing after each track.
The drawback of TAO is that you have a two second pause after each track. This means for audio CDs
that you can't write several tracks to a disk without two second silence between them.




:h2 res=2830.Cdrdao/2 for source device
:p.
Specify settings for the 1&colon.1 CD-copy source device on this page. There're different versions
of cdrdao available which often support different drives. You may use different cdrdao
releases for reading a CD and for writing a CD. Same goes for the driver settings and the options.
This way you may tweak your settings to fit your needs.
:p.
Select your reading device using the drop down listbox. If your device isn't listed you may choose a generic driver.
Many writers are using the same driver so chances are good that your device will work with one
of these generic drivers. If your recorder is not listed try one of the drivers according to the descriptions
given on :link reftype=hd res=2800.cdrdao page 1:elink..
:p.
:sl.
:li.:link reftype=hd res=2801.Options:elink.
:esl.

:h2 res=2860.Cdrdao/2 for target device
:p.
Specify settings for the 1&colon.1 CD-copy target device on this page. There're different versions
of cdrdao available which often support different drives. You may use different cdrdao
releases for reading a CD and for writing a CD. Same goes for the driver settings and the options.
This way you may tweak your settings to fit your needs.
:p.
Select your writer using the drop down listbox. If your writer isn't listed you may choose a generic driver.
Many writers are using the same driver so chances are good that your device will work with one
of these generic drivers. If your recorder is not listed try one of the drivers according to the descriptions
given on :link reftype=hd res=2800.cdrdao page 1:elink..
:p.
:sl.
:li.:link reftype=hd res=2801.Options:elink.
:esl.





