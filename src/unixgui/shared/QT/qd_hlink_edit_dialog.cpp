//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_hlink_edit_dialog.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuImage.h"
#include "qd_hlink_edit_dialog.h"
#include "qt_n_in_one.h"
#include "qt_color_menu.h"
#include "qlib.h"
#include "debug.h"
#include "exc_misc.h"
#include "qx_imager.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qmultilinedit.h>
#include <qcombobox.h>

#include "qt_fix.h"

#ifndef QT1
#include <q1xcompatibility.h>
#endif

//***************************************************************************
//******************************* QeHLPane **********************************
//***************************************************************************

void
QeHLPane::layout(void)
{
   if (map_area)
   {
      GRect brect=gmap_area->get_bound_rect();

      float cw=(float) width()/brect.width();
      float ch=(float) height()/brect.height();
      float coeff=cw>ch ? ch : cw;
      coeff*=0.9;

      int w=(int) (coeff*brect.width());
      int h=(int) (coeff*brect.height());
      GRect wrect((width()-w)/2, (height()-h)/2, w, h);
      mapper.clear();
      mapper.set_input(brect);
      mapper.set_output(wrect);
      mapper.mirrory();

      map_area->layout(GRect(0, 0, dimg->get_width(), dimg->get_height()));
   }
}

bool
QeHLPane::event(QEvent * ev)
{
   try
   {
      if (ev->type()==Event_User)
      {
	 DEBUG_MSG("QeHLPane::event(): resizing: width=" << width() << ", height=" << height() << "\n");
	 setMinimumWidth(height());
	 ActivateLayouts(this);
	 layout();
	 update();
	 return TRUE;
      }
   } catch(const GException & exc)
   {
      showError(this, exc);
   }
   return QWidget::event(ev);
}
   
void
QeHLPane::resizeEvent(QResizeEvent *)
{
   DEBUG_MSG("QeHLPane::resizeEvent(): delaying resize...\n");
   try
   {
      QEvent * ev=new QEvent(Event_User);
      QApplication::postEvent(this, ev);
   } catch(const GException & exc)
   {
      showError(this, exc);
   }
}

static GRect
invRect(const GRect & rect)
{
   return GRect(rect.xmin, -rect.ymax, rect.width(), rect.height());
}

void
QeHLPane::paintEvent(QPaintEvent * ev)
{
   try
   {
      if (map_area)
      {
	 GRect rectDoc(0, 0, dimg->get_width(), dimg->get_height());
	 mapper.map(rectDoc);
	 GRect rectDisp(Q2G(ev->rect()));

	 GRect irect;
	 if (irect.intersect(rectDoc, rectDisp))
	 {
	    GP<GPixmap> pm;
	    GP<GBitmap> bm;
	    QDPainter p(this);
	    if ((pm=dimg->get_pixmap(invRect(irect), invRect(rectDoc), gamma)))
	    {
		  // Map rectangle into image rectangle
	       GRect rectImg=irect;
	       mapper.unmap(rectImg);
	       map_area->draw(irect, pm, MapArea::DRAW_ACTIVE);
	       qxImager->dither(*pm, rectImg.xmin, rectImg.ymin);
	       p.drawPixmap(irect, pm, true);
	    } else
	    {
	       bm=dimg->get_bitmap(invRect(irect), invRect(rectDoc),
				   sizeof(int));
	       GRect hpm_rect;
	       GP<GPixmap> hpm;
	       map_area->draw(irect, bm, hpm_rect, hpm, MapArea::DRAW_ACTIVE);
	       if (hpm) p.drawPatchedBitmap(irect, bm, hpm_rect, hpm, true);
	       else p.drawBitmap(irect, bm, true);
	    }
	    p.end();
	 }

	 QePainter p(this);
	 QBrush brush(white);
	 
	 GRect rectBorder;
	 rectBorder=GRect(0, 0, width(), rectDoc.ymin);
	 if (irect.intersect(rectBorder, rectDisp))
	    p.fillRect(G2Q(irect), brush);
	 rectBorder=GRect(0, rectDoc.ymax, width(), height()-rectDoc.ymax);
	 if (irect.intersect(rectBorder, rectDisp))
	    p.fillRect(G2Q(irect), brush);
	 rectBorder=GRect(0, 0, rectDoc.xmin, height());
	 if (irect.intersect(rectBorder, rectDisp))
	    p.fillRect(G2Q(irect), brush);
	 rectBorder=GRect(rectDoc.xmax, 0, width()-rectDoc.xmax, height());
	 if (irect.intersect(rectBorder, rectDisp))
	    p.fillRect(G2Q(irect), brush);
      }
   } catch(const GException & exc)
   {
      showError(this, exc);
   }
   QFrame::paintEvent(ev);
}

void
QeHLPane::mapAreaChanged(void)
{
   map_area->reset();
}

QeHLPane::QeHLPane(const GP<DjVuImage> & image,
		   const GP<GMapArea> & map,
		   float gamma,
		   QWidget * parent, const char * name) :
      QFrame(parent, name), dimg(image), gmap_area(map), gamma(gamma)
{
   if (QApplication::style()==WindowsStyle)
      setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
   else
      setFrameStyle(QFrame::Panel | QFrame::Sunken);
   setMargin(0);
   setBackgroundColor(white);

   setWFlags(WResizeNoErase);
   setBackgroundMode(NoBackground);

   GUTF8String shape_name=gmap_area->get_shape_name();
   if (shape_name == GMapArea::RECT_TAG)
      map_area=new MapRect((GMapRect *) (GMapArea *) gmap_area);
   else if (shape_name == GMapArea::OVAL_TAG)
      map_area=new MapOval((GMapOval *) (GMapArea *) gmap_area);
   else if (shape_name == GMapArea::POLY_TAG)
      map_area=new MapPoly((GMapPoly *) (GMapArea *) gmap_area);
   if ( map_area )
   {
      map_area->setForceAlwaysActive(true);

      layout();
      map_area->attachWindow(this, &mapper);
   }
}

//***************************************************************************
//*************************** QDHLinkEditDialog *****************************
//***************************************************************************

#define LINK_URL	"URL"
#define LINK_PAGE	"Page number"
#define LINK_NAME	"Page file name"

#define BTYPE_NONE	"Don't Display"
#define BTYPE_XOR	"XOR Method"
#define BTYPE_SOLID	"Solid Border"
#define BTYPE_SHADOW_IN		"Shadow In"
#define BTYPE_SHADOW_OUT	"Shadow Out"
#define BTYPE_SHADOW_EIN	"Shadow Etched In"
#define BTYPE_SHADOW_EOUT	"Shadow Etched Out"

QString
QDHLinkEditDialog::url(void) const
{
   GUTF8String url=gmap_area->url;

   if (link_menu)
   {
      GUTF8String link_type=GStringFromQString(link_menu->currentText());
      if (!strcmp(link_type, LINK_URL) && url_text)
	 url=GStringFromQString(url_text->text());
      if (!strcmp(link_type, LINK_PAGE) && page_menu)
	 url=GUTF8String("#")+GUTF8String(page_menu->currentItem()+1);
      if (!strcmp(link_type, LINK_NAME) && name_menu)
	 url=GUTF8String("#")+GStringFromQString(name_menu->currentText());
   }
   return QStringFromGString(url);
}

QString
QDHLinkEditDialog::comment(void) const
{
   QString res;
   if (comment_text)
     res=comment_text->text();
   else
     res=(const char *) gmap_area->comment;
   return res;
}

int
QDHLinkEditDialog::borderWidth(void) const
{
   return (borderType()==GMapArea::SHADOW_IN_BORDER ||
	   borderType()==GMapArea::SHADOW_OUT_BORDER ||
	   borderType()==GMapArea::SHADOW_EIN_BORDER ||
	   borderType()==GMapArea::SHADOW_EOUT_BORDER) ?
			bwidth_spin->value() : 1;
}

u_int32
QDHLinkEditDialog::borderColor(void) const
{
   return bcolor_menu->color();
}

GMapArea::BorderType
QDHLinkEditDialog::borderType(void) const
{
   return (GMapArea::BorderType) btype_menu->currentItem();
}

bool
QDHLinkEditDialog::borderAlwaysVisible(void) const
{
   return bvisible_butt->isChecked();
}

u_int32
QDHLinkEditDialog::hiliteColor(void) const
{
   return hilite_menu->color();
}

QString
QDHLinkEditDialog::target(void) const
{
   if (target_menu)
   {
      int index=target_menu->currentItem();
      return
	 index==0 ? "_self" :
	  index==1 ? "_top" :
	  index==2 ? "_blank" :
		(const char *) target_text->text();
   } else return (const char *) gmap_area->target;
}

void
QDHLinkEditDialog::slotTargetChanged(int index)
{
   target_text->setEnabled(index==3);
   if (last_target_index==3) other_target=target_text->text();
   target_text->setText(index==0 ? "_self" :
			index==1 ? "_top" :
			index==2 ? "_blank" : (const char *) other_target);
   last_target_index=index;
}

void
QDHLinkEditDialog::slotLinkTypeChanged(const QString & qtype)
{
   const char * const type = qtype;
   bool url=false, page=false, name=false;
   if (!strcmp(type, LINK_PAGE))
      page=true;
   else if (!strcmp(type, LINK_NAME))
      name=true;
   else
      url=true;

   page_label->setEnabled(page);
   page_menu->setEnabled(page);

   name_label->setEnabled(name);
   name_menu->setEnabled(name);

   url_label->setEnabled(url);
   url_text->setEnabled(url);

   GUTF8String url_key;
   if (gmap_area->url[0]=='#')
      url_key=((const char *) gmap_area->url)+1;
   
   if (page)
   {
      if (url_key.length() && url_key.is_int())
	 page_menu->setCurrentItem(atoi(url_key)-1);
      slotPageChanged(page_menu->currentItem());
   }
   
   if (name)
   {
      if (url_key.length())
	 for(int i=0;i<name_menu->count();i++)
	    if (!strcmp(name_menu->text(i), url_key))
	    {
	       name_menu->setCurrentItem(i);
	       break;
	    }
      slotNameChanged(name_menu->currentItem());
   }
}

void
QDHLinkEditDialog::slotPageChanged(int page)
{
   name_menu->setCurrentItem(page);
   GUTF8String mesg=GUTF8String("#")+GUTF8String(page+1);
   url_text->setText(QStringFromGString(mesg));
}

void
QDHLinkEditDialog::slotNameChanged(int item)
{
   page_menu->setCurrentItem(item);
   GUTF8String mesg=GUTF8String("#")+GUTF8String(name_menu->currentText());
   url_text->setText(QStringFromGString(mesg));
}

void
QDHLinkEditDialog::slotBorderTypeChanged(const QString & qtype)
{ 
   const char * const type=qtype;
   int index=!strcmp(type, BTYPE_NONE) ? GMapArea::NO_BORDER :
	     !strcmp(type, BTYPE_XOR) ? GMapArea::XOR_BORDER :
	     !strcmp(type, BTYPE_SOLID) ? GMapArea::SOLID_BORDER :
	     !strcmp(type, BTYPE_SHADOW_IN) ? GMapArea::SHADOW_IN_BORDER :
	     !strcmp(type, BTYPE_SHADOW_OUT) ? GMapArea::SHADOW_OUT_BORDER :
	     !strcmp(type, BTYPE_SHADOW_EIN) ? GMapArea::SHADOW_EIN_BORDER :
	     !strcmp(type, BTYPE_SHADOW_EOUT) ? GMapArea::SHADOW_EOUT_BORDER :
	     GMapArea::NO_BORDER;
   
   switch(index)
   {
      default:
      case GMapArea::NO_BORDER:
      case GMapArea::XOR_BORDER:
	 if (bcolor_form) bcolor_form->setActiveWidget(bcolor_label);
	 if (bwidth_form) bwidth_form->setActiveWidget(bwidth_label);
	 break;

      case GMapArea::SOLID_BORDER:
	 if (bcolor_form) bcolor_form->setActiveWidget(bcolor_menu);
	 if (bwidth_form) bwidth_form->setActiveWidget(bwidth_label);
	 break;

      case GMapArea::SHADOW_IN_BORDER:
      case GMapArea::SHADOW_OUT_BORDER:
      case GMapArea::SHADOW_EIN_BORDER:
      case GMapArea::SHADOW_EOUT_BORDER:
	 if (bcolor_form) bcolor_form->setActiveWidget(bcolor_label);
	 if (bwidth_form) bwidth_form->setActiveWidget(bwidth_spin);
	 break;
   }
   
   if (index==GMapArea::NO_BORDER) bvisible_butt->setChecked(FALSE);
   bvisible_butt->setEnabled(index!=GMapArea::NO_BORDER);
   
   gmap_area->border_color=borderColor();
   gmap_area->border_width=borderWidth();
   gmap_area->border_type=(GMapArea::BorderType) index;
   preview_pane->mapAreaChanged();
}

void
QDHLinkEditDialog::slotBorderColorChanged(int)
{
   gmap_area->border_color=borderColor();
   preview_pane->mapAreaChanged();
}

void
QDHLinkEditDialog::slotBorderWidthChanged(int)
{
   gmap_area->border_width=borderWidth();
   preview_pane->mapAreaChanged();
}

void
QDHLinkEditDialog::slotHiliteColorChanged(int)
{
   gmap_area->hilite_color=hiliteColor();
   preview_pane->mapAreaChanged();
}

QeComboBox *
QDHLinkEditDialog::getBTypeMenu(QWidget * parent)
{
   QeComboBox * menu;
   menu=new QeComboBox(FALSE, parent, "btype_menu");
   menu->insertItem(BTYPE_NONE);
   menu->insertItem(BTYPE_XOR);
   menu->insertItem(BTYPE_SOLID);

   GUTF8String shape_name = gmap_area->get_shape_name();
   if (shape_name==GMapArea::RECT_TAG)
   {
      menu->insertItem(BTYPE_SHADOW_IN);
      menu->insertItem(BTYPE_SHADOW_OUT);
      menu->insertItem(BTYPE_SHADOW_EIN);
      menu->insertItem(BTYPE_SHADOW_EOUT);
   }

   if (shape_name != GMapArea::RECT_TAG &&
       gmap_area->border_type>GMapArea::SOLID_BORDER)
      gmap_area->border_type=GMapArea::SOLID_BORDER;
   menu->setCurrentItem(gmap_area->border_type);
   return menu;
}


void
QDHLinkEditDialog::done(int rc)
{
   DEBUG_MSG("QDHLinkEditDialog::done() called...\n");
   DEBUG_MAKE_INDENT(3);
   extern const char *DjEditEnterURLMessage;
   static const GUTF8String GDjEditEnterURLMessage(DjEditEnterURLMessage);
   if( rc == QDialog::Accepted && url_text &&
       (url_text->text().isEmpty() ||
	(GDjEditEnterURLMessage == GUTF8String((const char *)url_text->text()))))
   {
      showError(this, "DjVu Error", QString("A hyperlink must have non-empty URL"));
      return;
   }
   QeDialog::done(rc);
}

QDHLinkEditDialog::QDHLinkEditDialog(GP<DjVuDocEditor> & _doc,
				     const GP<DjVuImage> & _dimg,
				     const GP<GMapArea> & map,
				     float gamma, QWidget * parent,
				     const char * name, bool modal) :
      QeDialog(parent, name, modal), doc(_doc),
      dimg(_dimg), gmap_area(map->get_copy())
{
   DEBUG_MSG("QDHLinkEditDialog::QDHLinkEditDialoge(): Creating the dialog...\n");
   DEBUG_MAKE_INDENT(3);

   link_menu=0;
   page_label=0;
   page_menu=0;
   name_label=0;
   name_menu=0;
   url_label=0;
   url_text=0;
   target_menu=0;
   target_text=0;
   
   setResizable(true, true);
   
   if (!map) throw BAD_ARGUMENTS("QDHLinkEditDialog::QDHLinkEditDialog",
				 "Internal error: ZERO map area passed as input.");
   
   if (map->url.length())
      setCaption("DjVu: Hyperlink Properties");
   else
      setCaption("DjVu: Highlighted Area Properties");

   other_target=map->target;
   last_target_index=
      map->target=="_self" ? 0 :
      map->target=="_top" ? 1 :
      map->target=="_blank" ? 2 : 3;

   bool allow_hilite= GUTF8String(map->get_shape_name())==GMapArea::RECT_TAG;

   QeLabel * label;
   QFont font;

   QWidget * start=startWidget();
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10, "vlay");
   vlay->addSpacing(10);
   QHBoxLayout * hlay=new QHBoxLayout(10, "hlay");
   vlay->addLayout(hlay, 1);

      // ************************ 'Preview' box ****************************
   QeGroupBox * preview_box=new QeGroupBox("Preview", start, "preview_box");
   hlay->addWidget(preview_box, 1);
   QVBoxLayout * preview_vlay=new QVBoxLayout(preview_box, 10, 10, "preview_vlay");
   preview_vlay->addSpacing(preview_box->fontMetrics().height());
   preview_pane=new QeHLPane(dimg, gmap_area, gamma, preview_box, "preview_pane");
   preview_vlay->addWidget(preview_pane);
   preview_vlay->activate();

   QVBoxLayout * right_vlay=new QVBoxLayout(5);
   hlay->addLayout(right_vlay);

      // ************************ "Display" box ****************************
   QeGroupBox * disp_box=new QeGroupBox("Display", start, "disp_box");
   right_vlay->addWidget(disp_box, 0);

   QGridLayout * disp_glay=new QGridLayout(disp_box, 6, 2, 10, 5, "disp_glay");
   disp_glay->addRowSpacing(0, disp_box->fontMetrics().height());

   label=new QeLabel("Border &type", disp_box, "display_label");
   disp_glay->addWidget(label, 1, 0);
   btype_menu=getBTypeMenu(disp_box);
   label->setBuddy(btype_menu);
   disp_glay->addWidget(btype_menu, 1, 1, AlignCenter);

	 // 'Border always visible'
   bvisible_butt=new QeCheckBox("Border &always visible", disp_box, "bvisible_butt");
   bvisible_butt->setChecked(map->border_always_visible);
   disp_glay->addMultiCellWidget(bvisible_butt, 2, 2, 0, 1);

   label=new QeLabel("Border co&lor:", disp_box, "bcolor_label");
   disp_glay->addWidget(label, 3, 0);
   bcolor_form=new QeNInOne(disp_box, "bcolor_form");
   bcolor_form->dontResize(TRUE);
   bcolor_menu=new QeColorMenu(map->border_color, bcolor_form, "bcolor_menu");
   label->setBuddy(bcolor_menu);
   bcolor_label=new QeLabel("N/A", bcolor_form, "bcolor_label");
   disp_glay->addWidget(bcolor_form, 3, 1);
      
   label=new QeLabel("Border &width:", disp_box, "border_label");
   disp_glay->addWidget(label, 4, 0);
   bwidth_form=new QeNInOne(disp_box, "bwidth_form");
   bwidth_form->dontResize(TRUE);
   bwidth_spin=new QeSpinBox(3, 32, 1, bwidth_form, "bwidth_spin");
   bwidth_spin->setValue(map->border_width<3 ? 3 : map->border_width);
   label->setBuddy(bwidth_spin);
   bwidth_label=new QeLabel("N/A", bwidth_form, "bwidth_label");
   disp_glay->addWidget(bwidth_form, 4, 1);

	 // 'Hilite color'
   label=new QeLabel("&Highlight color", disp_box, "hilite_label");
   label->setEnabled(allow_hilite);
   disp_glay->addWidget(label, 5, 0);
   hilite_menu=new QeColorMenu(map->hilite_color, disp_box, "hilite_menu");
   hilite_menu->setDefaultItemLabel("Don't highlight");
   label->setBuddy(hilite_menu);
   hilite_menu->setEnabled(allow_hilite);
   disp_glay->addWidget(hilite_menu, 5, 1);
      
   disp_glay->activate();

      // ******************** "Popup note" box *************************

   QeGroupBox * comment_box=new QeGroupBox("Popup note", start, "comment_box");
   right_vlay->addWidget(comment_box, 1);

   QVBoxLayout * comment_vlay=new QVBoxLayout(comment_box, 10, 5);
   comment_vlay->addSpacing(comment_box->fontMetrics().height());

   comment_text=new QMultiLineEdit(comment_box);
   comment_text->setFixedVisibleLines(3);
   comment_text->setText(QStringFromGString(map->comment));
   comment_vlay->addWidget(comment_text);

   comment_vlay->activate();

   if (map->url.length())
   {
      GUTF8String url_key;
      if (map->url[0]=='#')
	 url_key=((const char *) map->url)+1;
      
      QeGroupBox * url_box=new QeGroupBox("Link", start, "url_box");
      vlay->addWidget(url_box);
   
      QVBoxLayout * url_vlay=new QVBoxLayout(url_box, 10, 10, "url_vlay");
      url_vlay->addSpacing(url_box->fontMetrics().height());

      QHBoxLayout * url_hlay=new QHBoxLayout(5, "url_hlay");
      url_vlay->addLayout(url_hlay);

	 //*** Link to:
      label=new QeLabel("L&ink to:", url_box);
      url_hlay->addWidget(label);

      link_menu=new QeComboBox(FALSE, url_box, "link_menu");
      link_menu->insertItem(LINK_URL);
      link_menu->insertItem(LINK_PAGE);
      link_menu->insertItem(LINK_NAME);
      url_hlay->addWidget(link_menu);
      label->setBuddy(link_menu);

      url_hlay->addStretch(1);

	 // *** "Page number:"
      page_label=new QeLabel("&Page number:", url_box);
      url_hlay->addWidget(page_label);

      page_menu=new QeComboBox(FALSE, url_box, "page_menu");
      page_label->setBuddy(page_menu);
      url_hlay->addWidget(page_menu);

      url_hlay->addStretch(1);

	 // *** "Page file name:"
      name_label=new QeLabel("Page &file name:", url_box);
      url_hlay->addWidget(name_label);
      
      name_menu=new QeComboBox(FALSE, url_box, "name_menu");
      name_label->setBuddy(name_menu);
      url_hlay->addWidget(name_menu);

	 // *** Fill out page_menu and name_menu
      GP<DjVmDir> djvm_dir=doc->get_djvm_dir();
      for(int page_num=0;page_num<djvm_dir->get_pages_num();page_num++)
      {
	 GP<DjVmDir::File> frec=djvm_dir->page_to_file(page_num);
	 char buffer[64];
	 sprintf(buffer, "%d", page_num+1);
	 page_menu->insertItem(buffer);
	 name_menu->insertItem(QStringFromGString(frec->get_save_name()));
      }

	 // *** Set the "link" menu current item
      link_menu->setCurrentItem(0);
      if (url_key.length())
	 if (url_key.is_int())
	 {
	    int page=atoi(url_key)-1;
	    if (page>=0 && page<djvm_dir->get_pages_num())
	       link_menu->setCurrentItem(1);
	 } else
	    if (djvm_dir->name_to_file(url_key))
	       link_menu->setCurrentItem(2);
      
      url_hlay=new QHBoxLayout(5, "url_hlay");
      url_vlay->addLayout(url_hlay);
      
      	 //*** URL pair
      url_label=new QeLabel("&URL", url_box);
      url_hlay->addWidget(url_label);
      url_text=new QLineEdit(url_box);
      url_text->setMinimumWidth(0);
      url_text->setFixedHeight(url_text->sizeHint().height());
      url_text->setText(QStringFromGString(map->url));
      url_label->setBuddy(url_text);
      QToolTip::add(url_text,
		    "Enter the URL here.\n"
		    "It will be treated as page-relative unless\n"
		    "it starts from one of standard prefixes.");
      url_hlay->addWidget(url_text, 3);

	 // *** Target Tripple
      label=new QeLabel("&Target", url_box, "target_label");
      url_hlay->addWidget(label);

      target_menu=new QeComboBox(FALSE, url_box, "target_menu");
      target_menu->insertItem("Same frame");
      target_menu->insertItem("Same window");
      target_menu->insertItem("New window");
      target_menu->insertItem("Other:");
      target_menu->setCurrentItem(last_target_index);
      label->setBuddy(target_menu);
      url_hlay->addWidget(target_menu);
      target_text=new QLineEdit(url_box, "target_text");
      target_text->setMinimumWidth(0);
      target_text->setFixedHeight(target_text->sizeHint().height());
      target_text->setText(QStringFromGString(map->target));
      target_text->setEnabled(last_target_index==3);
      url_hlay->addWidget(target_text, 1);	// 1/3 of url_text

      url_vlay->activate();
   }

   if (link_menu)
      slotLinkTypeChanged(link_menu->currentText());
   
   slotBorderTypeChanged(btype_menu->currentText());
   
   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QePushButton * ok_butt=new QePushButton("&OK", start, "ok_butt");
   ok_butt->setDefault(TRUE);
   butt_lay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton("&Cancel", start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);
   
   vlay->activate();

      // Connecting signals and slots
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
   connect(btype_menu, SIGNAL(activated(const QString &)),
	   this, SLOT(slotBorderTypeChanged(const QString &)));
   connect(bcolor_menu, SIGNAL(activated(int)),
	   this, SLOT(slotBorderColorChanged(int)));
   connect(bwidth_spin, SIGNAL(valueChanged(int)),
	   this, SLOT(slotBorderWidthChanged(int)));
   connect(hilite_menu, SIGNAL(activated(int)),
	   this, SLOT(slotHiliteColorChanged(int)));

   if (link_menu)
   {
      connect(link_menu, SIGNAL(activated(const QString &)),
	      this, SLOT(slotLinkTypeChanged(const QString &)));
   }

   if (page_menu)
      connect(page_menu, SIGNAL(activated(int)),
	      this, SLOT(slotPageChanged(int)));

   if (name_menu)
      connect(name_menu, SIGNAL(activated(int)),
	      this, SLOT(slotNameChanged(int)));
   
   if (target_menu)
      connect(target_menu, SIGNAL(activated(int)), this, SLOT(slotTargetChanged(int)));
}
