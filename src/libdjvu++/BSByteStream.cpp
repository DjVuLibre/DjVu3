//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1998-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: BSByteStream.cpp,v 1.27 2001-03-06 19:55:41 bcr Exp $
// $Name:  $

// - Author: Leon Bottou, 07/1998


#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "BSByteStream.h"
#undef BSORT_TIMER
#ifdef BSORT_TIMER
#include "GOS.h"
#endif

class BSByteStream::Decode : public BSByteStream
{
public:
  /** Creates a Static object for allocating the memory area of
      length #sz# starting at address #buffer#. */
  Decode(GP<ByteStream> bs);
  ~Decode();
  void init(void);
  // Virtual functions
  virtual size_t read(void *buffer, size_t sz);
  virtual void flush(void);
protected:
  unsigned int decode(void);
private:
  bool eof;
};

// ========================================
// --- Assertion

#define ASSERT(expr) do{if(!(expr))G_THROW("assertion ("#expr") failed");}while(0)

// ========================================
// --- Construction

BSByteStream::BSByteStream(GP<ByteStream> xbs)
: offset(0), bptr(0), blocksize(0), size(0), bs(xbs),
  gbs(xbs), gdata(data,0)
{
  // Initialize context array
  memset(ctx, 0, sizeof(ctx));
}

BSByteStream::~BSByteStream() {}

BSByteStream::Decode::Decode(GP<ByteStream> xbs)
: BSByteStream(xbs), eof(false) {}

void
BSByteStream::Decode::init(void)
{
  gzp=ZPCodec::create(gbs,false,true);
}

BSByteStream::Decode::~Decode() {}

GP<ByteStream>
BSByteStream::create(GP<ByteStream> xbs)
{
  BSByteStream::Decode *rbs=new BSByteStream::Decode(xbs);
  GP<ByteStream> retval=rbs;
  rbs->init();
  return retval;
}

void 
BSByteStream::Decode::flush()
{
  size = bptr = 0;
}

// ========================================
// -- Decoding


static int 
decode_raw(ZPCodec &zp, int bits)
{
  int n = 1;
  int m = (1<<bits);
  while (n < m)
    {
      int b = zp.decoder();
      n = (n<<1) | b;
    }
  return n - m;
}

static inline int 
decode_binary(ZPCodec &zp, BitContext *ctx, int bits)
{
  int n = 1;
  int m = (1<<bits);
  ctx = ctx - 1;
  while (n < m)
    {
      int b = zp.decoder(ctx[n]);
      n = (n<<1) | b;
    }
  return n - m;
}



unsigned int
BSByteStream::Decode::decode(void)
{
  /////////////////////////////////
  ////////////  Decode input stream
  
  int i;
  // Decode block size
  ZPCodec &zp=*gzp;
  size = decode_raw(zp, 24);
  if (!size)
    return 0;
  if (size>MAXBLOCK*1024)
    G_THROW("bytestream.corrupt");        //  Corrupted decoder input
  // Allocate
  if ((int)blocksize < size)
    {
      blocksize = size;
      if (data)
      {
        gdata.resize(0);
      }
    }
  if (! data) 
    gdata.resize(blocksize);
  // Decode Estimation Speed
  int fshift = 0;
  if (zp.decoder())
    {
      fshift += 1;
      if (zp.decoder())
        fshift += 1;
    }
  // Prepare Quasi MTF
  unsigned char mtf[256];
  unsigned int freq[FREQMAX];
  int m = 0;
  for (m=0; m<256; m++)
    mtf[m] = m;
  int fadd = 4;
  for (m=0; m<FREQMAX; m++)
    freq[m]= 0;
  // Decode
  int mtfno = 3;
  int markerpos = -1;
  for (i=0; i<size; i++)
    {
      int ctxid = CTXIDS-1;
      if (ctxid>mtfno) ctxid=mtfno;
      BitContext *cx = ctx;
      if (zp.decoder(cx[ctxid]))
        { mtfno=0; data[i]=mtf[mtfno]; goto rotate; }
      cx+=CTXIDS;
      if (zp.decoder(cx[ctxid]))
        { mtfno=1; data[i]=mtf[mtfno]; goto rotate; } 
      cx+=CTXIDS;
      if (zp.decoder(cx[0]))
        { mtfno=2+decode_binary(zp,cx+1,1); data[i]=mtf[mtfno]; goto rotate; } 
      cx+=1+1;
      if (zp.decoder(cx[0]))
        { mtfno=4+decode_binary(zp,cx+1,2); data[i]=mtf[mtfno]; goto rotate; } 
      cx+=1+3;
      if (zp.decoder(cx[0]))
        { mtfno=8+decode_binary(zp,cx+1,3); data[i]=mtf[mtfno]; goto rotate; } 
      cx+=1+7;
      if (zp.decoder(cx[0]))
        { mtfno=16+decode_binary(zp,cx+1,4); data[i]=mtf[mtfno]; goto rotate; } 
      cx+=1+15;
      if (zp.decoder(cx[0]))
        { mtfno=32+decode_binary(zp,cx+1,5); data[i]=mtf[mtfno]; goto rotate; } 
      cx+=1+31;
      if (zp.decoder(cx[0]))
        { mtfno=64+decode_binary(zp,cx+1,6); data[i]=mtf[mtfno]; goto rotate; } 
      cx+=1+63;
      if (zp.decoder(cx[0]))
        { mtfno=128+decode_binary(zp,cx+1,7); data[i]=mtf[mtfno]; goto rotate; } 
      mtfno=256;
      data[i]=0;
      markerpos=i;
      continue;
      // Rotate mtf according to empirical frequencies (new!)
    rotate:
      // Adjust frequencies for overflow
      int k;
      fadd = fadd + (fadd>>fshift);
      if (fadd > 0x10000000) 
        {
          fadd    >>= 24;
          freq[0] >>= 24;
          freq[1] >>= 24;
          freq[2] >>= 24;
          freq[3] >>= 24;
          for (k=4; k<FREQMAX; k++)
            freq[k] = freq[k]>>24;
        }
      // Relocate new char according to new freq
      unsigned int fc = fadd;
      if (mtfno < FREQMAX)
        fc += freq[mtfno];
      for (k=mtfno; k>=FREQMAX; k--) 
        mtf[k] = mtf[k-1];
      for (; k>0 && fc>=freq[k-1]; k--)
        {
          mtf[k] = mtf[k-1];
          freq[k] = freq[k-1];
        }
      mtf[k] = data[i];
      freq[k] = fc;
    }
  

  /////////////////////////////////
  ////////// Reconstruct the string
  
  if (markerpos<1 || markerpos>=size)
    G_THROW("bytestream.corrupt");        //  Corrupted decoder input
  // Allocate pointers
  unsigned int *posn;
  GPBuffer<unsigned int> gposn(posn,blocksize);
  memset(posn, 0, sizeof(unsigned int)*size);
  // Prepare count buffer
  int count[256];
  for (i=0; i<256; i++)
    count[i] = 0;
  // Fill count buffer
  for (i=0; i<markerpos; i++) 
    {
      unsigned char c = data[i];
      posn[i] = (c<<24) | (count[c] & 0xffffff);
      count[c] += 1;
    }
  for (i=markerpos+1; i<size; i++)
    {
      unsigned char c = data[i];
      posn[i] = (c<<24) | (count[c] & 0xffffff);
      count[c] += 1;
    }
  // Compute sorted char positions
  int last = 1;
  for (i=0; i<256; i++)
    {
      int tmp = count[i];
      count[i] = last;
      last += tmp;
    }
  // Undo the sort transform
  i = 0;
  last = size-1;
  while (last>0)
    {
      unsigned int n = posn[i];
      unsigned char c = (posn[i]>>24);
      data[--last] = c;
      i = count[c] + (n & 0xffffff);
    }
  // Free and check
  if (i != markerpos)
    G_THROW("bytestream.corrupt");        //  Corrupted decoder input
  return size;
}



// ========================================
// -- ByteStream interface



long 
BSByteStream::tell() const
{
  return offset;
}

size_t 
BSByteStream::Decode::read(void *buffer, size_t sz)
{
  if (eof)
    return 0;
  // Loop
  int copied = 0;
  while (sz>0 && !eof)
    {
      // Decode if needed
      if (!size)
        {
          bptr = 0;
          if (! decode()) 
          {
            size = 1 ;
            eof = true;
          }
          size -= 1;
        }
      // Compute remaining
      int bytes = size;
      if (bytes > (int)sz)
        bytes = sz;
      // Transfer
      if (buffer && bytes)
        {
          memcpy(buffer, data+bptr, bytes);
          buffer = (void*)((char*)buffer + bytes);
        }
      size -= bytes;
      bptr += bytes;
      sz -= bytes;
      copied += bytes;
      offset += bytes;
    }
  // Return copied bytes
  return copied;
}

