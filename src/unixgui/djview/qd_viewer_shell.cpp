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
// $Id: qd_viewer_shell.cpp,v 1.11 2001-10-17 19:00:53 docbill Exp $
// $Name:  $


#include "DjVuImage.h"
#include "DataPool.h"

#include "qd_viewer_shell.h"
#include "cin_data.h"

#include "qlib.h"
#include "djvu_viewer_res.h"
#include "throw_error.h"
#include "DjVuDocument.h"
#include "qd_wpaper.h"
#include "version.h"
#include "exc_msg.h"

#include <qapplication.h>
#include <qkeycode.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qimage.h>

#include <time.h>

#include "qt_fix.h"

#define START_STATUS	QT_TRANSLATE_NOOP("QDViewerShell","Select \"File\" and then \"Open\" to load a DjVu file")

int	QDViewerShell::instances;

void
QDViewerShell::setDjVuDir(const GUTF8String &qdir)
{
//     const char * const dir=qdir;
//     djvu_dir=dir;
   djvu_dir=qdir;
   if (djvu) djvu->setDjVuDir(qdir);
}

void
QDViewerShell::openFile(const QString &qfname, const QString &qid)
{
   const char * const fname=qfname;
   const char * id=qid;
   DEBUG_MSG("QDViewerShell::openFile(): opening file '" << fname << "'\n");
   DEBUG_MAKE_INDENT(3);

   GURL url;

   if (strcmp(fname, "-"))
   {
      url=GURL::Filename::UTF8(GURL::expand_name(fname));
      if (id)
      {
	 url=GURL::UTF8(url.fname()+id,url.base());
      }
   }
   openURL(url);
}

void
QDViewerShell::openURL(const GURL & url)
      // Empty URL means read stdin
      // The URL may contain a bunch of CGI arguments, which will
      // be processed by DjVuViewer. Here we will just open the
      // file associated with the url and will pump its data into
      // DjVuViewer.
{
   DEBUG_MSG("QDViewerShell::openURL(): opening url '" << url << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (!url.is_local_file_url())
      throw ERROR_MESSAGE("QDViewerShell::openURL",
			  ERR_MSG("QDViewerShell.cant_display_remote") "\t"
			  +(GUTF8String) url);

   GUTF8String fname="-";
   GURL clean_url;

   if (!url.is_empty())
   {
      clean_url=url;
      clean_url.clear_all_arguments();
      fname=clean_url.UTF8Filename();
   }

   try
   {
      char ch;
      GP<ByteStream> gstr=ByteStream::create(GURL::Filename::UTF8(fname),"rb");
      gstr->read(&ch, 1);
   } catch(...)
   {
      ThrowError("QDViewerShell::openURL", ERR_MSG("QDViewerShell.open_file_fail") "\t"
		 +fname);
   }

   if (fname!="-") QeFileDialog::lastLoadDir=QFileInfo(QStringFromGString(fname)).dirPath();

   try
   {
      main->setActiveWidget(wpaper);
      
      if (djvu)
      {
	 djvu->detach();
	 djvu=0;
      }
      djvu=new DjVuViewer(0, QDViewer::PluginData());
      connect(djvu, SIGNAL(sigShowStatus(const QString &)),
	      this, SLOT(slotShowStatus(const QString &)));
      connect(djvu, SIGNAL(sigGetURL(const GURL &, const GUTF8String &)),
	      this, SLOT(slotGetURL(const GURL &, const GUTF8String &)));

      djvu->setDjVuDir(djvu_dir);
      djvu->attach(vparent);
      main->setActiveWidget(vparent);

      DataPool::load_file(clean_url);
      GP<DataPool> pool=DataPool::create(clean_url);

	 // Pass the pool to the DjVuViewer
      djvu->newStream(clean_url, pool);
      slotAboutToShowMenu();
   } catch(const GException & exc)
   {
      main->setActiveWidget(wpaper);
      if (djvu)
      {
	 djvu->detach();
	 djvu=0;
      }
      slotShowStatus(tr(START_STATUS));
      slotAboutToShowMenu();
      throw;
   }
}

QDViewerShell *
QDViewerShell::createNewShell(const GURL & url) const
{
   QDViewerShell * v=new QDViewerShell(0, "qd_viewer_shell");
   qeApp->setWidgetGeometry(v->topLevelWidget());
   v->setDjVuDir(djvu_dir);
   v->show();
   if (!url.is_empty()) v->openURL(url);
   return v;
}

void
QDViewerShell::about(void)
{
   QeDialog * d=new QeDialog(this, "about", TRUE);
   d->setCaption(tr("About DjVu"));

   QWidget * start=d->startWidget();
   
   QeLabel * label;
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);

   QFrame * frame=new QFrame(start);
   frame->setFrameStyle(QFrame::Box | QFrame::Sunken);
   vlay->addWidget(frame);
   QVBoxLayout * frame_vlay=new QVBoxLayout(frame, 20, 10);

   label=new QeLabel(tr("LizardTech DjVu(tm) standalone viewer"), frame);
   QFont font=label->font();
   font.setBold(TRUE);
   font.setPointSize(font.pointSize()+5);
   label->setFont(font);
   label->setAlignment(AlignCenter);
   frame_vlay->addWidget(label);

   QString ver=tr("Version ")+DJVIEW_VERSION_STR;
   label=new QeLabel(ver, frame);
   label->setAlignment(AlignCenter);
   frame_vlay->addWidget(label);

   frame_vlay->activate();

   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   hlay->addStretch(1);
   QePushButton * butt=new QePushButton(tr("&Close"), start);
   butt->setDefault(TRUE);
   hlay->addWidget(butt);
   hlay->addStretch(1);
   
   vlay->activate();

   connect(butt, SIGNAL(clicked(void)), d, SLOT(accept(void)));

   d->exec();
   delete d;
}

void
QDViewerShell::help(void)
{
   GUTF8String fname=GURL::expand_name("help.djvu", djvu_dir);
   if (QFileInfo(QStringFromGString(fname)).isFile())
   {
      createNewShell(GURL::Filename::UTF8(fname));
   } else
   {
     QString mesg=tr("Failed to open file '")+QStringFromGString(fname)+"'";
     showError(this, tr("DjVu Error"), mesg);
   }
}

void
QDViewerShell::slotMenuCB(int cmd)
{
   try
   {
      if (cmd==IDC_EXIT) qApp->quit();
      else if (cmd==IDC_CLOSE)
	 qeApp->killWidget(this);
      else if (cmd==IDC_OPEN)
      {
	 static const char * filters[]={ "*.djvu", "*.djv", "*.iw44", "*.iw4", tr("All files (*)"), 0 };

	 QeFileDialog dialog(QeFileDialog::lastLoadDir, filters[0],
			     this, "file_dialog", TRUE);
	 dialog.setCaption(tr("DjVu: Select DjVu file to open"));
	 dialog.setForWriting(FALSE);
	 dialog.setFilters((const char **) filters);

	 if (dialog.exec()==QDialog::Accepted)
	    openFile(dialog.selectedFile());
      } else if (cmd==IDC_NEW_WINDOW)
      {
	 createNewShell(GURL());
      } if (cmd==IDC_ABOUT_DEJAVU)
      {
	 about();
      } else if (cmd==IDC_HELP_DEJAVU)
      {
	 help();
      } else
      {
	 QDViewer * viewer;
	 if (djvu && (viewer=djvu->getQDViewer()))
	    viewer->processCommand(cmd);
      }
   } catch(const GException & exc)
   {
      showError(this, tr("DjVu Error"), exc);
   }
}

void
QDViewerShell::slotGetURLTimeout(void)
{
   try
   {
      DEBUG_MSG("QDViewerShell::slotGetURLTimeout() called\n");
      DEBUG_MAKE_INDENT(3);
      DEBUG_MSG("url=" << gu_url << ", tgt='" << gu_target << "'\n");

      if (djvu!=gu_djvu)
      {
	 DEBUG_MSG("but the DjVuViewer, that ordered this stream has already gone => return\n");
	 return;
      }
      
      if (gu_url.is_local_file_url())
      {
	 GUTF8String fname=gu_url.UTF8Filename();
	 if (gu_target.length()) createNewShell(gu_url);
	 else
	 {
	    DEBUG_MSG("tgt='' => open file and send the stream to the same obj\n");
	    try
	    {
	       char ch;
	       GP<ByteStream> str=ByteStream::create(GURL::Filename::UTF8(fname),"rb");
	       str->read(&ch, 1);
	    } catch(...)
	    {
		  // Don't want to report 'file not found' errors
	       QString mesg=tr("Failed to open file '")+QStringFromGString(fname)+"'";
	       slotShowStatus(mesg);
	       DEBUG_MSG("Failed to open url=" << gu_url << "\n");
	       return;
	    }
	       // Create the pool and pass it to DjVuViewer
	    GP<DataPool> pool=DataPool::create(gu_url);
	    if (djvu)
            {
              djvu->newStream(gu_url, pool);
            }
	 }
      } else
      {
	 throw ERROR_MESSAGE("main",ERR_MSG("main.cant_display_remote"));
      }
   } catch(const GException & exc)
   {
      showError(this, tr("DjVu Error"), exc);
   }
}

void
QDViewerShell::slotGetURL(const GURL & url, const GUTF8String &target)
{
   gu_djvu=sender();
   gu_url=url;
   gu_target=target.length() ? target : GUTF8String();   
   gu_timer.start(0, TRUE);
}

void
QDViewerShell::slotAboutToShowMenu(void)
{
   QDViewer * viewer;
   if (!djvu || !(viewer=djvu->getQDViewer()))
   {
      setItemsEnabled(menu, FALSE);
   } else viewer->setupMenu(menu);
   
      // Enable the top-level things, Open and Exit and some more
   for(u_int i=0;i<menu->count();i++)
      menu->setItemEnabled(menu->idAt(i), TRUE);
   menu->setItemEnabled(IDC_OPEN, TRUE);
   menu->setItemEnabled(IDC_NEW_WINDOW, TRUE);
   menu->setItemEnabled(IDC_CLOSE, instances>1);
   menu->setItemEnabled(IDC_EXIT, TRUE);
   menu->setItemEnabled(IDC_ABOUT_DEJAVU, TRUE);
   menu->setItemEnabled(IDC_HELP_DEJAVU, TRUE);
}

void
QDViewerShell::slotShowStatus(const QString &qstatus)
{
   const char *status=qstatus;
   if (status)
   {
      if (!status[0]) status=" ";
      
      const char * ptr;
      for(ptr=status;*ptr;ptr++) if (*ptr=='\n') break;
   
      GUTF8String mesg=GUTF8String(status,ptr-status);
      status_bar->setText(QStringFromGString(mesg));
   }
}

void
QDViewerShell::closeEvent(QCloseEvent * e)
{
   // we need to accept the closing request first
   // in order to prevent an infinite recursion
   // see qwidget.cpp
   
   // close/hide the window then book it to be killed
   // note that the shell CAN'T be initialized with
   // WDestructiveClose
   e->accept();
   slotMenuCB(IDC_CLOSE);
}

QDViewerShell::QDViewerShell(QWidget * parent, const char * name)
   : QMainWindow(parent, name) // DON'T use WDestructiveClose
{
   setCaption(tr("DjVu Standalone Viewer"));

      // Set the icon for the shell
   GP<ByteStream> str=CINData::get("ppm_djview_icon");
   if (str)
   {
      GP<GPixmap> gpix=GPixmap::create(*str);
      setIcon(createIcon(*gpix));
   }

   connect(&gu_timer, SIGNAL(timeout(void)), this, SLOT(slotGetURLTimeout(void)));
   
   menu=new QeMenuBar(this, "menu_bar");
   connect(menu, SIGNAL(activated(int)), this, SLOT(slotMenuCB(int)));
   
   QPopupMenu * file_pane=new QPopupMenu();
   file_pane->insertItem(tr("&Open"), IDC_OPEN);
   file_pane->insertItem(tr("&New Window"), IDC_NEW_WINDOW);
   file_pane->insertSeparator();
   file_pane->insertItem(tr("Save D&ocument As..."), IDC_SAVE_DOC_AS);
   file_pane->insertItem(tr("Save P&age As..."), IDC_SAVE_PAGE_AS);
   file_pane->insertItem(tr("&Export page"), IDC_EXPORT_PAGE);
   file_pane->insertSeparator();
   file_pane->insertItem(tr("&Find"), IDC_SEARCH);
   file_pane->insertItem(tr("&Preferences"), IDC_PREFERENCES);
   file_pane->insertSeparator();
   file_pane->insertItem(tr("P&rint Page"), IDC_PRINT_PAGE);
   file_pane->insertItem(tr("Pr&int Document"), IDC_PRINT_DOC);
   file_pane->insertSeparator();
   file_pane->insertItem(tr("&Close"), IDC_CLOSE);
   file_pane->insertItem(tr("E&xit"), IDC_EXIT);
   connect(file_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   menu->insertItem(tr("&File"), file_pane, IDC_FILE);

   QPopupMenu * mode_pane=new QPopupMenu();
   mode_pane->setCheckable(TRUE);
   mode_pane->insertItem(tr("&Color"), IDC_DISPLAY_COLOR);
   mode_pane->insertItem(tr("Black and &White"), IDC_DISPLAY_BLACKWHITE);
   mode_pane->insertItem(tr("&Background"), IDC_DISPLAY_BACKGROUND);
   mode_pane->insertItem(tr("&Foreground"), IDC_DISPLAY_FOREGROUND);
   connect(mode_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   menu->insertItem(tr("&Display"), mode_pane, IDC_MODE);

   QPopupMenu * zoom_pane=new QPopupMenu();
   zoom_pane->setCheckable(TRUE);
   zoom_pane->insertItem("&300 %", IDC_ZOOM_300);
   zoom_pane->insertItem("15&0 %", IDC_ZOOM_150);
   zoom_pane->insertItem("&100 %", IDC_ZOOM_100);
   zoom_pane->insertItem("&75 %", IDC_ZOOM_75);
   zoom_pane->insertItem("&50 %", IDC_ZOOM_50);
   zoom_pane->insertItem("&25 %", IDC_ZOOM_25);
   zoom_pane->insertItem(tr("&Custom..."), IDC_ZOOM_CUSTOM);
   zoom_pane->insertSeparator();
   zoom_pane->insertItem(tr("One &to One"), IDC_ZOOM_ONE2ONE);
   zoom_pane->insertItem(tr("&Stretch"), IDC_ZOOM_STRETCH);
   zoom_pane->insertItem(tr("Fit &Width"), IDC_ZOOM_WIDTH);
   zoom_pane->insertItem(tr("Fit &Page"), IDC_ZOOM_PAGE);
   zoom_pane->insertSeparator();
   zoom_pane->insertItem(tr("Zoom &In"), IDC_ZOOM_ZOOMIN);
   zoom_pane->insertItem(tr("Zoom &Out"), IDC_ZOOM_ZOOMOUT);
   connect(zoom_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   menu->insertItem(tr("&Zoom"), zoom_pane, IDC_ZOOM);

   QPopupMenu * nav_pane=new QPopupMenu();
   nav_pane->insertItem(tr("&Next Page"), IDC_NAV_NEXT_PAGE);
   nav_pane->insertItem(tr("&Previous Page"), IDC_NAV_PREV_PAGE);
   nav_pane->insertItem(tr("&+10 Pages"), IDC_NAV_NEXT_PAGE10);
   nav_pane->insertItem(tr("&-10 Pages"), IDC_NAV_PREV_PAGE10);
   nav_pane->insertItem(tr("&First Page"), IDC_NAV_FIRST_PAGE);
   nav_pane->insertItem(tr("&Last Page"), IDC_NAV_LAST_PAGE);
   nav_pane->insertItem(tr("&Goto Page..."), IDC_NAV_GOTO_PAGE);
   connect(nav_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   menu->insertItem(tr("&Navigate"), nav_pane, IDC_NAVIGATE);

   QPopupMenu * info_pane=new QPopupMenu();
   info_pane->insertItem(tr("&Page Information"), IDC_ABOUT_PAGE);
   info_pane->insertItem(tr("&Document Information"), IDC_ABOUT_DOC);
   info_pane->insertItem(tr("Show &Thumbnails"), IDC_THUMB_SHOW);
   connect(info_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   menu->insertItem(tr("&Info"), info_pane, IDC_INFO);

   menu->insertSeparator();
   QPopupMenu * help_pane=new QPopupMenu();
   help_pane->insertItem(tr("&About"), IDC_ABOUT_DEJAVU);
      //help_pane->insertItem("&Help", IDC_HELP_DEJAVU);
   connect(help_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   menu->insertItem(tr("&Help"), help_pane);

   QWidget * central=new QWidget(this);
   QVBoxLayout * vlay=new QVBoxLayout(central, 0, 0, "vlay");
   main=new QeNInOne(central, "main");
   main->dontResize(TRUE);
   vlay->addWidget(main, 1);
   QFrame * frame=new QFrame(central);
   frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
   vlay->addWidget(frame);
   QHBoxLayout * flay=new QHBoxLayout(frame, 2, 0);
   status_bar=new QLabel(tr("Status bar"), frame, "status_bar");
   status_bar->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   status_bar->setFixedHeight(status_bar->sizeHint().height());
   status_bar->setAlignment(AlignVCenter | AlignLeft);
   flay->addWidget(status_bar);
   flay->activate();
   vlay->activate();
   setCentralWidget(central);

      // Create the startup image
   wpaper=0;
   str=CINData::get(time(0) % 2 ? "bmp_djview_front1" : "bmp_djview_front2");
   if (str)
   {
      GTArray<char> data;
      char buffer[1024];
      int length;
      while((length=str->read(buffer, 1024)))
      {
	 int data_size=data.size();
	 data.resize(data_size+length-1);
	 memcpy((char *) data+data_size, buffer, length);
      }
      QImage qimg;
      if (qimg.loadFromData((u_char *) (char *) data, data.size()))
      {
	 QPixmap qpix(qimg.width(), qimg.height(), x11Depth());
	 QPainter p(&qpix);
	 p.drawImage(0, 0, qimg);
	 p.end();

	    // OK. Successfully decoded image. Get rid of default
	    // wallpaper and create the label with the pixmap
	 QeLabel * l=new QeLabel(main, "wpaper");
	 l->setAlignment(AlignCenter);
	 l->setBackgroundColor(white);
	 l->setPixmap(qpix);
	 wpaper=l;
      }
   }
   if (!wpaper)
   {
      QDWPaper * wp=new QDWPaper(main, "wpaper");
      wp->setText(tr(START_STATUS));
      wpaper=wp;
   }

   vparent=new QWidget(main, "vparent");
   main->setActiveWidget(wpaper);

   status_bar->setText(tr(START_STATUS));
   slotAboutToShowMenu();

   instances++;
}

QDViewerShell::~QDViewerShell(void)
{
   DEBUG_MSG("QDViewerShell::QDViewerShell(): destroying itself\n");
   DEBUG_MAKE_INDENT(3);

   if (--instances==0) qApp->quit();
}

// END OF FILE
