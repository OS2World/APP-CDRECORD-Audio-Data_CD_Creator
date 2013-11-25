:h2 res=1100.Mkisofs setup
:p.
Use this page to specify the path to 'mkisofs'. Mkisofs is the program to build the ISO-image which will
be written to the CD. You can't write data files directly on the CD. It's always necessary to build an
appropriate CD filesystem first! This filesystem building may be done without an intermediate image file which
is usually called :hp2.On the fly writing:ehp2..
Mkisofs is included in the CDRecord/2 distribution (mkisofs.exe). 

:p.
You may give some additional options for mkisofs which are not covered by the other
pages of the settings notebook using the entryfield. Usually this isn't necessary. The only
exception to this rule is the :hp2.-graft-points:ehp2. option which is necessary for cdrecord distributions
:hp2.newer than 1.9a02:ehp2..
:p.
:nt.
:hp8.
There're Cdrecord/Cdrtools versions floating around with a broken implementation of this option (called Cdrtools:hp2.2:ehp2., note the :hp2.2:ehp2.). These releases
are NOT supported by Data-CD-Creator! Use only Cdrecord/Cdrtools packages from&colon.:ehp8.
:p.
:link reftype=launch
object='netscape.exe'
data='http://www.geocities.com/SiliconValley/Sector/5785/'.
http&colon.//www.geocities.com/SiliconValley/Sector/5785/:elink.
.br
:link reftype=launch
object='netscape.exe'
data='http://www.os2world.com/cdwriting'.
http&colon.//www.os2world.com/cdwriting/:elink.
:nt.
Klick to start Netscape with that URL.
:ent.
:ent.

Please read the section of the FAQ :hp2.Data CD creation fails with recent releases of mkisofs:ehp2.
in the tutorial!

:p.You may use Joliet filenames on the CD you want to create. See :link reftype=hd res=1400.filename options:elink.
for more information. To ensure special characters like Ž, ™, š contained in codepages different from the default
mkisofs copdepage (437) are mapped correctly to the Joliet unicode characters mkisofs is started with
the :hp2.-jcharset:ehp2. option. Data-CD-Creator checks if this translation is possible by checking the
current codepage of the system. If the necessary codepage isn't supported this will be indicated by displaying
:hp2.0:ehp2. for the used cp. You may disable the charset mapping using the checkbox if the codepage
detection fails. In that case you may select the cp on your own and enter the option (e.g. -jcharset cp850) into the
entry field. The following charsets are supported by mkisofs (V 1.12.1a05 and above):

:p.
cp437 cp737 cp775 cp850 cp852 cp855 cp857 cp860 cp861 cp862 cp863 cp864
cp865 cp866 cp869 cp874 iso8859-1 iso8859-2 iso8859-3 iso8859-4 iso8859-5
iso8859-6 iso8859-7 iso8859-8 iso8859-9 iso8859-14 iso8859-15 koi8-r

 

