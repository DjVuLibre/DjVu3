//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_editor_menu.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_editor_menu.h"
#include "debug.h"
#include "qx_imager.h"
#include "djvu_editor_res.h"
#include "qd_bg_col_dialog.h"
#include "qd_align_dialog.h"
#include "qd_mode_dialog.h"
#include "qd_zoom_dialog.h"
#include "qd_editor_prefs.h"
#include "qd_nav_goto_page.h"
#include "qd_set_zoom.h"
#include "DjVuDocument.h"
#include "qlib.h"
#include "qworkspace.h"
#include "exc_msg.h"
#include "qd_editor_thumb.h"

#include <qapplication.h>

#include "qt_fix.h"

static const char * djvuFilters[]={ "DjVu (*.djvu)","DjVu (*.djv)","IW44 (*.iw44)","(*.iw4)", "All files (*)", 0 };
static const char * importFilters[]={ "*.*", "All files (*)", 0 };      

void
QDEditor::createPopupMenu(void)
{
   DEBUG_MSG("QDEditor::createPopupMenu(): doing the stuff\n");
   DEBUG_MAKE_INDENT(3);
   
   popup_menu=new QPopupMenu(0, "djvu_popup_menu");
   connect(popup_menu, SIGNAL(activated(int)), this, SLOT(slotPopupCB(int)));
   QPopupMenu * displ_pane=new QPopupMenu(0, "displ_pane");
   connect(displ_pane, SIGNAL(activated(int)), popup_menu, SIGNAL(activated(int)));
   displ_pane->setCheckable(TRUE);
   displ_pane->insertItem(tr("&Color"), IDC_DISPLAY_COLOR);
   displ_pane->insertItem(tr("Black and &White"), IDC_DISPLAY_BLACKWHITE);
   displ_pane->insertItem(tr("&Background"), IDC_DISPLAY_BACKGROUND);
   displ_pane->insertItem(tr("&Foreground"), IDC_DISPLAY_FOREGROUND);
   popup_menu->insertItem(tr("&Display"), displ_pane, IDC_DISPLAY);

   QPopupMenu * zoom_pane=new QPopupMenu(0, "zoom_pane");
   connect(zoom_pane, SIGNAL(activated(int)), popup_menu, SIGNAL(activated(int)));
   zoom_pane->setCheckable(TRUE);
   zoom_pane->insertItem("&300 %", IDC_ZOOM_300);
   zoom_pane->insertItem("150 %", IDC_ZOOM_150);
   zoom_pane->insertItem("&100 %", IDC_ZOOM_100);
   zoom_pane->insertItem("&75 %", IDC_ZOOM_75);
   zoom_pane->insertItem("&50 %", IDC_ZOOM_50);
   zoom_pane->insertItem("&25 %", IDC_ZOOM_25);
   zoom_pane->insertItem(tr("&Custom..."), IDC_ZOOM_CUSTOM);
   zoom_pane->insertSeparator();
   zoom_pane->insertItem(tr("&One to One"), IDC_ZOOM_ONE2ONE);
   zoom_pane->insertItem(tr("&Stretch"), IDC_ZOOM_STRETCH);
   zoom_pane->insertItem(tr("Fit &Width"), IDC_ZOOM_WIDTH);
   zoom_pane->insertItem(tr("Fit &Page"), IDC_ZOOM_PAGE);
   zoom_pane->insertSeparator();
   zoom_pane->insertItem(tr("Zoom &In"), IDC_ZOOM_ZOOMIN);
   zoom_pane->insertItem(tr("Zoom &Out"), IDC_ZOOM_ZOOMOUT);
   popup_menu->insertItem(tr("&Zoom"), zoom_pane, IDC_ZOOM);

   QPopupMenu * nav_pane=new QPopupMenu(0, "nav_pane");
   connect(nav_pane, SIGNAL(activated(int)), popup_menu, SIGNAL(activated(int)));
   nav_pane->insertItem(tr("&Next Page"), IDC_NAV_NEXT_PAGE);
   nav_pane->insertItem(tr("&Previous Page"), IDC_NAV_PREV_PAGE);
   nav_pane->insertItem(tr("&+10 Pages"), IDC_NAV_NEXT_PAGE10);
   nav_pane->insertItem(tr("&-10 Pages"), IDC_NAV_PREV_PAGE10);
   nav_pane->insertItem(tr("&First Page"), IDC_NAV_FIRST_PAGE);
   nav_pane->insertItem(tr("&Last Page"), IDC_NAV_LAST_PAGE);
   nav_pane->insertItem(tr("&Goto Page..."), IDC_NAV_GOTO_PAGE);
   popup_menu->insertItem(tr("&Navigate"), nav_pane, IDC_NAVIGATE);

   popup_menu->insertSeparator();

   popup_menu->setCheckable(TRUE);
   popup_menu->insertItem(tr("&Find"), IDC_SEARCH);
   popup_menu->insertItem(tr("Page &Information"), IDC_ABOUT_PAGE);
   popup_menu->insertItem(tr("Show &Thumbnails"), IDC_THUMB_SHOW);

   popup_menu->insertSeparator();

   QPopupMenu * hlinks_pane=new QPopupMenu(0, "hlinks_pane");
   connect(hlinks_pane, SIGNAL(activated(int)), popup_menu, SIGNAL(activated(int)));
   hlinks_pane->insertItem(tr("&Edit"), IDC_HLINKS_EDIT);
   hlinks_pane->insertItem(tr("&Delete"), IDC_HLINKS_DEL);
   QPopupMenu * new_hlinks_pane=new QPopupMenu(0, "new_hlinks_pane");
   connect(new_hlinks_pane, SIGNAL(activated(int)), popup_menu, SIGNAL(activated(int)));
   new_hlinks_pane->insertItem(tr("&Rectangle"), IDC_HLINKS_NEW_RECT);
   new_hlinks_pane->insertItem(tr("&Polygon"), IDC_HLINKS_NEW_POLY);
   new_hlinks_pane->insertItem(tr("&Oval"), IDC_HLINKS_NEW_OVAL);
   hlinks_pane->insertItem(tr("&Create"), new_hlinks_pane, IDC_HLINKS_NEW);
   popup_menu->insertItem(tr("&Hyperlinks"), hlinks_pane, IDC_HLINKS);

   QPopupMenu * hilite_pane=new QPopupMenu(0, "hilite_pane");
   connect(hilite_pane, SIGNAL(activated(int)), popup_menu, SIGNAL(activated(int)));
   hilite_pane->insertItem(tr("&Edit"), IDC_HILITE_EDIT);
   hilite_pane->insertItem(tr("&Delete"), IDC_HILITE_DEL);
   hilite_pane->insertItem(tr("&Create"), IDC_HILITE_NEW_RECT);
   popup_menu->insertItem(tr("H&ighlighted areas"), hilite_pane, IDC_HILITE);

   popup_menu->insertSeparator();

   popup_menu->insertItem(tr("Save D&ocument As ..."), IDC_SAVE_DOC_AS);
   popup_menu->insertItem(tr("Save Do&cument ..."), IDC_SAVE_DOC);
   popup_menu->insertItem(tr("Save P&age As ..."), IDC_SAVE_PAGE_AS);
   if ( main_window->canCompress )
      popup_menu->insertItem(tr("&Compress Document ..."), IDC_COMPRESS_DOC);

   popup_menu->insertItem(tr("&Export page"), IDC_EXPORT_PAGE);
   
   popup_menu->insertSeparator();
   
   popup_menu->insertItem(tr("Print Pa&ge"), IDC_PRINT_PAGE);
   popup_menu->insertItem(tr("&Print Document"), IDC_PRINT_DOC);
   popup_menu->insertItem(tr("Print &Window"), IDC_PRINT_WIN);

   popup_menu->insertSeparator();
   
   popup_menu->insertItem(tr("P&references ..."), IDC_PREFERENCES);
}

void
QDEditor::setupMenu(QMenuData * menu, QMenuData * pane)
{
   DEBUG_MSG("QDEditor::setupMenu(): adjusting the menu\n");
   DEBUG_MAKE_INDENT(3);

   int pane_id=paneToID(menu, pane);
   if (!pane) pane=menu;
   
   if (!dimg || !dimg->get_width() || !dimg->get_height())
   {
	 // Do everything insensitive except for some stuff
      setItemsEnabled(pane, FALSE);

      pane->setItemEnabled(IDC_OPEN, TRUE);
      pane->setItemEnabled(IDC_IMPORT, TRUE);
      pane->setItemEnabled(IDC_EXIT, TRUE);
      pane->setItemEnabled(IDC_FILE, TRUE);
      pane->setItemEnabled(IDC_PAGE_INSERT_AFTER, TRUE);
      pane->setItemEnabled(IDC_PAGE_IMPORT_AFTER, TRUE);
      pane->setItemEnabled(IDC_HELP, TRUE);
      pane->setItemEnabled(IDC_ABOUT, TRUE);
      pane->setItemEnabled(IDC_WINDOWS, TRUE);
      extern char *djedit_help_script;
      pane->setItemEnabled(IDC_CONTENTS, (djedit_help_script?TRUE:FALSE));
   } else
   {
	 // Enable everything
      setItemsEnabled(pane, TRUE);

      if (pane_id==IDC_FILE)
      {
	 pane->setItemEnabled(IDC_THUMB_REMOVE, djvu_doc->get_thumbnails_num()>0);
	 if (thumbnails)
	    pane->setItemEnabled(IDC_SAVE_PAGES_AS, thumbnails->getSelectedItemsNum()>1);
      }

      pane->setItemChecked(IDC_DISPLAY_COLOR, getMode()==IDC_DISPLAY_COLOR);
      pane->setItemChecked(IDC_DISPLAY_BLACKWHITE, getMode()==IDC_DISPLAY_BLACKWHITE);
      pane->setItemChecked(IDC_DISPLAY_FOREGROUND, getMode()==IDC_DISPLAY_FOREGROUND);
      pane->setItemChecked(IDC_DISPLAY_BACKGROUND, getMode()==IDC_DISPLAY_BACKGROUND);

      pane->setItemEnabled(IDC_DISPLAY_COLOR, !dimg->is_legal_bilevel());
      pane->setItemEnabled(IDC_DISPLAY_BLACKWHITE, !dimg->is_legal_photo());
      pane->setItemEnabled(IDC_DISPLAY_BACKGROUND, dimg->is_legal_compound());
      pane->setItemEnabled(IDC_DISPLAY_FOREGROUND, dimg->is_legal_compound());
      
      pane->setItemChecked(IDC_ZOOM_25, getCMDZoom()==IDC_ZOOM_25);
      pane->setItemChecked(IDC_ZOOM_50, getCMDZoom()==IDC_ZOOM_50);
      pane->setItemChecked(IDC_ZOOM_75, getCMDZoom()==IDC_ZOOM_75);
      pane->setItemChecked(IDC_ZOOM_100, getCMDZoom()==IDC_ZOOM_100);
      pane->setItemChecked(IDC_ZOOM_150, getCMDZoom()==IDC_ZOOM_150);
      pane->setItemChecked(IDC_ZOOM_300, getCMDZoom()==IDC_ZOOM_300);
      pane->setItemChecked(IDC_ZOOM_ONE2ONE, getCMDZoom()==IDC_ZOOM_ONE2ONE);
      pane->setItemChecked(IDC_ZOOM_STRETCH, getCMDZoom()==IDC_ZOOM_STRETCH);
      pane->setItemChecked(IDC_ZOOM_WIDTH, getCMDZoom()==IDC_ZOOM_WIDTH);
      pane->setItemChecked(IDC_ZOOM_PAGE, getCMDZoom()==IDC_ZOOM_PAGE);

      pane->setItemChecked(IDC_ZOOM_CUSTOM,
			   !pane->isItemChecked(IDC_ZOOM_25) &&
			   !pane->isItemChecked(IDC_ZOOM_50) &&
			   !pane->isItemChecked(IDC_ZOOM_75) &&
			   !pane->isItemChecked(IDC_ZOOM_100) &&
			   !pane->isItemChecked(IDC_ZOOM_150) &&
			   !pane->isItemChecked(IDC_ZOOM_300) &&
			   !pane->isItemChecked(IDC_ZOOM_ONE2ONE) &&
			   !pane->isItemChecked(IDC_ZOOM_STRETCH) &&
			   !pane->isItemChecked(IDC_ZOOM_WIDTH) &&
			   !pane->isItemChecked(IDC_ZOOM_PAGE));

	 // ZoomIn and ZoomOut cases
      pane->setItemEnabled(IDC_ZOOM_ZOOMIN, getZoom()<IDC_ZOOM_MAX-IDC_ZOOM_MIN);
      pane->setItemEnabled(IDC_ZOOM_ZOOMOUT, getZoom()>5);

	 // Doing 'Navigate'
      int doc_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
      int doc_pages=djvu_doc->get_pages_num();
      pane->setItemEnabled(IDC_NAVIGATE, doc_pages>1);
      pane->setItemEnabled(IDC_NAV_PREV_PAGE, doc_page>0);
      pane->setItemEnabled(IDC_NAV_NEXT_PAGE, doc_page<=doc_pages-2);
      pane->setItemEnabled(IDC_NAV_PREV_PAGE10, doc_page>=10);
      pane->setItemEnabled(IDC_NAV_NEXT_PAGE10, doc_page<=doc_pages-11);
      pane->setItemEnabled(IDC_NAV_FIRST_PAGE, doc_page>0);
      pane->setItemEnabled(IDC_NAV_LAST_PAGE, doc_page!=doc_pages-1);

      pane->setItemEnabled(IDC_HLINKS_EDIT, cur_map_area &&
			   cur_map_area->isHyperlink());
      pane->setItemEnabled(IDC_HLINKS_DEL, cur_map_area &&
			   cur_map_area->isHyperlink());

      pane->setItemEnabled(IDC_HILITE_EDIT, cur_map_area &&
			   !cur_map_area->isHyperlink());
      pane->setItemEnabled(IDC_HILITE_DEL, cur_map_area &&
			   !cur_map_area->isHyperlink());

      pane->setItemEnabled(IDC_SAVE_DOC, djvu_doc->can_be_saved());
      pane->setItemEnabled(IDC_SAVE_DOC_AS, !djvu_doc->needs_compression());
      pane->setItemEnabled(IDC_COMPRESS_DOC, djvu_doc->can_compress());
      pane->setItemEnabled(IDC_SAVE_PAGE, !djvu_doc->needs_compression());
      pane->setItemEnabled(IDC_SAVE_PAGE_AS, !djvu_doc->needs_compression());

      pane->setItemChecked(IDC_THUMB_SHOW, thumbnailsShown());
   }
}

void
QDEditor::runPopupMenu(QMouseEvent * ev)
{
   DEBUG_MSG("QDEditor::runPopupMenu(): adjusting and showing the menu\n");
   DEBUG_MAKE_INDENT(3);

   if (!dimg) return;
   
   try
   {
      setupMenu(popup_menu);

	 // Strange as it may seem, but I can't process popup menu commands
	 // directly from a slot connected to the proper activate() signal.
	 // The reason is the fact, that QT dislikes when a modal dialog
	 // is created while the popup menu is still running.
      popup_menu_id=-1;
      popup_menu->exec(QCursor::pos());
      if (popup_menu_id>=0) processCommand(popup_menu_id);
   } catch(const GException & exc)
   {
      showError(this, exc);
   }
}

void
QDEditorMainWindow::processCommand(int cmd)
{
   DEBUG_MSG("QDEditorMainWindow::processCommand(): cmd=" << cmd << "\n");
   switch(cmd)
   {
   case IDC_OPEN:
      openDocument(djvuFilters);
      break;
   case IDC_EXIT:
      close();
      break;
   case IDC_ABOUT:
      about();
      break;      
   case IDC_CONTENTS:
      contents();
      break;

   default:
      QDEditor* m = (QDEditor *)ws->activeWindow();
      if ( m ) 
	 m->processCommand(cmd);
   }
}

void
QDEditor::processCommand(int cmd)
{
   DEBUG_MSG("QDEditor::processCommand(): cmd=" << cmd << "\n");
   DEBUG_MAKE_INDENT(3);

   try
   {
      int doc_page=0, doc_pages=1;
      if (dimg)
      {
	 doc_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
	 doc_pages=djvu_doc->get_pages_num();
      }
      switch(cmd)
      {

	 // temp disabled
//  	 case IDC_IMPORT: 
//  	    openDocument(importFilters);
//  	    break;

      case IDC_CLOSE:
	 close();
	 break;

      case IDC_DOC_BACK_COLOR:
	 {
	    u_int32 bg_color=0xffffffff;
	    if (shared_anno && shared_anno->ant)
	       bg_color=shared_anno->ant->bg_color;
	    
	    QDBGColorDialog d(bg_color, this,
			      "bg_color_dialog", TRUE);
	    if (d.exec()==QDialog::Accepted)
	    {
	       u_int32 back_color=d.color();
	       if (back_color!=bg_color)
	       {
		  if (!shared_anno_file)
		     createSharedAnno();
		  shared_anno->ant->bg_color=back_color;
		  copyAnnoBack();
		  if (page_anno->ant->bg_color==0xffffffff)
		     setBackgroundColor(back_color, true);	// Redraw
	       }
	    }
	    break;
	 }

      case IDC_DOC_ALIGNMENT:
	 {
	    int hor_align=DjVuANT::ALIGN_UNSPEC;
	    int ver_align=DjVuANT::ALIGN_UNSPEC;
	    if (shared_anno && shared_anno->ant)
	    {
	       hor_align=shared_anno->ant->hor_align;
	       ver_align=shared_anno->ant->ver_align;
	    }
	    QDAlignDialog d(hor_align, ver_align,
			    this, "align_dialog", TRUE);
	    if (d.exec()==QDialog::Accepted)
	    {
	       if (d.horAlignment()!=hor_align ||
		   d.verAlignment()!=ver_align)
	       {
		  if (!shared_anno_file)
		     createSharedAnno();
		  shared_anno->ant->hor_align=d.horAlignment();
		  shared_anno->ant->ver_align=d.verAlignment();
		  copyAnnoBack();
		  layout(1);
	       }
	    }
	    break;
	 }
	 
      case IDC_BACK_COLOR:
	 {
	    QDBGColorDialog d(page_anno->ant->bg_color, this,
			      "bg_color_dialog", TRUE);
	    if (d.exec()==QDialog::Accepted)
	    {
	       u_int32 back_color=d.color();
	       if (back_color!=page_anno->ant->bg_color)
	       {
		  page_anno->ant->bg_color=back_color;
		  copyAnnoBack();
		  setBackgroundColor(back_color, true);	// Redraw
	       }
	    }
	    break;
	 }

      case IDC_ALIGNMENT:
	 {
	    QDAlignDialog d(page_anno->ant->hor_align, page_anno->ant->ver_align,
			    this, "align_dialog", TRUE);
	    if (d.exec()==QDialog::Accepted)
	    {
	       if (d.horAlignment()!=page_anno->ant->hor_align ||
		   d.verAlignment()!=page_anno->ant->ver_align)
	       {
		  page_anno->ant->hor_align=d.horAlignment();
		  page_anno->ant->ver_align=d.verAlignment();
		  copyAnnoBack();
		  layout(1);
	       }
	    }
	    break;
	 }

      case IDC_START_MODE:
	 {
	    QDModeDialog d(page_anno->ant->mode, this, "mode_dialog", TRUE);
	    if (d.exec()==QDialog::Accepted)
	    {
	       int mode=d.getMode();
	       if (mode!=page_anno->ant->mode)
	       {
		  page_anno->ant->mode=mode;
		  copyAnnoBack();
	       }
	    }
	    break;
	 }

      case IDC_START_ZOOM:
	 {
	    QDZoomDialog d(page_anno->ant->zoom, this, "zoom_dialog", TRUE);
	    if (d.exec()==QDialog::Accepted)
	    {
	       int zoom=d.getZoom();
	       if (zoom!=page_anno->ant->zoom)
	       {
		  page_anno->ant->zoom=zoom;
		  copyAnnoBack();
	       }
	    }
	    break;
	 }

      case IDC_DISPLAY_COLOR:
      case IDC_DISPLAY_BLACKWHITE:
      case IDC_DISPLAY_FOREGROUND:
      case IDC_DISPLAY_BACKGROUND:
	 setMode(cmd, true, MODE_MANUAL);
	 break;

      case IDC_ZOOM_25:
      case IDC_ZOOM_50:
      case IDC_ZOOM_75:
      case IDC_ZOOM_100:
      case IDC_ZOOM_150:
      case IDC_ZOOM_300:
      case IDC_ZOOM_ONE2ONE:
      case IDC_ZOOM_STRETCH:
      case IDC_ZOOM_WIDTH:
      case IDC_ZOOM_PAGE:
      case IDC_ZOOM_ZOOMIN:
      case IDC_ZOOM_ZOOMOUT:
	 setZoom(cmd, true, ZOOM_MANUAL);
	 break;

      case IDC_ZOOM_CUSTOM:
	 {
	    QDSetZoom zoom_d(getZoom(), this, "qd_set_zoom");
	    if (zoom_d.exec()==QDialog::Accepted)
	       setZoom(zoom_d.getZoom()+IDC_ZOOM_MIN, 1, ZOOM_MANUAL);
	    break;
	 }

      case IDC_NAV_PREV_PAGE:
	 if (doc_page>0) gotoPage(doc_page-1);
	 break;

      case IDC_NAV_NEXT_PAGE:
	 if (doc_page<doc_pages-1) gotoPage(doc_page+1);
	 break;

      case IDC_NAV_PREV_PAGE10:
	 if (doc_page>=10) gotoPage(doc_page-10);
	 break;

      case IDC_NAV_NEXT_PAGE10:
	 if (doc_page<=doc_pages-11) gotoPage(doc_page+10);
	 break;

      case IDC_NAV_FIRST_PAGE:
	 if (doc_page>0) gotoPage(0);
	 break;

      case IDC_NAV_LAST_PAGE:
	 if (doc_page!=doc_pages-1) gotoPage(doc_pages-1);
	 break;

      case IDC_NAV_GOTO_PAGE:
	 {
            //GP<DjVuDocument> doc=djvu_doc;
	    QDNavGotoPage d((GP<DjVuDocument> &)djvu_doc, dimg,
			    this, "goto_page_dialog");
	    if (d.exec()==QDialog::Accepted)
	       gotoPage(d.getPageNum());
	    break;
	 }

      case IDC_HLINKS_EDIT:
	 HL_Edit();
	 break;

      case IDC_HLINKS_DEL:
	 HL_Del();
	 break;

      case IDC_HLINKS_NEW_RECT:
	 HL_Create(IDC_HLINKS_NEW_RECT);
	 break;

      case IDC_HLINKS_NEW_POLY:
	 HL_Create(IDC_HLINKS_NEW_POLY);
	 break;

      case IDC_HLINKS_NEW_OVAL:
	 HL_Create(IDC_HLINKS_NEW_OVAL);
	 break;

      case IDC_HILITE_EDIT:
	 HL_Edit();
	 break;

      case IDC_HILITE_DEL:
	 HL_Del();
	 break;
	    
      case IDC_HILITE_NEW_RECT:
	 HL_Create(IDC_HILITE_NEW_RECT);
	 break;

      case IDC_EDIT_MODE:
	 setEditorMode(EDIT);
	 break;

      case IDC_PREVIEW_MODE:
	 setEditorMode(PREVIEW);
	 break;
	    
      case IDC_PREFERENCES:
	 {
	    double dScreenGamma=prefs.dScreenGamma;
	    int fastZoom=prefs.fastZoom;
	    bool optimizeLCD=prefs.optimizeLCD;
	    QDEditorPrefs pd(&prefs, this, "djvu_prefs", TRUE);
	    if (pd.exec()==QDialog::Accepted)
	    {
	       bool do_redraw=prefs.dScreenGamma!=dScreenGamma ||
		  prefs.optimizeLCD!=optimizeLCD;
	       bool do_layout=prefs.fastZoom!=fastZoom;
	       bool repaint_thumb=prefs.dScreenGamma!=dScreenGamma;

	       qxImager->setOptimizeLCD(prefs.optimizeLCD);
	       setBackgroundColor(getBackgroundColor(), false);

	       if (prefs.toolBarOn) enableToolBar(true);
	       else enableToolBar(false);
	       if (prefs.toolBarAlwaysVisible) stickToolBar();
	       else unStickToolBar();
	       
	       bm_cache.setMaxSize(prefs.mcacheSize*1024*1024);
	       pm_cache.setMaxSize(prefs.mcacheSize*1024*1024);

	       if (thumbnailsShown())
		  thumbnails->setFastMode(prefs.fastThumb);
	       
	       qeApp->gamma=prefs.dScreenGamma;
	       if (do_layout) layout();
	       else if (do_redraw) redraw();
	       if (repaint_thumb && thumbnails) thumbnails->rereadGamma();
	    }
	    break;
	 }

      case IDC_ABOUT_PAGE:
	 if (dimg && dimg->get_width() && dimg->get_height())
	 {
	    GUTF8String desc=dimg->get_long_description();
	    showMessage(this, tr("DjVu: Page Information"), QStringFromGString(desc), 1, 1);
	 }
	 break;

      case IDC_PRINT_DOC:
      case IDC_PRINT_PAGE:
      case IDC_PRINT_WIN:
	 print(cmd);
	 break;

      case IDC_SAVE_PAGE_AS:
	 savePageAs();
	 break;

      case IDC_SAVE_PAGES_AS:
	 savePagesAs();
	 break;

      case IDC_SAVE_DOC:
	 saveDocument();
	 break;

      case IDC_SAVE_DOC_AS:
	 saveDocumentAs();
	 break;

      case IDC_COMPRESS_DOC:
	 compressDocument();
	 break;

      case IDC_EXPORT_PAGE:
	 exportToPNM();
	 break;

      case IDC_SEARCH:
	 search();
	 break;

      case IDC_PAGE_INSERT_BEFORE:
      case IDC_PAGE_INSERT_AFTER:
	 {
	    int page_num=-1;
	    if (dimg)
	    {
	       page_num=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
	       if (cmd==IDC_PAGE_INSERT_AFTER) page_num++;
	       if (page_num>=djvu_doc->get_pages_num()) page_num=-1;
	    }
	    insertPage(page_num,djvuFilters[0]);
	    break;
	 }
      case IDC_PAGE_IMPORT_BEFORE:
      case IDC_PAGE_IMPORT_AFTER:
	 {
	    int page_num=-1;
	    if (dimg)
	    {
	       page_num=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
	       if (cmd==IDC_PAGE_IMPORT_AFTER) page_num++;
	       if (page_num>=djvu_doc->get_pages_num()) page_num=-1;
	    }
	    insertPage(page_num,importFilters[0]);
	    break;
	 }
	    
      case IDC_PAGE_REMOVE:
	 removePage(djvu_doc->url_to_page(dimg->get_djvu_file()->get_url()));
	 break;

      case IDC_PAGE_MOVE:
	 movePage(djvu_doc->url_to_page(dimg->get_djvu_file()->get_url()));
	 break;

      case IDC_THUMB_SHOW:
	 {
	    if (thumbnailsShown()) hideThumbnails();
	    else showThumbnails();
	    break;
	 }

      case IDC_THUMB_GENERATE:
	 generateThumbnails();
	 break;
	    
      case IDC_THUMB_REMOVE:
	 removeThumbnails();
	 break;

      case IDC_OCR_DOC:
	 OCRProcessDocument();
	 break;
	 
      case IDC_OCR_PAGE:
	 OCRProcessPage();
	 break;
	 

      }
   } catch(const GException & exc)
   {
      showError(this, exc);
   }
}

// END OF FILE
