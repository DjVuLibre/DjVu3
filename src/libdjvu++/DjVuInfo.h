//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C- 
// 
// $Id: DjVuInfo.h,v 1.10 2000-11-09 20:15:06 jmw Exp $
// $Name:  $

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
    #$Id: DjVuInfo.h,v 1.10 2000-11-09 20:15:06 jmw Exp $# */
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
  /** The following boolian values are stored in the last character of the
      info structure.  Unused bits are reserved for possible future extensions
      and backwards compatability. */
  bool compressable;
  enum {COMPRESSABLE_FLAG=0x80,RESERVED_FLAGS1=0x7f};
};


//@}

// ----- THE END
#endif
