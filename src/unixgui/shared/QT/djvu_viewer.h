//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: djvu_viewer.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_DJVU_VIEWER
#define HDR_DJVU_VIEWER

#ifdef __GNUC__
#pragma interface
#endif

#include "qd_viewer.h"
#include "qd_messenger.h"
#include "DjVuDocument.h"

#include "qt_fix.h"

// It's the file providing the decoding on the fily using or not using
// threads. It has an interface reminding that in Netscape's plugins:
// attach(), detach(), newStream(), etc.

// Please note, that this is the only place where procedures running from
// other threads may be defined (in addition to QDMessenger and DejaVuDecoder)

class DjVuViewerPort : public DjVuPort
{
public:
   class DjVuViewer	* viewer;
   GSafeFlags		disabled;
   
   virtual GP<DataPool>	request_data(const DjVuPort * source, const GURL & url);
   virtual bool		notify_error(const DjVuPort * source, const GUTF8String &msg);
   virtual bool		notify_status(const DjVuPort * source, const GUTF8String &msg);

   DjVuViewerPort(class DjVuViewer * v) : viewer(v) {}
};

class DjVuViewer : public QObject, public GPEnabled
{
   Q_OBJECT
private:
   friend class DjVuViewerPort;
   enum CMD { SET_QDVIEWER_DOC };

   QDViewer::PluginData	plugin_data;
   int		in_netscape;

   bool		attach_postpone;
   QWidget	* attach_parent;

      // I need to keep copy of the following items here in case
      // if netscape requests detach/attach of the window
   GUTF8String	page_key;
   GUTF8String	djvu_dir;

   GP<DjVuViewerPort>	port;
   QDViewer	* viewer;
   QDMessenger	messenger;
   
   GP<DjVuDocument>	document;
   GURL			doc_url;
   GP<DataPool>		doc_pool;

   GP<DataPool>	(* request_data_cb)(const GURL & url, void * cl_data);
   void		* request_data_cl_data;

      // The following functions are just interface to messenger, so that
      // you may call them from any thread
   void		getURL(const GURL & url, const GUTF8String &target);
   void		showStatus(const QString &msg);
   void		showError(const GUTF8String &title, const GUTF8String &msg);
   GP<DataPool>	requestData(const GURL & url);
private slots:
   void		slotViewerDestroyed(void);
   void		slotShowError(const GUTF8String &, const GUTF8String &);
protected:
   virtual bool	eventFilter(QObject * obj, QEvent * ev);
signals:
   void		sigGetURL(const GURL & url, const GUTF8String &target);
   void		sigShowStatus(const QString &msg);
public:
   void		attach(QWidget * parent);
   void		detach(void);
   void		newStream(const GURL & URL, const GP<DataPool> & pool);
   SavedData	getSavedData(void);

   QDViewer *	getQDViewer(void) const { return viewer; }
   GP<DjVuDocument> getDjVuDocument(void) const { return document; };

   void		setDjVuDir(const GUTF8String &dir);

   void		setRequestDataCB(GP<DataPool> (*)(const GURL &, void *), void *);
   
   DjVuViewer(int in_netscape, const QDViewer::PluginData & pdata);
   ~DjVuViewer(void);
};

inline void
DjVuViewer::setDjVuDir(const GUTF8String &qdir)
{
//     const char * const dir=qdir;
//     djvu_dir=dir;
   djvu_dir=qdir;
   if (viewer) viewer->setDjVuDir(qdir);
}

inline void
DjVuViewer::setRequestDataCB(GP<DataPool> (* cb)(const GURL &, void *), void * data)
{
   request_data_cb=cb;
   request_data_cl_data=data;
}

inline void
DjVuViewer::getURL(const GURL & url, const GUTF8String &target)
{
   messenger.getURL(url, target);
}

inline void
DjVuViewer::showStatus(const QString &msg)
{
   messenger.showStatus(msg);
}

inline void
DjVuViewer::showError(const GUTF8String &title, const GUTF8String &msg)
{
   messenger.showError(title, msg);
}

#endif
