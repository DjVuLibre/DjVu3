//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_doc_saver.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_QD_DOC_SAVER
#define HDR_QD_DOC_SAVER

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuDocument.h"
#include "DjVmDir.h"
#include "qd_port.h"
#include "qd_messenger.h"

#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qprogressdialog.h>

#include "qt_fix.h"

class QDDocSaver : public QObject
{
   Q_OBJECT
private:
   QProgressDialog	* progress_dialog;
   QWidget		* parent;

   GP<DjVuDocument>	doc;

      // These variables are passed from function to function during
      // actual saving procedure
   GArray<GUTF8String>      comp_ids;
   GPArray<DjVuFile>    comp_files;
   GTArray<bool>        comp_done;
   int			done_comps;
   bool			saveas_bundled;

   QDPort		port;

   void		preloadNextPage(void);
private slots:
      // Slots to receive translated requests from QDPort
   void		slotNotifyError(const GP<DjVuPort> &, const GUTF8String &);
   void		slotNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long);
public:
   void		save(void);

      // Note, that structure of DjVuDocument should already be known
   QDDocSaver(const GP<DjVuDocument> & doc, QWidget * parent);
   ~QDDocSaver(void);
};

inline
QDDocSaver::~QDDocSaver(void)
{
   delete progress_dialog; progress_dialog=0;
}

#endif
