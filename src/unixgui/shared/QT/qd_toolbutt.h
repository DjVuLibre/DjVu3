//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_toolbutt.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_TOOLBUTT
#define HDR_QD_TOOLBUTT

#ifdef __GNUC__
#pragma interface
#endif

#include "ByteStream.h"

#include <qtoolbutton.h>
#include <qiconset.h>

#include "qt_fix.h"

// Makes QToolButton accept icons in my format (PPM), also since PPM doesn't
// contain transparent background, it makes some pixels transparent at the
// run-time.

class QDToolButton : public QToolButton
{
   Q_OBJECT
private:
   bool		shadow;
   QIconSet	* set_off, * set_on, * set_armed;

   QIconSet *	createSet(ByteStream & str, int shadow_width);

private slots:
   void		slotToggled(bool);
   void		slotPressed(void);
   void		slotReleased(void);
protected:
   virtual void	enterEvent(QEvent * ev);
   virtual void	leaveEvent(QEvent * ev);
public:
   class QLabel	* status_bar;
   
   int		cmd;
   void		setOnPixmap(ByteStream & str);
   QDToolButton(ByteStream & str, bool shadow, int cmd,
		QWidget * parent=0, const char * name=0);
   ~QDToolButton(void);
};

#endif
