This is the DjVu 3.0 Reference Library.

This code is intended to:
	- define the DjVu format
	- allow GNU projects to develop code that renders,
	  displays, and updates DjVu documents
	- allows the creation of unoptimized images from pre-segmented
	  image sources

This package does not contain value added libraries such as
our SDK, JB2 optimizer, segmentor, optimizer, or GUI products.

This code base, is what we are using for current product
development:  you will find features, like our message catalog,
that don't exist in any of our currently released products.


To build the reference library on a Unix platform:

	./configure
	make

To install: (as root)
	cp bin/* /usr/bin/.
	cp lib/* /usr/lib/.
	mkdir /etc/DjVu
	cp profiles/message.conf /etc/DjVu


To build under Windows:
	1. Build the src/libdjvu++ files.
	2. Build src/djvutools/reference files.
	3. Copy the profiles folder into the directory that contains your
	   executables.


