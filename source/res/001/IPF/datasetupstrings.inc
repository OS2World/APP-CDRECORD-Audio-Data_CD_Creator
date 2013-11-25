:h2 res=902
x=right y=bottom
width=100%
height=100%
group=2.Setup strings for Data-CD-Folders.
:p.
Data-CD-Folders support the following setup strings in addition to the usual folder setup strings.
:table cols='20 15 40' rules=both frame=box.
:row.
:c.KEYNAME
:c.VALUE
:c.DESCRIPTION

:row.
:c.APPLICATION
:c.Title
:c.The name of the application this CD contains.

:row.
:c.DISPLAYHELPPANEL
:c.index
:c.Displays the helppanel 'index' of the Data-Cd-Creator helpfile.  

:row.
:c.FLDRCDTYPEOPTIONS
:c.1[+2][+...]
:c.This sets the CD-type&colon.
:p.
:p.
1:    Multisession
:p.
:p.
Only if user defined selected:
:p.
8:    Tracktype: data
:p.
16:   Tracktype: mode 2
:p.
32:   Tracktype: XA/1
:p.
64:   Tracktype: XA/2
:p.
128:  Tracktype: CDI
:p.
512:  Fixate the disk after writing
:p.
:p.
1024: Boot-CD
:p.
2048: User defined
:p.
:p.
Note&colon.
:p.
The options for 'Merge session' and 'First session' are no longer
used when writing multisession CDs. The classes do handle these possibilities now. 

:row.
:c.FLDRCREATEOPTIONS
:c.1, 2, 4
:c.Sets the create options of the folder.
:p.
:p.
1: Create image
:p.
2: Write image
:p.
4: On the fly

:row.
:c.FLDRFILENAMEOPTIONS
:c.1[+4][+...]
:c.Sets the filename options&colon.
:p.
:p.
1:   Allow 32 chars
:p.
4:   Joliet names
:p.
128: Shadows in the root only
:p.
:p.
Example&colon.
:p.
:p.
FLDRFILENAMEOPTIONS=5

:row.
:c.FOLLOWALLSHADOWS
:c.1/0
:c.When set to '1' shadows in shadow folders are allowed for the Data-CD folder.
:p.
:p.
Example&colon.
:p.
:p.
FOLLOWALLSHADOWS=1

:row.
:c.FREESHAREDMEM
:c.1
:c.This setup string is used by the helper programs to signal the folder that's safe to free the shared memory used for communication.
It's sent after the data is copied to local memory.

:row.
:c.IMAGENAME
:c.Path
:c.Sets the path to the ISO-image
:p.
:p.
Example&colon.
:p.
:p.
IMAGENAME=D&colon.&bsl.imagename.raw

:row.
:c.PREPARER
:c.Name
:c.The name of this CDs preparer.

:row.
:c.PUBLISHER
:c.Name
:c.The name of this CDs publisher.

:row.
:c.SETSTATUSTEXT
:c.Textstring
:c.This sets the text in the status line of the folder.

:row.
:c.VOLUMENAME
:c.Name
:c.Sets the volume name of the CD.
:p.
:p.
Example&colon.
:p.
:p.
VOLUMENAME=My_Backup

:row.
:c.WRITECONFIGTOFILE
:c.filename
:c.Writes the configuration to a given file. This includes general settings as
specified in the main settings notebook but also the current settings of the folder
this setup string is sent to.
:p.
:p.
Example&colon.
:p.
:p.
WRITECONFIGTOFILE=x&colon.\the_config.txt

:etable.










