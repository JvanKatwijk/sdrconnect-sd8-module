
# FT8 module for SDRconnect

-------------------------------------------------------------------

![1.1](/res/pictures/front-picture.png?raw=true)

--------------------------------------------------------------------------
Important:
The FT8 Module (as other modules) use the **WEBSOCKET** interface to
the SDRconnect system.
Be sure that the selector to enable the WebSocket Server in SDRconnect
is enabled, it is probably nit enabled by default.

![1.1](/res/pictures/preferences.png?raw=true)

--------------------------------------------------------------------------
About ft8-module
-------------------------------------------------------------------------

ft8-module is a separate program dedicated to decoding FT8 messages
transmitted in the various radio amateur bands for use with SDRConnect
as front end

The program is written in C++ and uses Qt (version 6) and
qwt (6.3) for the GUI and websocket handling.

To ease use, precompiled versions (a Windows installer and an AppImage
for x64 based Linux boxes) are available.

For generating an executable, see the relevant section of this README

--------------------------------------------------------------------------

It us assumed that the reader is familiar with FT8 transmissions and
messages, FT8 transmissions consist of short encoded messages, transmitted on
defined regions within different amateur bands.

----------------------------------------------------------------------
About starting the program
----------------------------------------------------------------------

The program shows a GUI which is divded into two parts,
 * the left hand side contains the controls,
 * the right hand side shows decoded messages.

![1.1](/res/pictures/picture-1.png?raw=true)

of course, on program startup a connection must be made with an SDRconnect
server. As can be seen on the picture above,n IP address can be
specified in a small widget on top of the right half of the GUI.

The spinbox on the right is prefilled with the number 5454 which is the
default port number where the SDRconnect server is listening.

![1.2](/res/pictures/picture-2.png?raw=true)

FT8 is transmitted on specified frequencies, therefore ft8-module 
has a preset list.
Touching the button labeled **presets* shows the list,
touching a frequency on the list
instructs the software to switch over to the frequency specified.

![1.3](/res/pictures/picture-3.png?raw=true)

A **select freq** button is just below the button for the preset list.
Touching that button shows a small window where a frequency can be
set (of course SDRconnect itself has plenty possibilities of setting a frequency). The current frequency is shown on top of the left half of the GUI.

The currently selected frequency - regardless the way the selection was made -
can be added to the preset list by touching the button labeled "frequency save".

-------------------------------------------------------------------------
About process parameters
------------------------------------------------------------------------

A few parameters for the decosing process can be set

 * a spinbox with a (default) value 20 tells the decoder how many iterations
should be made over the raw message data. Experience shows that the default
number works well;
 * a spinbox with a (default) value of 4000 tells the decoder to set a
band filter with the specified width.

 * the combobox labeled **All items** allows choosing betwee showing all
messages (which is the default) or just the **CQ** messages.

-------------------------------------------------------------------------
Saving messages
-------------------------------------------------------------------------
If all goed well, the right hlf of the GUI shows messages that are received
and decoded. 

The button labeled **save output** controls whether the messages are also
saved in a **.csv** type file. If touched a small menu appears where
a filename can be selected.

------------------------------------------------------------------------
Using the pskReporter
------------------------------------------------------------------------

Messages received can be reported to the pskReporter, and subsequently,
it can be made visible on the map  where the originators of the messages 
were.

Connecting to the pskReporter requires some form of identification,
touching the button labeled "id" shows a small menu where call, location
and type of antenna can be filled in.
(the program will remeber this type of data between program invocations).
If given, the label **your call** shows the entered callsign.

Assuming the identification data is set, touching the button **Reporter**
will result in

 * starting a browser connected to the pskReporter server;
 * sending each received message to the pskReporter.


![1.2](/res/pictures/picture-4.png?raw=true)

-----------------------------------------------------------------------
Building an executable from sources
------------------------------------------------------------------------

While for both Linux (x64) and Windows precompiled versions are available
it is of course possible to build an executable yourself.

As said, ft8-module is built using C++ as implementation language
with use of the excellent framework of Qt for GUI, websockets etc.

So for building a executable one needs at least g++ and Qt6.
I have been using Qt6.2 (which is the supported version under Ubuntu 22)
and 6.9 (which runs on my Fedora development platform.)

The directory contains a file "qt-ft8.pro" that can be used to
generate a Makefile. In order to do so, Qt6 and **qmake6** need to be
installed.

The AppImage is built on Ubuntu 22, the required libraries and programs can be installed as given below (assuming the default Qt version is used)

 *   sudo apt-get update
 *   sudo apt-get install git
 *   sudo apt-get install cmake
 *   sudo apt-get install build-essential
 *   sudo apt-get install g++
 *   sudo apt-get install pkg-config
 *   sudo apt-get install libfftw3-dev
 *   sudo apt-get install zlib1g-dev 
 *   sudo apt-get install libusb-1.0-0-dev
 *   sudo apt-get install mesa-common-dev
 *   sudo apt-get install qt6-base-dev
 *   sudo apt-get install qt6-multimedia-dev
 *   sudo apt-get install libqt6websockets6-dev

As said, calling qmake6 from within the directory generates a makefile,
running the makefile generates an executable.

Copyrights
====================================================================

	Copyright (C)  2026
	Jan van Katwijk (J.vanKatwijk@gmail.com) 
	Lazy Chair Computing

	Copyright of libraries used - Qt-DAB, fftw,
	libusb-1, - is gratefully acknowledged.
	
	Some functions in the core of the decoder, in particular
	the ldpc decoding, is - with minor modifications -
	taken from Karlis Goba.
	All rights of Karlis Goba is gratefully acknowledged.

	ft8-module - the sources and the precompiled versions -
	is distributed under e GPL V2 library,  in the hope that
	it will be useful, but WITHOUT ANY WARRANTY; without even the
	implied warranty of MERCHANTABILITY or FITNESS FOR A
	PARTICULAR PURPOSE.  See the GNU General Public License for 
	more details.


