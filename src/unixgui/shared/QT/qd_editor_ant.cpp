//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_editor_ant.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_editor_ant.h"

#include "djvu_editor_res.h"
#include "GContainer.h"
#include "exc_msg.h"
#include "debug.h"
#include "qlib.h"

#include <qmessagebox.h>
#include <qprogressdialog.h>

#include "qt_fix.h"

static void
progress_cb(float done, void * cl_data)
{
   QProgressDialog * d=(QProgressDialog *) cl_data;
   if (done<0) done=0;
   else if (done>1) done=1;
   d->setProgress((int) (done*100));
}

void
QDEditor::decodeAnno(bool allow_redraw)
      // Will test that annotations are editable (are in the correct
      // locations), will decode shared annotations, and will
      // call the QDBase::decodeAnno() to finish up
{
   DEBUG_MSG("QDEditor::decodeAnno(): decoding...\n");
   DEBUG_MAKE_INDENT(3);

   if (!dimg)
   {
      DEBUG_MSG("dimg=NULL, no decoding performed\n");
      return;
   }
   
   GList<GURL> ignore_list;

      // Get the shared_anno and shared_anno_file
   // BUGGY: check out DjVuImage::get_decoded_anno() 
   shared_anno=0;
   shared_anno_file=djvu_doc->get_shared_anno_file();
   if (shared_anno_file)
   {
      ignore_list.append(shared_anno_file->get_url());
      GP<ByteStream> str=shared_anno_file->get_merged_anno();
      shared_anno=DjVuAnno::create();
      if (str)
	 shared_anno->decode(str);
      if (!shared_anno->ant)
	 shared_anno->ant=DjVuANT::create();
   }

   // extract text
   page_text = DjVuText::create();
   GP<ByteStream> txtstr = dimg->get_djvu_file()->get_text();
   if( txtstr )
      page_text->decode(txtstr);
   
      // See if we have annotations anywhere else
   int max_level=0;
   GP<ByteStream> str;
   str=dimg->get_djvu_file()->get_merged_anno(ignore_list, &max_level);
   page_anno=DjVuAnno::create();
   page_anno->ant=DjVuANT::create();
   if (str)
      page_anno->decode(str);
   if (max_level>0)
   {
      if (QMessageBox::information(this, "Unsupported DjVu document",
		"We have just discovered that this DjVu multipage document\n"
		"has annotations in non-standard locations.\n\n"
		"This editor allows annotations to be either in the top-level\n"
		"page files or in one file included by all the pages.\n\n"
		"If we do not fix it right now (by moving illegal annotations\n"
		"to the top-level page files), the editor behavior will\n"
		"be undefined. For example, you will not be able to remove\n"
		"certain annotations.\n\n"
		"Do you want to fix the problem right now?\n",
		"&Yes", "&No", 0, 1)==0)
      {
	 QProgressDialog d(this);
	 d.setLabelText("Fixing annotations... Please wait...");
	 d.setCaption("Fixing annotations...");
	 d.setTotalSteps(100);
	 d.setProgress(0);
	 d.show();
	 djvu_doc->simplify_anno(progress_cb, &d);
	 d.setProgress(100);
      }
   }
   
   anno=DjVuAnno::create();
   if ( shared_anno )
      anno->merge(shared_anno);
   if ( page_anno ) 
      anno->merge(page_anno);
   
   processAnno(allow_redraw);
}

void
QDEditor::copyAnnoBack(bool mark_doc_as_modified)
{

   DEBUG_MSG("QDEditor::copyAnnoBack() called\n");
   DEBUG_MAKE_INDENT(3);
   
      // First merge two annotations into one understood by QDBase
   anno=DjVuAnno::create();
   if (shared_anno)
      anno->merge(shared_anno);
   if (page_anno)
      anno->merge(page_anno);

      // Now encode the annotations back
   
      // To shared_anno_file first
   if (shared_anno_file)
   {
      GP<ByteStream> str = ByteStream::create();
      shared_anno->encode(str);
      str->seek(0);
      shared_anno_file->anno=str;
      shared_anno_file->set_modified(true);
   }

      // And to the page's top-level file last
   if (page_anno)
   {
      GP<ByteStream> str = ByteStream::create();
      page_anno->encode(str);
      str->seek(0);
      dimg->get_djvu_file()->anno=str;
      dimg->get_djvu_file()->set_modified(true);
   }

   if(page_text && page_text->txt)
      dimg->get_djvu_file()->change_text(page_text->txt, false);
   
   if (mark_doc_as_modified)
      setDocModified(true);

}

void
QDEditor::createSharedAnno(void)
{
   QProgressDialog d(this);
   d.setLabelText("Creating shared annotations... Please wait...");
   d.setCaption("Creating shared annotations...");
   d.setTotalSteps(100);
   d.setProgress(0);
   djvu_doc->create_shared_anno_file(progress_cb, &d);
   d.setProgress(100);
	 
   if (!(shared_anno_file=djvu_doc->get_shared_anno_file()))
      G_THROW("Unable to create file with shared annotations.");
   shared_anno=DjVuAnno::create();
   shared_anno->ant=DjVuANT::create();
}

