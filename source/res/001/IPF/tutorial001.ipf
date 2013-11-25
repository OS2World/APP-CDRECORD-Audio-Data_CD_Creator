:userdoc.

:docprof.

:title. Audio-CD-Creator/Data-CD-Creator

:h1 res=001
x=right y=bottom
width=100%
height=100%.Audio/Data-CD-Creator V0.56
:i1 global.Audio-CD-Creator
.ce (C) Chris Wohlgemuth 1999-2005
.ce Graphics and icons (C) Klaus Staedtler

:artwork align=center name='wrfolder.BMP'.
:p.
Audio/Data-CD-Creator helps you creating CDs with CDRecord/2 and cdrdao/2. It works also as a frontend for any grabber program.
As a folder subclass it uses features of the WPS like Drag and Drop and context menus. Have a look at the
:link reftype=hd res=150.feature list:elink. for more information.
:p.The most recent version may be obtained here&colon.

:p. 
:link reftype=launch
object='netscape.exe'
data='http://www.os2world.com/cdwriting'.
http&colon.//www.os2world.com/cdwriting/:elink.
:nt.
Klick to start Netscape with that URL.
:ent.


.im generalinformation.inc

:h2 res=200.License
:i1.License
.im licence.inc

:h2 res=250.Credits
.im credits.inc

.im easyinstallation.inc

.** .im advancedinstallation.inc


:h1 res=350
x=right y=bottom
width=100%
height=100%.Quick start (for those who don't read tutorials)
:p.
After successful :link reftype=hd res=320.installation:elink. you find a folder with several objects on you desktop.
The :hp2.CD-Creator settings:ehp2. object is used to specify global options like paths or writer options. Check these
settings before you proceed.
:p.
Writing is done using the templates for audio and data CDs. Drop a template somewhere in you system to create a
CD-creator folder and open it. Put the files you want to burn in the folder. The default action is to create
shadows when dropping objects but you may also move or copy files. Audio folders don't have that much options. They're
all located on the left side of the folder. Options for data CDs like filename length or single/multisession 
are set in the settings notebook of the folder. These settings are local to the particular folder. That means you
may set different options for any creator folder you create. After collecting the files and specifying the
characteristics of the CD press the :hp2.Write:ehp2. button to burn the CD.
:p.
There's help available for all the settings and the controls of the folders. Use the help button of the
settings pages and the right mouse button. Open the help for the folders for more information and use also
the help for menu items (press F1 while selecting a menu).
:p.
In case of questions read this turorial and the FAQs. Please keep in mind, we cannot 
answer any question here. If you have general questions about burning CD's please first take a look
at e.g.&colon. 
:p. 
:link reftype=launch
object='netscape.exe'
data='http://www.cdrfaq.org'.
http&colon.//www.cdrfaq.org:elink.

:h1 res=400
x=left y=bottom
width=35%
height=100%
group=1.Creating audio CDs
:link reftype=hd res=402 auto dependent.
:p.Audio-CD-Creator allows you to create audio CDs in DAO and TAO mode. 
Select one of the items below for more information how to burn the different kinds of CDs.

:p.
:link reftype=hd res=402.Using grabbing to create an audio CD:elink.
:p.
:link reftype=hd res=403.Using shadows to create an audio sampler:elink.
:p.
:link reftype=hd res=404.Create an audio CD from MP3 files:elink.
:p.
:link reftype=hd res=407.Use two cd-writers at the same time:elink.
:p.
:link reftype=hd res=408.Adjust the Pregap in DAO mode:elink.

.im writegrabwaves.inc

.im writewaveshadows.inc

.im writemp3.inc

.im twowriters.inc

.im pregap.inc

.**************************************

:h1 res=450
x=left y=bottom
width=35%
height=100%
group=1.Copy CD's
:link reftype=hd res=406 auto dependent.
:p.1&colon.1 CD-Copy allows you to copy your CD regardless of it's content

:p.
:link reftype=hd res=406.Create a copy of a cd:elink.

.im cd11.inc

.**************************************

:h1 res=550
x=left y=bottom
width=35%
height=100%
group=1.Creating ISO images
:link reftype=hd res=501 auto dependent.
:p.Data-CD-Creator allows you to create ISO imagefiles usable for singlesession and multisession CDs

:p.
:link reftype=hd res=501.Creating the image file:elink.
:p.
:link reftype=hd res=551.Mounting the image file:elink.

.im createimage.inc

.im mountimage.inc

.**************************************

:h1 res=500
x=left y=bottom
width=35%
height=100%
group=1.Creating data CDs
:link reftype=hd res=502 auto dependent.
:p.Data-CD-Creator allows you to create singlesession and multisession CDs on the fly
or by using an intermediate image file.
Select one of the items below for more information how to burn the different kinds of CDs.

:p.
:link reftype=hd res=502.Writing an existing image file:elink.
:p.
:link reftype=hd res=503.Creating a singlesession CD:elink.

.im writeimage.inc

.im singlesession.inc

.**************************************

:h1 res=570
x=left y=bottom
width=35%
height=100%
group=1.Creating Covers
:link reftype=hd res=571 auto dependent.
:p.Together with an installed Photographics Pro covers will be created automatically

:p.
:link reftype=hd res=571.Create a coversheet:elink.

.im createcover.inc

:h1 res=850
x=left y=bottom
width=35%
height=100%
group=1.Customization
:i1 id=customization.Customize Audio/Data-CD-Creator

:link reftype=hd res=851 auto dependent.
:p.
Audio/Data-CD-Creator offers some features to improve usability.

:p.
:link reftype=hd res=851.Launch area:elink.
:p.
:link reftype=hd res=852.Extended context menu:elink.
:p.
:link reftype=hd res=853.User menu:elink.
:p.
:link reftype=hd res=854.Cover creation:elink.
:p.
:link reftype=hd res=855.CDDB tracknames:elink.

.im launchpad.inc
.im contextmenu.inc
.im usermenu.inc
.im cover.inc
.im tracknames.inc

:h1 res=800
x=left y=bottom
width=35%
height=100%
group=1.FAQ and troubleshooting
:link reftype=hd res=801 auto dependent.
:p.Frequently Asked Questions about Audio/Data-CD-Creator.
Select one of the items below to get the answers.
:p.

.im adcfaq.inc


:h1 res=600
x=left y=bottom
width=35%
height=100%
group=1.CD-Record FAQ
:link reftype=hd res=601 auto dependent.
:p.Frequently Asked Questions about CD-Record/2.
Select one of the items below to get the answers.
:p.

.im cdrecordfaq.inc


:h1 res=700
x=left y=bottom
width=35%
height=100%
group=1.List of supported CD-Writers
:link reftype=hd res=701 auto dependent.

:p.
:link reftype=hd res=701.List of known CD-Writers which work with CDRecord/2:elink.

.im cdwriters.inc


:h1 res=900
x=left y=bottom
width=35%
height=100%
group=1.Setup strings

:p.The Creator-classes support all the setup strings of common WPS folders. In addition they support strings to set writing options,
default behaviour and status information. The helper programs started bei the classes use the latter to inform the user about events.
:p.

:sl.
:li.:link reftype=hd res=901.Setup strings for Audio-CD-Folders:elink.
:li.:link reftype=hd res=902.Setup strings for Data-CD-Folders:elink.
:li.:link reftype=hd res=903.Setup strings for Creator-Settings:elink.
:esl.

.im audiosetupstrings.inc
.im datasetupstrings.inc
.im creatorsetupstrings.inc

:h1 res=950.History
.im history.inc

:h1 res=960.Contact
.im contact.inc

:euserdoc.







