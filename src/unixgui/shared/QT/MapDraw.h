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
// $Id: MapDraw.h,v 1.4 2001-10-12 17:58:31 leonb Exp $
// $Name:  $

#ifndef HDR_MAPDRAW
#define HDR_MAPDRAW
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "GPixmap.h"
#include "GMapAreas.h"
#include "int_types.h"

class MapDraw
{
private:
   struct OvalPixel
   {
      GPixmap	* pm;
      int	x0, y0;
      int	x, y;
      bool	eor;
      u_char	r, g, b;
   };
   static void	drawOvalPixel(OvalPixel & pix);
public:
   static void	drawPixel(GPixmap & pm, int x, int y,
			  bool eor, u_char r, u_char g, u_char b);
   static void	drawLine1(GPixmap & pix, int x1, int y1, int x2, int y2, u_int32 color);
   static void	drawLine(GPixmap & pix, int x1, int y1, int x2, int y2, u_int32 color);
   static void	drawRect(GPixmap & pix, const GRect & grect, u_int32 color);
   static void	drawOval(GPixmap & pix, const GRect & grect, u_int32 color);
};

#endif
