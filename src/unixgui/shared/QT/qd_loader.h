//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_loader.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $

 
#ifndef HDR_QD_LOADER
#define HDR_QD_LOADER

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuImage.h"
#include "DataPool.h"
#include "DjVuDocument.h"
#include "qt_n_in_one.h"
#include "qd_port.h"

#include <qdialog.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>

#include "qt_fix.h"

// QDLoader: class used to load DjVu files and to decode them. It doesn't
// work with threads: decodes in the main thread only providing progress
// indicator (working due to NPByteStream tracking of data read by
// DejaVuDecoder)
// If you create it passing the name (not ByteStream) then QDLoader will
// give you a chance to modify the preinitialized name before actually
// loading the file.
// If you pass the byte stream to the constructor then QDLoader will
// start LOADING the file immediately. You may interrupt the process and
// reset the name though

class QDLoader : public QeDialog
{
   Q_OBJECT
private:
   QeNInOne	* form;
   QWidget	* decode_w, * load_w;
   QeNInOne	* progress_form;
   QProgressBar	* progress_bar;
   QeLabel	* progress_label;
   QLineEdit	* text;
   QePushButton	* ok_butt, * cancel_butt;
   QeLabel	* status_label;
   QDPort	port;

   bool		external_document;
   float	last_done;

   int		decode_page_num;
   GURL		pool_url;
   GP<DataPool>	pool;
   GP<DjVuMemoryPort>	mem_port;
   GP<DjVuSimplePort>	simple_port;
   
   GP<DjVuDocument>	document;
   GP<DjVuImage>	image;

   void		setUpForLoad(const QString &name);
   void		setUpForDecode(void);
   void		startDecoding(void);
   void		init(void);
private slots:
   void		slotOK(void);
   void		slotCancel(void);
   void		slotBrowse(void);

      // Slots to receive translated requests from QDPort
   void		slotNotifyError(const GP<DjVuPort> & source, const GUTF8String &msg);
   void		slotNotifyStatus(const GP<DjVuPort> & source, const QString &msg);
   void		slotNotifyFileFlagsChanged(const GP<DjVuFile> & source, long, long);
   void		slotNotifyDecodeProgress(const GP<DjVuPort> & source, float done);
protected:
   virtual void	keyPressEvent(QKeyEvent * ev);
public:
   GUTF8String	getFileName(void) const { return GStringFromQString(text->text()); };
   GP<DjVuDocument>	getDocument(void) const { return document; };
   GP<DjVuImage>	getImage(void) const { return image; };
   
   virtual void	show(void);

      /** Intializes the loader only with the suggested file name.
	  The user will be allowed to choose a new file if he wishes. */
   QDLoader(const char * fname, QWidget * parent=0, const char * name=0);

      /** Initializes the loader with the file name and preloaded data.
	  The loader will set up for decoding. If decoding fails
	  the user will have a choice to specify another file name. */
   QDLoader(const char* fname, const GP<DataPool> & pool, QWidget * parent=0,
	    const char * name=0);

      /** Initializes the loader with partially decoded document and
	  the page number to be decoded. If the decoding fails, it won't
	  suggest to enter a different document name (contrary to two other
	  constructors). */
   QDLoader(const GP<DjVuDocument> & document, int page_num,
	    QWidget * parent=0, const char * name=0);
   ~QDLoader(void);
};

inline
QDLoader::~QDLoader(void)
{
   delete mem_port; mem_port=0;
   delete simple_port; simple_port=0;
}

#endif
 
