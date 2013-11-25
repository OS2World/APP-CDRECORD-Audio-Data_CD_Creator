:h2 res=901
x=right y=bottom
width=100%
height=100%
group=2.Setup strings for Audio-CD-Folders.
:p.
Audio-CD-Folders support the following setup strings in addition to the usual folder setup strings.
:table cols='20 15 40' rules=both frame=box.
:row.
:c.KEYNAME
:c.VALUE
:c.DESCRIPTION

:row.
:c.DISPLAYHELPPANEL
:c.index
:c.Displays the helppanel 'index' of the Audio-Cd-Creator helpfile.  

:row.
:c.FLDRWRITEFLAGS
:c.2[+8[+...]]
:c.Sets the folder write options:
:p.
:p.
2:  No fix
:p.
8:  Test only
:p.
16: DAO
:p.
:p.
Example&colon.
:p.
:p.
FLDRWRITEFLAGS=24

:row.
:c.FREESHAREDMEM
:c.1
:c.This setup string is used by the helper programs to signal the folder that's safe to free the shared memory used for communication.
It's sent after the data is copied to local memory.

:row.
:c.SETSTATUSTEXT
:c.Textstring
:c.This sets the text in the status line of the folder.

:row.
:c.SETTRACKLBTEXT
:c.index Trackname
:c.Sets the track 'index' in the track listbox to Trackname.
:p.
:p.
Example&colon.
:p.
:p.
SETTRACKLBTEXT=3 The third trackname
:p.
:p.
Index starts with 1.

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










