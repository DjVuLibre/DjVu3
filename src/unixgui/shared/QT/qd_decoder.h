//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_decoder.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $

 
#ifndef HDR_QD_DECODER
#define HDR_QD_DECODER

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuDocument.h"
#include "qd_port.h"

#include <qprogressbar.h>
#include <qprogressdialog.h>
#include <qlabel.h>

#include "qt_fix.h"

// QDDecoder: class for decoding DjVuDocuments in the QT environment.
// Blocks execution until the image is completely ready.
// May display progress and status info in externally supplied
// progress bar and status label

class QDDecoder : public QObject
{
   Q_OBJECT
private:
   QProgressBar		* progress_bar;
   QProgressDialog	* progress_dlg;
   QLabel		* status_label;

   QDPort		port;
   GEvent		event;

   GP<DjVuDocument>	document;
   GP<DjVuImage>	image;
   float		last_done;

   void		waitTillDecodingEnds(void);
private slots:
      // Slots to receive translated requests from QDPort
   void		slotNotifyError(const GP<DjVuPort> & source, const GUTF8String &msg);
   void		slotNotifyStatus(const GP<DjVuPort> & source, const QString &msg);
   void		slotNotifyFileFlagsChanged(const GP<DjVuFile> & source,
					   long set_mask, long clr_mask);
   void		slotNotifyDecodeProgress(const GP<DjVuPort> & source, float done);
public:
   GP<DjVuImage>getPageImage(int page_num);

   void		setProgressBar(QProgressBar * progress_bar);
   void		setProgressDialog(QProgressDialog * progress_dlg);
   void		setStatusLabel(QLabel * status_label);
   
   QDDecoder(const GP<DjVuDocument> & doc);
   virtual ~QDDecoder(void) {};
};

inline void
QDDecoder::setProgressBar(QProgressBar * _progress_bar)
{
   progress_bar=_progress_bar;
   progress_bar->setTotalSteps(100);
}

inline void
QDDecoder::setProgressDialog(QProgressDialog * _progress_dlg)
{
   progress_dlg=_progress_dlg;
   progress_dlg->setTotalSteps(100);
}

inline void
QDDecoder::setStatusLabel(QLabel * _status_label)
{
   status_label=_status_label;
}

#endif
 
