.*Help page for expert mode
:h2 res=800.Grabber setup (all settings mode)
:p.
Use this page to specify your grabber and the grabber options. Any textmode grabber may be used e.g. :hp2.Leech:ehp2. or :hp2.cdda2wav:ehp2. (coming with the CDRecord/2 distribution).
:p.
Be sure to read the section about the grabber :link reftype=hd res=806.options:elink..
The changes take effect after closing of the notebook.
:p.
Get more information by clicking on the links:
:sl.
:li.:link reftype=hd res=803.Source CD-Rom drive:elink.	
:li.:link reftype=hd res=805.Path:elink.	
:li.:link reftype=hd res=801.Browse:elink.
:li.:link reftype=hd res=806.Options:elink.
:li.:link reftype=hd res=807.Add track numbers:elink.
:li.:link reftype=hd res=802.Undo:elink.
:esl. 


:h3 res=803.Source CD-Rom drive
:p.
Select the drive letter OS/2 assigned to your source drive. This information is used to
query the number of tracks when in grab mode.
:p.
Some grabbers (Leech) use this information to access the drive. Cdda2wav in contrast uses
the information given in the :link reftype=hd res=806.Options:elink. entry field.

:h3 res=805.Path
:p.
Enter the path to your textmode grabber into this entryfield or use the :link reftype=hd res=801.Browse:elink. button
to find the executable.

:h3 res=801.Browse
:p.
Use this button to find the executable of your grabber. A filedialog will pop up helping you browsing your
disks.

:h3 res=806.Options
:p.
Specify in the entryfield the grabber options. For the most common grabbers you 
can select the options from a dropdown list. No othersettings needed. If your 
grabber is not listed you must be familiar with your favourite grabber program 
to build the right commandline. Refer to the documentation of the program if 
you're in doubt. 
:p.
Audio-CD-Creator replaces :hp2.%1:ehp2. with the drive letter you've
chosen and :hp2.%2:ehp2. with the current track number. If your grabber 
needs a '&colon.' after the drive letter you have to specify it. 

:nt text='Example'.
Let's assume the grabber is called to grab track 2 from drive G&colon..
:p.
The options :hp2.%1&colon. -option1 -option2 # %2 /another_option:ehp2. will be translated into:
:p.
:hp2.G: -option1 -option2 # 2 /another_option:ehp2.
:ent.

:p.
Audio-CD-Creator assumes that the desired trackname has to be placed at the end of the grabber commandline.
You don't have to provide this name as an option. It is build from the folder location and the name you enter in
the entryfield of the main window. If your grabber does not expect the trackname as the last parameter contact the author. This version doesn't handle such a case.
:p.
Specify :hp2.-D<bus>,<device>,<lun> -H -t %2:ehp2. if you use cdda2wav. Be sure to specify the
right bus, device,lun for your grabbing device.

:h3 res=807.Add track numbers
:p.
Check this box if you want Audio-CD-Creator to insert the track number into the trackname e.g. track:hp2.02:ehp2..wav.
If your grabber inserts the numbers leave the box unchecked.
:p.
:hp2.cdda2wav.exe:ehp2. doesn't automatically add a number so check this setting if you use this grabber
which is coming with the CDRecord/2 package.

:h3 res=802.Undo
:p.
Click on this button to discard the changes you've made.


.*Help page for normal user mode
:h2 res=850.Grabber setup
:p.
Select the device you want to use for reading audio data from your source CD. It's
also necessary to specify the drive letter assigned to that device.
:p.
Get more information by selecting one of the following links.
:p.
:sl.
:li.:link reftype=hd res=808.Device for grabbing:elink.	
:li.:link reftype=hd res=804.Device drive letter:elink.	
:esl.

:nt.
If you want to use a particular grabber for reading the audio data other than
the default (cdda2wav) select :hp2.Show all settings:ehp2. on the first page. A new grabber settings page
will then allow to specify the necessary information. 

:ent.

:h3 res=808.Device for grabbing
:p.
This list shows the devices found in your system. Select the one you want to use
when reading tracks from an audio CD. It's possible to use your writer here. There's no need
to have an additional device because writing will be done after all grabbing is done. It's
possible to swap source and target CD before a burn process is started.


:h3 res=804.Device drive letter
:p.
Select the drive letter OS/2 assigned to your source device. This information is used to
query the number of tracks when in grab mode. Currently this information can't be
deduced from your selected device.

