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
// $Id: qd_loader.cpp,v 1.4 2001-10-12 17:58:31 leonb Exp $
// $Name:  $

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "qd_loader.h"

#include "debug.h"
#include "exc_msg.h"
#include "throw_error.h"
#include "qlib.h"
#include "GOS.h"
#include "djvu_file_cache.h"

#if THREADMODEL==COTHREADS
#include "qd_thr_yielder.h"
#endif

#include <qlayout.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qfiledialog.h>
#include <qkeycode.h>

#include "qt_fix.h"

//*************************** QDPort connected slots **************************

void
QDLoader::slotNotifyError(const GP<DjVuPort> &, const GUTF8String &qmsg)
{
   ::showError(this, tr("DjVu error"), QStringFromGString(qmsg));
}

void
QDLoader::slotNotifyStatus(const GP<DjVuPort> &, const QString &qmsg)
{
   const char * const msg=qmsg;
   GUTF8String text;
   for(const char * ptr=msg;*ptr && *ptr!='\n';ptr++)
      if (*ptr!='\t') text+=*ptr;
      else text+=' ';
   status_label->setText(text);
}

void
QDLoader::slotNotifyFileFlagsChanged(const GP<DjVuFile> & source, long set_mask, long)
{
   if (set_mask & DjVuFile::DECODE_OK)
      if (image && source==image->get_djvu_file())
      {
	 status_label->setText("");
	 accept();
      }
   if (set_mask & (DjVuFile::DECODE_FAILED | DjVuFile::DECODE_STOPPED))
      if (image && source==image->get_djvu_file())
      {
	 status_label->setText("");
	 if (!external_document) setUpForLoad(text->text());
	 else reject();
      }
}

void
QDLoader::slotNotifyDecodeProgress(const GP<DjVuPort> & source, float done)
{
   if (image && source==image->get_djvu_file())
   {
      if (done<0) progress_form->setActiveWidget(progress_label);
      else progress_form->setActiveWidget(progress_bar);
   
      if ((int) (done*50)!=(int) (last_done*50))
      {
	 progress_bar->setProgress((int) (done*100));
	 last_done=done;
      }
   }
}

void
QDLoader::keyPressEvent(QKeyEvent * ev)
{
   if (ev->key()==Key_Escape) slotCancel();
   else QeDialog::keyPressEvent(ev);
}

void
QDLoader::slotBrowse(void)
{
   static const char * filters[]={ "*.djvu", "*.djv", "*.iw44", "*.iw4", tr("All files (*)"), 0 };
   QeFileDialog dialog(QeFileDialog::lastLoadDir, filters[0],
		       this, "file_dialog", TRUE);
   dialog.setCaption(tr("Select a DjVu file to open"));
   dialog.setForWriting(FALSE);
   dialog.setFilters((const char **) filters);

   if (dialog.exec()==QDialog::Accepted)
      text->setText(dialog.selectedFile());
}

void
QDLoader::slotCancel(void)
{
   try
   {
      if (form->getActiveWidget()==load_w) reject();
      else if (image) image->get_djvu_file()->stop_decode(0);
   } catch(const GException & exc)
   {
      ::showError(this, tr("DjVu Error"), exc);
   }
}

void
QDLoader::slotOK(void)
{
   DEBUG_MSG("QDLoader::sloatOK(): OK pressed\n");
   DEBUG_MAKE_INDENT(3);
   
   // This callback can be called only in "loading" stage
   if (form->getActiveWidget()!=load_w) return;

   try
   {
      const char * name=text->text();
      pool_url=GOS::filename_to_url(name);
      pool=new DataPool(name);
      
      startDecoding();
   } catch(const GException & exc)
   {
      ::showError(this, tr("DjVu Error"), exc);
   }
}

void
QDLoader::startDecoding(void)
{
   DEBUG_MSG("QDLoader::startDecoding(): actually decoding...\n");
   DEBUG_MAKE_INDENT(3);

   setUpForDecode();

   last_done=0;

#if THREADMODEL==COTHREADS
   if (!QDThrYielder::isInitialized()) QDThrYielder::initialize();
#endif

   if (!external_document)
   {
      if (!mem_port) mem_port=new DjVuMemoryPort();
      if (!simple_port)
      {
	 simple_port=new DjVuSimplePort();
	 DjVuPort::get_portcaster()->add_route(mem_port, simple_port);
      }
      mem_port->add_data(pool_url, pool);
	 
      document=new DjVuDocument();
      document->init(pool_url, (DjVuPort *) mem_port);
      decode_page_num=-1;
   }
   image=document->get_page(decode_page_num, false, port.getPort());
}

void
QDLoader::show(void)
{
   DEBUG_MSG("QDLoader::show(): showing the dialog...\n");
   DEBUG_MAKE_INDENT(3);

   try
   {
      if (pool || external_document)
      {
	 startDecoding();
	 if (image && image->get_djvu_file()->is_decode_ok()) { accept(); return; }
      }
   } catch(const GException & exc)
   {
      ::showError(this, tr("DjVu Error"), exc);
   }
   QeDialog::show();
}

void
QDLoader::setUpForDecode(void)
{
   DEBUG_MSG("QDLoader::setUpForDecode(): Preparing for decoding...\n");
   DEBUG_MAKE_INDENT(3);

   setCaption("Decoding DjVu page...");

   cancel_butt->setText("&Interrupt");
   progress_bar->reset();
   ok_butt->hide();
   form->setActiveWidget(decode_w);
}

void
QDLoader::setUpForLoad(const QString &qname)
{
   const char * const name=qname;
   DEBUG_MSG("QDLoader:setUpForLoad(): letting user to set up the file name...\n");
   DEBUG_MAKE_INDENT(3);

   setCaption(tr("Select DjVu file to open..."));

   GUTF8String load_dir=QeFileDialog::lastLoadDir;
   GUTF8String file_name;
   if (name && strlen(name)) file_name=GOS::expand_name(name, load_dir);
   else file_name=GOS::expand_name("image.djvu", load_dir);

   DEBUG_MSG("file_name=" << file_name << "\n");

   text->setText(file_name);
   cancel_butt->setText("&Cancel");
   ok_butt->show();
   form->setActiveWidget(load_w);
}

void
QDLoader::init(void)
{
   DEBUG_MSG("QDLoader::init(): Creating all widgets\n");
   DEBUG_MAKE_INDENT(3);

   mem_port=0;
   simple_port=0;
   
   QWidget * start=startWidget();
   QeLabel * label;
   QFont font;

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10, "vlay");
   form=new QeNInOne(start, "loader_form");
   form->dontResize(TRUE);
   vlay->addWidget(form);

      // Creating the "load" widget
   load_w=new QWidget(form, "load_w");
   QVBoxLayout * lvlay=new QVBoxLayout(load_w, 0, 20, "lvlay");
   label=new QeLabel(tr("Please enter the DjVu file name to load in the field below"),
		     load_w, "load_title_label");
   font=label->font(); font.setWeight(QFont::Bold); label->setFont(font);
   label->setAlignment(AlignCenter);
   lvlay->addWidget(label);
   QHBoxLayout * lhlay=new QHBoxLayout(10, "lhlay");
   lvlay->addLayout(lhlay);
   label=new QeLabel(tr("DjVu file:"), load_w);
   lhlay->addWidget(label);
   text=new QLineEdit(load_w, "load_text");
   font=text->font(); font.setFamily("courier");
   text->setFont(font);
   text->setMinimumWidth(text->fontMetrics().width("abcdezyxvu")*4);
   lhlay->addWidget(text, 1);
   QePushButton * browse_butt=new QePushButton(tr("&Browse"), load_w, "browse_butt");
   lhlay->addWidget(browse_butt);
   lvlay->activate();

      // Creating the "decode" widget
   decode_w=new QWidget(form, "decode_w");
   QVBoxLayout * dvlay=new QVBoxLayout(decode_w, 0, 20, "dvlay");
   label=new QeLabel(tr("DjVu page is being decoded... Please stand by..."),
		     decode_w, "decode_title_label");
   font=label->font(); font.setWeight(QFont::Bold); label->setFont(font);
   label->setAlignment(AlignCenter);
   dvlay->addWidget(label);
   progress_form=new QeNInOne(decode_w, "progress_form");
   progress_form->dontResize(TRUE);
   dvlay->addWidget(progress_form);
   dvlay->activate();
   progress_bar=new QProgressBar(100, progress_form, "progress_bar");
   progress_bar->setMinimumHeight(progress_bar->sizeHint().height());
   progress_label=new QeLabel(tr("Completed % is unknown"), progress_form, "progress_label");
   progress_label->setAlignment(AlignCenter);

      // Creating the buttons
   QHBoxLayout * butt_lay=new QHBoxLayout(10, "butt_lay");
   vlay->addLayout(butt_lay);
   status_label=new QeLabel("", start, "status_label");
   status_label->setAutoResize(TRUE);
   butt_lay->addWidget(status_label);
   butt_lay->addStretch(1);
   ok_butt=new QePushButton(tr("&Load"), start, "ok_butt");
   ok_butt->setDefault(TRUE);
   butt_lay->addWidget(ok_butt);
   cancel_butt=new QePushButton(tr("&Cancel"), start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);

   vlay->activate();

      // connecting some signals
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(slotOK(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(slotCancel(void)));
   connect(browse_butt, SIGNAL(clicked(void)), this, SLOT(slotBrowse(void)));

   connect(&port, SIGNAL(sigNotifyError(const GP<DjVuPort> &, const GUTF8String &)),
	   this, SLOT(slotNotifyError(const GP<DjVuPort> &, const GUTF8String &)));
   connect(&port, SIGNAL(sigNotifyStatus(const GP<DjVuPort> &, const QString &)),
	   this, SLOT(slotNotifyStatus(const GP<DjVuPort> &, const QString &)));
   connect(&port, SIGNAL(sigNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)),
	   this, SLOT(slotNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)));
   connect(&port, SIGNAL(sigNotifyDecodeProgress(const GP<DjVuPort> &, float)),
	   this, SLOT(slotNotifyDecodeProgress(const GP<DjVuPort> &, float)));
}

QDLoader::QDLoader(const char * fname, QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE), external_document(0),
      decode_page_num(-1), port(1, 1)
{
   DEBUG_MSG("QDLoader::QDLoader(): Initializing class\n");
   DEBUG_MAKE_INDENT(3);

   init();

   setUpForLoad(fname);
}

QDLoader::QDLoader(const char * fname, const GP<DataPool> & _pool,
		   QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE), pool(_pool), external_document(0),
      decode_page_num(-1), port(1, 1)
{
   DEBUG_MSG("QDLoader::QDLoader(): Initializing class\n");
   DEBUG_MAKE_INDENT(3);

   GUTF8String full_fname=GOS::expand_name(fname);
   pool_url=GOS::filename_to_url(full_fname);
   
   init();
   text->setText(full_fname);

   setUpForDecode();
}

QDLoader::QDLoader(const GP<DjVuDocument> & _document, int page_num,
		   QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE), document(_document), external_document(1),
      decode_page_num(page_num), port(1, 1)
{
   DEBUG_MSG("QDLoader::QDLoader(): Initializing class\n");
   DEBUG_MAKE_INDENT(3);

   if (!document) G_THROW("Internal error: ZERO document passed as input.");
   
   init();

   setUpForDecode();
}
