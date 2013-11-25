:p.
0.56 (20.Feb.2005)&colon.
:sl.
:ul compact.
:li.CD size query for on the fly data writing was broken. Even ignoring wasn't possible.
:li.If free space is to small for grabbing to wave files show an informational box. 
:li.CD-R/W Tools dialog added to switch list.
:li.Added *.iso to search list when opening an image file in a data folder
:li.Dropping of disk objects on creator folders no longer possible
:li.DVD-Tools dialog
:li.Preliminary DVD writing
:li.Code changes for better support of other languages
:li.Codepage conversion for tracknames derived from CDDB data (only CP850 so far)
:li.Codepage conversion for MP3 data derived from CDDB (CP850 only)
:li.Some settings weren't shown without WPS-wizard installed
:li.Addon scripts weren't installed
:eul.
:esl.
:p.
:p.

V0.55 (27.June.2004)&colon.
:sl.
:ul compact.
:li.Fixed folder scripts (for the toolbars) so they work with ORexx.
:li.One of the installation skripts was broken.
:eul.
:esl.
:p.
:p.

V0.54 (22.May.2004)&colon.
:sl.
:ul compact.
:li.WPS-wizard V0.5.0 or above is now a prerequisite for using the CD-Creator classes.
:li.Introduction of "Expert" and "normal" mode with different amount of settings and choices to make the start for new users easier.
:li.Check installation: ASPIROUT.SYS check failed if parameters were given.
:li.One of the "User menu" menu items was broken.
:li.User defined MP3 encoding settings didn't work.
:li.Resized dialogs of helper applications.
:li.Reworked settings pages.
:li.New 'About' settings page.
:li.Sometimes carriage returns in object names truncated flyover help for launchpad buttons
:li.Scanning the SCSI bus sometimes resulted in bogus entries in the listbox.
:li.updated installer to V0.1.7
:li.Priority setting for MP3 encoder
:li.M_CW*Folder class was unloaded (at least on eCS 1.01) by the system even if still in use by some
 awake objects leading to WPS freeze on next access of a CW*Folder object. This was for example triggered
 by deleting a CD-Creator folder. The reason is not clear but some hopefully successful workarounds were added.
:li.Percent bar for audio folders showing the occupied space on a CD-R.
:li.Message boxes now are modal to the window they belong to. This prevents that error messages are popping up in the background.
:li.About dialog is modal now.
:li.Added setup strings for MP3 encoder.
:li.Enabled metall look for some more dialogs (On the fly writing, image writing, track grabbing, TAO audio writing).
:li.&amp. characters in FreeDB track names chrashed the on-the-fly MP3 encoder.
:eul.
:esl.
:p.
:p.

V0.53 (16.Feb.2003)&colon.
:sl.
:ul compact.
:li.Support for CW-MM classes V0.2.4 and above
:li.Fix for the "incorrect call to routine: call wait.Position x+20, y+dy/2" problem. Thanks to Kris Buelens for the patch.
:li.Write DAO: always used the settings from the main options not the specified ones from the dialog.
:li.Write DAO: better handling of write errors.
:li.Decode MP3: added error handling for missing MP3 player.
:li.Data and audio folder: added even more error handling for bad setup. Does anybody read the documentation at all?!?
:li.Added help pages for cdrdao settings page 2 and 3
:li.Mp3 decoding with MMIOMP3 works now when selected form folder menu
:li.ID3 tags are preserved when decoding to wave (saved in EAs). You need CW-MM 0.2.5 or higher
:li.New installation checker for those refusing to read help pages, tutorials and Readme files.
:li.MP3 encoding using LAME.
:li.On the fly MP3 encoding directly from CD.
:eul.
:esl.
:p.
:p.

V0.52 (31.Jul.2002)&colon.
:sl.
:ul compact.
:li.Writing of certain MP3 files with MMIOMP3 failed due to wrong time calculation.
:li.Wrong setup page for cdrdao (target).
:li.cdcopy fixed. Failed when two devices were used.
:li.On the fly frontend crashed when image calculation failed.
:li.MP3 decoding frontend tried to decode wave-files when using z!.
:li.CD-size menu was broken.
:li.If volume name was too long writing failed.
:li.Fixed background painting for empty folder launchpads.
:li.Preparation for use of CW-MM classes v0.2.3 and above.
:li.'Create shadows' is default after initial installation
:eul.
:esl.
:p.
:p.

V0.51 (20.Apr.2002)&colon.
:sl.
:ul compact.
:li.Playing with z! in MP3 decode dialog failed when z!-path contained spaces.
:li.New progressbars
:li.Template creation changed to prevent duplicate or lost templates.
:li.'Test only' settings of templates are no longer overriden when creating the folder. Use with care when changing. 
:li.Fixed settings page handling. Sometimes the notebook switched to the background.
:li.CD-size query when writing DAO always used device 0,1,0.
:li.Information about buffer when writing DAO.
:li.Sometimes the WPS blocked when closing the settings until CTRL-Esc was pressed.
:li.Additional options for cdrdao weren't used.
:li.New CD-copy frontend with progressbars
:li.Different cdrdao settings for CD copy and audio writing
:li.New 'metal' look (go to the user interface page to disable it)
:eul.
:esl.
:p.
:p.
V0.50 (23.Dez.2001)&colon.
:sl.
:ul compact.
:li.1&colon.1 copy dialog now remains open after starting the grab.
:li.Saving of CDCopy settings (path, drive etc.)
:li.When creating and writing several ISO-images sometimes a wrong name for writing was used.
:li.Added some more setup strings
:li.Added 'About' to settings object
:li.Added folder hints
:li.Updated mp3info from 0.2.11 to 0.2.16
:li.Changed logfile names to more meaningful ones
:li.Added MP3 decoding to folder
:li.DAO audio CD handling rewritten. You may specify pregaps now.
:li.Added help for some more objects (use F1)
:eul.
:esl.
:p.
:p.
V0.49 (22.Sep.2001)&colon.
:sl.
:ul compact.
:li.'Create another' menu item was lost.
:li.Fixed some minor memory leaks.
:li.The object for decoding MP3s had a wrong parameter.
:li.Fixed a CD/session detection problem when creating multisession CDs.
:li.Added mounting/unmounting support for :link reftype=launch
object='netscape.exe'
data='http://www.geocities.com/SiliconValley/Sector/5785/index.html'.ISO-FS:elink.
:li.Image creation failed when spaces in the path.
:li.Removed WPS name support (only useful for FAT) because it caused way to much troubles like duplicate names preventing succesful writing.
:li.'=' in filenames weren't handled.
:li.Check size for imagefiles sometimes failed (mainly because of WPS names).
:li.Enhanced CD-copy program to write and save (bin-)images, too.
:eul.
:esl.
:p.
:p.

V0.48 (20.July.2001)&colon.
:sl.
:ul compact.
:li.Image creation failed if WPS-object title contained carriage returns.
:li.Writing several CDs and image creation at the same time possible.
:li.CD-size check when writing DAO audio always used the default device not the selected one. 
:li.void CWDataFolder&colon.&colon.cwEnableMultiSessionCntrls(HWND hwnd, BOOL bEnable) removed
:li.Autodetermination if the first or additional sessions are to be written in multisession mode.
:li.Window position are saved for the PM-helpers
:li.MP3-decoding frontend for z! (http&colon.//dink.org) added
:li.New setup strings
:li.Default templates are no longer created in addition to those created by the installer.
:li.Toolbar improved&colon. objects may be placed freely.
:li.1&colon.1 CD-Copy drive selection fixed
:li.Removed flickering when resizing folders
:eul.
:esl.

:p.
:p.
V0.47 (13.May.2001)&colon.
:sl.
:ul compact.
:li.Eject CD after blanking to force OS/2 to discard the filesystem cache entries.
:li.Improved script for audio cover creation. Thanks to Christoph Kloeters.
:li.Helper for CD-size querying rewritten
:li.Added some more setup strings.
:li.Added 'Show ATIP' to CDR-Tools.
:li.Using WPS names when creating CD -> correct names even if sourcefiles are on a FAT drive.
:li.Fixed a buffer overflow bug in querycddb/Pmcddb when used with CDs with a lot of tracks (>70).
:li.Fixed a parsing problem in querycddb/Pmcddb for very long tracknames (several lines).
:li.Tweaked dynamic priority control for DAO (Hysterese)
:li.Experimental: Increased priority of on-the-fly process.
:li.lnchmkis.exe, onthefly.exe and prntsize.exe no longer lock the shared memory during running.
:li.Creating of shadows when dropping objects on closed creator-folders not always worked. Files were moved instead.
:li.Filelist.lst file isn't written on CD anymore.
:li.Playtime added to MP3/waveinfo dialogs.
:li.Associated the MP3/Waveinfo dialogs with the respective files.
:li.Configurable context menus if :link reftype=launch
object='netscape.exe'
data='http://www.geocities.com/SiliconValley/Sector/5785/index.html'.WPS-Wizard:elink. is installed.
:li.User defined menus if :link reftype=launch
object='netscape.exe'
data='http://www.geocities.com/SiliconValley/Sector/5785/index.html'.WPS-Wizard:elink. is installed.
:eul.
:esl.

:p.
:p.
V0.46 (25.Feb.2001)&colon.
:sl.
:ul compact.
:li.Settings for flyover help
:li.Some polishing of settings pages for Warp 4
:li.Predefined grabber settings
:li.Option for TRANS.TBL files
:li.Priority settings for onthefly.exe changed
:li.Bugfix: When calling mkisofs for multisession the bus was omitted
:li.Launchpad toolbar
:li.Bugfix: CD was ejected after each track when in TAO mode.
:li.Audio CD writing with several writers at the same time possible. 
This doesn't mean simultaneous writing of the same data on several 
drives. (you need the new ASPI-router by Paul Ratcliffe for this feature).
:li.Bugfix: Because of a race condition renaming of waves sometimes failed.
:li.Grab status for Leech, cdda2wav and JCDRead.
:li.Now using z! for format query of MP3 (if available). So you may use all 
MP3s for writing z! is able to decode. Thanks to dink (http&colon.//dink.org) for 
adding the option. You need z! from 2001/02/23 or above.
:eul.
:esl.
:p.
:p.
V0.45 (12.Jan.2001)&colon.
:sl.
:ul compact.
:li.Data-CD-Creator&colon. optional use only files with archive bit
:li.Audio-CD-Creator&colon. Display format of tracks in confirm dialog by doubleclicking
:li.CD-Copy&colon. overflow of disk sizes fixed
:li.Setup string bug fixed which prevented successful installation
:li.Missing toolbar bitmaps fixed (really? please report)
:li.Use WarpSans on Warp 4 and above
:eul.
:esl.
:p.
:p.
V0.44 (02.Jan.2001)&colon.
:sl.
:ul compact.
:li.Mnemnonics
:li.Format-Check of Tracks before burning
:li.Enable Burnproof
:li.PMAUDCPY.EXE for easy 1&colon.1 CD-copy
:li.Show Atip info in the CD-R/W Tools (not in the standalone version)
:li.Fix&colon. CDDB support
:eul.
:esl.
:p.
:p.
V0.43 (Dec.2000)&colon.
:sl.
:ul compact.
:li.Fix&colon. CDR-Tools ignored speed settings (standalone version
 not fixed yet)
:li.Fix&colon.Recalculate time after track grabbing&colon. correct time display 
:li.some cosmetic GUI fixes
:li.Unlimited shadow nesting (experimental)
:li.Follow all shadows added
:li.CDDB support improved&colon. track listbox shows track names
:li.Special Sony multisession handling (and some OEMs like HP)
:li.Eject after writing added
:li.Options for CDRDAO/2 added
:li.Filename page revised
:li.CD-type page cleaned up
:eul.
:esl.
:p.
:p.
V0.42 (Oct.2000)&colon.
:sl.
:ul compact.
:li.mkisofs runs with increased priority when writing on the fly
 preventing buffer underruns in high load situations
:li.priority of mkisofs now changed when FIFO is filled preventing
 high CPU load during filling.
:li.added toolbars
:li.Cover creation for data CDs
:li.preliminary CDDB support for audio tracks
:li.Option to create shadows as default
:li.min FIFO size was limited to 4MB when using spinbutton on general page, now 0MB
:li.Fix&colon. No statusbar update when writing an image with FIFO=0MB
:li.Keyboard navigation added to folder
:eul.
:esl.
:p.
:p.
V0.41 (26.Jun.2000)&colon.
:sl.
:ul compact.
:li.Again&colon. resource DLL error fixed (really?)
:li.Support for z! as mp3 decoder. You need a version later than 17.june.2000 to use
 this feature. Thanks to dink for the added option.
:li.Last image path now saved
:li.Cdrecord options for data CDs weren't saved
:li.Some options for data folders weren't saved
:li.Grab helper didn't find wrapper.exe -> grabbing failed
:li.Preliminary support for cover creation. You need Photo>Graphics Pro
to use this feature.
:eul.
:esl.
:p.
:p.
V0.40 (11.Jun.2000)&colon.
:sl.
:ul compact.
:li.Preliminary boot CD support
:li.CD size check may be completely overriden
:li.Included mp3info to get size of MP3 files. See docs for the features
 of this program.
:li.allowing spaces in folders for DAO
:li.subdirectory for logfiles
:li.Cleaned up installation directory
:li.codepage support when creating data CDs with Joliet names  
:li.Some more icon improvements
:li.On the fly Multisession CD creation with cdrecord V1.8.1a05 and above
:li.some bugs fixed, mainly the annoying can't find resource DLL error
:eul.
:esl.
:p.
:p.
V0.39 (13.Apr.2000)&colon.
:sl.
:ul compact.
:li.Check for valid CDRecord/2 path for scan button
:li.Added log for scanbus
:li.scanbus&colon. CDRecord/2 started with DosExecPgm(),
 not DosStartSession()->failed on some systems
:li.new mp3decod.exe
:li.log for mp3decod.exe
:li.Audio-CD-Creator methods removed&colon.
  AddCdrecordOptionPage, AddGrabOptionPage,
  AddMpg123OptionPage, AddCdrdaoOptionPage,cwAddGeneralOptionPage
:li.improved hires icons by Klaus Staedtler
:li.Error 'Not enough free CD space' may be overridden
:li.First version of a tutorial
:li.Settings object for global settings
:li.CDRTools&colon. changed startData.InheritOpt to SSF_INHERTOPT_PARENT
 for DosStartSession() so modified environment
 by a skript is inherited. Needed when using e.g. BEGINLIBPATH.
:eul.
:esl.
:p.
:p.
V0.38 (23.02.2000)&colon.
:sl.
:ul compact.
:li.Writer setup is now more easy (drop down list for device selection)
:li.Fifo setting is now also used for DAO
:li.Improved icons for Hires
:li.Some help improvements
:li.Fix&colon. Language settings for keyboard sometimes changed (really fixed?)
:li.Fix&colon. CDRTools was broken since 0.37
:li.Fix&colon. Settings for hide and close window were lost
:li.Fix&colon. Another problem mit freeing shared memory removed
:eul.
:esl.
:p.
:p.
V0.37 (06.02.2000)&colon.
:sl.
:ul compact.
:li.DAO for audio CDs using cdrdao/2 with dynamic priority control
:li.Better option handling for the helper programs. Most options will be set by the front end now
:li.Fix&colon. Creation of 2. session image when merging previos sessions was broken since V0.36
:li.Fix&colon. Shared memory wasn't always freed properly preventing creation of CDs
:li.Some minor enhancements like better logfile handling 
:eul.
:esl.
:p.
:p.
V0.36 (15.01.2000)&colon.
:sl.
:ul compact.
:li.MP3 writing with Audio-CD-Creator using mpg123 (only 44100 Hz)
:li.Several bugs in install script removed (thanks to Christian Hennecke)
:li.Spaces are allowed in preparer name, author name etc. (thanks to Alexander Wittig).
:eul.
:esl.
:p.
:p.
V0.35 (10.12.99)&colon.
:sl.
:ul compact.
:li.Progress bar for image writing
:li.Bug removed&colon. The number of shadows in the data folder was limited.
:li.Some more bug fixes
:eul.
:esl.
:p.
:p.
V0.34 (01.11.99)&colon.
:sl.
:ul compact.
:li.Progress bar for audio CD writing
:li.Folder backgrounds (Thanks to Arjen Meijer)
:li.different cdrecord options for audio and data folders
:li.Allow spaces in audio folder names
:li.Audio-CD-C lost setting for tracknumbers
:li.Code cleanups and preparation for NLS
:li.removed member function cwLoadIniValues()
:eul.
:esl.
:p.
:p.
New with V0.33 (14.10.99)&colon.
:sl.
:ul compact.
:li.Status window with progress bar for on the fly writing
:li.Status window for image writing
:li.Perform size checks (CD and image) before writing
:li.Button and menu for size check
:li.Allow spaces in file names for data CDs
:li.Audio- and Data-CD-Creator share the same version numbers now.
:eul.
:esl.
:p.
:p.
V0.32 (22.08.1999)
:sl.
:ul compact.
:li.Option to control the behaviour of the CDRecord/2 and mkisofs windows (hide, close on exit)
:li.Improved online help
:li.Bug causing WPS lockup removed
:li.Allow spaces in track names 
:li.Various other small fixes
:li.Data-CD-Creator V0.11:
  :ul compact.
  :li.On the fly CD creation.
  :li.Entryfield for image name sometimes not accepted the entered name
  :li.Directories linked as shadows are now properly inserted into the image
  :eul.
:eul.
:esl.
:p.
:p.
24.04.1999 V0.31b
:sl.
:ul compact.
:li.Hotfix release
:eul.
:esl.
:p.
:p.
11.04.1999 V0.31
:sl.
:ul compact.
:li.More status windows
:li.CDRecord write log
:li.Total time is shown
:li.More online help
:li.First public release of Data-CD-Creator (V0.1)
:eul.
:esl.
:p.
:p.
23.01.1999  V0.3
:sl.
:ul compact.
:li.Grabber support added
:li.Online help
:eul.
:esl.
:p.
:p.
08.01.1999  V0.2
:sl.
:ul compact.
:li.First public version of Audio CDCreator
:eul.
:esl.
:p.


