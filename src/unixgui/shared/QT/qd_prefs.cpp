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
// $Id: qd_prefs.cpp,v 1.6.2.1 2001-10-23 21:16:48 leonb Exp $
// $Name:  $

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "djvu_file_cache.h"
#include "qd_prefs.h"
#include "qlib.h"
#include "debug.h"
#include "exc_msg.h"
#include "GRect.h"
#include "GPixmap.h"
#include "qt_painter.h"
#include "qx_imager.h"

#include <qcheckbox.h>
#include <qslider.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qpaintdevicemetrics.h>
#include <qspinbox.h>

#include "qt_fix.h"

#ifndef QT1
#include <q1xcompatibility.h>
#endif


//***************************************************************************
//***************************** QDGammaDispl ********************************
//***************************************************************************

bool
QDGammaDispl::event(QEvent * ev)
{
   try
   {
      if (ev->type()==Event_User)
      {
	 DEBUG_MSG("Gamma::event(): resizing: width=" << width() << ", height=" << height() << "\n");
	 setMinimumWidth(height());
	 setMaximumWidth(height());
	 ActivateLayouts(this);
	 return FALSE;
      }
   } catch(const GException & exc)
   {
      showError(this, tr("DjVu Error"), exc);
   }
   return QWidget::event(ev);
}

void
QDGammaDispl::resizeEvent(QResizeEvent * ev)
{
   DEBUG_MSG("QDGammaDispl::resizeEvent(): delaying resize...\n");
   QEvent * uev=new QEvent(Event_User);
   QApplication::postEvent(this, uev);

   if (ev->oldSize()!=ev->size())
   {
	 // Important! MUST GO THRU THE EVENT QUEUE for event compression
      QPaintEvent * ev=new QPaintEvent(geometry());
      QApplication::postEvent(this, ev);
   }
}

void
QDGammaDispl::paintEvent(QPaintEvent *)
{
   DEBUG_MSG("QDGammaDispl::paintEvent(): painting...\n");
   DEBUG_MAKE_INDENT(3);

   try
   {
      GRect grect(0, 0, width(), height());
      grect.inflate(-5, -5);
      int size=grect.width()<grect.height() ? grect.width() : grect.height();
      if (size & 1) size--;
      int x=grect.width()-size;
      int y=grect.height()-size;
      grect.xmin+=x/2;
      grect.xmax+=x/2-x;
      grect.ymin+=y/2;
      grect.ymax+=y/2-y;

      GPixel gray={ 128, 128, 128 };
      GP<GPixmap> pm=GPixmap::create(size, size, &gray);

      DEBUG_MSG("pixmap size=" << size << "x" << size << "\n");
      for(int j=0;j<size;j++)
      {
	 GPixel * p=(*pm)[j];
	 const GPixel & c=(j & 1 ? GPixel::BLACK : GPixel::WHITE);
	 if (j>size/2) p+=size/2;
	 for(int i=0;i<size/2;i++) p[i]=c;
      }

      DEBUG_MSG("correcting color for gamma=" << gamma << "\n");
      pm->color_correct(gamma);

      DEBUG_MSG("seeing if dithering is necessary\n");
      qxImager->dither(*pm);

      QePainter p(this);
      p.drawPixmap(grect, pm);
      p.end();
   } catch(const GException & exc)
   {
      showError(this, tr("DjVu Error"), exc);
   }
}

void
QDGammaDispl::setGamma(int _gamma)
{
   gamma=_gamma/10.0;
   repaint(FALSE);
}

QDGammaDispl::QDGammaDispl(QWidget * parent, const char * name) :
      QFrame(parent, name), gamma(0.5)
{
   if (QApplication::style()==WindowsStyle)
      setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
   else
      setFrameStyle(QFrame::Panel | QFrame::Sunken);
   setMargin(5);
   setBackgroundColor(white);

   setWFlags(WResizeNoErase);	// qt-1.43 "cleverly" repaints the widget
   				// itself using repaint() => flicker
}

//***************************************************************************
//***************************** QDGammaPrefs ********************************
//***************************************************************************

double
QDGammaPrefs::displGamma(void) const
{
   return displ_slider->value()/10.0;
}

double
QDGammaPrefs::printGamma(void) const
{
   return print_slider->value()/10.0;
}

bool
QDGammaPrefs::match(void) const
{
   return match_butt->isChecked();
}

void
QDGammaPrefs::matchToggled(bool state)
{
   if (state) print_slider->setValue(displ_slider->value());
}

QDGammaPrefs::QDGammaPrefs(DjVuPrefs * prefs, QWidget * parent, const char * name) :
      QeGroupBox(tr("Gamma Correction"), parent, name)
{
   DEBUG_MSG("QDGammaPrefs::QDGammaPrefs(): Creating 'Gamma correction' box...\n");
   DEBUG_MAKE_INDENT(3);

   QeLabel * label;
   
   QGridLayout * glay=new QGridLayout(this, 4, 3, 10, 5, "gamma_glay");
   glay->addRowSpacing(0, fontMetrics().height());
   glay->addColSpacing(1, 15);

   QVBoxLayout * vlay=new QVBoxLayout(5);
   glay->addLayout(vlay, 1, 0);
   vlay->addStretch(1);
   label=new QeLabel(tr("Screen color correction.\n") +
                     tr("Adjust slider until\ngray shades look similar."), this );
   label->setAlignment(AlignCenter);
   vlay->addWidget(label);
   vlay->addSpacing(5);
   displ_slider=new QeSlider(5, 50, 1, (int) (prefs->dScreenGamma*10),
			     QSlider::Horizontal, this, "displ_slider");
   displ_slider->setTickmarks(QSlider::Below);
   displ_slider->setTickInterval(5);
   vlay->addWidget(displ_slider);

   QHBoxLayout * hlay=new QHBoxLayout(5);
   vlay->addLayout(hlay);
   hlay->addWidget(new QeLabel(tr("Darker"), this));
   hlay->addStretch(1);
   hlay->addWidget(new QeLabel(tr("Lighter"), this));
   vlay->addStretch(1);

      // Preview box
   hlay=new QHBoxLayout(5);
   glay->addLayout(hlay, 1, 2);
   hlay->addStretch(1);
   preview=new QDGammaDispl(this, "preview");
   preview->setGamma(displ_slider->value());
   hlay->addWidget(preview);
   hlay->addStretch(1);

      // Separator
   glay->addRowSpacing(2, 15);
   QFrame * frame=new QFrame(this, "separator");
   frame->setFrameStyle(QFrame::Sunken | QFrame::HLine);
   frame->setMinimumHeight(frame->sizeHint().height());
   glay->addMultiCellWidget(frame, 2, 2, 0, 2);

   vlay=new QVBoxLayout(5);
   glay->addLayout(vlay, 3, 0);
   vlay->addStretch(1);
   label=new QeLabel(tr("Printer color correction."), this);
   label->setAlignment(AlignCenter);
   vlay->addWidget(label);
   print_slider=new QeSlider(5, 50, 1, prefs->dPrinterGamma==0 ?
			     (int) (prefs->dScreenGamma*10) :
			     (int) (prefs->dPrinterGamma*10),
			     QSlider::Horizontal, this, "print_slider");
   print_slider->setTickmarks(QSlider::Below);
   print_slider->setTickInterval(5);
   vlay->addWidget(print_slider);

   hlay=new QHBoxLayout(5);
   vlay->addLayout(hlay);
   hlay->addWidget(new QeLabel(tr("Darker"), this));
   hlay->addStretch(1);
   hlay->addWidget(new QeLabel(tr("Lighter"), this));
   vlay->addStretch(1);
   
   match_butt=new QeCheckBox(tr("Automatic\ncolor matching."), this, "match_butt");
   match_butt->setChecked(false);
   if (prefs->dPrinterGamma==0)
     {
       match_butt->setChecked(true);
       print_slider->setValue(22);
       print_slider->setEnabled(false);
     }   
   glay->addWidget(match_butt, 3, 2);

   glay->activate();
   
      // Connecting signals and slots
   connect(displ_slider, SIGNAL(valueChanged(int)), preview, SLOT(setGamma(int)));
   connect(match_butt, SIGNAL(toggled(bool)), this, SLOT(matchToggled(bool)));
   QToolTip::add(preview, 
                 tr("Adjust slider on the left until all\n"
                    "areas of the square look approximately\nthe same"));
}

//***************************************************************************
//******************************* QDLensPrefs *******************************
//***************************************************************************

static const QString ctrl_hkey=QT_TRANSLATE_NOOP("QDLensPrefs","Control key");
static const QString shift_hkey=QT_TRANSLATE_NOOP("QDLensPrefs","Shift key");
static const QString alt_hkey=QT_TRANSLATE_NOOP("QDLensPrefs","Alt key");
static const QString mid_hkey=QT_TRANSLATE_NOOP("QDLensPrefs","Middle mouse button");

int
QDLensPrefs::size(void) const
{
   return DjVuPrefs::legal_mag_size[size_menu->currentItem() %
				    DjVuPrefs::legal_mag_size_num];
}

int
QDLensPrefs::scale(void) const
{
   return DjVuPrefs::legal_mag_scale[scale_menu->currentItem() %
				     DjVuPrefs::legal_mag_scale_num];
}

DjVuPrefs::MagButtType
QDLensPrefs::hotKey(void) const
{
   DjVuPrefs::MagButtType key=DjVuPrefs::MAG_CTRL;

   QString text=hkey_menu->currentText();
   if (text==ctrl_hkey)
      key=DjVuPrefs::MAG_CTRL;
   else if (text==shift_hkey)
      key=DjVuPrefs::MAG_SHIFT;
   else if (text==alt_hkey)
      key=DjVuPrefs::MAG_ALT;
   else if (text==mid_hkey)
      key=DjVuPrefs::MAG_MID;
   
   return key;
}

void
QDLensPrefs::slotHotKeyChanged(const QString & )
{
   emit sigHotKeyChanged(hotKey());
}

void
QDLensPrefs::slotHlHotKeyChanged(DjVuPrefs::HLButtType key)
{
      // Make sure that we don't have the same button selected
   QString cur_text=hkey_menu->currentText();
   
   if (key==DjVuPrefs::HLB_CTRL && cur_text==ctrl_hkey ||
       key==DjVuPrefs::HLB_SHIFT && cur_text==shift_hkey ||
       key==DjVuPrefs::HLB_ALT && cur_text==alt_hkey)
      hkey_menu->setCurrentItem(mid_hkey);
}
   
QDLensPrefs::QDLensPrefs(DjVuPrefs * prefs, QWidget * parent, const char * name) :
      QeGroupBox(tr("Magnifying Glass"), parent, name)
{
   DEBUG_MSG("QDLensPrefs::QDLensPrefs(): Creating 'Magnifying Glass' box...\n");
   DEBUG_MAKE_INDENT(3);

   QVBoxLayout * vlay=new QVBoxLayout(this, 10, 5, "lens_vlay");
   vlay->addSpacing(fontMetrics().height());

   QHBoxLayout * hlay=new QHBoxLayout(5);
   vlay->addLayout(hlay);

   QeLabel * size_label=new QeLabel(tr("Size: "), this);
   hlay->addWidget(size_label);
   
   size_menu=new QeComboBox(FALSE, this, "mag_size_menu");
   for(int i=0;i<DjVuPrefs::legal_mag_size_num;i++)
   {
      int size=DjVuPrefs::legal_mag_size[i];
      {
        QString mesg=QString::number(size)+tr(" pixels");
        size_menu->insertItem(mesg);
      }
      if (size==prefs->magnifierSize)
	 size_menu->setCurrentItem(i);
   }
   hlay->addWidget(size_menu);

   hlay->addStretch(1);
   
   QeLabel * mag_label=new QeLabel(tr("Magnification: "), this);
   hlay->addWidget(mag_label);
   
   scale_menu=new QeComboBox(FALSE, this, "mag_scale_menu");
   for(int i=0;i<DjVuPrefs::legal_mag_scale_num;i++)
   {
      int scale=DjVuPrefs::legal_mag_scale[i];
      char buffer[64];
      sprintf(buffer, "%1.1fx", scale/10.0);
      scale_menu->insertItem(buffer);
      if (scale==prefs->magnifierScale)
	 scale_menu->setCurrentItem(i);
   }
   hlay->addWidget(scale_menu);

   hlay=new QHBoxLayout(5);
   vlay->addLayout(hlay);

   QeLabel * label=new QeLabel(tr("To activate press: "), this);
   hlay->addWidget(label);
   
   hlay->addStretch(1);

   hkey_menu=new QeComboBox(FALSE, this, "hkey_menu");
   hkey_menu->insertItem(tr(ctrl_hkey));
   hkey_menu->insertItem(tr(shift_hkey));
   hkey_menu->insertItem(tr(alt_hkey));
   hkey_menu->insertItem(tr(mid_hkey));
   QString cur_hkey=ctrl_hkey;
   if (prefs->magnifierHotKey==DjVuPrefs::MAG_CTRL)
      cur_hkey=ctrl_hkey;
   else if (prefs->magnifierHotKey==DjVuPrefs::MAG_SHIFT)
      cur_hkey=shift_hkey;
   else if (prefs->magnifierHotKey==DjVuPrefs::MAG_ALT)
      cur_hkey=alt_hkey;
   else if (prefs->magnifierHotKey==DjVuPrefs::MAG_MID)
      cur_hkey=mid_hkey;
   hkey_menu->setCurrentItem(cur_hkey);
   hlay->addWidget(hkey_menu);

   vlay->activate();

   QString tip=tr("To use the magnifying glass\npress the CTRL key and point to the\ndocument area you want to magnify.");
   QToolTip::add(size_label, tip);
   QToolTip::add(mag_label, tip);
   QToolTip::add(size_menu, tip);
   QToolTip::add(scale_menu, tip);

   connect(hkey_menu, SIGNAL(activated(const QString &)),
	   this, SLOT(slotHotKeyChanged(const QString &)));
}

//***************************************************************************
//******************************* QDOptimPrefs ******************************
//***************************************************************************

bool
QDOptimPrefs::fastZoom(void) const
{
   return fastzoom_butt->isChecked();
}

bool
QDOptimPrefs::fastThumb(void) const
{
   return fastthumb_butt->isChecked();
}

bool
QDOptimPrefs::optimizeLCD(void) const
{
   return lcd_butt->isChecked();
}

QDOptimPrefs::QDOptimPrefs(DjVuPrefs * prefs, QWidget * parent, const char * name) :
      QeGroupBox(tr("Optimization"), parent, name)
{
   DEBUG_MSG("QDOptimPrefs::QDOptimPrefs(): Creating 'Optimization' box...\n");
   DEBUG_MAKE_INDENT(3);

   QVBoxLayout * vlay=new QVBoxLayout(this, 10, 5, "optim_vlay");
   vlay->addSpacing(fontMetrics().height());

   fastzoom_butt=new QeCheckBox(tr("Favor fast magnifications for \"Fit\" resolutions."),
				this, "fastzoom_butt");
   fastzoom_butt->setChecked(prefs->fastZoom);
   vlay->addWidget(fastzoom_butt);

   fastthumb_butt=new QeCheckBox(tr("Enable fast thumbnails."),
				 this, "fastthumb_butt");
   fastthumb_butt->setChecked(prefs->fastThumb);
   vlay->addWidget(fastthumb_butt);

   lcd_butt=new QeCheckBox(tr("Optimize images for 24-bit LCD displays."),
			   this, "lcd_butt");
   lcd_butt->setChecked(prefs->optimizeLCD);
   vlay->addWidget(lcd_butt);

   vlay->activate();
}

//***************************************************************************
//******************************* QDCachePrefs ******************************
//***************************************************************************

int
QDCachePrefs::mcacheSize(void) const
{
   int mcache_size;
   sscanf(mcache_menu->currentText(), "%d", &mcache_size);
   return mcache_size;
}

int
QDCachePrefs::pcacheSize(void) const
{
   int pcache_size=0;
   if (pcache_menu)
      sscanf(pcache_menu->currentText(), "%d", &pcache_size);
   return pcache_size;
}

void
QDCachePrefs::slotClearCache(void)
{
   get_file_cache()->clear();
}

void
QDCachePrefs::enablePCache(bool en)
{
   if (pcache_label) pcache_label->setEnabled(en);
   if (pcache_menu) pcache_menu->setEnabled(en);
   if (pcache_butt) pcache_butt->setEnabled(en);
}

void
QDCachePrefs::resetPCache(void)
{
   if (pcache_menu)
      pcache_menu->setCurrentItem(0);
}

QDCachePrefs::QDCachePrefs(DjVuPrefs * prefs, bool pcache_on,
			   QWidget * parent, const char * name) :
      QeGroupBox(tr("Cache Preferences"), parent, name)
{
   DEBUG_MSG("QDCachePrefs::QDCachePrefs(): Creating 'Cache Preferences' box...\n");
   DEBUG_MAKE_INDENT(3);

   QGridLayout * glay=new QGridLayout(this, 3, 2+pcache_on, 10, 5, "cache_glay");
   glay->addRowSpacing(0, fontMetrics().height());

   QeLabel * mcache_label=new QeLabel(tr("Off screen cache size"), this);
   glay->addWidget(mcache_label, 1, 0);
   mcache_menu=new QeComboBox(this, "mcache_menu");
   int mcache_item=-1;
   mcache_menu->insertItem("0 Mb"); mcache_item+=(prefs->mcacheSize>=0);
   mcache_menu->insertItem("1 Mb"); mcache_item+=(prefs->mcacheSize>=1);
   mcache_menu->insertItem("2 Mb"); mcache_item+=(prefs->mcacheSize>=2);
   mcache_menu->insertItem("4 Mb"); mcache_item+=(prefs->mcacheSize>=4);
   mcache_menu->insertItem("6 Mb"); mcache_item+=(prefs->mcacheSize>=6);
   mcache_menu->setCurrentItem(mcache_item);
   glay->addWidget(mcache_menu, 1, 1);

   if (pcache_on)
   {
      pcache_label=new QeLabel(tr("Decoded pages cache"), this);
      pcache_label->setEnabled(get_file_cache()->is_enabled());
      glay->addWidget(pcache_label, 2, 0);
      pcache_menu=new QeComboBox(this, "pcache_menu");
      pcache_menu->setEnabled(get_file_cache()->is_enabled());
      int pcache_item=-1;
      pcache_menu->insertItem("0 Mb"); pcache_item+=(prefs->pcacheSize>=0);
      pcache_menu->insertItem("2 Mb"); pcache_item+=(prefs->pcacheSize>=2);
      pcache_menu->insertItem("4 Mb"); pcache_item+=(prefs->pcacheSize>=4);
      pcache_menu->insertItem("6 Mb"); pcache_item+=(prefs->pcacheSize>=6);
      pcache_menu->insertItem("10 Mb"); pcache_item+=(prefs->pcacheSize>=10);
      pcache_menu->insertItem("15 Mb"); pcache_item+=(prefs->pcacheSize>=15);
      pcache_menu->insertItem("20 Mb"); pcache_item+=(prefs->pcacheSize>=20);
      pcache_menu->insertItem("30 Mb"); pcache_item+=(prefs->pcacheSize>=30);
      pcache_menu->setCurrentItem(pcache_item);
      glay->addWidget(pcache_menu, 2, 1);

      pcache_butt=new QPushButton(tr("Clear"), this, "pcache_butt");
      pcache_butt->setMinimumHeight(pcache_butt->sizeHint().height());
      pcache_butt->setMinimumWidth(pcache_butt->fontMetrics().width(tr("  Clear  ")));
      pcache_butt->setEnabled(get_file_cache()->is_enabled());
      glay->addWidget(pcache_butt, 2, 2);
   } else
   {
      pcache_label=0;
      pcache_menu=0;
      pcache_butt=0;
   }
   
   glay->activate();

   QString mcache_tip=tr("Off screen cache is used to cache the\nimage margins to make the scrolling smoother.");
   QString pcache_tip=tr("Decoded pages cache is used to store\npages that you have already seen in memory\nfor fast access in future.");
   QToolTip::add(mcache_label, mcache_tip);
   QToolTip::add(mcache_menu, mcache_tip);
   if (pcache_label)
      QToolTip::add(pcache_label, pcache_tip);
   if (pcache_menu)
      QToolTip::add(pcache_menu, pcache_tip);
   if (pcache_butt)
      connect(pcache_butt, SIGNAL(clicked(void)), this, SLOT(slotClearCache(void)));
}

//***************************************************************************
//******************************** QDTbarPrefs ******************************
//***************************************************************************

bool
QDTbarPrefs::enabled(void) const
{
   return on_butt->isChecked();
}

bool
QDTbarPrefs::visible(void) const
{
   return vis_butt->isChecked();
}

int
QDTbarPrefs::delay(void) const
{
   return delay_spin->value();
}

void
QDTbarPrefs::slotTBToggled(bool)
{
   vis_butt->setEnabled(on_butt->isChecked());
   delay_label->setEnabled(on_butt->isChecked() &&
			   !vis_butt->isChecked());
   delay_spin->setEnabled(delay_label->isEnabled());
}

QDTbarPrefs::QDTbarPrefs(DjVuPrefs * prefs, QWidget * parent, const char * name) :
      QeGroupBox(tr("Toolbar Preferences"), parent, name)
{
   DEBUG_MSG("QDTbarPrefs::QDTbarOptimPrefs(): Creating 'Toolbar Preferences' box...\n");
   DEBUG_MAKE_INDENT(3);

   QVBoxLayout * vlay=new QVBoxLayout(this, 10, 5, "tbar_vlay");
   vlay->addSpacing(fontMetrics().height());

   QHBoxLayout * butt_lay=new QHBoxLayout(5);
   vlay->addLayout(butt_lay);
   on_butt=new QeCheckBox(tr("Toolbar enabled"), this, "on_butt");
   on_butt->setChecked(prefs->toolBarOn);
   butt_lay->addWidget(on_butt);
   butt_lay->addStretch(1);
   vis_butt=new QeCheckBox(tr("Always visible"), this, "vis_butt");
   vis_butt->setChecked(prefs->toolBarAlwaysVisible);
   vis_butt->setEnabled(prefs->toolBarOn);
   butt_lay->addWidget(vis_butt);
   QHBoxLayout * spin_lay=new QHBoxLayout(5);
   vlay->addLayout(spin_lay);
   delay_label=new QeLabel(tr("Toolbar hide delay"), this, "delay_label");
   delay_label->setEnabled(prefs->toolBarOn && !prefs->toolBarAlwaysVisible);
   spin_lay->addWidget(delay_label);
   delay_spin=new QeSpinBox(0, 5000, 250, this, "delay_spin");
   delay_spin->setSuffix(" msec");
   delay_spin->setValue(prefs->toolBarDelay);
   delay_spin->setEnabled(prefs->toolBarOn && !prefs->toolBarAlwaysVisible);
   spin_lay->addWidget(delay_spin);
   
   vlay->activate();

   connect(on_butt, SIGNAL(toggled(bool)), this, SLOT(slotTBToggled(bool)));
   connect(vis_butt, SIGNAL(toggled(bool)), this, SLOT(slotTBToggled(bool)));
}
