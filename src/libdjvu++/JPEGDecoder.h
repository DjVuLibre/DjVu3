//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- 
//C- This software (the "Original Code") is subject to, and may be
//C- distributed under, the GNU General Public License, Version 2.
//C- The license should have accompanied the Original Code or you
//C- may obtain a copy of the license from the Free Software
//C- Foundation at http://www.fsf.org .
//C- 
//C- With respect to the Original Code, and subject to any third
//C- party intellectual property claims, LizardTech grants recipient
//C- a worldwide, royalty-free, non-exclusive license under patent
//C- claims infringed by making, using, or selling Original Code
//C- which are now or hereafter owned or controlled by LizardTech,
//C- but solely to the extent that any such patent is reasonably
//C- necessary to enable you to make, have made, practice, sell, or 
//C- otherwise dispose of Original Code (or portions thereof) and
//C- not to any greater extent that may be necessary to utilize
//C- further modifications or combinations.
//C- 
//C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
//C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
//C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: JPEGDecoder.h,v 1.8 2000-11-02 01:08:35 bcr Exp $
// $Name:  $


#ifndef _JPEGDECODER_H_
#define _JPEGDECODER_H_

#include "DjVuGlobal.h"
#ifdef NEED_JPEG_DECODER

#include "ByteStream.h"
#include "GPixmap.h"
#include "GException.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

#ifdef __cplusplus
}
#endif

#include <string.h>
#include <setjmp.h>

/** @name JPEGDecoder.h
    Files #"JPEGDecoder.h"# and #"JPEGDecoder.cpp"# implement an
    interface to the decoding subset of the IJG JPEG library.
    @memo
    Decoding interface to the IJG JPEG library.
    @version
    #$Id: JPEGDecoder.h,v 1.8 2000-11-02 01:08:35 bcr Exp $#
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
