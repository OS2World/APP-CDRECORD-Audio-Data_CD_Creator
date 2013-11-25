:h2 res=301
x=right y=bottom
width=65%
height=100%
group=2. First time installation
:p.
Be sure to have obtained the tools described in the section :link reftype=hd res=303.Prerequisites:elink..

:ol.
:li.Unzip the Aspi-Router distribution in a directory of your choice. Add the following lines to your
config.sys file.
:p.
:hp2.BASEDEV=OS2ASPI.DMD /ALL:ehp2.
:p.A similar line may be already present in your config.sys depending on the software you've got installed.
:p.
:hp2.DEVICE=x&colon.\os2\boot\aspirout.sys:ehp2.
:p.Change the path to aspirout.sys as necessary.
:note.The switch :hp2.:color fc=red./ALL:ehp2.:color fc=black. must be present or CDRecord/2 will not work!

:li.If you have an Atapi Cd-writer add - after unzipping the files to x&colon.\OS2\BOOT\ - 
to your config.sys
:p.
:hp2.BASEDEV=DANIS506.ADD:ehp2.
:p.
:hp2.BASEDEV=DANIATAPI.FLT:ehp2.
:p.
:hp2.BASEDEV=OS2ASPI.DMD /ALL:ehp2.
:p.A similar line may be already present in your config.sys depending on the software you've got installed.
:p.
:hp2.DEVICE=x&colon.\OS2\BOOT\ASPIROUT.SYS:ehp2.
:p.Change the path to aspirout.sys as necessary.
:note.The switch :hp2.:color fc=red./ALL:ehp2.:color fc=black. must be present or CDRecord/2 will not work!
:note.Adding switches to DANIS506.ADD is normally :color fc=red.not:color fc=black. necessary. In contrary 
it can cause several faults.

:li.and :color fc=red.REM or delete from your config.sys :color fc=black. 
:p.
:hp2.BASEDEV=IBM1S506.ADD:ehp2.
:p.
:hp2.BASEDEV=IBMATAPI.FLT:ehp2.
:p.
:hp2.BASEDEV=IBMIDECD.FLT:ehp2.
:p.
If one of these drivers is in your config.sys CD-Record refuses to work.

:li.If you have an Mix of SCSI and Atapi (e.g. Scsi CD-Player, Atapi CD-Recorder) 
use the Atapi settings.

:li.No additional settings needed. If you have added SET CDR_DEVICE, 
SET_FIFOSIZE, SET CDDA-DEVICE or SET CDR_SPEED to your config.sys delete them. If other 
software needs these settings make sure that they are correct, otherwise your 
CD-Recorder fails, or you get trashed CD's.

:li.Unzip the CDRecord/2 and cdrdao/2 distributions in directories of your choice. The directories havn't
to be in your path.

:li.Unzip the Audio/Data-CD-Creator distribution into a folder of your choice. Run Install.exe in that directory which in turn starts a graphical configuration program. Follow the instructions given on the different pages and finally press
Finish. You may cancel the installation at any time. During the installation a DLL will be copied to your
x&colon.\os2\dll directory and a helpfile to your system help directory. Afterwards the Skript registers all
classes and creates a CD-Writing folder on the desktop.
:note. Do not delete any of the programs or directories in the installation directory or CD writing will not work!

:li.Reboot. Thats necessary because OS/2 has to load changed or added drivers.

:li.Open the Creator settings object in the CD-writing folder and adjust the settings according to your system.

:eol.