//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1998 AT&T Corp.
//C- Copyright (c) 2000 LizardTech Inc.
//C- All rights reserved.
//C-
//C- This software may only be used by you under license from LizardTech
//C- Inc. A copy of LizardTech's Source Code Agreement is available at
//C- LizardTech's Internet website having the URL <http://www.djvu.com/open>.
//C- If you received this software without first entering into a license with
//C- LizardTech, you have an infringing copy of this software and cannot use it
//C- without violating LizardTech's intellectual property rights.
//C-
//C- $Id: BSByteStream.cpp,v 1.19 2000-10-06 21:47:21 fcrary Exp $
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


// ========================================
// --- Assertion

#define ASSERT(expr) do{if(!(expr))G_THROW("assertion ("#expr") failed");}while(0)
#define STR_(x)      #x
#define STR(x)      STR_(x)   



// ========================================
// --- Global Definitions
            

// Limits on block sizes
#define MINBLOCK           10
#define MAXBLOCK           4096

// Overflow required when encoding
#define OVERFLOW           32

// Sorting tresholds
#define RANKSORT_THRESH    10
#define QUICKSORT_STACK    512
#define PRESORT_THRESH     10
#define PRESORT_DEPTH      8
#define RADIX_THRESH       32768


// ========================================
// -- Sorting Routines

  
#ifndef NEED_DECODER_ONLY

class _BSort  // DJVU_CLASS
{
public:
  ~_BSort();
  _BSort(unsigned char *data, int size);
  void run(int &markerpos);
private:
  // Members
  int            size;
  unsigned char *data;
  unsigned int  *posn;
  int           *rank;
  // Helpers
  inline int GT(int p1, int p2, int depth);
  inline int GTD(int p1, int p2, int depth);
  // -- final in-depth sort
  void ranksort(int lo, int hi, int d);
  // -- doubling sort
  int  pivot3r(int *rr, int lo, int hi);
  void quicksort3r(int lo, int hi, int d);
  // -- presort to depth PRESORT_DEPTH
  unsigned char pivot3d(unsigned char *dd, int lo, int hi);
  void quicksort3d(int lo, int hi, int d);
  // -- radixsort
  void radixsort16(void);
  void radixsort8(void);
};


// blocksort -- the main entry point

static void 
blocksort(unsigned char *data, int size, int &markerpos)
{
  _BSort bsort(data, size);
  bsort.run(markerpos);
}


// _BSort construction

_BSort::_BSort(unsigned char *data, int size)
  : size(size), data(data), posn(0), rank(0)
{
  ASSERT(size>0 && size<0x1000000);
  posn = new unsigned int [size];
  rank = new int[size+1];
  rank[size] = -1;
}

_BSort::~_BSort()
{
  delete [] posn;
  delete [] rank;
}



// GT -- compare suffixes using rank information

inline int 
_BSort::GT(int p1, int p2, int depth)
{
  int r1, r2;
  int twod = depth + depth;
  while (1)
    {
      r1=rank[p1+depth]; r2=rank[p2+depth];
      p1+=twod;  p2+=twod;
      if (r1!=r2) 
        return (r1>r2);
      r1=rank[p1]; r2=rank[p2];
      if (r1!=r2) 
        return (r1>r2);
      r1=rank[p1+depth]; r2=rank[p2+depth];
      p1+=twod;  p2+=twod;
      if (r1!=r2) 
        return (r1>r2);
      r1=rank[p1]; r2=rank[p2];
      if (r1!=r2) 
        return (r1>r2);
      r1=rank[p1+depth]; r2=rank[p2+depth];
      p1+=twod;  p2+=twod;
      if (r1!=r2) 
        return (r1>r2);
      r1=rank[p1]; r2=rank[p2];
      if (r1!=r2) 
        return (r1>r2);
      r1=rank[p1+depth]; r2=rank[p2+depth];
      p1+=twod;  p2+=twod;
      if (r1!=r2) 
        return (r1>r2);
      r1=rank[p1]; r2=rank[p2];
      if (r1!=r2) 
        return (r1>r2);
    };
}


// _BSort::ranksort -- 
// -- a simple insertion sort based on GT

void 
_BSort::ranksort(int lo, int hi, int depth)
{
  int i,j;
  for (i=lo+1; i<=hi; i++)
    {
      int tmp = posn[i];
      for(j=i-1; j>=lo && GT(posn[j], tmp, depth); j--)
        posn[j+1] = posn[j];
      posn[j+1] = tmp;
    }
  for(i=lo;i<=hi;i++) 
    rank[posn[i]]=i;
}

// pivot -- return suitable pivot

inline int
_BSort::pivot3r(int *rr, int lo, int hi)
{
  int c1, c2, c3;
  if (hi-lo > 256)
    {
      c1 = pivot3r(rr, lo, (6*lo+2*hi)/8);
      c2 = pivot3r(rr, (5*lo+3*hi)/8, (3*lo+5*hi)/8);
      c3 = pivot3r(rr, (2*lo+6*hi)/8, hi);
    }
  else
    {
      c1 = rr[posn[lo]];
      c2 = rr[posn[(lo+hi)/2]];
      c3 = rr[posn[hi]];
    }
  // Extract median
  if (c1>c3)
    { int tmp=c1; c1=c3; c3=tmp; }
  if (c2<=c1)
    return c1;
  else if (c2>=c3)
    return c3;
  else
    return c2;
}


// _BSort::quicksort3r -- Three way quicksort algorithm 
//    Sort suffixes based on rank at pos+depth
//    The algorithm breaks into ranksort when size is 
//    smaller than RANKSORT_THRESH

static inline int
mini(int a, int b) 
{
  return (a<=b) ? a : b;
}

static inline void
vswap(int i, int j, int n, unsigned int *x)
{
  while (n-- > 0) 
    { int tmp = x[i]; x[i++]=x[j]; x[j++]=tmp; }
}

void 
_BSort::quicksort3r(int lo, int hi, int depth)
{
  /* Initialize stack */
  int slo[QUICKSORT_STACK];
  int shi[QUICKSORT_STACK];
  int sp = 1;
  slo[0] = lo;
  shi[0] = hi;
  // Recursion elimination loop
  while (--sp>=0)
    {
      lo = slo[sp];
      hi = shi[sp];
      // Test for insertion sort
      if (hi-lo<RANKSORT_THRESH)
        {
          ranksort(lo, hi, depth);
        }
      else
        {
          int tmp;
          int *rr=rank+depth;
          int med = pivot3r(rr,lo,hi);
          // -- positions are organized as follows:
          //   [lo..l1[ [l1..l[ ]h..h1] ]h1..hi]
          //      =        <       >        =
          int l1 = lo;
          int h1 = hi;
          while (rr[posn[l1]]==med && l1<h1) { l1++; }
          while (rr[posn[h1]]==med && l1<h1) { h1--; }
          int l = l1;
          int h = h1;
          // -- partition set
          for (;;)
            {
              while (l<=h)
                {
                  int c = rr[posn[l]] - med;
                  if (c > 0) break;
                  if (c == 0) { tmp=posn[l]; posn[l]=posn[l1]; posn[l1++]=tmp; }
                  l++;
                }
              while (l<=h)
                {
                  int c = rr[posn[h]] - med;
                  if (c < 0) break;
                  if (c == 0) { tmp=posn[h]; posn[h]=posn[h1]; posn[h1--]=tmp; }
                  h--;
                }
              if (l>h) break;
              tmp=posn[l]; posn[l]=posn[h]; posn[h]=tmp;
            }
          // -- reorganize as follows
          //   [lo..l1[ [l1..h1] ]h1..hi]
          //      <        =        > 
          tmp = mini(l1-lo, l-l1);
          vswap(lo, l-tmp, tmp, posn);
          l1 = lo + (l-l1);
          tmp = mini(hi-h1, h1-h);
          vswap(hi-tmp+1, h+1, tmp, posn);
          h1 = hi - (h1-h);
          // -- process segments
          ASSERT(sp+2<QUICKSORT_STACK);
          // ----- middle segment (=?) [l1, h1]
          for(int i=l1;i<=h1;i++) 
            rank[posn[i]] = h1;
          // ----- lower segment (<) [lo, l1[
          if (l1 > lo)
            {
              for(int i=lo;i<l1;i++) 
                rank[posn[i]]=l1-1;
              slo[sp]=lo;
              shi[sp]=l1-1;
              if (slo[sp] < shi[sp])  
                sp++;
            }
          // ----- upper segment (>) ]h1, hi]
          if (h1 < hi)
            {
              slo[sp]=h1+1;
              shi[sp]=hi;
              if (slo[sp] < shi[sp])  
                sp++;
            }
        }
    }
}






// GTD -- compare suffixes using data information 
//  (up to depth PRESORT_DEPTH)

inline int 
_BSort::GTD(int p1, int p2, int depth)
{
  unsigned char c1, c2;
  p1+=depth; p2+=depth;
  while (depth < PRESORT_DEPTH)
    {
      // Perform two
      c1=data[p1]; c2=data[p2];
      if (c1!=c2) 
        return (c1>c2);
      c1=data[p1+1]; c2=data[p2+1];
      p1+=2;  p2+=2; depth+=2;
      if (c1!=c2) 
        return (c1>c2);
    }
  if (p1<size && p2<size)
    return 0;
  return (p1<p2);
}

// pivot3d -- return suitable pivot

inline unsigned char
_BSort::pivot3d(unsigned char *rr, int lo, int hi)
{
  unsigned char c1, c2, c3;
  if (hi-lo > 256)
    {
      c1 = pivot3d(rr, lo, (6*lo+2*hi)/8);
      c2 = pivot3d(rr, (5*lo+3*hi)/8, (3*lo+5*hi)/8);
      c3 = pivot3d(rr, (2*lo+6*hi)/8, hi);
    }
  else
    {
      c1 = rr[posn[lo]];
      c2 = rr[posn[(lo+hi)/2]];
      c3 = rr[posn[hi]];
    }
  // Extract median
  if (c1>c3)
    { int tmp=c1; c1=c3; c3=tmp; }
  if (c2<=c1)
    return c1;
  else if (c2>=c3)
    return c3;
  else
    return c2;
}


// _BSort::quicksort3d -- Three way quicksort algorithm 
//    Sort suffixes based on strings until reaching
//    depth rank at pos+depth
//    The algorithm breaks into ranksort when size is 
//    smaller than PRESORT_THRESH

void 
_BSort::quicksort3d(int lo, int hi, int depth)
{
  /* Initialize stack */
  int slo[QUICKSORT_STACK];
  int shi[QUICKSORT_STACK];
  int sd[QUICKSORT_STACK];
  int sp = 1;
  slo[0] = lo;
  shi[0] = hi;
  sd[0] = depth;
  // Recursion elimination loop
  while (--sp>=0)
    {
      lo = slo[sp];
      hi = shi[sp];
      depth = sd[sp];
      // Test for insertion sort
      if (depth >= PRESORT_DEPTH)
        {
          for (int i=lo; i<=hi; i++)
            rank[posn[i]] = hi;
        }
      else if (hi-lo<PRESORT_THRESH)
        {
          int i,j;
          for (i=lo+1; i<=hi; i++)
            {
              int tmp = posn[i];
              for(j=i-1; j>=lo && GTD(posn[j], tmp, depth); j--)
                posn[j+1] = posn[j];
              posn[j+1] = tmp;
            }
          for(i=hi;i>=lo;i=j)
            {
              int tmp = posn[i];
              rank[tmp] = i;
              for (j=i-1; j>=lo && !GTD(tmp,posn[j],depth); j--)
                rank[posn[j]] = i;
            }
        }
        else
        {
          int tmp;
          unsigned char *dd=data+depth;
          unsigned char med = pivot3d(dd,lo,hi);
          // -- positions are organized as follows:
          //   [lo..l1[ [l1..l[ ]h..h1] ]h1..hi]
          //      =        <       >        =
          int l1 = lo;
          int h1 = hi;
          while (dd[posn[l1]]==med && l1<h1) { l1++; }
          while (dd[posn[h1]]==med && l1<h1) { h1--; }
          int l = l1;
          int h = h1;
          // -- partition set
          for (;;)
            {
              while (l<=h)
                {
                  int c = (int)dd[posn[l]] - (int)med;
                  if (c > 0) break;
                  if (c == 0) { tmp=posn[l]; posn[l]=posn[l1]; posn[l1++]=tmp; }
                  l++;
                }
              while (l<=h)
                {
                  int c = (int)dd[posn[h]] - (int)med;
                  if (c < 0) break;
                  if (c == 0) { tmp=posn[h]; posn[h]=posn[h1]; posn[h1--]=tmp; }
                  h--;
                }
              if (l>h) break;
              tmp=posn[l]; posn[l]=posn[h]; posn[h]=tmp;
            }
          // -- reorganize as follows
          //   [lo..l1[ [l1..h1] ]h1..hi]
          //      <        =        > 
          tmp = mini(l1-lo, l-l1);
          vswap(lo, l-tmp, tmp, posn);
          l1 = lo + (l-l1);
          tmp = mini(hi-h1, h1-h);
          vswap(hi-tmp+1, h+1, tmp, posn);
          h1 = hi - (h1-h);
          // -- process segments
          ASSERT(sp+3<QUICKSORT_STACK);
          // ----- middle segment (=?) [l1, h1]
          l = l1; h = h1;
          if (med==0) // special case for marker [slow]
            for (int i=l; i<=h; i++)
              if ((int)posn[i]+depth == size-1)
                { 
                  tmp=posn[i]; posn[i]=posn[l]; posn[l]=tmp; 
                  rank[tmp]=l++; break; 
                }
          if (l<h)
            { slo[sp] = l; shi[sp] = h; sd[sp++] = depth+1; }
          else if (l==h)
            { rank[posn[h]] = h; }
          // ----- lower segment (<) [lo, l1[
          l = lo;
          h = l1-1;
          if (l<h)
            { slo[sp] = l; shi[sp] = h; sd[sp++] = depth; }
          else if (l==h)
            { rank[posn[h]] = h; }
          // ----- upper segment (>) ]h1, hi]
          l = h1+1;
          h = hi;
          if (l<h)
            { slo[sp] = l; shi[sp] = h; sd[sp++] = depth; }
          else if (l==h)
            { rank[posn[h]] = h; }
        }
    }
}




// _BSort::radixsort8 -- 8 bit radix sort

void 
_BSort::radixsort8(void)
{
  int i;
  // Initialize frequency array
  int *lo = new int [256];
  int *hi = new int [256];
  for (i=0; i<256; i++)
    hi[i] = lo[i] = 0;
  // Count occurences
  for (i=0; i<size-1; i++)
    hi[data[i]] ++;
  // Compute positions (lo)
  int last = 1;
  for (i=0; i<256; i++)
    {
      lo[i] = last;
      hi[i] = last + hi[i] - 1;
      last = hi[i] + 1;
    }
  for (i=0; i<size-1; i++)
    {
      posn[ lo[data[i]]++ ] = i;
      rank[ i ] = hi[data[i]];
    }
  // Process marker "$"
  posn[0] = size-1;
  rank[size-1] = 0;
  // Extra element
  rank[size] = -1;
  // Free
  delete [] lo;
  delete [] hi;
}


// _BSort::radixsort16 -- 16 bit radix sort

void 
_BSort::radixsort16(void)
{
  int i;
  // Initialize frequency array
  int *ftab = new int [65536];
  for (i=0; i<65536; i++)
    ftab[i] = 0;
  // Count occurences
  unsigned char c1 = data[0];
  for (i=0; i<size-1; i++)
    {
      unsigned char c2 = data[i+1];
      ftab[(c1<<8)|c2] ++;
      c1 = c2;
    }
  // Generate upper position
  for (i=1;i<65536;i++)
    ftab[i] += ftab[i-1];
  // Fill rank array with upper bound
  c1 = data[0];
  for (i=0; i<size-2; i++)
    {
      unsigned char c2 = data[i+1];
      rank[i] = ftab[(c1<<8)|c2];
      c1 = c2;
    }
  // Fill posn array (backwards)
  c1 = data[size-2];
  for (i=size-3; i>=0; i--)
    {
      unsigned char c2 = data[i];
      posn[ ftab[(c2<<8)|c1]-- ] = i;
      c1 = c2;
    }
  // Fixup marker stuff
  ASSERT(data[size-1]==0);
  c1 = data[size-2];
  posn[0] = size-1;
  posn[ ftab[(c1<<8)] ] = size-2;
  rank[size-1] = 0;
  rank[size-2] = ftab[(c1<<8)];
  // Extra element
  rank[size] = -1;
  // Free ftab
  delete [] ftab;
}



// _BSort::run -- main sort loop

void
_BSort::run(int &markerpos)
{
  int lo, hi;
  ASSERT(size>0);
  ASSERT(data[size-1]==0);
#ifdef BSORT_TIMER
  long start = GOS::ticks();
#endif  
  // Step 1: Radix sort 
  int depth = 0;
  if (size > RADIX_THRESH)
    { 
      radixsort16();
      depth=2;
    }
  else
    { 
      radixsort8(); 
      depth=1;
    }
  // Step 2: Perform presort to depth PRESORT_DEPTH
  for (lo=0; lo<size; lo++)
    {
      hi = rank[posn[lo]];
      if (lo < hi)
        quicksort3d(lo, hi, depth);
      lo = hi;
    }
  depth = PRESORT_DEPTH;
#ifdef BSORT_TIMER
  long middle = GOS::ticks();
#endif  
  // Step 3: Perform rank doubling
  int again = 1;
  while (again)
    {
      again = 0;
      int sorted_lo = 0;
      for (lo=0; lo<size; lo++)
        {
          hi = rank[posn[lo]&0xffffff];
          if (lo == hi)
            {
              lo += (posn[lo]>>24) & 0xff;
            }
          else
            {
              if (hi-lo < RANKSORT_THRESH)
                {
                  ranksort(lo, hi, depth);
                }
              else
                {
                  again += 1;
                  while (sorted_lo < lo-1)
                    {
                      int step = mini(255, lo-1-sorted_lo);
                      posn[sorted_lo] = (posn[sorted_lo]&0xffffff) | (step<<24);
                      sorted_lo += step+1;
                    }
                  quicksort3r(lo, hi, depth);
                  sorted_lo = hi + 1;
                }
              lo = hi;
            }
        }
      // Finish threading
      while (sorted_lo < lo-1)
        {
          int step = mini(255, lo-1-sorted_lo);
          posn[sorted_lo] = (posn[sorted_lo]&0xffffff) | (step<<24);
          sorted_lo += step+1;
        }
      // Double depth
      depth += depth;
    }
  // Step 4: Permute data
  int i;
  markerpos = -1;
  for (i=0; i<size; i++)
    rank[i] = data[i];
  for (i=0; i<size; i++)
    {
      int j = posn[i] & 0xffffff;
      if (j>0) 
        { 
          data[i] = rank[j-1];
        } 
      else 
        {
          data[i] = 0;
          markerpos = i;
        }
    }
  ASSERT(markerpos>=0 && markerpos<size);
#ifdef BSORT_TIMER
  long end = GOS::ticks();
  fprintf(stderr,"Sorting time: %d bytes in %ld + %ld = %ld ms\n", 
          size-1, middle-start, end-middle, end-start);
#endif  
}


#endif // NEED_DECODER_ONLY


// ========================================
// -- Encoding

#define FREQMAX   4
#define FREQS0    100000
#define FREQS1    1000000
#define CTXIDS    3

#ifndef NEED_DECODER_ONLY
static void
encode_raw(ZPCodec &zp, int bits, int x)
{
  int n = 1;
  int m = (1<<bits);
  while (n < m)
    {
      x = (x & (m-1)) << 1;
      int b = (x >> bits);
      zp.encoder(b);
      n = (n<<1) | b;
    }
}
#endif

#ifndef NEED_DECODER_ONLY
static inline void
encode_binary(ZPCodec &zp, BitContext *ctx, int bits, int x)
{
  // Require 2^bits-1  contexts
  int n = 1;
  int m = (1<<bits);
  ctx = ctx - 1;
  while (n < m)
    {
      x = (x & (m-1)) << 1;
      int b = (x >> bits);
      zp.encoder(b, ctx[n]);
      n = (n<<1) | b;
    }
}
#endif

#ifndef NEED_DECODER_ONLY
unsigned int
BSByteStream::encode()
{ 
  /////////////////////////////////
  ////////////  Block Sort Tranform

  int markerpos = size-1;
  blocksort(data,size,markerpos);

  /////////////////////////////////
  //////////// Encode Output Stream

  // Header
  encode_raw(zp, 24, size);
  // Determine and Encode Estimation Speed
  int fshift = 0;
  if (size < FREQS0)
    { fshift=0; zp.encoder(0); }
  else if (size < FREQS1)
    { fshift = 1; zp.encoder(1); zp.encoder(0); }
  else
    { fshift = 2; zp.encoder(1); zp.encoder(1); }
  // MTF
  unsigned char mtf[256];
  unsigned char rmtf[256];
  unsigned int  freq[FREQMAX];
  int m = 0;
  for (m=0; m<256; m++)
    mtf[m] = m;
  for (m=0; m<256; m++)
    rmtf[mtf[m]] = m;
  int fadd = 4;
  for (m=0; m<FREQMAX; m++)
    freq[m] = 0;
  // Encode
  int i;
  int mtfno = 3;
  for (i=0; i<size; i++)
    {
      // Get MTF data
      int c = data[i];
      int ctxid = CTXIDS-1;
      if (ctxid>mtfno) ctxid=mtfno;
      mtfno = rmtf[c];
      if (i==markerpos)
        mtfno = 256;
      // Encode using ZPCoder
      int b;
      BitContext *cx = ctx;
      b = (mtfno==0);
      zp.encoder(b, cx[ctxid]);
      if (b) goto rotate; cx+=CTXIDS;
      b = (mtfno==1);
      zp.encoder(b, cx[ctxid]);
      if (b) goto rotate; cx+=CTXIDS;
      b = (mtfno<4);
      zp.encoder(b, cx[0]);
      if (b) { encode_binary(zp,cx+1,1,mtfno-2); goto rotate; } 
      cx+=1+1;
      b = (mtfno<8);
      zp.encoder(b, cx[0]);
      if (b) { encode_binary(zp,cx+1,2,mtfno-4); goto rotate; } 
      cx+=1+3;
      b = (mtfno<16);
      zp.encoder(b, cx[0]);
      if (b) { encode_binary(zp,cx+1,3,mtfno-8); goto rotate; } 
      cx+=1+7;
      b = (mtfno<32);
      zp.encoder(b, cx[0]);
      if (b) { encode_binary(zp,cx+1,4,mtfno-16); goto rotate; } 
      cx+=1+15;
      b = (mtfno<64);
      zp.encoder(b, cx[0]);
      if (b) { encode_binary(zp,cx+1,5,mtfno-32); goto rotate; } 
      cx+=1+31;
      b = (mtfno<128);
      zp.encoder(b, cx[0]);
      if (b) { encode_binary(zp,cx+1,6,mtfno-64); goto rotate; } 
      cx+=1+63;
      b = (mtfno<256);
      zp.encoder(b, cx[0]);
      if (b) { encode_binary(zp,cx+1,7,mtfno-128); goto rotate; } 
      continue;
      // Rotate MTF according to empirical frequencies (new!)
    rotate:
      // Adjust frequencies for overflow
      int k;
      fadd = fadd + (fadd>>fshift);
      if (fadd > 0x10000000) 
        {
          fadd = fadd>>24;
          freq[0] >>= 24;
          freq[1] >>= 24;
          freq[2] >>= 24;
          freq[3] >>= 24;
#if FREQMAX>4
          for (k=4; k<FREQMAX; k++)
            freq[k] = freq[k]>>24;
#endif
        }
      // Relocate new char according to new freq
      unsigned int fc = fadd;
      if (mtfno < FREQMAX)
        fc += freq[mtfno];
      for (k=mtfno; k>=FREQMAX; k--)
        {
          mtf[k] = mtf[k-1];
          rmtf[mtf[k]] = k;
        }
      for (; k>0 && fc>=freq[k-1]; k--)
        {
          mtf[k] = mtf[k-1];
          freq[k] = freq[k-1];
          rmtf[mtf[k]] = k;
        }
      mtf[k] = c;
      freq[k] = fc;
      rmtf[mtf[k]] = k;
    }
  // Terminate
  return 0;
}

#endif // NEED_DECODER_ONLY


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
BSByteStream::decode()
{
  /////////////////////////////////
  ////////////  Decode input stream
  
  int i;
  // Decode block size
  size = decode_raw(zp, 24);
  if (size == 0)
    return 0;
  if (size>MAXBLOCK*1024)
    G_THROW("bytestream.corrupt");        //  Corrupted decoder input
  // Allocate
  if ((int)blocksize < size)
    {
      blocksize = size;
      if (data)
        delete [] data; 
      data = 0;
    }
  if (! data) 
    data = new unsigned char[blocksize];
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
#if FREQMAX>4
          for (k=4; k<FREQMAX; k++)
            freq[k] = freq[k]>>24;
#endif
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
  unsigned int *posn = new unsigned int[blocksize];
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
  delete [] posn;
  if (i != markerpos)
    G_THROW("bytestream.corrupt");        //  Corrupted decoder input
  return size;
}



// ========================================
// --- Construction



BSByteStream::BSByteStream(ByteStream &xbs, int encoding)
  : encoding(encoding), offset(0), bptr(0), blocksize(0), 
    data(0), size(0), eof(0), bs(&xbs),
    zp(*bs, encoding?true:false, true)
{
  if (encoding)
    {
#ifdef NEED_DECODER_ONLY
      G_THROW("bytestream.decoder_only");     //  Compiled with NEED_DECODER_ONLY
#else
      if (encoding < MINBLOCK)
        encoding = MINBLOCK;
      if (encoding > MAXBLOCK)
        G_THROW("bytestream.blocksize\t" STR(MAXBLOCK));  //  "Requested block size must be less than " STR(MAXBLOCK) "Kbytes."
      // Record block size
      blocksize = encoding * 1024;
#endif
    }
  // Initialize context array
  memset(ctx, 0, sizeof(ctx));
}



BSByteStream::~BSByteStream()
{
#ifndef NEED_DECODER_ONLY
  // Flush
  if (encoding) 
    flush();
  // Encode EOF marker
  if (encoding)
    encode_raw(zp, 24, 0);
#endif
  // Free allocated memory
  if (data)
    delete [] data; 
}





// ========================================
// -- ByteStream interface



long 
BSByteStream::tell() const
{
  return offset;
}

void 
BSByteStream::flush()
{
#ifndef NEED_DECODER_ONLY
  if (encoding && bptr>0)
    {
      ASSERT(bptr<(int)blocksize);
      memset(data+bptr, 0, OVERFLOW);
      size = bptr+1;
      encode();
    }
#endif
  size = bptr = 0;
}

size_t 
BSByteStream::read(void *buffer, size_t sz)
{
  if (encoding)
    G_THROW("bytestream.cant_read");      //  Cannot read from BSByteStream created for encoding
  if (eof)
    return 0;
  // Loop
  int copied = 0;
  while (sz>0 && !eof)
    {
      // Decode if needed
      if (size == 0)
        {
          bptr = 0;
          if (! decode()) 
            size = eof = 1;
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

size_t 
BSByteStream::write(const void *buffer, size_t sz)
{
#ifdef NEED_ENCODER_ONLY
  G_THROW("bytestream.cant_write");         //  Cannot write to BSByteStream created for decoding
#else
  // Trivial checks
  if (! encoding)
    G_THROW("bytestream.cant_write");       //  Cannot write to BSByteStream created for decoding
  if (sz == 0)
    return 0;
  // Loop
  int copied = 0;
  while (sz > 0)
    {
      // Initialize
      if (!data) 
        {
          bptr = 0;
          data = new unsigned char[blocksize+OVERFLOW];
        }
      // Compute remaining
      int bytes = blocksize - 1 - bptr;
      if (bytes > (int)sz)
        bytes = sz;
      // Store date (todo: rle)
      memcpy(data+bptr, buffer, bytes);
      buffer = (void*)((char*)buffer + bytes);
      bptr += bytes;
      sz -= bytes;
      copied += bytes;
      offset += bytes;
      // Flush when needed
      if (bptr + 1 >= (int)blocksize)
        flush();
    }
  // return
  return copied;
#endif // NEED_DECODER_ONLY
}

