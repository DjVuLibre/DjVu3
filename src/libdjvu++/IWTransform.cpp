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
//C- $Id: IWTransform.cpp,v 1.9 1999-06-09 19:43:44 leonb Exp $



#include "IWTransform.h"
#include "GException.h"
#include "MMX.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define IWTRANSFORM_TIMER
#ifdef IWTRANSFORM_TIMER
#include "GOS.h"
#endif




//////////////////////////////////////////////////////
// MMX IMPLEMENTATION HELPERS
//////////////////////////////////////////////////////


// Note:
// MMX implementation for vertical transforms only.
// Speedup is basically related to faster memory transfer
// The IW44 transform is not CPU bound, it is memory bound.

#ifdef MMX

static short w9[]  = {9,9,9,9};
static short w1[]  = {1,1,1,1};
static int   d8[]  = {8,8};
static int   d16[] = {16,16};


static inline void
mmx_fv_1 ( short* &q, short* e, int s, int s3 )
{
  while (q<e && (((long)q)&0x7))
    {
      int a = (int)q[-s] + (int)q[s];
      int b = (int)q[-s3] + (int)q[s3];
      *q -= (((a<<3)+a-b+8)>>4);
      q++;
    }
  while (q+3<e)
    {
      MMXar( movq,       q-s,mm0);  // MM0=[ b3, b2, b1, b0 ]
      MMXar( movq,       q+s,mm2);  // MM2=[ c3, c2, c1, c0 ]
      MMXrr( movq,       mm0,mm1);  
      MMXrr( punpcklwd,  mm2,mm0);  // MM0=[ c1, b1, c0, b0 ]
      MMXrr( punpckhwd,  mm2,mm1);  // MM1=[ c3, b3, c2, b2 ]
      MMXar( pmaddwd,    w9,mm0);   // MM0=[ (c1+b1)*9, (c0+b0)*9 ]
      MMXar( pmaddwd,    w9,mm1);   // MM1=[ (c3+b3)*9, (c2+b2)*9 ]
      MMXar( movq,       q-s3,mm2);
      MMXar( movq,       q+s3,mm4);
      MMXrr( movq,       mm2,mm3);
      MMXrr( punpcklwd,  mm4,mm2);  // MM2=[ d1, a1, d0, a0 ]
      MMXrr( punpckhwd,  mm4,mm3);  // MM3=[ d3, a3, d2, a2 ]
      MMXar( pmaddwd,    w1,mm2);   // MM2=[ (a1+d1)*1, (a0+d0)*1 ]
      MMXar( pmaddwd,    w1,mm3);   // MM3=[ (a3+d3)*1, (a2+d2)*1 ]
      MMXar( paddd,      d8,mm0);
      MMXar( paddd,      d8,mm1);
      MMXrr( psubd,      mm2,mm0);  // MM0=[ (c1+b1)*9-a1-d1+8, ...
      MMXrr( psubd,      mm3,mm1);  // MM1=[ (c3+b3)*9-a3-d3+8, ...
      MMXir( psrad,      4,mm0);
      MMXar( movq,       q,mm7);    // MM7=[ p3,p2,p1,p0 ]
      MMXir( psrad,      4,mm1);
      MMXrr( packssdw,   mm1,mm0);  // MM0=[ x3,x2,x1,x0 ]
      MMXrr( psubw,      mm0,mm7);  // MM7=[ p3-x3, p2-x2, ... ]
      MMXra( movq,       mm7,q);
#if defined(_MSC_VER) && defined(_DEBUG)
      MMXemms;
#endif
      q += 4;
    }
}

static inline void
mmx_fv_2 ( short* &q, short* e, int s, int s3 )
{
  while (q<e && (((long)q)&0x7))
    {
      int a = (int)q[-s] + (int)q[s];
      int b = (int)q[-s3] + (int)q[s3];
      *q += (((a<<3)+a-b+16)>>5);
      q ++;
    }
  while (q+3<e)
    {
      MMXar( movq,       q-s,mm0);  // MM0=[ b3, b2, b1, b0 ]
      MMXar( movq,       q+s,mm2);  // MM2=[ c3, c2, c1, c0 ]
      MMXrr( movq,       mm0,mm1);  
      MMXrr( punpcklwd,  mm2,mm0);  // MM0=[ c1, b1, c0, b0 ]
      MMXrr( punpckhwd,  mm2,mm1);  // MM1=[ c3, b3, c2, b2 ]
      MMXar( pmaddwd,    w9,mm0);   // MM0=[ (c1+b1)*9, (c0+b0)*9 ]
      MMXar( pmaddwd,    w9,mm1);   // MM1=[ (c3+b3)*9, (c2+b2)*9 ]
      MMXar( movq,       q-s3,mm2);
      MMXar( movq,       q+s3,mm4);
      MMXrr( movq,       mm2,mm3);
      MMXrr( punpcklwd,  mm4,mm2);  // MM2=[ d1, a1, d0, a0 ]
      MMXrr( punpckhwd,  mm4,mm3);  // MM3=[ d3, a3, d2, a2 ]
      MMXar( pmaddwd,    w1,mm2);   // MM2=[ (a1+d1)*1, (a0+d0)*1 ]
      MMXar( pmaddwd,    w1,mm3);   // MM3=[ (a3+d3)*1, (a2+d2)*1 ]
      MMXar( paddd,      d16,mm0);
      MMXar( paddd,      d16,mm1);
      MMXrr( psubd,      mm2,mm0);  // MM0=[ (c1+b1)*9-a1-d1+8, ...
      MMXrr( psubd,      mm3,mm1);  // MM1=[ (c3+b3)*9-a3-d3+8, ...
      MMXir( psrad,      5,mm0);
      MMXar( movq,       q,mm7);    // MM7=[ p3,p2,p1,p0 ]
      MMXir( psrad,      5,mm1);
      MMXrr( packssdw,   mm1,mm0);  // MM0=[ x3,x2,x1,x0 ]
      MMXrr( paddw,      mm0,mm7);  // MM7=[ p3+x3, p2+x2, ... ]
      MMXra( movq,       mm7,q);
#if defined(_MSC_VER) && defined(_DEBUG)
      MMXemms;
#endif
      q += 4;
    }
}


inline void
mmx_bv_1 ( short* &q, short* e, int s, int s3 )
{
  while (q<e && (((long)q)&0x7))
    {
      int a = (int)q[-s] + (int)q[s];
      int b = (int)q[-s3] + (int)q[s3];
      *q -= (((a<<3)+a-b+16)>>5);
      q ++;
  }
  while (q+3 < e)
    {
      MMXar( movq,       q-s,mm0);  // MM0=[ b3, b2, b1, b0 ]
      MMXar( movq,       q+s,mm2);  // MM2=[ c3, c2, c1, c0 ]
      MMXrr( movq,       mm0,mm1);  
      MMXrr( punpcklwd,  mm2,mm0);  // MM0=[ c1, b1, c0, b0 ]
      MMXrr( punpckhwd,  mm2,mm1);  // MM1=[ c3, b3, c2, b2 ]
      MMXar( pmaddwd,    w9,mm0);   // MM0=[ (c1+b1)*9, (c0+b0)*9 ]
      MMXar( pmaddwd,    w9,mm1);   // MM1=[ (c3+b3)*9, (c2+b2)*9 ]
      MMXar( movq,       q-s3,mm2);
      MMXar( movq,       q+s3,mm4);
      MMXrr( movq,       mm2,mm3);
      MMXrr( punpcklwd,  mm4,mm2);  // MM2=[ d1, a1, d0, a0 ]
      MMXrr( punpckhwd,  mm4,mm3);  // MM3=[ d3, a3, d2, a2 ]
      MMXar( pmaddwd,    w1,mm2);   // MM2=[ (a1+d1)*1, (a0+d0)*1 ]
      MMXar( pmaddwd,    w1,mm3);   // MM3=[ (a3+d3)*1, (a2+d2)*1 ]
      MMXar( paddd,      d16,mm0);
      MMXar( paddd,      d16,mm1);
      MMXrr( psubd,      mm2,mm0);  // MM0=[ (c1+b1)*9-a1-d1+8, ...
      MMXrr( psubd,      mm3,mm1);  // MM1=[ (c3+b3)*9-a3-d3+8, ...
      MMXir( psrad,      5,mm0);
      MMXar( movq,       q,mm7);    // MM7=[ p3,p2,p1,p0 ]
      MMXir( psrad,      5,mm1);
      MMXrr( packssdw,   mm1,mm0);  // MM0=[ x3,x2,x1,x0 ]
      MMXrr( psubw,      mm0,mm7);  // MM7=[ p3-x3, p2-x2, ... ]
      MMXra( movq,       mm7,q);
#if defined(_MSC_VER) && defined(_DEBUG)
      MMXemms;
#endif
      q += 4;
    }
}


static inline void
mmx_bv_2 ( short* &q, short* e, int s, int s3 )
{
  while (q<e && (((long)q)&0x7))
    {
      int a = (int)q[-s] + (int)q[s];
      int b = (int)q[-s3] + (int)q[s3];
      *q += (((a<<3)+a-b+8)>>4);
      q ++;
    }
  while (q+3 < e)
    {
      MMXar( movq,       q-s,mm0);  // MM0=[ b3, b2, b1, b0 ]
      MMXar( movq,       q+s,mm2);  // MM2=[ c3, c2, c1, c0 ]
      MMXrr( movq,       mm0,mm1);  
      MMXrr( punpcklwd,  mm2,mm0);  // MM0=[ c1, b1, c0, b0 ]
      MMXrr( punpckhwd,  mm2,mm1);  // MM1=[ c3, b3, c2, b2 ]
      MMXar( pmaddwd,    w9,mm0);   // MM0=[ (c1+b1)*9, (c0+b0)*9 ]
      MMXar( pmaddwd,    w9,mm1);   // MM1=[ (c3+b3)*9, (c2+b2)*9 ]
      MMXar( movq,       q-s3,mm2);
      MMXar( movq,       q+s3,mm4);
      MMXrr( movq,       mm2,mm3);
      MMXrr( punpcklwd,  mm4,mm2);  // MM2=[ d1, a1, d0, a0 ]
      MMXrr( punpckhwd,  mm4,mm3);  // MM3=[ d3, a3, d2, a2 ]
      MMXar( pmaddwd,    w1,mm2);   // MM2=[ (a1+d1)*1, (a0+d0)*1 ]
      MMXar( pmaddwd,    w1,mm3);   // MM3=[ (a3+d3)*1, (a2+d2)*1 ]
      MMXar( paddd,      d8,mm0);
      MMXar( paddd,      d8,mm1);
      MMXrr( psubd,      mm2,mm0);  // MM0=[ (c1+b1)*9-a1-d1+8, ...
      MMXrr( psubd,      mm3,mm1);  // MM1=[ (c3+b3)*9-a3-d3+8, ...
      MMXir( psrad,      4,mm0);
      MMXar( movq,       q,mm7);    // MM7=[ p3,p2,p1,p0 ]
      MMXir( psrad,      4,mm1);
      MMXrr( packssdw,   mm1,mm0);  // MM0=[ x3,x2,x1,x0 ]
      MMXrr( paddw,      mm0,mm7);  // MM7=[ p3+x3, p2+x2, ... ]
      MMXra( movq,       mm7,q);
#if defined(_MSC_VER) && defined(_DEBUG)
      MMXemms;
#endif
      q += 4;
    }
}

#endif

//////////////////////////////////////////////////////
// NEW FILTERS
//////////////////////////////////////////////////////

static void
filter_begin(int w, int h)
{
  if (MMXControl::mmxflag < 0)  
    MMXControl::enable_mmx();
}


static void
filter_end()
{
#ifdef MMX
  if (MMXControl::mmxflag > 0)
    MMXemms;
#endif
}


static void 
filter_fv(short *p, int w, int h, int rowsize, int scale)
{
  int y = 0;
  int s = scale*rowsize;
  int s3 = s+s+s;
  h = ((h-1)/scale)+1;
  y += 1;
  p += s;
  while (y-3 < h)
    {
      // 1-Delta
      {
        short *q = p;
        short *e = q+w;
        if (y>=3 && y+3<h)
          {
            // Generic case
#ifdef MMX
            if (scale==1 && MMXControl::mmxflag>0)
              mmx_fv_1(q, e, s, s3);
#endif
            while (q<e)
              {
                int a = (int)q[-s] + (int)q[s];
                int b = (int)q[-s3] + (int)q[s3];
                *q -= (((a<<3)+a-b+8)>>4);
                q += scale;
              }
          }
        else if (y<h)
          {
            // Special cases
            short *q1 = (y+1<h ? q+s : q-s);
            while (q<e)
              {
                int a = (int)q[-s] + (int)(*q1);
                *q -= ((a+1)>>1);
                q += scale;
                q1 += scale;
              }
          }
      }
      // 2-Update
      {
        short *q = p-s3;
        short *e = q+w;
        if (y>=6 && y<h)
          {
            // Generic case
#ifdef MMX
            if (scale==1 && MMXControl::mmxflag>0)
              mmx_fv_2(q, e, s, s3);
#endif
            while (q<e)
              {
                int a = (int)q[-s] + (int)q[s];
                int b = (int)q[-s3] + (int)q[s3];
                *q += (((a<<3)+a-b+16)>>5);
                q += scale;
              }
          }
        else if (y>=3)
          {
            // Special cases
            short *q1 = (y-2<h ? q+s : 0);
            short *q3 = (y<h ? q+s3 : 0);
            if (y>=6)
              {
                while (q<e)
                  {
                    int a = (int)q[-s] + (q1 ? (int)(*q1) : 0);
                    int b = (int)q[-s3] + (q3 ? (int)(*q3) : 0);
                    *q += (((a<<3)+a-b+16)>>5);
                    q += scale;
                    if (q1) q1 += scale;
                    if (q3) q3 += scale;
                  }
              }
            else if (y>=4)
              {
                while (q<e)
                  {
                    int a = (int)q[-s] + (q1 ? (int)(*q1) : 0);
                    int b = (q3 ? (int)(*q3) : 0);
                    *q += (((a<<3)+a-b+16)>>5);
                    q += scale;
                    if (q1) q1 += scale;
                    if (q3) q3 += scale;
                  }
              }
            else
              {
                while (q<e)
                  {
                    int a = (q1 ? (int)(*q1) : 0);
                    int b = (q3 ? (int)(*q3) : 0);
                    *q += (((a<<3)+a-b+16)>>5);
                    q += scale;
                    if (q1) q1 += scale;
                    if (q3) q3 += scale;
                  }
              }
          }
      }
      y += 2;
      p += s+s;
    }
}


static void 
filter_bv(short *p, int w, int h, int rowsize, int scale)
{
  int y = 0;
  int s = scale*rowsize;
  int s3 = s+s+s;
  h = ((h-1)/scale)+1;
  while (y-3 < h)
    {
      // 1-Lifting
      {
        short *q = p;
        short *e = q+w;
        if (y>=3 && y+3<h)
          {
            // Generic case
#ifdef MMX
            if (scale==1 && MMXControl::mmxflag>0)
              mmx_bv_1(q, e, s, s3);
#endif
            while (q<e)
              {
                int a = (int)q[-s] + (int)q[s];
                int b = (int)q[-s3] + (int)q[s3];
                *q -= (((a<<3)+a-b+16)>>5);
                q += scale;
              }
          }
        else if (y<h)
          {
            // Special cases
            short *q1 = (y+1<h ? q+s : 0);
            short *q3 = (y+3<h ? q+s3 : 0);
            if (y>=3)
              {
                while (q<e)
                  {
                    int a = (int)q[-s] + (q1 ? (int)(*q1) : 0);
                    int b = (int)q[-s3] + (q3 ? (int)(*q3) : 0);
                    *q -= (((a<<3)+a-b+16)>>5);
                    q += scale;
                    if (q1) q1 += scale;
                    if (q3) q3 += scale;
                  }
              }
            else if (y>=1)
              {
                while (q<e)
                  {
                    int a = (int)q[-s] + (q1 ? (int)(*q1) : 0);
                    int b = (q3 ? (int)(*q3) : 0);
                    *q -= (((a<<3)+a-b+16)>>5);
                    q += scale;
                    if (q1) q1 += scale;
                    if (q3) q3 += scale;
                  }
              }
            else
              {
                while (q<e)
                  {
                    int a = (q1 ? (int)(*q1) : 0);
                    int b = (q3 ? (int)(*q3) : 0);
                    *q -= (((a<<3)+a-b+16)>>5);
                    q += scale;
                    if (q1) q1 += scale;
                    if (q3) q3 += scale;
                  }
              }
          }
      }
      // 2-Interpolation
      {
        short *q = p-s3;
        short *e = q+w;
        if (y>=6 && y<h)
          {
            // Generic case
#ifdef MMX
            if (scale==1 && MMXControl::mmxflag>0)
              mmx_bv_2(q, e, s, s3);
#endif
            while (q<e)
              {
                int a = (int)q[-s] + (int)q[s];
                int b = (int)q[-s3] + (int)q[s3];
                *q += (((a<<3)+a-b+8)>>4);
                q += scale;
              }
          }
        else if (y>=3)
          {
            // Special cases
            short *q1 = (y-2<h ? q+s : q-s);
            while (q<e)
              {
                int a = (int)q[-s] + (int)(*q1);
                *q += ((a+1)>>1);
                q += scale;
                q1 += scale;
              }
          }
      }
      y += 2;
      p += s+s;
    }
}


static void 
filter_fh(short *p, int w, int h, int rowsize, int scale)
{
  int y = 0;
  int s = scale;
  int s3 = s+s+s;
  rowsize *= scale;
  while (y<h)
    {
      short *q = p+s;
      short *e = p+w;
      int a0=0, a1=0, a2=0, a3=0;
      int b0=0, b1=0, b2=0, b3=0;
      if (q < e)
        {
          // Special case: x=1
          a1 = a2 = q[-s];
          if (q+s<e)
            a2 = q[s];
          if (q+s3<e)
            a3 = q[s3];
          b3 = q[0] - ((a1+a2+1)>>1);
          q[0] = b3;
          q += s+s;
        }
      while (q+s3 < e)
        {
          // Generic case
          a0=a1; 
          a1=a2; 
          a2=a3;
          a3=q[s3];
          b0=b1; 
          b1=b2; 
          b2=b3;
          b3 = q[0] - ((((a1+a2)<<3)+(a1+a2)-a0-a3+8) >> 4);
          q[0] = b3;
          q[-s3] = q[-s3] + ((((b1+b2)<<3)+(b1+b2)-b0-b3+16) >> 5);
          q += s+s;
        }
      while (q < e)
        {
          // Special case: w-3 <= x < w
          a1=a2; 
          a2=a3;
          b0=b1; 
          b1=b2; 
          b2=b3;
          b3 = q[0] - ((a1+a2+1)>>1);
          q[0] = b3;
          q[-s3] = q[-s3] + ((((b1+b2)<<3)+(b1+b2)-b0-b3+16) >> 5);
          q += s+s;
        }
      while (q-s3 < e)
        {
          // Special case  w <= x < w+3
          b0=b1; 
          b1=b2; 
          b2=b3;
          b3=0;
          q[-s3] = q[-s3] + ((((b1+b2)<<3)+(b1+b2)-b0-b3+16) >> 5);
          q += s+s;
        }
      y += scale;
      p += rowsize;
    }
}


static void 
filter_bh(short *p, int w, int h, int rowsize, int scale)
{
  int y = 0;
  int s = scale;
  int s3 = s+s+s;
  rowsize *= scale;
  while (y<h)
    {
      short *q = p;
      short *e = p+w;
      int a0=0, a1=0, a2=0, a3=0;
      int b0=0, b1=0, b2=0, b3=0;
      if (q<e)
        {
          // Special case:  x=0
          if (q+s < e)
            a2 = q[s];
          if (q+s3 < e)
            a3 = q[s3];
          b2 = b3 = q[0] - ((((a1+a2)<<3)+(a1+a2)-a0-a3+16) >> 5);
          q[0] = b3;
          q += s+s;
        }
      if (q<e)
        {
          // Special case:  x=2
          a0 = a1;
          a1 = a2;
          a2 = a3;
          if (q+s3 < e)
            a3 = q[s3];
          b3 = q[0] - ((((a1+a2)<<3)+(a1+a2)-a0-a3+16) >> 5);
          q[0] = b3;
          q += s+s;
        }
      if (q<e)
        {
          // Special case:  x=4
          b1 = b2;
          b2 = b3;
          a0 = a1;
          a1 = a2;
          a2 = a3;
          if (q+s3 < e)
            a3 = q[s3];
          b3 = q[0] - ((((a1+a2)<<3)+(a1+a2)-a0-a3+16) >> 5);
          q[0] = b3;
          q[-s3] = q[-s3] + ((b1+b2+1)>>1);
          q += s+s;
        }
      while (q+s3 < e)
        {
          // Generic case
          a0=a1; 
          a1=a2; 
          a2=a3;
          a3=q[s3];
          b0=b1; 
          b1=b2; 
          b2=b3;
          b3 = q[0] - ((((a1+a2)<<3)+(a1+a2)-a0-a3+16) >> 5);
          q[0] = b3;
          q[-s3] = q[-s3] + ((((b1+b2)<<3)+(b1+b2)-b0-b3+8) >> 4);
          q += s+s;
        }
      while (q < e)
        {
          // Special case:  w-3 <= x < w
          a0=a1;
          a1=a2; 
          a2=a3;
          a3=0;
          b0=b1; 
          b1=b2; 
          b2=b3;
          b3 = q[0] - ((((a1+a2)<<3)+(a1+a2)-a0-a3+16) >> 5);
          q[0] = b3;
          q[-s3] = q[-s3] + ((((b1+b2)<<3)+(b1+b2)-b0-b3+8) >> 4);
          q += s+s;
        }
      while (q-s3 < e)
        {
          // Special case  w <= x < w+3
          b0=b1; 
          b1=b2; 
          b2=b3;
          q[-s3] = q[-s3] + ((b1+b2+1)>>1);
          q += s+s;
        }
      y += scale;
      p += rowsize;
    }
}




//////////////////////////////////////////////////////
// WAVELET TRANSFORM 
//////////////////////////////////////////////////////


//----------------------------------------------------
// Function for applying bidimensional IW44 between 
// scale intervals begin(inclusive) and end(exclusive)


void
IWTransform::forward(short *p, int w, int h, int rowsize, int begin, int end)
{ 
  // PREPARATION
  filter_begin(w,h);
  // LOOP ON SCALES
  for (int scale=begin; scale<end; scale<<=1)
    {
#ifdef IWTRANSFORM_TIMER
      int tv,th;
      th = tv = GOS::ticks();
#endif
      filter_fh(p, w, h, rowsize, scale);
#ifdef IWTRANSFORM_TIMER
      th = GOS::ticks();
      tv = th - tv;
#endif
      filter_fv(p, w, h, rowsize, scale);
#ifdef IWTRANSFORM_TIMER
      th = GOS::ticks()-th;
      fprintf(stderr,"forw%d\tv=%dms h=%dms\n", scale,th,tv);
#endif
      // Progress
      DJVU_PROGRESS("decomp",scale);
    }
  // TERMINATE
  filter_end();
}

void
IWTransform::backward(short *p, int w, int h, int rowsize, int begin, int end)
{ 
  // PREPARATION
  filter_begin(w,h);
  // LOOP ON SCALES
  for (int scale=begin>>1; scale>=end; scale>>=1)
    {
#ifdef IWTRANSFORM_TIMER
      int tv,th;
      th = tv = GOS::ticks();
#endif
      filter_bv(p, w, h, rowsize, scale);
#ifdef IWTRANSFORM_TIMER
      th = GOS::ticks();
      tv = th - tv;
#endif
      filter_bh(p, w, h, rowsize, scale);
#ifdef IWTRANSFORM_TIMER
      th = GOS::ticks()-th;
      fprintf(stderr,"back%d\tv=%dms h=%dms\n", scale,tv,th);
#endif
    }
  // TERMINATE
  filter_end();
}
  



//////////////////////////////////////////////////////
// COLOR TRANSFORM 
//////////////////////////////////////////////////////


/* Utilities */

static inline int
min (int x, int y)
{
  return (x <= y) ? x : y;
}

static inline int
max (int x, int y)
{
  return (y <= x) ? x : y;
}

static const float 
rgb_to_ycc[3][3] = 
{ { 0.304348F,  0.608696F,  0.086956F },      
  { 0.463768F, -0.405797F, -0.057971F },
  {-0.173913F, -0.347826F,  0.521739F } };



/* Converts YCbCr to RGB. */
void 
IWTransform::YCbCr_to_RGB(GPixel *p, int w, int h, int rowsize)
{
  for (int i=0; i<h; i++,p+=rowsize)
    {
      GPixel *q = p;
      for (int j=0; j<w; j++,q++)
        {
          signed char y = ((signed char*)q)[0];
          signed char b = ((signed char*)q)[1];
          signed char r = ((signed char*)q)[2];
          // This is the Pigeon transform
          int t1 = b >> 2 ; 
          int t2 = r + (r >> 1);
          int t3 = y + 128 - t1;
          int tr = y + 128 + t2;
          int tg = t3 - (t2 >> 1);
          int tb = t3 + (b << 1);
          q->r = max(0,min(255,tr));
          q->g = max(0,min(255,tg));
          q->b = max(0,min(255,tb));
        }
    }
}


/* Extracts Y */
void 
IWTransform::RGB_to_Y(const GPixel *p, int w, int h, int rowsize, 
                      signed char *out, int outrowsize)
{
  int rmul[256], gmul[256], bmul[256];
  for (int k=0; k<256; k++)
    {
      rmul[k] = (int)(k*0x10000*rgb_to_ycc[0][0]);
      gmul[k] = (int)(k*0x10000*rgb_to_ycc[0][1]);
      bmul[k] = (int)(k*0x10000*rgb_to_ycc[0][2]);
    }
  for (int i=0; i<h; i++, p+=rowsize, out+=outrowsize)
    {
      const GPixel *p2 = p;
      signed char *out2 = out;
      for (int j=0; j<w; j++,p2++,out2++)
        {
          int y = rmul[p2->r] + gmul[p2->g] + bmul[p2->b] + 32768;
          *out2 = (y>>16) - 128;
        }
    }
}


/* Extracts Cb */
void 
IWTransform::RGB_to_Cb(const GPixel *p, int w, int h, int rowsize, 
                       signed char *out, int outrowsize)
{
  int rmul[256], gmul[256], bmul[256];
  for (int k=0; k<256; k++)
    {
      rmul[k] = (int)(k*0x10000*rgb_to_ycc[2][0]);
      gmul[k] = (int)(k*0x10000*rgb_to_ycc[2][1]);
      bmul[k] = (int)(k*0x10000*rgb_to_ycc[2][2]);
    }
  for (int i=0; i<h; i++, p+=rowsize, out+=outrowsize)
    {
      const GPixel *p2 = p;
      signed char *out2 = out;
      for (int j=0; j<w; j++,p2++,out2++)
        {
          int c = rmul[p2->r] + gmul[p2->g] + bmul[p2->b] + 32768;
          *out2 = max(-128, min(127, c>>16));
        }
    }
}


/* Extracts Cr */
void 
IWTransform::RGB_to_Cr(const GPixel *p, int w, int h, int rowsize, 
                       signed char *out, int outrowsize)
{
  int rmul[256], gmul[256], bmul[256];
  for (int k=0; k<256; k++)
    {
      rmul[k] = (int)((k*0x10000)*rgb_to_ycc[1][0]);
      gmul[k] = (int)((k*0x10000)*rgb_to_ycc[1][1]);
      bmul[k] = (int)((k*0x10000)*rgb_to_ycc[1][2]);
    }
  for (int i=0; i<h; i++, p+=rowsize, out+=outrowsize)
    {
      const GPixel *p2 = p;
      signed char *out2 = out;
      for (int j=0; j<w; j++,p2++,out2++)
        {
          int c = rmul[p2->r] + gmul[p2->g] + bmul[p2->b] + 32768;
          *out2 = max(-128, min(127, c>>16));
        }
    }
}


