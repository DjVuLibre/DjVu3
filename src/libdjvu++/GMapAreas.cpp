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
//C- $Id: GMapAreas.cpp,v 1.12 2000-10-12 19:01:51 bcr Exp $

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

const char GMapArea::MAPAREA_TAG[] = 		"maparea";
const char GMapArea::RECT_TAG[] = 		"rect";
const char GMapArea::POLY_TAG[] = 		"poly";
const char GMapArea::OVAL_TAG[] = 		"oval";
const char GMapArea::NO_BORDER_TAG[] = 		"none";
const char GMapArea::XOR_BORDER_TAG[] = 	"xor";
const char GMapArea::SOLID_BORDER_TAG[] = 	"border";
const char GMapArea::SHADOW_IN_BORDER_TAG[] = 	"shadow_in";
const char GMapArea::SHADOW_OUT_BORDER_TAG[] = 	"shadow_out";
const char GMapArea::SHADOW_EIN_BORDER_TAG[] = 	"shadow_ein";
const char GMapArea::SHADOW_EOUT_BORDER_TAG[] = "shadow_eout";
const char GMapArea::BORDER_AVIS_TAG[] = 	"border_avis";
const char GMapArea::HILITE_TAG[] = 		"hilite";
const char GMapArea::URL_TAG[] = 		"url";
const char GMapArea::TARGET_SELF[] = 		"_self";
static const char zero_width[] = "GMapAreas.zero_width";
static const char zero_height[] = "GMapAreas.zero_height";
static const char width_1[] = "GMapAreas.width_1";
static const char width_3_32 [] = "GMapAreas.width_3-32";
static const char error_poly_border [] = "GMapAreas.poly_border";
static const char error_poly_hilite [] = "GMapAreas.poly_hilite";
static const char error_oval_border [] = "GMapAreas.oval_border";
static const char error_oval_hilite [] = "GMapAreas.oval_hilite";
static const char error_too_few_points [] = "GMapAreas.too_few_points";
static const char error_intersect [] = "GMapAreas.intersect";

void
GMapArea::initialize_bounds(void)
{
   xmin=gma_get_xmin();
   xmax=gma_get_xmax();
   ymin=gma_get_ymin();
   ymax=gma_get_ymax();
   bounds_initialized=true;
}

int
GMapArea::get_xmin(void)
{
   if (!bounds_initialized)
     initialize_bounds();
   return xmin;
}

int
GMapArea::get_ymin(void)
{
   if (!bounds_initialized)
     initialize_bounds();
   return ymin;
}

int
GMapArea::get_xmax(void)
{
   if (!bounds_initialized)
     initialize_bounds();
   return xmax;
}

int
GMapArea::get_ymax(void)
{
   if (!bounds_initialized)
     initialize_bounds();
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
   if (dx || dy)
   {
     if (bounds_initialized)
     {
        xmin+=dx;
        ymin+=dy;
        xmax+=dx;
        ymax+=dy;
     }
     gma_move(dx, dy);
   }
}

void
GMapArea::resize(int new_width, int new_height)
{
   if (get_xmax()-get_xmin()!=new_width ||
       get_ymax()-get_ymin()!=new_height)
   {
     gma_resize(new_width, new_height);
     bounds_initialized=false;
   }
}

void
GMapArea::transform(const GRect & grect)
{
   if (grect.xmin!=get_xmin() || grect.ymin!=get_ymin() ||
       grect.xmax!=get_xmax() || grect.ymax!=get_ymax())
   {
     gma_transform(grect);
     bounds_initialized=false;
   }
}

char const * const
GMapArea::check_object(void)
{
   char const *retval;
   if (get_xmax()==get_xmin())
   {
     retval=zero_width;
   }
   else if (get_ymax()==get_ymin())
   {
     retval=zero_height;
   }
   else if ((border_type==XOR_BORDER ||
       border_type==SOLID_BORDER) && border_width!=1)
   {
     retval=width_1;
   }
   else if ((border_type==SHADOW_IN_BORDER ||
       border_type==SHADOW_OUT_BORDER ||
       border_type==SHADOW_EIN_BORDER ||
       border_type==SHADOW_EOUT_BORDER)&&
       (border_width<3 || border_width>32))
   {
     retval=width_3_32;
   }else
   {
     retval=gma_check_object();
   }
   return retval;
}

bool
GMapArea::is_point_inside(int x, int y)
{
   if (!bounds_initialized)
      initialize_bounds();
   return (x>=xmin && x<xmax && y>=ymin && y<ymax) ?
	      gma_is_point_inside(x, y) : false;
}

GString
GMapArea::print(void)
{
      // Make this hard check to make sure, that *no* illegal GMapArea
      // can be stored into a file.
   const char * const errors=check_object();
   if (errors[0])
   {
     G_THROW(errors);
   }
   
   int i;
   GString tmp;
   GString url1, target1, comment1;
   GString url_str=(const char *) url;
   for(i=0;i<(int) url_str.length();i++)
   {
      char ch=url_str[i];
      if (ch=='"')
        url1+='\\';
      url1+=ch;
   }
   for(i=0;i<(int) target.length();i++)
   {
      char ch=target[i];
      if (ch=='"')
        target1+='\\';
      target1+=ch;
   }
   for(i=0;i<(int) comment.length();i++)
   {
      char ch=comment[i];
      if (ch=='"')
        comment1+='\\';
      comment1+=ch;
   }
   
   GString border_color_str;
   border_color_str.format("#%02X%02X%02X",
	   (border_color & 0xff0000) >> 16,
	   (border_color & 0xff00) >> 8,
	   (border_color & 0xff));

   static const GString left('(');
   static const GString right(')');
   static const GString space(' ');
   static const GString quote('"');
   GString border_type_str;
   switch(border_type)
   {
      case NO_BORDER:
        border_type_str=left+NO_BORDER_TAG+right;
        break;
      case XOR_BORDER:
        border_type_str=left+XOR_BORDER_TAG+right;
        break;
      case SOLID_BORDER:
        border_type_str=left+SOLID_BORDER_TAG+space+border_color_str+right;
        break;
      case SHADOW_IN_BORDER:
        border_type_str=left+SHADOW_IN_BORDER_TAG+space+GString(border_width)+right;
        break;
      case SHADOW_OUT_BORDER:
        border_type_str=left+SHADOW_OUT_BORDER_TAG+space+GString(border_width)+right;
        break;
      case SHADOW_EIN_BORDER:
        border_type_str=left+SHADOW_EIN_BORDER_TAG+space+GString(border_width)+right;
        break;
      case SHADOW_EOUT_BORDER:
        border_type_str=left+SHADOW_EOUT_BORDER_TAG+space+GString(border_width)+right;
        break;
      default:
        border_type_str=left+XOR_BORDER_TAG+right;
        break;
   }

   GString hilite_str;
   if (hilite_color!=0xffffffff)
   {
      hilite_str.format("(%s #%02X%02X%02X)",
	      HILITE_TAG, (hilite_color & 0xff0000) >> 16,
	      (hilite_color & 0xff00) >> 8,
	      (hilite_color & 0xff));
   }
   
   GString URL;
   if (target1==TARGET_SELF)
   {
      URL=quote+url1+quote;
   }else
   {
      URL=left+URL_TAG+space+quote+url1+quote+space+quote+target1+quote+right;
   }

   GString total=left+MAPAREA_TAG+space+URL+space+quote+comment1+quote+space+gma_print()+border_type_str;
   if (border_always_visible)
     total+=space+left+BORDER_AVIS_TAG+right;
   if (hilite_str[0])
     total+=space+hilite_str;
   total+=right;
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
   sprintf(buffer, "(%s %d %d %d %d) ",
	   RECT_TAG, xmin, ymin, xmax-xmin, ymax-ymin);
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
      x1>=grect.xmin && x1<=grect.xmax && y1>=grect.ymin && y1<=grect.ymax ||
      x2>=grect.xmin && x2<=grect.xmax && y2>=grect.ymin && y2<=grect.ymax ||
      do_segments_intersect(grect.xmin, grect.ymin, grect.xmax, grect.ymax,
			    x1, y1, x2, y2) ||
      do_segments_intersect(grect.xmax, grect.ymin, grect.xmin, grect.ymax,
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

char const * const
GMapPoly::check_data(void)
{
  if (open && points<2 || !open && points<3) 
    return error_too_few_points;
  for(int i=0;i<sides;i++)
  {
    for(int j=i+2;j<sides;j++)
    {
      if (i != (j+1)%points )
      {
        if (do_segments_intersect(xx[i], yy[i], xx[i+1], yy[i+1],
				      xx[j], yy[j], xx[(j+1)%points], yy[(j+1)%points]))
        {
          return error_intersect;
        }
      }
    }
  }
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
GMapPoly::gma_is_point_inside(const int xin, const int yin)
{
   if (open)
     return false;
   
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
GMapPoly::gma_get_xmin(void) const
{
   int x=xx[0];
   for(int i=1;i<points;i++)
      if (x>xx[i]) x=xx[i];
   return x;
}

int
GMapPoly::gma_get_xmax(void) const
{
   int x=xx[0];
   for(int i=1;i<points;i++)
      if (x<xx[i]) x=xx[i];
   return x+1;
}

int
GMapPoly::gma_get_ymin(void) const
{
   int y=yy[0];
   for(int i=1;i<points;i++)
      if (y>yy[i]) y=yy[i];
   return y;
}

int
GMapPoly::gma_get_ymax(void) const
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

char const * const
GMapPoly::gma_check_object(void) const
{
   const char * str=(const_cast<GMapPoly *>(this))->check_object();
   if (!str[0])
   {
     str=(border_type!=NO_BORDER &&
       border_type!=SOLID_BORDER &&
       border_type!=XOR_BORDER)?error_poly_border:
         ((hilite_color!=0xffffffff)?error_poly_hilite:"");
   }
   return str;
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
   char const * const res=check_data();
   if (res[0])
     G_THROW(res);
}

int      
GMapPoly::add_vertex(int x, int y)
{
    points++;
    sides=points-(open!=0);

    xx.resize(points-1); yy.resize(points-1);
    xx[points-1] = x;
    yy[points-1] = y;

    return points;
}

void
GMapPoly::close_poly()
{
    open = false;
    sides=points;
}

GString
GMapPoly::gma_print(void)
{
   static const GString space(' ');
   GString res=GString('(')+POLY_TAG+space;
   for(int i=0;i<points;i++)
   {
      char buffer[128];
      sprintf(buffer, "%d %d ", xx[i], yy[i]);
      res+=buffer;
   }
   res.setat(res.length()-1, ')');
   res+=space;
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
GMapOval::gma_is_point_inside(const int x, const int y)
{
   return
      sqrt((x-xf1)*(x-xf1)+(y-yf1)*(y-yf1))+
      sqrt((x-xf2)*(x-xf2)+(y-yf2)*(y-yf2))<=2*rmax;
}

char const * const
GMapOval::gma_check_object(void) const
{
   return (border_type!=NO_BORDER &&
       border_type!=SOLID_BORDER &&
       border_type!=XOR_BORDER)?error_oval_border:
      ((hilite_color!=0xffffffff)?"":error_oval_hilite);
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
   sprintf(buffer, "(%s %d %d %d %d) ",
	   OVAL_TAG, xmin, ymin, xmax-xmin, ymax-ymin);
   return buffer;
}
