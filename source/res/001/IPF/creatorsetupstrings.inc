:h2 res=903
x=right y=bottom
width=100%
height=100%
group=2.Setup strings for CD-Creator Settings.
:p.
The CD-Creator Settings notebook supports the following strings.
:table cols='20 15 40' rules=both frame=box.
:row.
:c.KEYNAME
:c.VALUE
:c.DESCRIPTION

:row.
:c.CDRDAOOPTIONS
:c.options
:c.Sets options for cdrdao when writing audio CDs. To remove the current options use
CLEAR as the value for the setup string.

:row.
:c.CDRDAOOPTIONS2
:c.options
:c.Sets options for cdrdao for 1&colon.1 copy, source. To remove the current options use
CLEAR as the value for the setup string.

:row.
:c.CDRDAOOPTIONS3
:c.options
:c.Sets options for cdrdao for 1&colon.1 copy, target. To remove the current options use
CLEAR as the value for the setup string.

:row.
:c.CDRDAOPATH
:c.path
:c.Sets the path to cdrdao for writing audio CDs..

:row.
:c.CDRDAOPATH2
:c.path
:c.Sets the path to cdrdao for 1&colon.1 copy, source.

:row.
:c.CDRDAOPATH3
:c.path
:c.Sets the path to cdrdao for 1&colon.1 copy, target.

:row.
:c.CDRECORDOPTIONSA
:c.options
:c.Sets global cdrecord options for audio CDs. To remove the current options use
CLEAR as the value for the setup string.

:row.
:c.CDRECORDOPTIONSD
:c.options
:c.Sets global cdrecord options for data CDs. To remove the current options use
CLEAR as the value for the setup string.

:row.
:c.CDRECORDPATH
:c.path
:c.Sets the path to cdrecord.
:p.
:p.
Example&colon.
:p.
:p.
CDRECORDPATH=D&colon.&bsl.CDBurn&bsl.cdrecord.exe

:row.
:c.DAYTIPENABLE
:c.1/0
:c.Enable (1) or disable (0) the tip of the day.
:p.
:p.
Example&colon.
:p.
:p.
DAYTIPENABLE=1

:row.
:c.DISPLAYHELPPANEL
:c.index
:c.Displays the helppanel 'index' of the Cd-Creator helpfile.  
:p.
:p.
Example&colon.
:p.
:p.
DISPLAYHELPPANEL=850

:row.
:c.FIFOSIZE
:c.size
:c.Sets the size of the write buffer to <size> MB. OS/2 only allows 64MB shared mem at all but it's unlikely you
can allocate that much RAM so only use a size smaller than 64MB. The following example sets the buffer to 16MB.
:p.
:p.
Example&colon.
:p.
:p.
FIFOSIZE=16

:row.
:c.FLDRHINTENABLE
:c.1/0
:c.Enable (1) or disable (0) the folderhints.
:p.
:p.
Example&colon.
:p.
:p.
FLDRHINTENABLE=1

:row.
:c.GRABBEROPTIONS
:c.options
:c.Sets options for the grabber. To remove the current options use
CLEAR as the value for the setup string.
:p.
:p.
Example&colon.
:p.
:p.
GRABBEROPTIONS=CLEAR

:row.
:c.GRABBERPATH
:c.path
:c.Sets the path to the grabber.

:row.
:c.GRABBERSELECT
:c.cdda2wav, leech, jcdread2
:c.Selects one of the known grabber. This sets also the right options.
:p.
:p.
Example&colon.
:p.
:p.
GRABBERSELECT=cdda2wav

:row.
:c.MKISOFSOPTIONS
:c.options
:c.Sets global options mkisofs. To remove the current options use
CLEAR as the value for the setup string.
:p.
:p.
Example&colon.
:p.
:p.
MKISOFSOPTIONS=-graft-points

:row.
:c.MKISOFSPATH
:c.path
:c.Sets the path to mkisofs.

:row.
:c.MP3DECPATH
:c.path
:c.Sets the path to the MP3 decoder.

:row.
:c.MP3ENCODERBITRATE
:c.bitrate
:c.Sets the bitrate for MP3 files when using average bitrate encoding. 
:p.
:p.
Example&colon.
:p.
:p.
MP3ENCODERBITRATE=192

:row.
:c.MP3ENCODEROPTIONS
:c.options
:c.Sets the option for the MP3 encoder. To remove the current options use
CLEAR as the value for the setup string.
:p.
:p.
Example&colon.
:p.
:p.
MP3ENCODEROPTIONS=CLEAR

:row.
:c.MP3ENCODERPATH
:c.path
:c.Sets the path to the MP3 encoder.

:row.
:c.MP3ENCODERPRIORITY
:c.1...4
:c.Sets the priority for the MP3 encoder process. 
:p.
:p.
Example&colon.
:p.
:p.
MP3ENCODERPRIORITY=1

:row.
:c.MP3ENCODERQUALITY
:c.STANDARD, EXTREME, BITRATE, USERDEFINED
:c.Sets the MP3 encoding quality. Note that you have to set the proper options if you select
USERDEFINED quality.
:p.
:p.
Example&colon.
:p.
:p.
MP3ENCODERQUALITY=STANDARD

:row.
:c.MP3SELECT
:c.z!, mpg123, MMIO
:c.Selects one of the known MP3 decoders. This does not set any additional options.
:p.
:p.
Example&colon.
:p.
:p.
MP3SELECT=z!

:row.
:c.MP3SWAPBYTES
:c.1/0
:c.Selects byte swapping for decoded audio data.

:row.
:c.MUSICLIBRARYPATH
:c.path
:c.Sets the path to the directory where encoded MP3 files will be saved. This path is used if
you encode a CD directly to MP3 files. 

:row.
:c.OPENCDRTOOLS
:c.1
:c.Shows the CDR-Tools dialog. Same as selecting the menu.
:p.
:p.
Example&colon.
:p.
:p.
OPENCDRTOOLS=1

:row.
:c.TBFLYOVERDELAY
:c.delay
:c.Sets the delay of the flyoverhelp to <delay> ms.
:p.
:p.
Example&colon.
:p.
:p.
TBFLYOVERDELAY=250

:row.
:c.TBFLYOVERENABLE
:c.1/0
:c.Enables/disables the flyoverhelp for the folder toolbar.
:p.
:p.
Example&colon.
:p.
:p.
TBFLYOVERENABLE=1

:row.
:c.WRITECONFIGTOFILE
:c.filename
:c.Writes the configuration to a given file. This means general settings as
specified in the main settings notebook. If you want also the settings of a particular folder
send this string to the folder in question.
:p.
:p.
Example&colon.
:p.
:p.
WRITECONFIGTOFILE=x&colon.\the_config.txt

:row.
:c.WRITESPEED
:c.speed
:c.The default writing speed as specified on the general page.
:p.
:p.
Example&colon.
:p.
:p.
WRITESPEED=4


:etable.










