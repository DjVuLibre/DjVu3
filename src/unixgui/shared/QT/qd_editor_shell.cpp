//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_editor_shell.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_editor_shell.h"
#include "debug.h"
#include "djvu_editor_res.h"
#include "DjVuDocument.h"
#include "qlib.h"
#include "cin_data.h"
#include "qt_n_in_one.h"
#include "version.h"

#include "qworkspace.h"

#include <qkeycode.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtoolbar.h>
#include <qimage.h>

#include "qt_fix.h"

#include "qd_tbar_nav_piece.h"
#include "qd_tbar_mode_piece.h"

#ifndef QT1
#include <qmotifstyle.h>
static QMotifStyle *motif=0;
#endif

const char *djedit_help_script=0;

class QPBar : public QProgressBar
{
protected:
   virtual void	show(void)
   {
      QProgressBar::show();
      setFrameStyle(QFrame::Panel | QFrame::Sunken);
      setLineWidth(1);
   }
public:
   QPBar(QWidget * parent=0, const char * name=0, WFlags f=0) :
	 QProgressBar(parent, name, f) {}
   ~QPBar(void) {}
};

void
QDEditorMainWindow::createShell(void)
{
   DEBUG_MSG("QDEditorMainWindow::createShell(): building top level shell and reparenting QDANTBase\n");
   DEBUG_MAKE_INDENT(3);

   //qeApp->setWidgetGeometry(this);

   closing=FALSE;

   QWidget::setCaption(tr("DjVu Shop"));

   GP<ByteStream> str=CINData::get("ppm_djedit_icon");
   if (str)
   {
      GP<GPixmap> gpix=GPixmap::create(*str);
      setIcon(createIcon(*gpix));
   }

   //**** Creating menu system
   menu=new QeMenuBar(this, "menu_bar");
   connect(menu, SIGNAL(activated(int)), this, SLOT(slotMenuCB(int)));
   
   QPopupMenu * file_pane=new QPopupMenu();
   connect(file_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   file_pane->insertItem(tr("&Open"), IDC_OPEN);
   file_pane->insertItem(tr("&Insert page(s)"), IDC_PAGE_INSERT_AFTER);
   if ( canCompress )
   {
      file_pane->insertItem(tr("&Import"), IDC_IMPORT);
      file_pane->insertItem(tr("&Import page(s)"), IDC_PAGE_IMPORT_AFTER);
   }
   file_pane->insertSeparator();
   file_pane->insertItem(tr("Save Document &As..."), IDC_SAVE_DOC_AS);
   file_pane->insertItem(tr("Save &Document..."), IDC_SAVE_DOC);
   file_pane->insertItem(tr("&Save Page As..."), IDC_SAVE_PAGE_AS);
   file_pane->insertItem(tr("Sa&ve Pages As..."), IDC_SAVE_PAGES_AS);
   if ( canCompress )
      file_pane->insertItem(tr("&Compress Document"), IDC_COMPRESS_DOC);
   file_pane->insertItem(tr("&Export Page"), IDC_EXPORT_PAGE);
   file_pane->insertSeparator();
   file_pane->insertItem(tr("I&ntegrate Thumbnails"), IDC_THUMB_GENERATE);
   file_pane->insertItem(tr("Remove &Thumbnails"), IDC_THUMB_REMOVE);
   file_pane->insertSeparator();
   file_pane->insertItem(tr("&Print Document"), IDC_PRINT_DOC);
   file_pane->insertItem(tr("P&rint Page"), IDC_PRINT_PAGE);
   file_pane->insertSeparator();
   file_pane->insertItem(tr("E&xit"), IDC_EXIT);
   menu->insertItem(tr("&File"), file_pane, IDC_FILE);

   QPopupMenu * edit_pane=new QPopupMenu();
   edit_pane->insertItem(tr("&Find"), IDC_SEARCH);
   edit_pane->insertSeparator();

   edit_pane->insertItem(tr("&Insert Page Before"), IDC_PAGE_INSERT_BEFORE);
   edit_pane->insertItem(tr("Insert Page &After"), IDC_PAGE_INSERT_AFTER);
   if ( canCompress )
   {
      edit_pane->insertItem(tr("&Import Page Before"), IDC_PAGE_IMPORT_BEFORE);
      edit_pane->insertItem(tr("Import Page &After"), IDC_PAGE_IMPORT_AFTER);
   }
   edit_pane->insertItem(tr("Remove Current &Page"), IDC_PAGE_REMOVE);
   edit_pane->insertItem(tr("M&ove Current Page"), IDC_PAGE_MOVE);
   edit_pane->insertSeparator();

   edit_pane->insertItem(tr("Document &Background Color"), IDC_DOC_BACK_COLOR);
   edit_pane->insertItem(tr("Document A&lignment"), IDC_DOC_ALIGNMENT);
   edit_pane->insertSeparator();
   
   QPopupMenu * hlink_pane=new QPopupMenu();
   hlink_pane->insertItem(tr("New &Rectangle Hyperlink"), IDC_HLINKS_NEW_RECT);
   hlink_pane->insertItem(tr("New &Polygon Hyperlink"), IDC_HLINKS_NEW_POLY);
   hlink_pane->insertItem(tr("New &Oval Hyperlink"), IDC_HLINKS_NEW_OVAL);
   hlink_pane->insertItem(tr("New &Highlighted area"), IDC_HILITE_NEW_RECT);
   connect(hlink_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));   
   edit_pane->insertItem(tr("&New"), hlink_pane, IDC_HLINKS);
   
   edit_pane->insertItem(tr("Recommended Page &Mode"), IDC_START_MODE);
   edit_pane->insertItem(tr("Recommended Page &Zoom"), IDC_START_ZOOM);
   edit_pane->insertItem(tr("Page Ba&ckground Color"), IDC_BACK_COLOR);
   edit_pane->insertItem(tr("Page Ali&gnment"), IDC_ALIGNMENT);
   edit_pane->insertSeparator();
   edit_pane->insertItem(tr("Pre&ferences"), IDC_PREFERENCES);
   connect(edit_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   menu->insertItem(tr("&Edit"), edit_pane, IDC_EDIT);

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

   QPopupMenu * OCR_pane=new QPopupMenu();
   OCR_pane->insertItem(tr("OCR &Entire Document"), IDC_OCR_DOC);
   OCR_pane->insertItem(tr("OCR &Current Page"), IDC_OCR_PAGE);
   connect(OCR_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   menu->insertItem(tr("&OCR"), OCR_pane, IDC_OCR);

   QPopupMenu * info_pane=new QPopupMenu();
   info_pane->insertItem(tr("&Page Information"), IDC_ABOUT_PAGE);
   info_pane->insertItem(tr("Show &Thumbnails"), IDC_THUMB_SHOW);
   connect(info_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   menu->insertItem(tr("&Info"), info_pane, IDC_INFO);

   windows_pane = new QPopupMenu();
   windows_pane->setCheckable( TRUE );
   connect( windows_pane, SIGNAL( aboutToShow(void) ), this, SLOT( slotAboutToShowWindowsMenu(void) ) );
   menu->insertItem( tr("&Windows"), windows_pane, IDC_WINDOWS );
   
   menu->insertSeparator();
   QPopupMenu * help_pane=new QPopupMenu();
   help_pane->insertItem(tr("&Contents"), IDC_CONTENTS);
   help_pane->insertSeparator();
   help_pane->insertItem(tr("&About"), IDC_ABOUT);
   connect(help_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   menu->insertItem(tr("&Help"), help_pane, IDC_HELP);


   //**** Creating the edit toolbar
   edit_toolbar=new QToolBar(this, "edit_toolbar");
   open_tbutt=new QDToolButton(*CINData::get("ppm_open"), false,
			       IDC_OPEN, edit_toolbar, tr("Open"));
   connect(open_tbutt, SIGNAL(clicked(void)), this, SLOT(slotToolButtonClicked(void)));
   save_tbutt=new QDToolButton(*CINData::get("ppm_save"), false,
			       IDC_SAVE_DOC, edit_toolbar, tr("Save Document"));
   connect(save_tbutt, SIGNAL(clicked(void)), this, SLOT(slotToolButtonClicked(void)));
   print_tbutt=new QDToolButton(*CINData::get("ppm_print"), false,
				IDC_PRINT, edit_toolbar, tr("Print"));
   connect(print_tbutt, SIGNAL(clicked(void)), this, SLOT(slotToolButtonClicked(void)));
   hlrect_tbutt=new QDToolButton(*CINData::get("ppm_hlrect"), false,
				 IDC_HILITE_NEW_RECT, edit_toolbar, tr("New highlighted area"));
   hlrect_tbutt->setToggleButton(TRUE);
   connect(hlrect_tbutt, SIGNAL(clicked(void)), this, SLOT(slotToolButtonClicked(void)));
   rect_tbutt=new QDToolButton(*CINData::get("ppm_rect"), false,
			       IDC_HLINKS_NEW_RECT, edit_toolbar, tr("New rectangular hyperlink"));
   rect_tbutt->setToggleButton(TRUE);
   connect(rect_tbutt, SIGNAL(clicked(void)), this, SLOT(slotToolButtonClicked(void)));
   oval_tbutt=new QDToolButton(*CINData::get("ppm_oval"), false,
			       IDC_HLINKS_NEW_OVAL, edit_toolbar, tr("New oval hyperlink"));
   oval_tbutt->setToggleButton(TRUE);
   connect(oval_tbutt, SIGNAL(clicked(void)), this, SLOT(slotToolButtonClicked(void)));
   poly_tbutt=new QDToolButton(*CINData::get("ppm_poly"), false,
			       IDC_HLINKS_NEW_POLY, edit_toolbar, tr("New polygon hyperlink"));
   poly_tbutt->setToggleButton(TRUE);
   connect(poly_tbutt, SIGNAL(clicked(void)), this, SLOT(slotToolButtonClicked(void)));
   edit_tbutt=new QDToolButton(*CINData::get("ppm_edit"), false,
			       IDC_EDIT_MODE, edit_toolbar, tr("Edit mode"));
   edit_tbutt->setToggleButton(TRUE);
   connect(edit_tbutt, SIGNAL(clicked(void)), this, SLOT(slotToolButtonClicked(void)));
   preview_tbutt=new QDToolButton(*CINData::get("ppm_preview"), false,
				  IDC_PREVIEW_MODE, edit_toolbar, tr("Preview mode"));
   preview_tbutt->setToggleButton(TRUE);
   connect(preview_tbutt, SIGNAL(clicked(void)), this, SLOT(slotToolButtonClicked(void)));
   
   // create the view toolbar
   view_toolbar=new QToolBar(this, "view_toolbar");

   mode_tbar=new QDTBarModePiece(view_toolbar);
   connect(mode_tbar, SIGNAL(sigSetZoom(int)), this, SLOT(slotSetZoom(int)));
   connect(mode_tbar, SIGNAL(sigSetMode(int)), this, SLOT(slotSetMode(int)));

   nav_tbar=new QDTBarNavPiece(view_toolbar);   
   connect(nav_tbar, SIGNAL(sigGotoPage(int)), this, SLOT(slotGotoPage(int)));
   connect(nav_tbar, SIGNAL(sigDoCmd(int)), this, SLOT(slotDoCmd(int)));
   
   // I want to show the toolbars right here because otherwise QMainWindow
   // shows it too late, and the image is redrawn an extra time
   edit_toolbar->show();
   view_toolbar->show();

   //**** Creating the central widget and status bar and reparenting 'this'
   QWidget * central=new QWidget(this);
   QVBoxLayout * vlay=new QVBoxLayout(central, 0, 0, "vlay");

   wpaper_switch=new QeNInOne(central, "wpaper_switch");
   vlay->addWidget(wpaper_switch, 1);
   
   // Create the startup image (wpaper)
   wpaper=0;
   //if (canCompress)
   str=CINData::get("bmp_djvushop_front");
   //else
   //str=CINData::get("bmp_djedit_front");

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
	 QeLabel * l=new QeLabel(wpaper_switch, "wpaper");
	 l->setAlignment(AlignCenter);
	 l->setBackgroundColor(white);
	 l->setPixmap(qpix);
	 wpaper=l;
      }
   }

   if (!wpaper)
      wpaper=new QeLabel(tr("Select \"File\" and then \"Open\" to load a DjVu file"), wpaper_switch);

   // create work space as child of wpaper_switch 
   ws = new QWorkspace(wpaper_switch);
   connect(ws, SIGNAL(windowActivated(QWidget *)), this, SLOT(slotWindowActivated(QWidget *)));
   
   wpaper_switch->setActiveWidget(wpaper);
   
      // Creating the status bar
   QFrame * frame=new QFrame(central);
   frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
   vlay->addWidget(frame);
   QHBoxLayout * flay=new QHBoxLayout(frame, 2, 0);
   progress_bar=new QPBar(frame, "editor_progress");
#ifdef QT1
   progress_bar->setStyle(MotifStyle);
#else
   if(!motif)
      motif=new QMotifStyle();
   progress_bar->setStyle(motif);
#endif
   // Change base color of the palette to be the background (not just white)
   QColorGroup grp=progress_bar->colorGroup();
   QColorGroup newgrp(grp.foreground(), grp.background(),
		      grp.light(), grp.dark(), grp.mid(),
		      grp.text(), grp.background());
   progress_bar->setPalette(QPalette(newgrp, newgrp, newgrp));
   progress_bar->setMinimumWidth(100);
   flay->addWidget(progress_bar);
   status_bar=new QLabel(tr("Status bar"), frame, "status_bar");
   status_bar->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   status_bar->setFixedHeight(status_bar->sizeHint().height());
   status_bar->setAlignment(AlignVCenter | AlignLeft);
   flay->addWidget(status_bar, 1);
   flay->activate();
   vlay->activate();

   open_tbutt->status_bar=save_tbutt->status_bar=print_tbutt->status_bar=
      hlrect_tbutt->status_bar=rect_tbutt->status_bar=
      oval_tbutt->status_bar=poly_tbutt->status_bar=
      edit_tbutt->status_bar=preview_tbutt->status_bar=status_bar;

   setCentralWidget(central);
   // by default all toolbar is dockable
   //setDockEnabled(edit_toolbar,Unmanaged,TRUE);

   // reset the sensitivity of menu items
   resetMenuAndEditingToolBar();

   // the show is needed so that the main window size can be determined
   // as early as possible. the confusion about the size is evident
   // when loading doc from command line (if without show())
   show();
}


void 
QDEditorMainWindow::contents(void)
{
  if(djedit_help_script)
  {
    system(djedit_help_script);
  }
}

void
QDEditorMainWindow::about(void)
{
   QeDialog * d=new QeDialog(this, "about", TRUE);
   d->setCaption(tr("About DjVu Editor"));

   QWidget * start=d->startWidget();
   
   QeLabel * label;
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);

   QFrame * frame=new QFrame(start);
   frame->setFrameStyle(QFrame::Box | QFrame::Sunken);
   vlay->addWidget(frame);
   QVBoxLayout * frame_vlay=new QVBoxLayout(frame, 20, 10);

   label=new QeLabel(tr("DjVu(tm) Editor"), frame);
   QFont font=label->font();
   font.setBold(TRUE);
   font.setPointSize(font.pointSize()+5);
   label->setFont(font);
   label->setAlignment(AlignCenter);
   frame_vlay->addWidget(label);

   QString ver=tr("Version ")+DJEDIT_VERSION_STR;
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
QDEditorMainWindow::slotToolButtonClicked(void)
{
   DEBUG_MSG("QDEditorMainWindow::slotToolButtonClicked()\n");
   try
   {
      const QObject * obj=sender();
      if (obj && obj->inherits("QDToolButton"))
      {
	 QDToolButton * butt=(QDToolButton *) obj;
	 if (butt->isToggleButton() && !butt->isOn())
	    butt->setOn(TRUE);
	 else processCommand(((QDToolButton *) obj)->cmd);
      }
   } catch(const GException & exc)
   {
      showError(this, "DjVu Error", exc);
   }
}

void
QDEditorMainWindow::slotAboutToShowMenu(void)
      // Don't forget, there is also a runPopupMenu() function
      // doing the same stuff for popup menu.
{
   DEBUG_MSG("QDEditorMainWindow::slotAboutToShowMenu(): adjusting and menu items\n");
   DEBUG_MAKE_INDENT(3);

   try
   {
      const QObject * pane_w=sender();
      QMenuData * pane=0;

      if (pane_w && pane_w->inherits("QPopupMenu"))
	 pane=(QPopupMenu *) pane_w;

      QDEditor *ed = (QDEditor *)ws->activeWindow();
      if ( ed )
	 ed->setupMenu(menu, pane);
   } catch(const GException & exc)
   {
      showError(this, "DjVu Error", exc);
   }
}

void
QDEditor::updateEditToolBar(void)
{
   DEBUG_MSG("QDEditor::updateEditToolBar(): Updating the toolbar buttons sensitivity\n");

   if ( !main_window ) return;
   
   main_window->open_tbutt->setEnabled(cur_editor_mode!=CREATE);
   main_window->save_tbutt->setEnabled(dimg && cur_editor_mode!=CREATE);
   main_window->print_tbutt->setEnabled(dimg && cur_editor_mode!=CREATE);
   main_window->hlrect_tbutt->setEnabled(dimg);
   main_window->rect_tbutt->setEnabled(dimg);
   main_window->oval_tbutt->setEnabled(dimg);
   main_window->poly_tbutt->setEnabled(dimg);
   main_window->edit_tbutt->setEnabled(dimg);
   main_window->preview_tbutt->setEnabled(dimg);
   
   main_window->hlrect_tbutt->setOn(cur_editor_mode==CREATE &&
				    new_marea_type==IDC_HILITE_NEW_RECT);
   main_window->rect_tbutt->setOn(cur_editor_mode==CREATE &&
				  new_marea_type==IDC_HLINKS_NEW_RECT);
   main_window->poly_tbutt->setOn(cur_editor_mode==CREATE &&
				  new_marea_type==IDC_HLINKS_NEW_POLY);
   main_window->oval_tbutt->setOn(cur_editor_mode==CREATE &&
				  new_marea_type==IDC_HLINKS_NEW_OVAL);
   main_window->edit_tbutt->setOn(cur_editor_mode==EDIT);
   main_window->preview_tbutt->setOn(cur_editor_mode==PREVIEW);
}

void
QDEditorMainWindow::resetMenuAndEditingToolBar(void)
{
   DEBUG_MSG("QDEditorMainWindow::resetMenuAndEditingToolBar()\n");

   // menu items

   // Do everything insensitive except for some stuff
   setItemsEnabled(menu, FALSE);

   menu->setItemEnabled(IDC_OPEN, TRUE);
   menu->setItemEnabled(IDC_IMPORT, TRUE);
   menu->setItemEnabled(IDC_EXIT, TRUE);
   menu->setItemEnabled(IDC_FILE, TRUE);
   menu->setItemEnabled(IDC_PAGE_INSERT_AFTER, TRUE);
   menu->setItemEnabled(IDC_PAGE_IMPORT_AFTER, TRUE);
   menu->setItemEnabled(IDC_HELP, TRUE);
   menu->setItemEnabled(IDC_ABOUT, TRUE);
   menu->setItemEnabled(IDC_CONTENTS, (djedit_help_script?TRUE:FALSE));
   
   // editing toolbar items
   open_tbutt->setEnabled(TRUE);
   save_tbutt->setEnabled(FALSE);
   print_tbutt->setEnabled(FALSE);
   hlrect_tbutt->setEnabled(FALSE);
   rect_tbutt->setEnabled(FALSE);
   oval_tbutt->setEnabled(FALSE);
   poly_tbutt->setEnabled(FALSE);
   edit_tbutt->setEnabled(FALSE);
   preview_tbutt->setEnabled(FALSE);
   
   hlrect_tbutt->setOn(FALSE);
   rect_tbutt->setOn(FALSE);
   poly_tbutt->setOn(FALSE);
   oval_tbutt->setOn(FALSE);
   edit_tbutt->setOn(FALSE);
   preview_tbutt->setOn(FALSE);

   // viewing toolbar items
   mode_tbar->setEnabled(FALSE);
   nav_tbar->setEnabled(FALSE);
}


void
QDEditorMainWindow::slotAboutToShowWindowsMenu(void)
{
   DEBUG_MSG("QDEditorMainWindow::slotAboutToShowWindowsMenu\n");
   
   windows_pane->clear();
   int cascadeId = windows_pane->insertItem(tr("&Cascade"), ws, SLOT(cascade() ) );
   int tileId = windows_pane->insertItem(tr("&Tile"), ws, SLOT(tile() ) );
   if ( ws->windowList().isEmpty() ) {
      windows_pane->setItemEnabled( cascadeId, FALSE );
      windows_pane->setItemEnabled( tileId, FALSE );
   }
   windows_pane->insertSeparator();
   QWidgetList windows = ws->windowList();
   for ( int i = 0; i < int(windows.count()); ++i ) {
      int id = windows_pane->insertItem(windows.at(i)->caption(),
				       this, SLOT( slotWindowsMenuActivated( int ) ) );
      windows_pane->setItemParameter( id, i );
      windows_pane->setItemChecked( id, ws->activeWindow() == windows.at(i) );
   }
}

void
QDEditorMainWindow::slotWindowsMenuActivated( int id )
{
   DEBUG_MSG("QDEditorMainWindow::slotWindowsMenuActivated()\n");
   QDEditor *w =dynamic_cast<QDEditor *>(ws->windowList().at(id));
   if (w && !w->hasFocus())
      w->setFocus();
}

void
QDEditorMainWindow::slotWindowActivated(QWidget *w)
{
   DEBUG_MSG("QDEditorMainWindow::slotWindowActivated()\n");
   // w could be NULL or garbage
   if ( !w ) return;

   bool update=FALSE;
   QWidgetList windows = ws->windowList();
   //singal windowActivated may emit garbage. so do a sanity check first.
   //note that we should have removed any editor from the window list 
   //if it's not created correctly - see QDEditor constructor 
   for ( int i = 0; i < int(windows.count()); ++i )
   {
      if ( w == windows.at(i) ) 
      {
	 update=TRUE;
	 break;
      }
   }
   if ( !update ) return;
   
   // so w is sane
   QDEditor *ed=dynamic_cast<QDEditor *>(w);
   if ( ed && ed->open_ok )
   {
      ed->updateEditToolBar(); // editing tool bar 
      ed->updateToolBar(); // mode and navigation toolbar
      updateMainMenus(ed->cur_editor_mode!=QDEditor::CREATE);       
   }
}

void
QDEditorMainWindow::updateMainMenus(bool viewingMode)
{
   DEBUG_MSG("QDEditorMainWindow::updateMainMenus()\n");

   if ( !menu ) return;
   
   menu->setItemEnabled(IDC_EDIT, viewingMode);
   menu->setItemEnabled(IDC_MODE, viewingMode);
   menu->setItemEnabled(IDC_ZOOM, viewingMode);
   menu->setItemEnabled(IDC_NAVIGATE, viewingMode);

#if 1
   menu->setItemEnabled(IDC_FILE, viewingMode);
#else
   // this won't work well since setupMenu() will overwrite
   // everything
   menu->setItemEnabled(IDC_SAVE_DOC_AS, viewingMode);
   menu->setItemEnabled(IDC_SAVE_DOC, viewingMode);
   menu->setItemEnabled(IDC_SAVE_PAGE_AS, viewingMode);
   menu->setItemEnabled(IDC_SAVE_PAGES_AS, viewingMode);
   menu->setItemEnabled(IDC_EXPORT_PAGE, viewingMode);
   menu->setItemEnabled(IDC_THUMB_GENERATE, viewingMode);
   menu->setItemEnabled(IDC_THUMB_REMOVE, viewingMode);
   menu->setItemEnabled(IDC_PRINT_DOC, viewingMode);
   menu->setItemEnabled(IDC_PRINT_PAGE, viewingMode);
   
   if ( canCompress )
      menu->setItemEnabled(IDC_COMPRESS_DOC, viewingMode);
   
#endif
}

// END OF FILE
