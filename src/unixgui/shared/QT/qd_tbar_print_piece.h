//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_tbar_print_piece.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_TBAR_PRINT_PIECE
#define HDR_QD_TBAR_PRINT_PIECE

#ifdef __GNUC__
#pragma interface
#endif

#include "qd_toolbar.h"

class QDTBarPrintPiece : public QDTBarPiece
{
   Q_OBJECT
private:
   class QDToolButton	* print_butt;
   class QDToolButton	* find_butt;
   class QDToolButton	* save_butt;
signals:
   void		sigPrint(void);
   void		sigFind(void);
   void		sigSave(void);
public:
   virtual void	setEnabled(bool en);
   
   QDTBarPrintPiece(QDToolBar * toolbar);
};

#endif
