:h2 res=1400.Filename options

Specify the filename options for the files to be written to CD on this page. For more information on the meaning
of the options read the mkisofs documentation or click the items below. Keep in mind that the documentation
coming with mkisofs is usually more up to date.

:p.
If you want to create a CD readable with pure DOS check :hp2.8.3 characters names:ehp2..
This will create a CD with 8.3 ISO names (only uppercase letter). The longnames will be written to
TRANS.TBL files if the option is checked. 
:p.
:hp2.32 characters longnames (-l):ehp2. selects ISO longnames. These are names containing only uppercase letters,
underscores and a few more special characters. The original names will also be written to
TRANS.TBL files if the option is checked.
:p.
CDs for W*n systems should have :hp2.-J:ehp2. enabled. This is also useful for OS/2 CDs. The TRANS.TBL files
are always hidden in the Joliet tree. Unfortunately 
CDFS.IFS seems to have a bug so some CDs created with this option are not completely readable. But you can
always access all files using the ISO (long)names. You have to
activate Joliet support for OS/2 by adding the :hp2./W:ehp2. switch to the CDFS.IFS statement in the config.sys.
:p.
The generation of Rockridge names (-r) is enabled by default because it doesn't hurt and you need the Rockridge
extensions for creating multisession CDs. It comes handy if you mount such a CD on a *nix system, too.

:p.
For more information click on the links below.
:sl.
:li.:link reftype=hd res=1401.32 character longnames:elink.
:li.:link reftype=hd res=1402.8.3 characters names (DOS like):elink.
:li.:link reftype=hd res=1403.Create TRANS.TBL files:elink.
:li.:link reftype=hd res=1404.Generate Joliet filenames:elink.
:esl. 

:h3 res=1401.32 character longnames
:p.
From the mkisofs (V1.2b4) documentation:
:p.
-l Allow full 32 character filenames. Normally the ISO9660 filename will be in an 8.3 format 
which is compatible with MS-DOS, even though the ISO9660 standard allows filenames of up 
to 32 characters. If you use this option, the disc may be difficult to use on a MS-DOS system, 
but this comes in handy on some other systems (such as the Amiga). Use with caution.


:h3 res=1402.8.3 characters names (DOS like)
:p.
Set the iso9660 conformance level 1. With level 1, files may only consist 
of one section and filenames are restricted to 8.3 characters.

:h3 res=1403.Create TRANS.TBL files
:p.
-T Generate a file TRANS.TBL  in  each  directory  on  the CDROM, which 
can be used on non-Rock Ridge capable systems to help establish the correct 
file  names. There is also information present in the file that indicates 
the major and minor numbers  for  block  and  character devices, and each 
symlink has the name of the link file given.
:p.
The TRANS.TBL files are hidden on the Joliet tree.

:h3 res=1404.Generate Joliet filenames
:p.
From the mkisofs (V1.2b14) documentation:
:p.
-J Generate Joliet directory records in addition to regular iso9660 file names. This is primarily 
useful when the discs are to be used on Windows-NT or Windows-95 machines. The Joliet 
filenames are specified in Unicode and each path component can be up to 64 Unicode characters long.

