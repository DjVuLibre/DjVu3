//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_tbar_nav_piece.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_TBAR_NAV_PIECE
#define HDR_QD_TBAR_NAV_PIECE

#ifdef __GNUC__
#pragma interface
#endif

#include "qd_toolbar.h"
#include "qt_fix.h"

class QDTBarNavPiece : public QDTBarPiece
{
   Q_OBJECT
private:
   bool		created;
   
   class QeComboBox	* page_menu;
   class QFrame		* separator;
   class QeLabel	* label;
   class QDToolButton	* npage_butt, * ppage_butt;
   class QDToolButton	* nnpage_butt, * pppage_butt;
   class QDToolButton	* fpage_butt, * lpage_butt;
   void		create(void);
   void		destroy(void);
   bool         qdtoolbar_child;   
private slots:
   void		slotPage(const QString &);
   void		slotPage(void);
protected:
signals:
   void		sigGotoPage(int page_num);
   void		sigDoCmd(int cmd);
public:
   virtual void	setEnabled(bool en);
   void		update(int page_num, int pages_num);
   
   QDTBarNavPiece(QWidget * toolbar);
};

#endif
