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
// $Id: qt_pnote.h,v 1.5 2001-10-17 19:02:05 docbill Exp $
// $Name:  $


#ifndef HDR_QT_PNOTE
#define HDR_QT_PNOTE

#include "GPopupNote.h"

#include <qdialog.h>
#include <qlabel.h>

#include "qt_fix.h"

// This class provides the "Bubbling" functionality of the popup note
// It will take care of the bubble-like shape of the window, but there is
// still smth else to be done like:
//    1. Get rid of WM decorations (toolbar, which survives XShape)
//    2. Move the note with the parent
//    3. Take care of the proper stacking order
//
// All the mentioned stuff is done in QxPopupNote (for X11) There should
// be a separate file for windows as well.
//
// The QePopupNote is self-destructible, that is you don't have to destroy
// it manually: it will die when its parent dies.

class QePopupNote : public QDialog, public GPopupNote
{
   Q_OBJECT
private:
      // 'contents' is the widget containing just QMultiLineEdit and button
      // There may be some space below it left for bubbles.
   QWidget	* contents;
   QeLabel	* text;

   int		shape_ext_supported;

   void		getAnchorPos(int * x, int * y);
   
   void		getBubbleCoords(int bubble, int * x, int * y, int * w, int * h);
   int		getBottomMargin(void);
   void		setBubbleMask(int bubble);
protected:
      // 'ref_widget' is the 'parent' passed to the constructor. In fact,
      // it's not a parent at all. Just smth to use as a reference.
      // You may want to use ref_pos/ref_size/ref_widget as I do it when
      // I catch ConfigureNotify events directed to the ref_widget's shell
      // ref_pos and ref_size help to reposition the note properly.
   QPoint		ref_pos;
   QSize		ref_size;
   QWidget		* ref_widget;

   void		moveNote(int x, int y);	// Relative to "parent"
signals:
   void		sigClosed(void);
public:
   virtual void	done(int rc)
   {
      emit sigClosed();
      QDialog::done(rc);
   };
   
   void		showNote(int x, int y);	// Relative to "parent"

      // Parent below may not be zero. It will be used as a reference
      // widget to know where to position the note
   QePopupNote(int shape_ext_supported, const GPopupNote & note,
	       QWidget * parent, const char * name=0);
   virtual ~QePopupNote(void) {};
};

#endif
