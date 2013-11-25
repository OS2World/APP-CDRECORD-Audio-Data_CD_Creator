
:sl.
:li.:link reftype=hd res=601.The -scanbus command does not work.:elink.
:li.:link reftype=hd res=604.The -fifo command seems not to work:elink.
:li.:link reftype=hd res=605.What's about the wildcards mentioned in the manual?:elink.
:li.:link reftype=hd res=606.I want to copy a CD. cdrecord dev=1,0 speed=2 -isosize /dev/scd0 does
not work. If I provide a drive letter or a directory I get the message 'no
read access' or 'permission denied':elink.
:li.:link reftype=hd res=607.How do I create an image file with mkisofs?:elink.
:li.:link reftype=hd res=608.How do I create an image of my whole partition?:elink.	
:li.:link reftype=hd res=609.I can't read the documentation.:elink.
:li.:link reftype=hd res=610.If I click the drives icon in the Warpcenter while burning my machine
hangs:elink.
:li.:link reftype=hd res=611.I've got OS/2 Warp 3 (red) on floppy disks and want to create an
installation CD:elink.
:li.:link reftype=hd res=612.I only see the first session of a multisession disk with my
SCSI-drive:elink.
:li.:link reftype=hd res=613.How do I get the ISO-image of a CD?:elink.
:li.:link reftype=hd res=615.How do I make a 1&colon.1 copy of a CD?:elink.
:li.:link reftype=hd res=616.OS/2 traps with a Yamaha CRW4416s:elink.
:li.:link reftype=hd res=617.CDRecord/2 throws out an error in -dummy mode with a Symbios Logic
host adapter:elink.
:li.:link reftype=hd res=619.CDRecord/2 can't allocate filehandles for more than about 13 tracks:elink.
:li.:link reftype=hd res=620.There's a SIGTERM message every time I burn a CD!:elink.
:li.:link reftype=hd res=621.CDRecord/2 complains about a wrong EMX option:elink.
:li.:link reftype=hd res=622.CDRecord/2 does not work anymore after installing my scanner software:elink.
:li.:link reftype=hd res=623.Filenames do not have special characters like 'Umlaute':elink.
:li.:link reftype=hd res=624.Does CDRecord/2 work together with RSJ cd writer?:elink.
:li.:link reftype=hd res=625.Does CDRecord/2 work together with UDF read/write support by IBM?:elink.
:li.:link reftype=hd res=626.Does CDRecord/2 work 'anti-coaster' technology CD-Recoders ?:elink.
:li.:link reftype=hd res=627.How can I identify if my CDRecorder fits with the CDR/RW Type I use ?:elink.
:esl. 

:h2 res=601
x=right y=bottom
width=65%
height=100%
group=2.The -scanbus command does not work.
:p.
This is a bug in the OS/2 version of cdrecord 1.8a8. It's partly fixed
in V1.8a15. Get a recent release for a fixed version.


:h2 res=604
x=right y=bottom
width=65%
height=100%
group=2.The -fifo command seems not to work
:p.
That's right for all versions up to V1.8a23. This feature is supported
with V1.8a24 and above.


:h2 res=605
x=right y=bottom
width=65%
height=100%
group=2.What's about the wildcards mentioned in the manual?
:p.
They are not working with versions before V1.8a19. You should upgrade to
a more recent release.


:h2 res=606
x=right y=bottom
width=65%
height=100%
group=2.I get the message 'no read access' or 'permission denied'
:p.
You can't mount a CD as an ISO-image with OS/2. So you have to use
mkisofs or mkhybrid to create such an image prior to burning. You can tie
mkisofs and cdrecord together using pipes:

mkisofs -l -L -R -J x&colon./ | cdrecord dev=1,0 -data -

So there is no image file necessary. I highly recommend to raise the
priority of the mkisofs process to prevent buffer underruns:

spe t28 mkisofs -l -L -R -J x&colon./ | cdrecord dev=1,0 -data -

Important: Use the version of mkisofs which is included in the cdrecord/2
distribution when working with pipes! Other versions may have got a bug
which causes opening of stdout in textmode. Also use cdrecord V1.8a15 or
above only.

Cdrecord/2 V1.8a15 runs with timecritical priority 31. The priority of
mkisofs must be <=30 to ensure cdrecord gets the necessary processor time.
V1.8a8 runs with normal priority 0. The versions V1.8a22 and above have got
a dynamic priority scheme with a maximum of timecritical 31. Depending on
your system you may adjust the priority of mkisofs.

As an alternative solution you may use READCD.EXE of V1.8a24 and above to get the
image of the CD. Then write it on your CD-R.

:p.
Have a look at the following question, too:
:p.
:link reftype=hd res=623.Filenames do not have special characters like 'Umlaute':elink.


:h2 res=607
x=right y=bottom
width=65%
height=100%
group=2.How do I create an image file with mkisofs?
:p.
Use the following command line:

mkisofs -l -L -R -J -o image.raw x&colon./dir_with_data

:p.
Have a look at the following question, too:
:p.
:link reftype=hd res=623.Filenames do not have special characters like 'Umlaute':elink.

:h2 res=608
x=right y=bottom
width=65%
height=100%
group=2.How do I create an image of my whole partition?
:p.
Use the following command line:

mkisofs -l -L -R -J -o image.raw x&colon./

:p.
Have a look at the following question, too:
:p.
:link reftype=hd res=623.Filenames do not have special characters like 'Umlaute':elink.

:h2 res=609
x=right y=bottom
width=65%
height=100%
group=2.I can't read the documentation
:p.
The documents are postscript files. Get ghostview/ghostscript from Hobbes
or LEO. It's a free postscript renderer to view and print those files.

:h2 res=610
x=right y=bottom
width=65%
height=100%
group=2.My Machine hangs If I click the drives icon in the Warpcenter
:p.
Cdrecord writes to the CDR without telling OS/2 anything about it. It
uses directly the SCSI-Bus on a rather low level. When opening the drives
folder the Warpcenter checks every drive. When doing so the write requests
of cdrecord and the read requests of the Warpcenter collide and the system
gets confused.

:h2 res=611
x=right y=bottom
width=65%
height=100%
group=2.I want to creat an OS/2 Warp 3 (red) CD from floppies
:p.
An installation CD may be done by simply copying the disks on the CD.
Create for every disk a directory and name it

 DISK_0 DISK_1  DISK_2  DISK_3  DISK_4
 DISK_5 DISK_6  DISK_7  DISK_8  DISK_9
 DISK_10DISK_11 DISK_12 DISK_13
 DISP_1 DISP_2  DISP_3  DISP_4
 PMDD_1 PMDD_2  PMDD_3

On disk 1 of your floppy installation boot disks modify the config.sys in
the following way:

SET OS2_SHELL=sysinst2.exe x&colon.\.disk_root_dir_on_CD

The same way you may install from a HD partition.

:h2 res=612
x=right y=bottom
width=65%
height=100%
group=2.I only see the first session of a multisession disk
:p.
Seems to be a problem with the way SCSI-drives access the second
session. There're AFAIK different methods to do that and if the OS2*.DMD
doesn't know the name of the drive you're lost. I've read in a newsgroup
that's possible to patch the *.DMD so it knows the right name afterwards to
solve this problem. But I don't know how to do that exactly. You can usualy
read the disk with an IDE-drive without problems.

:h2 res=613
x=right y=bottom
width=65%
height=100%
group=2.How do I get the ISO-image of a CD?
:p.
Use READCD.EXE included in V1.8a24 to read the contents of the CD into
an image file. See AN-1.8a19 for a description. The versions in
V1.8a22/V1.8a23 are buggy so you can't use them.


:h2 res=615
x=right y=bottom
width=65%
height=100%
group=2.How do I make a 1&colon.1 copy of a CD?
:p.
Use READCD.EXE included in V1.8a24 to read the contents of the CD into
an image file. See AN-1.8a19 for a description. The versions in
V1.8a22/V1.8a23 are buggy so you can't use them.


:h2 res=616
x=right y=bottom
width=65%
height=100%
group=2.OS/2 traps with a Yamaha CRW4416s
:p.
This problem is fixed with V1.8a24b.


:h2 res=617
x=right y=bottom
width=65%
height=100%
group=2.Errors with Symbios Logic host adapter
:p.
This problem is fixed with V1.8a24b.
If it still doesn't work try a driver version prior to V4.09/V4.10. V4.06 works
well and I had reports that V4.00 and V4.01 also work without problems.


:h2 res=619
x=right y=bottom
width=65%
height=100%
group=2.CDRecord/2 can't allocate filehandles for more than about 13 tracks
:p.
Should be fixed with V1.8a15 and above!. Put the following statement in
your config.sys:

SET EMXOPT=-h255


:h2 res=620
x=right y=bottom
width=65%
height=100%
group=2.There's a SIGTERM message every time I burn a CD!
:p.
The message is coming from the FIFO process being killed when CDRecord/2
is exiting. Since the CD is already burned nothing happens except this
message. The CD is not damaged! This is normal behaviour with the current
implementation.


:h2 res=621
x=right y=bottom
width=65%
height=100%
group=2.CDRecord/2 complains about a wrong EMX option
:p.
The following line is found in config.sys:

SET EMXOPT= -C -N -H1024

A: Change the H to h.

:h2 res=622
x=right y=bottom
width=65%
height=100%
group=2.CDRecord/2 does not work anymore after installing my scanner software
:p.
Some software changes the /ALL switch to /SHARE in the line

BASEDEV=OS2ASPI.DMD /ALL

Cdrecord/2 only works if /ALL is present!


:h2 res=623
x=right y=bottom
width=65%
height=100%
group=2.Filenames do not have special characters like 'Umlaute'
:p.
Umlaute like &ae., &oe., &ue. and other special characters may get lost when
writing Joliet CDs. You have to tell mkisofs which codepage you're using.
For german users this is codepage 850:
:p.
             mkisofs -l -L -R -jcharset cp850 -o image.raw x&colon./
:p.
The following codepages are avaiable with mkisofs from V1.8.1a5 and above:
:p.
cp437 cp737 cp775 cp850 cp852 cp855 cp857 cp860 cp861 cp862 cp863 cp864
cp865 cp866 cp869 cp874 iso8859-1 iso8859-2 iso8859-3 iso8859-4 iso8859-5
iso8859-6 iso8859-7 iso8859-8 iso8859-9 iso8859-14 iso8859-15 koi8-r


:h2 res=624
x=right y=bottom
width=65%
height=100%
group=2.Does CDRecord/2 work together with RSJ cd writer?
:p.
Yes.

:h2 res=625
x=right y=bottom
width=65%
height=100%
group=2.Does CDRecord/2 work together with UDF read/write support by IBM?
:p.
Yes. Naturally you can't write with CDRecord/2 on a UDF formatted disk, as UDF 
can't write on a CD-RW written with CDRecord/2. In both cases you first have 
to blank (=erase all informations) the CD-RW by using the 'CDR-Tools'.



:h2 res=626
x=right y=bottom
width=65%
height=100%
group=2.Does CDRecord/2 work 'anti-coaster' technology CD-Recoders ?
:p.
CDrecord/2 works with the following anti-coaster technologies&colon.
BURN-Proof Sanyo, Just-Link Ricoh, Super-Buffer Philips, Write-Proof TEAC, 
Safe-Burn Yamaha, Buffer Underrun Free MMC-3.
:p.
Except for Just-Link you have to enable it by using the page 2 of the 
general page of the CD-Creator settings.
:p.
CDRecord/2 now shows whether BURN-Free is active. This makes sense as 
Ricoh's "Just-Link" is enabled by default. Unfortunately, the way 
"Just-link" is implemented is not compliant with the MMC standard. 
You may also have a look at the pwrt*.log files situated in the \logfiles 
subdirectory to see if an anti-coaster works or not.

:h2 res=627
x=right y=bottom
width=65%
height=100%
group=2.How can I identify if my CDRecorder fits with the CDR/RW Type I use ?
:p.
Every CD-R has important info on the disc itself which is called ATIP 
(Absolute Time In Pre-groove) or pre-groove for short. Without this 
data/info the CD-R cannot be used by a writer. 
The ATIP contains the following info: 
:p.
The capacity of the CD-R 
The manufacturer of the CD-R 
The type of the CD-R (eg. Audio only) 
Supported writing speeds 
Absolute lead-in time 
Last possible address where data can be written 
etc...
:p.
Only a CD-Writer can read the ATIP (because like I said before this ATIP 
is only needed when writing a CD-R).
:p.
If you open the 'CDR-Tools', you find under 'Misc' 'Show atip info'.
Simply mark the checkboy and press 'Do it' with the CDR/RW inserted in 
your recorder. Watch the output screen. If you don't see the informations 
mentioned above your recorder can't work with these CDR/RW Type.
