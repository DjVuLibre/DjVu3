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
//C- $Id: GHLObjects.h,v 1.3 1999-05-25 19:42:29 eaf Exp $

#ifndef _GHLOBJECTS_H
#define _GHLOBJECTS_H

#ifdef __GNUC__
#pragma interface
#endif

#include "GRect.h"
#include "Arrays.h"
#include "GString.h"

/** @name GHLObjects.h

    Files #"GHLObjects.h"# and #"GHLObjects.cpp"# implement base hyperlink
    objects used by the plugin to display and manage hyperlinks inside a
    \Ref{DjVuImage} page.

    The currently supported hyperlinks can be rectangular (\Ref{GHLRect}),
    elliptical (\Ref{GHLOval}) and polygonal (\Ref{GHLPoly}). Every
    hyperlink object besides the defition of its shape contains the {\bf URL},
    which it refers to, a comment (normally displayed as a popup hint or in
    a status line) and highlight method (like #XOR#, #SHADOW_IN# or
    #SHADOW_ETCHED_IN#).

    The classes also implement some useful functions to ease geometry
    manipulations

    @memo Definition of base hyperlink classes
    @author Andrei Erofeev <eaf@geocities.com>
    @version
    #$Id: GHLObjects.h,v 1.3 1999-05-25 19:42:29 eaf Exp $# */
//@{

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

/** This is the base object for all hyperlinks. It defines the #URL#,
    #target#, #comment# and highlighting style.

    There are also some functions here (like \Ref{is_point_inside}()), which
    cannot be fully implemented in this class, but which define a single
    interface for all derived hyperlinks. */

class GHLObject
{
public:
   enum HLType { NONE=0, XOR=1, BORDER=2, SHADOW_IN=3, SHADOW_OUT=4,
		 SHADOW_EIN=5, SHADOW_EOUT=6 };
      /// The url, that the hyperlink points to
   GString	url;
      /** The target. Standard targets are:
	  \begin{itemize}
	     \item #_blank# - Load the link in a new blank window
	     \item #_self# - Load the link into the plugin window
	     \item #_top# - Load the link into the top-level frame
	  \end{itemize} */
   GString	target;
      /// Comment (displayed in a status line or as a popup hint)
   GString	comment;
      /** Highlighting style. One of the following:
	  \begin{itemize}
	     \item #NONE# - Invisible hyperlink
	     \item #XOR# - The hyperlink is highlighted by contour drawn
	           using XOR method.
	     \item #BORDER# - The hyperlink is highlighted by a regular contour
	     \item #SHADOW_IN# - Supported for \Ref{GHLRect} only. The hyperlink
	           area becomes "pushed-in" when highlighted
	     \item #SHADOW_OUT# - The opposite of #SHADOW_IN#
	     \item #SHADOW_EIN# - Also for \Ref{GHLRect} only. Is translated
	           as "shadow etched in"
	     \item #SHADOW_EOUT# - The opposite of #SHADOW_EIN#.
	  \end{itemize} */
   HLType	hltype;
      /// Border color (when relevant) in #0x00RRGGBB# format
   u_int32	hlcolor_rgb;
      /// Border thickness in pixels
   int		shadow_thick;

      /// Default constructor
   GHLObject(void);
   virtual ~GHLObject(void);

      /// Returns 1 if the given point is inside the hyperlink area
   bool		is_point_inside(int x, int y);

      /// Returns xmin of the bounding rectangle
   int		get_xmin(void);
      /// Returns ymin of the bounding rectangle
   int		get_ymin(void);
      /// Returns xmax of the bounding rectangle
   int		get_xmax(void);
      /// Returns ymax of the bounding rectangle
   int		get_ymax(void);
      /// Returns the hyperlink bounding rectangle
   GRect	get_bound_rect(void);
      /** Moves the hyperlink along the given vector. Is used by the
	  hyperlinks editor. */
   void		move(int dx, int dy);
      /** Resizes the hyperlink to fit new bounding rectangle while
	  keeping the (xmin, ymin) points at rest. */
   void		resize(int new_width, int new_height);
      /** Transforms the hyperlink to be within the specified rectangle */
   void		transform(const GRect & grect);
      /** Checks if the object is OK. Especially useful with \Ref{GHLPoly}
	  where edges may intersect. */
   bool		check_object(void);
      /** Stores the contents of the hyperlink object in a lisp-like format
	  for sving into #ANTa# chunk (see \Ref{DjVuAnno}) */
   GString	print(void);

      /// Virtual function returning the shape name.
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

/** Implements rectangular hyperlinks. This is the only kind of hyperlinks
    supporting #SHADOW_IN#, #SHADOW_OUT#, #SHADOW_EIN# and #SHADOW_EOUT#
    highlighting methods. */

class GHLRect: public virtual GHLObject, public GPEnabled
{
public:
      /// Returns the width of the rectangle
   int		get_width(void) const;
      /// Returns the height of the rectangle
   int		get_height(void) const;

   GHLRect(void);
   GHLRect(const GRect & rect);
   virtual ~GHLRect(void);

      /// Returns #"rect"#
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

/** Implements polygonal hyperlinks. The only supported styles of highlighting
    are #XOR# and #BORDER#. It's worth mentioning here that despite its
    name the polygon may be open, which basically makes it a broken line.
    This very specific mode is used by the hyperlink editor when creating
    the polygonal hyperlink. */

class GHLPoly : public virtual GHLObject, public GPEnabled
{
public:
   GHLPoly(void);
   GHLPoly(const int * xx, const int * yy, int points, bool open=0);
   virtual ~GHLPoly(void);

      /// Returns 1 if side #side# crosses the specified rectangle #rect#.
   bool		does_side_cross_rect(const GRect & grect, int side);

      /// Returns the number of vertices in the polygon
   int		get_points_num(void) const;

      /// Returns the number sides in the polygon
   int		get_sides_num(void) const;

      /// Returns x coordinate of vertex number #i#
   int		get_x(int i) const;
   
      /// Returns y coordinate of vertex number #i#
   int		get_y(int i) const;

      /// Moves vertex #i# to position (#x#, #y#)
   void		move_vertex(int i, int x, int y);

      /// Returns #"poly"# all the time
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

/** Implements elliptical hyperlinks. The only supported styles of highlighting
    are #XOR# and #BORDER#. */

class GHLOval: public virtual GHLObject, public GPEnabled
{
public:
   GHLOval(void);
   GHLOval(const GRect & rect);
   virtual ~GHLOval(void);

      /// Returns (xmax-xmin)/2
   int		get_a(void) const;
      /// Returns (ymax-ymin)/2
   int		get_b(void) const;
      /// Returns the lesser of \Ref{get_a}() and \Ref{get_b}()
   int		get_rmin(void) const;
      /// Returns the greater of \Ref{get_a}() and \Ref{get_b}()
   int		get_rmax(void) const;

      /// Returns #"oval"#
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

//@}

#endif
