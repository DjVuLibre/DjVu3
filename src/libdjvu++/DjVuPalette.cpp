//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.5)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: DjVuPalette.cpp,v 1.28 2001-07-24 17:52:04 bcr Exp $
// $Name:  $

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

#define MAXPALETTESIZE 65535 // Limit for a 16 bit unsigned read.


inline unsigned char 
umax(unsigned char a, unsigned char b) 
{ return (a>b) ? a : b; }

inline unsigned char 
umin(unsigned char a, unsigned char b) 
{ return (a>b) ? b : a; }

inline float 
fmin(float a, float b) 
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

DjVuPalette& 
DjVuPalette::operator=(const DjVuPalette &ref)
{
  if (this != &ref)
    {
      if (hcube) { delete [] hcube; hcube=0; }
      if (pcube) { delete [] pcube; pcube=0; }
      palette = ref.palette;
      colordata = ref.colordata;
    }
  return *this;
}

DjVuPalette::DjVuPalette(const DjVuPalette &ref)
  : hcube(0), pcube(0)
{
  this->operator=(ref);
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
  int boxsize;
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
  unsigned char *aa = ((PColor*)a)->p;
  unsigned char *bb = ((PColor*)b)->p;
  if (aa[3] != bb[3])
    return aa[3]-bb[3];
  else if (aa[2] != bb[2])
    return aa[2]-bb[2];
  else if (aa[1] != bb[1])
    return aa[1]=bb[1];
  else
    return aa[0]-bb[0];
}

int
DjVuPalette::compute_palette(int maxcolors, int minboxsize)
{
  if (!hcube)
    G_THROW( ERR_MSG("DjVuPalette.no_color") );
  if (maxcolors<1 || maxcolors>MAXPALETTESIZE)
    G_THROW( ERR_MSG("DjVuPalette.many_colors") );
  
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
        data.p[0] = (unsigned char) fmin(255, static_cast<float>(hist.p[0]/hist.w));
        data.p[1] = (unsigned char) fmin(255, static_cast<float>(hist.p[1]/hist.w));
        data.p[2] = (unsigned char) fmin(255, static_cast<float>(hist.p[2]/hist.w));
        data.w = hist.w;
        sum += data.w;
      }
  // Create first box
  GList<PBox> boxes;
  PBox newbox;
  newbox.data = pdata;
  newbox.colors = ncolors;
  newbox.boxsize = 256;
  newbox.sum = sum;
  boxes.append(newbox);
  // Repeat spliting boxes
  while (boxes.size() < maxcolors)
    {
      // Find suitable box
      GPosition p;
      for (p=boxes; p; ++p)
        if (boxes[p].colors>=2 && boxes[p].boxsize>minboxsize) 
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
      int bl = pmax[0]-pmin[0]; 
      int gl = pmax[1]-pmin[1];
      int rl = pmax[2]-pmin[2];
      splitbox.boxsize = (bl>gl ? (rl>bl ? rl : bl) : (rl>gl ? rl : gl));
      if (splitbox.boxsize <= minboxsize)
        continue;
      if (gl == splitbox.boxsize)
        qsort(splitbox.data, splitbox.colors, sizeof(PData), gcomp);
      else if (rl == splitbox.boxsize)
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
      float bsum = 0;
      float gsum = 0;
      float rsum = 0;
      for (int j=0; j<box.colors; j++)
        {
          float w = (float)box.data[j].w;
          bsum += box.data[j].p[0] * w;
          gsum += box.data[j].p[1] * w;
          rsum += box.data[j].p[2] * w;
        }
      PColor &color = palette[ncolors++];
      color.p[0] = (unsigned char) fmin(255, bsum/box.sum);
      color.p[1] = (unsigned char) fmin(255, gsum/box.sum);
      color.p[2] = (unsigned char) fmin(255, rsum/box.sum);
      color.p[3] = ( color.p[0]*BMUL + color.p[1]*GMUL + color.p[2]*RMUL) / SMUL;
    }
  // Save dominant color
  PColor dcolor = palette[0];
  // Sort palette colors in luminance order
  qsort((PColor*)palette, ncolors, sizeof(PColor), lcomp);
  // Clear invalid data
  colordata.empty();
  if (pcube) 
    allocate_pcube();
  // Return dominant color
  return color_to_index_slow(dcolor.p);
}



int 
DjVuPalette::compute_pixmap_palette(const GPixmap &pm, int ncolors, int minboxsize)
{
  // Prepare histogram
  histogram_clear();
  for (int j=0; j<(int)pm.rows(); j++)
    {
      const GPixel *p = pm[j];
      for (int i=0; i<(int)pm.columns(); i++)
        histogram_add(p[i], 1);
    }
  // Compute palette
  return compute_palette(ncolors, minboxsize);
}


#endif




// -------- QUANTIZATION

int 
DjVuPalette::color_to_index_slow(const unsigned char *bgr)
{
  const int ncolors = palette.size();
  if (! ncolors)
  {
    G_THROW( ERR_MSG("DjVuPalette.not_init") );
  }
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
DjVuPalette::compute_palette_and_quantize(GPixmap &pm, int maxcolors, int minboxsize)
{
  int result = compute_pixmap_palette(pm, maxcolors, minboxsize);
  quantize(pm);
  return result;
}

void 
DjVuPalette::color_correct(double corr)
{
  const int palettesize = palette.size();
  if (palettesize > 0)
    {
      // Copy colors
      int i;
      GTArray<GPixel> pix(0,palettesize-1);
      GPixel *r = pix;
      PColor *q = palette;
      for (i=0; i<palettesize; i++) 
        {
          r[i].b = q[i].p[0];
          r[i].g = q[i].p[1];
          r[i].r = q[i].p[2];
        }
      // Apply color correction
      GPixmap::color_correct(corr, r, palettesize);
      // Restore colors
      for (i=0; i<palettesize; i++) 
        {
          q[i].p[0] = r[i].b;
          q[i].p[1] = r[i].g;
          q[i].p[2] = r[i].r;
        }
    }
}

#endif


// -------- ENCODE AND DECODE

#define DJVUPALETTEVERSION 0

void
DjVuPalette::encode_rgb_entries(ByteStream &bs) const
{
  const int palettesize = palette.size();
  for (int c=0; c<palettesize; c++)
    {
      unsigned char p[3];
      p[2] = palette[c].p[0];
      p[1] = palette[c].p[1];
      p[0] = palette[c].p[2];
      bs.writall((const void*)p, 3);
    }
}

void 
DjVuPalette::encode(GP<ByteStream> gbs) const
{
  ByteStream &bs=*gbs;
  const int palettesize = palette.size();
  const int datasize = colordata.size();
  // Code version number
  int version = DJVUPALETTEVERSION;
  if (datasize>0) version |= 0x80;
  bs.write8(version);
  // Code palette
  bs.write16(palettesize);
  for (int c=0; c<palettesize; c++)
    {
      unsigned char p[3];
      p[0] = palette[c].p[0];
      p[1] = palette[c].p[1];
      p[2] = palette[c].p[2];
      bs.writall((const void*)p, 3);
    }
  // Code colordata
  if (datasize > 0)
    {
      bs.write24(datasize);
      GP<ByteStream> gbsb=BSByteStream::create(gbs, 50);
      ByteStream &bsb=*gbsb;
      for (int d=0; d<datasize; d++)
        bsb.write16(colordata[d]);
    }
}

void 
DjVuPalette::decode_rgb_entries(ByteStream &bs, const int palettesize)
{
  palette.resize(0,palettesize-1);
  for (int c=0; c<palettesize; c++)
    {
      unsigned char p[3];
      bs.readall((void*)p, 3);
      palette[c].p[0] = p[2];
      palette[c].p[1] = p[1];
      palette[c].p[2] = p[0];
      palette[c].p[3] = (p[0]*BMUL+p[1]*GMUL+p[2]*RMUL)/SMUL;
    }
}

void 
DjVuPalette::decode(GP<ByteStream> gbs)
{
  ByteStream &bs=*gbs;
  // Make sure that everything is clear
  if (hcube) 
    delete [] hcube;
  if (pcube)
    delete [] pcube;
  hcube = 0;
  pcube = 0;
  // Code version
  int version = bs.read8();
  if ( (version & 0x7f) != DJVUPALETTEVERSION)
    G_THROW( ERR_MSG("DjVuPalette.bad_version") );
  // Code palette
  const int palettesize = bs.read16();
  if (palettesize<0 || palettesize>MAXPALETTESIZE)
    G_THROW( ERR_MSG("DjVuPalette.bad_palette") );
  palette.resize(0,palettesize-1);
  for (int c=0; c<palettesize; c++)
    {
      unsigned char p[3];
      bs.readall((void*)p, 3);
      palette[c].p[0] = p[0];
      palette[c].p[1] = p[1];
      palette[c].p[2] = p[2];
      palette[c].p[3] = (p[0]*BMUL+p[1]*GMUL+p[2]*RMUL)/SMUL;
    }
  // Code data
  if (version & 0x80)
    {
      int datasize = bs.read24();
      if (datasize<0)
        G_THROW( ERR_MSG("DjVuPalette.bad_palette") );
      colordata.resize(0,datasize-1);
      GP<ByteStream> gbsb=BSByteStream::create(gbs);
      ByteStream &bsb=*gbsb;
      for (int d=0; d<datasize; d++)
        {
          short s = bsb.read16();
          if (s<0 || s>=palettesize)
            G_THROW( ERR_MSG("DjVuPalette.bad_palette") );        
          colordata[d] = s;
        }
    }
}


// -------- TEST


#ifdef TEST
int main(int argc, char **argv)
{

   DArray<GString> dargv(0,argc-1);
   for( int i=0; i < argc; ++i)
   {
      GString g(argv[i]);
      dargv[i]=g.getNative2UTF8();
   }

  G_TRY
    {

      if (argc!=4)
        G_THROW( ERR_MSG("DjVuPalette.test_usage") );
      int maxcolors = dargv[1].toInt(); //atoi(argv[1]);
      int minboxsize = dargv[2].toInt(); //atoi(argv[2]);
      GP<ByteStream> ibs=ByteStream::create(dargv[3],"rb");
      GPixmap pm(*ibs);
      DjVuPalette pal;
      int ncolors = pal.compute_palette_and_quantize(pm, maxcolors, minboxsize);
      DjVuPrintErrorUTF8("%d colors allocated\n", ncolors);
      GP<ByteStream> obs=ByteStream::create(stdout,"wb",false);
      pm.save_ppm(*obs);
    }
  G_CATCH(ex)
    {
      ex.perror();
      exit(10);
    }
  G_ENDCATCH;
  return 0;
}
#endif
