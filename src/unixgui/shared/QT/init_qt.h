//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: init_qt.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_INIT_QT
#define HDR_INIT_QT

#ifdef __GNUC__
#pragma interface
#endif

// QT wants qapplication be included before Xlib.h to avoid redefinition
// of TrueColor define.

#include <qapplication.h>

#include "qt_fix.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

extern Display		* displ;
extern Visual		* visual;
extern Colormap		colormap;
extern int		depth;

void InitializeQT(int argc, char ** argv);
void InitStandalone(int argc, char ** argv);
void ParseQTArgs(int & argc, char ** argv);

#endif
