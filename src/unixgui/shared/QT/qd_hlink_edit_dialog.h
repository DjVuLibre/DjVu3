//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_hlink_edit_dialog.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_HLINK_EDIT_DIALOG
#define HDR_QD_HLINK_EDIT_DIALOG

#ifdef __GNUC__
#pragma interface
#endif

#include "MapAreas.h"
#include "DjVuDocEditor.h"

#include <qdialog.h>
#include <qframe.h>
#include <qcombobox.h>

#include "qt_fix.h"

class QeHLPane : public QFrame
{
   Q_OBJECT
private:
   GP<DjVuImage>dimg;
   GP<GMapArea>	gmap_area;
   GP<MapArea>	map_area;
   GRectMapper	mapper;
   float	gamma;

   void		layout(void);
protected:
   virtual void	paintEvent(QPaintEvent * ev);
   virtual void	resizeEvent(QResizeEvent *);
   virtual bool	event(QEvent *);
public:
   void		mapAreaChanged(void);
   
   QeHLPane(const GP<DjVuImage> & dimg, const GP<GMapArea> & map,
	    float gamma, QWidget * parent=0, const char * name=0);
   ~QeHLPane(void) {}
};

// Dialog used to create 'MapAreas': hyperlinks or just highlighted areas

class QDHLinkEditDialog : public QeDialog
{
   Q_OBJECT
private:
   QString	other_target;
   int		last_target_index;

   class QeHLPane	* preview_pane;

   class QeComboBox	* link_menu;
   class QeLabel	* page_label;
   class QeComboBox	* page_menu;
   class QeLabel	* name_label;
   class QeComboBox	* name_menu;
   class QeLabel	* url_label;
   class QLineEdit	* url_text;
   class QMultiLineEdit	* comment_text;
   class QeComboBox	* target_menu;
   class QLineEdit	* target_text;
   
   class QeComboBox	* btype_menu;
   class QeLabel	* bwidth_label, * bcolor_label;
   class QeNInOne	* bcolor_form, * bwidth_form;
   class QWidget	* bcolor_w, * border_w;
   class QeColorMenu	* bcolor_menu;
   class QeSpinBox	* bwidth_spin;
   class QeCheckBox	* bvisible_butt;
   class QeColorMenu	* hilite_menu;

   GP<DjVuDocEditor>	doc;
   GP<DjVuImage>	dimg;
   GP<GMapArea>		gmap_area;

   QeComboBox *	getBTypeMenu(QWidget * parent);
private slots:
   void		slotTargetChanged(int);
   void		slotBorderTypeChanged(const QString &);
   void		slotLinkTypeChanged(const QString &);
   void		slotPageChanged(int);
   void		slotNameChanged(int);
   void		slotBorderColorChanged(int);
   void		slotBorderWidthChanged(int);
   void		slotHiliteColorChanged(int);
protected:
   virtual void	done(int);
public:
   QString	url(void) const;
   QString	comment(void) const;
   QString	target(void) const;
   GMapArea::BorderType	borderType(void) const;
   bool		borderAlwaysVisible(void) const;
   int		borderWidth(void) const;
   u_int32	borderColor(void) const;
   u_int32	hiliteColor(void) const;
   QDHLinkEditDialog(GP<DjVuDocEditor> & doc,
		     const GP<DjVuImage> & dimg,
		     const GP<GMapArea> & map,
		     float gamma, QWidget * parent=0,
		     const char * name=0, bool modal=FALSE);
   ~QDHLinkEditDialog(void) {};
};

#endif
