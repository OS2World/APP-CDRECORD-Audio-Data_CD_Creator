
:h1 res=5200.CD copy dialog
:p.
This dialog is a frontend to cdrdao/2 to handle binary images in
a convenient way. Don't confuse these images with the files created by
Data-CD-Creator. They are not the same. The latter only contain the data
while the former also contain sector information and checksums for the writer.

:p.
Get further information by selecting the following items:
:ul.
:li.:link reftype=hd res=5201.Copy CD:elink.
:li.:link reftype=hd res=5202.Grab image only:elink.
:li.:link reftype=hd res=5203.Write image:elink.
:eul.

:h2 res=5201.Copy CD
:p.
Choose this if you want to copy a CD. Select your source (and target) device and the
target drive for the intermediate image file. Keep in mind that the image size is
around 700 MB so choose the drive accordingly.
:p.
:nt.Cdrdao needs to know which driver to use. This information is taken
from the cdrdao page 2 and 3 of the main settings notebook. So be sure the selected
device uses the selected driver or change the driver setting.
:ent.
:nt.Copy-on-the-fly works 'in principle'. As there is no 100% working cdrdao version 
available yet which is able to do copy-on-the-fly, you must use a intermediate file.


:h2 res=5202.Grab image only
:p.
After selecting this radio button you may select a destination for the image
file and the source device. Keep in mind that the image size is
around 700 MB so choose the drive accordingly. The file should have an extension
of :hp2..bin:ehp2.. A corresponding TOC file needed for later writing is
placed in the same directory as the bin file with an extension of :hp2..toc:ehp2..
:p.
:nt.Cdrdao needs to know which driver to use. This information is taken
from the cdrdao page 2 and 3 of the main settings notebook. So be sure the selected
source device uses the selected driver or change the driver setting.
:ent.

:h2 res=5203.Write image
:p.
If you already have an image file (and the corresponding TOC file) you
may write it on the CD using this option. Select the TOC file using
the browse button and choose the target device.
:p.
:nt.Cdrdao needs to know which driver to use. This information is taken
from the cdrdao page 2 and 3 of the main settings notebook. So be sure the selected
target device uses the selected driver or change the driver setting.
:ent.
