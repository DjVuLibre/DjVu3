//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qt_imager.h,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifndef HDR_QT_IMAGER
#define HDR_QT_IMAGER

#ifdef __GNUC__
#pragma interface
#endif

#include "int_types.h"

#include <qcolor.h>
#include <qpixmap.h>

// The purpose of this class is to provide standard interface to Imagers
// doing the real job: like QXImager for X11.

// The point is to use this class (include this header) whenever possible
// and deal with QXImager (or QWImager) only when necessary (e.g. when
// initializing)

class QeImager
{
public:
   virtual u_int32	getGrayXColor(float level)=0;
   virtual u_int32	getXColor(u_char r, u_char g, u_char b)=0;
   virtual u_int32	getXColor(u_int32 color)=0;
   virtual u_int32	getXColor(const char *name)=0;

   virtual QColor	getGrayColor(float level)=0;
   virtual QColor	getColor(u_char r, u_char g, u_char b)=0;
   virtual QColor	getColor(u_int32 color)=0;
   virtual QColor	getColor(const char *name)=0;
   virtual QColor	getColor(const QColor & col)=0;

      // Creates a pixmap of the given size, fills it with the given
      // color, dithers according to the depth and returns it.
   virtual QPixmap	getColorPixmap(int width, int height,
				       u_char r, u_char g, u_char b)=0;
   virtual QPixmap	getColorPixmap(int width, int height,
				       u_int32 color)=0;

   QeImager(void);
   virtual ~QeImager(void) {};
};

extern QeImager	* qeImager;

#endif
