//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_zoom_dialog.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_ZOOM_DIALOG
#define HDR_QD_ZOOM_DIALOG

#ifdef __GNUC__
#pragma interface
#endif

#include <qdialog.h>
#include <qcombobox.h>

#include "qt_fix.h"

// Use this dialog to set the default zoom for a DjVu page.

class QDZComboBox : public QeComboBox
{
   Q_OBJECT
protected:
   virtual void keyPressEvent(QKeyEvent * ev);
public:
   QDZComboBox(bool rw, QWidget * parent=0, const char * name=0) :
	 QeComboBox(rw, parent, name) {};
};

class QDZoomDialog : public QeDialog
{
   Q_OBJECT
private:
   QeComboBox	* menu;
protected:
   virtual void	done(int);
public:
   int		getZoom(void) const;

      // Zoom is in the DjVuAnno format
   QDZoomDialog(int zoom, QWidget * parent=0,
		const char * name=0, bool modal=FALSE);
   ~QDZoomDialog(void) {};
};

#endif
