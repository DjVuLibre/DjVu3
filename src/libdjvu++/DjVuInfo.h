//C-  -*- C++ -*-
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
//C- $Id: DjVuInfo.h,v 1.5 1999-11-06 00:52:36 praveen Exp $

#ifndef _DJVUINFO_H
#define _DJVUINFO_H


/** @name DjVuInfo.h
    Each instance of class #DjVuInfo# represents the information
    contained in the information chunk of a DjVu file.  This #"INFO"#
    chunk is always the first chunk of a DjVu file.
    @memo
    DjVu information chunk.
    @author
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id: DjVuInfo.h,v 1.5 1999-11-06 00:52:36 praveen Exp $# */
//@{


#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"
#include "GThreads.h"
#include "GSmartPointer.h"
#include "ByteStream.h"

/** @name DjVu version constants
    @memo DjVu file format version. */
//@{
/** Current DjVu format version.  The value of this macro represents the
    version of the DjVu file format implemented by this release of the DjVu
    Reference Library. */
#define DJVUVERSION          21
/** Oldest DjVu format version supported by this library.  This release of the
    library cannot completely decode DjVu files whose version field is less
    than or equal to this number. */
#define DJVUVERSION_TOO_OLD  15
/** Newest DjVu format partially supported by this library.  This release of
    the library will attempt to decode files whose version field is smaller
    than this macro.  If the version field is greater than or equal to this
    number, the decoder will just throw a \Ref{GException}.  */
#define DJVUVERSION_TOO_NEW  25
//@}


/** Information component.
    Each instance of class #DjVuInfo# represents the information
    contained in the information chunk of a DjVu file.  This #"INFO"#
    chunk is always the first chunk of a DjVu file.
 */

class DjVuInfo : public GPEnabled
{
public:
  /** Constructs an empty DjVuInfo object.
      The #width# and #height# fields are set to zero.
      All other fields are initialized with suitable default values. */
  DjVuInfo();
  /** Decodes the DjVu #"INFO"# chunk.  This function reads binary data from
      ByteStream #bs# and populates the fields of this DjVuInfo object.  It is
      normally called after detecting an #"INFO"# chunk header with function
      \Ref{IFFByteStream::get_chunk}. */
  void decode(ByteStream &bs);
  /** Encodes the DjVu #"INFO"# chunk. This function writes the fields of this
      DjVuInfo object into ByteStream #bs#. It is normally called after
      creating an #"INFO"# chunk header with function
      \Ref{IFFByteStream::put_chunk}. */
  void encode(ByteStream &bs);  
  /** Returns the number of bytes used by this object. */
  unsigned int get_memory_usage() const;
  /** Width of the DjVu image (in pixels). */
  int width;
  /** Height of the DjVu image (in pixels). */
  int height;
  /** DjVu file version number.  This number characterizes the file format
      version used by the encoder to generate this DjVu image.  A decoder
      should compare this version number with the constants described in
      section "\Ref{DjVu version constants}". */
  int version;
  /** Resolution of the DjVu image.  The resolution is given in ``pixels per
      2.54 centimeters'' (this unit is sometimes called ``pixels per
      inch''). Display programs can use this information to determine the
      natural magnification to use for rendering a DjVu image. */
  int dpi;
  /** Gamma coefficient of the display for which the image was designed.  The
      rendering functions can use this information in order to perform color
      correction for the intended display device. */
  double gamma;
  /** Reserved byte. The IFF padding rules give the opportunity to store an
      extra byte in the #"INFO"# chunk.  This is reserved for possible
      extensions and for backward compatibility. */
  unsigned char reserved;
};


//@}

// ----- THE END
#endif
