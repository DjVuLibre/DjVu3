//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_toolbar.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_TOOLBAR
#define HDR_QD_TOOLBAR

#ifdef __GNUC__
#pragma interface
#endif

#include "GContainer.h"

#include <qframe.h>

#include "qt_fix.h"

class QDToolBar : public QFrame
{
   Q_OBJECT
private:
   GList<QWidget *>		left_list, right_list;
   GList<class QDTBarPiece *>	pieces;

   bool		positionWidgets(int width, int rows, bool move=false, int * height_ptr=0);
   int		positionWidgets(void);
   void		addLeftWidgets(const GList<QWidget *> & list);
private slots:
   void		slotWidgetDestroyed(void);
protected:
   virtual bool	event(QEvent * ev);
public:
   bool		being_destroyed;
   
   int		computeHeight(int width);
   void		addLeftWidget(QWidget * widget);
   void		addLeftWidgets(QWidget * w1, QWidget * w2, QWidget * w3=0,
			       QWidget * w4=0, QWidget * w5=0, QWidget * w6=0);
   void		addRightWidget(QWidget * widget);
   void		deleteWidget(QWidget * widget);
   void		adjustPositions(void);
   void		addPiece(QDTBarPiece * piece);
   
   virtual void	setEnabled(bool en);
   
   QDToolBar(QWidget * parent=0, const char * name=0);
   ~QDToolBar(void) { being_destroyed=true; }
};

class QDTBarPiece : public QObject
{
   Q_OBJECT
protected:
   QWidget	* toolbar;
public:
   virtual void	setEnabled(bool on)=0;

   QDTBarPiece(QWidget * _toolbar) :
	 QObject(_toolbar), toolbar(_toolbar) {}
};

#endif
