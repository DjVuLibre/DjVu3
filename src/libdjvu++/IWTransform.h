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
// $Id: IWTransform.h,v 1.8 2000-11-03 02:08:37 bcr Exp $
// $Name:  $

#ifndef _IWTRANSFORM_H_
#define _IWTRANSFORM_H_

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GPixmap.h"


/** @name IWTransform.h
    This code implements fast IW44 transforms
    used by the internal routines of #"IWImage.cpp"#.
    @memo 
    Fast IW44 transforms.
    @version 
    #$Id: IWTransform.h,v 1.8 2000-11-03 02:08:37 bcr Exp $# 
    @author: 
    L\'eon Bottou <leonb@research.att.com> -- initial implementation */
//@{


/** IW44Transform.
*/

class IWTransform
{
 public:
 // WAVELET TRANSFORM
  /** Forward transform. */
  static void forward(short *p, int w, int h, int rowsize, int begin, int end);
  /** Forward transform. */
  static void backward(short *p, int w, int h, int rowsize, int begin, int end);
  
  // COLOR TRANSFORM
  /** Converts YCbCr to RGB. */
  static void YCbCr_to_RGB(GPixel *p, int w, int h, int rowsize);
  /** Extracts Y */
  static void RGB_to_Y(const GPixel *p, int w, int h, int rowsize, 
                       signed char *out, int outrowsize);
  /** Extracts Cb */
  static void RGB_to_Cb(const GPixel *p, int w, int h, int rowsize, 
                        signed char *out, int outrowsize);
  /** Extracts Cr */
  static void RGB_to_Cr(const GPixel *p, int w, int h, int rowsize, 
                        signed char *out, int outrowsize);
};

//@}

// ------------ THE END
#endif
      
      
             

    
