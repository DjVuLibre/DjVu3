//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qxlib.h,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifndef HDR_QXLIB
#define HDR_QXLIB

#ifdef __GNUC__
#pragma interface
#endif

#include <qwidget.h>
#include <qrect.h>

#include <sys/types.h>

// Sends Expose event thru X11 system. Helps to take advantage of QT
// event compression, which works only for X11 events.
void	x11Redraw(QWidget * w, const QRect * grect=0);

// Actual prorotype is 'Window x11GetTopLevelWindow(Display * displ, Window start)'
u_long	x11GetTopLevelWindow(void * displ, u_long start);

#endif
