//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GRect.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $


// -- Implementation of class GRect and GRectMapper
// - File "$Id: GRect.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $"
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


// -- Class GRatio

GRatio::GRatio(int p, int q)
  : p(p), q(q)
{
  simplify();
}

void 
GRatio::simplify()
{
  if (q == 0) 
    THROW("Division by zero");
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


GRatio 
operator*(const GRatio &r1, const GRatio &r2)
{ 
  return GRatio(r1.p*r2.p, r1.q*r2.q); 
}

GRatio 
operator+(const GRatio &r1, const GRatio &r2)
{ 
  return GRatio(r1.p*r2.q+r1.q*r2.p, r1.q*r2.q); 
}

int
operator==(const GRatio &r1, const GRatio &r2)
{
  return (r1.p==r2.p && r1.q==r2.q);
}

int
operator!=(const GRatio &r1, const GRatio &r2)
{
  return (r1.p!=r2.p || r1.q!=r2.q);
}

int
operator<=(const GRatio &r1, const GRatio &r2)
{
  return (r1.p * r2.q <= r1.q * r2.p);
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
    THROW("Illegal empty rect in GRectMapper::set_input");
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
    THROW("Illegal empty rect in GRectMapper::set_output");
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
  rw = GRatio(rectTo.width(), rectFrom.width());
  rh = GRatio(rectTo.height(), rectFrom.height());
}

void 
GRectMapper::map(int &x, int &y)
{
  int mx = x;
  int my = y;
  // precalc
  if (! (rw.get_p() && rh.get_p()))
    precalc();
  // swap and mirror
  if (code & SWAPXY)
    iswap(mx,my);
  if (code & MIRRORX)
    mx = rectFrom.xmin + rectFrom.xmax - mx;
  if (code & MIRRORY)
    my = rectFrom.ymin + rectFrom.ymax - my;
  // scale and translate
  x = rectTo.xmin + (mx - rectFrom.xmin) * rw.get_p() / rw.get_q();
  y = rectTo.ymin + (my - rectFrom.ymin) * rh.get_p() / rh.get_q();
}

void 
GRectMapper::unmap(int &x, int &y)
{
  // precalc 
  if (! (rw.get_p() && rh.get_p()))
    precalc();
  // scale and translate
  int mx = rectFrom.xmin 
    + ( (x - rectTo.xmin) * rw.get_q() + rw.get_p()/2 ) / rw.get_p();
  int my = rectFrom.ymin 
    + ( (y - rectTo.ymin) * rh.get_q() + rh.get_p()/2 ) / rh.get_p();
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
