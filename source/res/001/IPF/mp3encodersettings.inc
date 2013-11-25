:h2 res=5500.Mp3 encoder options page 1
:p.
Specify the path to LAME 3.93.1 or higher (the MP3 encoder supported) and the MP3 encoding options on 
this page if you want to create MP3 files from wave files or an audio CD.
:p.
You may choose the priority of the encoder process. It's not recommended to use a value higher
than :hp2.2:ehp2. or your system may get unresponsive. Read more about the priority setting :link  reftype=hd res=5501.here:elink.. 
:p.
The radio buttons give you access to some common encoding modes.
:ul.
:li.Use :hp2.VBR standard:ehp2. for standard (but quite good) quality, about 192Kbit.
:li.Use :hp2.VBR extreme:ehp2. for even better quality, about 256kBit.
:li.Use :hp2.ABR (average bitrate):ehp2. if you want to specify the average bitrate of your file. Note that the quality may not be as good as when using VBR encoding.
:li.Use :hp2.User defined:ehp2. if you want to have full control over the LAME options to be used.
:eul.
:p.Read the LAME documentation and the LAME help for an explanation of possible options.

:nt.
The user defined options field isn't only used when selecting :hp2.User defined:ehp2. but also when selecting one of the given presets.
In that case the inserted options will be added to the preset options.
:ent.

:h3 res=5501.Mp3 encoder priority
:p.
The following is mainly taken from the documentation coming with LAME V3.90.2:

:ul.
:li.:hp2.0 (Low priority):ehp2.
.br
Priority 0 assumes "IDLE" class, with delta 0.
LAME will have the lowest priority possible, and the encoding may be suspended very frequently by user interaction.

:li.:hp2.1 (Medium priority):ehp2.
.br
Priority 1 assumes "IDLE" class, with delta +31.
LAME won't interfere at all with what you're doing.
Recommended if you have a slower machine.

:li.:hp2.2 (Regular priority):ehp2.
.br
Priority 2 assumes "REGULAR" class, with delta -31.
LAME won't interfere with your activity. It'll run just like a regular process, but will spare just a bit of idle time for the system. Recommended for most users.

:li.:hp2.3 (High priority):ehp2.
.br
Priority 3 assumes "REGULAR" class, with delta 0.
LAME will run with a priority of a normal process.
Good if you're just running LAME by itself or with moderate user interaction.

:li.:hp2.4 (Maximum priority):ehp2.
.br
Priority 4 assumes "REGULAR" class, with delta +31.
LAME will run with a very high priority, and may interfere with the machine response.
Recommended if you only intend to run LAME by itself, or if you have a fast processor.
:eul.

:p.
Priority 1 or 2 is recommended for most users. 



:h2 res=5520.Mp3 encoder options page 2
:p.
Specify the path to your MP3 library on this page and how new MP3 files will
be named during encoding.
:p.
It's possible to directly grab audio data from a CD into an MP3 file when cdda2wav
is selected as grabber. The file may be placed into an audio folder as usual but
also into a directory specified by the MP3 library path. A directory tree is built
from the artist name and the album so the files are properly sorted.
:p.
The naming scheme for encoded MP3 files may be configured using the data
in the combination boxes. The filename may be built from at most four parts
separated by individual strings which may be inserted into the entry fields.
