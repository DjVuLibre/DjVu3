This is the DjVu Viewer 3.5 

WHAT IS IT:
-----------

This package contains source code for a viewer and a Netscape plugin
for displaying DjVu documents.


WHAT IS NEEDED TO BUILD THE VIEWER:
--------------------------------

To build the viewer, you will need DjVu reference library,
QT 2.x or above, gcc-2.95.3 and a version of ksh. 

QT X11 Free Edition may be obtained from: http://www.trolltech.com 

'ksh' may be obtained from: http://web.cs.mun.ca/~michael/pdksh/
Currently, 'ksh' is required to run the configuration scripts.

gcc-2.95.3 may be obtained from: http://gcc.gnu.org
You may use gcc-3.0.0 instead, but gcc must be built with thread
support to work properly.

DjVu reference library may be obtained from:
http://www.lizardtech.com/products/djvu/referencelibrary/DjVuRefLib_3.5.html

HOW TO BUILD:
-------------

Once you have installed the required packages you are ready
to build the the viewer. 

To build:

	mkdir build
	cd build; ../configure-unixgui 
          (for a list of configure options, run "configure-unixgui --help")
	make
         
To install: (as root)
        cp src/unixgui/nsdejavu/nsdejavu.so $MOZILLA_HOME/plugins/.
        mkdir $MOZILLA_HOME/DjVu
        cp bin/djview $MOZILLA_HOME/DjVu/.
	cp lib/* /usr/lib/.
	mkdir /etc/DjVu
	cp -r profiles/. /etc/DjVu
       
        where MOZILLA_HOME is your mozilla/netscape 6.x installation 
        directory. For netscape 4.x, you may use $HOME/.netscape
        and define NPX_PLUGIN_PATH if needed.


Report bugs to: bcr@lizardtech.com


