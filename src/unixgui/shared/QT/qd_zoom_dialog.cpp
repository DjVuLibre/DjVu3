//C-  -*- C++ -*-
//C-
//C- DjVu® Unix Viewer (v. 3.5)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: qd_zoom_dialog.cpp,v 1.7 2001-10-17 19:09:18 docbill Exp $
// $Name:  $


#include "qd_zoom_dialog.h"
#include "DjVuAnno.h"
#include "djvu_base_res.h"
#include "qlib.h"

#include "debug.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <ctype.h>
#include <qkeycode.h>

#include "qt_fix.h"

class QDZEValidator : public QValidator
{
public:
   virtual void		fixup(QString &);
#ifdef QT1
   virtual State	validate(QString &, int &);
#else
   virtual State	validate(QString &, int &) const;
#endif
   
   QDZEValidator(QComboBox * parent, const char * name=0) :
   QValidator(parent, name) {};
};

void
QDZEValidator::fixup(QString & str)
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
QDZEValidator::validate(QString & input, int & pos)
#ifndef QT1
const
#endif
{
   if (!input.length()) return Valid;

   char buffer[128];
   strncpy(buffer, input, 127); buffer[127]=0;
   char * ptr=buffer+strlen(buffer)-1;
   while(isspace(*ptr) || *ptr=='%') *ptr--=0;
   char * start=buffer;
   while(isspace(*start)) start++;
   QString str(start);
   
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

void
QDZoomDialog::done(int rc)
{
   if (rc==Accepted)
      if (getZoom()==-1000)
      {
	 QString mesg=tr("Invalid zoom specified: '")+menu->currentText()+"'";
	 showError(this, tr("DjVu: Input error"),mesg);
	 return;
      }
   QeDialog::done(rc);
}

void
QDZComboBox::keyPressEvent(QKeyEvent * ev)
{
   if (ev->key()==Key_Return || ev->key()==Key_Enter)
   {
      QWidget * w=this;
      while(w && strcmp(w->className(), "QDZoomDialog")) w=w->parentWidget();
      if (w && ((QDZoomDialog *) w)->getZoom()==-1000)
      {
	 QString mesg=tr("Invalid zoom specified: '")+currentText()+"'";
	 showError(this, tr("DjVu: Input error"),mesg);
	 return;
      }
   }
   QeComboBox::keyPressEvent(ev);
}

static const int menu_items_size=11;
static const struct MenuItems {
  const char *str;
  int zoom;
} menu_items[menu_items_size] = {
  {"300 %",300},
  {"150 %",150},
  {"100 %",100},
  {"75 %",75},
  {"50 %",50},
  {"25 %",25},
  {QT_TRANSLATE_NOOP("QDZoomDialog","Fit Width"),DjVuANT::ZOOM_WIDTH},
  {QT_TRANSLATE_NOOP("QDZoomDialog","Fit Page"),DjVuANT::ZOOM_PAGE},
  {QT_TRANSLATE_NOOP("QDZoomDialog","One to one"),DjVuANT::ZOOM_ONE2ONE},
  {QT_TRANSLATE_NOOP("QDZoomDialog","Stretch"),DjVuANT::ZOOM_STRETCH},
  {QT_TRANSLATE_NOOP("QDZoomDialog","Default"),DjVuANT::ZOOM_UNSPEC}
};

QDZoomDialog::QDZoomDialog(int zoom, QWidget * parent,
			   const char * name, bool modal) :
      QeDialog(parent, name, modal)
{
   setCaption(tr("DjVu: Recommended Zoom"));
   
   QWidget * start=startWidget();
   
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 15);
   QeLabel * label=new QeLabel(tr("Please specify the recommended resolution in which\nthe page should be displayed by the browser.\n"), start);
   label->setAlignment(AlignCenter);
   vlay->addWidget(label);

   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   label=new QeLabel(tr("Recommended zoom:"), start);
   hlay->addWidget(label, 1);
   menu=new QDZComboBox(TRUE, start, "zoom_menu");
   menu->setInsertionPolicy(QComboBox::NoInsertion);
   menu->setValidator(new QDZEValidator(menu));
   int i;
   for(i=0;i<menu_items_size;i++)
   {
     menu->insertItem(tr(menu_items[i].str));
   }
   hlay->addWidget(menu);
   for(i=0;i<menu_items_size;i++)
   {
     if(zoom == menu_items[i].zoom)
     {
       menu->setCurrentItem(i);
       break;
     }
   }
   if(i == menu_items_size)
   {
     char buffer[128];
     sprintf(buffer, "%d %%", zoom);
     menu->setEditText(buffer);
   }

   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QePushButton * ok_butt=new QePushButton(tr("&OK"), start, "ok_butt");
   ok_butt->setDefault(TRUE);
   butt_lay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton(tr("&Cancel"), start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);

   vlay->activate();

   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));

   menu->setFocus();
}

int
QDZoomDialog::getZoom(void) const
{
   int retval=-1000;
   int i;
   const char * selection=menu->currentText();
   for(i=0;i<menu_items_size;i++)
   {
     if(!strcmp(selection,menu_items[i].str))
     {
       retval=menu_items[i].zoom;
       break;
     }
   }
   if(i == menu_items_size)
   {
      int pos;
      QString str=selection;
      if (menu->validator()->validate(str, pos)==QValidator::Acceptable)
	 retval=atoi(selection);
   }
   return retval;
}

