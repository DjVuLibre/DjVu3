//C-  -*- C++ -*-
//C-
//C- This software contains software code or other information proprietary to
//C- AT&T Corp. ("AT&T").  Unless you accept a license to use this software,
//C- you shall not use, execute, compile, modify, redistribute, reverse
//C- compile, disassemble, or otherwise reverse engineer the AT&T software or
//C- any derived work of the AT&T software.  The text of a license can be
//C- found in file "ATTLICENSE" or at the Internet website having the URL
//C- "http://www.djvu.att.com/open".
//C-
//C- This software is provided to you "AS IS".  YOU "AS IS".  YOU ASSUME TOTAL
//C- RESPONSIBILITY AND RISK FOR YOUR USE OF THEM INCLUDING THE RISK OF ANY
//C- DEFECTS OR INACCURACIES THEREIN.  AT&T DOES NOT MAKE, AND EXPRESSLY
//C- DISCLAIMS, ANY EXPRESS OR IMPLIED WARRANTIES OF ANY KIND WHATSOEVER,
//C- INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//C- OR FITNESS FOR A PARTICULAR PURPOSE, WARRANTIES OF TITLE OR
//C- NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS OR TRADEMARK RIGHTS,
//C- ANY WARRANTIES ARISING BY USAGE OF TRADE, COURSE OF DEALING OR COURSE OF
//C- PERFORMANCE, OR ANY WARRANTY THAT THE AT&T SOURCE CODE RELEASE OR AT&T
//C- CAPSULE ARE "ERROR FREE" WILL MEET YOUR REQUIREMENTS.
//C-
//C-     (C) AT&T Corp. All rights reserved.
//C-     AT&T is a registered trademark of AT&T Corp.
//C-
//C- $Id: BSByteStream.h,v 1.9 1999-03-16 20:21:27 leonb Exp $


#ifndef _BSBYTESTREAM_H
#define _BSBYTESTREAM_H

/** @name BSByteStream.h
    
    Files #"BSByteStream.h"# and #"BSByteStream.cpp"# implement a very compact
    general purpose compressor based on the Burrows-Wheeler transform.  The
    utility program \Ref{bzz} provides a front-end for this class. Although
    this compression model is not currently used in DjVu files, it may be used
    in the future for encoding textual data chunks.

    {\bf Algorithms} --- The Burrows-Wheeler transform (also named Block-Sorting)
    is performed using a combination of the Karp-Miller-Rosenberg and the
    Bentley-Sedgewick algorithms. This is comparable to (Sadakane, DCC 98)
    with a slightly more flexible ranking scheme. Symbols are then ordered
    according to a running estimate of their occurrence frequencies.  The
    symbol ranks are then coded using a simple fixed tree and the
    \Ref{ZPCodec} binary adaptive coder.

    {\bf Performances} --- The basic algorithm is mostly similar to those
    implemented in well known compressors like #bzip# or #bzip2#
    (\URL{http://www.muraroa.demon.co.uk}).  The adaptive binary coder however
    generates small differences. The adaptation noise may cost up to 5\% in
    file size, but this penalty is usually offset by the benefits of
    adaptation.  This is good when processing large and highly structured
    files like spreadsheet files.  Compression and decompression speed is
    about twice slower than #bzip2# but the sorting algorithms is more
    robust. Unlike #bzip2# (as of August 1998), this code can compress half a
    megabyte of "abababab...." in bounded time.
    
    Here are some comparative results (in bits per character) obtained on the
    Canterbury Corpus (\URL{http://corpus.canterbury.ac.nz}) as of August
    1998. The BSByteStream performance on the single spreadsheet file #Excl#
    moves #bzz#'s weighted average ahead of much more sophisticated methods,
    like Suzanne Bunton's #fsmxBest# system
    \URL{http://corpus.canterbury.ac.nz/methodinfo/fsmx.html}.  This result
    will not last very long.

    {\footnotesize
    \begin{tabular}{lccccccccccccc}
      & text & fax & Csrc & Excl & SPRC & tech 
      & poem & html & lisp & man & play & Weighted & Average \\
      compress 
      & 3.27 & 0.97 & 3.56 & 2.41 & 4.21 & 3.06 
      & 3.38 & 3.68 & 3.90 & 4.43 & 3.51 
      & 2.55 & 3.31 \\
      gzip -9
      & 2.85 & 0.82 & 2.24 & 1.63 & 2.67 & 2.71 
      & 3.23 & 2.59 & 2.65 & 3.31 & 3.12 
      & 2.08 & 2.53 \\  
      bzip2 -9
      & 2.27 & 0.78 & 2.18 & 1.01 & 2.70 & 2.02 
      & 2.42 & 2.48 & 2.79 & 3.33 & 2.53 
      & 1.54 & 2.23 \\
      ppmd
      & 2.31 & 0.99 & 2.11 & 1.08 & 2.68 & 2.19 
      & 2.48 & 2.38 & 2.43 & 3.00 & 2.53 
      & 1.65 & 2.20 \\
      fsmx
      & {\bf 2.10} & 0.79 & {\bf 1.89} & 1.48 & {\bf 2.52} & {\bf 1.84} 
      & {\bf 2.21} & {\bf 2.24} & {\bf 2.29} & {\bf 2.91} & {\bf 2.35} 
      & 1.63 & {\bf 2.06} \\
      {\bf bzz}
      & 2.25 & {\bf 0.76} & 2.13 & {\bf 0.78} & 2.67 & 2.00
      & 2.40 & 2.52 & 2.60 & 3.19 & 2.52 
      & {\bf 1.44} & 2.16
    \end{tabular}
    }

    Note that the DjVu people have several entries in this table.  Program
    #compress# was written some time ago by Joe Orost
    (\URL{http://www.research.att.com/info/orost}). The #ppmc# method, (a
    precursor of #ppmd#) was created by Paul Howard
    (\URL{http://www.research.att.com/info/pgh}). The #bzz# program is just
    below your eyes.

    @author
    L\'eon Bottou <leonb@research.att.com> -- Initial implementation\\
    Andrei Erofeev <eaf@research.att.com> -- Improved Block Sorting algorithm.
    @memo
    Simple Burrows-Wheeler general purpose compressor.
    @version
    #$Id: BSByteStream.h,v 1.9 1999-03-16 20:21:27 leonb Exp $# */
//@{

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GException.h"
#include "ByteStream.h"
#include "ZPCodec.h"


/** Performs bzz compression/decompression.
    
    Class #BSByteStream# defines a \Ref{ByteStream} which transparently
    performs the BZZ compression/decompression. The constructor of class
    \Ref{BSByteStream} takes another \Ref{ByteStream} as argument.  Any data
    written to the BSByteStream is compressed and written to this second
    ByteStream. Any data read from the BSByteStream is internally generated by
    decompressing data read from the second ByteStream.

    Program \Ref{bzz} demonstrates how to use this class.  All the hard work
    is achieved by a simple ByteStream to ByteStream copy, as shown below.
    \begin{verbatim}
      StdioByteStream in(infile,"rb");
      StdioByteStream out(outfile,"wb");
      if (encoding) {
          BSByteStream bsb(&out, blocksize);
          bsb.copy(in);
      } else {
          BSByteStream bsb(&in);
          out.copy(bsb);
      }
    \end{verbatim}
    Due to the block oriented nature of the Burrows-Wheeler transform, there
    is a very significant latency between the data input and the data output.
    You can use function #flush# to force data output at the expense of
    compression efficiency.  Destroying the BSByteStream performs an implicit
    #flush#.  
*/
class BSByteStream : public ByteStream
{
public:
  /** Constructs a BSByteStream.
      Argument #blocksize# determines whether the BSByteStream will be used
      for compressing or decompressing data. 
      \begin{description}
      \item[Decompression]
      Setting #blocksize# to zero initializes the decompressor.  Chunks of
      data will be read from ByteStream #bs# and decompressed into an internal
      buffer. Function #read# can be used to access the decompressed data.
      \item[Compression]
      Setting #blocksize# to a positive number between 100 and 4096
      initializes the compressor.  Data written to the BSByteStream will be
      accumulated into an internal buffer.  The buffered data will be
      compressed and written to ByteStream #bs# whenever the buffer sizes
      reaches the maximum value specified by argument #blocksize# (in
      kilobytes).  Using a larger block size usually increases the compression
      ratio at the expense of computation time.  There is no need however to
      specify a block size larger than the total number of bytes to compress.
      Setting #blocksize# to #1024# is a good starting point.
      \end{description} */
  BSByteStream(ByteStream &bs, int blocksize=0);
  // ByteStream Interface
  ~BSByteStream();
  size_t read(void *buffer, size_t size);
  size_t write(const void *buffer, size_t size);
  long tell();
  void flush();
private:
  // Data
  int             encoding;
  long            offset;
  int             bptr;
  unsigned int    blocksize;
  unsigned char  *data;
  int             size;
  int             eof;
  ByteStream	 *bs;
  // Coder
  ZPCodec           zp;
  BitContext        ctx[300];
private:  
  // Cancel C++ default stuff
  BSByteStream(const BSByteStream &);
  BSByteStream & operator=(const BSByteStream &);
  // Helpers
  unsigned int encode(void);
  unsigned int decode(void);
};


//@}

#endif
