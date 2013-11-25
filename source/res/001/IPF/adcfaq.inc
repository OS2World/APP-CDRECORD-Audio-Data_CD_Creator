
:sl.
:li.:link reftype=hd res=827.I can't find some global settings.:elink.
:li.:link reftype=hd res=828.There're no toolbars.:elink.
:li.:link reftype=hd res=801.The screen is cluttered with open VIO windows.:elink.
:li.:link reftype=hd res=817.No light, no sound.:elink.
:li.:link reftype=hd res=802.I can't write multisession CDs with a Sony or HP-writer.:elink.
:li.:link reftype=hd res=803.Data CD creation fails with recent releases of mkisofs.:elink.
:li.:link reftype=hd res=825.Data CD creation still fails.:elink.
:li.:link reftype=hd res=804.z! decodes fine but writing fails.:elink.
:li.:link reftype=hd res=805.My CD-R/RW drive is not in the list does it work?:elink.
:li.:link reftype=hd res=806.Create Multisession CD's.:elink.
:li.:link reftype=hd res=807.Create Boot-CD's.:elink.
:li.:link reftype=hd res=808.I can't write a CD-R/RW.:elink.
:li.:link reftype=hd res=809.FreeDB query doesn't work.:elink.
:li.:link reftype=hd res=810.I used shadows when creating the CD but don't see the files.:elink.
:li.:link reftype=hd res=811.No mount/unmount menus for ISO images.:elink.
:li.:link reftype=hd res=812.TAO audio writing doesn't work with TEAC drive.:elink.
:li.:link reftype=hd res=813.The data folder suddenly stops collecting files.:elink.
:li.:link reftype=hd res=814.I have strange errors when using the folders after upgrading.:elink.
:li.:link reftype=hd res=815.I can't grab tracks from my SCSI-CD-Rom.:elink.
:li.:link reftype=hd res=816.After using checkini some objects refuse to work.:elink.
:li.:link reftype=hd res=818.Update a set of ConvPak CDs:elink.
:li.:link reftype=hd res=819.There's only noise on my decoded MP3-CD:elink.
:li.:link reftype=hd res=820.There's music with noise on my decoded MP3-CD:elink.
:li.:link reftype=hd res=821.The playtime of MP3 files is wrong:elink.
:li.:link reftype=hd res=822.Playtime calculation fails:elink.
:li.:link reftype=hd res=823.There's no audio information in details view:elink.
:li.:link reftype=hd res=824.Is it possible to copy CD's on the fly?:elink.
:li.:link reftype=hd res=826.On the fly MP3 encoding doesn't work.:elink.
:li.:link reftype=hd res=870.I have still problems.:elink.
:li.:link reftype=hd res=871.This stuff is a pile of crap thrown together by a moron.:elink.
.*Last id: 827
:esl.

:h2 res=827
x=right y=bottom
width=65%
height=100%
group=2.I can't find some global settings.
:p.
With V0.54 of the package some seldom used settings are hidden and the settings notebook
only contains a few necessary pages. If you want to see all settings because you want for example
use z! for MP3 decoding or another release of cdrecord do the following.
:p.
Open the main settings notebook in the CD-Writing Folder and select :hp2.Show every setting:ehp2. on the
first page. The notebook will close and immediately reopen with updated settings pages.


:h2 res=828
x=right y=bottom
width=65%
height=100%
group=2.There're no toolbars.
:p.
:ul.
:li.Make sure WPS-wizard is properly installed.
:li.REXX support must be enabled in WPS-wizard.
:li.CD-Creator folders from a previous installation will have standard toolbars without items for
cd writing. To change that open the folders settings notebook, switch to the :hp2.REXX script:ehp2. page
and select the script :hp2.CWAudioFolder.rx:ehp2. or :hp2.CWDataFolder.rx:ehp2. respectively from
the scripts directory of your Audio/Data-CD-Creator installation.
:eul.
:h2 res=801
x=right y=bottom
width=65%
height=100%
group=2.The screen is cluttered with open VIO windows.
:p.
Open the main settings notebook in the CD-Writing Folder and select :hp2.Show every setting:ehp2. on the
first page.
:p.
Go to the :hp2.General page:ehp2. and check :hp2.Start hidden:ehp2. and :hp2.Close window on exit:ehp2..


:h2 res=817
x=right y=bottom
width=65%
height=100%
group=2.No light, no sound.
:p.
If nothing seems to work do the following:

:ol.
:li.Check your EMX installation. The EMX runtime libraries must be in your LIBPATH. Make sure there're no old revision
in a directory at the beginning of your libpath. Use the EMXREV.CMD coming with EMX to check. 
:li.Check your cdrecord installation. In a OS/2 window enter the command :hp2.cdrecord -scanbus:ehp2..
The result must be a listing of all your devices on the SCSI- (or ATAPI-) bus.
:li.If you get error boxes read the logfiles in the directory :hp2.\logfiles:ehp2..
:li.Deselect :hp2.close window on exit:ehp2. on the general page of the main settings notebook
to read the output of mkisofs, cdrecord, z! and cdrdao.
:li.If you have strange errors when using only some parts of the package read the topic
":link reftype=hd res=814.I have strange errors when using the folders after upgrading.:elink.".
:li.Read this tutorial and use the helppages available for every dialog, the custom menus and
some of the controls (hint: there's a second button on your mouse...).
:eol.

:h2 res=802
x=right y=bottom
width=65%
height=100%
group=2.I can't write multisession CDs with a HP or Sony writer.
:p.
Open the main settings notebook in the CD-Writing Folder and select :hp2.Show every setting:ehp2. on the
first page.
:p.
Go to the :hp2.General page:ehp2. of the notebook and check
:hp2.Sony multisession fix:ehp2..

:h2 res=803
x=right y=bottom
width=65%
height=100%
group=2.Data CD creation fails with recent releases of mkisofs.
:p.
Mkisofs from cdrecord distributions :hp2.newer than V1.9a02:ehp2. uses different options when
creating a virtual tree of files. Insert :hp2.-graft-points:ehp2. into the mkisofs-options
field of the main settings notebook to solve the problem.
:p.
:nt.
:hp8.
There're Cdrecord/Cdrtools versions (called Cdrtools2 <- note the :hp2.2:ehp2.) floating around with a broken implementation of this option. These releases
are NOT supported by Data-CD-Creator! Use only Cdrecord/Cdrtools packages from&colon.:ehp8.
:p.
:link reftype=launch
object='netscape.exe'
data='http://www.geocities.com/SiliconValley/Sector/5785/'.
http&colon.//www.geocities.com/SiliconValley/Sector/5785/:elink.
.br
:link reftype=launch
object='netscape.exe'
data='http://www.os2world.com/cdwriting'.
http&colon.//www.os2world.com/cdwriting/:elink.
:nt.
Klick to start Netscape with that URL.
:ent.
:ent.

:h2 res=825
x=right y=bottom
width=65%
height=100%
group=2.Data CD creation still fails.
:p.
Did you check the question :link reftype=hd res=803.Data CD creation fails with recent releases of mkisofs:elink.?
:p.
:ul.
:li.If Mkisofs setup is correct check the log of mkisofs in the :hp2.logfiles:ehp2. directory.
:li.When writing a CD with Joliet filenames make sure the names of the source files are not longer than 64 characters. Mkisofs
will truncate them but isn't very smart while doing that. The truncating may end with duplicate filenames preventing a successful
creation of the image.

:eul.
:h2 res=804
x=right y=bottom
width=65%
height=100%
group=2.z! decodes fine but writing fails.
:p.
Make sure you've the right settings for z!. The player may be configured to use 8bit
soundcards and this setting also affects decoding to wave. For writing on CD the wave must be 16bit.

:h2 res=805
x=right y=bottom
width=65%
height=100%
group=2.My CD-R/RW drive is not in the list does it work?
:p.
As all Recorders following the SCSI or ATAPI MMC-3 conventions are supported, almost 
every actual CD-Recorders works. Select the generic-mmc driver on the 'CDRDAO/2' page 
of the CD-Creator Settings.

:h2 res=806
x=right y=bottom
width=65%
height=100%
group=2.Create Multisession CD's.
:p.
Select in the 'Create Data-CD' Folder 'CD-Settings'. On the first page 
'CD type' you can specify the type of CD you want to burn.


:h2 res=807
x=right y=bottom
width=65%
height=100%
group=2.Create Boot-CD's.
:p.
Select in the 'Create Data-CD' Folder 'CD-Settings'. On the first page 
'CD type' you can specify the type of CD you want to burn.
Be sure to read the Boot-CD-Howto, and follow carefully all described steps.
The 'Boot-CD-Howto' can be obtained here&colon.
:link reftype=launch
object='netscape.exe'
data='http://www.geocities.com/SiliconValley/Sector/5785/bootcd/Boot_cw.htm'.
http&colon.//www.geocities.com/SiliconValley/Sector/5785/bootcd/Boot_cw.htm:elink.

:h2 res=808
x=right y=bottom
width=65%
height=100%
group=2.I can't write a CD-R/RW.
:p.
:ul compact.
:li.Make sure that the setup is 100% correct.
:sl.

:li.
:p.
Test your cdrecord installation by calling :hp2.cdrecord -scanbus:ehp2. on the
command line. If this doesn't show your devices, check your config.sys.
:p.
If you don't see your ATAPI devices the DaniAtapi.flt isn't installed correctly.
:p.
:esl.
:li.Don't use the cdrecord versions created by Nick Lepehin. They are incompatible. 
:li.If necessary read this tutorial twice.
:li.Use quality media supported by your recorder.
:li.Maybe updating your Writers flash-rom could solve the problem.
:li.Have a look at the logfiles in the logfiles directory.
:eul.

:h2 res=809
x=right y=bottom
width=65%
height=100%
group=2.FreeDB query doesn't work.
:p.
:ul compact.
:li.Make sure your email address is specified on the FreeDB page of the CD-Creator settings notebook. 
:li.You must have a direct connection to the internet. There's no proxy support.
:li.There must be some space on the installation drive and the :hp2.cddbdata:ehp2. directory must be writable.
:eul.


:h2 res=810
x=right y=bottom
width=65%
height=100%
group=2.I used shadows when creating the CD but don't see the files.
:p.
Make sure you've selected the right option on the :hp2.Special:ehp2. page of the folder settings
notebook. When using :hp2.Shadows only in the root:ehp2. only those shadows are used which are
in the data-CD folder itself. Any shadow located in subfolders or in a shadowed folder is lost when
creating the CD.
:p.
CDFS.IFS has a bug when using Joliet names. Sometimes files and folders are not visible. They
are indeed on the CD as you may check out on a W*n system. The files are always accessible using
ISO filenames though. There's currently no solution for this problem.



:h2 res=811
x=right y=bottom
width=65%
height=100%
group=2.No mount/unmount menues for ISO images.
:p.
The ISO-filesystem must be properly installed to use this feature. The Data-CD-Creator class
looks for the :hp2.Mount:ehp2. object to determine if the FS is present. This object
MUST have the ID <ISOFS_MOUNT>.


:h2 res=812
x=right y=bottom
width=65%
height=100%
group=2.TAO audio writing doesn't work with TEAC drive
:p.
TEAC drives (at least some of them) don't support the :hp2.-nofix:ehp2. parameter used when writing
audio CDs in TAO mode. Write in DAO mode when using a TEAC drive.


:h2 res=813
x=right y=bottom
width=65%
height=100%
group=2.The data folder suddenly stops collecting files.
:p.
This happens if your INI files are broken and the WPS can't query the name of
a file. Use an utility like checkini to fix your INI files. In some cases running an
INI file repair tool doesn't fix this error. If this happens to you go to the :hp2.Special:ehp2. 
page of the folder properties notebook and activate :hp2.Folders only in the root:ehp2.. Be
aware that shadows in shadow folders are not followed with this setting.

:h2 res=814
x=right y=bottom
width=65%
height=100%
group=2.I have strange errors when using the folders after upgrading.
:p.
You may have a mixed installation of the new release and a previous release.
Run the skript :hp2.checkinstall.cmd:ehp2. in the BIN directory of your installation
for a quick check. Deinstall the classes, restart the WPS and clean your INI files.
Browse all your disks for copies of :hp2.aucdfldr.dll:ehp2.. Remove any DLL you may find
except the one in the creator installation directory. After this cleanup reinstall the package. 

:h2 res=815
x=right y=bottom
width=65%
height=100%
group=2.I can't grab tracks from my SCSI-CD-Rom.
:p.
First check if your SCSI-chain is setup correctly (SCSI-Id's, Termination, 
Cable-lenght...). If this doesn't solve the problem detach your scanner from 
the chain. Don't forget to correct the termination if you don't have a SCSI-Card 
with autotermination.

:h2 res=816
x=right y=bottom
width=65%
height=100%
group=2.After using checkini some objects refuse to work.
:p.
Until now it is not clear what causes this malfunction (checkini, OS/2), cause 
this failure can't be reproduced. If this occurs go to the :hp2.Configuration and 
(Un)installation:ehp2. folder and do a 'Recreate WPS-Objects'. After that all 
Audio/Data-Cd-Creator objects work again.


:h2 res=818
x=right y=bottom
width=65%
height=100%
group=2.Update a set of ConvPak CDs.
:p.
To update a set of ConvPak CDs (Installation CD and Data CD) follow 
this description by Rainer Feuerstein&colon.
:ol.
:li.add/replace driver on disk0_1.img 
:li.(Install CD, \bootimgs\disk0_1.img) 
:li.cdbl.cmd, add driver, edit config.sys, cdbs.cmd 
:li.copy \bootimgs\disk0_1.img to Data CD directory 
:li.add/replace driver in OS2IMAGE\DISK_7\scsiadds 
:li.pack2 \OS2\BOOT\AICU160.ADD scsiadds /R 
:li.pack2 AICU160.ADD scsiadds /H&colon.\OS2\BOOT\AICU160.ADD /A 
:eol.
:p.
For eCS this has to be done for the first CD, for ConveniencePak some 
tricks have to be used, to cause the pair of Installation CD
and Data CD to function (the disk*.img must reside on the same position 
on both CDs). 
:p.
P.S. cdbl.cmd and cdbs.cmd are using loaddskf /y /q and savedskf /d /a 
to load and save the Images. 
As virtual Floppy use VFDisk 2.0 (http&colon.//hobbes.nmsu.edu/pub/os2/system/drivers/filesys/vfdisk20.zip)

:h2 res=819
x=right y=bottom
width=65%
height=100%
group=2.There's only noise on my decoded MP3-CD
:i2 refid=mp3writing.Only noise on the CD
:p.
Your writer needs the audio data in a different byte order. Go to the MP3-decoder page and select
:hp2.swap bytes when writing:ehp2..
:p.

:h2 res=820
x=right y=bottom
width=65%
height=100%
group=2.There's music with noise on my decoded MP3-CD
:i2 refid=mp3writing.Music with noise on the CD
:p.
The version of cdrdao you're using for audio writing is broken. Use the one coming with Audio-CD-Creator.
:p.
You may use other cdrdao releases when writing wave files (see below) or copying CDs. This cdrdao bug only affects
audio CDs when using MMIO procedures for decoding.

:p.
:nt.
If MMIO procedures are selected as the decoder :hp2.all:ehp2. audio files (wave files included) are written using
such a procedure and thus the bug take effect. Select z! or mpg123 as a decoder to write waves without an MMIO
procedure. You don't have to install one of these decoders.
:ent. 

:h2 res=821
x=right y=bottom
width=65%
height=100%
group=2.The playtime of MP3 files is wrong
:i2 refid=mp3writing.The playtime of MP3 files is wrong
:p.
This is a problem showing with VBR MP3 files. Use a recent release of the MMIOMP3 procedure. Get it
from:
:p.
:link reftype=launch
object='netscape.exe'
data='http://www.math.berkeley.edu/~roconnor/'.
http&colon.//www.math.berkeley.edu/~roconnor/:elink.

:nt.
If the playtime is calculated wrong writing of MP3 files in DAO mode will fail!
:ent.

:h2 res=822
x=right y=bottom
width=65%
height=100%
group=2.Playtime calculation fails
:p.
The folder shows the message :hp2.Calculating playtime...:ehp2. but nothing seems to happen.
:p.
MMOS2 must be installed. Probably the helper calculating the time fails to start because
of a missing DLL. Go to the General page 1 and deselect :hp2.Start hidden:ehp2. and 
:hp2.Close window on exit:ehp2. to read the error message of the helper.

:h2 res=823
x=right y=bottom
width=65%
height=100%
group=2.There's no audio information in details view
:p.
The CW-Multimedia classes must be installed for this feature. Get them from
:p.
:link reftype=launch
object='netscape.exe'
data='http://www.os2world.com/cdwriting'.
http&colon.//www.os2world.com/cdwriting:elink.
:p.
If the classes are already installed make sure the details class is set to :hp2.Audio-Shadow (CWAudioShadow):ehp2.
or :hp2.Ext.-Digital Audio (CWAudio):ehp2.. You can't display details for audio files and audio shadows at
the same time. This is a limitation of the WPS.

:h2 res=824
x=right y=bottom
width=65%
height=100%
group=2.Is it possible to copy CD's on the fly ?
:p.
Audio-Data-Creator is already prepared to copy CD's on the fly. As soon there 
is a non-broken cdrdao/2 (the VAC compiled versions with dll can't be used) 
1.1.6 (or higher) available and installed copy CD's on the fly will function.


:h2 res=826
x=right y=bottom
width=65%
height=100%
group=2.On the fly MP3 encoding doesn't work.
:p.
If the status dialog opens but nothing happens make sure you have set the grabber
parameters correctly. The only supported grabber for MP3 encoding is :hp2.cdda2wav:ehp2. from
the Cdrtools/Cdrecord distribution. 
:p.
To close the non functional dialog open the window list and select close.

:h2 res=870
x=right y=bottom
width=65%
height=100%
group=2.I have still problems.

.im contact.inc

:h2 res=871
x=right y=bottom
width=65%
height=100%
group=2.This stuff is a pile of crap thrown together by a moron.
:p.
Thank you for your warm and helpful words. I don't care about your opinion. If you
don't like this program just don't use it. It's really that easy...

