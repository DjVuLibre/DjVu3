//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_thumb_gen.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_THUMB_GEN
#define HDR_QD_THUMB_GEN

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuDocEditor.h"

#include <qdialog.h>

#include "qt_fix.h"

// Dialog for entering the exact zoom factor to use
class QDThumbGen : public QeDialog
{
   Q_OBJECT
private:
   GP<DjVuDocEditor>	doc;
   class QeSpinBox	* spin;
   class QProgressDialog* progress_dialog;

   bool			progress_cb(int page_num);
protected:
   virtual void		done(int rc);
signals:
   void		sigProgress(int page_num);
   void		sigDocModified(void);
public:
   
   QDThumbGen(GP<DjVuDocEditor> & doc, QWidget * parent=0, const char * name=0);
   virtual ~QDThumbGen(void) {};
};

#endif
