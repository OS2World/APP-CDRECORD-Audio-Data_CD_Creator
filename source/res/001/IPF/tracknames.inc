:h2 res=855
x=right y=bottom
width=65%
height=100%
group=2.FreeDB tracknames
:i2 refid=customization.Tracknames
:p.
If sufficient information is provided on the :hp2.FreeDB:ehp2. page of the settings notebook tracknames
of waves are queried from a FreeDB server during grabbing. The information is processed by the REXX skript
:hp2.renwave.cmd:ehp2.. If you want to change the naming style for grabbed wavefiles adapt this skript. 
:p.
The information from the database is also stored in the extended attributes of the wavefile so it isn't lost
even after renaming of a file and may for example be used when encoding MP3s to build the tag information.
See the :hp2.Keyword:ehp2. EA on the third file page of the settings notebook of a wave for more information.
These values may even be read using REXX. See :hp2.renwave.cmd:ehp2. or :hp2.REXX Tips &amp. Tricks:ehp2. for
some code. 
:p.
If you've installed the CW-Multimedia classes V0.2.4 or higher the track information is also stored in some special
multimedia EAs. That information is preserved even if you convert the audio files from one format to another
using the multimedia classes or the creator classes.


