//C-  -*- C++ -*-
//C-
//C-  Copyright � 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_decoder.cpp,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_decoder.h"

#include "debug.h"
#include "exc_msg.h"
#include "throw_error.h"
#include "qlib.h"

#include <qapplication.h>

#include "qt_fix.h"

//***************************** Decoder callbacks **************************

void
QDDecoder::slotNotifyError(const GP<DjVuPort> &, const GUTF8String &qmsg)
{
   ::showError(0, "DjVu decoder error", QStringFromGString(qmsg));
   event.set();
}

void
QDDecoder::slotNotifyStatus(const GP<DjVuPort> &, const QString &qmsg)
{
   const char * const msg=qmsg;
   if (status_label)
   {
      GUTF8String text;
      for(const char * ptr=msg;*ptr && *ptr!='\n';ptr++)
      {
	 if (*ptr!='\t')
         {
           text+=*ptr;
	 }else
         {
           text+=' ';
         }
      }
      status_label->setText(QStringFromGString(text));
   }
   event.set();
}

void
QDDecoder::slotNotifyFileFlagsChanged(const GP<DjVuFile> &, long set_mask, long)
{
   if (set_mask & (DjVuFile::DECODE_OK | DjVuFile::DECODE_FAILED | DjVuFile::DECODE_STOPPED))
   {
      if (status_label) status_label->setText("");
      if (progress_bar) progress_bar->reset();
      if (progress_dlg) progress_dlg->setProgress(100);
      event.set();
   }
}

void
QDDecoder::slotNotifyDecodeProgress(const GP<DjVuPort> &, float done)
{
   if (progress_bar || progress_dlg)
      if ((int) (done*50)!=(int) (last_done*50))
      {
	 if (progress_bar) progress_bar->setProgress((int) (done*100));
	 if (progress_dlg) progress_dlg->setProgress((int) (done*100));
	 last_done=done;
      }
   event.set();
}

void
QDDecoder::waitTillDecodingEnds(void)
{
   DEBUG_MSG("QDDecoder::waitTillDecodingEnds(): blocking till decode is over\n");
   DEBUG_MAKE_INDENT(3);
   
   while(image->get_djvu_file()->is_decoding())
   {
      event.wait(100);
      qApp->processEvents(100);
      if (progress_dlg && progress_dlg->wasCancelled())
	 image->get_djvu_file()->stop_decode(true);
   }
   if (progress_bar) progress_bar->reset();
   if (progress_dlg) progress_dlg->setProgress(100);
}

GP<DjVuImage>
QDDecoder::getPageImage(int page_num)
{
   DEBUG_MSG("QDDecoder::getPageImage(): page_num=" << page_num << "\n");
   DEBUG_MAKE_INDENT(3);

   last_done=0;
   if (progress_dlg) progress_dlg->setProgress(0);

   image=document->get_page(page_num, false, port.getPort());
   waitTillDecodingEnds();

   if (image->get_djvu_file()->is_decode_ok()) return image;
   else return 0;
}

QDDecoder::QDDecoder(const GP<DjVuDocument> & doc) :
      port(true, true), document(doc)
{
   DEBUG_MSG("QDDecoder::QDDecoder(): initializing class...\n");
   DEBUG_MAKE_INDENT(3);
   
   progress_bar=0;
   progress_dlg=0;
   status_label=0;
   last_done=0;

      // Connect slots & signals
   connect(&port, SIGNAL(sigNotifyError(const GP<DjVuPort> &, const GUTF8String &)),
	   this, SLOT(slotNotifyError(const GP<DjVuPort> &, const GUTF8String &)));
   connect(&port, SIGNAL(sigNotifyStatus(const GP<DjVuPort> &, const QString &)),
	   this, SLOT(slotNotifyStatus(const GP<DjVuPort> &, const QString &)));
   connect(&port, SIGNAL(sigNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)),
	   this, SLOT(slotNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)));
   connect(&port, SIGNAL(sigNotifyDecodeProgress(const GP<DjVuPort> &, float)),
	   this, SLOT(slotNotifyDecodeProgress(const GP<DjVuPort> &, float)));
}
