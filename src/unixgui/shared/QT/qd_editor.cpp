//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_editor.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_editor.h"
#include "djvu_editor_res.h"
#include "debug.h"
#include "qlib.h"
#include "qworkspace.h"
#include <qvbox.h>
#include "exc_msg.h"
#include "qd_print_dialog.h"
#include "qd_search_dialog.h"
#include "qd_doc_saver.h"
#include "qd_page_saver.h"
#include "qd_decoder.h"
#include "qd_editor_thumb.h"
#include "qd_thumb_gen.h"
#include "qd_move_page.h"
#include "qd_move_pages.h"
#include "qd_wait_dialog.h"
#include "GOS.h"

#include "qd_tbar_nav_piece.h"
#include "qd_tbar_mode_piece.h"

#include <qkeycode.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qfiledialog.h>
#include <qclipboard.h>

#include "qt_fix.h"
#include "cin_data.h"
#include "qd_editor_thumb.h"

#include "DataPool.h"
#include <qcstring.h>

#include <time.h>

#include "ClassFactory.h"
#include "qd_editor_dlg_iface.h"

#include "DjVuOCR.h"

#ifdef QT1
#define QTNAMESPACE_QT  /* */
#else
#include <qnamespace.h>
#include <q1xcompatibility.h>
#define QTNAMESPACE_QT  Qt
#endif

#if 0
const char *global_compression_profile=0;
int global_compression_dpi=0;
#endif 

// I have to use this hack to force "maximization" of the first editor.
int QDEditor::firstTime=1;

QDEditorMainWindow::QDEditorMainWindow(GP<DjVuDocEditor> &djvu_doc_in,
				       int page_num, QWidget * parent,
				       bool _canCompress, const char * name) 
   : QMainWindow( 0, name ), canCompress(_canCompress)
{

   // initialize workspace and menus ...
   createShell();

   if (&djvu_doc_in)
   {
      GP<QDEditor> editor;      
      editor=new QDEditor(djvu_doc_in, page_num, this, ws, "qd_editor");
      if ( editor->open_ok ) 
	 editorList.append(editor);
      else
	 // if editor is not created correctly, it will be
	 // destroyed 
	 ws->windowList().remove(editor); 
   }
}

QDEditorMainWindow::QDEditorMainWindow(const char *fname,
				       int page_num, QWidget * parent,
				       bool _canCompress, const char * name) 
   : QMainWindow( 0, name ), canCompress(_canCompress)
{

   // initialize workspace and menus ...
   createShell();

   if ( !fname ) return;

   // check fname 
   try
   {
      char ch;
      GP<ByteStream> str=ByteStream::create(GURL::Filename::UTF8(fname),"rb");
      str->read(&ch, 1);
   } catch(...)
   {
      QString msg=QString("Failed to open file '")+fname+"'";
      showError(this, "DjVu Error", msg);
      DEBUG_MSG("Failed to open file=" << fname << "\n");
      return;
   }
   
   GP<QDEditor> editor;   
   editor=new QDEditor(fname, page_num, this, ws, "qd_editor");
   if ( editor->open_ok ) 
      editorList.append(editor);
   else
      // if editor is not created correctly, it will be
      // destroyed 
      ws->windowList().remove(editor);
}

bool
QDEditorMainWindow::close(bool forceKill)
{
   // close all children first
   closing=TRUE;
   QWidgetList windows = ws->windowList();   
   for ( int i = 0; i < int(windows.count()); ++i )
   {
      if ( !((QDEditor *)windows.at(i))->close() )
	 closing=FALSE;
   }
   
   if ( closing ) 
      return QWidget::close(forceKill);
   else
      return FALSE;
   
}
   
QDEditorMainWindow::~QDEditorMainWindow()
{
   // cleanup ?
   delete ws;
   delete wpaper_switch;   
}


// END OF MAIN WINDOW


class QDEditorBusy
{
public:
   QDEditorBusy(void)
   {
      QApplication::setOverrideCursor(QTNAMESPACE_QT::waitCursor);
   }
   ~QDEditorBusy(void)
   {
      QApplication::restoreOverrideCursor();
   }
};



QDEditor::QDEditor(GP<DjVuDocEditor> &djvu_doc_in,
		   int page_num, QDEditorMainWindow *_main_window,
		   QWidget * parent, const char * name) :
   QDBase(parent, name), main_window(_main_window), djvu_doc(djvu_doc_in),
   open_ok(true), port(true, true)
{
  init(djvu_doc,page_num);
}

QDEditor::QDEditor(const char *fname, int page_num,
		   QDEditorMainWindow *_main_window, QWidget * parent,
		   const char * name) :
   QDBase(parent, name), main_window(_main_window), djvu_doc(0),
   open_ok(true), port(true, true)
{

   if(fname)
   {
      try {
	 djvu_doc=DjVuDocEditor::create_wait(GURL::Filename::UTF8(fname));
      } catch(const GException & exc)
      {
	 QString msg=tr("We failed to open file '")+QStringFromGString(fname)+"'\n"+
	    tr("The error log is as follows:\n\n");
	 msg+=QStringFromGString(exc.get_cause());
	 showError(this, "DjVu Error", msg);
	 DEBUG_MSG("Failed to open file=" << fname << "\n");

	 // throw an exception in the contructor may
	 // not be a good idea
	 // the object will be destroyed shortly 
	 open_ok=false;
      }     
   }else
      open_ok=false;
   
   if ( open_ok ) 
      init(djvu_doc,page_num);
}

void
QDEditor::init( GP<DjVuDocEditor> &djvu_doc_in, int page_num)
{
   DEBUG_MSG("QDEditor::QDEditor(): Initializing class...\n");
   DEBUG_MAKE_INDENT(3);

   // "calc" editor position so that there will be no
   // overlap ... most time 
   int editor_pos;
   if ( firstTime || main_window->ws->windowList().isEmpty() )
      editor_pos=0;
   else
   {
      int itmp=main_window->width() < main_window->height() ?
	 main_window->width()/5 : main_window->height()/5;
      editor_pos=time(0)%itmp;
   }
   
   setGeometry(editor_pos,editor_pos,sizeHint().width(),sizeHint().height());
   
   GP<ByteStream> str=CINData::get("ppm_djedit_icon");
   if (str)
   {
      GP<GPixmap> gpix=GPixmap::create(*str);
      setIcon(createIcon(*gpix));
   }

   thumbnails=0;
   popup_menu=0;
   search_parent=0;
   doc_modified=false;
   close_again=false;

   // default editing mode
   cur_editor_mode=EDIT;
   
   cur_ptr=new QCursor(ArrowCursor);

   ignore_ant_mode_zoom=1;

   createPopupMenu();
   //createToolBar();		// Actually, recreate

      // Connect QDPort's signals to our corresponding slots
   connect(&port, SIGNAL(sigNotifyError(const GP<DjVuPort> &, const GUTF8String &)),
	   this, SLOT(slotNotifyError(const GP<DjVuPort> &, const GUTF8String &)));
   connect(&port, SIGNAL(sigNotifyStatus(const GP<DjVuPort> &, const QString &)),
	   this, SLOT(slotNotifyStatus(const GP<DjVuPort> &, const QString &)));
   connect(this, SIGNAL(sigShowStatus(const QString &)),
	   this, SLOT(slotShowStatus(const QString &)));

   setDjVuDocument(djvu_doc, page_num);
   
   // show the very first window in maximized mode
   if ( firstTime || main_window->ws->windowList().isEmpty() )
   {
      firstTime=0;
      showMaximized();
   }
   else
   {
      show();
   }
   
   main_window->updateMainMenus(cur_editor_mode!=CREATE);
   
   // the new doc should have the focus
   setFocus();
}


QSize
QDEditor::sizeHint(void) const
{
   QSize size;

   size.setHeight(main_window->height()*11/16);
   size.setWidth(main_window->width()*11/16);

   return size;
}

QDEditor::~QDEditor(void)
{
   DEBUG_MSG("QDEditor::~QDEditor(): destroying class...\n");
   //delete popup_menu; popup_menu=0;
}


void
QDEditor::show(void)
{
   QDBase::show();

      // If we don't do it here some widgets of the toolbar will appear
      // squeezed. I don't know why. This is just a fast workaround.
   updateToolBar();
}

bool
QDEditor::close(bool forceKill)
{
   // rc=0 => Save first and then close
   // rc=1 => Close w/o saving
   // rc=2 => return from this function
   try
   {
      int rc=1;
      
      if (doc_modified)
      {
	 QString d_title=tr("File: ") + QStringFromGString(djvu_doc->get_doc_url().fname());
	 rc=QMessageBox::warning(this, d_title,
				 tr("You have made some unsaved changes.\n")+
				 tr("Do you want to save them first?"),
				 tr("&Yes"), tr("&No"), tr("&Cancel"), 0, 2);
      }

      if (rc==0)
      {
	 close_again=true;
	 saveDocument();
      }

      if ( rc != 2 )
      {
	 if ( !main_window->closing && main_window->ws->windowList().count() == 1 )
	    main_window->resetMenuAndEditingToolBar();
	 
	 return QWidget::close(forceKill);
      }

   } catch(const GException & exc)
   {
      showError(this, "DjVu Error", exc);
   }
   
   return FALSE;
}



void
QDEditor::savePageAs(int page_num)
{
   GP<DjVuFile> djvu_file;
   
   if (page_num<0)
   {
	 // Save current page
      if (!dimg || !dimg->get_djvu_file()->is_all_data_present())
      {
	 DEBUG_MSG("dimg==0 or it's not fully decoded => return\n");
	 return;
      }
      djvu_file=dimg->get_djvu_file();
   } else
   {
     djvu_file=djvu_doc->get_djvu_file(page_num);
   }

   QDPageSaver page_saver(djvu_file, this);
   page_saver.save();
}

void
QDEditor::savePagesAs(const GList<int> & page_list)
{
   QString msg=tr("This will export pages ")+QDMovePagesDialog::listToString(page_list)+
      tr(" into a\n")+tr("multipage DjVu BUNDLED document.\n")+"\n"+
      tr("Press \"OK\" to continue, or \"Cancel\" to abort.\n");
   if (QMessageBox::information(this, tr("Saving selected pages"),
				msg, tr("&OK"), tr("&Cancel"), 0, 0, 1)==0)
   {
      static char * filters[]={ "*.djvu", "*.djv", "All files (*)", 0 };
      QeFileDialog fd(QeFileDialog::lastSaveDir, filters[0], this, "fd", TRUE);
      fd.setFilters((const char **) filters);
      fd.setCaption(tr("Save pages as..."));

      if (fd.exec()==QDialog::Accepted)
      {
	 GP<ByteStream> str=ByteStream::create(GURL::Filename::UTF8(GStringFromQString(fd.selectedFile())), "w");
	 djvu_doc->save_pages_as(str, page_list);
      }
   }
}

void
QDEditor::savePagesAs(void)
{
   if (thumbnails)
   {
      GList<int> page_list;
      int pages=thumbnails->count();
      for(int page=0;page<pages;page++)
	 if (thumbnails->isSelected(page))
	    page_list.append(page);
      
      if (page_list.size())
	 savePagesAs(page_list);
   }
}

void
QDEditor::saveDocument(void)
{
   DEBUG_MSG("QDEditor::saveDocument(): Saving the document...\n");
   DEBUG_MAKE_INDENT(3);

   if (djvu_doc->can_be_saved())
   {
      bool do_save=false;
      int orig_type=djvu_doc->get_orig_doc_type();
      int save_type=djvu_doc->get_save_doc_type();
      if (orig_type!=save_type)
      {
	 QString msg=tr("This will convert the document to ");
	 switch(save_type)
	 {
	    case DjVuDocument::BUNDLED: msg+=tr("the BUNDLED"); break;
	    case DjVuDocument::INDIRECT: msg+=tr("the INDIRECT"); break;
	    case DjVuDocument::OLD_INDEXED: msg+=tr("the OLD_INDEXED"); break;
	    case DjVuDocument::OLD_BUNDLED: msg+=tr("the OLD_BUNDLED"); break;
	    case DjVuDocument::SINGLE_PAGE: msg+=tr("the SINGLE_PAGE"); break;
	    default: msg+=tr("an UNKNOWN");
	 }
	 msg+=tr(" format.\nDo you want to proceed?");
	 if (QMessageBox::information(this, tr("Really save?"),
				      msg, tr("&Yes"), tr("&No"), 0, 0, 1)==0)
	    do_save=true;
      } else do_save=true;
      if (do_save) do_save=checkThumbnails();
      if (do_save)
      {
	 QDEditorBusy busy;
	 djvu_doc->save();
      }
      setDocModified(false);
      updateEditToolBar();
   } else saveDocumentAs();
}

void
QDEditor::saveDocumentAs(void)
{
   DEBUG_MSG("QDEditor::saveDocumentAs(): Saving the document.\n");
   DEBUG_MAKE_INDENT(3);

   if( djvu_doc->needs_compression())
   {
      compressDocument();
   }else if (checkThumbnails())
   {
      QDDocSaver ds((GP<DjVuDocument> &)djvu_doc, this);
      ds.save();
      setCaption();
      setDocModified(false);
      updateEditToolBar();
   }
}

void
QDEditorMainWindow::openDocument(const char *filters[])
{
   QeFileDialog fd(QDir::currentDirPath(), filters[0], this, "djvu_fd", TRUE);
   fd.setCaption(tr("Choose DjVu document to open"));
   fd.setFilters((const char **) filters);
   fd.setForWriting(FALSE);

   if (fd.exec()==QDialog::Accepted)
   {
      GUTF8String new_file=GStringFromQString(fd.selectedFile());
      if ( !new_file.length() )
	 return;
      // if the selected file is already open, set the focus to the
      // MDI window that contains it
      QWidgetList windows = ws->windowList();
      for ( int i = 0; i < int(windows.count()); ++i ) 
      {
	 GUTF8String file_at_pos=((QDEditor *)windows.at(i))->djvu_doc->get_doc_url().UTF8Filename();
	 if (file_at_pos==new_file)
	 {
	    ((QDEditor *)windows.at(i))->setFocus();
	    return;
	 }
      }
    
      GP<QDEditor> editor;
      editor=new QDEditor(new_file, 0, this, ws, "qd_editor");
      if (editor->open_ok)
	 editorList.append(editor);
      else
	 // if editor is not created correctly, it will be
	 // destroyed 
	 ws->windowList().remove(editor);
   }  
}

void
QDEditor::compressDocument(void)
{

   ClassFactory cf1("libcompress.so");

   QDEditorDlgIface   * compressor=NULL;
   
   // create the initial interface pointer
   if (!cf1.IsLoaded() || !cf1.CreateInterface("QDEditorDlgIface", reinterpret_cast<IBase**>(&compressor)))
      return;

   bool shown=thumbnailsShown();
   if(shown)
   {
      hideThumbnails();
   }

   GURL url=djvu_doc->get_init_url();
   int cur_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());

   compressor->init(djvu_doc, this, "compress");
   compressor->open();

   DjVuPortcaster::clear_all_aliases();
   GURL newurl=djvu_doc->get_init_url();
      
   // this won't work since url may not change at all ?
   // this may result in a funny state for the doc.
   // one solution is to do compression on exit only if needed

   // seems to me that we may have to reload doc anyway since
   // there is no way to turn the flag needs_compression in
   // DjVuDocument back once it is on ?

   //if(newurl != url)
   if ( !close_again ) 
   {
      //if(newurl.get_string()=="")
      if ( newurl.is_empty() )
	 newurl=url;

      GP<DjVuDocEditor> new_doc=DjVuDocEditor::create_wait(newurl);
      setDjVuDocument(new_doc,cur_page);

      if(shown)
      {
	 showThumbnails();
      }
   }

   if ( compressor )
   {
      compressor->Destroy();
      compressor=NULL;
   }

//   setDjVuDocument(, 0);
//   DjVuPortcaster::clear_all_aliases();
//   GURL url=djvu_doc->get_init_url();
//   GUTF8String fname=GOS::url_to_filename(url);
//   GP<DjVuDocEditor> new_doc=new DjVuDocEditor();
//   new_doc->init(fname);
//   setDjVuDocument(new_doc, 0);

}

void
QDEditor::slotCompressOK(int state)
{
   DEBUG_MSG("QDEditor::slotCompressOK\n");
}

#if 0
void
QDEditor::compressDocument(void)
{
   char *profiles[3]={"scan","clean","bitonal"};
   int rc1=QMessageBox::warning(this,"Compression Options",
     "Select your document type\n",
     "&scanned","&clean","&bitonal",0,2);
   int rc2=QMessageBox::warning(this,"Resolution",
     "Select your resolution range\n",
     "&high res>=300","&low res<300","Cancel",0,2);
   global_compression_dpi=0;
   int rc3=0;
   switch(rc2)
   {
     case 1:
       rc3=QMessageBox::warning(this,"Resolution",
       "Select your resolution\n",
       "&100 DPI","1&50 DPI","&200 DPI",0,2);
       global_compression_dpi=(rc3*50)+100;
       break;
     case 0:
       rc3=QMessageBox::warning(this,"Resolution",
       "Select your resolution\n",
       "&300 DPI","&400 DPI","&600 DPI",0,2);
       switch(rc3)
       {
         case 0:
          global_compression_dpi=300;
          break;
         case 1:
          global_compression_dpi=400;
          break;
         default:
          global_compression_dpi=600;
          break;
       }
     default:
       break;
   }
   if(global_compression_dpi)
   {
     GURL oldurl=djvu_doc->get_init_url();
     global_compression_profile=profiles[rc1];
     djvu_doc->set_needs_compression();
     saveDocumentAs();

     GURL url=djvu_doc->get_init_url();

     if(oldurl != url)
     {
       setDjVuDocument(url, 0);
     }
   }
}
#endif

static void
refresh_cb(void * cl_data)
{
   QWidget * w=(QWidget *) cl_data;
   if (w && !w->isVisible())
      G_THROW("Interrupted by User");
   qApp->processEvents(100);
}

void
QDEditor::insertPage(int page_num,const char filter[])
{
   QString dir=QeFileDialog::lastLoadDir;
   if (!QFileInfo(dir).isDir())
      dir=QFileInfo(QStringFromGString(djvu_doc->get_init_url().UTF8Filename())).dirPath();
   if (!QFileInfo(dir).isDir()) dir=QDir::currentDirPath();
   QStringList list(QFileDialog::getOpenFileNames(filter,dir, this));

   GList<GURL> names_list;

   QStringList::Iterator pos=list.begin();
   QStringList::Iterator last=list.end();
   while(pos != last)
   {
     names_list.append(GURL::Filename::UTF8(GStringFromQString(*pos)));
     ++pos;
   }

   if (names_list.size())
   {
      QeFileDialog::lastLoadDir=QFileInfo(QStringFromGString(names_list[names_list].UTF8Filename())).dirPath();
      try
      {
	 QDEditorBusy busy;
	 QDWaitDialog * w=new QDWaitDialog(tr("Inserting pages..."), tr("&Stop"),
					   this, "wait_dialog", TRUE);
	 w->show();
	 qApp->syncX();
	 for(int i=0;i<5;i++) refresh_cb(w);
	 djvu_doc->insert_group(names_list, page_num, refresh_cb, w);
	 delete w;
	 showInfo(this, "DjVu", tr("Page(s) have been successfully inserted."));
      } catch(const GException & exc)
      {
	 QString msg=tr("We failed to insert some of the files.\n")+
		     tr("The error log is as follows:\n\n");
	 msg+=QStringFromGString(exc.get_cause());
	 showError(this, "DjVu Error", msg);
      }
      if (thumbnails)
      {
        thumbnails->rescan(true);
      }
      if (!dimg)
      { 
        gotoPage(0);
      }
      imageUpdated();
      setDocModified(true);
   }
}


void
QDEditor::removePage(int page_num)
{
   GList<int> page_list;
   page_list.append(page_num);
   removePages(page_list);
}

void
QDEditor::removePages(const GList<int> & page_list)
{
   if (dimg)
   {
      QString msg;
      
      if (page_list.size() >= djvu_doc->get_pages_num())
      {
	 QMessageBox::critical(this, tr("Page Deleting Error"),
			       tr("Document cannot be empty and must contain at least one page."));
	 return;
      }
      
      if (page_list.size()==1)
	 msg=tr("This will remove page ")+QString::number(page_list[page_list]+1);
      else
	 msg=tr("This will remove pages ")+QDMovePagesDialog::listToString(page_list);
      msg+=tr(" (no UNDO).\n\nAre you sure you want to proceed?\n");
      if (QMessageBox::warning(this, tr("Are you sure?"), msg,
			       tr("&Yes"), tr("&No"), 0, 0, 1)==0)
      {
	 QDEditorBusy busy;
	 
	 int cur_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
	 bool cur_page_removed=(page_list.contains(cur_page)!=0);

	 djvu_doc->remove_pages(page_list);

	 setDocModified(true);

	 if (thumbnails)
         {
           thumbnails->rescan(true);
         }

	 if (cur_page_removed)
	 {
	       // Oops. Just removed the current page
	    while(cur_page>=djvu_doc->get_pages_num()) cur_page--;
	    if (cur_page>=0)
	    {
	       QDDecoder decoder((GP<DjVuDocument> &)djvu_doc);
	       decoder.setProgressBar(main_window->progress_bar); 
	       GP<DjVuImage> new_dimg=decoder.getPageImage(cur_page);
	       setDjVuImage(new_dimg, true);
	    } else
	    {
	       setDjVuImage(0, true);
	       setDjVuDocument(GURL(), 0);
	    }
	 } else imageUpdated();
      }
   }
}


void
QDEditor::movePage(int page_num)
{
   QDMovePageDialog dlg(page_num, djvu_doc->get_pages_num(), this);
   if (dlg.exec()==QDialog::Accepted)
   {
      djvu_doc->move_page(page_num, dlg.pageNum());

      if (thumbnails)
      {
        thumbnails->rescan(true);
      }
      imageUpdated();
      setDocModified(true);
   }
}

void
QDEditor::movePages(const GList<int> & page_list)
{
   QDMovePagesDialog dlg(page_list, djvu_doc->get_pages_num(), this);
   if (dlg.exec()==QDialog::Accepted)
   {
      int shift=dlg.shift();
      if (shift!=0)
	 djvu_doc->move_pages(page_list, shift);

      if (thumbnails)
      {
        thumbnails->rescan(true);
      }
      imageUpdated();
      setDocModified(true);
   }
}

void
QDEditor::setCaption(void)
{
   QString str;
   if (dimg)
   {
      GURL doc_url=djvu_doc->get_doc_url();
      int page_num=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
      str=QStringFromGString(doc_url.name())+", "+tr("page ")+QString::number(page_num+1);
   }
   if (doc_modified)
      str="[*] - "+str;
   QWidget::setCaption(str);
}

void
QDEditor::setDocModified(bool on)
{
   doc_modified=on;
   setCaption();
}

void
QDEditor::imageUpdated(void)
{
   if (thumbnails && dimg)
   {
      thumbnails->setCurPageNum(djvu_doc->url_to_page(dimg->get_djvu_file()->get_url()));
   }

   setCaption();
   QDBase::imageUpdated();
}

void
QDEditor::setDjVuImage(const GP<DjVuImage> & new_dimg, int do_redraw)
{
   if (new_dimg)
   {
      if (!new_dimg->get_width() || !new_dimg->get_height())
	 throw ERROR_MESSAGE("QDEditor::setDjVuImage",
			     "Can't edit empty page");
      if (new_dimg->get_djvu_file()->is_decode_failed())
	 throw ERROR_MESSAGE("QDEditor::setDjVuImage",
			     "Can't edit a page, which was decoded with errors.");
      if (new_dimg->get_djvu_file()->is_decode_stopped())
	 throw ERROR_MESSAGE("QDEditor::setDjVuImage",
			     "Can't edit a page when decoding has been interrupted.");
      if (!new_dimg->get_djvu_file()->is_decode_ok())
	 throw ERROR_MESSAGE("QDEditor::setDjVuImage",
			     "Can't edit a page, which is not fully decoded.");
   }

   if (new_dimg)
      main_window->wpaper_switch->setActiveWidget(main_window->ws);
   else
      main_window->wpaper_switch->setActiveWidget(main_window->wpaper);

   ignore_ant_mode_zoom=false;
   try
   {
      DjVuPortcaster * pcaster=DjVuPort::get_portcaster();
      if (dimg) pcaster->del_route(dimg, port.getPort());
      pcaster->add_route(new_dimg, port.getPort());
      
      QDBase::setDjVuImage(new_dimg, do_redraw);
   } catch(...)
   {
      ignore_ant_mode_zoom=true;
      throw;
   }
   ignore_ant_mode_zoom=true;

   setCaption();
   
   showStatus(" ");

   //updateEditToolBar();
   setupMenu(main_window->menu, 0); 

   int page_num=-1;
   if (dimg)
   {
     page_num=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
     showThumbnails();
   }else
   {
     hideThumbnails();
   }
   if (thumbnails)
   {
     thumbnails->setCurPageNum(page_num);
   }

   emit sigPageChanged(page_num);

   redraw();
}

void
QDEditor::setDjVuDocument(const GURL &url, int page_num)
{
   GP<DjVuDocEditor> doc;
   if(!url.is_empty() && url.is_valid())
      doc=DjVuDocEditor::create_wait(url);
   else
      doc=DjVuDocEditor::create_wait();

   setDjVuDocument(doc,page_num);
}

void
QDEditor::setDjVuDocument( GP<DjVuDocEditor> &doc, int page_num)
{
   DEBUG_MSG("QDEditor::setDjVuDocument(): changing the document to edit\n");
   DEBUG_MAKE_INDENT(3);

   if (!doc)
      G_THROW("Attempt to initialize the editor with ZERO document\n");

   setDocModified(false);
   shared_anno=0;
   shared_anno_file=0;
   
   qeApp->killWidget(search_parent); search_parent=0;
   
   djvu_doc=doc;
   
   // will enable thumbnails on top/bottom later ...
   // thumbnails' position in stand-alone editor 
   // has to be determined by either command line option
   // or an option on the preference panel 
   if (!thumbnails)
      createThumbnails(false);
   
   if (thumbnails)
      thumbnails->setDjVuDocument(doc);
   
   if (djvu_doc->get_pages_num()>0)
   {
      QDDecoder decoder((GP<DjVuDocument> &)doc);
      GP<DjVuImage> dimg=decoder.getPageImage(0);
      setDjVuImage(dimg, 1);
   }
   updateEditToolBar();
}

void
QDEditor::slotNotifyError(const GP<DjVuPort> &, const GUTF8String &qmsg)
{
   showError(this, "DjVu Error", QStringFromGString(qmsg));
}

void
QDEditor::slotNotifyStatus(const GP<DjVuPort> &, const QString &qmsg)
{
   slotShowStatus(qmsg);
}

void
QDEditor::slotShowStatus(const QString &qmsg)
{
   const char *msg=qmsg;
   if (!msg || !strlen(msg)) msg=" ";
      
   const char * ptr;
   for(ptr=msg;*ptr;ptr++) if (*ptr=='\n') break;
   
   GUTF8String mesg(msg, ptr-msg);
   main_window->status_bar->setText(QStringFromGString(mesg)); 
}

void
QDEditor::gotoPage(int page)
{
   DEBUG_MSG("QDEditor::gotoPage(): Request to go to page #" << page << "\n");
   DEBUG_MAKE_INDENT(3);

   if (cur_editor_mode!=CREATE)
   {
      int doc_page=!dimg ? -1 : djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
      int doc_pages=djvu_doc->get_pages_num();
      if (page<0) page=doc_pages-1;
      if (page>=doc_pages) page=doc_pages-1;

      if (page!=doc_page)
      {
	 QDEditorBusy busy;
	 QDDecoder decoder((GP<DjVuDocument> &)djvu_doc);
	 decoder.setProgressBar(main_window->progress_bar); 
	 GP<DjVuImage> new_dimg=decoder.getPageImage(page);
	 setDjVuImage(new_dimg, 1);
	 emit sigPageChanged(page);
      }
   }
}

bool
QDEditor::eventFilter(QObject *obj, QEvent *e)
{
   if (obj!=pane)
      return QDBase::eventFilter(obj, e);
   
   try
   {
      int doc_page=0, doc_pages=1;
      if (dimg)
      {
	 doc_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
	 doc_pages=djvu_doc->get_pages_num();
      }
      
      switch(e->type())
      {
	 case Event_MouseButtonDblClick:
	 {
	    DEBUG_MSG("QDEditor::eventFilter(): Got Event_MouseButtonDblClick event\n");
	    DEBUG_MAKE_INDENT(3);
	    QMouseEvent * ev=(QMouseEvent *) e;
	    HL_Btn1DblClick(ev->x(), ev->y());
	    return TRUE;
	 }
	 
	 case Event_MouseButtonPress:
	 {
	    DEBUG_MSG("QDEditor::eventFilter(): Got Event_MousePress\n");
	    DEBUG_MAKE_INDENT(3);

	    if (!isLensVisible())
	    {
	       QMouseEvent * ev=(QMouseEvent *) e;
	       if (ev->button()==RightButton)
		  if (cur_editor_mode!=CREATE)
		  {
		     runPopupMenu(ev);
		     return TRUE;
		  }
	       if (ev->button()==LeftButton)
		  if (HL_Btn1Down(ev->x(), ev->y())) return TRUE;
	    }
	    break;
	 }

	 case Event_MouseButtonRelease:
	 {
	    DEBUG_MSG("QDEditor::eventFilter(): Got Event_MouseRelease\n");
	    DEBUG_MAKE_INDENT(3);

	    if (!isLensVisible())
	    {
	       QMouseEvent * ev=(QMouseEvent *) e;
	       if (ev->button()==LeftButton)
		  if (HL_Btn1Up(ev->x(), ev->y())) return true;
	    }
	    break;
	 }

	 case Event_MouseMove:
	 {
	    DEBUG2_MSG("QDEditor::eventFilter(): Got Event_MouseMove\n");
	    DEBUG_MAKE_INDENT(3);

	    QMouseEvent * ev=(QMouseEvent *) e;
	    if (!isLensVisible() && !(ev->state() & ControlButton))
	    {
	       if ((ev->state() & LeftButton) && cur_editor_mode!=PREVIEW)
		  if (HL_Btn1Motion(ev->x(), ev->y())) return true;
	       if (!(ev->state() & (LeftButton | MidButton | RightButton)))
		  if (HL_Motion(ev->x(), ev->y())) return true;
	    }
	    break;
	 }

	 case Event_Leave:
	 {
	    DEBUG_MSG("QDEditor::eventFilter(): Got Event_Leave\n");
	    DEBUG_MAKE_INDENT(3);

	       // We don't zero cur_hlink here because it's used in actions executed
	       // from a popup menu (when LeaveNotify is generated by X11).
	       // Nor do we call QDBase::eventFilter(). The reason is that
	       // KDE in "Focus follows mouse" mode generates Event_Leave,
	       // Event_Enter sequence, which clears cur_hlink in QDBase
	    showStatus(" ");
	    return true;
	 }

	 case Event_KeyPress:
	 {
	    DEBUG_MSG("QDEditor::eventFilter(): Got Event_KeyPress\n");
	    DEBUG_MAKE_INDENT(3);
	    
	    if (!isLensVisible())
	    {
	       QKeyEvent * ev=(QKeyEvent *) e;
	       switch(ev->key())
	       {
		  case Key_F:
		  case Key_F3:
		     search();
		     break;
		  
		  case Key_Home:
		     if (ev->state() & ControlButton)
		     {
			gotoPage(0); return TRUE;
		     }
		     break;

		  case Key_End:
		     if (ev->state() & ControlButton)
		     {
			gotoPage(doc_pages-1);
			return TRUE;
		     }
		     break;

		  case Key_PageUp:
		     if (rectDocument.ymin>=rectVisible.ymin && doc_page>0)
		     {
			gotoPage(doc_page-1);
			return TRUE;
		     }
		     break;

		  case Key_Backspace:
		     if (doc_page>0)
		     {
			gotoPage(doc_page-1);
			return TRUE;
		     }
		     break;

		  case Key_PageDown:
		  case Key_Return:
		  case Key_Enter:
		     if (rectDocument.ymax<=rectVisible.ymax &&
			 doc_page<doc_pages-1)
		     {
			gotoPage(doc_page+1);
			return TRUE;
		     }
		     break;

		  case Key_Space:
		     if (doc_page<doc_pages-1)
		     {
			gotoPage(doc_page+1);
			return TRUE;
		     }
		     break;

		  case Key_Escape:
		     HL_AbortMotion();
		     return TRUE;

		  case Key_Delete:
		     HL_Del();
		     return TRUE;
	       }
	    }
	    break;
	 }
         default: // DANGER DANGER DANGER
           // ingored
           break;
      } // switch(e->type())
   } catch(const GException & exc)
   {
      showError(this, exc);
   }
   return QDBase::eventFilter(obj, e);
}

void
QDEditor::print(int what)
{
   if (dimg && dimg->get_width() && dimg->get_height())
   {
      GRect prn_rect=rectVisible;
      ma_mapper.unmap(prn_rect);
      QDPrintDialog print((GP<DjVuDocument> &)djvu_doc,
			  dimg, &prefs, getMode(), getZoom(),
			  prn_rect, this, "djvu_print", TRUE);
      print.setPrint(what==IDC_PRINT_WIN ? QDPrintDialog::PRINT_WIN :
		     what==IDC_PRINT_CUSTOM ? QDPrintDialog::PRINT_CUSTOM :
		     what==IDC_PRINT_DOC ? QDPrintDialog::PRINT_DOC :
		     QDPrintDialog::PRINT_PAGE);
      print.exec();
   }
}

void
QDEditor::setCursor(void)
{
   DEBUG_MSG("QDEditor::setCursor(): Checking what cursor to install...\n");
   DEBUG_MAKE_INDENT(3);

   if (!dimg)
   {
      pane->setCursor(*(const QCursor *)cur_ptr);
      return;
   }
	 
   if (cur_editor_mode!=PREVIEW && !isLensVisible())
   {
      if (cur_editor_mode==CREATE)
      {
	 DEBUG_MSG("installing cur_ptr cursor\n");
	 pane->setCursor(*(const QCursor *)cur_ptr);
	 cur_last=cur_ptr;
	 return;
      }
      if (cur_map_area)
      {
	 DEBUG_MSG("installing cur_map_area cursor\n");
	 GPQCursor cur_new=cur_map_area->getCursor(cur_marea_vic_code, cur_marea_vic_data);
	 pane->setCursor(*(const QCursor *)cur_new);
	 cur_last=cur_new;
	 return;
      }
   }

   QDBase::setCursor();
}

void
QDEditor::setEditorMode(EDITOR_MODE mode)
{
   if (mode!=cur_editor_mode)
   {
      if (cur_editor_mode==CREATE)
	 HL_CancelNewMapArea();
      if (thumbnails)
	 thumbnails->setEnabled(mode!=CREATE);
      for(GPosition pos=map_areas;pos;++pos)
      {
	 GP<MapArea> ma=map_areas[pos];
	 ma->enableEditControls(mode!=PREVIEW, false);
	 ma->setForceAlwaysActive(mode!=PREVIEW);
	 if (mode==PREVIEW) ma->setActive(false, false);
	 ma->repaint();
      }
      
      cur_editor_mode=mode;

      //setItemsEnabled(main_window->menu, cur_editor_mode!=CREATE);
      main_window->updateMainMenus(cur_editor_mode!=CREATE);
      
      updateToolBar();
      updateEditToolBar();
      showStatus(cur_editor_mode==EDIT ? tr("Edit mode selected") :
		 cur_editor_mode==PREVIEW ? tr("Preview mode selected") :
		 tr("Creating map area..."));
      setCursor();
   }
}

//****************************************************************************
//******************************** Search ************************************
//****************************************************************************

void
QDEditor::slotSearchClosed(void)
{
   eraseSearchResults();
   qeApp->killWidget((QWidget *) sender());
}

void
QDEditor::slotDisplaySearchResults(int page_num, const GList<DjVuTXT::Zone *> & zones_list)
{
   eraseSearchResults();

   if (zones_list.size())
   {
	 // Switch the page. Here it's easy. The function will return
	 // only after the page is completely decoded.
      gotoPage(page_num);

      displaySearchResults(zones_list);
   }
}

void
QDEditor::search(void)
{
   if (dimg)
   {
      if (!search_parent) search_parent=new QWidget(this, "search_parent");

      QDSearchDialog * d;
      d=new QDSearchDialog(djvu_doc->url_to_page(dimg->get_djvu_file()->get_url()),
			   (GP<DjVuDocument> &)djvu_doc, search_parent, "search", FALSE);
      connect(d, SIGNAL(sigClosed(void)), this, SLOT(slotSearchClosed(void)));
      connect(d, SIGNAL(sigCancelled(void)), this, SLOT(slotSearchClosed(void)));
      connect(d, SIGNAL(sigDisplaySearchResults(int, const GList<DjVuTXT::Zone *> &)),
	      this, SLOT(slotDisplaySearchResults(int, const GList<DjVuTXT::Zone *> &)));
      connect(this, SIGNAL(sigPageChanged(int)), d, SLOT(slotSetPageNum(int)));
      d->show();
   }
}

//****************************************************************************
//******************************* Toolbar ************************************
//****************************************************************************

void
QDEditor::slotGotoPage(int page)
{
   try
   {
      gotoPage(page);
   } catch(const GException & exc)
   {
      ::showError(this, exc);
   }
}

//  void
//  QDEditor::slotDoCmd(int cmd)
//  {
//     try
//     {
//        processCommand(cmd);
//     } catch(const GException & exc)
//     {
//        ::showError(this, exc);
//     }
//  }

void
QDEditorMainWindow::slotGotoPage(int page)
{
   try
   {
      QDEditor *ed = (QDEditor *)ws->activeWindow();
      if ( ed )
	 ed->gotoPage(page);
   } catch(const GException & exc)
   {
      ::showError(this, exc);
   }
}

void
QDEditorMainWindow::slotDoCmd(int cmd)
{
   try
   {
      processCommand(cmd);
   } catch(const GException & exc)
   {
      ::showError(this, exc);
   }
}

void
QDEditor::updateToolBar(void)
{   
   DEBUG_MSG("QDEditor::updateToolBar\n");

   if ( !main_window ) return;
   
   if (main_window->mode_tbar)
      main_window->mode_tbar->update(getMode(), dimg && dimg->is_legal_compound(),
				     cmd_zoom, getZoom());
   
   if (main_window->nav_tbar)
      if (dimg)
      {
	 int doc_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
	 int doc_pages=djvu_doc->get_pages_num();
	 if (cur_editor_mode==CREATE)
	 {
	    main_window->nav_tbar->update(doc_page, doc_pages);
	    main_window->nav_tbar->setEnabled(FALSE);
	 } else
	 {
	    main_window->nav_tbar->setEnabled(TRUE);
	    main_window->nav_tbar->update(doc_page, doc_pages);
	 }
      }
      else
      {
	 main_window->nav_tbar->update(-1, 0);
      }
}


void
QDEditorMainWindow::slotSetZoom(int cmd_zoom)
{
   //if (getCMDZoom()!=cmd_zoom) // tmp disabled
      setZoom(cmd_zoom, true, QDBase::ZOOM_MANUAL);
}

void
QDEditorMainWindow::slotSetMode(int cmd_mode)
{
   //if (getMode()!=cmd_mode) // tmp disabled
      setMode(cmd_mode, true, QDBase::MODE_MANUAL);
}

void
QDEditorMainWindow::setMode(int cmd, bool do_redraw, int src)
{
   QDEditor *ed = (QDEditor *)ws->activeWindow();
   if ( ed )
      ed->setMode(cmd, do_redraw, src);
}

void
QDEditorMainWindow::setZoom(int cmd, bool do_layout, int src)
{
   QDEditor *ed = (QDEditor *)ws->activeWindow();
   if ( ed )
      ed->setZoom(cmd, do_layout, src);
}


//****************************************************************************
//******************************* Thumbnails *********************************
//****************************************************************************

void
QDEditor::generateThumbnails(void)
{
   QDThumbGen gen(djvu_doc, this, "thumb_gen");
   if (thumbnails)
   {
      connect(&gen, SIGNAL(sigProgress(int)),
	      thumbnails, SLOT(slotReloadPage(int)));      
      connect(&gen, SIGNAL(sigDocModified(void)),
	      this, SLOT(slotDocModified(void)));
   }
      
   gen.exec();
}

void
QDEditor::removeThumbnails(void)
{
   if (QMessageBox::warning(this, "DjVu",
			    tr("This will erase all integrated thumbnails.\n\n")+
			    tr("Do you want to proceed?"),
			    tr("&Yes"), tr("&No"), 0, 0, 1)==0)
   {
      djvu_doc->remove_thumbnails();
      setDocModified(true);
   }
}

bool
QDEditor::checkThumbnails(void)
      // Will generate or remove thumbnails
      // Will return TRUE if saving procedure should go on
{
   int thumbnails=djvu_doc->get_thumbnails_num();
   if (thumbnails>0 && thumbnails<djvu_doc->get_pages_num())
   {
      QString msg=tr("Some pages of this DjVu document have thumbnails, some don't.\n")+
	 tr("We can either generate missing thumbnail images now or\n")+
	 tr("remove them completely.\n");
      switch(QMessageBox::information(this, tr("DjVu: Thumbnails"),
				      msg, tr("&Generate"), tr("&Remove"),
				      tr("&Cancel"), 0, 2))
      {
	 case 0:
	    generateThumbnails();
	    break;
	 case 1:
	    removeThumbnails();
	    break;
	 default:
	 case 2:
	    return false;
      }
   }
   return true;
}

void
QDEditor::slotInsertPage(int page_num)
{
   insertPage(page_num,"*.djvu");
}

void
QDEditor::slotImportPage(int page_num)
{
   insertPage(page_num,"*.*");
}


void
QDEditor::slotRemovePages(const GList<int> & page_list)
{
   removePages(page_list);
}

void
QDEditor::slotMovePages(const GList<int> & page_list)
{
   if ( page_list.size() < 1 )
      return;
   else if ( page_list.size() == 1 )
      movePage(page_list[page_list.firstpos()]);
   else
      movePages(page_list);
}


void
QDEditor::slotSavePagesAs(const GList<int> & page_list)
{
   if ( page_list.size() < 1 )
      return;
   else if ( page_list.size() == 1 )
      savePageAs(page_list[page_list.firstpos()]);
   else
      savePagesAs(page_list);
}

void
QDEditor::slotCopy(const GList<int> & page_list)
{
   DEBUG_MSG("QDEditor::slotCopy\n");
   QApplication::clipboard()->setData(new QDClipBoardDjVuData(this, djvu_doc, page_list)); 
}

void
QDEditor::slotCut(const GList<int> & page_list)
{
   DEBUG_MSG("QDEditor::slotCut\n");
   QApplication::clipboard()->setData(new QDClipBoardDjVuData(this, djvu_doc, page_list)); 
   removePages(page_list);
}

void
QDEditor::slotPasteBefore(void)
{
   DEBUG_MSG("QDEditor::slotPasteBefore\n");
   int cur_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
   if ( cur_page >= 0 )
      paste(cur_page);
}

void
QDEditor::slotPasteAfter(void)
{
   DEBUG_MSG("QDEditor::slotPasteAfter\n");
   int cur_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
   if ( cur_page < djvu_doc->get_pages_num() )
      paste(++cur_page);
}
 
void
QDEditor::paste(int _atPage)
{
   DEBUG_MSG("QDEditor::paste\n");

   G_TRY {
      QDEditorBusy busy;

      QDClipBoardDjVuData * djvu_cb_data= dynamic_cast<QDClipBoardDjVuData *>(QApplication::clipboard()->data());

      if ( !djvu_cb_data ) return;

      QByteArray a= djvu_cb_data->getEncodedData();

      char *cdata=a.data();

      if ( !cdata )
	 G_THROW("Clipboard contains no data");
      //get the number of pages
      short pages;
      memcpy(&pages, cdata, sizeof(short));
      cdata += sizeof(short);

      int atPage=_atPage;
      for( int i=0; i<pages; i++)
      {
	 GUTF8String name;
	 GP<DataPool> pool=DataPool::create();

	 name = cdata;
	 cdata += name.length()+1;
	 long size;
	 memcpy(&size, cdata, sizeof(size));
	 cdata += sizeof(size);
	 pool->add_data(cdata, size);
	 cdata += size;

	 // add page now
	 djvu_doc->insert_page(pool, GURL::Filename::UTF8(name), atPage);
	 atPage++;
      }

      setDocModified(true);

      if (thumbnails)
      {
	 thumbnails->rescan(true);
      }
   } G_CATCH(exc) {
      showError(this, "DjVu Error", QString("Failed to paste"));
   }
   G_ENDCATCH;
      
}


void
QDEditor::slotCloseThumbnails(void)
{
   hideThumbnails();
}

QWidget *
QDEditor::createThumbnails(bool _rowMajor)
{
   if (!thumbnails)
   {
      thumbnails=new QDEditorThumbnails(this, main_window->canCompress, "thumbnails", _rowMajor);
      thumbnails->setFastMode(prefs.fastThumb);
      connect(thumbnails, SIGNAL(sigGotoPage(int)), this, SLOT(slotGotoPage(int)));
      connect(thumbnails, SIGNAL(sigInsert(int)), this, SLOT(slotInsertPage(int)));
      connect(thumbnails, SIGNAL(sigImport(int)), this, SLOT(slotImportPage(int)));
      connect(thumbnails, SIGNAL(sigRemove(const GList<int> &)),
	      this, SLOT(slotRemovePages(const GList<int> &)));
      connect(thumbnails, SIGNAL(sigMove(const GList<int> &)),
	      this, SLOT(slotMovePages(const GList<int> &)));
      connect(thumbnails, SIGNAL(sigSaveAs(const GList<int> &)),
	      this, SLOT(slotSavePagesAs(const GList<int> &)));
      connect(thumbnails, SIGNAL(sigCloseThumbnails(void)),
	      this, SLOT(slotCloseThumbnails(void)));
      connect(thumbnails, SIGNAL(sigCopy(const GList<int> &)),
	      this, SLOT(slotCopy(const GList<int> &)));
      connect(thumbnails, SIGNAL(sigCut(const GList<int> &)),
	      this, SLOT(slotCut(const GList<int> &)));
      connect(thumbnails, SIGNAL(sigPasteBefore(void)), this, SLOT(slotPasteBefore(void)));
      connect(thumbnails, SIGNAL(sigPasteAfter(void)), this, SLOT(slotPasteAfter(void)));
      
      if (!!djvu_doc)
      {
	 thumbnails->setDjVuDocument(djvu_doc);
	 if (dimg)
	    thumbnails->setCurPageNum(djvu_doc->url_to_page(dimg->get_djvu_file()->get_url()));
      
      }
   }
   return thumbnails;
}


bool
QDEditor::getFileDataFileName(int page, GP<DataPool> &data, GUTF8String &name)
{
   if( page < 0 || page >= djvu_doc->get_pages_num() )
      return false;

   GP<DjVuFile> file = 0;
   if( doc_modified && dimg && page == djvu_doc->url_to_page(dimg->get_djvu_file()->get_url()) )
   {
      copyAnnoBack();
   }
   
   file = djvu_doc->get_djvu_file(page);

   if( !file )
      return false;

   data = file->get_djvu_data(true, true);
   
   GURL docURL=dimg->get_djvu_file()->get_url();
   name = GOS::basename(docURL.UTF8Filename());
   
   return true;
}


void
QDEditor::OCRProcessDocument() 
{
   djvu_rtk rtk;

   if (rtk.has_rtk())
   {
      QDEditorBusy busy;
      int i;
      int numPages=djvu_doc->get_pages_num();
      int cur_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());

      for (i = 0; i < numPages; i++)
      {
	 GP<DjVuFile> file = djvu_doc->get_djvu_file(i);
	 //file->wait_for_finish();
	 
	 GP<DjVuImage> fimg = djvu_doc->get_page(i);
	 if(fimg)
	 {
	    GP<DjVuTXT> txt = rtk.init(0,*fimg).get_text();
	    file->change_text(txt,false);
	 }
      }
      
      // reload file
      setDocModified(true);
      //thumbnails->reloadPage(cur_page);
      gotoPage(cur_page);
   }
   else
   {
      QString msg=tr("Failed to start the OCR engine");
      showError(this, "DjVu Error", msg);
   }
}


void
QDEditor::OCRProcessPage() 
{
   djvu_rtk rtk;
   
   if (rtk.has_rtk())
   {
      QDEditorBusy busy;	 
      page_text->txt = rtk.init(0,*dimg).get_text();      
      setDocModified(true);	 
   }
   else
   {
      QString msg=tr("Failed to start the OCR engine");
      showError(this, "DjVu Error", msg);
   }
}

// END OF FILE
