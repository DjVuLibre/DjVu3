//C-  -*- C++ -*-
//C-
//C-  Copyright � 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_print_dialog.cpp,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_print_dialog.h"
#include "qlib.h"
#include "debug.h"
#include "exc_msg.h"
#include "exc_misc.h"
#include "djvu_base_res.h"
#include "GOS.h"
#include "ByteStream.h"

#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtooltip.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qprogressbar.h>
#include <qspinbox.h>
#include <qcombobox.h>

#include "qt_fix.h"

#ifndef QT1
#include <qmotifstyle.h>
#include <q1xcompatibility.h>
static QMotifStyle *motif=0;
#endif


static const char print_page_str[]="current page";
static const char print_custom_str[]="custom pages";
static const char print_doc_str[]="document";
static const char print_win_str[]="window";

static const char fit_page_str[]="Reduce to Fit";
static const char one_to_one_str[]="One to One";
static const char current_zoom_str[]="Current Zoom";
static const char custom_zoom_str[]="Custom Zoom:";

const char *
QDPrintDialog::id2str(int id)
{
   return
      id==PRINT_PAGE ? print_page_str :
      id==PRINT_CUSTOM ? print_custom_str :
      id==PRINT_DOC ? print_doc_str :
      id==PRINT_WIN ? print_win_str : 0;
}

int
QDPrintDialog::str2id(const char * str)
{
   return
      !strcmp(str, print_page_str) ? PRINT_PAGE :
      !strcmp(str, print_custom_str) ? PRINT_CUSTOM :
      !strcmp(str, print_doc_str) ? PRINT_DOC :
      !strcmp(str, print_win_str) ? PRINT_WIN : -1;
}

void
QDPrintDialog::setPSFormat(bool ps)
{
   if (ps) ps_butt->setChecked(TRUE);
   else eps_butt->setChecked(TRUE);
}

void
QDPrintDialog::setPortrait(bool portrait)
{
   if (portrait) portrait_butt->setChecked(TRUE);
   else landscape_butt->setChecked(TRUE);
}

void
QDPrintDialog::setColorMode(bool color)
{
   if (color) color_butt->setChecked(TRUE);
   else grey_butt->setChecked(TRUE);
}

void
QDPrintDialog::setPSLevel(int level)
{
   if (level==2) level2_butt->setChecked(TRUE);
   else level1_butt->setChecked(TRUE);
}

void
QDPrintDialog::setZoom(int zoom)
{
#ifdef QT1
   const char *setting;
#else
   QString setting;
#endif
   if (zoom<0)
   {
      setting=fit_page_str;
   }else if (zoom==100)
   {
      setting=one_to_one_str;
   }else if (zoom==cur_zoom)
   {
      setting=current_zoom_str;
   }else
   {
      setting=custom_zoom_str;
   }
   zoom_menu->setCurrentItem(setting);
}

void
QDPrintDialog::setCurZoom(int zoom)
{
   cur_zoom=zoom;
   if (!strcmp(zoom_menu->currentText(), current_zoom_str))
      zoom_spin->setValue(cur_zoom);
}

void
QDPrintDialog::setPrint(What what)
{
   const char * str=id2str(what);
   for(int i=0;i<what_menu->count();i++)
      if (!strcmp(what_menu->text(i), str))
      {
	 what_menu->setCurrentItem(i);
	 break;
      }
}

void
QDPrintDialog::setFileName(const QString &qname)
{
   file_text->setText(qname);
}

void
QDPrintDialog::setCommand(const QString &qcmd)
{
   printer_text->setText(qcmd);
}

void
QDPrintDialog::printToFile(int file)
{
   if (file) file_butt->setChecked(TRUE);
   else printer_butt->setChecked(TRUE);
}

void
QDPrintDialog::slotBrowse(void)
{
   int ps_mode=ps_butt->isChecked();
   const char * filter=ps_mode ? "*.ps" : "*.eps";

   QeFileDialog dialog(QFileInfo(file_text->text()).dirPath(),
		       filter, this, "file_dialog", TRUE);
   dialog.setCaption("Select output file name");
   dialog.setForWriting(1);

   if (dialog.exec()==QDialog::Accepted)
      file_text->setText(dialog.selectedFile());
}

void
QDPrintDialog::adjustScaling(void)
{
   if (eps_butt->isChecked())
   {
#ifdef QT1
     const char *setting=one_to_one_str;
#else
     const QString setting(one_to_one_str);
#endif
     zoom_menu->setCurrentItem(setting);
   }
   zoom_menu->setEnabled(ps_butt->isChecked());
}

void
QDPrintDialog::slotWhatChanged(const QString & qtext)
{
   const char * const text=qtext;
   custompages_text->setEnabled(!(strcmp(text, print_custom_str)));
}

void
QDPrintDialog::adjustWhat(void)
{
   bool force_one_page=false;
   bool force_one_copy=false;
   if (eps_butt->isChecked() || !doc || doc->get_pages_num()==1)
      force_one_page=true;
   if (eps_butt->isChecked())
      force_one_copy=true;

   int id=-1;
   if (what_menu->count()>0)
      id=str2id(what_menu->currentText());
   what_menu->clear();
   
   if (!force_one_page)
      what_menu->insertItem(print_custom_str);
   what_menu->insertItem(print_page_str);
   if (!force_one_page)
      what_menu->insertItem(print_doc_str);
   if (!print_rect.isempty())
      what_menu->insertItem(print_win_str);
   
   if (id>=0)
      setPrint((What) id);
   else
      setPrint(PRINT_PAGE);
   
   custompages_label->setEnabled(!force_one_page);
   copies_spin->setEnabled(!force_one_copy);
   if (force_one_copy)
      copies_spin->setValue(1);
}

void
QDPrintDialog::slotFormatChanged(void)
{
   char buffer[MAXPATHLEN+1];
   strcpy(buffer, file_text->text());

   int ps_mode=ps_butt->isChecked();

   char * ptr=buffer, * dot;
   for(ptr=dot=buffer;*ptr;ptr++)
      if (*ptr=='.') dot=ptr;
   if (dot!=buffer)
   {
      *dot=0;
      if (ps_mode) strcat(buffer, ".ps");
      else strcat(buffer, ".eps");
      file_text->setText(buffer);
   }
	 
   if (!ps_mode) portrait_butt->setChecked(TRUE);
   portrait_butt->setEnabled(ps_mode);
   landscape_butt->setEnabled(ps_mode);

   adjustScaling();
   adjustWhat();
}

void
QDPrintDialog::slotDstChanged(void)
{
   int printer=printer_butt->isChecked();
   dst_widget->setActiveWidget(printer ? printer_widget : file_widget);
   eps_butt->setEnabled(!printer);
   ps_butt->setEnabled(!printer);
   if (printer) ps_butt->setChecked(TRUE);
}

void
QDPrintDialog::slotZoomChanged(const QString & qtext)
{
   const char * const text=qtext;
   zoom_spin->setEnabled(!strcmp(text, custom_zoom_str));
   if (!strcmp(text, one_to_one_str))
      zoom_spin->setValue(100);
   else if (!strcmp(text, current_zoom_str))
      zoom_spin->setValue(cur_zoom);
}

void
QDPrintDialog::refresh_cb(void * cl_data)
{
   QDPrintDialog * th=(QDPrintDialog *) cl_data;
   qApp->processEvents();
   
   if (th->interrupt_printing)
      throw Interrupted("QDPrintDialog::refresh_cb", "Printing interrupted.");
}

void
QDPrintDialog::decProgress_cb(float done, void * cl_data)
{
   QDPrintDialog * th=(QDPrintDialog *) cl_data;
   if (done<0)
   {
	 // Alas the progress is unknown.
      char buffer[128];
      sprintf(buffer, "Decoding: %d bytes", (int) -done);
      th->progress->setPrefix(buffer);
      th->progress->setProgress(0);
      th->progress->show();
   } else th->progress->setProgress(done*20);
}

void
QDPrintDialog::prnProgress_cb(float done, void * cl_data)
{
   QDPrintDialog * th=(QDPrintDialog *) cl_data;
   th->progress->setProgress(done*20);
}

void
QDPrintDialog::info_cb(int page_num, int page_cnt, int tot_pages,
		       DjVuToPS::Stage stage, void * cl_data)
{
   QDPrintDialog * th=(QDPrintDialog *) cl_data;
   QString prefix=stage==DjVuToPS::DECODING ? "Decoding" : "Printing";
   if (tot_pages>1)
   {
      char buffer[128];
      sprintf(buffer, " page #%d (%d/%d): ", page_num+1,
	      page_cnt+1, tot_pages);
      prefix=prefix+buffer;
   } else
   {
      if (th->doc && th->doc->get_pages_num()>1)
      {
	 char buffer[128];
	 sprintf(buffer, " page %d: ", page_num+1);
	 prefix=prefix+buffer;
      } else prefix=prefix+": ";
   }
   th->progress->setPrefix(prefix);
   th->progress->reset();
}

void
QDPrintDialog::done(int rc)
{
   DEBUG_MSG("QDPrintDialog::done() called\n");
   DEBUG_MAKE_INDENT(3);
   
   if (rc==Accepted)
   {
      DEBUG_MSG("OK pressed...\n");
      FILE * file=0;
      QString fname;

      try
      {
	 int what=str2id(what_menu->currentText());
	 QString customPages=custompages_text->text();
	 if (what==PRINT_CUSTOM && customPages.length()==0)
	    throw ERROR_MESSAGE("QDPrintDialog::done",
				"Empty \"Custom pages\" list specified.");
	 
	 if (file_butt->isChecked())
	 {
	    QString fname=file_text->text();
	    struct stat st;
	    if (stat(fname, &st)>=0)
	       if (QMessageBox::warning(this, "DjVu",
					"File '"+fname+"' already exists.\n"
					"Are you sure you want to overwrite it?",
					"&Yes", "&No", 0, 0, 1))
		  return;
	 }
      
	 printing=1;
	 interrupt_printing=0;

	 ::setEnabled(this, FALSE);
	 ::setEnabled(cancel_butt, TRUE);
	 ::setEnabled(progress, TRUE);
	 
	 bool printToFile=file_butt->isChecked();
	 bool printColor=color_butt->isChecked();
	 bool printPortrait=portrait_butt->isChecked();
	 bool printPS=ps_butt->isChecked();
	 bool printLevel2=level2_butt->isChecked();
	 int zoom=!strcmp(zoom_menu->currentText(), fit_page_str) ?
		  DjVuToPS::Options::FIT_PAGE :
		  !strcmp(zoom_menu->currentText(), one_to_one_str) ? 100 :
		  !strcmp(zoom_menu->currentText(), current_zoom_str) ? cur_zoom :
		  zoom_spin->value();
	 	 
	 QString printFile=file_text->text();
	 QString printCommand=printer_text->text();
	 
	 if (printToFile) fname=printFile; else fname=tmpnam(0);

	 DEBUG_MSG("creating file '" << (const char *) fname << "'\n");
	 GP<ByteStream> gstr=ByteStream::create(GURL::Filename::UTF8(GStringFromQString(fname)),"wb");
	 ByteStream &str=*gstr;

	 prog_widget->setActiveWidget(progress);
	 progress->reset();
	 GP<DjVuToPS> print=DjVuToPS::create();
	 if (prefs->dPrinterGamma>0)
	    print->options.set_gamma(prefs->dPrinterGamma);
	 else
	    print->options.set_gamma(prefs->dScreenGamma);
	 DjVuToPS::Options & opt=print->options;
	 opt.set_mode(displ_mode==IDC_DISPLAY_BACKGROUND ? DjVuToPS::Options::BACK :
		      displ_mode==IDC_DISPLAY_BLACKWHITE ? DjVuToPS::Options::BW :
		      displ_mode==IDC_DISPLAY_FOREGROUND ? DjVuToPS::Options::FORE :
		      DjVuToPS::Options::COLOR);
	 
	 opt.set_format(printPS ? DjVuToPS::Options::PS : DjVuToPS::Options::EPS);
	 opt.set_level(printLevel2 ? 2 : 1);
	 opt.set_orientation(printPortrait ? DjVuToPS::Options::PORTRAIT :
			     DjVuToPS::Options::LANDSCAPE);
	 opt.set_color(printColor);
	 opt.set_zoom((DjVuToPS::Options::Zoom) zoom);
	 opt.set_copies(copies_spin->value());
	 opt.set_frame(what==PRINT_WIN);
	 print->set_refresh_cb(refresh_cb, this);
	 print->set_dec_progress_cb(decProgress_cb, this);
	 print->set_prn_progress_cb(decProgress_cb, this);
	 print->set_info_cb(info_cb, this);
	 if ((what==PRINT_DOC || what==PRINT_CUSTOM) && doc)
	    print->print(str, doc, (what==PRINT_CUSTOM && customPages.length()) ?
			(const char *) customPages : 0);
	 else
	 {
	    GRect img_rect(0, 0, dimg->get_width(), dimg->get_height());
	    GRect prn_rect=(what==PRINT_WIN) ? print_rect : img_rect;
	    if (prn_rect.isempty())
	       prn_rect=img_rect;
	    print->print(str, dimg, prn_rect, img_rect);
	 }
	 prog_widget->setActiveWidget(save_butt);
	 str.flush();
      
	 if (!printToFile)
	 {
	    DEBUG_MSG("sending created file to printer...\n");
	    QString cmd="cat "+fname+" | "+printCommand+"; rm -f "+fname;
	    DEBUG_MSG("command='" << (const char *) cmd << "'\n");
	    system(cmd);
	    fname="";
	 }
      
	 DEBUG_MSG("updating preferences\n");
	 DjVuPrefs disk_prefs;
	 disk_prefs.printLevel2=prefs->printLevel2=printLevel2;
	 disk_prefs.printPS=prefs->printPS=printPS;
	 disk_prefs.printToFile=prefs->printToFile=printToFile;
	 disk_prefs.printColor=prefs->printColor=printColor;
	 disk_prefs.printPortrait=prefs->printPortrait=printPortrait;
	 disk_prefs.printFile=prefs->printFile=GStringFromQString(printFile);
	 disk_prefs.printCommand=prefs->printCommand=GStringFromQString(printCommand);
	 disk_prefs.printFitPage=zoom==DjVuToPS::Options::FIT_PAGE;
	 disk_prefs.printAllPages=prefs->printAllPages=
				  what==PRINT_DOC || what==PRINT_CUSTOM;
	 disk_prefs.save();

	 ::setEnabled(this, TRUE);
	 setSensitivity();
	 QeDialog::done(rc);
      } catch(Interrupted &)
      {
	 if (file) fclose(file);
	 if (fname!="") unlink(fname);
	 prog_widget->setActiveWidget(save_butt);
	 printing=0;
	 ::setEnabled(this, TRUE);
	 setSensitivity();
      } catch(const GException & exc)
      {
	 if (file) fclose(file);
	 if (fname!="") unlink(fname);
	 prog_widget->setActiveWidget(save_butt);
	 printing=0;
	 ::setEnabled(this, TRUE);
	 setSensitivity();
      
	 showError(this, exc);
      }
   } else
   {
      if (printing)
      {
	 DEBUG_MSG("interrupting printing\n");
	 interrupt_printing=1;
      } else QeDialog::done(rc);
   }
}

void
QDPrintDialog::setSensitivity(void)
{
   slotFormatChanged();
   slotWhatChanged(what_menu->currentText());
   slotDstChanged();
   
   adjustScaling();
   adjustWhat();
}

bool
QDPrintDialog::eventFilter(QObject * obj, QEvent * ev)
      // Why do I need this function?!
      // Because QT sucks compared to Motif's geometry management.
      // There I would simply use XmATTACH_POSITION and XmATTACH_OPPOSITE_WIDGET
      //
      // Here I'm trying to achieve this:
      //   - The 'format', 'orientation' and 'color' boxes all have
      //     the same width. QHBoxLayout or QGridLayout can do it only
      //     if *stretch* and *minimum width* are the same for every box.
      //     This is precily what I'm doing: equalizing minimumWidth.
      //   - The 'What' box is twice as big as adjacent 'Scale' box.
      //
      // You may argue, that this can be done with just one QGridLayout,
      // and I'd reply again, that QT sucks, because "multicell widgets
      // do not affect columns minimum width", which means, that should
      // the "what" box become bigger than 2*"scale" box, the dialog
      // will not grow.
{
   if (ev->type()==Event_LayoutHint)
   {
	 // Adjust the sizes of 'Scale' and 'What' boxes first
      QSize scale_size=scale_bg->minimumSize();
      QSize what_size=what_bg->minimumSize();
      if (scale_size.width()*2>what_size.width()-10)
	 what_bg->setMinimumWidth(scale_size.width()*2+10);
      else if (scale_size.width()*2<what_size.width()+10)
      {
	 scale_bg->setMinimumWidth((what_size.width()-10)/2);
	 what_bg->setMinimumWidth(scale_bg->minimumSize().width()*2+10);
      }

	 // Now see if we need to equalize minimum widths of
	 // 'Format', 'Orientation' and 'Color' boxes.
      QSize format_size=format_bg->minimumSize();
      QSize orient_size=orient_bg->minimumSize();
      QSize color_size=color_bg->minimumSize();
      int max_width=format_size.width();
      if (max_width<orient_size.width())
	 max_width=orient_size.width();
      if (max_width<color_size.width())
	 max_width=color_size.width();
      if (max_width>format_size.width())
	 format_bg->setMinimumWidth(max_width);
      if (max_width>orient_size.width())
	 orient_bg->setMinimumWidth(max_width);
      if (max_width>color_size.width())
	 color_bg->setMinimumWidth(max_width);
   }
   return FALSE;
}

QDPrintDialog::QDPrintDialog(const GP<DjVuDocument> & _doc,
			     const GP<DjVuImage> & _dimg,
			     DjVuPrefs * _prefs, int _displ_mode,
			     int _cur_zoom, const GRect & _print_rect,
			     QWidget * parent, const char * name, bool modal) :
      QeDialog(parent, name, modal), doc(_doc), dimg(_dimg), cur_page_num(0),
      prefs(_prefs), displ_mode(_displ_mode), cur_zoom(_cur_zoom),
      print_rect(_print_rect)
{
   DEBUG_MSG("QDPrintDialog(): Initializing...\n");

   printing=0;
   interrupt_printing=0;

   if (!prefs) throw ERROR_MESSAGE("QDPrintDialog::QDPrintDialog",
				   "Zero PREFERENCES passed as input.");
   if (!doc) throw ERROR_MESSAGE("QDPrintDialog::QDPrintDialog",
				 "Zero document passed as input.");
   if (!dimg) throw ERROR_MESSAGE("QDPrintDialog::QDPrintDialog",
				  "Zero image passed as input.");

   if (print_rect.isempty())
      print_rect=GRect(0, 0, dimg->get_width(), dimg->get_height());

   cur_page_num=doc->url_to_page(dimg->get_djvu_file()->get_url());

   QWidget * start=startWidget();
   QeLabel * label;
   
   setCaption("DjVu Print Dialog");

   start->installEventFilter(this);
   
   QVBoxLayout * vlay=new QVBoxLayout(start, 10);
   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);

   QeButtonGroup * bg;
   QVBoxLayout * bg_lay;

      //************* Creating 'File format' frame *******************
   bg=format_bg=new QeButtonGroup("File format", start);
   hlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   ps_butt=new QeRadioButton("&PostScript", bg, "ps_butt");
   bg_lay->addWidget(ps_butt);
   eps_butt=new QeRadioButton("&EPSF", bg, "eps_butt");
   bg_lay->addWidget(eps_butt);
   bg_lay->activate();

      //************* Creating 'Orientation' frame *******************
   bg=orient_bg=new QeButtonGroup("Orientation", start);
   hlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   portrait_butt=new QeRadioButton("Po&rtrait", bg, "portrait_butt");
   bg_lay->addWidget(portrait_butt);
   landscape_butt=new QeRadioButton("&Landscape", bg, "landscape_butt");
   bg_lay->addWidget(landscape_butt);
   bg_lay->activate();

      //************* Creating 'Color model' frame *******************
   bg=color_bg=new QeButtonGroup("Color model", start);
   hlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   color_butt=new QeRadioButton("&Color", bg, "color_butt");
   bg_lay->addWidget(color_butt);
   grey_butt=new QeRadioButton("&GreyScale", bg, "grey_butt");
   bg_lay->addWidget(grey_butt);
   bg_lay->activate();

   hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   
   //************* Creating 'Scaling' frame *******************
   bg=scale_bg=new QeButtonGroup("Scaling", start);
   hlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   zoom_menu=new QeComboBox(FALSE, bg, "zoom_menu");
   zoom_menu->insertItem(fit_page_str);
   zoom_menu->insertItem(one_to_one_str);
   zoom_menu->insertItem(current_zoom_str);
   zoom_menu->insertItem(custom_zoom_str);
   bg_lay->addWidget(zoom_menu);
   zoom_spin=new QeSpinBox(5, 999, 1, bg, "zoom_spin");
   zoom_spin->setSuffix("%");
   zoom_spin->setEnabled(FALSE);
   zoom_spin->setValue(100);
   bg_lay->addWidget(zoom_spin);
   bg_lay->activate();

   QHBoxLayout * bg_hlay;
   
   //************* Creating 'What to Print' frame *******************
   bg=what_bg=new QeButtonGroup("What to print", start);
   hlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   bg_hlay=new QHBoxLayout(10);
   bg_lay->addLayout(bg_hlay);
   
   copies_spin=new QeSpinBox(1, 99, 1, bg, "print_spin");
   copies_spin->setSpecialValueText("1 copy");
   copies_spin->setSuffix(" copies");
   copies_spin->setValue(1);
   bg_hlay->addWidget(copies_spin);

   label=new QeLabel("of the", bg);
   bg_hlay->addWidget(label);
   
   what_menu=new QeComboBox(FALSE, bg, "print_menu");
   bg_hlay->addWidget(what_menu);

   bg_hlay=new QHBoxLayout(10);
   bg_lay->addLayout(bg_hlay);

   custompages_label=new QeLabel("C&ustom pages:", bg);
   bg_hlay->addWidget(custompages_label);
   custompages_text=new QeLineEdit(bg, "custompages_text");
   custompages_text->setMaxLength(128);
   custompages_text->setEnabled(FALSE);
   bg_hlay->addWidget(custompages_text, 1);
   custompages_label->setBuddy(custompages_text);
   bg_lay->activate();
   static const char tip[]="Enter a list of page ranges here\n"
			   "separated by commas like 1,2,10-12.\n"
			   "To make multiple copies try 1,1,1,1.\n"
			   "To print in reverse try 10-1.";
   QToolTip::add(custompages_label, tip);
   QToolTip::add(custompages_text, tip);
   

      //*********** Creating 'PostScript level' frame ****************
   QGridLayout * bgrid;
   bg=new QeButtonGroup("PostScript level", start);
   vlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bgrid=new QGridLayout(2, 2, 20); bg_lay->addLayout(bgrid);
   level1_butt=new QeRadioButton("Level &1", bg, "level1_butt");
   bgrid->addWidget(level1_butt, 0, 0);
   label=new QeLabel("Use this setting to generate a portable\n"
		     "PostScript file. Images will not be compressed.", bg, "level1_label");
   bgrid->addWidget(label, 0, 1);
   level2_butt=new QeRadioButton("Level &2", bg, "level2_butt");
   bgrid->addWidget(level2_butt, 1, 0);
   label=new QeLabel("Create a smaller PostScript file. The file will not be\n"
		     "compatible with PS Level 1 printers.",
		     bg, "level2_label");
   bgrid->addWidget(label, 1, 1);
 
   bg_lay->activate();

      //**************** Creating the "destination" frame *************
   QeGroupBox * gb=new QeGroupBox("Print destination", start, "gb");
   vlay->addWidget(gb);
   QVBoxLayout * gb_lay=new QVBoxLayout(gb, 10, 5, "gb_lay");
   gb_lay->addSpacing(gb->fontMetrics().height());
   QHBoxLayout * gb_lay1=new QHBoxLayout(0, "gb_lay1");
   gb_lay->addLayout(gb_lay1);
   QWidget * top_w=new QWidget(gb, "top_w");
   gb_lay1->addStretch(1);
   gb_lay1->addWidget(top_w);
   gb_lay1->addStretch(1);
   QeNInOne * bot_w=new QeNInOne(gb, "bot_w");
   gb_lay->addWidget(bot_w);
   gb_lay->activate();

      // *** top_w
   QHBoxLayout * top_lay=new QHBoxLayout(top_w, 0, 5, "top_lay");
   label=new QeLabel("Send Image To:", top_w, "crazy_label");
   top_lay->addWidget(label);
   QeButtonGroup * dst_bg=new QeButtonGroup(top_w);
   dst_bg->setFrameStyle(QFrame::NoFrame);
   top_lay->addWidget(dst_bg);
   QHBoxLayout * dst_lay=new QHBoxLayout(dst_bg, 0, 5);
   printer_butt=new QeRadioButton("Printe&r", dst_bg, "printer_butt");
   dst_lay->addWidget(printer_butt);
   file_butt=new QeRadioButton("F&ile", dst_bg, "file_butt");
   dst_lay->addWidget(file_butt);
   dst_lay->activate();
   top_lay->activate();

      // *** bot_w
   dst_widget=bot_w;
   printer_widget=new QWidget(bot_w, "printer_widget");
   QHBoxLayout * print_lay=new QHBoxLayout(printer_widget, 0, 5, "print_lay");
   label=new QeLabel("Print Command:", printer_widget);
   print_lay->addWidget(label);
   printer_text=new QeLineEdit(printer_widget, "printer_text");
   printer_text->setMaxLength(128);
   print_lay->addWidget(printer_text, 1);
   print_lay->activate();

   file_widget=new QWidget(bot_w, "file_widget");
   QHBoxLayout * file_lay=new QHBoxLayout(file_widget, 0, 5, "file_lay");
   label=new QeLabel("File Name:", file_widget);
   file_lay->addWidget(label);
   file_text=new QeLineEdit(file_widget, "file_text");
   file_text->setMaxLength(128);
   file_lay->addWidget(file_text, 1);
   QePushButton * browse_butt=new QePushButton("Browse...", file_widget, "browse_butt");
   file_lay->addWidget(browse_butt);
   file_lay->activate();

   //***************** Creating the OK/Cancel buttons ***************
   QHBoxLayout * prg_lay=new QHBoxLayout(5, "prg_lay");
   vlay->addLayout(prg_lay);
   prog_widget=new QeNInOne(start, "prog_widget");
   prg_lay->addWidget(prog_widget, 1);
   progress=new QeProgressBar(20, prog_widget, "progress_bar");
#ifdef QT1
   progress->setStyle(MotifStyle);
#else
   if(!motif)
     motif=new QMotifStyle();
   progress->setStyle(motif);
#endif
   save_butt=new QeCheckBox("&Save settings to disk", prog_widget, "save_butt");
   save_butt->setChecked(TRUE);
   prog_widget->setActiveWidget(save_butt);
   QePushButton * ok_butt=new QePushButton("&OK", start, "ok_butt");
   ok_butt->setDefault(TRUE);
   prg_lay->addWidget(ok_butt);
   cancel_butt=new QePushButton("&Cancel", start, "cancel_butt");
   prg_lay->addWidget(cancel_butt);
   
   vlay->activate();

      // Connecting signals
   connect(ps_butt, SIGNAL(toggled(bool)), this, SLOT(slotFormatChanged(void)));
   connect(eps_butt, SIGNAL(toggled(bool)), this, SLOT(slotFormatChanged(void)));
   connect(file_butt, SIGNAL(toggled(bool)), this, SLOT(slotDstChanged(void)));
   connect(printer_butt, SIGNAL(toggled(bool)), this, SLOT(slotDstChanged(void)));
   connect(browse_butt, SIGNAL(clicked(void)), this, SLOT(slotBrowse(void)));
   connect(what_menu, SIGNAL(activated(const QString &)),
	   this, SLOT(slotWhatChanged(const QString &)));
   connect(zoom_menu, SIGNAL(activated(const QString &)),
	   this, SLOT(slotZoomChanged(const QString &)));
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
   
      // Setting buttons states
   if (!prefs->printToFile) prefs->printPS=1;
   if (!prefs->printPS) prefs->printPortrait=1;
   setPSFormat(prefs->printPS);
   setPortrait(prefs->printPortrait);
   setColorMode(prefs->printColor);
   setPSLevel(prefs->printLevel2 ? 2 : 1);
   setFileName(QStringFromGString(prefs->printFile));
   setCommand(QStringFromGString(prefs->printCommand));
   printToFile(prefs->printToFile);
   if (prefs->printFitPage) setZoom(-1);
   else setZoom(100);
   setPrint(prefs->printAllPages ? PRINT_DOC : PRINT_PAGE);

   if (!dimg->is_legal_photo() && !dimg->is_legal_compound() ||
       displ_mode==IDC_DISPLAY_BLACKWHITE)
      grey_butt->setChecked(TRUE);

   adjustScaling();
   adjustWhat();

   setSensitivity();
}
