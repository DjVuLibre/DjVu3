//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_pnote_editor.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_PNOTE_EDITOR
#define HDR_QD_PNOTE_EDITOR

#ifdef __GNUC__
#pragma interface
#endif

#include "qt_color_menu.h"
#include "GPopupNote.h"
#include "qt_pnote.h"

#include <qdialog.h>
#include <qpushbutton.h>
#include <qmultilinedit.h>

#include "qt_fix.h"

// Class allowing you to compose a popup note, set its background/foreground
// and preview the sample.

class QDPNoteEditor : public QeDialog, public GPopupNote
{
   Q_OBJECT
private:
   QeColorMenu	* back_menu, * fore_menu;
   QMultiLineEdit	* text;
   QePushButton		* prv_butt;

   QePopupNote	* pnote_shell;
private slots:
   void		slotShowPreview(void);
   void		slotClosePreview(void);
public:
   GPopupNote	getNote(void) const;

      // This dialog can't be made modal because otherwise it would be
      // impossible to close the preview shell
   QDPNoteEditor(const GPopupNote & note, QWidget * parent=0, const char * name=0);
   ~QDPNoteEditor(void);
};

#endif
