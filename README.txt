This is the DjVu 3.5 Reference Library.

WHAT IS IT:
-----------
This code is intended to:
	- define the DjVu format
	- allow GNU projects to develop code that renders,
	  displays, and updates DjVu documents
	- allows the creation of unoptimized images from pre-segmented
	  image sources

This version is improved over the 3.0 version as follows:
	- The core is now internalized.  Internal strings are respresented
          with UTF8, with functions to convert to and from the Native
          multibyte format.  (See GUTF8String and GNativeString in GString.h)
        - Messages are now externalized.  The profiles/languages.xml file
          list the mapping between locale and message files (in XML).  When
          installing, simply copy the profiles/. directory to /etc/DjVu/.
          and you all messages will appear normally.
        - Support has been added for XML import and export of annotations,
          hidden text, and other forms of metadata.
        - Most constructors have been replaced by create() methods, that
          avoid the problem of having exceptions thrown from within a 
          constructor.

WHAT ISN'T IT:
--------------
This package does not contain value added libraries such as
our SDK, JB2 optimizer, segmentor, optimizer, or GUI products.

This code base, is what we are using for current product
development.  You will find features, like our message catalog,
that don't exist in any of our currently released products.

WHAT 3RD-PARTY SOURCE IS NEEDED:
--------------------------------

To build and use the reference library, you will need gcc-2.95.3,
a version of ksh.  Also, a version of libjpeg is recommended.

'ksh' may be obtained from: http://web.cs.mun.ca/~michael/pdksh/
Currently, 'ksh' is required to run the configuration scripts.

gcc-2.95.3 may be obtained from: http://gcc.gnu.org
You may use gcc-3.0.0 instead, but gcc must be built with thread
support to work properly.

jpeg-6b may be obtained from: http://www.ijg.org 
If you wish to build the reference library with JPEG support, then
copy the source code to DjVu3/src/3rd-party/libjpeg .

HOW TO BUILD UNDER UNIX:
------------------------

Once you have installed the above 3rd-party packages you are ready
to build the the Reference Library.

To build the reference library on a Unix platform:

	mkdir build
	cd build;../configure
	make

To build the reference library on a Unix platform, with libjpeg loaded at
runtime (requires working a working version of libdl, and libjpeg.so
preinstalled ):

	mkdir build
	cd build;../configure --with-jpeg=libjpeg.so
        make

To install: (as root)
	cp bin/* /usr/bin/.
	cp lib/* /usr/lib/.
	mkdir /etc/DjVu
	cp -r profiles/. /etc/DjVu

Alternatively, you may use the DJVU_CONFIG_DIR environmental variables, or
copy the profiles to a path relative to the executables.  i.e.

	cp bin/* /usr/local/bin/.
	cp lib/* /usr/local/lib/.
	cp -r profiles /usr/local/profiles

Note: Relative paths work based on argv[0], so symbolic links may cause
problems.


HOW TO BUILD UNDER WINDOWS:
------------------------

To build under Windows:
        1. Open the project file DjVu3/src/libdjvu++/libdjvu++.dsw
	2. Build the src/libdjvu++ files.
	3. Open the project file in src/djvutools/reference.
	4. Build the tools desired.
        5. The profiles directory should be located in either the same
           directory as the excutables, or one directory up.


Report bugs to: bcr@lizardtech.com


