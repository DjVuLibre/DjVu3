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
//C- $Id: GMapAreas.cpp,v 1.4 1999-10-18 16:50:39 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GMapAreas.h"
#include "GException.h"

#include <math.h>
#include <stdio.h>

/****************************************************************************
***************************** GMapArea definition ***************************
****************************************************************************/

void
GMapArea::initialize_bounds(void)
{
   xmin=gma_get_xmin(); xmax=gma_get_xmax();
   ymin=gma_get_ymin(); ymax=gma_get_ymax();
   bounds_initialized=1;
}

int
GMapArea::get_xmin(void)
{
   if (!bounds_initialized) initialize_bounds();
   return xmin;
}

int
GMapArea::get_ymin(void)
{
   if (!bounds_initialized) initialize_bounds();
   return ymin;
}

int
GMapArea::get_xmax(void)
{
   if (!bounds_initialized) initialize_bounds();
   return xmax;
}

int
GMapArea::get_ymax(void)
{
   if (!bounds_initialized) initialize_bounds();
   return ymax;
}

GRect
GMapArea::get_bound_rect(void)
{
   return GRect(get_xmin(), get_ymin(), get_xmax()-get_xmin(),
		get_ymax()-get_ymin());
}

void
GMapArea::move(int dx, int dy)
{
   if (!dx && !dy) return;
   if (bounds_initialized)
   {
      xmin+=dx; ymin+=dy; xmax+=dx; ymax+=dy;
   }
   gma_move(dx, dy);
}

void
GMapArea::resize(int new_width, int new_height)
{
   if (get_xmax()-get_xmin()==new_width &&
       get_ymax()-get_ymin()==new_height) return;
   gma_resize(new_width, new_height);
   bounds_initialized=0;
}

void
GMapArea::transform(const GRect & grect)
{
   if (grect.xmin==get_xmin() && grect.ymin==get_ymin() &&
       grect.xmax==get_xmax() && grect.ymax==get_ymax())
      return;
   gma_transform(grect);
   bounds_initialized=0;
}

GString
GMapArea::check_object(void)
{
   if (get_xmax()==get_xmin()) return "Area width is ZERO";
   if (get_ymax()==get_ymin()) return "Area height is ZERO";
   if (border_type==XOR_BORDER ||
       border_type==SOLID_BORDER)
      if (border_width!=1)
	 return "Border width must be 1 for XOR and SOLID border types";
   if (border_type==SHADOW_IN_BORDER ||
       border_type==SHADOW_OUT_BORDER ||
       border_type==SHADOW_EIN_BORDER ||
       border_type==SHADOW_EOUT_BORDER)
      if (border_width<3 || border_width>32)
	 return "Border width must be between 3 and 32 for SHADOW border types";
   return gma_check_object();
}

bool
GMapArea::is_point_inside(int x, int y)
{
   if (!bounds_initialized) initialize_bounds();
   return (x>=xmin && x<xmax && y>=ymin && y<ymax) ?
	      gma_is_point_inside(x, y) : false;
}

GString
GMapArea::print(void)
{
      // Make this hard check to make sure, that *no* illegal GMapArea
      // can be stored into a file.
   GString errors=check_object();
   if (errors.length()) THROW(errors);
   
   int i;
   GString tmp;
   GString url1, target1, comment1;
   GString url_str=(const char *) url;
   for(i=0;i<(int) url_str.length();i++)
   {
      char ch=url_str[i];
      if (ch=='"') url1+='\\';
      url1+=ch;
   }
   for(i=0;i<(int) target.length();i++)
   {
      char ch=target[i];
      if (ch=='"') target1+='\\';
      target1+=ch;
   }
   for(i=0;i<(int) comment.length();i++)
   {
      char ch=comment[i];
      if (ch=='"') comment1+='\\';
      comment1+=ch;
   }
   
   char border_width_str[128];
   sprintf(border_width_str, "%d", border_width);
   
   GString border_color_str;
   char buffer[128];
   sprintf(buffer, "#%02X%02X%02X",
	   (border_color & 0xff0000) >> 16,
	   (border_color & 0xff00) >> 8,
	   (border_color & 0xff));
   border_color_str=buffer;
   
   GString border_type_str;
   switch(border_type)
   {
      case NO_BORDER: border_type_str="(" NO_BORDER_TAG ")"; break;
      case XOR_BORDER: border_type_str="(" XOR_BORDER_TAG ")"; break;
      case SOLID_BORDER: border_type_str="(" SOLID_BORDER_TAG " "+border_color_str+")"; break;
      case SHADOW_IN_BORDER: border_type_str=GString("(" SHADOW_IN_BORDER_TAG " ")+border_width_str+")"; break;
      case SHADOW_OUT_BORDER: border_type_str=GString("(" SHADOW_OUT_BORDER_TAG " ")+border_width_str+")"; break;
      case SHADOW_EIN_BORDER: border_type_str=GString("(" SHADOW_EIN_BORDER_TAG " ")+border_width_str+")"; break;
      case SHADOW_EOUT_BORDER: border_type_str=GString("(" SHADOW_EOUT_BORDER_TAG " ")+border_width_str+")"; break;
      default: border_type_str="(" XOR_BORDER_TAG ")"; break;
   }

   GString hilite_str;
   if (hilite_color!=0xffffffff)
   {
      char buffer[128];
      sprintf(buffer, "(" HILITE_TAG " #%02X%02X%02X)",
	      (hilite_color & 0xff0000) >> 16,
	      (hilite_color & 0xff00) >> 8,
	      (hilite_color & 0xff));
      hilite_str=buffer;
   }
   
   GString URL;
   if (target1=="_self") URL="\""+url1+"\"";
   else URL="(url \""+url1+"\" \""+target1+"\")";

   GString total="(" MAPAREA_TAG " "+URL+" \""+comment1+"\" "+gma_print()+border_type_str;
   if (border_always_visible) total+=" (" BORDER_AVIS_TAG ")";
   if (hilite_str.length()) total+=" "+hilite_str;
   total+=")";
   return total;
}

/****************************************************************************
**************************** GMapRect definition ****************************
****************************************************************************/

void
GMapRect::gma_resize(int new_width, int new_height)
{
   xmax=xmin+new_width;
   ymax=ymin+new_height;
}

void
GMapRect::gma_transform(const GRect & grect)
{
   xmin=grect.xmin; ymin=grect.ymin;
   xmax=grect.xmax; ymax=grect.ymax;
}

GString
GMapRect::gma_print(void)
{
   char buffer[128];
   sprintf(buffer, "(" RECT_TAG " %d %d %d %d) ",
	   xmin, ymin, xmax-xmin, ymax-ymin);
   return buffer;
}

/****************************************************************************
**************************** GMapPoly definition ****************************
****************************************************************************/

inline int
GMapPoly::sign(int x) { return x<0 ? -1 : x>0 ? 1 : 0; }

bool
GMapPoly::does_side_cross_rect(const GRect & grect, int side)
{
   int x1=xx[side], x2=xx[(side+1)%points];
   int y1=yy[side], y2=yy[(side+1)%points];
   int xmin=x1<x2 ? x1 : x2;
   int ymin=y1<y2 ? y1 : y2;
   int xmax=x1+x2-xmin;
   int ymax=y1+y2-ymin;
   if (xmax<grect.xmin || xmin>grect.xmax ||
       ymax<grect.ymin || ymin>grect.ymax) return false;
   
   return
      x1>=grect.xmin && x1<grect.xmax && y1>=grect.ymin && y1<grect.ymax ||
      x2>=grect.xmin && x2<grect.xmax && y2>=grect.ymin && y2<grect.ymax ||
      do_segments_intersect(grect.xmin, grect.ymin, grect.xmax-1, grect.ymax-1,
			    x1, y1, x2, y2) ||
      do_segments_intersect(grect.xmax-1, grect.ymin, grect.xmin, grect.ymax-1,
			    x1, y1, x2, y2);
}

bool
GMapPoly::is_projection_on_segment(int x, int y, int x1, int y1, int x2, int y2)
{
   int res1=(x-x1)*(x2-x1)+(y-y1)*(y2-y1);
   int res2=(x-x2)*(x2-x1)+(y-y2)*(y2-y1);
   return sign(res1)*sign(res2)<=0;
}

bool
GMapPoly::do_segments_intersect(int x11, int y11, int x12, int y12,
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
   }
   int sign1=sign(res11)*sign(res12);
   int sign2=sign(res21)*sign(res22);
   return sign1<=0 && sign2<=0;
}

bool
GMapPoly::are_segments_parallel(int x11, int y11, int x12, int y12,
				int x21, int y21, int x22, int y22)
{
   return (x12-x11)*(y22-y21)-(y12-y11)*(x22-x21)==0;
}

GString
GMapPoly::check_data(void)
{
   if (open && points<2 || !open && points<3) return "Too few points in polygon.";
   for(int i=0;i<sides;i++)
      for(int j=i+2;j<sides;j++)
         if ((j+1)%points!=i)
	    if (do_segments_intersect(xx[i], yy[i], xx[i+1], yy[i+1],
				      xx[j], yy[j], xx[(j+1)%points], yy[(j+1)%points]))
		return "Some polygon segments intersect.";
   return "";
}

void
GMapPoly::optimize_data(void)
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
	 }
	 points--; sides--;
	 if (!points) return;
      }
   }
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
	 }
	 points--; sides--;
	 if (!points) return;
      }
   }
}

bool
GMapPoly::gma_is_point_inside(int xin, int yin)
{
   if (open) return false;
   
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
	    return true;
	 }
      }
      if (res1<0 && res2>0 || res1>0 && res2<0)
      {
	 int x1=xx[i%points], y1=yy[i%points];
	 int x2=xx[(i+1)%points], y2=yy[(i+1)%points];
	 int _res1=(xin-x1)*(y2-y1)-(yin-y1)*(x2-x1);
	 int _res2=(xfar-x1)*(y2-y1)-(yin-y1)*(x2-x1);
	 if (!_res1 || !_res2)
	 {
	    // The point is on this boundary
	    return true;
	 }
	 if (sign(_res1)*sign(_res2)<0) intersections++;
      }
   }
   return (intersections % 2)!=0;
}

int
GMapPoly::gma_get_xmin(void)
{
   int x=xx[0];
   for(int i=1;i<points;i++)
      if (x>xx[i]) x=xx[i];
   return x;
}

int
GMapPoly::gma_get_xmax(void)
{
   int x=xx[0];
   for(int i=1;i<points;i++)
      if (x<xx[i]) x=xx[i];
   return x+1;
}

int
GMapPoly::gma_get_ymin(void)
{
   int y=yy[0];
   for(int i=1;i<points;i++)
      if (y>yy[i]) y=yy[i];
   return y;
}

int
GMapPoly::gma_get_ymax(void)
{
   int y=yy[0];
   for(int i=1;i<points;i++)
      if (y<yy[i]) y=yy[i];
   return y+1;
}

void
GMapPoly::gma_move(int dx, int dy)
{
   for(int i=0;i<points;i++)
   {
      xx[i]+=dx; yy[i]+=dy;
   }
}

void
GMapPoly::gma_resize(int new_width, int new_height)
{
   int width=get_xmax()-get_xmin();
   int height=get_ymax()-get_ymin();
   int xmin=get_xmin(), ymin=get_ymin();
   for(int i=0;i<points;i++)
   {
      xx[i]=xmin+(xx[i]-xmin)*new_width/width;
      yy[i]=ymin+(yy[i]-ymin)*new_height/height;
   }
}

void
GMapPoly::gma_transform(const GRect & grect)
{
   int width=get_xmax()-get_xmin();
   int height=get_ymax()-get_ymin();
   int xmin=get_xmin(), ymin=get_ymin();
   for(int i=0;i<points;i++)
   {
      xx[i]=grect.xmin+(xx[i]-xmin)*grect.width()/width;
      yy[i]=grect.ymin+(yy[i]-ymin)*grect.height()/height;
   }
}

GString
GMapPoly::gma_check_object(void)
{
   GString str=check_data();
   if (str.length()) return str;
   if (border_type!=NO_BORDER &&
       border_type!=SOLID_BORDER &&
       border_type!=XOR_BORDER)
      return "This border type is not supported by polygon map areas";
   if (hilite_color!=0xffffffff)
      return "Area highlighting is not supported by polygon map areas";
   return "";
}

GMapPoly::GMapPoly(const int * _xx, const int * _yy, int _points, bool _open) :
   open(_open), points(_points)
{
   sides=points-(open!=0);
   
   xx.resize(points-1); yy.resize(points-1);
   for(int i=0;i<points;i++)
   {
      xx[i]=_xx[i]; yy[i]=_yy[i];
   }
   optimize_data();
   GString res=check_data();
   if (res.length()) THROW(res);
}

GString
GMapPoly::gma_print(void)
{
   GString res="(" POLY_TAG " ";
   for(int i=0;i<points;i++)
   {
      char buffer[128];
      sprintf(buffer, "%d %d ", xx[i], yy[i]);
      res+=buffer;
   }
   res.setat(res.length()-1, 0);
   res+=") ";
   return res;
}

/****************************************************************************
**************************** GMapOval definition ****************************
****************************************************************************/

void
GMapOval::gma_resize(int new_width, int new_height)
{
   xmax=xmin+new_width;
   ymax=ymin+new_height;
   initialize();
}

void
GMapOval::gma_transform(const GRect & grect)
{
   xmin=grect.xmin; ymin=grect.ymin;
   xmax=grect.xmax; ymax=grect.ymax;
   initialize();
}

bool
GMapOval::gma_is_point_inside(int x, int y)
{
   return
      sqrt((x-xf1)*(x-xf1)+(y-yf1)*(y-yf1))+
      sqrt((x-xf2)*(x-xf2)+(y-yf2)*(y-yf2))<=2*rmax;
}

GString
GMapOval::gma_check_object(void)
{
   if (border_type!=NO_BORDER &&
       border_type!=SOLID_BORDER &&
       border_type!=XOR_BORDER)
      return "This border type is not supported by oval map areas";
   if (hilite_color!=0xffffffff)
      return "Area highlighting is not supported by oval map areas";
   return "";
}

void
GMapOval::initialize(void)
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
   }
}

GMapOval::GMapOval(const GRect & rect) : xmin(rect.xmin), ymin(rect.ymin),
   xmax(rect.xmax), ymax(rect.ymax)
{
   initialize();
}

GString
GMapOval::gma_print(void)
{
   char buffer[128];
   sprintf(buffer, "(" OVAL_TAG " %d %d %d %d) ",
	   xmin, ymin, xmax-xmin, ymax-ymin);
   return buffer;
}
