.*This help panel is used in expert mode
:h2 res=2700.Mp3 decoding (all settings mode)
:p.
Enter the path to the MP3 decoder (not necessary when using the MMIOMP3 codec) on 
this page if you want to use MP3 files for the creation of audio CDs usable 
with an ordinary CD player. The files will be decoded and written
on the CD. Be sure the MP3 files are 16bit, 44.1kHz, 2 channels.
:p.
Audio-CD-Creator supports mpg123, :link reftype=hd res=2710.z&xclm.:elink. or :link reftype=hd res=2711.MMIO procedures:elink. (preferred) as a decoding backend. Select the appropriate radio button.
:p.
If you use mpg123 or the MMIO procedures for the decoding and only hear some noise after writing the MP3 to CD
your writer probably expects the raw data in
swapped byte order. Select the checkbox in that case. If you get still noisy CDs have a look at the FAQ
in the tutorial.
:nt.
Using z&xclm. or the MMIO procedures gives you the ability to decide where the 
decoded wave files are stored when using 'Decode MP3' from the context menu of 
MP3 files.
:ent.

:h3 res=2710.z&xclm.
z&xclm. is a full featured MP3 player with a complex VIO user interface written by dink. It supports streaming, playlists,
play control over pipes, id3 tag editing, decoding to waves and much more. And most important it sounds great!
Thanks to dink for adding a special decoding option so it may be used with Audio-CD-Creator. 
:p.
Get it from 
:link reftype=launch
object='netscape.exe'
data='http://dink.org'.
http&colon.//dink.org:elink.
:nt.
Klick to start Netscape with that URL.
:ent.

:h3 res=2711.MMIO procedures
:p.
MMIO procedures are used by OS/2 to play or decode multimedia files. One may extend the
playing ability of OS/2 just by adding an appropriate MMIO procedure (which is a single DLL).
:p.
Russell O'Connor has written MMIO procedures for MP3 and Ogg files. After installation OS/2
can read and play MP3 and Ogg files like any other audio file.
:p.
Get them from 
:link reftype=launch
object='netscape.exe'
data='http://www.math.berkeley.edu/~roconnor/'.
http&colon.//www.math.berkeley.edu/~roconnor/:elink.

:nt.
Klick to start Netscape with that URL.
:ent.

.*The following help panel is used in normal mode
:h2 res=2750.Mp3 decoding
:p.
If you only hear some noise after writing the MP3 files to CD as an audio disk your writer probably
expects the decoded, raw data in
swapped byte order. Select the checkbox in that case. If you get still noisy CDs have a look at the FAQ
in the tutorial.

:nt.It's possible to use :hp2.z&xclm.:ehp2. or :hp2.mpg123:ehp2. as an MP3 decoder if for some reason
the OS/2 MMIO procedures normally used are not appropriate for the task. If you want to do so go to the first settings page,
select :hp2.Show all settings:ehp2. and return to this page.:ent.
