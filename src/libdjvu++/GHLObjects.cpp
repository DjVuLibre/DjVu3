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
//C- $Id: GHLObjects.cpp,v 1.1 1999-04-26 19:30:33 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GHLObjects.h"
#include "GException.h"

#include <math.h>
#include <stdio.h>

/****************************************************************************
*************************** GHLObject declaration ***************************
****************************************************************************/

GString
GHLObject::print(void)
{
   int i;
   GString tmp;
   GString url1, target1, comment1;
   GString url_str=(const char *) url;
   for(i=0;i<(int) url_str.length();i++)
   {
      char ch=url_str[i];
      if (ch=='"') url1+='\\';
      url1+=ch;
   };
   for(i=0;i<(int) target.length();i++)
   {
      char ch=target[i];
      if (ch=='"') target1+='\\';
      target1+=ch;
   };
   for(i=0;i<(int) comment.length();i++)
   {
      char ch=comment[i];
      if (ch=='"') comment1+='\\';
      comment1+=ch;
   };
   
   char thick[128];
   sprintf(thick, "%d", shadow_thick);
   
   GString color;
   char buffer[128];
   sprintf(buffer, "#%02X%02X%02X",
	   (hlcolor_rgb & 0xff0000) >> 16,
	   (hlcolor_rgb & 0xff00) >> 8,
	   (hlcolor_rgb & 0xff));
   color=buffer;
   
   GString hlstr;
   switch(hltype)
   {
      case NONE: hlstr="(" NONE_TAG ")"; break;
      case XOR: hlstr="(" XOR_TAG ")"; break;
      case BORDER: hlstr="(" BORDER_TAG " "+color+")"; break;
      case SHADOW_IN: hlstr=GString("(" SHADOW_IN_TAG " ")+thick+")"; break;
      case SHADOW_OUT: hlstr=GString("(" SHADOW_OUT_TAG " ")+thick+")"; break;
      case SHADOW_EIN: hlstr=GString("(" SHADOW_EIN_TAG " ")+thick+")"; break;
      case SHADOW_EOUT: hlstr=GString("(" SHADOW_EOUT_TAG " ")+thick+")"; break;
      default: hlstr="(" XOR_TAG ")"; break;
   };
   GString URL;
   if (target1=="_self") URL="\""+url1+"\"";
   else URL="(url \""+url1+"\" \""+target1+"\")";
   return "(" MAPAREA_TAG " "+URL+" \""+comment1+"\" "+GHL_print()+hlstr+")";
}

/****************************************************************************
**************************** GHLRect declaration ****************************
****************************************************************************/

void
GHLRect::GHL_resize(int new_width, int new_height)
{
   xmax=xmin+new_width;
   ymax=ymin+new_height;
}

void
GHLRect::GHL_transform(const GRect & grect)
{
   xmin=grect.xmin; ymin=grect.ymin;
   xmax=grect.xmax; ymax=grect.ymax;
}

GString
GHLRect::GHL_print(void)
{
   char buffer[128];
   sprintf(buffer, "(" RECT_TAG " %d %d %d %d) ",
	   xmin, ymin, xmax-xmin, ymax-ymin);
   return buffer;
}

/****************************************************************************
**************************** GHLPoly declaration ****************************
****************************************************************************/

inline int
GHLPoly::sign(int x) { return x<0 ? -1 : x>0 ? 1 : 0; }

bool
GHLPoly::does_side_cross_rect(const GRect & grect, int side)
{
   int x1=xx[side], x2=xx[(side+1)%points];
   int y1=yy[side], y2=yy[(side+1)%points];
   return
      x1>=grect.xmin && x1<grect.xmax && y1>=grect.ymin && y1<grect.ymax ||
      x2>=grect.xmin && x2<grect.xmax && y2>=grect.ymin && y2<grect.ymax ||
      do_segments_intersect(grect.xmin, grect.ymin, grect.xmax-1, grect.ymax-1,
			    x1, y1, x2, y2) ||
      do_segments_intersect(grect.xmax-1, grect.ymin, grect.xmin, grect.ymax-1,
			    x1, y1, x2, y2);
}

bool
GHLPoly::is_projection_on_segment(int x, int y, int x1, int y1, int x2, int y2)
{
   int res1=(x-x1)*(x2-x1)+(y-y1)*(y2-y1);
   int res2=(x-x2)*(x2-x1)+(y-y2)*(y2-y1);
   return sign(res1)*sign(res2)<=0;
}

bool
GHLPoly::do_segments_intersect(int x11, int y11, int x12, int y12,
			       int x21, int y21, int x22, int y22)
{
   int res11=(x11-x21)*(y22-y21)-(y11-y21)*(x22-x21);
   int res12=(x12-x21)*(y22-y21)-(y12-y21)*(x22-x21);
   int res21=(x21-x11)*(y12-y11)-(y21-y11)*(x12-x11);
   int res22=(x22-x11)*(y12-y11)-(y22-y11)*(x12-x11);
   if (!res11 && !res12)
   {
      // Segments are on the same line
      return
	 is_projection_on_segment(x11, y11, x21, y21, x22, y22) ||
	 is_projection_on_segment(x12, y12, x21, y21, x22, y22) ||
	 is_projection_on_segment(x21, y21, x11, y11, x12, y12) ||
	 is_projection_on_segment(x22, y22, x11, y11, x12, y12);
   };
   int sign1=sign(res11)*sign(res12);
   int sign2=sign(res21)*sign(res22);
   return sign1<=0 && sign2<=0 || sign1<=0 && sign2<=0;
}

bool
GHLPoly::are_segments_parallel(int x11, int y11, int x12, int y12,
			       int x21, int y21, int x22, int y22)
{
   return (x12-x11)*(y22-y21)-(y12-y11)*(x22-x21)==0;
}

GString
GHLPoly::check_data(void)
{
   if (open && points<2 || !open && points<3) return "GHLPoly::check_data(): Too few points.";
   for(int i=0;i<sides;i++)
      for(int j=i+2;j<sides;j++)
         if ((j+1)%points!=i)
	    if (do_segments_intersect(xx[i], yy[i], xx[i+1], yy[i+1],
				      xx[j], yy[j], xx[(j+1)%points], yy[(j+1)%points]))
		return "GHLPoly::check_data(): Some segments intersect.";
   return "";
}

void
GHLPoly::optimize_data(void)
{
   // Removing segments of length zero
   int i;
   for(i=0;i<sides;i++)
   {
      while(xx[i]==xx[(i+1)%points] && yy[i]==yy[(i+1)%points])
      {
	 for(int k=(i+1)%points;k<points-1;k++)
	 {
	    xx[k]=xx[k+1]; yy[k]=yy[k+1];
	 };
	 points--; sides--;
	 if (!points) return;
      };
   };
   // Concatenating consequitive parallel segments
   for(i=0;i<sides;i++)
   {
      while((open && i+1<sides || !open) &&
	    are_segments_parallel(xx[i], yy[i],
				  xx[(i+1)%points], yy[(i+1)%points],
				  xx[(i+1)%points], yy[(i+1)%points],
				  xx[(i+2)%points], yy[(i+2)%points]))
      {
	 for(int k=(i+1)%points;k<points-1;k++)
	 {
	    xx[k]=xx[k+1]; yy[k]=yy[k+1];
	 };
	 points--; sides--;
	 if (!points) return;
      };
   };
}

bool
GHLPoly::GHL_is_point_inside(int xin, int yin)
{
   if (open) return 0;
   
   int xfar=get_xmax()+(get_xmax()-get_xmin());
   
   int intersections=0;
   for(int i=0;i<points;i++)
   {
      int res1=yy[i]-yin;
      if (!res1) continue;
      int res2, isaved=i;
      while(!(res2=yy[(i+1)%points]-yin)) i++;
      if (isaved!=i)
      {
	 // Some points fell exactly on the line
	 if ((xx[(isaved+1)%points]-xin)*
	     (xx[i%points]-xin)<=0)
	 {
	    // Test point is exactly on the boundary
	    return 1;
	 };
      };
      if (res1<0 && res2>0 || res1>0 && res2<0)
      {
	 int x1=xx[i%points], y1=yy[i%points];
	 int x2=xx[(i+1)%points], y2=yy[(i+1)%points];
	 int _res1=(xin-x1)*(y2-y1)-(yin-y1)*(x2-x1);
	 int _res2=(xfar-x1)*(y2-y1)-(yin-y1)*(x2-x1);
	 if (!_res1 || !_res2)
	 {
	    // The point is on this boundary
	    return 1;
	 };
	 if (sign(_res1)*sign(_res2)<0) intersections++;
      };
   };
   return intersections%2;
}

int
GHLPoly::GHL_get_xmin(void)
{
   int x=xx[0];
   for(int i=1;i<points;i++)
      if (x>xx[i]) x=xx[i];
   return x;
}

int
GHLPoly::GHL_get_xmax(void)
{
   int x=xx[0];
   for(int i=1;i<points;i++)
      if (x<xx[i]) x=xx[i];
   return x+1;
}

int
GHLPoly::GHL_get_ymin(void)
{
   int y=yy[0];
   for(int i=1;i<points;i++)
      if (y>yy[i]) y=yy[i];
   return y;
}

int
GHLPoly::GHL_get_ymax(void)
{
   int y=yy[0];
   for(int i=1;i<points;i++)
      if (y<yy[i]) y=yy[i];
   return y+1;
}

void
GHLPoly::GHL_move(int dx, int dy)
{
   for(int i=0;i<points;i++)
   {
      xx[i]+=dx; yy[i]+=dy;
   };
}

void
GHLPoly::GHL_resize(int new_width, int new_height)
{
   int width=get_xmax()-get_xmin();
   int height=get_ymax()-get_ymin();
   int xmin=get_xmin(), ymin=get_ymin();
   for(int i=0;i<points;i++)
   {
      xx[i]=xmin+(xx[i]-xmin)*new_width/width;
      yy[i]=ymin+(yy[i]-ymin)*new_height/height;
   };
}

void
GHLPoly::GHL_transform(const GRect & grect)
{
   int width=get_xmax()-get_xmin();
   int height=get_ymax()-get_ymin();
   int xmin=get_xmin(), ymin=get_ymin();
   for(int i=0;i<points;i++)
   {
      xx[i]=grect.xmin+(xx[i]-xmin)*grect.width()/width;
      yy[i]=grect.ymin+(yy[i]-ymin)*grect.height()/height;
   };
}

bool
GHLPoly::GHL_check_object(void)
{
   GString res=check_data();
   return res.length()==0;
}

GHLPoly::GHLPoly(const int * _xx, const int * _yy, int _points, bool _open) :
   open(_open), points(_points)
{
   sides=points-(open!=0);
   
   xx.resize(points-1); yy.resize(points-1);
   for(int i=0;i<points;i++)
   {
      xx[i]=_xx[i]; yy[i]=_yy[i];
   };
   optimize_data();
   GString res=check_data();
   if (res.length()) THROW(res);
}

GString
GHLPoly::GHL_print(void)
{
   GString res="(" POLY_TAG " ";
   for(int i=0;i<points;i++)
   {
      char buffer[128];
      sprintf(buffer, "%d %d ", xx[i], yy[i]);
      res+=buffer;
   };
   res.setat(res.length()-1, 0);
   res+=") ";
   return res;
}

/****************************************************************************
**************************** GHLOval declaration ****************************
****************************************************************************/

void
GHLOval::GHL_resize(int new_width, int new_height)
{
   xmax=xmin+new_width;
   ymax=ymin+new_height;
   initialize();
}

void
GHLOval::GHL_transform(const GRect & grect)
{
   xmin=grect.xmin; ymin=grect.ymin;
   xmax=grect.xmax; ymax=grect.ymax;
   initialize();
}

bool
GHLOval::GHL_is_point_inside(int x, int y)
{
   return
      sqrt((x-xf1)*(x-xf1)+(y-yf1)*(y-yf1))+
      sqrt((x-xf2)*(x-xf2)+(y-yf2)*(y-yf2))<=2*rmax;
}

void
GHLOval::initialize(void)
{
   int xc=(xmax+xmin)/2;
   int yc=(ymax+ymin)/2;
   int f;
   
   a=(xmax-xmin)/2;
   b=(ymax-ymin)/2;
   if (a>b)
   {
      rmin=b; rmax=a;
      f=(int) sqrt(rmax*rmax-rmin*rmin);
      xf1=xc+f; xf2=xc-f; yf1=yf2=yc;
   } else
   {
      rmin=a; rmax=b;
      f=(int) sqrt(rmax*rmax-rmin*rmin);
      yf1=yc+f; yf2=yc-f; xf1=xf2=xc;
   };
}

GHLOval::GHLOval(const GRect & rect) : xmin(rect.xmin), ymin(rect.ymin),
   xmax(rect.xmax), ymax(rect.ymax)
{
   initialize();
}

GString
GHLOval::GHL_print(void)
{
   char buffer[128];
   sprintf(buffer, "(" OVAL_TAG " %d %d %d %d) ",
	   xmin, ymin, xmax-xmin, ymax-ymin);
   return buffer;
}
