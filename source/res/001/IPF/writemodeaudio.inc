:p.
In write mode you can select on the left side of the window the write options for audio-CDs. Order the wavefiles in
the container as desired and press :hp2.Write:ehp2.. A tracklist dialog pops up where you can check the files 
and confirm the write. After doing that CDRecord/2 is called to actually burn the CD. The whole setup for CDRecord/2
e.g. which drive to use is done in the settings notebook.
:p.
Click the items below to get more information:

:sl.
:li.:link reftype=hd res=611.No closing check box:elink.
:li.:link reftype=hd res=615.DAO check box:elink.
:li.:link reftype=hd res=613.Test only check box:elink.
:li.:link reftype=hd res=614.Write:elink.
:esl.

:h3 hide res=611.No closing check box
:p.
Check this box to leave the CD unclosed after writing the last selected track. This is necessary if you want to append
more tracks later. You can close the CD later using the CDR-Tools. Closing the CD means writing the table of contents (TOC).
Audio players can only read properly closed CDs.

:h3 hide res=615.DAO check box
:p.
Check this if you want to write the CD in disk at once mode. Only when using DAO you can prevent the
insertion of a two second pause between each track. Cdrdao/2 must be installed which is the default.
:note.You need the MMIOMP3 IO procedure if you want to write MP3 files in DAO mode. See the tutorial for more info.

:h3 hide res=613.Test only check box
:p.
If checked the CD is written in test mode. This means all tasks are performed but the laser of the writer is turned of.
This is for example useful to check if the performance of the system is sufficient to write a CD while doing other tasks.

:h3 hide res=614.Write
:p.
Click this button to start the writing of the CD. A dialog pops up where you can check the wave files and
confirm the write. If you've checked 'Test only' everything is done with write laser turned of.
