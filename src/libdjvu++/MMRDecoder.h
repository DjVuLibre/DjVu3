//C-   -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.  All rights reserved.
//C-
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet website having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: MMRDecoder.h,v 1.4 2000-01-31 21:14:45 leonb Exp $

#ifndef _MMRDECODER_H_
#define _MMRDECODER_H_

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GSmartPointer.h"
#include "ByteStream.h"
#include "JB2Image.h"


/** @name MMRDecoder.h
    Files #"MMRDecoder.h"# and #"MMRDecoder.cpp"# implement a 
    CCITT-G4/MMR decoder suitable for use in DjVu.  The main 
    entry point is function \Ref{MMRDecoder::decode}.

    The foreground mask layer of a DjVu file is usually encoded with a
    #"Sjbz"# chunk containing JB2 encoded data (cf. \Ref{JB2Image.h}).
    Alternatively, the qmask layer may be encoded with a #"Smmr"#
    chunk containing a small header followed by MMR encoded data.

    This encoding scheme produces significantly larger files. On the
    other hand, many scanners a printers talk MMR using very efficient
    hardware components.  This is the reason behind the introduction
    of #"Smmr"# chunks.

    The first three header bytes have values #0x42#, #0x42#, #0x54#
    (i.e. #"MMR"#).  The next bytes is similar to TIFF's
    "min-is-black" tag. It is either #0# (for normal image) or #1#
    (for an inverted image).  Bytes #4# and #5# represent the image
    width (MSB first).  Bytes and #6# and #7# represent the image
    height (MSB first).
   
    @memo
    CCITT-G4/MMR decoder.
    @version
    #$Id: MMRDecoder.h,v 1.4 2000-01-31 21:14:45 leonb Exp $#
    @author
    Parag Deshmukh <parag@sanskrit.lz.att.com> */
//@{




/** Class for G4/MMR decoding.  The simplest way to use this class is
    the static member function \Ref{MMRDecoder::decode}.  This
    function internally creates an instance of #MMRDecoder# which
    processes the MMR data scanline by scanline.  */
class MMRDecoder
{
 public:
  /** Main decoding routine that (a) decodes the header using
      #decode_header#, (b) decodes the MMR data using an instance of
      #MMRDecoder#, and returns a new \Ref{JB2Image} composed of tiles
      whose maximal width and height is derived from the size of the
      image. */
  static GP<JB2Image> decode(ByteStream &inp);
  /** Only decode the header. */
  static void decode_header(ByteStream &inp, int &width, int &height, 
                            int &invert, int &striped);
public:
  ~MMRDecoder();
  /** Construct a MMRDecoder object for decoding an image
      of size #width# by #height#. Flag $striped# must be set
      if the image is composed of multiple stripes. */
  MMRDecoder(ByteStream &bs, int width, int height, int striped=0);
  /** Decodes a scanline and returns a pointer to the scanline data.
      Returns a pointer to the scanline buffer. The scanline data
      should be copied before calling this function again. */
  const unsigned char *scanline();
 private:
  int width;
  int height;
  int lineno;
  int striplineno;
  int rowsperstrip;
  unsigned char *refline;
  class VLSource;
  class VLTable;
  VLSource *src;
  VLTable *mrtable;
  VLTable *wtable;
  VLTable *btable;
  friend class VLSource;
  friend class VLTable;
};


//@}


// -----------
#endif
