//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qx_pnote.h,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifndef HDR_QX_PNOTE
#define HDR_QX_PNOTE

#ifdef __GNUC__
#pragma interface
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
