//C-  -*- C++ -*-
//C-
//C- DjVu� Unix Viewer (v. 3.5)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C-
// 
// $Id: djvu_viewer.h,v 1.2 2001-07-25 17:10:41 mchen Exp $
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
