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
//C- $Id: DjVuPalette.cpp,v 1.1 1999-11-10 21:25:58 leonb Exp $

// File "$Id: DjVuPalette.cpp,v 1.1 1999-11-10 21:25:58 leonb Exp $"
// -- Implements class PIXMAP
// Author: Leon Bottou 07/1997


#ifdef __GNUC__
#pragma implementation
#endif

#include "GException.h"
#include "ByteStream.h"
#include "BSByteStream.h"
#include "DjVuPalette.h"
#include <stdlib.h>
#include <math.h>


#define CUBEBITS  4
#define CUBESIDE  (1<<CUBEBITS)
#define CUBESIZE  (CUBESIDE*CUBESIDE*CUBESIDE)

#define RMUL 5
#define GMUL 9
#define BMUL 2
#define SMUL (RMUL+GMUL+BMUL)


inline unsigned char 
umax(unsigned char a, unsigned char b) 
{ return (a>b) ? a : b; }

inline unsigned char 
umin(unsigned char a, unsigned char b) 
{ return (a>b) ? b : a; }



// ------- DJVUPALETTE


bool DjVuPalette::initialized = false;
int DjVuPalette::hind[3][256];

DjVuPalette::DjVuPalette()
  : hcube(0), pcube(0)
{
  // Initialization of static tables
  if (!initialized)
    {
      for (int i=0; i<256; i++)
        {
          hind[0][i] = (i>>(8-CUBEBITS));
          hind[1][i] = hind[0][i] << CUBEBITS;
          hind[2][i] = hind[1][i] << CUBEBITS;
        }
      initialized = true;
    }
}

DjVuPalette::~DjVuPalette()
{
  if (hcube) 
    delete [] hcube;
  if (pcube)
    delete [] pcube;
}

void
DjVuPalette::allocate_hcube()
{
  if (! hcube)  hcube = new PHist[CUBESIZE];
  memset(hcube, 0, sizeof(PHist)*CUBESIZE);
}

void
DjVuPalette::allocate_pcube()
{
  if (! pcube)  pcube = new short[CUBESIZE];
  for (int i=0; i<CUBESIZE; i++) pcube[i] = -1;
}



// -------- PALETTE COMPUTATION


#ifndef NEED_DECODER_ONLY

struct PData
{
  unsigned char p[3];
  int w;
};

struct PBox 
{
  PData *data;
  int colors;
  int sum;
};

int
DjVuPalette::bcomp (const void *a, const void *b)
{
  return ((PData*)a)->p[0] - ((PData*)b)->p[0];
}

int
DjVuPalette::gcomp (const void *a, const void *b)
{
  return ((PData*)a)->p[1] - ((PData*)b)->p[1];
}

int
DjVuPalette::rcomp (const void *a, const void *b)
{
  return ((PData*)a)->p[2] - ((PData*)b)->p[2];
}

int
DjVuPalette::lcomp (const void *a, const void *b)
{
  return ((PColor*)a)->p[3] - ((PColor*)b)->p[3];
}

int
DjVuPalette::compute_palette(int maxcolors)
{
  if (!hcube)
    THROW("Color histogram not found");
  if (maxcolors<1 || maxcolors>1024)
    THROW("Unrealistic number of colors");
  
  // Paul Heckbert: "Color Image Quantization for Frame Buffer Display", 
  // SIGGRAPH '82 Proceedings, page 297.  (also in ppmquant)
  
  // Collect histogram colors
  int sum = 0;
  int ncolors = 0;
  GTArray<PData> pdata;
  for (int i=0; i<CUBESIZE; i++)
    if (hcube[i].w > 0)
      {
        pdata.touch(ncolors);
        PData &data = pdata[ncolors++];
        PHist &hist = hcube[i];
        data.p[0] = hist.p[0]/hist.w;
        data.p[1] = hist.p[1]/hist.w;
        data.p[2] = hist.p[2]/hist.w;
        data.w = hist.w;
        sum += hist.w;
      }
  // Create first box
  GList<PBox> boxes;
  PBox newbox;
  newbox.data = pdata;
  newbox.colors = ncolors;
  newbox.sum = sum;
  boxes.append(newbox);
  // Repeat spliting boxes
  while (boxes.size() < maxcolors)
    {
      // Find suitable box
      GPosition p;
      for (p=boxes; p; ++p)
        if (boxes[p].colors >= 2)
          break;
      if (! p)
        break;
      // Find box boundaries
      PBox &splitbox = boxes[p];
      unsigned char pmax[3];
      unsigned char pmin[3];
      pmax[0] = pmin[0] = splitbox.data->p[0];
      pmax[1] = pmin[1] = splitbox.data->p[1];
      pmax[2] = pmin[2] = splitbox.data->p[2];
      for (int j=1; j<splitbox.colors; j++)
        {
          pmax[0] = umax(pmax[0], splitbox.data[j].p[0]);
          pmax[1] = umax(pmax[1], splitbox.data[j].p[1]);
          pmax[2] = umax(pmax[2], splitbox.data[j].p[2]);
          pmin[0] = umin(pmin[0], splitbox.data[j].p[0]);
          pmin[1] = umin(pmin[1], splitbox.data[j].p[1]);
          pmin[2] = umin(pmin[2], splitbox.data[j].p[2]);
        }
      // Determine split direction and sort
      int blum = (pmax[0]-pmin[0]) * BMUL;
      int glum = (pmax[1]-pmin[1]) * GMUL;
      int rlum = (pmax[2]-pmin[2]) * RMUL;
      if (glum>=rlum && glum>=blum)
        qsort(splitbox.data, splitbox.colors, sizeof(PData), gcomp);
      else if (rlum>blum)
        qsort(splitbox.data, splitbox.colors, sizeof(PData), rcomp);
      else
        qsort(splitbox.data, splitbox.colors, sizeof(PData), bcomp);
      // Find median
      int lowercolors = 0;
      int lowersum = 0;
      while (lowercolors<splitbox.colors-1 && lowersum+lowersum<splitbox.sum)
        lowersum += splitbox.data[lowercolors++].w;
      // Compute new boxes
      newbox.data = splitbox.data + lowercolors;
      newbox.colors = splitbox.colors - lowercolors;
      newbox.sum = splitbox.sum - lowersum;
      splitbox.colors = lowercolors;
      splitbox.sum = lowersum;
      // Insert boxes at proper location
      GPosition q;
      for (q=p; q; ++q)
        if (boxes[q].sum < newbox.sum)
          break;
      boxes.insert_before(q, newbox);
      for (q=p; q; ++q)
        if (boxes[q].sum < splitbox.sum)
          break;
      boxes.insert_before(q, boxes, p);
    }
  // Fill palette array
  ncolors = 0;
  palette.empty();
  palette.resize(0,boxes.size()-1);
  for (GPosition p=boxes; p; ++p)
    {
      PBox &box = boxes[p];
      // Compute box representative color
      int bsum = 0;
      int gsum = 0;
      int rsum = 0;
      for (int j=0; j<box.colors; j++)
        {
          int w = box.data[j].w;
          bsum += box.data[j].p[0] * w;
          gsum += box.data[j].p[1] * w;
          rsum += box.data[j].p[2] * w;
        }
      PColor &color = palette[ncolors++];
      color.p[0] = bsum/box.sum;
      color.p[1] = gsum/box.sum;
      color.p[2] = rsum/box.sum;
      color.p[3] = ( color.p[0]*BMUL + color.p[1]*GMUL + color.p[2]*RMUL) / SMUL;
    }
  // Sort palette colors in luminance order
  qsort((PColor*)palette, ncolors, sizeof(PColor), lcomp);
  if (pcube) allocate_pcube();
  return ncolors;
}

#endif




// -------- QUANTIZATION

int 
DjVuPalette::color_to_index_slow(const unsigned char *bgr)
{
  int ncolors = palette.size();
  if (! ncolors) THROW("Palette is not initialized");
  PColor *pal = palette;
  // Should be able to do better
  int found = 0;
  int founddist = 3*256*256;
  for (int i=0; i<ncolors; i++)
    {
      int bd = bgr[0] - pal[i].p[0];
      int gd = bgr[1] - pal[i].p[1];
      int rd = bgr[2] - pal[i].p[2];
      int dist = (bd*bd)+(gd*gd)+(rd*rd);
      if (dist < founddist)
        {
          found = i;
          founddist = dist;
        }
    }
  return found;
}


#ifndef NEED_DECODER_ONLY

void 
DjVuPalette::quantize(GPixmap &pm)
{
  for (int j=0; j<(int)pm.rows(); j++)
    {
      GPixel *p = pm[j];
      for (int i=0; i<(int)pm.columns(); i++)
        index_to_color(color_to_index(p[i]), p[i]);
    }
}

int 
DjVuPalette::compute_palette_and_quantize(GPixmap &pm, int maxcolors)
{
  // Prepare histogram
  histogram_clear();
  for (int j=0; j<(int)pm.rows(); j++)
    {
      GPixel *p = pm[j];
      for (int i=0; i<(int)pm.columns(); i++)
        histogram_add(p[i], 1);
    }
  // Execute
  int ncolors = compute_palette(maxcolors);
  quantize(pm);
  return ncolors;
}

#endif


// -------- ENCODE AND DECODE


void 
DjVuPalette::encode(ByteStream &bs) const
{
}

void 
DjVuPalette::decode(ByteStream &bs)
{
}


// -------- TEST



#ifdef TEST

int main(int argc, char **argv)
{
  TRY
    {
      if (argc!=3)
        THROW("Usage: quant <ncol> <ppmfile>");
      int maxcolors = atoi(argv[1]);
      StdioByteStream ibs(argv[2],"rb");
      GPixmap pm(ibs);
      DjVuPalette pal;
      int ncolors = pal.compute_palette_and_quantize(pm, maxcolors);
      fprintf(stderr,"%d colors allocated\n", ncolors);
      StdioByteStream obs(stdout,"wb");
      pm.save_ppm(obs);
    }
  CATCH(ex)
    {
      ex.perror();
      exit(10);
    }
  ENDCATCH;
  return 0;
}




#endif
