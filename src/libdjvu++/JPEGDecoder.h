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
//C- $Id: JPEGDecoder.h,v 1.2 1999-10-19 14:48:37 leonb Exp $

#ifndef _JPEGDECODER_H_
#define _JPEGDECODER_H_

#include "DjVuGlobal.h"
#ifdef NEED_JPEG_DECODER

#include "ByteStream.h"
#include "GPixmap.h"
#include "GException.h"
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"
#include <string.h>
#include <setjmp.h>

/** @name JPEGDecoder.h
    Files #"JPEGDecoder.h"# and #"JPEGDecoder.cpp"# implement an
    interface to the decoding subset of the IJG JPEG library.
    @memo
    Decoding interface to the IJG JPEG library.
    @version
    #$Id: JPEGDecoder.h,v 1.2 1999-10-19 14:48:37 leonb Exp $#
    @author
    Parag Deshmukh <parag@sanskrit.lz.att.com> 
*/
//@{

/** This class ensures namespace isolation. */
class JPEGDecoder {
public:
  /** Decodes the JPEG formated ByteStream */ 
  static GP<GPixmap> decode(ByteStream & bs);
};

//@}

#endif // NEED_JPEG_DECODER
#endif // _JPEGDECODER_H_
