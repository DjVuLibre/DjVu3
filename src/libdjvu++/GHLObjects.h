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
//C- $Id: GHLObjects.h,v 1.1 1999-04-26 19:30:33 eaf Exp $

#ifndef _GHLOBJECTS_H
#define _GHLOBJECTS_H

#ifdef __GNUC__
#pragma interface
#endif

#include "GRect.h"
#include "Arrays.h"
#include "GString.h"

/****************************************************************************
*************************** GHLObject declaration ***************************
****************************************************************************/

#define MAPAREA_TAG     "maparea"
#define RECT_TAG        "rect"
#define POLY_TAG        "poly"
#define OVAL_TAG        "oval"
#define NONE_TAG        "none"
#define XOR_TAG         "xor"
#define BORDER_TAG      "border"
#define SHADOW_IN_TAG   "shadow_in"
#define SHADOW_OUT_TAG  "shadow_out"
#define SHADOW_EIN_TAG  "shadow_ein"
#define SHADOW_EOUT_TAG "shadow_eout"

class GHLObject
{
public:
   enum HLType { NONE=0, XOR=1, BORDER=2, SHADOW_IN=3, SHADOW_OUT=4,
		 SHADOW_EIN=5, SHADOW_EOUT=6 };
   GString	url, target;
   GString	comment;
   HLType	hltype;
   u_int32	hlcolor_rgb;
   int		shadow_thick;
   
   GHLObject(void);
   virtual ~GHLObject(void);
   
   bool		is_point_inside(int x, int y);

   int		get_xmin(void);
   int		get_ymin(void);
   int		get_xmax(void);
   int		get_ymax(void);
   GRect	get_bound_rect(void);
   void		move(int dx, int dy);
   void		resize(int new_width, int new_height);
   void		transform(const GRect & grect);
   bool		check_object(void);
   GString	print(void);
   
   virtual GString	get_shape_name(void) const=0;
protected:
   virtual int	GHL_get_xmin(void)=0;
   virtual int	GHL_get_ymin(void)=0;
   virtual int	GHL_get_xmax(void)=0;
   virtual int	GHL_get_ymax(void)=0;
   virtual void	GHL_move(int dx, int dy)=0;
   virtual void	GHL_resize(int new_width, int new_height)=0;
   virtual void	GHL_transform(const GRect & grect)=0;
   virtual bool	GHL_is_point_inside(int x, int y)=0;
   virtual bool	GHL_check_object(void)=0;
   virtual GString GHL_print(void)=0;
   
   void		clear_bounds(void);
private:
   int		xmin, xmax, ymin, ymax;
   bool		bounds_initialized;

   void		initialize_bounds(void);
};

inline
GHLObject::GHLObject(void) : target("_self"), hltype(XOR), hlcolor_rgb(0xff),
   shadow_thick(3), bounds_initialized(0) {}

inline
GHLObject::~GHLObject(void) {}

inline void
GHLObject::clear_bounds(void) { bounds_initialized=0; }

inline void
GHLObject::initialize_bounds(void)
{
   xmin=GHL_get_xmin(); xmax=GHL_get_xmax();
   ymin=GHL_get_ymin(); ymax=GHL_get_ymax();
   bounds_initialized=1;
}

inline int
GHLObject::get_xmin(void)
{
   if (!bounds_initialized) initialize_bounds();
   return xmin;
}

inline int
GHLObject::get_ymin(void)
{
   if (!bounds_initialized) initialize_bounds();
   return ymin;
}

inline int
GHLObject::get_xmax(void)
{
   if (!bounds_initialized) initialize_bounds();
   return xmax;
}

inline int
GHLObject::get_ymax(void)
{
   if (!bounds_initialized) initialize_bounds();
   return ymax;
}

inline GRect
GHLObject::get_bound_rect(void)
{
   return GRect(get_xmin(), get_ymin(), get_xmax()-get_xmin(),
		get_ymax()-get_ymin());
}

inline void
GHLObject::move(int dx, int dy)
{
   if (!dx && !dy) return;
   if (bounds_initialized)
   {
      xmin+=dx; ymin+=dy; xmax+=dx; ymax+=dy;
   }
   GHL_move(dx, dy);
}

inline void
GHLObject::resize(int new_width, int new_height)
{
   if (get_xmax()-get_xmin()==new_width &&
       get_ymax()-get_ymin()==new_height) return;
   GHL_resize(new_width, new_height);
   bounds_initialized=0;
}

inline void
GHLObject::transform(const GRect & grect)
{
   if (grect.xmin==get_xmin() && grect.ymin==get_ymin() &&
       grect.xmax==get_xmax() && grect.ymax==get_ymax())
      return;
   GHL_transform(grect);
   bounds_initialized=0;
}

inline bool
GHLObject::check_object(void)
{
   return (get_xmax()==get_xmin() || get_ymax()==get_ymin()) ? 0 :
	  GHL_check_object();
}

inline bool
GHLObject::is_point_inside(int x, int y)
{
   if (!bounds_initialized) initialize_bounds();
   return (x>=xmin && x<xmax && y>=ymin && y<ymax) ?
	      GHL_is_point_inside(x, y) : 0;
}

/****************************************************************************
**************************** GHLRect declaration ****************************
****************************************************************************/

class GHLRect: public virtual GHLObject, public GPEnabled
{
public:
   int		get_width(void) const;
   int		get_height(void) const;

   GHLRect(void);
   GHLRect(const GRect & rect);
   virtual ~GHLRect(void);

   virtual GString	get_shape_name(void) const;
protected:
   int		xmin, ymin, xmax, ymax;
   virtual int	GHL_get_xmin(void);
   virtual int	GHL_get_ymin(void);
   virtual int	GHL_get_xmax(void);
   virtual int	GHL_get_ymax(void);
   virtual void	GHL_move(int dx, int dy);
   virtual void	GHL_resize(int new_width, int new_height);
   virtual void	GHL_transform(const GRect & grect);
   virtual bool	GHL_is_point_inside(int x, int y);
   virtual bool	GHL_check_object(void);
   virtual GString GHL_print(void);
};

inline
GHLRect::GHLRect(void) : xmin(0), ymin(0), xmax(0), ymax(0) {}

inline
GHLRect::GHLRect(const GRect & rect) : xmin(rect.xmin), ymin(rect.ymin),
   xmax(rect.xmax), ymax(rect.ymax) {}

inline
GHLRect::~GHLRect(void) {}

inline int
GHLRect::get_width(void) const { return xmax-xmin; }

inline int
GHLRect::get_height(void) const { return ymax-ymin; }

inline int
GHLRect::GHL_get_xmin(void) { return xmin; }

inline int
GHLRect::GHL_get_ymin(void) { return ymin; }

inline int
GHLRect::GHL_get_xmax(void) { return xmax; }

inline int
GHLRect::GHL_get_ymax(void) { return ymax; }

inline bool
GHLRect::GHL_check_object(void) { return 1; }

inline void
GHLRect::GHL_move(int dx, int dy)
{
   xmin+=dx; xmax+=dx; ymin+=dy; ymax+=dy;
}

inline bool
GHLRect::GHL_is_point_inside(int x, int y)
{
   return x>=xmin && x<xmax && y>=ymin && y<ymax;
}

inline GString
GHLRect::get_shape_name(void) const { return "rect"; }

/****************************************************************************
**************************** GHLPoly declaration ****************************
****************************************************************************/

class GHLPoly : public virtual GHLObject, public GPEnabled
{
public:
   GHLPoly(void);
   GHLPoly(const int * xx, const int * yy, int points, bool open=0);
   virtual ~GHLPoly(void);

   bool		does_side_cross_rect(const GRect & grect, int side);

   int		get_points_num(void) const;
   int		get_sides_num(void) const;
   int		get_x(int i) const;
   int		get_y(int i) const;
   void		move_vertex(int i, int x, int y);
   
   virtual GString	get_shape_name(void) const;
protected:
   virtual int	GHL_get_xmin(void);
   virtual int	GHL_get_ymin(void);
   virtual int	GHL_get_xmax(void);
   virtual int	GHL_get_ymax(void);
   virtual void	GHL_move(int dx, int dy);
   virtual void	GHL_resize(int new_width, int new_height);
   virtual void	GHL_transform(const GRect & grect);
   virtual bool	GHL_is_point_inside(int x, int y);
   virtual bool	GHL_check_object(void);
   virtual GString GHL_print(void);
private:
   bool		open;
   int		points, sides;
   TArray<int>	xx, yy;
   static int	sign(int x);
   static bool	is_projection_on_segment(int x, int y, int x1, int y1, int x2, int y2);
   static bool	do_segments_intersect(int x11, int y11, int x12, int y12,
				      int x21, int y21, int x22, int y22);
   static bool	are_segments_parallel(int x11, int y11, int x12, int y12,
				      int x21, int y21, int x22, int y22);
   GString	check_data(void);
   void		optimize_data(void);
};

inline
GHLPoly::GHLPoly(void) : points(0), sides(0) {}

inline
GHLPoly::~GHLPoly(void) {}

inline int
GHLPoly::get_points_num(void) const { return points; }

inline int
GHLPoly::get_sides_num(void) const { return sides; }

inline int
GHLPoly::get_x(int i) const { return xx[i]; }

inline int
GHLPoly::get_y(int i) const { return yy[i]; }

inline void
GHLPoly::move_vertex(int i, int x, int y)
{
   xx[i]=x; yy[i]=y;
   clear_bounds();
}

inline GString
GHLPoly::get_shape_name(void) const { return "poly"; }

/****************************************************************************
**************************** GHLOval declaration ****************************
****************************************************************************/

class GHLOval: public virtual GHLObject, public GPEnabled
{
public:
   GHLOval(void);
   GHLOval(const GRect & rect);
   virtual ~GHLOval(void);
   
   int		get_a(void) const;
   int		get_b(void) const;
   int		get_rmin(void) const;
   int		get_rmax(void) const;
   
   virtual GString	get_shape_name(void) const;
protected:
   virtual int	GHL_get_xmin(void);
   virtual int	GHL_get_ymin(void);
   virtual int	GHL_get_xmax(void);
   virtual int	GHL_get_ymax(void);
   virtual void	GHL_move(int dx, int dy);
   virtual void	GHL_resize(int new_width, int new_height);
   virtual void	GHL_transform(const GRect & grect);
   virtual bool	GHL_is_point_inside(int x, int y);
   virtual bool	GHL_check_object(void);
   virtual GString GHL_print(void);
private:
   int		rmax, rmin;
   int		a, b;
   int		xf1, yf1, xf2, yf2;
   int		xmin, ymin, xmax, ymax;
   
   void		initialize(void);
};

inline
GHLOval::GHLOval(void) : xmin(0), ymin(0), xmax(0), ymax(0) {}

inline
GHLOval::~GHLOval(void) {}

inline int
GHLOval::get_a(void) const { return a; }

inline int
GHLOval::get_b(void) const { return b; }

inline int
GHLOval::get_rmin(void) const { return rmin; }

inline int
GHLOval::get_rmax(void) const { return rmax; }

inline int
GHLOval::GHL_get_xmin(void) { return xmin; }

inline int
GHLOval::GHL_get_ymin(void) { return ymin; }

inline int
GHLOval::GHL_get_xmax(void) { return xmax; }

inline int
GHLOval::GHL_get_ymax(void) { return ymax; }

inline bool
GHLOval::GHL_check_object(void) { return 1; }

inline void
GHLOval::GHL_move(int dx, int dy)
{
   xmin+=dx; xmax+=dx; ymin+=dy; ymax+=dy;
   xf1+=dx; yf1+=dy; xf2+=dx; yf2+=dy;
}

inline GString
GHLOval::get_shape_name(void) const { return "oval"; }

#endif
