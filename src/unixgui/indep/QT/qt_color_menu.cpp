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
// $Id: qt_color_menu.cpp,v 1.3.2.1 2001-10-23 21:16:46 leonb Exp $
// $Name:  $

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qt_color_menu.h"

#include "qt_imager.h"
#include "col_db.h"
#include "qt_color_dialog.h"

#include <qbitmap.h>
#include <qlistbox.h>
#include <qpainter.h>

#include "qt_fix.h"

#ifdef QT1
#define QNAMESPACE_QT /* */
#else
#include <qnamespace.h>
#define QNAMESPACE_QT Qt
#include <q1xcompatibility.h>
#endif


//***************************** QeColorItem ********************************
//*** Base for QeStandardItem (for standard colors) and
//*** QeCustomItem (for custom color)
//*** implements procedures for displaying in QComboBox and QListBox

class QeColorItem : public QListBoxItem
{
protected:
   const static int	margin=4;
   QString	text;
   QPixmap	pix, big_pix;
   int		pix_size;
   int		item_width;
   int		item_height;
protected:
   virtual void	paint(QPainter *);
   virtual int	width(const QListBox *) const { return item_width; }
   virtual int	height(const QListBox *) const { return item_height; }
   virtual const QPixmap * pixmap(void) const { return &big_pix; }
public:
   QeColorItem(QListBox * list, u_int32 color, const char * name);
   ~QeColorItem(void) {}
};

void
QeColorItem::paint(QPainter * p)
      // This function is used to draw the item in QListBox,
      // not in QComboBox
{
   p->drawPixmap(1, 1, pix);
   p->drawRect(1, 1, pix_size, pix_size);
   p->drawText(1+pix_size+margin, margin+p->fontMetrics().ascent(), text);
}

QeColorItem::QeColorItem(QListBox * list, u_int32 color, const char * name) :
      text(name)
{
   item_height=list->fontMetrics().height()+2*margin;
   pix_size=item_height-2;
   item_width=1+pix_size+margin+list->fontMetrics().boundingRect(text).width()+1;
   pix=qeImager->getColorPixmap(pix_size, pix_size, color);

      // big_pix will be used by QComboBox to draw the current selection
   big_pix=QPixmap(item_width, item_height, pix.depth());
   big_pix.fill(list->backgroundColor());
   QPainter p(&big_pix, list);
   paint(&p);
   p.end();

      // Create mask (we want the big_pix to be partially transparent)
      // In fact, we'd also like the text's color to be inverted when
      // mouse pointer is over the QComboBox, but this is beyond our power
   QBitmap bmp(item_width, item_height, TRUE);
   QPainter p1(&bmp, list);
   p1.setPen(QNAMESPACE_QT::white);
   p1.fillRect(1, 1, pix_size, pix_size, QNAMESPACE_QT::white);
   p1.drawText(1+pix_size+margin, margin+p1.fontMetrics().ascent(), text);
   p1.end();
   big_pix.setMask(bmp);
}

//***************************** QeStandardItem ******************************
//*** Just converts color_num to color and name before calling QeColorItem

class QeStandardItem : public QeColorItem
{
public:
   QeStandardItem(QListBox * list, int color_num) :
	 QeColorItem(list, ColorDB::Num_to_C32(color_num),
		     ColorDB::Num_to_Name(color_num)) {}
   ~QeStandardItem(void) {}
};

//***************************** QeCustomItem *******************************
//*** Overrides paint(), which allows it to be displayed as just "Other" in
//*** QListBox, but still as normal item in QComboBox

class QeCustomItem : public QeColorItem
{
protected:
   virtual void	paint(QPainter * p)
   {
      p->drawText(1+pix_size+margin, margin+p->fontMetrics().ascent(), "Other...");
   }
public:
   QeCustomItem(QListBox * list, u_int32 color) :
	 QeColorItem(list, color, "Custom") {}
   ~QeCustomItem(void) {}
};

//***************************** QeDefaultItem *******************************
//*** Overrides paint() and text(), which allows it to be displayed as just
//*** "Default" both in QListBox and in QComboBox. I could have used
//*** QListBoxText::, but it won't indent the string in the QListBox as I need

class QeDefaultItem : public QeColorItem
{
protected:
   virtual void	paint(QPainter * p)
   {
      p->drawText(1+pix_size+margin, margin+p->fontMetrics().ascent(),
		  QeColorItem::text);
   }
#ifdef QT1
   virtual const char * text(void) const { return QeColorItem::text; }
#else
   virtual QString text(void) const { return QeColorItem::text; }
#endif
public:
   QeDefaultItem(QListBox * list, const char * label="Default") :
	 QeColorItem(list, 0xffffff, label) {}
   ~QeDefaultItem(void) {}
};

//***************************** QeColorMenu *******************************
//*** Actually, the menu of standard colors and of "custom one"

u_int32
QeColorMenu::color(void) const
{
   int item=currentItem();
   if (item==count()-1) return 0xffffffff;
   else if (item==count()-2) return custom_color;
   else return ColorDB::Num_to_C32(currentItem());
}

void
QeColorMenu::itemSelected(int item)
{
   if (item==count()-2)
   {
	 // "Custom" item selected
      QeColorDialog dialog(custom_color, this, "color_dialog", TRUE);
      if (dialog.exec()==QDialog::Accepted)
      {
	 custom_color=dialog.color();
	 QListBox * combo_list=listBox();
	 combo_list->changeItem(new QeCustomItem(combo_list, custom_color), item);

	 int color_num=ColorDB::GetColorNum(custom_color);
	 if (color_num>=0) setCurrentItem(color_num);
	 else setCurrentItem(count()-2);
	 repaint();
      }
   }
}

void
QeColorMenu::setDefaultItemLabel(const QString &qlabel)
{
   QListBox * list_box=listBox();
   list_box->removeItem(list_box->count()-1);
   list_box->insertItem(new QeDefaultItem(list_box, qlabel));
}

QeColorMenu::QeColorMenu(u_int32 color, QWidget * parent, const char * name) :
      QeComboBox(parent, name), custom_color(color)
      // I could have just created transparent pixmap and have used them
      // as QComboBox items, but HELL who is going to invert the text's color
      // in this case?!

      // So I create new popup list box and custom items, which draw BOTH
      // the square pixmap AND text. This gives perfect white text on blue
      // background when an item is selected in the list

      // Unfortunately, the current selection in QComboBox is drawn not using
      // QListBoxItem::paint() but just as a pixmap obtained by calling
      // QListBoxItem::pixmap(). This results in the fact, that in the
      // combo box itself the text color is not inverted when mouse pointer
      // moves over the combo box.
{
   QListBox * combo_list=new QListBox(this, "combo_list");
   for(int i=0;i<ColorDB::colors;i++)
   {
      QeStandardItem * item=new QeStandardItem(combo_list, i);
      combo_list->insertItem(item);
   }
   QListBoxItem * item=new QeCustomItem(combo_list, custom_color);
   combo_list->insertItem(item);
   item=new QeDefaultItem(combo_list);
   combo_list->insertItem(item);
   
   setListBox(combo_list);
   combo_list->setBackgroundColor(QNAMESPACE_QT::white);

   connect(this, SIGNAL(activated(int)), this, SLOT(itemSelected(int)));

   if (custom_color==0xffffffff) setCurrentItem(count()-1);
   else
   {
      int color_num=ColorDB::GetColorNum(custom_color);
      if (color_num>=0) setCurrentItem(color_num);
      else setCurrentItem(count()-2);
   }
}
