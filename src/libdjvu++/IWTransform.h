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
//C- $Id: IWTransform.h,v 1.1 1999-05-24 19:31:59 leonb Exp $

#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"


/** @name IWTransform.h
    This code implements fast IW44 transforms
    used by the internal routines of #"IWImage.cpp"#.
    @memo 
    Fast IW44 transforms.
    @version 
    #$Id: IWTransform.h,v 1.1 1999-05-24 19:31:59 leonb Exp $# 
    @author: 
    L\'eon Bottou <leonb@research.att.com> -- initial implementation */
//@{


/** IW44Transform.
*/

class IWTransform
{
 public:
  // MMX DETECTION
  /** Detects and enable MMX or similar technologies.  This function chects
      whether a specialized implementations of the IW44 transform is available
      (such as the MMX implementation) and enables it.  Returns a boolean
      indicating whether such an implementation is available.  Speedups
      factors may vary. */
  static int enable_mmx();
  /** Disable MMX or similar technologies.  The IW44 transform will then be
      performed using the baseline code. */
  static int disable_mmx();
  /** Forward transform. */
  static void forward(short *p, int w, int h, int rowsize, int begin, int end);
  /** Forward transform. */
  static void backward(short *p, int w, int h, int rowsize, int begin, int end);
};

// ------------ THE END
#endif
      
      
             

    
