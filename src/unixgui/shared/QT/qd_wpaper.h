//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_wpaper.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_WPAPER
#define HDR_QD_WPAPER

#ifdef __GNUC__
#pragma interface
#endif

#include <qwidget.h>

// QDWPaper: just "empty" widget displaying "DjVu" label as a wallpaper.

class QDWPaper : public QWidget
{
   Q_OBJECT
private:
   QString	text;

   virtual void	paintEvent(QPaintEvent * ev);
public:
   void		setText(const QString &txt) { text=txt; update(); };
   
   QDWPaper(QWidget * parent=0, const char * name=0);
   ~QDWPaper(void) {};
};

#endif
