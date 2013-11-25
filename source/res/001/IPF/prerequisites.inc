:h2 res=303
x=right y=bottom
width=65%
height=100%
group=2. Prerequisites.
:p.
Multimedia support must be installed if you want to create audio CDs.
You don't need a soundcard just the files coming with MMOS2.
:nt.If you've installed
Netscape you may click on the links to open the mentioned web pages.  
:ent.
:p.For successful CD creation get these tools available for free:
:ul.

:li.The EMX runtime libraries V0.9d fix 4 or above.Get them from 
:link reftype=launch
object='netscape.exe'
data='ftp://ftp.leo.org/pub/comp/os/os2/leo/gnu/emx+gcc/emxrt.zip'.
ftp&colon.//ftp.leo.org/pub/comp/os/os2/leo/gnu/emx+gcc/emxrt.zip:elink.

:li.The Aspi-Router V1.01 (c) D. Dorau&colon. 
:link reftype=launch
object='netscape.exe'
data='ftp://ftp.leo.org/pub/comp/os/os2/leo/drivers/misc/aspir101.zip'.
ftp&colon.//ftp.leo.org/pub/comp/os/os2/leo/drivers/misc/aspir101.zip:elink.

:li.Or the Aspi-Router V1.1 (c) Paul Ratcliffe&colon. This is a must if you 
want to use two recorders at the same time (at the moment only support for 
writing audio-cdr's).
:link reftype=launch
object='netscape.exe'
data='http://home.clara.net/orac/os2.htm'.
http&colon.//home.clara.net/orac/os2.htm:elink.

:li.CDrecord/2 1.11a06 or higher&colon.
.br
:link reftype=launch
object='netscape.exe'
data='http://www.os2world.com/cdwriting/'.
http&colon.//www.os2world.com/cdwriting/:elink.
.br
:link reftype=launch
object='netscape.exe'
data='http://www.geocities.com/SiliconValley/Sector/5785/'.
http&colon.//www.geocities.com/SiliconValley/Sector/5785/:elink.

.*data='http://www.os2world.com/cdwriting/cdrecord/files/cdrecord-1_11a06_os2.zip'.
.*http&colon.//www.os2world.com/cdwriting/cdrecord/files/cdrecord-1_11a06_os2.zip:elink.

:nt.
Audio/Data-CD-Creator works with any version of cdrecord/mkisofs. But if you use
mkisofs from a release :hp2.newer than V1.9a02:ehp2. you must insert :hp2.-graft-points:ehp2.
into the mkisofs-options field of the main settings notebook.
.br

:hp8.There're Cdrecord/Cdrtools versions floating around with a broken implementation of this option (usually called
Cdrtools:hp2.2:ehp2., note the '2'). These releases
are NOT supported by Data-CD-Creator! Use only distributions from the URLs above.:ehp8. 
:ent.

:eul.

:p.These are the minimum requirements.
:p.If you have an Atapi or PCCard CD-Writer you need additionally&colon.
:ul.

:li.Danis506.add (c) D. Engert&colon. 
:link reftype=launch
object='netscape.exe'
data='http://hobbes.nmsu.edu/cgi-bin/h-browse?dir=/pub/os2/system/drivers/storage'.
http&colon.//hobbes.nmsu.edu/pub/os2/system/drivers/storage/:elink.

:li.Daniatapi.flt (c) D. Engert&colon. 
:link reftype=launch
object='netscape.exe'
data='http://hobbes.nmsu.edu/cgi-bin/h-browse?dir=/pub/os2/system/drivers/storage'.
http&colon.//hobbes.nmsu.edu/pub/os2/system/drivers/storage/:elink.
:eul.

:p.If you have an USB CD-Writer you need additionally&colon.
:ul.

:li.The USB Mass Storage Device Driver. For more information go to
:link reftype=launch
object='netscape.exe'
data='http://service5.boulder.ibm.com/2bcprod.nsf/186d96ebef23e6438625689c007ac1ef/a3aa799333d1c87386256a3e0071e040?OpenDocument'.
http&colon.//service5.boulder.ibm.com/2bcprod.nsf/186d96ebef23e6438625689c007ac1ef/a3aa799333d1c87386256a3e0071e040?OpenDocument:elink.
:eul.

:nt.
The most recommended hardware setting is&colon. Attach the CD-Writer to your 
second (E)IDE adapter and jumper it as Master. Other configurations my fail.
:ent.

:p.You may also obtain cdrdao/2 if you want to create CDs in DAO mode (recommended)&colon.
:link reftype=launch
object='netscape.exe'
data='http://www.os2world.com/cdwriting/cdrdao/cdrdao-os2-1_1_3_a1.zip'.
http&colon.//www.os2world.com/cdwriting/cdrdao/cdrdao-os2-1_1_3_a1.zip:elink.
:nt.
This version is broken for CD 1&colon.1-copy for Sony and HP writers.
:ent.
:p.There are also some newer for CD 1&colon.1-copy approved cdrdao/2 versions
(using them for burning MP3 files will produce distorted disks).
:nt.
All VAC compiled versions with dll will fail to work so you are unable to use 
cdrdao1.1.6-pre1-f2.zip or higher.
:ent.
:p.Depending on your drives the use of JJSDROM.DMD could also be necessary.
:link reftype=launch
object='netscape.exe'
data='http://hobbes.nmsu.edu/cgi-bin/h-browse?dir=/pub/os2/system/drivers/cdrom'.
http&colon.//hobbes.nmsu.edu/pub/os2/system/drivers/cdrom:elink.

:p.
For decoding and burning :hp2.MP3:ehp2. files you may want to install some additional software, 
like Z!, the MMIOMP3 codecs and the CW-Multimedia Classes.
:p.
You may also want to install the ISOFS installable file system and WPS-Wizard.
:p.
Audio-Data-Cdcreator is already prepared to use these packages.


