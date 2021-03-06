//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.5)
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
// 
// $Id: GRect.h,v 1.32 2001-10-17 18:56:48 docbill Exp $
// $Name:  $

#ifndef _GRECT_H_
#define _GRECT_H_

#include "DjVuGlobal.h"

/** @name GRect.h
    Files #"GRect.h"# and #"GRect.cpp"# implement basic operations on
    rectangles. Class \Ref{GRect} is used to represent rectangles.  Class
    \Ref{GRectMapper} represent the correspondence between points relative to
    given rectangles.  Class \Ref{GRatio} is used to represent scaling factors
    as rational numbers.
    @memo
    Rectangle manipulation class.
    @author
    L\'eon Bottou <leonb@research.att.com> -- initial implementation.
    @version
    #$Id: GRect.h,v 1.32 2001-10-17 18:56:48 docbill Exp $# */
//@{



/** @name Point Coordinates vs. Pixel Coordinates

    The DjVu technology relies on the accurate superposition of images at
    different resolutions.  Such an accuracy cannot be reached with the usual
    assumption that pixels are small enough to be considered infinitesimally
    small.  We must distinguish very precisely ``points'' and ``pixels''.
    This distinction is essential for performing scaling operations.

    The pixels of an image are identified by ``pixel coordinates''.  The
    bottom-left corner pixel has coordinates #(0,0)# and the top-right corner
    pixel has coordinates #(w-1,h-1)# where #w# and #h# are the image size.
    Pixel coordinates are necessarily integers since pixels never overlap.

    An infinitesimally small point is identified by its ``point coordinates''.
    There may be fractional point coordinates, although this library does not
    make use of them.  Points with integer coordinates are located {\em on the
    corners of each pixel}.  They are not located on the pixel centers.  The
    center of the pixel with pixel coordinates #(i,j)# is located at point
    coordinates #(i+1/2,j+1/2)#.  In other words, the pixel #(i,j)# extends
    from point #(i,j)# to point #(i+1,j+1)#.

    Therefore, the point located on the bottom left corner of an image has
    coordinates #(0,0)#.  This point is in fact the bottom left corner of the
    bottom left pixel of the image.  The point located on the top right corner
    of an image has coordinates #(w,h)# where #w# and #h# are the image size.
    This is in fact the top right corner of pixel #(w-1,h-1)# which is the
    image pixel with the highest coordinates.
*/
//@{
//@}



/** Rectangle class.  Each instance of this class represents a rectangle whose
    sides are parallel to the axis. Such a rectangle represents all the points
    whose coordinates lies between well defined minimal and maximal values.
    Member functions can combine several rectangles by computing the
    intersection of rectangles (\Ref{intersect}) or the smallest rectangle
    enclosing two rectangles (\Ref{recthull}).  */

class GRect 
{
public:
  /** #OrientationBits# defines 3 mutually exclusive
     bits to indicate the image orientation.

     There are four possible rotation values for an image
     which are 0 degrees, 90 degrees, 180 degrees, and 270 degrees.
     In addition the image can be mirrored backwards in any of these
     orientations, giving a possible of 8 orientations.  To sanely deal
     with these orientations, we have defined 3 mutually exclusive
     bits.  These are BOTTOM_UP, MIRROR, and ROTATE90_CW.
  */
  enum OrientationBits
  {
    BOTTOM_UP=0x1,  /* Upside down */
    MIRROR=0x2,     /* Written backwards. (right to left) */
    ROTATE90_CW=0x4 /* rotated 90 degrees */
  };

  /**  #Orientations# defines all 8 possible orientations, using
   the three \Ref{OrientationBits}.
   \begin{itemize}
   \item {\em TDLRNR} for Top Down,  Left to Right, No Rotation.
   \item {\em BULRNR} for Bottom Up, Left to Right, No Rotation.
   \item {\em TDRLNR} for Top Down,  Right to Left, No Rotation.
   \item {\em BURLNR} for Bottom Up, Right to Left, No Rotation.
   \item {\em TDLRCW} for Top Down,  Left to Right, 90 degree CW rotation.
   \item {\em BULRCW} for Bottom Up, Left to Right, 90 degree CW rotation.
   \item {\em TDRLCW} for Top Down,  Right to Left, 90 degree CW rotation.
   \item {\em BURLCW} for Bottom Up, Right to Left, 90 degree CW rotation.
   \end{itemize}
  */
  enum Orientations
  {
    TDLRNR=0,                             /* normal orientation */
    BULRNR=BOTTOM_UP,                     /* upside down */
    TDRLNR=MIRROR,                        /* backwards (right to left) */
    BURLNR=MIRROR|BOTTOM_UP,              /* rotate 180 */
    TDLRCW=ROTATE90_CW,                   /* rotated 90 */
    BULRCW=ROTATE90_CW|BOTTOM_UP,         /* backwards and rotate 180 */
    TDRLCW=ROTATE90_CW|MIRROR,            /* backwards and rotate 90 */
    BURLCW=ROTATE90_CW|MIRROR|BOTTOM_UP   /* rotate 270 */
  };

  // Rotate the given orientation clockwise by the angle given (in degrees).
  // Actually rotate by the multiple of 90 degrees that is closest to the angle.
  static Orientations
  rotate(const int angle,Orientations orientation)
  {
    for(int a=(((angle)%360)+405)%360 ; a>90 ; a-=90)
      orientation=(Orientations)( (int)orientation ^ ( (int)(orientation&ROTATE90_CW)?BURLCW:TDLRCW ) );
              // Advance by 90 deg. as many times as is necessary.
              // An examination of all eight cases shows that:
              // --if the initial orientation does not have a 90 deg. rotation
              //   (orientation & ROTATE90_CW == 0) then set the ROTATE90_CW bit,
              //   which is equivalent to XOR with TDLRCW.
              // --if the initial orientation does have a 90 deg. rotation
              //   (orientation & ROTATE90_CW != 0) then invert all the bits,
              //   which is equivalent to XOR with BURLCW.
    return orientation;
  }

  // Determine the angle of rotation of the given orientation.
  // Proceeds by rotating each of the standard orientations by multiples
  // of 90 degrees until it finds a match with the supplied orientation.
  static int
  findangle(const Orientations orientation)
  {
    int a=270;
    while(a&&(rotate(a,BULRNR)!=orientation)&&(rotate(a,TDLRNR)!=orientation))
      a-=90;
    return a;
  }

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
  /** Returns the area of the rectangle. */
  int  area() const;
  /** Returns true if the rectangle is empty. */
  int  isempty() const;
  /** Returns true if the rectangle contains pixel (#x#,#y#).  A rectangle
      contains all pixels with horizontal pixel coordinates in range #xmin#
      (inclusive) to #xmax# (exclusive) and vertical coordinates #ymin#
      (inclusive) to #ymax# (exclusive). */
  int  contains(int x, int y) const;
  /** Returns true if this rectangle contains the passed rectangle #rect#.
      The function basically checks, that the intersection of this rectangle
      with #rect# is #rect#. */
  int  contains(const GRect & rect) const;
  /** Returns true if rectangles #r1# and #r2# are equal. */
  friend int operator==(const GRect & r1, const GRect & r2);
  /** Returns true if rectangles #r1# and #r2# are not equal. */
  friend int operator!=(const GRect & r1, const GRect & r2);
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
      This function returns true if the intersection rectangle is not empty. */
  int  intersect(const GRect &rect1, const GRect &rect2);
  /** Sets the rectangle to the smallest rectangle containing the points of
      both rectangles #rect1# and #rect2#. This function returns true if the
      created rectangle is not empty. */
  int  recthull(const GRect &rect1, const GRect &rect2);
  /** Multiplies xmin, ymin, xmax, ymax by factor and scales the rectangle*/
  void scale(float factor);
  /** Multiplies xmin, xmax by xfactor and ymin, ymax by yfactor and scales the rectangle*/
  void scale(float xfactor, float yfactor);
  /** Minimal horizontal point coordinate of the rectangle. */
  int xmin;
  /** Minimal vertical point coordinate of the rectangle. */
  int ymin;
  /** Maximal horizontal point coordinate of the rectangle. */
  int xmax;
  /** Maximal vertical point coordinate of the rectangle. */
  int ymax;
};


/** Maps points from one rectangle to another rectangle.  This class
    represents a relation between the points of two rectangles. Given the
    coordinates of a point in the first rectangle (input rectangle), function
    \Ref{map} computes the coordinates of the corresponding point in the
    second rectangle (the output rectangle).  This function actually implements
    an affine transform which maps the corners of the first rectangle onto the
    matching corners of the second rectangle. The scaling operation is
    performed using integer fraction arithmetic in order to maximize
    accuracy. */
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
  /** Returns the input rectangle. */
  GRect get_input();
  /** Sets the output rectangle. */
  void set_output(const GRect &rect);
  /** Returns the output rectangle. */
  GRect get_output();
  /** Composes the affine transform with a rotation of #count# quarter turns
      counter-clockwise.  This operation essentially is a modification of the
      match between the corners of the input rectangle and the corners of the
      output rectangle. */
  void rotate(int count=1);
  /** Composes the affine transform with a symmetry with respect to the
      vertical line crossing the center of the output rectangle.  This
      operation essentially is a modification of the match between the corners
      of the input rectangle and the corners of the output rectangle. */
  void mirrorx();
  /** Composes the affine transform with a symmetry with respect to the
      horizontal line crossing the center of the output rectangle.  This
      operation essentially is a modification of the match between the corners
      of the input rectangle and the corners of the output rectangle. */
  void mirrory();
  /** Maps a point according to the affine transform.  Variables #x# and #y#
      initially contain the coordinates of a point. This operation overwrites
      these variables with the coordinates of a second point located in the
      same position relative to the corners of the output rectangle as the
      first point relative to the matching corners of the input rectangle.
      Coordinates are rounded to the nearest integer. */
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
      input rectangle. Coordinates are rounded to the nearest integer. */
  void unmap(int &x, int &y);
  /** Maps a rectangle according to the inverse of the affine transform. This
      operation consists in mapping the rectangle corners and reordering the
      corners in the canonical rectangle representation.  Variable #rect# is
      overwritten with the new rectangle coordinates. */
  void unmap(GRect &rect);
private:
  // GRatio
  struct GRatio {
    GRatio ();
    GRatio (int p, int q);
    int p;
    int q;
  };
  // Data
  GRect rectFrom;
  GRect rectTo;
  int   code;
  // Helper
  void  precalc();
  friend int operator*(int n, GRatio r ) { return (int)(((double)n * (double)r.p + (double)r.q/2) / (double)r.q); };
  friend int operator/(int n, GRatio r ) { return (int)(((double)n * (double)r.q + (double)r.p/2) / (double)r.p); };
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
GRect::area() const
{
  return isempty() ? 0 : (xmax-xmin)*(ymax-ymin);
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

inline int
operator!=(const GRect & r1, const GRect & r2)
{
   return !(r1==r2);
}

// ---- THE END
#endif
