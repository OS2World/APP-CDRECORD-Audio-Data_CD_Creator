:h2 res=2900.Writer setup
:p.
On this page you can select your writer using the drop down listbox. If the list is empty press the 
scan button to fill it with the devices on your SCSI or ATAPI bus.

:note. The scanning only works if you provide the path to CDRecord/2 on the appropriate 'Helpers' page.
That page is automatically inserted into this settings notebook if for some reason there's a problem with the path.
Make sure to close the notebook after entering the information and reopen it so the new path is read. 

:p.
You may adapt the settings for the write speed and the FIFO. The FIFO is a memory buffer used by
CDRecord/2 and cdrdao/2 to store some data for the CDR to make sure no buffer underruns occur
during writing. Choose a reasonable value for your system. It's no good idea to select a high value
which causes swapping. Memory areas swapped out to disk are rather useless for caching. OS/2
has a built in limit for this memory area of 64MB. Don't use this upper limit because it
will cause some negative effects for other processes.

:sl.
:li.:link reftype=hd res=3501.eject disk:elink.
:li.:link reftype=hd res=3503.Use BURN-Proof:elink.
:esl.


:h3 res=3501.eject disk
:p.
When checked the CD will be ejected when writing is finished. If you don't want 
to use this feature leave it unchecked, so the CD-R(W) stays in the tray.

:h3 res=3503.Use BURN-Proof
:p.
If you posses a CD-Recorder equipped with the :artwork name='burnproof.bmp' runin. 
technology enable the checkbox so Burnproof will be used.
:p.
With recent releases of cdrecord (1.11a06 and above) similar technologies by other vendors
are also supported like Ricoh Just-link.
:nt.
If your drive doesn't support this feature the checkbox is disabled.
ent.
:p.
How does BURN-Proof work?
.br
When the CD Recorder starts recording, 
the status of the buffer is constantly checked. Shortly before a 
buffer underrun might occur (typically when the buffer falls below 
10% of its maximum capacity), the CD Recorder will stop recording 
as of a specific location on the CD. The CD Recorder continues attempting 
to receive data from the PC and to refill the buffer. In the meantime, the 
BURN-Proof circuit determines where the last successful sector is written. 
Using the location of the last successful sector, the BURN-Proof circuit 
will position the Optical Pickup. As soon as the buffer has been refilled, 
the CD Recorder will start recording again. 

