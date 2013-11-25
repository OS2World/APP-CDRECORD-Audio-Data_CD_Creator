:userdoc.

:docprof.

:title. Help for Audio-CD-Creator/Data-CD-Creator

.**** Help for the settings notebook
:h1 res=003.CD-Creator settings
:i1 global.CD-Creator settings
:p.
Use this object to specify global settings for Audio/Data-CD-Creator.
For example the paths to the various helper programs like CDRecord/2, mkisofs etc.
or options which are specific for your writer and which only must be set once
after installation.
:p.
:p.
This DLL and the accompaigning program are provided 'as is' under the terms of the Gnu Public Licence (GPL).
For further information read the file :link reftype=hd res=202.'COPYING':elink..
:p.
:link reftype=hd res=201.History:elink.
:p.
:link reftype=hd res=203.Credits:elink.
:p.
:note text='Author:'. Chris Wohlgemuth.
:note text='E-Mail:'. chris.wohlgemuth@cityweb.de
:note text='WWW:'.    :link reftype=launch
object='netscape.exe'
data='http://www.geocities.com/SiliconValley/Sector/5785/index.html'.
http&colon.//www.geocities.com/SiliconValley/Sector/5785/index.html:elink.
:p.

.im aboutsettings.inc

.im generalsettings.inc

.im generalsettings2.inc

.im toolbarsettings.inc

.im hintsettings.inc

.im cddbsettings.inc

.im cdrecordsettings.inc

.im cdrdaosettings.inc

.im grabbersettings.inc

.im mpg123settings.inc

.im mp3encodersettings.inc

.im mkisofssettings.inc

.im automaticconfig.inc

:h1 res=001.Audio-CD-Creator V0.56
:i1 global.Audio-CD-Creator
:p.
Audio-CD-Creator helps you creating Audio-CDs with CDRecord/2 and cdrdao/2. It works also as a frontend for any grabber program.
As a folder subclass it uses features of the WPS like Drag and Drop and context menus. This version supports TAO- and DAO-mode.

:p.
Put the wavefiles you want to burn into this folder, 
arrange them in the desired order and press the write-button. It's possible to use shadows of wavefiles, too. The
folder follows the links and gives the right names to CDRecord/2 or cdrdao/2. It's also possible to write MPS3 files
without manual decoding to a CD when mpg123 (included) or z! is properly setup.
Any other object is skipped while processing the
folder contents. So you can put dokumentation files, program objects and so on into this folder, too.
All options are set using the settings notebook of the folder object. Help is avaiable for most functions. Use the
right mouse button.
:p.
:link reftype=hd res=600.Main window help:elink.
:p.
This DLL and the accompaning program are provided 'as is' under the terms of the Gnu Public Licence (GPL).
For further information read the file :link reftype=hd res=202.'COPYING':elink..
:p.
:link reftype=hd res=201.History:elink.
:p.
:link reftype=hd res=203.Credits:elink.
:p.
:note text='Author:'. Chris Wohlgemuth.
:note text='E-Mail:'. chris.wohlgemuth@cityweb.de
:note text='WWW:'.    http&colon.&slr.&slr.www.geocities.com&slr.SiliconValley&slr.Sector&slr.5785&slr.
:p.


:h2 res=600.Main window help
.im mainwindowaudio.inc

:h3 res=601.Write mode
.im writemodeaudio.inc


:h3 hide res=1200.Write status window
:p.
This window shows the write status. 
:p.
When pressing the :hp2.Break:ehp2. button the write process is terminated after the current track.
The process does not end immediately. The track will be properly written and closed to prevent
damaging the CD-R. Killing the write process during writing a track produces an unusable CD. It may
be necessary to manually fix the CD so an audio player can read it. Use CDR-Tools to do so.

.im tracklist.inc

:h3 res=602.Grab mode
.im grabmodeaudio.inc

:h3 hide res=900.Grab status window
:p.
This window shows the grab status. 
:p.
When pressing the :hp2.Break:ehp2. button the grabber process is terminated after the current track.
If you want to kill the process immediately chose the :hp2.Grab track:ehp2. window in the window list
and type CTRL-C in that window. If you selected several tracks the next track will be grabbed so it
may be necessary to click :hp2.Break:ehp2. prior to killing the process.


:h2 res=603.Status line
:p.
The status line shows the total time of all tracks. It's important to know that the cd-writer inserts a pause of 
2 seconds between each two tracks. These pauses are not included in the shown time. To change the time between the 
tracks use the pregap setting when writing in DAO mode.
.im pregap.inc

:h2 res=203.Credits
.im credits.inc

:h2 res=202.Licence
.im licence.inc

:h2 res=201.History
.im history.inc




:h1 res=002.Data-CD-Creator V0.56
:i1 global.Data-CD-Creator
:p.
Data-CD-Creator helps you creating data-CDs with CDRecord/2. It shields you from the commandline syntax of
CDRecord/2 and mkisofs. As a folder subclass it uses features of the WPS like Drag and Drop and context menus.
:p.
It's possible to create singlesession- and multisession CDs. Choose the features you want in the settings notebook.
After putting folders and files you want to burn on CD into the folder press the 'Create image' button. Mkisofs
will be called with the options you've set. Afterwards select the 'Write-CD' button to write the created ISO-Image on
CD. The contents of the folder will be written into the root-directory of the CD.
:p.
Beside filesystem objects you can also put shadows into the folder
to create a virtual tree of files. When building the image
the linked to files and folders are searched.
:p.
:link reftype=hd res=221.Main window help:elink.
:p.
This DLL and the accompaigning program are provided 'as is' under the terms of the Gnu Public Licence (GPL).
For further information read the file :link reftype=hd res=202.'COPYING':elink..
:p.
:link reftype=hd res=201.History:elink.
:p.
:link reftype=hd res=203.Credits:elink.
:p.
:note text='Author:'. Chris Wohlgemuth.
:note text='E-Mail:'. chris.wohlgemuth@cityweb.de
:note text='WWW:'.    http&colon.&slr.&slr.www.geocities.com&slr.SiliconValley&slr.Sector&slr.5785&slr.
:p.


:h2 res=221.Main window help
.im datamainwindowhelp.inc


:h3 res=1501.Imagename
:p.
Enter the full path to the imagefile into this field. If you want to write the image on
CD it's the source path.
:p.
If you want to create an image it's the destination path. If you select an already existing
filename, this image will be overwritten.
:p.

:h3 res=1610.Create image only
:p.
Use this option if you only want to create an image file contain your data. You may
burn this image later on a CDR after checking :hp2.Write only:ehp2..

:h3 res=1611.Write only
:p.
Select this check box if you want to write an already existing image on CDR. Use the
entryfield and the :hp2.browse:ehp2. button at the top to select the image file. After doing so
press the :hp2.write:ehp2. button. You may select :hp2.Test only:ehp2. if you only want to do a test. 

:h3 res=1612.On the fly
:p.
When choosing :hp2.On the fly:ehp2. the data will be written directly on the CDR without
creating an intermediate image file. Be sure the performance of your system is sufficient
or a buffer underrun may occur causing the disk to be damaged. It's always a good idea to do
some testing in test mode first (check :hp2.Test only:ehp2.) to check the configuration.

:h3 res=226.Test only
:p.
When selecting this option all writing is done in test mode. This means the laser of the writer
is turned off and no data is written on the CDR. This is useful to test the performance and
configuration of the machine to make sure a real write will be successful.

:h3 res=1502.Browse
:p.
Use this button to search an image file using the standard file dialog. If you know the name
and position of the file you may enter the full path directly into the entryfield.
:p.
If you want to create an image you may use the file dialog to find the location where
the file will be written to. Select the apropriate subdirectory and enter a filename into
the file dialog. After pressing :hp2.Ok:ehp2. the full path will be put into the entryfield.
If the file already exists the image will be overwritten.

:h3 res=1503.Toolbar
:p.
The Toolbar gives you access to several tools, settings etc. You can also drop 
any preferred program into the empty button.

:h3 res=1607.Check size
:p.
Press this button to determine the size of the image file. If an already created file
should be written on the CD the size of this file is displayed in the status line. In the
other cases mkisofs is called to calculate the size.

:h3 res=1604.Write CD
:p.
After pressing this button a status dialog will be shown and the CD written according
to the settings. If you disabled the 'Test only' switch you will be asked for confirmation
first. 

:h3 res=1601.Create image
:p.
Press this button to create the ISO image. The name will be taken from the entry field.
Specify options in the settings notebook first.

.im cdtypesettings.inc

.im filenamesettings.inc

.**** Author settings page
:h2 res=1700.Author
:p.
You may specify some information about the CD on this page.
:ul.
:li.:hp2.Application&colon.:ehp2.
:sl.
:li.This field is used to describe the application written on the CD.
:esl.
:li.:hp2.Volume name&colon.:ehp2.
:sl.
:li.The name of the CD as shown when inserted into the drive.
:esl.
:li.:hp2.CD-Publisher&colon.:ehp2.
:sl.
:li.The publisher of this CD. 
:esl.
:li.:hp2.CD-Preparer&colon.:ehp2.
:sl.
:li.The person which prepared the data.
:esl.
:eul.

.im dataspecial.inc

.**** More help panels for objects (tutorial, folder etc.) ****
.im objecthelp.inc


.**** Help panels for menus ****
.im menuhelp.inc

.*** The help panels for the mount/unmount dialogs
.im isoimagedialogs.inc

.*** The help panels for the CD-copy dialog
.im cdcopydialog.inc

.*** The help panels for the MP3 decoding dialog
.im mp3decodedialog.inc

.*** The help panels for the MP3 encoding dialog
.im mp3encodedialog.inc

.*** The help panels for the on the fly encoding dialog
.im ontheflyencodedialog.inc

.*** The help panels for the DAO writing dialog
.im daohelp.inc

:euserdoc.





