:h2 res=408
x=right y=bottom
width=65%
height=100%
group=2. Adjust the Pregap in DAO mode

:p.
The pregap is the pause before each track. Usually this pause is silent but you may do some tricks with it.

:ul.
:li.The  standard requires that an audio disc begins with a two-second "pregap" (=150 sectors) before the first track.
The actual starting point of that first track is specified in the TOC, so the pregap can be made longer than two seconds
 - the equivalent of 300 KB - without confusing most players. By setting a larger pregap, information can be
stored and hidden from 'normal' operations. Conventional mastering programs do not give you access to the pregap and do
not provide ways to use it if you get there.
:li.On Compact discs multimedia data may be placed in an expanded "pregap" area of the disc prior to TRACK 1 with the
audio placed in its normal location starting at TRACK 1 [A.K.A. CD-ROM Ready]. This is currently the dominant format
for enhanced CDs because it requires no special hardware or software drivers.
:li.The pregap, actually "hides" the computer data at the front of the audio CD between index 0 and index 1 of track one. 
A CD player still thinks it's an audio CD, while a computer is fooled into thinking that it's a CD-ROM. Pregap 
improves upon Mixed Mode by not allowing the audio user to access the Enhanced CD track directly, but other 
problems exist. Don't rewind on such CDs from the beginning of track 1 into the pregap. The computer data may
harm your audio equipment when played as audio. 
:li.The pregap may also be used to create unusual pauses between audio tracks when writing DAO. In general
the pause is two seconds long but you may extend it if you wish. Additionally it's possible to put audio
data into the pregap area of a track which strictly spoken violates the standard which defines silence for
the pause. Anyway any CD player plays this data (have a look at your favourite Live album ;-)). 
:eul.
:p.
:p.
The pregap feature in ADC may be used for audio samplers written in DAO mode. Without a gap every song
would immediatly be followed by the next. This isn't always what you want especially if there's no silence
at the end of a track. On the other hand live recordings shouldn't be interupted by a 2 second pause.
:p.
How to specify pregaps.
:ol.
:li.To adjust the pregap with audio-CD-creator select first 'Write', so the following subwindow opens
:artwork name='dao.bmp'.
:li.Now do a right click on the selected track and choose 'define pregap' from the menue 
:artwork name='pregap.bmp'.
:li.Here you can change the pregap to a size different than the standard 2 secs. When checking
the box the new value will be copied to every track.
:eol.
:p.
At the moment it's not
possible to put data into the pregap. If you want to do so save the TOC (table of content) using the
menu, edit it as described in the cdrdao manual and write your CD using the commandline.

:p.
:nt.
It's not possible to change the pregap of the first track. That's not a bug but intended because
you can't place any data into this gap yet. So there's no benefit in changing it. Instead of using the menu
you may also ALT-click on the pregap field of the container to change the value.
:ent.



