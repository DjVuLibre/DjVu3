//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// 
// $Id: IWTransform.h,v 1.10 2001-01-04 22:04:55 bcr Exp $
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
    #$Id: IWTransform.h,v 1.10 2001-01-04 22:04:55 bcr Exp $# 
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
      
      
             

    
