:h2 res=404
x=right y=bottom
width=65%
height=100%
group=2.Create an audio CD from MP3 files
:i1 id=mp3writing.MP3 writing
Audio-Cd-Creator writes MP3s directly to a CD suitable for normal CD players. CD-Creator comes with
a version of :hp2.mpg123:ehp2. capable to decode MP3s to files.
It is recommened either to use 
:link reftype=launch
object='netscape.exe'
data='http://dink.org'.z&xclm.:elink. (klick to start Netscape with the page http&colon.//dink.org) 
or (preferred) the MMIOMP3 codec 
:link reftype=launch
object='netscape.exe'
data='http://www.math.berkeley.edu/~roconnor/MMIOMP3'.
http&colon.//www.math.berkeley.edu/~roconnor/MMIOMP3:elink.
for the decoding.

MP3 writing is done similar to normal audio CD creation. You may use real 
files and also shadows and mix MP3s with normal wave files in any order.
:p.
As there exist three possible ways to do the job (either using TAO or DAO) there 
are three descriptions.
:p.
:p.
:hp2.Writing MP3's in TAO mode (DAO must be unselected):ehp2.
:nt.
Using TAO enables you to write multitrack CD's, but they might fail to 
work with your music CD-Player (this is true especially for older players). Multitrack means each file will
be written as a single track with 2 seconds between each track.
:ent.
:ol.
:li.Open the CD-writing folder.
:li.Drag the :hp2.Create audio CD:ehp2. template somewhere and drop it in your file system. A new Audio-CD-Creator folder will be created. Open this new folder by double clicking it.
:li.Put your MP3s or shadows of them into the folder. You may also add wave files or wave file shadows.
Audio-CD-Creator will decode the MP3s and write them together with the waves on the CD.
:li.Order the files as desired and press :hp2.Write:ehp2. after selection of the write options. If you're uncertain about the meaning of the write options use the right mouse button to get help.
Audio-CD-Creator will first decode the MP3's to wav. After conversion they are burned without the need of 
any user interaction.
:artwork name='MP3TAO.bmp'.
:eol.

:note.The MP3 files must be in 44100Hz/16bit/2 Channels format, if not there takes no conversion
place. If you only hear some noise and no music your writer probably expects the data in swapped
byte order. Go to the :hp2.mpg123:ehp2. page of the Creator settings and select :hp2.Swap byte order:ehp2..
This problem only may occur with :hp2.mpg123:ehp2. not with :hp2.z!:ehp2..

:note.You can only write MP3 files this way in TAO mode. 

:note.You may put other files like text files into the folder, too. Audio-CD-Creator skips these
files when creating the CD.

:note.When using shadows to MP3s keep in mind that they will converted to wav and the wave files 
will be stored in the folder, so conversion may fail due to limited diskspace.

:note. To get multitrack capability select the 'nofix' checkbox. Don't forget to uncheck it when
writing the last track or use the -fix command of the CDR-Tools dialog.
:p.
:p.
:hp2.Writing MP3's converted to wav in DAO mode (DAO must be selected).:ehp2.

:nt.
All music cd's are in DAO.
:ent.
:ol.
:li.Open the CD-writing folder.
:li.Drag the :hp2.Create audio CD:ehp2. template somewhere and drop it in your file system. A new Audio-CD-Creator folder will be created. Open this new folder by double clicking it.
:li.Put your MP3s or shadows of them into the folder. You may also add wave files or wave file shadows.
:li.Now select first all MP3's by using 'edit' 'select all' from the menu bar. Press then the 'decode MP3 files' from
the button bar. 
:artwork name='decode.bmp'.
:li.Order the files as desired and press :hp2.Write:ehp2. after selection of the write options. If you're uncertain about the meaning of the write options use the right mouse button to get help.
:eol.

:note.Make sure the 'DAO' mode is selected. 
:note.Keep in mind that all MP3's will converted to wav and the wave files will be 
stored in the folder, so conversion may fail due to limited diskspace.
:p.
:p.
:hp2.Burn MP3 files without manual conversion to wav in DAO-mode.:ehp2.

:nt.
To work the MMIOMP3 codecs must be installed and selected as MP3 decoder.
:ent.

:ol.
:li.Open the CD-writing folder.
:li.Drag the :hp2.Create audio CD:ehp2. template somewhere and drop it in your file system. A new Audio-CD-Creator folder will be created. Open this new folder by double clicking it.
:li.Put your MP3s or shadows of them into the folder. You may also add wave files or wave file shadows.
:li.Order the files as desired and press :hp2.Write:ehp2. after selection of the write options. If you're uncertain about the meaning of the write options use the right mouse button to get help.
:eol.

:note.Make sure the 'DAO' mode is selected.

