:h1 res=320
x=left y=bottom
width=35%
height=100%
group=1.Easy installation
:link reftype=hd res=321 auto dependent.

:p.
If you're new to Audio/Data-CD-Creator or to CD writing in general follow the
instructions in this chapter.

:p.
:nt.If you've installed
Netscape you may click on the links to open the mentioned web pages.  
:ent.
:p.
:ol.
:li.:link reftype=hd res=321.Prerequisites:elink.
:li.:link reftype=hd res=322.Installation:elink.
:li.:link reftype=hd res=323.Configuration:elink.
:li.:link reftype=hd res=324.Useful addons:elink.
:eol.

:h2 res=321
x=right y=bottom
width=65%
height=100%
group=2.Prerequisites
:p.
Multimedia support must be installed if you want to create audio CDs.
You don't need a soundcard just the files coming with MMOS2.
:p.For successful CD creation get these tools available for free:
:ul.
:li.WPS-wizard
:link reftype=launch
object='netscape.exe'
data='http://www.os2world.com/cdwriting'.
http&colon.//www.os2world.com/cdwriting:elink.

:li.The EMX runtime libraries V0.9d fix 4 or above. Get them from
.br
:link reftype=launch
object='netscape.exe'
data='ftp://ftp.leo.org/pub/comp/os/os2/leo/gnu/emx+gcc/emxrt.zip'.
ftp&colon.//ftp.leo.org/pub/comp/os/os2/leo/gnu/emx+gcc/emxrt.zip:elink.

:li.CDrecord/2 2.00 or higher&colon.
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

:nt.
:hp8.There're Cdrecord/Cdrtools versions floating around with a broken implementation of some options (usually called
Cdrtools:hp2.2:ehp2., note the '2'). These releases
are NOT supported by Data-CD-Creator! Use only distributions from the URLs above.:ehp8. 
:ent.

:li.Danis506.add (c) D. Engert&colon. 
.br
:link reftype=launch
object='netscape.exe'
data='http://hobbes.nmsu.edu/cgi-bin/h-browse?dir=/pub/os2/system/drivers/storage'.
http&colon.//hobbes.nmsu.edu/pub/os2/system/drivers/storage/:elink.

:li.Daniatapi.flt (c) D. Engert&colon.
.br
:link reftype=launch
object='netscape.exe'
data='http://hobbes.nmsu.edu/cgi-bin/h-browse?dir=/pub/os2/system/drivers/storage'.
http&colon.//hobbes.nmsu.edu/pub/os2/system/drivers/storage/:elink.
:eul.
:p.
These are the minimum requirements.
:p.
It's recommended to also install the MMIOMP3 IO-procedures. These procedures allow you to play
MP3 and OGG files using your standard multimedia applications. Some features of the Audio-CD-Creator
are only available with the IO procedures (for example on the fly writing of MP3 files) and
a lot of configuration items can be set automatically when the procedure are present, which makes
the setup of the CD-Creator package a lot of easier. 
:p.
Get them from:
:p.
:link reftype=launch
object='netscape.exe'
data='http://www.math.berkeley.edu/~roconnor/'.
http&colon.//www.math.berkeley.edu/~roconnor/:elink.


.****************** Installation ************************
:h2 res=322
x=right y=bottom
width=65%
height=100%
group=2.Installation
:p.
:hp8.
Make a copy of your INI files before installing the classes or create a new archive using the archiving function
of the WPS!
:ehp8. 

:ol.
:li.Install WPS-wizard follwing the instructions coming with that package.

:li.Install the EMX runtime libraries according to the instructions coming with the package. This means
basically to put the DLL files somewhere in your libpath.

:li.If you have an ATAPI CD writer (a writer connected to your IDE controller) or an ATAPI CD reader add 
- after unzipping the files to 
x&colon.\OS2\BOOT\ (replace :hp2.x:ehp2. with your boot drive letter) - the following lines to your config.sys
:p.
:hp2.BASEDEV=DANIS506.ADD:ehp2.
:p.
:hp2.BASEDEV=DANIATAPI.FLT:ehp2.
:p.
Adding switches to DANIS506.ADD is normally :color fc=red.not:color fc=black. necessary. In contrary 
it can cause several faults.
:nt.
The most recommended hardware setting is&colon. Attach the CD-Writer to your 
second (E)IDE adapter and jumper it as Master.
:ent.

:li.and :color fc=red.REM or delete:color fc=black. from your config.sys 
:p.
:hp2.BASEDEV=IBM1S506.ADD:ehp2.
:p.
:hp2.BASEDEV=IBMATAPI.FLT:ehp2.
:p.
:hp2.BASEDEV=IBMIDECD.FLT:ehp2.
:p.
If one of the latter drivers is in your config.sys CD-Record refuses to work.
:li.Install CDRecord using the installation program coming with it. If there's no installation program then you're
using the wrong release.
:li.Now reboot. This is necessary so the new drivers are loaded by the system.

:li.Go to the :hp2.CD writing folder:ehp2. on your desktop, open the CDRTools folder
and start the :hp2.Scan the SCSI/ATAPI bus for devices:ehp2. object. A command window
opens and you should see a listing of all devices on your SCSI and ATAPI bus. If you don't
see such a listing the installation of the CDRecord/CDRTools package is not complete. Do the following:
:ul.
:li.Check your EMX installation. EMX is coming with a script :hp2.EMXREV.CMD:ehp2. to do so.
:li.Check the installation of ASPIROUT.SYS and OS2ASPI.DMD in your config.sys:
:p.
:hp2.BASEDEV=OS2ASPI.DMD /ALL:ehp2.
:p.A similar line must be present in your config.sys.
:note.The switch :hp2.:color fc=red./ALL:ehp2.:color fc=black. must be present or CDRecord/2 will not work!
:p.
:hp2.DEVICE=x&colon.\OS2\BOOT\ASPIROUT.SYS:ehp2.
:eul.
:li.When cdrecord is working install Audio/Data-CD-Creator by starting the installation program :hp2.install.exe:ehp2..
Follow the instructions given on the different pages and finally press
Finish. You may cancel the installation at any time. During the installation a DLL will be copied to your
x&colon.\os2\dll directory and a helpfile to your system help directory.
.*** :li.Restart the system. This is necessary so the new WPS classes are loaded.
:eol.

:p.
In the :hp2.CD writing folder:ehp2. you should find a couple of objects now.
:p.


.***************** Configuration *******************
:h2 res=323
x=right y=bottom
width=65%
height=100%
group=2.Configuration
:p.
Before starting creating a CD you have to adjust some settings the installation program can't
figure out on it's own. Follow the instructions below.
The installation program already opened the Creator settings object in the CD-writing folder for you.
:nt.
Use the 'help' button of the settings pages. There is often additional information which can be quite useful.
:ent.

:ol.
:li.Select your writer on the :hp2.Writer setup:ehp2. page. You may use the :hp2.Scan:ehp2. button
to rediscover the devices in your system.

:li.If you want to use FreeDB to get the names of audio tracks on inserted CD enable this on the
:hp2.FreeDB setup:ehp2. page.

:li.Go to the :hp2.cdrdao/2:ehp2. page and select your writer or a general driver from the list. 
See also chapter 'List of supported CD-Writers'. If necessary enter the optional command(s) for your drive 
in 'Options'. Make these steps for all three cdrdao/2 pages. You can specify different cdrdao versions
and different drivers for every page.
:nt.If your drive is not listet&colon. Almost every CD-Writer build since the year 99 should work with 
:hp2.(driver) generic-mmc:ehp2. or :hp2.(driver) generic-mmc-raw:ehp2.:ent.
:nt.
Setup of Cdrdao is only necessary if you want to create 1&colon.1 copies or audio CDs in DAO mode.
:ent.

:li.Select the :hp2.Grabber:ehp2. page. Choose the device you want to use for grabbing. Also
specify the drive letter assigned to that device by OS/2.

:li.Normally you don't have to change anything on the :hp2.MP3 decoding:ehp2. page. But if CDs written
from MP3 files only contain noise you have to change the :hp2.Swap bytes when writing:ehp2. setting.

:li.The :hp2.MP3 encoder:ehp2. pages may be used to setup LAME for creating MP3 files from audio files
or CDs.

:li.Gratulations&colon. You have finished the setup.
:eol.
:p.



:h2 res=324
x=right y=bottom
width=65%
height=100%
group=2.Useful addons
:p.
When basic CD writing is working, you may consider to install the following addons.
:p.
:ul.
:li.CW-Multimedia Classes and z! for playing MP3 files.
:li.ISOFS installable file system to mount ISO images as drives.
:eul.
:p.
Audio-Data-Cdcreator is already prepared to use these packages.


