//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: MapDraw.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_MAPDRAW
#define HDR_MAPDRAW

#ifdef __GNUC__
#pragma interface
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
      bool	xor;
      u_char	r, g, b;
   };
   static void	drawOvalPixel(OvalPixel & pix);
public:
   static void	drawPixel(GPixmap & pm, int x, int y,
			  bool xor, u_char r, u_char g, u_char b);
   static void	drawLine1(GPixmap & pix, int x1, int y1, int x2, int y2, u_int32 color);
   static void	drawLine(GPixmap & pix, int x1, int y1, int x2, int y2, u_int32 color);
   static void	drawRect(GPixmap & pix, const GRect & grect, u_int32 color);
   static void	drawOval(GPixmap & pix, const GRect & grect, u_int32 color);
};

#endif
