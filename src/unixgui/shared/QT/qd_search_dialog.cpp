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
// $Id: qd_search_dialog.cpp,v 1.4.2.1 2001-10-23 21:16:48 leonb Exp $
// $Name:  $

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "DjVuAnno.h"
#include "DjVuFile.h"
#include "DataPool.h"
#include "ByteStream.h"

#include "qd_search_dialog.h"

#include "debug.h"
#include "qlib.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qapplication.h>

#include "qt_fix.h"

bool	QDSearchDialog::all_pages=true;
bool	QDSearchDialog::case_sensitive=false;
bool	QDSearchDialog::whole_word=false;
bool	QDSearchDialog::search_backwards=false;

void
QDSearchDialog::slotSearch(void)
{
   if (in_search)
   {
      stop=true;
      return;
   }
   
   if (text->text()=="") return;
   
   try
   {
      bool fwd=!back_butt->isChecked();
      in_search=true;
      stop=false;
      search_butt->setText(tr("&Stop"));
      text->setEnabled(FALSE);
      clear_butt->setEnabled(FALSE);

	 // 'asked_once' is useful for one-page searches
      bool asked_once=false;
	 // 'start_page_num' is for the document search
      int start_page_num=page_num;
	 // Will be true if no TXT chunks have been found
      bool no_txt=true;

	 // We want to place 'anno' here because to keep it alive as long
	 // as zone_list is alive.
      GP<DjVuText> anno;
      GList<DjVuTXT::Zone *> zone_list;
      while(true)
      {
	 GP<DjVuFile> page_file=doc->get_djvu_file(page_num);

	 if (!page_file->is_all_data_present())
	 {
	    GP<DataPool> pool=page_file->get_init_data_pool();
	    int last_done=-1;
	       // Wait until we have data for this file
	    while(!stop && !page_file->is_all_data_present())
	    {
	       int done=5*(20*pool->get_size()/pool->get_length());
	       if (done!=last_done)
	       {
		  QString buffer=tr("Loading page ")+QString::number(page_num+1)+": "+
		     QString::number(done)+"%...";
		  status_label->setText(buffer);
		  last_done=done;
	       }
	       qApp->processEvents(100);
	    }
	 }
	 if (stop)
	 {
	    page_num=seen_page_num;
	    break;
	 }
	 
	 GP<ByteStream> str=page_file->get_text();
	 if (str)
	 {
	    str->seek(0);
	    anno=DjVuText::create();
	    anno->decode(str);
	    GP<DjVuTXT> txt=anno->txt;
	    if (txt)
	    {
	       no_txt=false;
	       int saved_page_pos=page_pos;
	       zone_list=txt->search_string(GStringFromQString(text->text()), page_pos,
					    fwd, case_butt->isChecked(),
					    whole_word_butt->isChecked());
	       if (page_pos==saved_page_pos)
	       {
		     // Arghh. found the same thing. Shift the page_pos
		  if (fwd) page_pos++;
		  else page_pos--;
		  zone_list=txt->search_string(GStringFromQString(text->text()), page_pos,
					       fwd, case_butt->isChecked());
	       }
	       if (zone_list.size()) break;
	    }
	 }
	    // Didn't find anything. Switch to the next page or give up
	 int pages_num=doc->get_pages_num();
	 if (!all_pages_butt->isChecked() ||
	     fwd && page_num>=pages_num-1 ||
	     !fwd && page_num==0)
	 {
	    if (asked_once)
	    {
	       page_num=seen_page_num;
	       break;
	    } else
	    {
	       asked_once=true;
	       QString msg=all_pages_butt->isChecked() ? tr("document") : tr("page");
	       if (fwd)
               {
                 msg=tr("End of ")+msg+tr(" reached. Continue from the beginning?");
               }else
               {
                 msg=tr("Beginning of ")+msg+tr(" reached. Continue from the end?");
               }
	       if (QMessageBox::information(this, "DjVu", msg, tr("&Yes"), tr("&No"))==0)
	       {
		  if (all_pages_butt->isChecked())
		  {
		     if (fwd) page_num=-1;
		     else page_num=doc->get_pages_num();
		  } else
		  {
		     if (fwd) page_num--;
		     else page_num++;
		  }
	       } else
	       {
		  page_num=seen_page_num;
		  break;
	       }
	    }
	 }
	    // Going to the next page (or rewinding to the other side of
	    // this page if asked to)
	 if (fwd)
	 {
	    page_num++;
	    page_pos=-1;
	 } else
	 {
	    page_num--;
	    page_pos=0xffffff;
	 }

         {
           QString mesg=tr("Page ")+QString::number(page_num+1);
	   status_label->setText(mesg);
         }

	    // Wrapped back and returned
	 if (all_pages_butt->isChecked() &&
	     page_num==start_page_num)
	 {
	    page_num=seen_page_num;
	    break;
	 }
      } // while(true)

      emit sigDisplaySearchResults(seen_page_num=page_num, zone_list);
      if (zone_list.size()==0)
      {
	 if (no_txt)
	 {
	    if (all_pages_butt->isChecked())
	       showMessage(this, tr("DjVu Search Failed"),
			   tr("After looking through every page of this document\n")+
			   tr("we have found, that neither of them contain\ntextual information.")+
			   tr("This means, that the document\ncreator did not run an OCR engine on this document.\n\n")+
			   tr("The search is impossible."), true, false, true);
	    else
	       showMessage(this, tr("DjVu Search Failed"),
			   tr("This page does not contain textual information,\n")+
			   tr("which means that either creator of this document did not run an OCR engine on it, ")+
			   tr("or the OCR engine did not recognize any text on this page.\n\n")+
			   tr("The search is impossible."), true, false, true);
	 } else showInfo(this, "DjVu", tr("Search string not found"));
      }

      in_search=false;
      search_butt->setText(tr("&Find"));
      {
        QString mesg=tr("Page ")+QString::number(page_num+1);
        status_label->setText(mesg);
      }
      text->setEnabled(TRUE);
      clear_butt->setEnabled(TRUE);
   } catch(const GException & exc)
   {
      status_label->hide();
      in_search=false;
      search_butt->setText(tr("&Find"));
      {
        QString mesg=tr("Page ")+QString::number(page_num+1);
        status_label->setText(mesg);
      }
      text->setEnabled(TRUE);
      clear_butt->setEnabled(TRUE);
      showError(this, exc);
   }
}

void
QDSearchDialog::slotSetPageNum(int page_num_)
{
   seen_page_num=page_num_;

   if (!in_search)
   {
      if (page_num!=page_num_)
	 page_pos=back_butt->isChecked() ? 0xffffff : -1;
      page_num=seen_page_num;
   }
   
   QString mesg=tr("Page ")+QString::number(page_num+1);
   status_label->setText(mesg);
}

void
QDSearchDialog::slotTextChanged(const QString & qtext)
{
   const char * const str=qtext;
   search_butt->setEnabled(str && strlen(str));
}

void
QDSearchDialog::done(int rc)
{
   stop=true;
   all_pages=all_pages_butt->isChecked();
   case_sensitive=case_butt->isChecked();
   whole_word=whole_word_butt->isChecked();
   search_backwards=back_butt->isChecked();
   QeDialog::done(rc);
}

QDSearchDialog::QDSearchDialog(int page_num_, const GP<DjVuDocument> & doc_,
			       QWidget * parent, const char * name, bool modal) :
      QeDialog(parent, name, modal), page_num(page_num_), doc(doc_)
{
   seen_page_num=page_num;
   page_pos=search_backwards ? 0xffffff : -1;
   in_search=false;
   
   setCaption(tr("DjVu: Find"));
   setResizable(true, false);
   
   QWidget * start=startWidget();
   QeLabel * label;
   
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);

   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   label=new QeLabel(tr("Find: "), start);
   hlay->addWidget(label);
   text=new QeLineEdit(start, "search_text");
   label->setBuddy(text);
   hlay->addWidget(text, 1);

   hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);

   all_pages_butt=new QeCheckBox(tr("Search &all pages"), start, "all_pages_butt");
   if (doc->get_pages_num()>1)
      all_pages_butt->setChecked(all_pages);
   else
   {
      all_pages_butt->setChecked(TRUE);
      all_pages_butt->setEnabled(FALSE);
   }
   hlay->addWidget(all_pages_butt);
   hlay->addStretch(1);
   
   case_butt=new QeCheckBox(tr("&Case sensitive"), start, "case_butt");
   case_butt->setChecked(case_sensitive);
   hlay->addWidget(case_butt);
   hlay->addStretch(1);

   whole_word_butt=new QeCheckBox(tr("&Whole word"), start, "whole_word_butt");
   whole_word_butt->setChecked(whole_word);
   hlay->addWidget(whole_word_butt);
   hlay->addStretch(1);

   back_butt=new QeCheckBox(tr("Search &backwards"), start, "back_butt");
   back_butt->setChecked(search_backwards);
   hlay->addWidget(back_butt);
   
   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   status_label=new QLabel(tr("Loading page WWWWWWW"), start);
   status_label->setMinimumSize(status_label->sizeHint());
   {
     QString mesg=tr("Page ")+QString::number(page_num+1);
     status_label->setText(mesg);
   }
   butt_lay->addWidget(status_label);
   butt_lay->addStretch(1);
   search_butt=new QePushButton(tr("&Find"), start, "search_butt");
   search_butt->setEnabled(FALSE);
   butt_lay->addWidget(search_butt);
   clear_butt=new QePushButton(tr("&Clear"), start, "clear_butt");
   butt_lay->addWidget(clear_butt);
   QePushButton * close_butt=new QePushButton(tr("C&lose"), start, "close_butt");
   butt_lay->addWidget(close_butt);
   search_butt->setDefault(TRUE);

   vlay->activate();

   connect(search_butt, SIGNAL(clicked(void)), this, SLOT(slotSearch(void)));
   connect(clear_butt, SIGNAL(clicked(void)), text, SLOT(clear(void)));
   connect(close_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));

   connect(text, SIGNAL(textChanged(const QString &)),
	   this, SLOT(slotTextChanged(const QString &)));
}
