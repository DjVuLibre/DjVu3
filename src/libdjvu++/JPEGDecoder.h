//C-  -*- C++ -*- //C-
//C- Copyright (c) 1999 AT&T Corp.  All rights reserved.
//C-
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet website having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: JPEGDecoder.h,v 1.1 1999-10-06 22:54:25 orost Exp $

#ifndef _JPEGDECODER_
#define _JPEGDECODER_

#include <string.h>

#include "ByteStream.h"
#include "GPixmap.h"
#include "GException.h"

#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"
#include <setjmp.h>

/** @name JPEGDecoder.h
    
    Files #"JPEGDecoder.h"# and #"JPEGDecoder.cpp"# implement JPEG  decoding
    algorithm. 
    
    @memo
    JPEG decoder.
    @version
    #$Id: JPEGDecoder.h,v 1.1 1999-10-06 22:54:25 orost Exp $#
    @author
    Parag Deshmukh <parag@sanskrit.lz.att.com> */

//@{

class JPEGDecoder {
	public:
		~JPEGDecoder(){};
		JPEGDecoder(){};
    /** Decodes the JPEG formated ByteStream */ 

		static GP<GPixmap> decode(ByteStream & bs);

};

#endif

//@}
