:h2 res=350.About page
.ce Welcome to Audio/Data-CD-Creator
:p.
This application allows you to create audio and data CDs in an easy and intuitive way. Have
a look at the tutorial for more information about the unique features of this software. 
:p.
Starting with V0.54 ADC supports two user modes. One for those wanting control over every
(sometimes esoteric) option available with CDRecord/2, mkisofs and the other helper applications.
The other is for users just wanting to write their CDs without hassles. The latter uses reasonable
defaults useful for the vast majority of users and hides most settings not necessary for daily use,
like for example notebook pages for path selection of helper applications when these are already
configured.
:p.
Change the :hp2.Show all settings:ehp2. checkbox to switch from one mode to the other. After
changing that setting the notebook will close and reopen with updated settings pages.
:p.
Read about the differences of the two user modes :link reftype=hd res=351.here:elink..

:h3 res=351.Mode differences
:nt.
It's recommended to install the MMIOMP3 IO-procedure and use the automatic configuration feature
of recent CDRecord/2 releases (e.g. CDRecord 2.00 final) to set the necessary options. By doing so
a minimal amount of options must be filled in normal mode.
:ent.
:p.In normal mode (:hp2.Show all settings:ehp2. not selected) the follwing pages will be hidden.
:ul.
:li.CDRecord/2 settings (if path is valid)
:li.Mkisofs settings (if path is valid)
:li.General settings
:li.User interface settings
:li.FreeDB server settings
:eul.
:p.
The grabber is defined as :hp2.cdda2wav:ehp2. and for MP3 decoding the MMIOMP3 IO-procedure
will be used if installed (recommended). If cdda2wav or the MMIOMP3 procedure are missing enhanced
notebook pages with additional options can be used to specify the necessary settings.
:p.
In enhanced mode (:hp2.Show all settings:ehp2. selected) all pages will be shown and the enhanced
version of notebook pages will be used.
