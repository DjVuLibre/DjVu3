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
//C- $Id: IWTransform.h,v 1.4 1999-06-08 15:50:34 leonb Exp $

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
    #$Id: IWTransform.h,v 1.4 1999-06-08 15:50:34 leonb Exp $# 
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
      
      
             

    
