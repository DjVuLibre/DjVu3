//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: mime_check.cpp,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "prefs.h"
#include "mime_check.h"
#include "mime_utils.h"
#include "GString.h"
#include "ByteStream.h"
#include "qd_mime_dialog.h"
#include "GURL.h"

#include <stdlib.h>
#include <unistd.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include "debug.h"

class QMimeChecker : public QObject
{
   Q_OBJECT
public slots:
   void	slotCheckMimeTypes(void);
};

void
QMimeChecker::slotCheckMimeTypes(void)
{
   DEBUG_MSG("QMimeChecker::slotCheckMimeTypes\n");
   DjVuPrefs prefs;
   if (prefs.mimeDontCheck) return;
   
   G_TRY {
      GUTF8String ptr=getenv("HOME");
      if (ptr.length())
      {
	 GUTF8String name_in=ptr+"/.mime.types";
	 GUTF8String name_out=name_in+".new";
	 bool fixed=fixMimeTypes(name_in, name_out);

	 if (fixed)
	 {
	    bool do_it=false;
	    if (prefs.mimeDontAsk) do_it=true;
	    else
	    {
	       QDMimeDialog d(QStringFromGString(name_in), 0, "mime_check_dialog", TRUE);
	       if (d.exec()==QDialog::Accepted)
	       {
		  do_it=true;
		  QMessageBox::information(0, "DjVu",
					   "Please restart Netscape for the changes to take effect");
	       }
	       prefs.mimeDontAsk=d.dontAsk();
	       prefs.mimeDontCheck=d.dontCheck();
	       prefs.save();
	    }
	    if (do_it)
	    {
	       unlink(name_in);
	       GP<ByteStream> str_in=ByteStream::create(GURL::Filename::UTF8(name_out),"r");
	       GP<ByteStream> str_out=ByteStream::create(GURL::Filename::UTF8(name_in),"w");
	       str_out->copy(*str_in);
	       unlink(name_out);
	    } else unlink(name_out);
	 }
      }
   } G_CATCH(exc) {} G_ENDCATCH;
}

void
checkMimeTypes(void)
{
   DEBUG_MSG("checkMimeTypes\n");
   static QTimer * timer;
   static QMimeChecker * checker;

   DjVuPrefs prefs;
   if (prefs.mimeDontCheck) return;
   
   if (!timer) timer=new QTimer();
   if (!checker) checker=new QMimeChecker();
   QObject::connect(timer, SIGNAL(timeout(void)), checker, SLOT(slotCheckMimeTypes(void)));

   timer->start(5000, TRUE);
}

void
fixMimeTypes(void)
{
   DEBUG_MSG("fixMimeTypes\n");
   GUTF8String ptr=getenv("HOME");
   if (ptr.length())
   {
      GUTF8String name_in=ptr+"/.mime.types";
      GUTF8String name_out=name_in+".new";
      bool fixed=fixMimeTypes(name_in, name_out);

      if (fixed)
      {
	 unlink(name_in);
	 GP<ByteStream> str_in=ByteStream::create(GURL::Filename::UTF8(name_out),"r");
	 GP<ByteStream> str_out=ByteStream::create(GURL::Filename::UTF8(name_in),"w");
	 str_out->copy(*str_in);
      }
      unlink(name_out);
   }
}

#include "mime_check_moc.inc"
