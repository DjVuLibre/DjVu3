//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GScaler.h,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $

#ifndef _GSCALER_H_
#define _GSCALER_H_


#include "GException.h"
#include "GRect.h"
#include "GBitmap.h"
#include "GPixmap.h"

/** @name GScaler.h 

    Files #"GScaler.h"# and #"GScaler.cpp"# implement a fast bilinear
    interpolation scheme to rescale a \Ref{GBitmap} or a \Ref{GPixmap}.
    Common setup functions are implemented by the base class \Ref{GScaler}.
    The actual function for rescaling a gray level image is implemented by
    class \Ref{GBitmapScaler}.  The actual function for rescaling a color
    image is implemented by class \Ref{GPixmapScaler}.

    {\bf Remark} --- The bilinear interpolation code relies on fixed precision
    tables.  It becomes suboptimal when supersampling (i.e. zooming into) an
    image by a factor greater than eight.  Very contracted images displayed at
    high magnification may contain visible jaggies.

    @memo
    Rescaling images with bilinear interpolation.
    @author
    Leon Bottou <leonb@research.att.com>
    @version
    #$Id: GScaler.h,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $# */
//@{


/** Base class for GBitmapScaler and GPixmapScaler.  This base class
    implements the common elements of class \Ref{GBitmapScaler} and
    \Ref{GPixmapScaler}.  Functions \Ref{set_input_size} and
    \Ref{set_output_size} are used to specify the size of the input image and
    the size of the output image.  Functions \Ref{set_horz_ratio} and
    \Ref{set_vert_ratio} may be used to override the scaling ratios computed
    from the image sizes.  You can then call function \Ref{get_input_rect} to
    know which pixels of the input image are necessary to compute a specified
    rectangular zone of the output image.  The actual computation is then
    performed by calling function #scale# in class \Ref{GBitmapScaler} and
    \Ref{GPixmapScaler}.  
*/
class GScaler 
{
protected:  
  GScaler();
public:
  virtual ~GScaler();
  /** Sets the size of the input image. Argument #w# (resp. #h#) contains the
      horizontal (resp. vertical) size of the input image.  This size is used
      to initialize the internal data structures of the scaler object. */
  void set_input_size(int w, int h);
  /** Sets the size of the output image. Argument #w# (resp. #h#) contains the
      horizontal (resp. vertical) size of the output image. This size is used
      to initialize the internal data structures of the scaler object. */
  void set_output_size(int w, int h);
  /** Sets the horizontal scaling ratio #numer/denom#.  This function may be
      used to force an exact scaling ratio.  The scaling ratios are otherwise
      derived from the sizes of the input and output images. */
  void set_horz_ratio(int numer, int denom);
  /** Sets the vertical scaling ratio to #numer/denom#.  This function may be
      used to force an exact scaling ratio.  The scaling ratios are otherwise
      derived from the sizes of the input and output images. */
  void set_vert_ratio(int numer, int denom);
  /** Computes which input pixels are required to compute specified output
      pixels.  Let us assume that we only need a part of the output
      image. This part is defined by rectangle #desired_output#.  Only a part
      of the input image is necessary to compute the output pixels.  Function
      #get_input_rect# computes the coordinates of that part of the input
      image, and stores them into rectangle #required_input#.  */
  void get_input_rect( const GRect &desired_output, GRect &required_input );
protected:
  // The sizes
  int inw, inh;
  int xshift, yshift;
  int redw, redh;
  int outw, outh;
  // Fixed point coordinates
  int *vcoord;
  int *hcoord;
  // Helper
  void make_rectangles(const GRect &desired, GRect &red, GRect &inp);
};



/** Fast rescaling code for gray level images.  This class augments the base
    class \Ref{GScaler} with a function for rescaling gray level
    images.  Function \Ref{GBitmapScaler::scale} computes an arbitrary segment
    of the output image given the corresponding pixels in the input image.

    {\bf Example} --- The following functions returns an gray level image
    (sixteen gray levels, size #nw# by #nh#) containing a rescaled version of
    the input image #in#.
    \begin{verbatim}
    GBitmap *rescale_bitmap(const GBitmap &in, int nw, int nh)
    {
      int w = in.columns();       // Get input width
      int h = in.raws();          // Get output width
      GBitmapScaler scaler(w,h,nw,nh);  // Creates bitmap scaler
      GRect desired(0,0,nw,nh);   // Desired output = complete bitmap
      GRect provided(0,0,w,h);    // Provided input = complete bitmap
      GBitmap *out = new GBitmap;
      scaler.scale(provided, in, desired, *out);  // Rescale
      out->change_grays(16);      // Reduce to 16 gray levels
      return out;
    }
    \end{verbatim} */
class GBitmapScaler : public GScaler
{
public:
  virtual ~GBitmapScaler();
  /** Constructs an empty GBitmapScaler. You must call functions
      \Ref{GScaler::set_input_size} and \Ref{GScaler::set_output_size} before
      calling any of the scaling functions. */
  GBitmapScaler();
  /** Constructs a GBitmapScaler. The size of the input image is given by
      #inw# and #inh#.  This function internally calls
      \Ref{GScaler::set_input_size} and \Ref{GScaler::set_output_size}. The
      size of the output image is given by #outw# and #outh#.  . */
  GBitmapScaler(int inw, int inh, int outw, int outh);
  /** Computes a segment of the rescaled output image.  The bitmap #output# is
      overwritten with the segment of the output image specified by the
      rectangle #desired_output#.  The rectangle #provided_input# specifies
      which segment of the input image is provided in the bitmap #input#.  An
      exception \Ref{GException} is thrown if the rectangle #provided_input#
      is smaller then the rectangle #required_input# rectangle returned by
      function \Ref{GScaler::get_input_rect}.  Note that the output bitmap
      always contain 256 gray levels. You may want to use function
      \Ref{GBitmap::change_grays} to reduce the numebr of gray levels. */
  void scale( const GRect &provided_input, const GBitmap &input,
              const GRect &desired_output, GBitmap &output );
protected:
  // Helpers
  unsigned char *get_line(int, const GRect &, const GRect &, const GBitmap &);
  // Temporaries
  unsigned char *lbuffer;
  unsigned char *conv;
  unsigned char *p1;
  unsigned char *p2;
  int l1;
  int l2;
};


/** Fast rescaling code for color images.  This class augments the base class
    \Ref{GScaler} with a function for rescaling gray level images.  Function
    \Ref{GPixmapScaler::scale} computes an arbitrary segment of the output
    image given the corresponding pixels in the input image.

    {\bf Example} --- The following functions returns a color image
    of size #nw# by #nh# containing a rescaled version of
    the input image #in#.
    \begin{verbatim}
    GPixmap *rescale_pixmap(const GPixmap &in, int nw, int nh)
    {
      int w = in.columns();       // Get input width
      int h = in.raws();          // Get output width
      GPixmapScaler scaler(w,h,nw,nh);  // Creates bitmap scaler
      GRect desired(0,0,nw,nh);   // Desired output = complete image
      GRect provided(0,0,w,h);    // Provided input = complete image
      GPixmap *out = new GPixmap;
      scaler.scale(provided, in, desired, *out);  // Rescale
      return out;
    }
    \end{verbatim}

 */
class GPixmapScaler : public GScaler
{
public:
  virtual ~GPixmapScaler();
  /** Constructs an empty GPixmapScaler. You must call functions
      \Ref{GScaler::set_input_size} and \Ref{GScaler::set_output_size} before
      calling any of the scaling functions. */
  GPixmapScaler();
  /** Constructs a GPixmapScaler. The size of the input image is given by
      #inw# and #inh#.  This function internally calls
      \Ref{GScaler::set_input_size} and \Ref{GScaler::set_output_size}. The
      size of the output image is given by #outw# and #outh#.  . */
  GPixmapScaler(int inw, int inh, int outw, int outh);
  /** Computes a segment of the rescaled output image.  The pixmap #output# is
      overwritten with the segment of the output image specified by the
      rectangle #desired_output#.  The rectangle #provided_input# specifies
      which segment of the input image is provided in the pixmap #input#.  An
      exception \Ref{GException} is thrown if the rectangle #provided_input#
      is smaller then the rectangle #required_input# rectangle returned by
      function \Ref{GScaler::get_input_rect}. */
  void scale( const GRect &provided_input, const GPixmap &input,
              const GRect &desired_output, GPixmap &output );
protected:
  // Helpers
  GPixel *get_line(int, const GRect &, const GRect &, const GPixmap &);
  // Temporaries
  GPixel *lbuffer;
  GPixel *p1;
  GPixel *p2;
  int    l1;
  int    l2;
};





//@}
    



// -------- END
#endif
