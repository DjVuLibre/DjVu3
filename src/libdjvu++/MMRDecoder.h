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
//C- $Id: MMRDecoder.h,v 1.7 2000-09-18 17:10:24 bcr Exp $

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

    The #Smmr# chunk starts by a header containing the following data:
    \begin{verbatim}
        BYTE*3    :  'M' 'M' 'R'
        BYTE      :  0xb000000<s><i>
        INT16     :  <width> (MSB first)
        INT16     :  <height> (MSB first)
    \end{verbatim}

    The header is followed by the encoded data.  Bit 0 of the fourth header
    byte (#<i>#) is similar to TIFF's ``min-is-black'' tag.  This bit is set
    for a reverse video image.  The encoded data can be in either ``regular''
    MMR form or ``striped'' MMR form.  This is indicated by bit 1 of the
    fourth header byte (#<s>#).  This bit is set to indicate ``striped''
    data.  The ``regular'' data format consists of ordinary MMR encoded data.
    The ``striped'' data format isconsists of one sixteen bit integer (msb
    first) containing the number of rows per stripe, followed by data for each
    stripe as follows.
    \begin{verbatim}
        INT16     :  <rowsperstripe> (MSB first)
        INT32          :  <nbytes1>
        BYTE*<nbytes1> :  <mmrdata1>
        INT32          :  <nbytes2>
        BYTE*<nbytes2> :  <mmrdata2>
          ...
    \end{verbatim}
    Static function \Ref{MMRDecoder::decode_header} decodes the header.  You
    can then create a \Ref{MMRDecoder} object with the flags #inverted# and
    #striped# as obtained when decoding the header.  One can also decode raw
    MMR data by simply initialising a \Ref{MMRDecoder} object with flag
    #striped# unset.  Each call to \Ref{MMRDecoder::scanruns},
    \Ref{MMRDecoder::scanrle} or \Ref{MMRDecoder::scanline} wil then decode a
    row of the MMR encoded image.

    Function \Ref{MMRDecoder::decode} is a convenience function for decoding
    the contents of a #"Smmr"# chunk.  It returns a \Ref{JB2Image} divided
    into manageable blocks in order to provide the zooming and panning
    features implemented by class \Ref{JB2Image}.

    @memo
    CCITT-G4/MMR decoder.
    @version
    #$Id: MMRDecoder.h,v 1.7 2000-09-18 17:10:24 bcr Exp $#
    @author
    Parag Deshmukh <parag@sanskrit.lz.att.com> \\
    Leon Bottou <leonb@research.att.com> */
//@{



#define MMRDECODER_HAS_SCANRUNS  1
#define MMRDECODER_HAS_SCANRLE   1



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
  /** Decodes a scanline and returns a pointer to an array of run lengths.
      The returned buffer contains the length of alternative white and black
      runs.  These run lengths sum to the image width. They are followed by
      two zeroes.  The position of these two zeroes is stored in the pointer
      specified by the optional argument #endptr#.  The buffer data should be
      processed before calling this function again. */
  const unsigned short *scanruns(const unsigned short **endptr=0);
  /** Decodes a scanline and returns a pointer to RLE encoded data.  The
      buffer contains the length of the runs for the current line encoded as
      described in \Ref{PNM and RLE file formats}.)  The flag #invert# can be
      used to indicate that the MMR data is encoded in reverse video.  The RLE
      data is followed by two zero bytes.  The position of these two zeroes is
      stored in the pointer specified by the optional argument #endptr#.  The
      buffer data should be processed before calling this function again. This
      is implemented by calling \Ref{MMRDecoder::scanruns}. */
  const unsigned char  *scanrle(int invert, const unsigned char **endptr=0);
  /** Decodes a scanline and returns a pointer to an array of #0# or #1# bytes.
      Returns a pointer to the scanline buffer containing one byte per pixel. 
      The buffer data should be processed before calling this function again.
      This is implemented by calling \Ref{MMRDecoder::scanruns}. */
  const unsigned char *scanline();
 private:
  int width;
  int height;
  int lineno;
  int striplineno;
  int rowsperstrip;
  unsigned char  *line;
  unsigned short *lineruns;
  unsigned short *prevruns;
public:
  class VLSource;
  class VLTable;
private:
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
