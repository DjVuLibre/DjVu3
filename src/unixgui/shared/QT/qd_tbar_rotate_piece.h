//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_tbar_rotate_piece.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_TBAR_ROTATE_PIECE
#define HDR_QD_TBAR_ROTATE_PIECE

#ifdef __GNUC__
#pragma interface
#endif

#include "qd_toolbar.h"

class QDTBarRotatePiece : public QDTBarPiece
{
   Q_OBJECT
private:
   class QDToolButton	* rotate90_butt;
   class QDToolButton	* rotate270_butt;
signals:
   void		sigRotate(int cmd_rotate);
private slots:
   void         slotRotate(void);
 
public:
   virtual void	setEnabled(bool en);
   
   QDTBarRotatePiece(QDToolBar * toolbar);
};

#endif
