//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_tbar_mode_piece.cpp,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_tbar_mode_piece.h"
#include "debug.h"
#include "qlib.h"
#include "qd_toolbutt.h"
#include "djvu_base_res.h"
#include "cin_data.h"

#include <qcombobox.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qcheckbox.h>

#include "qt_fix.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#define ONE2ONE_STR	"1 : 1"
#define STRETCH_STR	"Stretch"
#define FIT_WIDTH_STR	"Fit Width"
#define FIT_PAGE_STR	"Fit Page"

class QDZoomValidator : public QValidator
{
public:
   virtual void		fixup(QString &);
#ifdef QT1
   virtual State	validate(QString &, int &);
#else
   virtual State	validate(QString &, int &) const;
#endif
   
   QDZoomValidator(QComboBox * parent, const char * name=0) :
	 QValidator(parent, name) {}
};

void
QDZoomValidator::fixup(QString & str)
{
#ifdef QT1
   str.detach();
#else
   str.truncate(0);
#endif

   QComboBox * menu=(QComboBox *) parent();
   menu->setEditText(str=menu->text(menu->currentItem()));
}

QValidator::State
QDZoomValidator::validate(QString & input, int & pos)
#ifndef QT1
const
#endif
{
   if (!input.length()) return Valid;

   char buffer[128];
   strncpy(buffer, input, 127); buffer[127]=0;
   char * ptr=buffer+strlen(buffer)-1;
   while(isspace(*ptr) || *ptr=='%') *ptr--=0;
   QString str(buffer);
   
   bool status;
   int zoom=str.toInt(&status);
   if (!status) return Invalid;

   if (zoom>IDC_ZOOM_MAX-IDC_ZOOM_MIN) return Invalid;
   if (zoom<=0) return Invalid;
   if (zoom<5)
      if (str.length()==1) return Valid;
      else return Invalid;
   
   if (zoom<5 || zoom>IDC_ZOOM_MAX-IDC_ZOOM_MIN) return Invalid;
   return Acceptable;
}

//****************************************************************************
//***************************** QDTBarModePiece ******************************
//****************************************************************************

void
QDTBarModePiece::setEnabled(bool en)
{
   zoom_menu->setEnabled(en);
   mode_menu->setEnabled(en);
   zoom_in_butt->setEnabled(en);
   zoom_out_butt->setEnabled(en);
   if ( pin_butt )
      pin_butt->setEnabled(en);
}

static const int menu_items_size=10;
static const struct MenuItems {
  const char *str;
  int zoom;
} menu_items[menu_items_size] = {
  {"300 %",IDC_ZOOM_MIN+300},
  {"150 %",IDC_ZOOM_MIN+150},
  {"100 %",IDC_ZOOM_MIN+100},
  {"75 %",IDC_ZOOM_MIN+75},
  {"50 %",IDC_ZOOM_MIN+50},
  {"25 %",IDC_ZOOM_MIN+25},
  {QT_TRANSLATE_NOOP("QDTBarModePiece","Fit Width"),IDC_ZOOM_WIDTH},
  {QT_TRANSLATE_NOOP("QDTBarModePiece","Fit Page"),IDC_ZOOM_PAGE},
  {"1 : 1",IDC_ZOOM_ONE2ONE},
  {QT_TRANSLATE_NOOP("QDTBarModePiece","Stretch"),IDC_ZOOM_STRETCH},
};

QDTBarModePiece::QDTBarModePiece(QWidget * toolbar) : QDTBarPiece(toolbar)
{
   if ( dynamic_cast<QDToolBar *>(toolbar) )
      qdtoolbar_child=TRUE;
   else
      qdtoolbar_child=FALSE;
   
   QFrame * frame;
   
   mode_menu=new QeComboBox(FALSE, toolbar, "mode_menu");
   mode_menu->insertItem(tr("Color"));
   mode_menu->insertItem(tr("B&W"));
   mode_menu->insertItem(tr("Fore"));
   mode_menu->insertItem(tr("Back"));
   connect(mode_menu, SIGNAL(activated(int)), this, SLOT(slotMode(int)));
   QToolTip::add(mode_menu, tr("Display mode"));

   if ( qdtoolbar_child ) 
      ((QDToolBar *)toolbar)->addLeftWidget(mode_menu);

   frame=new QFrame(toolbar, "separator");
   frame->setFrameStyle(QFrame::VLine | QFrame::Sunken);
   frame->setMinimumWidth(10);
   if ( qdtoolbar_child ) 
      ((QDToolBar *)toolbar)->addLeftWidget(frame);
   
   zoom_menu=new QeComboBox(TRUE, toolbar, "zoom_menu");
   zoom_menu->setInsertionPolicy(QComboBox::NoInsertion);
   zoom_menu->setValidator(new QDZoomValidator(zoom_menu));
   int i;
   for(i=0;i<menu_items_size;i++)
   {
     zoom_menu->insertItem(tr(menu_items[i].str));
   }
   connect(zoom_menu, SIGNAL(activated(const QString &)),
	   this, SLOT(slotZoom(const QString &)));
   QToolTip::add(zoom_menu, tr("Zoom"));
   if ( qdtoolbar_child )
      ((QDToolBar *)toolbar)->addLeftWidget(zoom_menu);

   zoom_in_butt=new QDToolButton(*CINData::get("ppm_vzoom_in"), true,
				 IDC_ZOOM_ZOOMIN, toolbar, "Zoom In");
   connect(zoom_in_butt, SIGNAL(clicked(void)), this, SLOT(slotZoom(void)));

   zoom_out_butt=new QDToolButton(*CINData::get("ppm_vzoom_out"), true,
				  IDC_ZOOM_ZOOMOUT, toolbar, "Zoom Out");
   connect(zoom_out_butt, SIGNAL(clicked(void)), this, SLOT(slotZoom(void)));
   if ( qdtoolbar_child ) 
      ((QDToolBar *)toolbar)->addLeftWidgets(zoom_in_butt, zoom_out_butt);

   if ( qdtoolbar_child )
   {
      pin_butt=new QDToolButton(*CINData::get("ppm_vpin_out"), false,
				-1, toolbar, "Stick");
      pin_butt->setToggleButton(TRUE);
      pin_butt->setOnPixmap(*CINData::get("ppm_vpin_in"));
      connect(pin_butt, SIGNAL(toggled(bool)), this, SIGNAL(sigStick(bool)));
      ((QDToolBar *)toolbar)->addRightWidget(pin_butt);

      ((QDToolBar *)toolbar)->addPiece(this);
   }
   else
   {
      pin_butt=NULL;
   }
}

void
QDTBarModePiece::update(int cmd_mode, bool mode_enabled, int cmd_zoom, int zoom)
{
   int i;
   for(i=0;i<menu_items_size;i++)
   {
     if(cmd_zoom == menu_items[i].zoom)
     {
       zoom_menu->setCurrentItem(i);
       break;
     }
   }
   if(i==menu_items_size)
   {
      char buffer[64];
      sprintf(buffer, "%d %%", zoom);
      zoom_menu->setEditText(buffer);
   }

   zoom_in_butt->setEnabled(zoom<IDC_ZOOM_MAX-IDC_ZOOM_MIN);
   zoom_out_butt->setEnabled(zoom>5);
   
   switch(cmd_mode)
   {
      case IDC_DISPLAY_COLOR:
        mode_menu->setCurrentItem(0);
        break;
      case IDC_DISPLAY_BLACKWHITE:
        mode_menu->setCurrentItem(1);
        break;
      case IDC_DISPLAY_FOREGROUND:
        mode_menu->setCurrentItem(2);
        break;
      case IDC_DISPLAY_BACKGROUND:
        mode_menu->setCurrentItem(3);
        break;
      default:
        break;
   }

   if ( !qdtoolbar_child )
      zoom_menu->setEnabled(TRUE);
   
   mode_menu->setEnabled(mode_enabled);

   zoom_menu->setFixedSize(zoom_menu->sizeHint());
   mode_menu->setFixedSize(mode_menu->sizeHint());

      // Keep everything disabled if the toolbar is disabled.
   if (!toolbar->isEnabled()) setEnabled(false);
}

bool
QDTBarModePiece::isStuck(void) const
{
   if ( !pin_butt ) return FALSE;
   return pin_butt->isOn();
}

void
QDTBarModePiece::stick(bool en)
{
   if ( pin_butt )
      pin_butt->setOn(en);
}

void
QDTBarModePiece::slotZoom(const QString & qstr)
{
  int i;
  for(i=0;i<menu_items_size;i++)
  {
    if(qstr==tr(menu_items[i].str))
    {
      emit sigSetZoom(menu_items[i].zoom);
      break;
    }
  }
  if(i == menu_items_size)
  {
     const char * const str=qstr;
    i=atoi(str);
    if(i>0)
    {
      emit sigSetZoom(i+IDC_ZOOM_MIN);
    }
  }
}

void
QDTBarModePiece::slotZoom(void)
{
   const QObject * obj=sender();
   if (obj && obj->isWidgetType() && obj->inherits("QDToolButton"))
   {
      const QDToolButton * butt=(QDToolButton *) obj;
      emit sigSetZoom(butt->cmd);
   }
}

void
QDTBarModePiece::slotMode(int index)
{
   switch(index)
   {
      case 0:
        emit sigSetMode(IDC_DISPLAY_COLOR);
        break;
      case 1:
        emit sigSetMode(IDC_DISPLAY_BLACKWHITE);
        break;
      case 2:
        emit sigSetMode(IDC_DISPLAY_FOREGROUND);
        break;
      case 3:
        emit sigSetMode(IDC_DISPLAY_BACKGROUND);
        break;
      default:
        break;
   }
}

// END OF FILE
