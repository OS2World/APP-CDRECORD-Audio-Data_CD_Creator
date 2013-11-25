:h2 res=501
x=right y=bottom
width=65%
height=100%
group=2.Creating the image file
:isyn root=image.image imagefile ISO isoimage
:i1 roots='image'.Creating an image file
:i1 id=isoimage.ISO image file
:i2 refid=isoimage.Creation
:ol.
:li.Open the CD-writing folder.
:li.Drag the :hp2.Create data CD:ehp2. template somewhere and drop it in your file system. A new Data-CD-Creator folder will be created. Open this new folder by double clicking it.
:li.Open the settings notebook by clicking :hp2.CD-Settings...:ehp2. and select the type of CD you want to create.
If you select :hp2.multisession:ehp2. you have to put the target CD into the drive so Data-CD-Creator may
determine if it's the first session or a following session.
Only singlesession, multisession and Boot CD are valid choices.
:p.
:artwork name='cdproperties.bmp'.
:li.On the filename page select how filenames are written on the CD. If you want to create a
CD readable with plain DOS you have to leave :hp2.-l:ehp2. unchecked so an
image with only 8.3 names is created.
:p.If :hp2.Create TRANS.TBL files :ehp2. is checked a file is written in every directory describing
which short name is used for a longname.
:p.Generating :hp2.Joliet:ehp2. names will give you the longnames on W*n systems and OS/2 if the 
:hp2./W:ehp2. switch is present after CDFS.IFS.

:li.Fill in the information about the CD on the next page of the notebook. Close the notebook.

:li.Put all the files and folders you want to write into the image into the folder. You may also use shadows.
Data-CD-Creator will follow them and write the shadowed filesystem.

:note.Only one level of shadow linking is allowed when :hp2.Shadows only in the root:ehp2. is selected on the
:hp2.Special:ehp2. page. This means shadows in folders would be lost. Choose the other selection to follow
also shadows in shadow dirs. 

:li.Select :hp2.Create image only:ehp2. and insert the filename into the entry field. You may use the
:hp2.Browse:ehp2. button to find the destination directory.
:artwork name='createimage.bmp'.

:li.Click :hp2.Create image:ehp2. to build the image.
:eol.

:note.Make sure your diskspace is sufficient when creating the image. You may first use the :hp2.Check size:ehp2. button to calculate the image size of the selected files.


