//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_viewer_shell.h,v 1.1 2001-05-30 17:29:32 mchen Exp $
// $Name:  $


#ifndef HDR_QD_VIEWER_SHELL
#define HDR_QD_VIEWER_SHELL

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuImage.h"		// Must be included before QT

#include <qmenubar.h>
#include <qmainwindow.h>
#include <qlabel.h>
#include <qtimer.h>

#include "qt_n_in_one.h"
#include "prefs.h"
#include "djvu_viewer.h"

#include "qt_fix.h"

// Use this class if you want to create a standalone viewer. It creates
// menu system, wallpaper, DjVuViewer, opens a file - does everything 
// needed to open and display DjVu document without your concern.
// You just need to initialize it and let it live.

class QDViewerShell : public QMainWindow
{
   Q_OBJECT
private:
   static int	instances;
   
   GP<DjVuViewer>djvu;
   
   QeMenuBar	* menu;
   QLabel	* status_bar;
   QeNInOne	* main;
   QWidget	* wpaper;
   QWidget	* vparent;

   QTimer	gu_timer;
   const QObject * gu_djvu;
   GURL		gu_url;
   GUTF8String	gu_target;
   
   GUTF8String	djvu_dir;
   
   void		getURL(const GURL & url, const GUTF8String &target);
   void		about(void);
   void		help(void);
   QDViewerShell * createNewShell(const GURL & url) const;
private slots:
   void		slotAboutToShowMenu(void);
   void		slotMenuCB(int cmd);
   void		slotGetURLTimeout(void);
   void		slotGetURL(const GURL & url, const GUTF8String &target);
protected:
   virtual void	closeEvent(QCloseEvent * e);
public slots:
   void		slotShowStatus(const QString &msg);
public:
      // Opens the document with the specified name in the current
      // window. ID serves to identify page to open. This is an obsolete
      // function. openURL() allows to do the same and much more as it
      // understands CGI arguments in the URL. Use that instead.
      // fname='-' means to read data from stdin, in which case the
      // ID is ignored anyway.
   void		openFile(const QString &fname, const QString &id=QString::null);

      // Opens the specified URL in the current window.
      // Empty URL means to read stdin
   void		openURL(const GURL & url);
   void		setDjVuDir(const GUTF8String &dir);
   
   QDViewerShell(QWidget * parent=0, const char * name=0);
   ~QDViewerShell(void);
};

#endif
