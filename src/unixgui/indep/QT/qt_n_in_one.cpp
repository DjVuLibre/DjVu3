//C-  -*- C++ -*-
//C-
//C-  Copyright � 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qt_n_in_one.cpp,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#ifdef QT1
#include <qgmanager.h>
#else
#include <qabstractlayout.h>
#endif

#include "qt_n_in_one.h"
#include "qlib.h"
#ifndef QT1
#include <q1xcompatibility.h>
#define QGManager QLayout
#endif

void QeNInOne::recomputeMinMax(void)
{
   int min_w=0, min_h=0;
   int max_w=QGManager::unlimited, max_h=QGManager::unlimited;

   const QObjectList * objectList=children();
   if (objectList)
   {
      QObjectListIt it(*objectList);
      QObject * obj;
      while((obj=it.current()))
      {
	 ++it;
	 if (obj->isWidgetType())
	 {
	    QWidget * w=(QWidget *) obj;
	    QSize min=w->minimumSize();
	    QSize max=w->maximumSize();
	    if (min_w<min.width()) min_w=min.width();
	    if (min_h<min.height()) min_h=min.height();
	    if (max_w>max.width()) max_w=max.width();
	    if (max_h>max.height()) max_h=max.height();
	 }
      }
   }

   QSize min=minimumSize();
   QSize max=maximumSize();
   int done=0;
   if (min_w!=min.width()) { setMinimumWidth(min_w); done=1; }
   if (max_w!=max.width()) { setMaximumWidth(max_w); done=1; }
   if (min_h!=min.height()) { setMinimumHeight(min_h); done=1; }
   if (max_h!=max.height()) { setMaximumHeight(max_h); done=1; }
   if (done) ActivateLayouts(this);
}

QWidget	* QeNInOne::checkWidget(QWidget * w)
      // Check and return a legal widget, maybe different from w. If w==0 and
      // there is at least one child - it will be returned
{
   QWidget * last_w=0;
   const QObjectList * objectList=children();
   if (objectList)
   {
      QObjectListIt it(*objectList);
      QObject * obj;
      while((obj=it.current()))
      {
	 ++it;
	 if (obj->isWidgetType())
	 {
	    last_w=(QWidget *) obj;
	    if (last_w==w) return w;
	 }
      }
   }
   return last_w;
}

void QeNInOne::setActiveWidget(QWidget * new_w)
{
   if (new_w==activeWidget) return;

   const QObjectList * objectList=children();
   if (objectList)
   {
      QObjectListIt it(*objectList);
      QObject * obj;
      while((obj=it.current()))
      {
	 ++it;
	 if (obj->isWidgetType())
	 {
	    QWidget * w=(QWidget *) obj;
	    if (w!=new_w) w->hide();
	 }
      }
   }
   activeWidget=new_w;
   if (activeWidget)
   {
      activeWidget->resize(size());
      activeWidget->show();

      if (resizable && isVisible())
      {
	    // Otherwise min/max size is set when a child is added/removed
	 setMinimumSize(activeWidget->minimumSize());
	 setMaximumSize(activeWidget->maximumSize());
	 ActivateLayouts(this);
      }
   }
}

bool QeNInOne::event(QEvent * ev)
{
   try
   {
      if (ev->type()==Event_ChildRemoved) checkActiveWidget();
      else if (ev->type()==Event_ChildInserted)
      {
#ifdef QT1
	 QWidget * w_ins=((QChildEvent *) ev)->child();
#else
	 QWidget * w_ins=(QWidget *)((QChildEvent *) ev)->child();
#endif
	 w_ins->move(0, 0);

	 checkActiveWidget();
      } else if (ev->type()==Event_LayoutHint)
      {
	    // Looks like min/max dimensions of a child changed...
	 if (resizable && isVisible())
	 {
	    checkActiveWidget();
	    setMinimumSize(activeWidget->minimumSize());
	    setMaximumSize(activeWidget->maximumSize());
	 }
      }
		    
      if (!resizable)
      {
	    // If not resizable, it's necessary to recompute min/max
	    // every time when a child is added.
	    // Otherwise it's done every time a new active widget is chosen
	    // or when LayoutHint event is received.
	 recomputeMinMax();
      }
   } catch(const GException & exc)
   {
      showError(this, "Error", exc);
   }
   return QWidget::event(ev);
}

void QeNInOne::hide(void)
{
   if (resizable)
   {
      setFixedSize(QSize(1, 1));
      ActivateLayouts(this);
   }

   QWidget::hide();
}

void QeNInOne::show(void)
{
   if (resizable)
   {
      checkActiveWidget();
      if (activeWidget)
      {
	 setMinimumSize(activeWidget->minimumSize());
	 setMaximumSize(activeWidget->maximumSize());
      }
   }

   QWidget::show();
}