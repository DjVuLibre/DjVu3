//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GRect.h,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $


#ifndef _GRECT_H_
#define _GRECT_H_

#ifdef __GNUC__
#pragma interface
#endif

/** @name GRect.h
    Files #"GRect.h"# and #"GRect.cpp"# implement basic operations on
    rectangles. Class \Ref{GRect} is used to represent rectangles.  Class
    \Ref{GRectMapper} represent the correspondance between points relative to
    given rectangles.  Class \Ref{GRatio} is used to represent scaling factors
    as rational numbers.
    @memo
    Rectangle manipulation class.
    @author
    Leon Bottou <leonb@research.att.com> -- initial implementation.
    @version
    #$Id: GRect.h,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $# */
//@{

#include "DjVuGlobal.h"

/** Rectangle class.  Each instance of this class represents a rectangle whose
    sides are parallel to the axis. Such a rectangle is composed of points
    whose coordinates lies between well defined minimal and maximal values.
    Member functions can combine several rectangles by computing the
    intersection of rectangles (\Ref{intersect}) or the smallest rectangle
    enclosing two rectangles (\Ref{recthull}).  */

class GRect 
{
public:
  /** Constructs an empty rectangle */
  GRect();
  /** Constructs a rectangle given its minimal coordinates #xmin# and #ymin#,
      and its measurements #width# and #height#. Setting #width# or #height# to zero
      produces an empty rectangle.  */
  GRect(int xmin, int ymin, unsigned int width=0, unsigned int height=0);
  /** Returns the rectangle width. */
  int  width() const;
  /** Returns the rectangle height. */
  int  height() const;
  /** Returns true iff the rectangle is empty. */
  int  isempty() const;
  /** Returns true iff the rectangle contains point (#x#,#y#). */
  int  contains(int x, int y) const;
  /** Returns true iff rectangles #r1# and #r2# are equal. */
  friend int operator==(const GRect & r1, const GRect & r2);
  /** Resets the rectangle to the empty rectangle */
  void clear();
  /** Fatten the rectangle. Both vertical sides of the rectangle are pushed
      apart by #dx# units. Both horizontal sides of the rectangle are pushed
      apart by #dy# units. Setting arguments #dx# (resp. #dy#) to a negative
      value reduces the rectangle horizontal (resp. vertical) size. */
  int  inflate(int dx, int dy);
  /** Translate the rectangle. The new rectangle is composed of all the points
      of the old rectangle translated by #dx# units horizontally and #dy#
      units vertically. */
  int  translate(int dx, int dy);
  /** Sets the rectangle to the intersection of rectangles #rect1# and #rect2#.
      This function returns true iff the intersection rectangle is not empty. */
  int  intersect(const GRect &rect1, const GRect &rect2);
  /** Sets the rectangle to the smallest rectangle containing the points of
      both rectangles #rect1# and #rect2#. This function returns true iff the
      intersection rectangle is not empty. */
  int  recthull(const GRect &rect1, const GRect &rect2);
  /** Minimal (inclusive) horizontal coordinate of the rectangle points. */
  int xmin;
  /** Minimal (inclusive) vertical coordinate of the rectangle points. */
  int ymin;
  /** Maximal (exclusive) horizontal coordinate of the rectangle points. */
  int xmax;
  /** Maximal (exclusive) vertical coordinate of the rectangle points. */
  int ymax;
};


/** Rational number.
    This is a minimal implementation of rational numbers.
    More support will be added if the need arises.
    Rational numbers are used to implement exact integer arithmetic 
    in the rectangle mapping code (See \Ref{GRectMapper}). 
*/

class GRatio
{
public:
  /** Constructs rational number #0#/#1#. */
  GRatio() : p(0), q(1) {};
  /** Constructs rational number #n#/#1#. This constructor provides an
      implicit conversion from integers to rational numbers. */
  GRatio(int n) : p(n), q(1) {};
  /** Constructs rational number #p#/#q#.  Argument #q# must be non zero. This
      constructor automatically divides both #p# and #q# by their greatest
      common divisor. */
  GRatio(int p, int q);
  /** Returns the numerator of the rational number. */
  int get_p() const { return p; };
  /** Returns the denominator of the rational number. */
  int get_q() const { return q; };
  /** Convert a rational into a floating point number. */
  operator double() const { return (double)p/(double)q; };
  /** Multiplies two rational numbers. */
  friend GRatio operator * (const GRatio &r1, const GRatio &r2);
  /** Adds two rational numbers. */
  friend GRatio operator + (const GRatio &r1, const GRatio &r2);
  /** Retrurns true iff #r1# is equal to #r2#. */
  friend int operator ==   (const GRatio &r1, const GRatio &r2);
  /** Retrurns true iff #r1# is equal to #r2#. */
  friend int operator !=   (const GRatio &r1, const GRatio &r2);
  /** Retrurns true iff #r1# is smaller or equal to #r2#. */
  friend int operator <=   (const GRatio &r1, const GRatio &r2);
private:
  // Members
  int p;
  int q;
  // Helpers
  void simplify();
};



/** Maps points from one rectangle to another rectangle.  This class
    represents a relation between the points of two rectangles. Given the
    coordinates of a point in the first rectangle (input rectangle), function
    \Ref{map} computes the coordinates of the corresponding point in the
    second rectangle (the ouput rectangle).  This function actually implements
    an affine transform which maps the corners of the first rectangle onto the
    matching corners of the second rectangle. The scaling operation is
    performed using integer fraction arithmetic in order to maximize
    acurracy. */
class GRectMapper 
{
public:
  /** Constructs a rectangle mapper. */
  GRectMapper();
  /** Resets the rectangle mapper state. Both the input rectangle
      and the output rectangle are marked as undefined. */
  void clear();
  /** Sets the input rectangle. */
  void set_input(const GRect &rect);
  /** Sets the output rectangle. */
  void set_output(const GRect &rect);
  /** Composes the affine tranform with a rotation of #count# quarter turns
      counter-clockwise.  This operation essentially is a modification of the
      match between the corners of the input rectangle and the corners of the
      output rectangle. */
  void rotate(int count=1);
  /** Composes the affine tranform with a symmetry with respect to the
      vertical line crossign the center of the output rectangle.  This
      operation essentially is a modification of the match between the corners
      of the input rectangle and the corners of the output rectangle. */
  void mirrorx();
  /** Composes the affine tranform with a symmetry with respect to the
      horizontal line crossign the center of the output rectangle.  This
      operation essentially is a modification of the match between the corners
      of the input rectangle and the corners of the output rectangle. */
  void mirrory();
  /** Maps a point according to the affine transform.  Variables #x# and #y#
      initially contain the coordinates of a point. This operation overwrites
      these variables with the coordinates of a second point located in the
      same position relative to the corners of the output rectangle as the
      first point relative to the matching corners of the input rectangle. */
  void map(int &x, int &y);
  /** Maps a rectangle according to the affine transform. This operation
      consists in mapping the rectangle corners and reordering the corners in
      the canonical rectangle representation.  Variable #rect# is overwritten
      with the new rectangle coordinates. */
  void map(GRect &rect);
  /** Maps a point according to the inverse of the affine transform.
      Variables #x# and #y# initially contain the coordinates of a point. This
      operation overwrites these variables with the coordinates of a second
      point located in the same position relative to the corners of input
      rectangle as the first point relative to the matching corners of the
      input rectangle. */
  void unmap(int &x, int &y);
  /** Maps a rectangle according to the inverse of the affine transform. This
      operation consists in mapping the rectangle corners and reordering the
      corners in the canonical rectangle representation.  Variable #rect# is
      overwritten with the new rectangle coordinates. */
  void unmap(GRect &rect);
private:
  // Data
  GRect rectFrom;
  GRect rectTo;
  int   code;
  // Helper
  void  precalc();
  GRatio rw;
  GRatio rh;
};


//@}



// ---- INLINES

inline
GRect::GRect()
: xmin(0), ymin(0), xmax(0), ymax(0)
{
}

inline 
GRect::GRect(int xmin, int ymin, unsigned int width, unsigned int height)
: xmin(xmin), ymin(ymin), xmax(xmin+width), ymax(ymin+height)
{
}

inline int 
GRect::width() const
{
  return xmax - xmin;
}

inline int 
GRect::height() const
{
  return ymax - ymin;
}

inline int 
GRect::isempty() const
{
  return (xmin>=xmax || ymin>=ymax);
}

inline int
GRect::contains(int x, int y) const
{
  return (x>=xmin && x<xmax && y>=ymin && y<ymax);
}
  
inline void 
GRect::clear()
{
  xmin = xmax = ymin = ymax = 0;
}



// ---- THE END
#endif
