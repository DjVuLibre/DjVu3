#C-
#C- DjVu® Reference Library (v. 3.5)
#C- 
#C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
#C- The DjVu Reference Library is protected by U.S. Pat. No.
#C- 6,058,214 and patents pending.
#C- 
#C- This software is subject to, and may be distributed under, the
#C- GNU General Public License, Version 2. The license should have
#C- accompanied the software or you may obtain a copy of the license
#C- from the Free Software Foundation at http://www.fsf.org .
#C- 
#C- The computer code originally released by LizardTech under this
#C- license and unmodified by other parties is deemed the "LizardTech
#C- Original Code."
#C- 
#C- With respect to the LizardTech Original Code ONLY, and subject
#C- to any third party intellectual property claims, LizardTech
#C- grants recipient a worldwide, royalty-free, non-exclusive license
#C- under patent claims now or hereafter owned or controlled by
#C- LizardTech that are infringed by making, using, or selling
#C- LizardTech Original Code, but solely to the extent that any such
#C- patent(s) is/are reasonably necessary to enable you to make, have
#C- made, practice, sell, or otherwise dispose of LizardTech Original
#C- Code (or portions thereof) and not to any greater extent that may
#C- be necessary to utilize further modifications or combinations.
#C- 
#C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
#C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
#C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
#C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id: Makefile.am,v 1.7 2001-08-24 18:38:05 docbill Exp $
# $Name:  $



SUBDIRS    =  Test

INCLUDES   =  $(all_includes)


# ------------------------------------------------------------------------------
#   Define Primary Variables for Targets, Sources, Headers, etc.
# ------------------------------------------------------------------------------



## The target libraries to be built in this directory



lib_LTLIBRARIES = libdjvu.la





## The sources for libraries

## Populate include_HEADERS with .h files to be copied to install/include 
## directory.
include_HEADERS  = 


libdjvu_sources = \
	Arrays.cpp \
	BSByteStream.cpp \
	BSEncodeByteStream.cpp \
	ByteStream.cpp \
	DataPool.cpp \
	DjVmDir.cpp \
	DjVmDir0.cpp \
	DjVmDoc.cpp \
	DjVuAnno.cpp \
	DjVuDocEditor.cpp \
	DjVuDocument.cpp \
	DjVuDumpHelper.cpp \
	DjVuErrorList.cpp \
	DjVuFile.cpp \
	DjVuFileCache.cpp \
	DjVuGlobal.cpp \
	DjVuGlobalMemory.cpp \
	DjVuImage.cpp \
	DjVuInfo.cpp \
	DjVuMessage.cpp \
	DjVuMessageLite.cpp \
	DjVuNavDir.cpp \
	DjVuPalette.cpp \
	DjVuPort.cpp \
	DjVuText.cpp \
	DjVuToPS.cpp \
	GBitmap.cpp \
	GContainer.cpp \
	GException.cpp \
	GIFFManager.cpp \
	GMapAreas.cpp \
	GOS.cpp \
	GPixmap.cpp \
	GRect.cpp \
	GScaler.cpp \
	GSmartPointer.cpp \
	GString.cpp \
	GThreads.cpp \
	GURL.cpp \
	GUnicode.cpp \
	IFFByteStream.cpp \
	IW44EncodeCodec.cpp \
	IW44Image.cpp \
	JB2EncodeCodec.cpp \
	JB2Image.cpp \
	JPEGDecoder.cpp \
	MMRDecoder.cpp \
	MMX.cpp \
	UnicodeByteStream.cpp \
	XMLParser.cpp \
	XMLTags.cpp \
	ZPCodec.cpp \
	debug.cpp \
	parseoptions.cpp


libdjvu_headers = \
	Arrays.h \
	BSByteStream.h \
	ByteStream.h \
	DataPool.h \
	DjVmDir.h \
	DjVmDir0.h \
	DjVmDoc.h \
	DjVuAnno.h \
	DjVuDocEditor.h \
	DjVuDocument.h \
	DjVuDumpHelper.h \
	DjVuErrorList.h \
	DjVuFile.h \
	DjVuFileCache.h \
	DjVuGlobal.h \
	DjVuImage.h \
	DjVuInfo.h \
	DjVuMessage.h \
	DjVuMessageLite.h \
	DjVuNavDir.h \
	DjVuPalette.h \
	DjVuPort.h \
	DjVuText.h \
	DjVuToPS.h \
	GBitmap.h \
	GContainer.h \
	GException.h \
	GIFFManager.h \
	GMapAreas.h \
	GOS.h \
	GPixmap.h \
	GRect.h \
	GScaler.h \
	GSmartPointer.h \
	GString.h \
	GThreads.h \
	GURL.h \
	IFFByteStream.h \
	IW44Image.h \
	JB2Image.h \
	JPEGDecoder.h \
	MMRDecoder.h \
	MMX.h \
	Template.h \
	UnicodeByteStream.h \
	XMLParser.h \
	XMLTags.h \
	ZPCodec.h \
	debug.h \
	parseoptions.h



libdjvu_la_SOURCES =  $(libdjvu_headers) $(libdjvu_sources)













