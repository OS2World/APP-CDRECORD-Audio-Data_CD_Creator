:h2 res=1800.Special
:p.
Change the handling of shadows on this page.
:p.
:hp2.Follow all shadows&colon.:ehp2.
:p.
When creating the CD all shadows are followed. CD-Creator recurses in every
directory and builds a virtual tree from all folders, files and shadows. You may
put shadows in folders or in shadow folders, too.

:p.
:hp2.Shadows only in the root&colon.:ehp2.
:p.
Choose this if you want to discard any shadows which are not in the root of the
folder. This is the behaviour of releases prior to V0.43.

:p.
:nt.
The following two controls only work if the 'Follow all shadows' checkbox is set.
:ent.

:p.
:hp2.Only files with archive bit set&colon.:ehp2.
:p.
When checked only files with the archive bit set will be put on the CD. This means only
files touched since the last backup will be considered. This of course only works if
the backup software does support the archive bit.

:p.
:hp2.Reset archive bit after creating&colon.:ehp2.
:p.
When checked the archive bit will be reset after a successful write or image creation.
When doing a test write the bit will not be reset.

:p.
:hp2.Reset now...&colon.:ehp2.
:p.
Using this button you may reset the archive bit of all files in the folder and subfolders.
:hp8.Use with care!:ehp8.
