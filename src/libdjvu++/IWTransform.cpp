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
//C- $Id: IWTransform.cpp,v 1.4 1999-05-24 21:58:15 leonb Exp $



#include "IWTransform.h"
#include "GException.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef IWTRANSFORM_TIMER
#include "GOS.h"
#endif



//////////////////////////////////////////////////////
// MMX SUPPORT MACROS
//////////////////////////////////////////////////////



// ----------------------------------------
// GCC MMX MACROS

#if defined(__GNUC__) && defined(__i386__)
#define MMXemms \
  __asm__ volatile("emms") 
#define MMXrr(op,src,dst) \
  __asm__ volatile( #op ## " %%" ## #src ## ",%%" ## #dst : : : "memory") 
#define MMXir(op,imm,dst) \
  __asm__ volatile( #op ## " %0,%%" ## #dst : : "i" (imm) : "memory") 
#define MMXar(op,addr,dst) \
  __asm__ volatile( #op ## " %0,%%" ## #dst : : "rm" (*(int*)(addr)) : "memory") 
#define MMXra(op,src,addr) \
  __asm__ volatile( #op ## " %%" #src ## ",%0" : : "rm" (*(int*)(addr)) : "memory") 
#define MMX 1
#endif

// ----------------------------------------
// MSVC MMX MACROS

#if defined(_MSC_VER) && defined(_M_IX86)
// Compiler option /GM is required
#pragma warning( 4799 : disable )
#define MMXemms \
  __asm { emms }
#define MMXrr(op,src,dst) \
  __asm { op dst,src }
#define MMXir(op,imm,dst) \
  __asm { op dst,imm }
#define MMXar(op,addr,dst) \
  { register __int64 var=*(__int64*)(addr); __asm { op dst,var } }
#define MMXra(op,src,addr) \
  { register __int64 var; __asm { op [var],src };  *(__int64*)addr = var; } 
// Probably not as efficient as GCC macros
// Not thoroughly tested.
#define MMX 1
#endif


// ----------------------------------------
// PRINTING MMX REGISTERS (Debug)


#if defined(MMX)
static void
mmx_show()
{
  int mmregs[16];
  MMXra( movq,  mm0, &mmregs[0]);
  MMXra( movq,  mm1, &mmregs[2]);
  MMXra( movq,  mm2, &mmregs[4]);
  MMXra( movq,  mm3, &mmregs[6]);
  MMXra( movq,  mm4, &mmregs[8]);
  MMXra( movq,  mm5, &mmregs[10]);
  MMXra( movq,  mm6, &mmregs[12]);
  MMXra( movq,  mm7, &mmregs[14]);
  MMXemms;
  for (int i=0; i<8; i++)
    printf("mm%d: %08x%08x\n", i, 
           mmregs[i+i+1], mmregs[i+i]);
  MMXar( movq,  &mmregs[0], mm0);
  MMXar( movq,  &mmregs[2], mm1);
  MMXar( movq,  &mmregs[4], mm2);
  MMXar( movq,  &mmregs[6], mm3);
  MMXar( movq,  &mmregs[8], mm4);
  MMXar( movq,  &mmregs[10], mm5);
  MMXar( movq,  &mmregs[12], mm6);
  MMXar( movq,  &mmregs[14], mm7);
}
#endif



//////////////////////////////////////////////////////
// MMX ENABLE/DISABLE
//////////////////////////////////////////////////////

// Default settings autodetect MMX.
// Use macro DISABLE_MMX to disable MMX by default.

#if defined(MMX) && !defined(DISABLE_MMX)
static int mmxflag = -1;
#else
static int mmxflag = 0;
#endif

int 
IWTransform::disable_mmx()
{
  mmxflag = 0;
  return mmxflag;
}

int 
IWTransform::enable_mmx()
{
  int cpuflags = 0;
#if defined(MMX) && defined(__GNUC__) && defined(__i386__)
  // Detection of MMX for GCC
  __asm__ volatile ("pushfl\n\t"    
                    "popl %%ecx\n\t"
                    "xorl %%edx,%%edx\n\t"
                    // Check that CPUID exists
                    "movl %%ecx,%%eax\n\t"
                    "xorl $0x200000,%%eax\n\t"
                    "pushl %%eax\n\t"
                    "popfl\n\t"
                    "pushfl\n\t"
                    "popl %%eax\n\t"
                    "xorl %%ecx,%%eax\n\t"
                    "jz 1f\n\t"
                    "pushl %%ecx\n\t"
                    "popfl\n\t"
                    // Check that CR0:EM is clear
                    "smsw %%ax\n\t"
                    "andl $4,%%eax\n\t"
                    "jnz 1f\n\t"
                    // Execute CPUID
                    "movl $1,%%eax\n\t"
                    "cpuid\n"
                    "1:\tmovl %%edx, %0"
                    : "=m" (cpuflags) :
                    : "eax","ebx","ecx","edx");
#endif
  
#if defined(MMX) && defined(_MSC_VER) && defined(_M_IX86)
  // Detection of MMX for MSVC
  __asm {  pushfd
           pop     ecx
           xor     edx,edx
           mov     eax,ecx         ; check that CPUID exists
           xor     eax,0x200000
           push    eax
           popfd
           pushfd
           pop     eax
           xor     eax,ecx
           jz      fini
           push    ecx
           popfd
           smsw    ax              ; check that CR0:EM is zero
           and     eax,4
           jnz     fini
           mov     eax,1           ; execute CPUID
           cpuid
         fini:
           mov     cpuflags,edx 
         }
#endif
  mmxflag = !!(cpuflags & 0x800000);
  return mmxflag;
}



//////////////////////////////////////////////////////
// MMX IMPLEMENTATION HELPERS
//////////////////////////////////////////////////////


// Note:
// MMX implementation for vertical transforms only.
// Speedup is basically related to faster memory transfer
// The IW44 transform is not CPU bound, it is memory bound.


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


//////////////////////////////////////////////////////
// NEW FILTERS
//////////////////////////////////////////////////////


static short *zeroes = 0;

static void
filter_begin(int w, int h)
{
  if (mmxflag < 0)  
    IWTransform::enable_mmx();
  if (zeroes)
    delete [] zeroes;
  zeroes = new short[w];
  memset(zeroes, 0, w*sizeof(short));
}


static void
filter_end()
{
#ifdef MMX
  if (mmxflag>0)
    MMXemms;
#endif
  if (zeroes)
    delete [] zeroes;
  zeroes = 0;
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
            if (scale==1 && mmxflag>0)
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
            if (scale==1 && mmxflag>0)
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
            short *q1 = (y-2<h ? q+s : zeroes);
            short *q3 = (y<h ? q+s3 : zeroes);
            if (y>=6)
              {
                while (q<e)
                  {
                    int a = (int)q[-s] + (int)(*q1);
                    int b = (int)q[-s3] + (int)(*q3);
                    *q += (((a<<3)+a-b+16)>>5);
                    q += scale;
                    q1 += scale;
                    q3 += scale;
                  }
              }
            else if (y>=4)
              {
                while (q<e)
                  {
                    int a = (int)q[-s] + (int)(*q1);
                    int b = (int)(*q3);
                    *q += (((a<<3)+a-b+16)>>5);
                    q += scale;
                    q1 += scale;
                    q3 += scale;
                  }
              }
            else
              {
                while (q<e)
                  {
                    int a = (int)(*q1);
                    int b = (int)(*q3);
                    *q += (((a<<3)+a-b+16)>>5);
                    q += scale;
                    q1 += scale;
                    q3 += scale;
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
            if (scale==1 && mmxflag>0)
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
            short *q1 = (y+1<h ? q+s : zeroes);
            short *q3 = (y+3<h ? q+s3 : zeroes);
            if (y>=3)
              {
                while (q<e)
                  {
                    int a = (int)q[-s] + (int)(*q1);
                    int b = (int)q[-s3] + (int)(*q3);
                    *q -= (((a<<3)+a-b+16)>>5);
                    q += scale;
                    q1 += scale;
                    q3 += scale;
                  }
              }
            else if (y>=1)
              {
                while (q<e)
                  {
                    int a = (int)q[-s] + (int)(*q1);
                    int b = (int)(*q3);
                    *q -= (((a<<3)+a-b+16)>>5);
                    q += scale;
                    q1 += scale;
                    q3 += scale;
                  }
              }
            else
              {
                while (q<e)
                  {
                    int a = (int)(*q1);
                    int b = (int)(*q3);
                    *q -= (((a<<3)+a-b+16)>>5);
                    q += scale;
                    q1 += scale;
                    q3 += scale;
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
            if (scale==1 && mmxflag>0)
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
// TRANSFORM ENTRY POINTS
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
  



