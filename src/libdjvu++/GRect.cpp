//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1997-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: GRect.cpp,v 1.21 2001-03-29 18:50:06 praveen Exp $
// $Name:  $

// -- Implementation of class GRect and GRectMapper
// - Author: Leon Bottou, 05/1997

#ifdef __GNUC__
#pragma implementation
#endif

#include "GRect.h"
#include "GException.h"


// -- Local utilities

static inline int 
imin(int x, int y)
{
  if (x < y) 
    return x;
  else
    return y;
}

static inline int 
imax(int x, int y)
{
  if (x > y) 
    return x;
  else
    return y;
}

static inline void
iswap(int &x, int &y)
{
  int tmp = x; x = y; y = tmp;
}

// -- Class GRect

int 
operator==(const GRect & r1, const GRect & r2)
{
  int isempty1 = r1.isempty();
  int isempty2 = r2.isempty();
  if (isempty1 || isempty2)
    if (isempty1 && isempty2)
      return 1;
  if ( r1.xmin==r2.xmin && r1.xmax==r2.xmax 
       && r1.ymin==r2.ymin && r1.ymax==r2.ymax )
    return 1;
  return 0;
}

int 
GRect::inflate(int dx, int dy)
{
  xmin -= dx;
  xmax += dx;
  ymin -= dy;
  ymax += dy;
  if (! isempty()) 
    return 1;
  xmin = ymin = xmax = ymax = 0;
  return 0;
}

int 
GRect::translate(int dx, int dy)
{
  xmin += dx;
  xmax += dx;
  ymin += dy;
  ymax += dy;
  if (! isempty()) 
    return 1;
  xmin = ymin = xmax = ymax = 0;
  return 0;
}

int  
GRect::intersect(const GRect &rect1, const GRect &rect2)
{
  xmin = imax(rect1.xmin, rect2.xmin);
  xmax = imin(rect1.xmax, rect2.xmax);
  ymin = imax(rect1.ymin, rect2.ymin);
  ymax = imin(rect1.ymax, rect2.ymax);
  if (! isempty()) 
    return 1;
  xmin = ymin = xmax = ymax = 0;
  return 0;
}

int  
GRect::recthull(const GRect &rect1, const GRect &rect2)
{
  if (rect1.isempty())
    {
      xmin = rect2.xmin;
      xmax = rect2.xmax;
      ymin = rect2.ymin;
      ymax = rect2.ymax;
      return !isempty();
    }
  if (rect2.isempty())
    {
      xmin = rect1.xmin;
      xmax = rect1.xmax;
      ymin = rect1.ymin;
      ymax = rect1.ymax;
      return !isempty();
    }
  xmin = imin(rect1.xmin, rect2.xmin);
  xmax = imax(rect1.xmax, rect2.xmax);
  ymin = imin(rect1.ymin, rect2.ymin);
  ymax = imax(rect1.ymax, rect2.ymax);
  return 1;
}

int
GRect::contains(const GRect & rect) const
{
   GRect tmp_rect;
   tmp_rect.intersect(*this, rect);
   return tmp_rect==rect;
}

void
GRect::scale(float factor)
{
	xmin = (int)(((float)xmin) * factor);
	ymin = (int)(((float)ymin) * factor);
	xmax = (int)(((float)xmax) * factor);
	ymax = (int)(((float)ymax) * factor);
}

void
GRect::scale(float xfactor, float yfactor)
{
	xmin = (int)(((float)xmin) * xfactor);
	ymin = (int)(((float)ymin) * yfactor);
	xmax = (int)(((float)xmax) * xfactor);
	ymax = (int)(((float)ymax) * yfactor);
}
// -- Class GRatio


inline
GRectMapper::GRatio::GRatio()
  : p(0), q(1)
{
}

inline
GRectMapper::GRatio::GRatio(int p, int q)
  : p(p), q(q)
{
  if (q == 0) 
    G_THROW("GRect.div_zero");
  if (p == 0)
    q = 1;
  if (q < 0)
    {
      p = -p; 
      q = -q; 
    }
  int gcd = 1;
  int g1 = p; 
  int g2 = q; 
  if (g1 > g2)
    {
      gcd = g1;
      g1 = g2;
      g2 = gcd;
    }
  while (g1 > 0)
    {
      gcd = g1;
      g1 = g2 % g1;
      g2 = gcd;
    }
  p /= gcd;
  q /= gcd;
}



// -- Class GRectMapper

#define MIRRORX  1
#define MIRRORY  2
#define SWAPXY 4


GRectMapper::GRectMapper()
: rectFrom(0,0,1,1), 
  rectTo(0,0,1,1),
  code(0)
{

}

void
GRectMapper::clear()
{
  rectFrom = GRect(0,0,1,1);
  rectTo = GRect(0,0,1,1);
  code = 0;
}

void 
GRectMapper::set_input(const GRect &rect)
{
  if (rect.isempty())
    G_THROW("GRect.empty_rect1");
  rectFrom = rect;
  if (code & SWAPXY)
  {
    iswap(rectFrom.xmin, rectFrom.ymin);
    iswap(rectFrom.xmax, rectFrom.ymax);
  }
  rw = rh = GRatio();
}

void 
GRectMapper::set_output(const GRect &rect)
{
  if (rect.isempty())
    G_THROW("GRect.empty_rect2");
  rectTo = rect;
  rw = rh = GRatio();
}

void 
GRectMapper::rotate(int count)
{
  int oldcode = code;
  switch (count & 0x3)
    {
    case 1:
      code ^= (code & SWAPXY) ? MIRRORY : MIRRORX;
      code ^= SWAPXY;
      break;
    case 2:
      code ^= (MIRRORX|MIRRORY);
      break;
    case 3:
      code ^= (code & SWAPXY) ? MIRRORX : MIRRORY;
      code ^= SWAPXY;
      break;
    }
  if ((oldcode ^ code) & SWAPXY)
    { 
      iswap(rectFrom.xmin, rectFrom.ymin);
      iswap(rectFrom.xmax, rectFrom.ymax);
      rw = rh = GRatio();
    }
}

void 
GRectMapper::mirrorx()
{
  code ^= MIRRORX;
}

void 
GRectMapper::mirrory()
{
  code ^= MIRRORY;
}

void
GRectMapper::precalc()
{
  if (rectTo.isempty() || rectFrom.isempty())
    G_THROW("GRect.empty_rect3");
  rw = GRatio(rectTo.width(), rectFrom.width());
  rh = GRatio(rectTo.height(), rectFrom.height());
}

void 
GRectMapper::map(int &x, int &y)
{
  int mx = x;
  int my = y;
  // precalc
  if (! (rw.p && rh.p))
    precalc();
  // swap and mirror
  if (code & SWAPXY)
    iswap(mx,my);
  if (code & MIRRORX)
    mx = rectFrom.xmin + rectFrom.xmax - mx;
  if (code & MIRRORY)
    my = rectFrom.ymin + rectFrom.ymax - my;
  // scale and translate
  x = rectTo.xmin + (mx - rectFrom.xmin) * rw;
  y = rectTo.ymin + (my - rectFrom.ymin) * rh;
}

void 
GRectMapper::unmap(int &x, int &y)
{
  // precalc 
  if (! (rw.p && rh.p))
    precalc();
  // scale and translate
  int mx = rectFrom.xmin + (x - rectTo.xmin) / rw;
  int my = rectFrom.ymin + (y - rectTo.ymin) / rh;
  //  mirror and swap
  if (code & MIRRORX)
    mx = rectFrom.xmin + rectFrom.xmax - mx;
  if (code & MIRRORY)
    my = rectFrom.ymin + rectFrom.ymax - my;
  if (code & SWAPXY)
    iswap(mx,my);
  x = mx;
  y = my;
}

void 
GRectMapper::map(GRect &rect)
{
  map(rect.xmin, rect.ymin);
  map(rect.xmax, rect.ymax);
  if (rect.xmin >= rect.xmax)
    iswap(rect.xmin, rect.xmax);
  if (rect.ymin >= rect.ymax)
    iswap(rect.ymin, rect.ymax);
}

void 
GRectMapper::unmap(GRect &rect)
{
  unmap(rect.xmin, rect.ymin);
  unmap(rect.xmax, rect.ymax);
  if (rect.xmin >= rect.xmax)
    iswap(rect.xmin, rect.xmax);
  if (rect.ymin >= rect.ymax)
    iswap(rect.ymin, rect.ymax);
}

GRect 
GRectMapper::get_input()
{
    return rectFrom;
}

GRect 
GRectMapper::get_output()
{
    return rectTo;
}

