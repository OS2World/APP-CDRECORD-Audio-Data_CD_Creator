:h2 res=3500.General setup
:p.
The settings on this page are only shown when :hp2.Show all settings:ehp2.
is selected on the first page. It's normally not necessary to change anything
on this page.

:sl.
:li.:link reftype=hd res=2901.Start hidden:elink.
:li.:link reftype=hd res=2902.Close window on exit:elink.
:li.:link reftype=hd res=2903.Always create shadows:elink.
:li.:link reftype=hd res=3502.Sony multisession fix:elink.
:esl.

:h3 res=2901.Start hidden
:p.
If checked the window of any helper process which is started in the background is hidden. You can always
make it visible using the window list to read the helper messages. The messages are recorded in the 
write logs in the Audio/Data-CD-Creator installation directory so you may read them later. Uncheck the
button to start the helper programs visible.

:h3 res=2902.Close window on exit
:p.
If checked the window of a helper program closes after termination of the process. The messages are
recorded in the write logs in the Audio/Data-CD-Creator installation directory.

:h3 res=2903.Always create shadows
:p.
Check this to create a shadow when dropping an object on a CD-creator folder. Otherwise the
standard operation for a folder will be performed which is usually a move operation.

:h3 res=3502.Sony multisession fix
:p.
When checked a workaround for Sony-CDR Drives (or Sony OEM drives, e.g. many HP) 
is enabled, so you can produce multisession CD's with these drives.
:p.
Cdrecord 1.11a06 and above changed the Sony writer handling so it may be this fix isn't
necessary anymore. Test it on your own.

