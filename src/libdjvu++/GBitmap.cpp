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
// $Id: GBitmap.cpp,v 1.43 2001-01-04 22:04:55 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GBitmap.h"
#include "ByteStream.h"
#include "GRect.h"
#include "GString.h"
#include "GThreads.h"
#include "GException.h"
#include <string.h>

// File "$Id: GBitmap.cpp,v 1.43 2001-01-04 22:04:55 bcr Exp $"
// - Author: Leon Bottou, 05/1997


// ----- constructor and destructor

GBitmap::~GBitmap()
{
  delete [] bytes_data;
  delete [] rle;
  delete [] rlerows;
}

void
GBitmap::destroy(void)
{
  delete [] bytes_data;
  delete [] rle;
  delete [] rlerows;
  bytes = bytes_data = rle = 0;
  rlerows = 0;
  rlelength = 0;
}

GBitmap::GBitmap()
  : nrows(0), ncolumns(0), border(0), 
    bytes_per_row(0), grays(0), bytes(0), bytes_data(0), 
    rle(0), rlerows(0), rlelength(0),
    monitorptr(0)
{
}

GBitmap::GBitmap(int nrows, int ncolumns, int border)
  : nrows(0), ncolumns(0), border(0), 
    bytes_per_row(0), grays(0), bytes(0), bytes_data(0), 
    rle(0), rlerows(0), rlelength(0),
    monitorptr(0)
{
  G_TRY
  { 
    init(nrows, ncolumns, border);
  }
  G_CATCH_ALL
  {
    destroy();
    G_RETHROW;
  }
  G_ENDCATCH;
}

GBitmap::GBitmap(ByteStream &ref, int border)
  : nrows(0), ncolumns(0), border(0), 
    bytes_per_row(0), grays(0), bytes(0), bytes_data(0),
    rle(0), rlerows(0), rlelength(0),
    monitorptr(0)
{
  G_TRY
  { 
    init(ref, border);
  }
  G_CATCH_ALL
  {
    destroy();
    G_RETHROW;
  }
  G_ENDCATCH;
}

GBitmap::GBitmap(const GBitmap &ref)
  : nrows(0), ncolumns(0), border(0), 
    bytes_per_row(0), grays(0), bytes(0), bytes_data(0), 
    rle(0), rlerows(0), rlelength(0),
    monitorptr(0)
{
  G_TRY
  { 
    init(ref, ref.border);
  }
  G_CATCH_ALL
  {
    destroy();
    G_RETHROW;
  }
  G_ENDCATCH;
}

GBitmap::GBitmap(const GBitmap &ref, int border)
  : nrows(0), ncolumns(0), border(0), 
    bytes_per_row(0), grays(0), bytes(0), bytes_data(0),
    rle(0), rlerows(0), rlelength(0),
    monitorptr(0)
{
  G_TRY
  { 
    init(ref, border);
  }
  G_CATCH_ALL
  {
    destroy();
    G_RETHROW;
  }
  G_ENDCATCH;
}


GBitmap::GBitmap(const GBitmap &ref, const GRect &rect, int border)
  : nrows(0), ncolumns(0), border(0), 
    bytes_per_row(0), grays(0), bytes(0), bytes_data(0),
    rle(0), rlerows(0), rlelength(0),
    monitorptr(0)
{
  G_TRY
  { 
    init(ref, rect, border);
  }
  G_CATCH_ALL
  {
    destroy();
    G_RETHROW;
  }
  G_ENDCATCH;
}






// ----- initialization

void 
GBitmap::init(int arows, int acolumns, int aborder)
{
  GMonitorLock lock(monitor());
  destroy();
  grays = 2;
  nrows = arows;
  ncolumns = acolumns;
  border = aborder;
  bytes_per_row = ncolumns + border;
  int npixels = nrows * bytes_per_row + border;
  zeroes(bytes_per_row + border);
  if (npixels > 0) 
    {
      bytes = bytes_data = new unsigned char [ npixels ];
      memset((void*)bytes_data, 0, npixels);
    }
}


void 
GBitmap::init(const GBitmap &ref, int aborder)
{
  GMonitorLock lock(monitor());
  if (this != &ref) 
    {
      GMonitorLock lock(ref.monitor());
      init(ref.nrows, ref.ncolumns, aborder);
      grays = ref.grays;
      unsigned char *row = bytes_data+border;
      for (int n=0; n<nrows; n++, row+=bytes_per_row)
      memcpy( (void*)row, (void*)ref[n],  ncolumns );
    }
  else if (aborder > border)
    {
      minborder(aborder);
    }
}


void 
GBitmap::init(const GBitmap &ref, const GRect &rect, int border)
{
  GMonitorLock lock(monitor());
  // test bitmap physical equality
  if (this == &ref)
    {
      GBitmap tmp;
      tmp.grays = grays;
      tmp.border = border;
      tmp.bytes_per_row = bytes_per_row;
      tmp.ncolumns = ncolumns;
      tmp.nrows = nrows;
      tmp.bytes = bytes;
      tmp.bytes_data = bytes_data;
      tmp.rle = rle;
      bytes = bytes_data = 0;
      rle = 0;
      init(tmp, rect, border);
    }
  else
    {
      GMonitorLock lock(ref.monitor());
      // create empty bitmap
      init(rect.height(), rect.width(), border);
      grays = ref.grays;
      // compute destination rectangle
      GRect rect2(0, 0, ref.columns(), ref.rows() );
      rect2.intersect(rect2, rect);
      rect2.translate(-rect.xmin, -rect.ymin);
      // copy bits
      if (! rect2.isempty())
        {
          for (int y=rect2.ymin; y<rect2.ymax; y++)
            {
              unsigned char *dst = (*this)[y];
              const unsigned char *src = ref[y+rect.ymin] + rect.xmin;
              for (int x=rect2.xmin; x<rect2.xmax; x++)
                dst[x] = src[x];
            }
        }
    }
}


void 
GBitmap::init(ByteStream &ref, int aborder)
{
  GMonitorLock lock(monitor());
  // Get magic number
  char magic[2];
  magic[0] = magic[1] = 0;
  ref.readall((void*)magic, sizeof(magic));
  char lookahead = '\n';
  int acolumns = read_integer(lookahead, ref);
  int arows = read_integer(lookahead, ref);
  init(arows, acolumns, aborder);
  // go reading file
  if (magic[0]=='P')
    {
      switch(magic[1])
        {
        case '1':
          grays = 2;
          read_pbm_text(ref); 
          return;
        case '2':
          grays = 1 + read_integer(lookahead, ref);
          read_pgm_text(ref); 
          return;
        case '4':
          grays = 2;
          read_pbm_raw(ref); 
          return;
        case '5':
          grays = 1 + read_integer(lookahead, ref);
          read_pgm_raw(ref); 
          return;
        }
    }
  else if (magic[0]=='R')
    {
      switch(magic[1])
        {
        case '4':
          grays = 2;
          read_rle_raw(ref); 
          return;
        }
    }
  G_THROW("GBitmap.bad_format");
}

void
GBitmap::donate_data(unsigned char *data, int w, int h)
{
  destroy();
  grays = 2;
  nrows = h;
  ncolumns = w;
  border = 0;
  bytes_per_row = w;
  bytes = bytes_data = data;
  rlelength = 0;
}

void
GBitmap::donate_rle(unsigned char *rledata, unsigned int rledatalen, int w, int h)
{
  destroy();
  rlerows = 0;
  grays = 2;
  nrows = h;
  ncolumns = w;
  border = 0;
  bytes_per_row = w;
  rle = rledata;
  rlelength = rledatalen;
}


unsigned char *
GBitmap::take_data(size_t &offset)
{
  GMonitorLock lock(monitor());
  unsigned char *ret = bytes_data;
  if (ret) offset = (size_t)border;
  bytes_data=0;
  return ret;
}

const unsigned char *
GBitmap::get_rle(unsigned int &rle_length)
{
  if(!rle)
    compress();
  rle_length=rlelength;
  return rle; 
}

// ----- compression


void 
GBitmap::compress()
{
  if (grays > 2)
    G_THROW("GBitmap.cant_compress");
  GMonitorLock lock(monitor());
  if (bytes)
    {
      delete [] rle;
      delete [] rlerows;
      rle = 0;
      rlerows = 0;
      rlelength = encode(&rle);
      if (rlelength)
        {
          delete [] bytes_data;
          bytes = bytes_data = 0;
        }
    }
}

void
GBitmap::uncompress()
{
  GMonitorLock lock(monitor());
  if (!bytes && rle)
    decode(rle);
}



unsigned int 
GBitmap::get_memory_usage() const
{
  unsigned long usage = sizeof(GBitmap);
  if (bytes)
    usage += nrows * bytes_per_row + border;
  if (rle)
    usage += rlelength;
  return usage;
}


void 
GBitmap::minborder(int minimum)
{
  if (border < minimum)
    {
      GMonitorLock lock(monitor());
      if (border < minimum)
        {
          if (bytes)
            {
              GBitmap tmp(*this, minimum);
              delete [] bytes_data;
              bytes_per_row = tmp.bytes_per_row;
              bytes = bytes_data = tmp.bytes_data;
              tmp.bytes = tmp.bytes_data = 0;
            }
          border = minimum;
          zeroes(border + ncolumns + border);
        }
    }
}


#define NMONITORS 8
static GMonitor monitors[NMONITORS];

void
GBitmap::share()
{
  if (!monitorptr)
    {
      unsigned long x = (unsigned long)this;
      monitorptr = &monitors[(x^(x>>5)) % NMONITORS];
    }
}


// ----- gray levels

void
GBitmap::set_grays(int ngrays)
{
  if (ngrays<2 || ngrays>256)
    G_THROW("GBitmap.bad_levels");
  // set gray levels
  GMonitorLock lock(monitor());
  grays = ngrays;
  if (ngrays>2 && !bytes)
    uncompress();
}

void 
GBitmap::change_grays(int ngrays)
{
  GMonitorLock lock(monitor());
  // set number of grays
  int ng = ngrays - 1;
  int og = grays - 1;
  set_grays(ngrays);
  // setup conversion table
  unsigned char conv[256];
  for (int i=0; i<256; i++)
    {
      if (i > og)
        conv[i] = ng;
      else
        conv[i] = (i*ng+og/2)/og;
    }
  // perform conversion
  for (int row=0; row<nrows; row++)
    {
      unsigned char *p = (*this)[row];
      for (int n=0; n<ncolumns; n++)
        p[n] = conv[ p[n] ];
    }
}

void 
GBitmap::binarize_grays(int threshold)
{
  GMonitorLock lock(monitor());
  if (bytes)
    for (int row=0; row<nrows; row++)
      {
        unsigned char *p = (*this)[row];
        for(unsigned char const * const pend=p+ncolumns;p<pend;++p)
        {
          *p = (*p>threshold) ? 1 : 0;
        }
      }
  grays = 2;
}


// ----- additive blitting

#undef min
#undef max

static inline int
min(int x, int y) 
{ 
  return (x < y ? x : y);
}

static inline int
max(int x, int y) 
{ 
  return (x > y ? x : y);
}

void 
GBitmap::blit(const GBitmap *bm, int x, int y)
{
  // Check boundaries
  if ((x >= ncolumns)              || 
      (y >= nrows)                 ||
      (x + (int)bm->columns() < 0) || 
      (y + (int)bm->rows() < 0)     )
    return;

  // Perform blit
  GMonitorLock lock1(monitor());
  GMonitorLock lock2(bm->monitor());
  if (bm->bytes)
    {
      if (!bytes_data)
        uncompress();
      // Blit from bitmap
      const unsigned char *srow = bm->bytes + bm->border;
      unsigned char *drow = bytes_data + border + y*bytes_per_row + x;
      for (int sr = 0; sr < bm->nrows; sr++)
        {
          if (sr+y>=0 && sr+y<nrows) 
            {
              int sc = max(0, -x);
              int sc1 = min(bm->ncolumns, ncolumns-x);
              while (sc < sc1)
                {
                  drow[sc] += srow[sc];
                  sc += 1;
                }
            }
          srow += bm->bytes_per_row;
          drow += bytes_per_row;
        }
    }
  else if (bm->rle)
    {
      if (!bytes_data)
        uncompress();
      // Blit from rle
      const unsigned char *runs = bm->rle;
      unsigned char *drow = bytes_data + border + y*bytes_per_row + x;
      int sr = bm->nrows - 1;
      drow += sr * bytes_per_row;
      int sc = 0;
      char p = 0;
      while (sr >= 0)
        {
          const int z = read_run(runs);
          if (sc+z > bm->ncolumns)
            G_THROW("GBitmap.lost_sync");
          int nc = sc + z;
          if (p && sr+y>=0 && sr+y<nrows) 
            {
              if (sc + x < 0) 
                sc = min(-x, nc); 
              while (sc < nc && sc + x<ncolumns)
                drow[sc++] += 1;
            }
          sc = nc;
          p = 1 - p;
          if (sc >= bm->ncolumns) 
            {
              p = 0;
              sc = 0;
              drow -= bytes_per_row;
              sr -= 1; 
            }
        }
    }
}



void 
GBitmap::blit(const GBitmap *bm, int xh, int yh, int subsample)
{
  // Use code when no subsampling is necessary
  if (subsample == 1)
    {
      blit(bm, xh, yh);
      return;
    }

  // Check boundaries
  if ((xh >= ncolumns * subsample) || 
      (yh >= nrows * subsample)    ||
      (xh + (int)bm->columns() < 0)   || 
      (yh + (int)bm->rows() < 0)     )
    return;

  // Perform subsampling blit
  GMonitorLock lock1(monitor());
  GMonitorLock lock2(bm->monitor());
  if (bm->bytes)
    {
      if (!bytes_data)
        uncompress();
      // Blit from bitmap
      int dr, dr1, zdc, zdc1;
      euclidian_ratio(yh, subsample, dr, dr1);
      euclidian_ratio(xh, subsample, zdc, zdc1);
      const unsigned char *srow = bm->bytes + bm->border;
      unsigned char *drow = bytes_data + border + dr*bytes_per_row;
      for (int sr = 0; sr < bm->nrows; sr++)
        {
          if (dr>=0 && dr<nrows) 
            {
              int dc = zdc;
              int dc1 = zdc1;
              for (int sc=0; sc < bm->ncolumns; sc++) 
                {
                  if (dc>=0 && dc<ncolumns)
                    drow[dc] += srow[sc];
                  if (++dc1 >= subsample) 
                    {
                      dc1 = 0;
                      dc += 1;
                    }
                }
            }
          // next line in source
          srow += bm->bytes_per_row;
          // next line fraction in destination
          if (++dr1 >= subsample)
            {
              dr1 = 0;
              dr += 1;
              drow += bytes_per_row;
            }
        }
    }
  else if (bm->rle)
    {
      if (!bytes_data)
        uncompress();
      // Blit from rle
      int dr, dr1, zdc, zdc1;
      euclidian_ratio(yh+bm->nrows-1, subsample, dr, dr1);
      euclidian_ratio(xh, subsample, zdc, zdc1);
      const unsigned char *runs = bm->rle;
      unsigned char *drow = bytes_data + border + dr*bytes_per_row;
      int sr = bm->nrows -1;
      int sc = 0;
      char p = 0;
      int dc = zdc;
      int dc1 = zdc1;
      while (sr >= 0)
        {
          int z = read_run(runs);
          if (sc+z > bm->ncolumns)
            G_THROW("GBitmap.lost_sync");
          int nc = sc + z;

          if (dr>=0 && dr<nrows)
            while (z>0 && dc<ncolumns)
              {
                int zd = subsample - dc1;
                if (zd > z) 
                  zd = z;
                if (p && dc>=0) 
                  drow[dc] += zd;
                z -= zd;
                dc1 += zd;
                if (dc1 >= subsample)
                  {
                    dc1 = 0;
                    dc += 1;
                  }
              }
          // next fractional row
          sc = nc;
          p = 1 - p;
          if (sc >= bm->ncolumns) 
            {
              sc = 0;
              dc = zdc;
              dc1 = zdc1;
              p = 0;
              sr -= 1; 
              if (--dr1 < 0)
                {
                  dr1 = subsample - 1;
                  dr -= 1;
                  drow -= bytes_per_row;
                }
            }
        }
    }
}



// ------ load bitmaps


unsigned int 
GBitmap::read_integer(char &c, ByteStream &bs)
{
  unsigned int x = 0;
  // eat blank before integer
  while (c==' ' || c=='\t' || c=='\r' || c=='\n' || c=='#') 
    {
      if (c=='#') 
        do { } while (bs.read(&c,1) && c!='\n' && c!='\r');
      c = 0; 
      bs.read(&c, 1);
    }
  // check integer
  if (c<'0' || c>'9')
    G_THROW("GBitmap.not_int");
  // eat integer
  while (c>='0' && c<='9') 
    {
      x = x*10 + c - '0';
      c = 0;
      bs.read(&c, 1);
    }
  return x;
}


void 
GBitmap::read_pbm_text(ByteStream &bs)
{
  unsigned char *row = bytes_data + border;
  row += (nrows-1) * bytes_per_row;
  for (int n = nrows-1; n>=0; n--) 
    {
      for (int c = 0; c<ncolumns; c++) 
        {
          char bit = 0;
          bs.read(&bit,1);
          while (bit==' ' || bit=='\t' || bit=='\r' || bit=='\n')
            { 
              bit=0; 
              bs.read(&bit,1); 
            }
          if (bit=='1')
            row[c] = 1;
          else if (bit=='0')
            row[c] = 0;
          else
            G_THROW("GBitmap.bad_PBM");
        }
      row -= bytes_per_row;
    }
}

void 
GBitmap::read_pgm_text(ByteStream &bs)
{
  unsigned char *row = bytes_data + border;
  row += (nrows-1) * bytes_per_row;
  char lookahead = '\n';
  for (int n = nrows-1; n>=0; n--) 
    {
      for (int c = 0; c<ncolumns; c++)
        row[c] = grays - 1 - read_integer(lookahead, bs);
      row -= bytes_per_row;
    }
}

void 
GBitmap::read_pbm_raw(ByteStream &bs)
{
  unsigned char *row = bytes_data + border;
  row += (nrows-1) * bytes_per_row;
  for (int n = nrows-1; n>=0; n--) 
    {
      unsigned char acc = 0;
      unsigned char mask = 0;
      for (int c = 0; c<ncolumns; c++)
        {
          if (!mask) 
            {
              bs.read(&acc, 1);
              mask = (unsigned char)0x80;
            }
          if (acc & mask)
            row[c] = 1;
          else
            row[c] = 0;
          mask >>= 1;
        }
      row -= bytes_per_row;
    }
}

void 
GBitmap::read_pgm_raw(ByteStream &bs)
{
  unsigned char *row = bytes_data + border;
  row += (nrows-1) * bytes_per_row;
  for (int n = nrows-1; n>=0; n--) 
    {
      for (int c = 0; c<ncolumns; c++)
        {
          unsigned char x;
          bs.read((void*)&x, 1);
          row[c] = grays - 1 - x;
        }
      row -= bytes_per_row;
    }
}

void 
GBitmap::read_rle_raw(ByteStream &bs)
{
  // interpret runs data
  unsigned char h;
  unsigned char p = 0;
  unsigned char *row = bytes_data + border;
  int n = nrows - 1;
  row += n * bytes_per_row;
  int c = 0;
  while (n >= 0)
    {
      bs.read(&h, 1);
      int x = h;
      if (x >= (int)RUNOVERFLOWVALUE)
        {
          bs.read(&h, 1);
          x = h + ((x - (int)RUNOVERFLOWVALUE) << 8);
        }
      if (c+x > ncolumns)
        G_THROW("GBitmap.lost_sync");
      while (x-- > 0)
        row[c++] = p;
      p = 1 - p;
      if (c >= ncolumns) 
        {
          c = 0;
          p = 0;
          row -= bytes_per_row;
          n -= 1; 
        }
    }
}


// ------ save bitmaps

void 
GBitmap::save_pbm(ByteStream &bs, int raw)
{
  // check arguments
  if (grays > 2)
    G_THROW("GBitmap.cant_make_PBM");
  GMonitorLock lock(monitor());
  // header
  {
    GString head;
    head.format("P%c\n%d %d\n", (raw ? '4' : '1'), ncolumns, nrows);
    bs.writall((void*)(const char *)head, head.length());
  }
  // body
  if(raw)
  {
    if(!rle)
      compress();
    const unsigned char *runs=rle;
    const unsigned char * const runs_end=rle+rlelength;
    const int count=(ncolumns+7)>>3;
    unsigned char *buf=new unsigned char[count];
    while(runs<runs_end)
    {
      rle_get_bitmap(ncolumns,runs,buf,false);
      bs.writall(buf,count);
    }
    delete [] buf;
  }else
  {
    if (!bytes)
      uncompress();
    const unsigned char *row = bytes + border;
    int n = nrows - 1;
    row += n * bytes_per_row;
    while (n >= 0)
    {
      unsigned char eol='\n';
      for (int c=0; c<ncolumns;)
      {
        unsigned char bit= (row[c] ? '1' : '0');
        bs.write((void*)&bit, 1);
        c += 1;
        if (c==ncolumns || (c&(int)RUNMSBMASK)==0) 
          bs.write((void*)&eol, 1);          
       }
      // next row
      row -= bytes_per_row;
      n -= 1;
    }
  }
}

void 
GBitmap::save_pgm(ByteStream &bs, int raw)
{
  // checks
  GMonitorLock lock(monitor());
  if (!bytes)
    uncompress();
  // header
  GString head;
  head.format("P%c\n%d %d\n%d\n", (raw ? '5' : '2'), ncolumns, nrows, grays-1);
  bs.writall((void*)(const char *)head, head.length());
  // body
  const unsigned char *row = bytes + border;
  int n = nrows - 1;
  row += n * bytes_per_row;
  while (n >= 0)
    {
      if (raw)
        {
          for (int c=0; c<ncolumns; c++)
            {
              char x = grays - 1 - row[c];
              bs.write((void*)&x, 1);
            }
        }
      else 
        {
          unsigned char eol='\n';
          for (int c=0; c<ncolumns; )
            {
              head.format("%d ", grays - 1 - row[c]);
              bs.writall((void*)(const char *)head, head.length());
              c += 1;
              if (c==ncolumns || (c&0x1f)==0) 
                bs.write((void*)&eol, 1);          
            }
        }
      row -= bytes_per_row;
      n -= 1;
    }
}

void 
GBitmap::save_rle(ByteStream &bs)
{
  // checks
  if (ncolumns==0 || nrows==0)
    G_THROW("GBitmap.not_init");
  GMonitorLock lock(monitor());
  if (grays > 2)
    G_THROW("GBitmap.cant_make_PBM");
  // header
  GString head;
  head.format("R4\n%d %d\n", ncolumns, nrows);
  bs.writall((void*)(const char *)head, head.length());
  // body
  if (rle)
    {
      bs.writall((void*)rle, rlelength);
    }
  else
    {
      unsigned char *runs = 0;
      int size = encode(&runs);
      bs.writall((void*)runs, size);
      delete [] runs;
    }
}


// ------ runs


unsigned char **
GBitmap::makerows(int nrows, int ncolumns, unsigned char *runs)
{
  int r = nrows;
  unsigned char **rlerows = new unsigned char* [nrows];
  while (r-- > 0)
    {
      rlerows[r] = runs;
      int c;
      for(c=0;c<ncolumns;c+=GBitmap::read_run(runs))
      	EMPTY_LOOP;
      if (c > ncolumns)
        G_THROW("GBitmap.lost_sync2");
    }
  return rlerows;
}


void
GBitmap::rle_get_bitmap (
  const int ncolumns,
  const unsigned char *&runs,
  unsigned char *bitmap,
  const bool invert )
{
  const int obyte_def=invert?0xff:0;
  const int obyte_ndef=invert?0:0xff;
  int mask=0x80,obyte=0;
  for(int c=ncolumns;c > 0 ;)
  {
    int x=read_run(runs);
    c-=x;
    while((x--)>0)
    {
      if(!(mask>>=1))
      {
        *(bitmap++) = obyte^obyte_def;
        obyte=0;
        mask=0x80;
        for(;x>=8;x-=8)
        {
          *(bitmap++)=obyte_def;
        }
      }
    }
    if(c>0)
    {
      int x=read_run(runs);
      c-=x;
      while((x--)>0)
      {
        obyte|=mask;
        if(!(mask>>=1))
        {
          *(bitmap++)=obyte^obyte_def;
          obyte=0;
          mask=0x80;
          for(;(x>8);x-=8)
            *(bitmap++)=obyte_ndef;
        }
      }
    }
  }
  if(mask != 0x80)
  {
    *(bitmap++)=obyte^obyte_def;
  }
}

int 
GBitmap::rle_get_bits(int rowno, unsigned char *bits) const
{
  GMonitorLock lock(monitor());
  if (!rle)
    return 0;
  if (rowno<0 || rowno>=nrows)
    return 0;
  if (!rlerows)
    (unsigned char**&)rlerows = makerows(nrows,ncolumns,rle);
  int n = 0;
  int p = 0;
  int c = 0;
  unsigned char *runs = rlerows[rowno];
  while (c < ncolumns)
    {
      const int x=read_run(runs);
      if ((c+=x)>ncolumns)
        c = ncolumns;
      while (n<c)
        bits[n++] = p;
      p = 1-p;
    }
  return n;
}


int 
GBitmap::rle_get_runs(int rowno, int *rlens) const
{
  GMonitorLock lock(monitor());
  if (!rle)
    return 0;
  if (rowno<0 || rowno>=nrows)
    return 0;
  if (!rlerows)
    *(unsigned char***)&rlerows = makerows(nrows,ncolumns,rle);
  int n = 0;
  int d = 0;
  int c = 0;
  unsigned char *runs = rlerows[rowno];
  while (c < ncolumns)
    {
      const int x=read_run(runs);
      if (n>0 && !x)
        {
          n--;
          d = d-rlens[n];
        }
      else 
        {
          rlens[n++] = (c+=x)-d;
          d = c;
        }
    }
  return n;
}


int 
GBitmap::rle_get_rect(GRect &rect) const
{
  GMonitorLock lock(monitor());
  if (!rle) 
    return 0;
  int area = 0;
  unsigned char *runs = rle;
  rect.xmin = ncolumns;
  rect.ymin = nrows;
  rect.xmax = 0;
  rect.ymax = 0;
  int r = nrows;
  while (--r >= 0)
    {
      int p = 0;
      int c = 0;
      int n = 0;
      while (c < ncolumns)
        {
          const int x=read_run(runs);
          if(x)
            {
              if (p)
                {
                  if (c < rect.xmin) 
                    rect.xmin = c;
                  if ((c += x) > rect.xmax) 
                    rect.xmax = c-1;
                  n += x;
                }
              else
                {
                  c += x;
                }
            }
          p = 1-p;
        }
      area += n;
      if (n)
        {
          rect.ymin = r;
          if (r > rect.ymax) 
            rect.ymax = r;
        }
    }
  if (area==0)
    rect.clear();
  return area;
}



// ------ helpers

int
GBitmap::encode(unsigned char **pruns) const
{
  // uncompress rle information
  *pruns = 0;
  if (nrows==0 || ncolumns==0)
    return 0;
  if (!bytes)
    {
      unsigned char *runs = new unsigned char[rlelength];
      memcpy((void*)runs, rle, rlelength);
      *pruns = runs;
      return rlelength;
    }
  // create run array
  int pos = 0;
  int maxpos = 1024 + ncolumns + ncolumns;
  unsigned char *runs = new unsigned char[maxpos];
  // encode bitmap as rle
  const unsigned char *row = bytes + border;
  int n = nrows - 1;
  row += n * bytes_per_row;
  while (n >= 0)
    {
      if (maxpos < pos+ncolumns+ncolumns+2)
        {
          maxpos += 1024 + ncolumns + ncolumns;
          unsigned char *newruns = new unsigned char[maxpos];
          memcpy(newruns, runs, pos);
          delete [] runs;
          runs = newruns;
        }

      unsigned char *runs_pos=runs+pos;
      const unsigned char * const runs_pos_start=runs_pos;
      append_line(runs_pos,row,ncolumns);
      pos+=(size_t)runs_pos-(size_t)runs_pos_start;
      row -= bytes_per_row;
      n -= 1;
    }
  // return result
  *pruns = new unsigned char[pos];
  memcpy((void*)*pruns, (void*)(unsigned char*)runs, pos);
  delete [] runs;
  return pos;
}

void 
GBitmap::decode(unsigned char *runs)
{
  // initialize pixel array
  if (nrows==0 || ncolumns==0)
    G_THROW("GBitmap.not_init");
  bytes_per_row = ncolumns + border;
  if (runs==0)
    G_THROW("GBitmap.null_arg");
  int npixels = nrows * bytes_per_row + border;
  if (!bytes_data)
    bytes = bytes_data = new unsigned char [ npixels ];
  memset(bytes_data, 0, npixels);
  zeroes(bytes_per_row + border);
  // interpret runs data
  int c, n;
  unsigned char p = 0;
  unsigned char *row = bytes_data + border;
  n = nrows - 1;
  row += n * bytes_per_row;
  c = 0;
  while (n >= 0)
    {
      int x = read_run(runs);
      if (c+x > ncolumns)
        G_THROW("GBitmap.lost_sync2");
      while (x-- > 0)
        row[c++] = p;
      p = 1 - p;
      if (c >= ncolumns) 
        {
          c = 0;
          p = 0;
          row -= bytes_per_row;
          n -= 1; 
        }
    }
  // Free rle data possibly attached to this bitmap
  delete [] rle;
  delete [] rlerows;
  rle = 0;
  rlerows = 0;
  rlelength = 0;
#ifdef DEBUG
  check_border();
#endif
}


int GBitmap::zerosize = 0;
unsigned char *GBitmap::zerobuffer = 0;

void
GBitmap::zeroes(int required)
{
  if (zerosize < required)
    {
      GMonitorLock lock(&monitors[0]); // any monitor would do
      if (zerosize < required)
        {
          if (zerosize < 256)
            zerosize = 256;
          while (zerosize < required)
            zerosize = 2*zerosize;
          delete [] zerobuffer;
          zerobuffer = new unsigned char[zerosize];
          memset(zerobuffer, 0, zerosize);
        }
    }
}


// Fills a bitmap with the given value
void 
GBitmap::fill(unsigned char value)
{
  GMonitorLock lock(monitor());
  for(unsigned int y=0; y<rows(); y++)
    {
      unsigned char* bm_y = (*this)[y];
      for(unsigned int x=0; x<columns(); x++)
        bm_y[x] = value;
    }
}


void 
GBitmap::append_long_run(unsigned char *&data, int count)
{
  while (count > MAXRUNSIZE)
    {
      data[0] = data[1] = 0xff;
      data[2] = 0;
      data += 3;
      count -= MAXRUNSIZE;
    }
  if (count < RUNOVERFLOWVALUE)
    {
      data[0] = count;
      data += 1;
    }
  else
    {
      data[0] = (count>>8) + GBitmap::RUNOVERFLOWVALUE;
      data[1] = (count & 0xff);
      data += 2;
    }
}


void
GBitmap::append_line(unsigned char *&data,const unsigned char *row,
                     const int rowlen,bool invert)
{
  const unsigned char *rowend=row+rowlen;
  bool p=!invert;
  while(row<rowend)
    {
      int count=0;
      if ((p=!p)) 
        {
          if(*row)
            for(++count,++row;(row<rowend)&&*row;++count,++row)
            	EMPTY_LOOP;
        } 
      else if(!*row)
        {
          for(++count,++row;(row<rowend)&&!*row;++count,++row)
          	EMPTY_LOOP;
        }
      append_run(data,count);
    }
}

#ifdef DEBUG
void 
GBitmap::check_border() const
{int col ;
  if (bytes)
    {
      const unsigned char *p = (*this)[-1];
      for (col=-border; col<ncolumns+border; col++)
        if (p[col])
          G_THROW("GBitmap.zero_damaged");
      for (int row=0; row<nrows; row++)
        {
          p = (*this)[row];
          for (col=-border; col<0; col++)
            if (p[col])
              G_THROW("GBitmap.left_damaged");
          for (col=ncolumns; col<ncolumns+border; col++)
            if (p[col])
              G_THROW("GBitmap.right_damaged");
        }
    }
}
#endif

