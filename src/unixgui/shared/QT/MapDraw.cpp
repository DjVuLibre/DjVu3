//C-  -*- C++ -*-
//C-
//C- DjVu� Unix Viewer (v. 3.5)
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
//C-
// 
// $Id: MapDraw.cpp,v 1.2 2001-07-25 17:10:42 mchen Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "MapDraw.h"
#include "debug.h"

#include <math.h>

static inline int
sign(int x)
{
   return x<0 ? -1 : x>0 ? 1 : 0;
}

void
MapDraw::drawPixel(GPixmap & pm, int x, int y, bool xor,
		   u_char r, u_char g, u_char b)
{
   if (y>=0 && y<(int) pm.rows())
      if (xor)
      {
	 GPixel * pix=pm[pm.rows()-1-y]+x;
	 if (x>=0 && x<(int) pm.columns())
	 {
	    pix->r^=0xff;
	    pix->g^=0xff;
	    pix->b^=0xff;
	 }
      } else
      {
	 GPixel * pix=pm[pm.rows()-1-y]+x;
	 if (x>=0 && x<(int) pm.columns())
	 {
	    pix->r=r;
	    pix->g=g;
	    pix->b=b;
	 }
      }
}

void
MapDraw::drawLine1(GPixmap & pm, int x1, int y1, int x2, int y2, u_int32 color)
{
   int R=(color >> 16) & 0xff;
   int G=(color >> 8) & 0xff;
   int B=color & 0xff;
   bool xor=(color==0xff000000);
   
   int dx=x2-x1;
   int dy=y2-y1;

   int r,c,f,g;
   int inc1, inc2;

   char pos_slope=(dx>0);

   if (dy<0) pos_slope=!pos_slope;

   if (abs(dx)>abs(dy))
   {
      if (dx>0)
      {
	 c=x1;
	 r=y1;
	 f=x2;
      } else
      {
	 c=x2;
	 r=y2;
	 f=x1;
      }

      inc1=abs(dy)<<1;
      g=(abs(dy)<<1)-abs(dx);
      inc2=(abs(dy)-abs(dx))<<1;

      if (pos_slope)
      {
	 while(c<=f)
	 {
	    if (c!=x2 || r!=y2) drawPixel(pm, c, r, xor, R, G, B);

	    c++;

	    if (g>=0)
	    {
	       r++;
	       g+=inc2;
	    } else
	    {
	       g+=inc1;
	    }
	 }
      } else
      {
	 while(c<=f)
	 {
	       /// set pixel at c, r
	    if (c!=x2 || r!=y2) drawPixel(pm, c, r, xor, R, G, B);

	    c++;

	    if (g>0)
	    {
	       r--;
	       g+=inc2;
	    } else
	    {
	       g+=inc1;
	    }
	 }
      }
   } else
   {
      if (dy>0)
      {
	 c=y1;
	 r=x1;
	 f=y2;
      } else
      {
	 c=y2;
	 r=x2;
	 f=y1;
      }

      inc1=abs(dx)<<1;
      g=(abs(dx)<<1)-abs(dy);
      inc2=(abs(dx)-abs(dy))<<1;

      if (pos_slope)
      {
	 while(c<=f)
	 {
	    if (r!=x2 || c!=y2) drawPixel(pm, r, c, xor, R, G, B);

	    c++;

	    if (g>=0)
	    {
	       r++;
	       g+=inc2;
	    } else
	    {
	       g+=inc1;
	    }
	 }
      } else
      {
	 while(c<=f)
	 {
	       /// set pixel at r, c
	    if (r!=x2 || c!=y2) drawPixel(pm, r, c, xor, R, G, B);

	    c++;

	    if(g>0)
	    {
	       r--;
	       g+=inc2;
	    } else
	    {
	       g+=inc1;
	    }
	 }
      }
   }
}

void
MapDraw::drawLine(GPixmap & pm, int x1, int y1, int x2, int y2, u_int32 color)
      // Draws line between two points. Coordinate system origin is in
      // the upper left corner of the pixmap. color=0xff000000 means do XOR
      // End points can be outside of the pixmap
{
      // Color components
   int r=(color >> 16) & 0xff;
   int g=(color >> 8) & 0xff;
   int b=color & 0xff;
   bool xor=(color==0xff000000);

   if (x1==x2)
   {
	 // Vertical line
      if (x1>=0 && x1<(int) pm.columns())
      {
	 int ystart=y1<y2 ? y1 : y2;
	 int yend=y1+y2-ystart;
	 if (ystart<(int) pm.rows() && yend>=0)
	 {
	    if (ystart<0) ystart=0;
	    if (yend>=(int) pm.rows()) yend=(int) pm.rows()-1;

	    int jstart=(int) pm.rows()-1-yend;
	    int jend=(int) pm.rows()-1-ystart;
	    if (xor)
	       for(int j=jstart;j<=jend;j++)
	       {
		  GPixel & pix=pm[j][x1];
		  pix.r^=0xff;
		  pix.g^=0xff;
		  pix.b^=0xff;
	       }
	    else
	       for(int j=jstart;j<=jend;j++)
	       {
		  GPixel & pix=pm[j][x1];
		  pix.r=r;
		  pix.g=g;
		  pix.b=b;
	       }
	 }
      }
   } else if (y1==y2)
   {
	 // Horizontal line
      if (y1>=0 && y1<(int) pm.rows())
      {
	 int xstart=x1<x2 ? x1 : x2;
	 int xend=x1+x2-xstart;
	 if (xstart<(int) pm.columns() && xend>=0)
	 {
	    if (xstart<0) xstart=0;
	    if (xend>=(int) pm.columns()) xend=(int) pm.columns()-1;

	    int j=(int) pm.rows()-1-y1;
	    if (xor)
	       for(int x=xstart;x<=xend;x++)
	       {
		  GPixel & pix=pm[j][x];
		  pix.r^=0xff;
		  pix.g^=0xff;
		  pix.b^=0xff;
	       }
	    else
	       for(int x=xstart;x<=xend;x++)
	       {
		  GPixel & pix=pm[j][x];
		  pix.r=r;
		  pix.g=g;
		  pix.b=b;
	       }
	 }
      }
   } else drawLine1(pm, x1, y1, x2, y2, color);
}

void
MapDraw::drawRect(GPixmap & pm, const GRect & grect, u_int32 color)
{
   drawLine(pm, grect.xmin, grect.ymin, grect.xmax-1, grect.ymin, color);
   drawLine(pm, grect.xmax-1, grect.ymin, grect.xmax-1, grect.ymax-1, color);
   drawLine(pm, grect.xmax-1, grect.ymax-1, grect.xmin, grect.ymax-1, color);
   drawLine(pm, grect.xmin, grect.ymax-1, grect.xmin, grect.ymin, color);

      /* Cross drawing code
   drawLine(pm, grect.xmin, grect.ymin, grect.xmax-1, grect.ymax-1, color);
   drawLine(pm, grect.xmin, grect.ymax-1, grect.xmax-1, grect.ymin, color);*/
}

inline void
MapDraw::drawOvalPixel(OvalPixel & pix)
{
   drawPixel(*pix.pm, pix.x0+pix.x, pix.y0-pix.y, pix.xor, pix.r, pix.g, pix.b);
   if (pix.x!=0) drawPixel(*pix.pm, pix.x0-pix.x, pix.y0-pix.y, pix.xor, pix.r, pix.g, pix.b);
   if (pix.y!=0)
   {
      drawPixel(*pix.pm, pix.x0+pix.x, pix.y0+pix.y, pix.xor, pix.r, pix.g, pix.b);
      if (pix.x!=0) drawPixel(*pix.pm, pix.x0-pix.x, pix.y0+pix.y, pix.xor, pix.r, pix.g, pix.b);
   }
}

void
MapDraw::drawOval(GPixmap & pm, const GRect & grect, u_int32 color)
{
   DEBUG_MSG("MapDraw::drawOval() called\n");
      // "Correct" width and height to be odd. This is cheating, but
      // who cares. It's all very approximate anyway.
   int xmin=grect.xmin, ymin=grect.ymin;
   int xmax=grect.xmax, ymax=grect.ymax;
   if ((grect.width() % 2)==0) xmax--;
   if ((grect.height() % 2)==0) ymax--;
//     int a=(xmax-xmin)/2;	// Yes, I know, I lose 1/2 here.
//     int b=(ymax-ymin)/2;	// And here too
   float a=(xmax-xmin)/2.;
   float b=(ymax-ymin)/2.;
   float epsilon=0.0000001;     // this has to be defined globally
   float a2=a*a, b2=b*b;
   if ( fabs(a2) < epsilon || fabs(b2) < epsilon )
   {
      DEBUG_MSG("bounding box's width/height is too small, abort drawing oval\n");
      return;
   }
   float ab=a2/b2, ba=b2/a2;
   int x0=(int) floor((xmin+xmax)/2.0);
   int y0=(int) floor((ymin+ymax)/2.0);

      // Set up the context for drawOvalPixel()
   OvalPixel pix;
   pix.pm=&pm;
   pix.x0=x0;
   pix.y0=y0;
   pix.xor=(color==0xff000000);
   pix.r=(color >> 16) & 0xff;
   pix.g=(color >> 8) & 0xff;
   pix.b=color & 0xff;

   int ix=0, iy;
   float fx=0, fy=y0-ymin;
   pix.x=0;
   pix.y=iy=(int) rint(fy);
   drawOvalPixel(pix);

   a2=a/sqrt(2);

      // Integrate ~1/8 of the oval (y>x>0)
   for(ix++;ix<a2;ix++)
   {
      float new_fy=fy-ba*ix/fy;
      int new_iy=(int) rint(new_fy);
      if (new_iy==iy)
      {
	    // Simple: draw only one pixel
	 pix.x=ix;
	 drawOvalPixel(pix);
      } else if (new_iy==iy-1)
      {
	    // Not much more difficult: one pixel too
	 pix.x=ix;
	 pix.y=new_iy;
	 drawOvalPixel(pix);
      } else
      {
	    // Draw a vertical line
	 int ystart=iy-1;
	 int yend=new_iy;
	 int l=(ystart-yend)/2;
	 for(int i=0;i<l && ystart>=yend;i++, ystart--)
	 {
	    pix.y=ystart;
	    drawOvalPixel(pix);
	 }
	 pix.x=ix;
	 for(;ystart>=yend;ystart--)
	 {
	    pix.y=ystart;
	    drawOvalPixel(pix);
	 }
      }
      fy=new_fy;
      iy=new_iy;
   }

   fx=--ix;

      // Integrate ~1/8 of the oval (x>y>0)
   for(iy--;iy>=0;iy--)
   {
      float new_fx=fx+ab*iy/fx;
      int new_ix=(int) rint(new_fx);
      if (new_ix==ix)
      {
	    // Simple: draw only one pixel
	 pix.y=iy;
	 drawOvalPixel(pix);
      } else if (new_ix==ix+1)
      {
	    // Not much more difficult: one pixel too
	 pix.x=new_ix;
	 pix.y=iy;
	 drawOvalPixel(pix);
      } else
      {
	    // Draw a horizontal line
	 int xstart=ix+1;
	 int xend=new_ix;
	 int l=(xend-xstart)/2;
	 for(int i=0;i<l && xstart<=xend;i++, xstart++)
	 {
	    pix.x=xstart;
	    drawOvalPixel(pix);
	 }
	 pix.y=iy;
	 for(;xstart<=xend;xstart++)
	 {
	    pix.x=xstart;
	    drawOvalPixel(pix);
	 }
      }
      fx=new_fx;
      ix=new_ix;
   }	       
   DEBUG_MSG("MapDraw::drawOval(): DONE\n");
}

