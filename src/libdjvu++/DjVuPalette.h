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
//C- $Id: DjVuPalette.h,v 1.13 2000-02-24 16:38:48 leonb Exp $



#ifndef _DJVUPALETTE_H_
#define _DJVUPALETTE_H_

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include <string.h>
#include <new.h>
#include "GException.h"
#include "GSmartPointer.h"
#include "GContainer.h"
#include "GPixmap.h"
#include "ByteStream.h"


/** @name DjVuPalette.h
    Files #"DjVuPalette.h"# and #"DjVuPalette.cpp"# implement a single class
    \Ref{DjVuPalette} which provides facilities for computing optimal color
    palettes, coding color palettes, and coding sequences of color indices.
    @memo 
    DjVuPalette header file
    @version 
    #$Id: DjVuPalette.h,v 1.13 2000-02-24 16:38:48 leonb Exp $#
    @author: 
    L\'eon Bottou <leonb@research.att.com> */
//@{


/** Computing and coding color palettes and index arrays.
    This class provides facilities for computing optimal color palettes,
    coding color palettes, and coding sequences of color indices.
    
    {\bf Creating a color palette} -- The recipe for creating a color palette
    consists in (a) creating a DjVuPalette object, (b) constructing a color
    histogram using \Ref{histogram_add}, and (c) calling function
    \Ref{compute_palette}.

    {\bf Accessing the color palette} -- Conversion between colors and color
    palette indices is easily achieved with \Ref{color_to_index} and
    \Ref{index_to_color}.  There are also functions for computing a palette
    and quantizing a complete pixmap.

    {\bf Sequences of color indices} -- The DjVuPalette object also contains
    an array \Ref{colordata} optionally containing a sequence of color
    indices.  This array will be encoded and decoded by functions \Ref{encode}
    and \Ref{decode}.  This feature simplifies the implementation of the ``one
    color per symbol'' model in DjVu.

    {\bf Coding color palettes and color indices} -- Two functions
    \Ref{encode} and \Ref{decode} are provided for coding the color palette
    and the array of color indices when appropriate.  */

class DjVuPalette : public GPEnabled
{
public:
  DjVuPalette();
  ~DjVuPalette();
  // COPY
  DjVuPalette(const DjVuPalette &ref);
  DjVuPalette& operator=(const DjVuPalette &ref);
  // PALETTE COMPUTATION
  /** Resets the color histogram to zero. */
  void histogram_clear();
  /** Adds the color specified by #p# to the histogram.
      Argument #weight# represent the number of pixels with this color. */
  void histogram_add(const GPixel &p, int weight);
  /** Adds the color specified by the triple #bgr# to the histogram.
      Argument #weight# represent the number of pixels with this color. */
  void histogram_add(const unsigned char *bgr, int weight);
  /** Adds the color specified by the weighted triple #bgr# to the histogram.
      Argument #weight# represent the number of pixels with this color.  This
      function will compute the actual color by dividing the elements of the
      #bgr# array by #weight# and then use the unnormalized values to compute
      the average color per bucket.  This is all a way to avoid excessive loss
      of accuracy. */
  void histogram_norm_and_add(const int *bgr, int weight);
  /** Computes an optimal palette for representing an image where colors
      appear according to the histogram.  Argument #maxcolors# is the maximum
      number of colors allowed in the palette (up to 1024).  Argument
      #minboxsize# controls the minimal size of the color cube area affected
      to a color palette entry.  Returns the index of the dominant color. */
  int compute_palette(int maxcolors, int minboxsize=0);
  /** Computes the optimal palette for pixmap #pm#.  This function builds the
      histogram for pixmap #pm# and computes the optimal palette using
      \Ref{compute_palette}. */
  int compute_pixmap_palette(const GPixmap &pm, int ncolors, int minboxsize=0);
  // CONVERSION
  /** Returns the number of colors in the palette. */
  int size() const;
  /** Returns the best palette index for representing color #p#. */
  int color_to_index(const GPixel &p);
  /** Returns the best palette index for representing color #bgr#. */
  int color_to_index(const unsigned char *bgr);
  /** Overwrites #p# with the color located at position #index# in the palette. */
  void index_to_color(int index, GPixel &p) const;
  /** Overwrites #rgb[0..3]# with the color located at 
      position #index# in the palette. */
  void index_to_color(int index, unsigned char *bgr) const;
  /** Quantizes pixmap #pm#. All pixels are replaced by their closest
      approximation available in the palette. */
  void quantize(GPixmap &pm);
  /** Calls \Ref{compute_pixmap_palette} and \Ref{quantize}. */
  int compute_palette_and_quantize(GPixmap &pm, int maxcolors, int minboxsize=0);
  // COLOR CORRECTION
  /** Applies a luminance gamma correction factor of #corr# to the palette
      entries.  Values greater than #1.0# make the image brighter.  Values
      smaller than #1.0# make the image darker.  The documentation of program
      \Ref{ppmcoco} explains how to properly use this function. */
  void color_correct(double corr);
  // COLOR INDEX DATA
  /** Contains an optional sequence of color indices. 
      Function \Ref{encode} and \Ref{decode} also encode and decode this
      sequence when such a sequence is provided. */
  GTArray<short> colordata;
  /** Returns colors from the color index sequence.  Pixel #out# is
      overwritten with the color corresponding to the #nth# element of the
      color sequence \Ref{colordata}. */
  void get_color(int nth, GPixel &out) const;
  // CODING
  /** Encodes the palette and the color index sequence into bytestream #bs#. 
      Note that the color histogram is never saved. */
  void encode(ByteStream &bs) const;
  /** Initializes the object by reading data from bytestream #bs#.  This will
      populate the palette, and optionally populate the \Ref{colordata}
      array. Note that the color histogram is never saved. */
  void decode(ByteStream &bs);

private:
  int color_to_index_slow(const unsigned char *bgr);
  // Histogram
  struct PHist { float p[3]; int w; };
  PHist *hcube;
  void allocate_hcube();
  static int hind[3][256];
  static bool initialized;
  // Quantization data
  struct PColor { unsigned char p[4]; };
  GTArray<PColor> palette;
  void allocate_pcube();
  short *pcube;
  static int bcomp (const void*, const void*);
  static int gcomp (const void*, const void*);
  static int rcomp (const void*, const void*);
  static int lcomp (const void*, const void*);
};


//@}

// ------------ INLINES


inline void 
DjVuPalette::histogram_clear()
{
  allocate_hcube();
}

inline void 
DjVuPalette::histogram_add(const GPixel &p, int weight)
{
  if (!hcube) allocate_hcube();
  PHist &d = hcube[hind[0][p.b]+hind[1][p.g]+hind[2][p.r]];
  d.p[0] += weight*p.b;
  d.p[1] += weight*p.g;
  d.p[2] += weight*p.r;
  d.w += weight;
}

inline void 
DjVuPalette::histogram_add(const unsigned char *bgr, int weight)
{
  if (!hcube) allocate_hcube();
  if (weight>0) 
    {
      PHist &d = hcube[hind[0][bgr[0]]+hind[1][bgr[1]]+hind[2][bgr[2]]];
      d.p[0] +=  bgr[0] * weight;
      d.p[1] +=  bgr[1] * weight;
      d.p[2] +=  bgr[2] * weight;
      d.w += weight;
    }
}

inline void 
DjVuPalette::histogram_norm_and_add(const int *bgr, int weight)
{
  if (!hcube) allocate_hcube();
  if (weight>0) 
    {
      int p0 = bgr[0]/weight; if (p0>255) p0=255;
      int p1 = bgr[1]/weight; if (p1>255) p1=255;
      int p2 = bgr[2]/weight; if (p2>255) p2=255;
      PHist &d = hcube[hind[0][p0]+hind[1][p1]+hind[2][p2]];
      d.p[0] +=  bgr[0];
      d.p[1] +=  bgr[1];
      d.p[2] +=  bgr[2];
      d.w += weight;
    }
}

inline int
DjVuPalette::size() const
{
  return palette.size();
}

inline int 
DjVuPalette::color_to_index(const unsigned char *bgr)
{
  if (!pcube) allocate_pcube();
  short &d = pcube[hind[0][bgr[0]]+hind[1][bgr[1]]+hind[2][bgr[2]]];
  if (d < 0) d = color_to_index_slow(bgr);
  return d;
}

inline int 
DjVuPalette::color_to_index(const GPixel &p)
{
  return color_to_index(&p.b);
}

inline void 
DjVuPalette::index_to_color(int index, unsigned char *bgr) const
{
  const PColor &color = palette[index];
  bgr[0] = color.p[0];
  bgr[1] = color.p[1];
  bgr[2] = color.p[2];
}

inline void 
DjVuPalette::index_to_color(int index, GPixel &p) const
{
  index_to_color(index, &p.b);
}

inline void
DjVuPalette::get_color(int nth, GPixel &p) const
{
  index_to_color(colordata[nth], p);
}



// ------------ THE END
#endif
      
      
             

    
