
:h2 res=304
x=right y=bottom
width=65%
height=100%
group=2. Using the first time
:p.Before starting creating any CDs you have to adjust some settings the installation program can't
figure out on it's own. Follow the instructions below.
:p.
:hp8.Make sure the Cdrecord package is properly installed and you rebooted after installing it.:ehp8.
See :link reftype=hd res=322.installation:elink. for more information.
:p.
:ol.
:li.Open the Creator settings object in the CD-writing folder.
:li.Make sure all paths of the helper programs are correct.
:li.Press the :hp2.Scan:ehp2. button on the general page to fill the drop down box with the devices
on your SCSI bus. Using DANIS506.ADD and DANIATAPI.FLT ATAPI devices are routed to SCSI, 
so they will appear as a SCSI-Device. Select your writer device from the list.
:li.If you have two writer devices select the device you want to use for audio and data. 
This will be the default writer.
:li.If you posess a Sony or a Sony OEM CDR(W) drive (e.g. many HP) go to page 2 of the general page 
by clicking on the small + sign on the upper right end of the page and select 'Sony multisession fix', 
otherwise you won't be able to write multisession CD's with these drives.
:li.If you posess a CD-Writer equipped with the Burnproof technology or any other anti-coaster technology 
go to page 2 of the general page by clicking on the small + sign on the upper right end of the page and select 
'Use BURN-Proof' to enable Burnproof during writing a CDR. As 'Just-Link' is enabled by 
default, don't check 'Use BURN-Proof'. 
:nt. "burnproof" is internally renamed to "burnfree". This has been done as this technology now appears 
in the MMC standard. 
Currently, Joerg Schilling knows of the following similar technologies:
BURN-Proof Sanyo, Just-Link Ricoh, Super-Buffer Philips, Write-Proof TEAC, 
Safe-Burn Yamaha, Buffer Underrun Free MMC-3. Cdrecord now shows whether 
BURN-Free is active. This makes sense as Ricoh's "Just-Link" is enabled by default. 
Unfortunately, the way "Just-link" is implemented is not compliant with the MMC standard. 
So you may have a look at the pwrt*.log files situated in the \logfiles subdirectory to 
see if an anti-coaster works or not.

:li.Go to :hp2.User Interface:ehp2.. On these pages you can disable the fly over help and change its appearance, 
enable folder hints, enable tips and change the look to OS/2 default. As long as you are not quite familiar with 
audio-data-cdcreator it is recommended to enable everything. Naturally changing the look is a matter of taste.

:li.Go to :hp2.CDDB setup:ehp2.. If you want to enable CDDB select 'Use CDDB/Freedb'. Then enter in 'User name: 
the userid part of your e-mail address, i.e. captain.kirk if your e-mail address is something like 
captain.kirk@enterprise.com. This setting is mandatory. At 'User Host' insert the domain name of 
your e-mail address, e.g. enterprise.com if your e-mail address is captain.kirk@enterprise.com. 
This setting is also mandatory. If you don't want to use the preset servers from freedb.org or 
cddb.com you can specify other CDDB servers on the next page if you click on the + sign at the right 
upper end of the tab.

:li.Go to the :hp2.cdrdao/2:ehp2. page and select your writer or a general driver from the list. 
See also chapter 'List of supported CD-Writers'. If necessary enter the optional command(s) for your drive 
in 'Options'. Make these steps for all three cdrdao/2 pages. You can specify different cdrdao versions
and different drivers for every page.
:nt.If your drive is not listet&colon. Almost every CD-Writer build since 99 should work with '(driver) generic-mmc'.:ent.

:li.Select the :hp2.Grabber:ehp2. page. Choose the drive you want to use for grabbing. 
Now select from the dropdownlist the grabber you want to use. If you grabber is 
not listed you have to insert the options for your grabber maually. Use help to 
figure out which options your program needs.

:li.Next go to :hp2.MP3 decoding:ehp2.. Select there the MP3 decoder you want to use. 'mpg123' which 
is delivered with Audio-Data-Cdcreator, 'Z!' by Dink or the MMIOMP3 codecs by Russell O'Connor. Make 
sure the correct path is set.

:li.Finally go to :hp2.Mkisofs setup:ehp2. and select the path to 'mkisofs.exe'. If you use 
an cdrecord version newer than 1.9a02 enter :hp2.-graft-points:ehp2. in the field for additional 
Mkisofs options. You can disable the translation codepage too.

:li.Gratulations &colon.You have finished the setup.

:li.Use the 'help' button. There are several additional informations which can be quite useful.
:eol.

