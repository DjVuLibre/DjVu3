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
//C- $Id: DjVuPalette.h,v 1.1 1999-11-10 21:25:58 leonb Exp $



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
    Files #"DjVuPalette.h"# and #"DjVuPalette.cpp"# ...
    @memo 
    DjVuPalette header file
    @version 
    #$Id: DjVuPalette.h,v 1.1 1999-11-10 21:25:58 leonb Exp $#
    @author: 
    L\'eon Bottou <leonb@research.att.com>
*/
//@{




class DjVuPalette : public GPEnabled
{
public:
  DjVuPalette();
  ~DjVuPalette();
  // QUANTIZATION
  void histogram_clear();
  void histogram_add(const GPixel &p, int weight);
  void histogram_add(const unsigned char *bgr, int weight);
  int compute_palette(int ncolors);
  // CONVERSION
  int size() const;
  int color_to_index(const GPixel &p);
  int color_to_index(const unsigned char *bgr);
  void index_to_color(int index, GPixel &p) const;
  void index_to_color(int index, unsigned char *bgr) const;
  // ENCODING
  void encode(ByteStream &bs) const;
  void decode(ByteStream &bs);
  // MASS CONVERSION
  void quantize(GPixmap &pm);
  int compute_palette_and_quantize(GPixmap &pm, int ncolors);
public:
  // COLOR ARRAY
  GTArray<short> colordata;
private:
  // Histogram
  struct PHist { int p[3]; int w; };
  PHist *hcube;
  void allocate_hcube();
  static int hind[3][256];
  static bool initialized;
  // Quantization data
  struct PColor { unsigned char p[4]; };
  GTArray<PColor> palette;
  void allocate_pcube();
  short *pcube;
  int color_to_index_slow(const unsigned char *bgr);
  static int bcomp (const void*, const void*);
  static int gcomp (const void*, const void*);
  static int rcomp (const void*, const void*);
  static int lcomp (const void*, const void*);
};


//@}

// ------------ INLINES


void 
DjVuPalette::histogram_clear()
{
  allocate_hcube();
}

void 
DjVuPalette::histogram_add(const GPixel &p, int weight)
{
  if (!hcube) allocate_hcube();
  PHist &d = hcube[hind[0][p.b]+hind[1][p.g]+hind[2][p.r]];
  d.p[0] += p.b;
  d.p[1] += p.g;
  d.p[2] += p.r;
  d.w += weight;
}


void 
DjVuPalette::histogram_add(const unsigned char *bgr, int weight)
{
  if (!hcube) allocate_hcube();
  PHist &d = hcube[hind[0][bgr[0]]+hind[1][bgr[1]]+hind[2][bgr[2]]];
  d.p[0] += bgr[0];
  d.p[1] += bgr[1];
  d.p[2] += bgr[2];
  d.w += weight;
}

int
DjVuPalette::size() const
{
  return palette.size();
}

int 
DjVuPalette::color_to_index(const unsigned char *bgr)
{
  if (!pcube) allocate_pcube();
  short &d = pcube[hind[0][bgr[0]]+hind[1][bgr[1]]+hind[2][bgr[2]]];
  if (d < 0) d = color_to_index_slow(bgr);
  return d;
}

int 
DjVuPalette::color_to_index(const GPixel &p)
{
  return color_to_index(&p.b);
}

void 
DjVuPalette::index_to_color(int index, unsigned char *bgr) const
{
  const PColor &color = palette[index];
  bgr[0] = color.p[0];
  bgr[1] = color.p[1];
  bgr[2] = color.p[2];
}

void 
DjVuPalette::index_to_color(int index, GPixel &p) const
{
  index_to_color(index, &p.b);
}





// ------------ THE END
#endif
      
      
             

    
