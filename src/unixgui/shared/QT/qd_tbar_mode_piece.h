//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_tbar_mode_piece.h,v 1.2 2001-06-20 18:15:17 mchen Exp $
// $Name:  $


#ifndef HDR_QD_TBAR_MODE_PIECE
#define HDR_QD_TBAR_MODE_PIECE

#ifdef __GNUC__
#pragma interface
#endif

#include "qd_toolbar.h"
#include "qt_fix.h"

class QDTBarModePiece : public QDTBarPiece
{
   Q_OBJECT
private:
   class QeComboBox	* zoom_menu, * mode_menu;
   class QDToolButton	* zoom_in_butt, * zoom_out_butt, * pane_butt;
   class QDToolButton	* zoom_select_butt, * text_select_butt, * pin_butt;
   bool         qdtoolbar_child;
private slots:
   void		slotZoom(const QString &);
   void		slotZoom(void);
   void		slotMode(int);
   void		slotPaneMode(void);
signals:
   void		sigSetMode(int cmd_mode);
   void		sigSetZoom(int cmd_mode);
   void		sigStick(bool on);
   void		sigSetPaneMode(int cmd_mode);
public:
   virtual void	setEnabled(bool en);
   void		stick(bool en);
   bool		isStuck(void) const;
   void		update(int cmd_mode, bool mode_enabled,
		       int cmd_zoom, int zoom, int pane_mode);
   
   QDTBarModePiece(QWidget * toolbar);
};

#endif
