//C-  -*- C++ -*-
//C-
//C- DjVu® Unix Viewer (v. 3.5)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: qx_pnote.h,v 1.3 2001-10-12 17:58:31 leonb Exp $
// $Name:  $

#ifndef HDR_QX_PNOTE
#define HDR_QX_PNOTE
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "qt_pnote.h"

#ifndef UNIX
#error "'qt_pnote.h' should be used only by UNIX version."
#endif

class QxPopupNote : public QePopupNote
{
   Q_OBJECT
private:
      // 'decor_win' is the window created by WM for the shell nearest
      // to the note's parerent (ref_widget)
      // 'common_pwin' is the parent of both the note and the decor_win. It's
      // likely to be ROOT window, but I don't want to check.
   u_long	decor_win, common_pwin;
   int		isShapeExtSupported(void);
public slots:
   void		gotX11Event(XEvent * event);
public:
   QxPopupNote(const GPopupNote & note, QWidget * parent, const char * name=0);
   virtual ~QxPopupNote(void);
};

#endif
