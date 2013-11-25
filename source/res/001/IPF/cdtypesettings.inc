:h2 res=1900.CD-type selection

Select on this page the type of CD you want to create. There is support for singlesession and
multisession CDs.
:p. 
Choose one of the items below to get further information. 
:sl.
:li.:link reftype=hd res=1910.Singlesession:elink.	
:li.:link reftype=hd res=1920.Multisession:elink.
:li.:link reftype=hd res=1940.Boot CD:elink.
:li.:link reftype=hd res=1950.User defined:elink.
:esl. 


:h3 res=1910.Singlesession
:i1.Singlesession
:p.
You can't append any data to a singlesession CD no matter how small the written session was.
If you want to write more data later you have to choose multisession as the CD type for every session
but the last one.


:h3 res=1920.Multisession
:i1.Multisession
:p.
When creating a multisession CD the image is written in a way on the CD which
allows to append another session. For every session written in multisession format about
21MB of additional CD space is needed. 
:p.
Data-CD-Creator automatically detects if you wan't to write the first session or a following
session of a multisession CD. For doing so it's alway necessary to insert the target disk when
asked so even if creating only an image file. The data of any previous session remains visible
when creating additional session.

:p.
:hp2.Attention &colon.:ehp2. If this option is selected, the default track type is CD-ROM XA mode 2. 
The Sony drives (and OEM like some HP) have no hardware support for CD-ROM XA mode 2 so Multisession fails.
Use the 'Sony multisession fix' option in CD Creator Settings, General Page 2, as a workaround.
:p.With cdrecord 1.11a06 and above Sony drives are handled different so it may be the workaround isn't
necessary anymore. You have to test it on your own because I don't own a Sony drive. 
:p.

:h3 res=1940.Boot CD
:i1.Boot CD
:p.
If you want to create a bootable CD you must provide a bootable floppy image. This image must be named 
:hp2.OSBOOT.IMG:ehp2.. Do not use lower case or mixed case! During building of the CD image a file
:hp2.boot.cat:ehp2. will be placed in the root directory of the CD. This file will overwrite any file
with the same name!

:nt.
Boot CDs are written as singlesession CDs.
:ent.

:p.Follow
:link reftype=launch
object='netscape.exe'
data='http://www.geocities.com/SiliconValley/Sector/5785/bootcd/Boot_cw.htm'.
http&colon.//www.geocities.com/SiliconValley/Sector/5785/bootcd/Boot_cw.htm:elink.
for a description how to create the necessary boot image.


:h3 res=1950.User defined
:i1.User defined
:p.
Here you can define the tracktype you want to use. It is possible that not all 
tracktypes work with your recorder, so use it the first time together with the 
'Test only' option.
:p.
:nt.If you have a Teac CD-R50S or CD-R55S select :hp2.FIX:ehp2. because they don't understand the
-nofix flag given to the writer otherwise.
:ent.
:p.
:sl.
:li.:link reftype=hd res=1951.data:elink.
:li.:link reftype=hd res=1952.mode2:elink.	
:li.:link reftype=hd res=1953.XA1:elink.
:li.:link reftype=hd res=1954.XA2:elink.
:li.:link reftype=hd res=1955.CDI:elink.
:li.:link reftype=hd res=1956.FIX:elink.
:esl. 

:h3 res=1951.data
:p.
-data: If this flag is  present,  all  subsequent  tracks  are 
written in CD-ROM mode 1 (Yellow Book) format. The data is a multiple 
of 2048 bytes.  The file with track  data should  contain an ISO-9660 
or Rock Ridge filesystem image (see mkisofs for more details). If the 
track data is an ufs filesystem image, fragment size should be set to 2 
KB or more to allow CR-drives with 2 KB sector size to to be used for reading.

:h3 res=1952.mode2
:p.
-mode2: 
If this flag is present, all subsequent tracks are written in CD-ROM mode 2 format. The data is 
a multiple of 2048 bytes.
:p.
The Yellow Book specification defines two data structures: Mode 1 and Mode 2. The mode byte, 
which is included in the header field of a CD-ROM sector, describes the type of data contained 
in the data field. Mode 1 denotes CD-ROM data with Error Correction Code (ECC), which provides 
room for 2,048 bytes of user data and is the mode used to store data that is 
unforgiving of error, like computer programs or databases. Mode 2 denotes a 
sector with data stored without ECC, which provides more room (2,336 bytes) for user data, but 
which is typically used for data that is more tolerant of error, like audio, 
video, or graphics. 
:p.
Most CD-ROM titles that hold databases, shareware archives, and computer programs are Yellow Book, 
Mode 1, and most CD-ROM discs published--period--are in "plain-vanilla" Yellow Book, Mode 1, 
ISO 9660 Level 1, for the DOS or Windows platform. These discs are also "accessible" on Macintosh 
and UNIX platforms, but they don't appear or perform like Apple or UNIX "natives." In spite of 
and because of this, the plain vanilla CD-ROM is the single most standardized data storage
medium ever created.
:p.
Mode 2 is a way of interleaving sectors of data with extra error correction (Form 1) with sectors 
of data without extra error correction (Form 2), since Mode 1 does not allow unlike sectors to reside 
in the same session on a disc. The Mode 2 branch of the family tree is the show-biz side. CD-ROM/XA, 
Bridge discs (including Photo CD, Karaoke CD, and Video CD), and Green Book, or CD-i, are performing 
cousins to the computer CD-ROM. They have something in common besides Mode 2: 
they are all intended to play on dedicated consumer electronics platforms. 
Some, like Photo CD and Video CD, will play on either a computer or a dedicated platform such as a 
CD-i player, a Video CD player, or a dedicated Photo CD player (if you could find one).

:h3 res=1953.XA1
:p.
-xa1: If this flag is present, all subsequent tracks are written in CD-ROM XA mode 1 format. The 
data is a multiple of 2048 bytes.

:h3 res=1954.XA2
:p.
-xa2: If this flag is present, all subsequent tracks are written in CD-ROM XA mode 2 format. The 
data is a multiple of 2048 bytes.

:h3 res=1955.CDI
:p.
-cdi If this flag is present, all subsequent tracks are written in CDI format. The data is a multiple of 
2048 bytes.
:p.
CD-i is short for Compact Disc Interactive. It is an interactive multimedia system combining moving and 
still video, audio and program content on a compact disc, which can be played back in a dedicated CD-i 
player.

:h3 res=1956.FIX
:p.
You have to check the 'fix disk' button so CD-ROM drives
are able to read the written CD. If you forgot to do so you may later fix the disk using the menu CDR-Tools. 
:p.
:nt.Unchecking the Fix switch doesn't work with Teac CD-Recorders CD-R50S and CD-R55S, so leave it checked.
:ent.
