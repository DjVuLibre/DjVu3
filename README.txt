This is the DjVu 3.0 Reference Library.

This code is intended to:
	- define the DjVu format
	- allow GNU projects to develop code that renders and
	  displays, and updates DjVu documents
	- allows the creation of unoptimized images from pre-segmented
	  image sources.

We this package does not contain value added libraries, like
our SDK, JB2 optimizer, segmenter, optimizer, or GUI products.
Even if we were to release these additional items as open source,
most likely, patent rights would exclude there use in the open
source community.

The provided code base, is what we are using for current product
development.  So, you will find features, like our message catalog
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
	1. First build the src/libdjvu++ files.
	2. Next build src/djvutools/reference files.

	3. Copy the profiles folder into the folder with your
	   executables.


