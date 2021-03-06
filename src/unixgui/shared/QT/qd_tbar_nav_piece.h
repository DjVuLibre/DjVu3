//C-  -*- C++ -*-
//C-
//C- DjVu� Unix Viewer (v. 3.5)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: qd_tbar_nav_piece.h,v 1.5 2001-10-17 19:09:17 docbill Exp $
// $Name:  $


#ifndef HDR_QD_TBAR_NAV_PIECE
#define HDR_QD_TBAR_NAV_PIECE

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
