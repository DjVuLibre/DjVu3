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
//C- $Id: GBitmap.cpp,v 1.11 1999-06-02 23:33:53 leonb Exp $


#ifdef __GNUC__
#pragma implementation
#endif

#include <string.h>
#include "GBitmap.h"
#include "ByteStream.h"
#include "GRect.h"
#include "GString.h"
#include "Arrays.h"


// File "$Id: GBitmap.cpp,v 1.11 1999-06-02 23:33:53 leonb Exp $"
// - Author: Leon Bottou, 05/1997

// ----- constructor and destructor

GBitmap::~GBitmap()
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
    rle(0), rlerows(0), rlelength(0)
{
}

GBitmap::GBitmap(int nrows, int ncolumns, int border)
  : nrows(0), ncolumns(0), border(0), 
    bytes_per_row(0), grays(0), bytes(0), bytes_data(0), 
    rle(0), rlerows(0), rlelength(0)
{
  init(nrows, ncolumns, border);
}

GBitmap::GBitmap(ByteStream &ref, int border)
  : nrows(0), ncolumns(0), border(0), 
    bytes_per_row(0), grays(0), bytes(0), bytes_data(0),
    rle(0), rlerows(0), rlelength(0)
{
  init(ref, border);
}

GBitmap::GBitmap(const GBitmap &ref)
  : nrows(0), ncolumns(0), border(0), 
    bytes_per_row(0), grays(0), bytes(0), bytes_data(0), 
    rle(0), rlerows(0), rlelength(0)
{
  init(ref, ref.border);
}

GBitmap::GBitmap(const GBitmap &ref, int border)
  : nrows(0), ncolumns(0), border(0), 
    bytes_per_row(0), grays(0), bytes(0), bytes_data(0),
    rle(0), rlerows(0), rlelength(0)
{
  init(ref, border);
}


GBitmap::GBitmap(const GBitmap &ref, const GRect &rect, int border)
  : nrows(0), ncolumns(0), border(0), 
    bytes_per_row(0), grays(0), bytes(0), bytes_data(0),
    rle(0), rlerows(0), rlelength(0)
{
  init(ref, rect, border);
}






// ----- initialization

void 
GBitmap::init(int arows, int acolumns, int aborder)
{
  delete [] bytes_data;
  delete [] rle;
  delete [] rlerows;
  bytes = bytes_data = rle = 0;
  rlerows = 0;
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
  if (this != &ref) 
  {
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
  char magic[2];
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
  THROW("Unknown PBM, PGM or RLE file format");
}

void
GBitmap::borrow_data(unsigned char *data, int w, int h)
{
  delete [] bytes_data;
  delete [] rle;
  delete [] rlerows;
  bytes = bytes_data = rle = 0;
  rlerows = 0;
  grays = 2;
  nrows = h;
  ncolumns = w;
  border = 0;
  bytes_data = 0;
  bytes_per_row = w;
  bytes = data;
  rlelength = 0;
}

void
GBitmap::borrow_rle(unsigned char *rledata, unsigned int rledatalen, int w, int h)
{
  delete [] bytes_data;
  delete [] rle;
  delete [] rlerows;
  bytes = bytes_data = rle = 0;
  rlerows = 0;
  grays = 2;
  nrows = h;
  ncolumns = w;
  border = 0;
  bytes_data = 0;
  bytes_per_row = w;
  rle = rledata;
  rlelength = rledatalen;
}


unsigned char *
GBitmap::take_data(size_t &offset)
{
  unsigned char *ret = bytes_data;
  if (ret) offset = (size_t)border;
  bytes_data=0;
  return ret;
}



// ----- compression

void 
GBitmap::compress()
{
  if (grays > 2)
    THROW("Cannot compress gray level bitmap");
  if (!bytes)
    return;
  delete [] rle;
  delete [] rlerows;
  rlerows = 0;
  rlelength = encode(&rle);
  delete [] bytes_data;
  bytes = bytes_data = 0;
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


// ----- gray levels

void
GBitmap::set_grays(int ngrays)
{
  if (ngrays<2 || ngrays>256)
    THROW("(GBitmap::set_grays) Illegal number of gray levels");
  // set gray levels
  grays = ngrays;
  if (ngrays>2 && !bytes)
    decode(rle);
}

void 
GBitmap::change_grays(int ngrays)
{
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
      for (int n=0; n<ncolumns; n++,p++)
        p[n] = conv[ p[n] ];
    }
}

void 
GBitmap::binarize_grays(int threshold)
{
  if (!bytes)
    return;
  for (int row=0; row<nrows; row++)
    {
      unsigned char *p = (*this)[row];
      for (int n=0; n<ncolumns; n++,p++)
        *p = (*p>threshold ? 1 : 0);
    }
  grays = 2;
}


// ----- additive blitting


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
  {
    return;
  }

  // Perform blit
  if (bm->bytes)
    {
      if (!bytes_data)
        decode(rle);
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
        decode(rle);
      // Blit from rle
      const unsigned char *runs = bm->rle;
      unsigned char *drow = bytes_data + border + y*bytes_per_row + x;
      int sr = bm->nrows - 1;
      drow += sr * bytes_per_row;
      int sc = 0;
      char p = 0;
      while (sr >= 0)
        {
          int z = *runs++;
          if (z >= 0xc0)
            z = *runs++ + ((z - 0xc0) << 8);
          if (sc+z > bm->ncolumns)
            THROW("RLE: synchronization lost");
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
  {
    return;
  }

  // Perform subsampling blit
  if (bm->bytes)
    {
      if (!bytes_data)
        decode(rle);
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
        decode(rle);
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
          int z = *runs++;
          if (z >= 0xc0)
            z = *runs++ + ((z - 0xc0) << 8);
          if (sc+z > bm->ncolumns)
            THROW("RLE: synchronization lost");
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
    THROW("Reading bitmap: integer expected");
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
            THROW("Malformed PBM file");
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
  int x, c, n;
  unsigned char h;
  unsigned char p = 0;
  unsigned char *row = bytes_data + border;
  n = nrows - 1;
  row += n * bytes_per_row;
  c = 0;
  while (n >= 0)
    {
      bs.read(&h, 1);
      x = h;
      if (x >= 0xc0)
        {
          bs.read(&h, 1);
          x = h + ((x - 0xc0) << 8);
        }
      if (c+x > ncolumns)
        THROW("RLE: synchronization lost");
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
    THROW("Cannot make PBM file with a gray level bitmap");
  if (!bytes)
    decode(rle);
  // header
  GString head;
  head.format("P%c\n%d %d\n", (raw ? '4' : '1'), ncolumns, nrows);
  bs.writall((void*)(const char *)head, head.length());
  // body
  const unsigned char *row = bytes + border;
  int n = nrows - 1;
  row += n * bytes_per_row;
  while (n >= 0)
    {
      if (raw)
        {
          unsigned char acc = 0;
          unsigned char mask = 0;
          for (int c=0; c<ncolumns; c++)
            {
              if (mask == 0)
                mask = 0x80;
              if (row[c])
                acc |= mask;
              mask >>= 1;
              if (mask==0)
                {
                  bs.write((void*)&acc, 1);
                  acc = mask = 0;
                }
            }
          if (mask != 0)
            bs.write((void*)&acc, 1);
        }
      else
        {
          unsigned char eol='\n';
          for (int c=0; c<ncolumns;)
            {
              unsigned char bit= (row[c] ? '1' : '0');
              bs.write((void*)&bit, 1);
              c += 1;
              if (c==ncolumns || (c&0x3f)==0) 
                bs.write((void*)&eol, 1);          
            }
        }
      // next row
      row -= bytes_per_row;
      n -= 1;
    }
}

void 
GBitmap::save_pgm(ByteStream &bs, int raw)
{
  // checks
  if (!bytes)
    decode(rle);
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
  if (grays > 2)
    THROW("Cannot make PBM file with a gray level bitmap");
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
      unsigned char *runs;
      int size = encode(&runs);
      bs.writall((void*)runs, size);
      delete [] runs;
    }
}


// ------ runs


static unsigned char **
makerows(int nrows, int ncolumns, unsigned char *runs)
{
  int r = nrows;
  unsigned char **rlerows = new (unsigned char*)[nrows];
  while (r-- > 0)
    {
      int c=0;
      rlerows[r] = runs;
      while (c<ncolumns)
        {
          int x = *runs++;
          if (x>=0xC0)
            x = ((x&0x3f)<<8) | (*runs++);
          c += x;
          if (c > ncolumns)
            THROW("(GBitmap::decode) RLE synchronization lost");
        }
    }
  return rlerows;
}


int 
GBitmap::rle_get_bits(int rowno, unsigned char *bits) const
{
  if (!rle)
    return 0;
  if (rowno<0 || rowno>=nrows)
    return 0;
  if (!rlerows)
    *(unsigned char***)&rlerows = makerows(nrows,ncolumns,rle);
  int n = 0;
  int p = 0;
  int c = 0;
  unsigned char *runs = rlerows[rowno];
  while (c < ncolumns)
    {
      int x = *runs++;
      if (x>=0xC0)
        x = ((x&0x3f)<<8) | (*runs++);
      c += x;
      if (c>ncolumns)
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
      int x = *runs++;
      if (x>=0xC0)
        x = ((x&0x3f)<<8) | (*runs++);
      c += x;
      if (n>0 && !x)
        {
          n--;
          d = d-rlens[n];
        }
      else 
        {
          rlens[n++] = c-d;
          d = c;
        }
    }
  return n;
}


int 
GBitmap::rle_get_rect(GRect &rect) const
{
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
          int x = *runs++;
          if (x>=0xC0)
            x = ((x&0x3f)<<8) | (*runs++);
          if (p && x>0)
            {
              if (c < rect.xmin) 
                rect.xmin = c;
              if (c+x > rect.xmax) 
                rect.xmax = c+x-1;
              n += x;
            }
          c += x;
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
  if (nrows==0 || ncolumns==0)
    THROW("Uninitialized bitmap");
  if (!bytes)
    {
      unsigned char *runs = new unsigned char[rlelength];
      memcpy((void*)runs, rle, rlelength);
      *pruns = runs;
      return rlelength;
    }
  // create run array
  int pos = 0;
  int size = 0;
  int maxpos = 1024 + ncolumns + ncolumns;
  unsigned char *runs = new unsigned char[maxpos];
  // encode bitmap as rle
  const unsigned char *row = bytes + border;
  int n = nrows - 1;
  row += n * bytes_per_row;
  while (n >= 0)
    {
      int c = 0;
      unsigned char p = 0;
      if (maxpos < pos+ncolumns+ncolumns+2)
        {
          maxpos += 1024 + ncolumns + ncolumns;
          unsigned char *newruns = new unsigned char[maxpos];
          memcpy(newruns, runs, pos);
          delete [] runs;
          runs = newruns;
        }
      while (c < ncolumns)
        {
          int x = 0;
          if (p) 
            {
              while (c<ncolumns && row[c]) 
                x++, c++;
            } 
          else 
            {
              while (c<ncolumns && !row[c]) 
                x++, c++;            
            }
          while (x >= 0x4000)
            {
              runs[pos++] = 0xFF;
              runs[pos++] = 0xFF;
              runs[pos++] = 0;
              x -= 0x3FFF;
            }
          if (x >= 0xC0) 
            runs[pos++] = (x>>8 | 0xc0);
          runs[pos++] = x & 0xFF;
          p = 1 - p;
        }
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
    THROW("Uninitialized bitmap");
  bytes_per_row = ncolumns + border;
  if (runs==0)
    THROW("(GBitmap::decode) Called with null argument");
  int npixels = nrows * bytes_per_row + border;
  if (!bytes_data)
    bytes = bytes_data = new unsigned char [ npixels ];
  memset(bytes_data, 0, npixels);
  zeroes(bytes_per_row + border);
  // interpret runs data
  int x, c, n;
  unsigned char p = 0;
  unsigned char *row = bytes_data + border;
  n = nrows - 1;
  row += n * bytes_per_row;
  c = 0;
  while (n >= 0)
    {
      x = *runs++;
      if (x >= 0xc0)
        x = *runs++ + ((x - 0xc0) << 8);
      if (c+x > ncolumns)
        THROW("(GBitmap::decode) RLE synchronization lost");
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
      if (zerosize < 256)
        zerosize = 256;
      while (zerosize < required)
        zerosize = 2*zerosize;
      delete [] zerobuffer;
      zerobuffer = new unsigned char[zerosize];
      memset(zerobuffer, 0, zerosize);
    }
}


// Fills a bitmap with the given value
void 
GBitmap::fill(unsigned char value)
{
  for(unsigned int y=0; y<rows(); y++)
    {
      unsigned char* bm_y = (*this)[y];
      for(unsigned int x=0; x<columns(); x++)
        bm_y[x] = value;
    }
}


#ifdef DEBUG
void 
GBitmap::check_border() const
{
  if (bytes)
    {
      const unsigned char *p = (*this)[-1];
      for (int col=-border; col<ncolumns+border; col++)
        if (p[col])
          THROW("debug: zero array is damaged");
      for (int row=0; row<nrows; row++)
        {
          p = (*this)[row];
          for (int col=-border; col<0; col++)
            if (p[col])
              THROW("debug: left border is damaged");
          for (int col=ncolumns; col<ncolumns+border; col++)
            if (p[col])
              THROW("debug: right border is damaged");
        }
    }
}
#endif
