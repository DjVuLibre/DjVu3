//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_search_dialog.cpp,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
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
      search_butt->setText("&Stop");
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
		  char buffer[128];
		  sprintf(buffer, "Loading page %d: %d%%...", page_num+1, done);
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
	       GUTF8String msg=all_pages_butt->isChecked() ? "document" : "page";
	       if (fwd)
               {
                 msg="End of "+msg+" reached. Continue from the beginning?";
               }else
               {
                 msg="Beginning of "+msg+" reached. Continue from the end?";
               }
	       if (QMessageBox::information(this, "DjVu", QStringFromGString(msg), "&Yes", "&No")==0)
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
           GUTF8String mesg="Page "+GUTF8String(page_num+1);
	   status_label->setText(QStringFromGString(mesg));
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
	       showMessage(this, "DjVu Search Failed",
			   "After looking through every page of this document\n"
			   "we have found, that neither of them contain\n"
			   "textual information. This means, that the document\n"
			   "creator did not run an OCR engine on this document.\n\n"
			   "The search is impossible.", true, false, true);
	    else
	       showMessage(this, "DjVu Search Failed",
			   "This page does not contain textual information,\n"
			   "which means that either creator of this document "
			   "did not run an OCR engine on it, or the OCR engine "
			   "did not recognize any text on this page.\n\n"
			   "The search is impossible.", true, false, true);
	 } else showInfo(this, "DjVu", "Search string not found");
      }

      in_search=false;
      search_butt->setText("&Find");
      {
        GUTF8String mesg="Page "+GUTF8String(page_num+1);
        status_label->setText(QStringFromGString(mesg));
      }
      text->setEnabled(TRUE);
      clear_butt->setEnabled(TRUE);
   } catch(const GException & exc)
   {
      status_label->hide();
      in_search=false;
      search_butt->setText("&Find");
      {
        GUTF8String mesg="Page "+GUTF8String(page_num+1);
        status_label->setText(QStringFromGString(mesg));
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
   
   GUTF8String mesg="Page "+GUTF8String(page_num+1);
   status_label->setText(QStringFromGString(mesg));
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
   
   setCaption("DjVu: Find");
   setResizable(true, false);
   
   QWidget * start=startWidget();
   QeLabel * label;
   
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);

   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   label=new QeLabel("Find: ", start);
   hlay->addWidget(label);
   text=new QeLineEdit(start, "search_text");
   label->setBuddy(text);
   hlay->addWidget(text, 1);

   hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);

   all_pages_butt=new QeCheckBox("Search &all pages", start, "all_pages_butt");
   if (doc->get_pages_num()>1)
      all_pages_butt->setChecked(all_pages);
   else
   {
      all_pages_butt->setChecked(TRUE);
      all_pages_butt->setEnabled(FALSE);
   }
   hlay->addWidget(all_pages_butt);
   hlay->addStretch(1);
   
   case_butt=new QeCheckBox("&Case sensitive", start, "case_butt");
   case_butt->setChecked(case_sensitive);
   hlay->addWidget(case_butt);
   hlay->addStretch(1);

   whole_word_butt=new QeCheckBox("&Whole word", start, "whole_word_butt");
   whole_word_butt->setChecked(whole_word);
   hlay->addWidget(whole_word_butt);
   hlay->addStretch(1);

   back_butt=new QeCheckBox("Search &backwards", start, "back_butt");
   back_butt->setChecked(search_backwards);
   hlay->addWidget(back_butt);
   
   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   status_label=new QLabel("Loading page WWWWWWW", start);
   status_label->setMinimumSize(status_label->sizeHint());
   {
     GUTF8String mesg="Page "+GUTF8String(page_num+1);
     status_label->setText(QStringFromGString(mesg));
   }
   butt_lay->addWidget(status_label);
   butt_lay->addStretch(1);
   search_butt=new QePushButton("&Find", start, "search_butt");
   search_butt->setEnabled(FALSE);
   butt_lay->addWidget(search_butt);
   clear_butt=new QePushButton("&Clear", start, "clear_butt");
   butt_lay->addWidget(clear_butt);
   QePushButton * close_butt=new QePushButton("C&lose", start, "close_butt");
   butt_lay->addWidget(close_butt);
   search_butt->setDefault(TRUE);

   vlay->activate();

   connect(search_butt, SIGNAL(clicked(void)), this, SLOT(slotSearch(void)));
   connect(clear_butt, SIGNAL(clicked(void)), text, SLOT(clear(void)));
   connect(close_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));

   connect(text, SIGNAL(textChanged(const QString &)),
	   this, SLOT(slotTextChanged(const QString &)));
}
