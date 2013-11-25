:p.
In grab mode you have a listbox on the left side to choose the tracks you want to grab.
:ul.
:li.Press :link reftype=hd res=622.Refresh:elink. to
fill the listbox with the tracks of the inserted CD. Your grabbing drive and the grabber are specified in the settings
notebook.
:li.Choose the tracks you want to grab by clicking on them in the :link reftype=hd res=621.listbox:elink..
:li.Enter a name in the entryfield at
the top and press :hp2.Grab:ehp2..
:li.If you have enabled CDDB no name has to be specified.
:eul.
:p.
A status window pops up and the grabbing takes place in the background. A logfile named
:hp2.grab.log:ehp2. is created in the folder which contains any messages from :hp2.stderr:ehp2. of the grabber.
Examine it if anything goes wrong. You can close the folder while grabbing.
:p.
Click the items below to get more information:

:sl.
:li.:link reftype=hd res=621.Tracks listbox :elink.
:li.:link reftype=hd res=622.Refresh button:elink.
:li.:link reftype=hd res=624.Select all button:elink.
:li.:link reftype=hd res=623.Grab button:elink.
:esl.

:h3 hide res=621.Tracks listbox 
:p.
The tracks listbox shows the audio tracks of the inserted CD. Press :link reftype=hd res=622.Refresh:elink. to update the contents.
This is necessary whenever you change the CD or after the opening of an Audio-CD-Creator folder.
You can select tracks for grabbing simply by clicking on them. Multiple selections are possible. Press :link reftype=hd 
res=624.Select all:elink. if you want to grab an entire CD.

:h3 hide res=622.Refresh button
:p.
Press this button to fill the :link reftype=hd res=621.tracks listbox:elink. with the tracks of the currently inserted CD.
This is necessary whenever you change the CD in the grab drive. You can also use this button to deselect currently selected
tracks in the tracks listbox.

:h3 hide res=623.Grab button
:p.
Press this button to start the grabbing of the selected tracks.

:h3 hide res=624.Select all button
:p.
Press this button to select all tracks of the inserted CD for grabbing. Clicking on a selected track deselects it.
:p.
There is a 'cosmetic' bug when using cdda2wav from cdrecord 1.9a02 when selecting 
'all tracks'. You get a trap and a popuplog with the last file, don't worry all 
tracks are grabbed correctly. This bug doesn't occur when using cdda2wav from 
cdrecord 1.10x.
