//C-  -*- C++ -*-
//C-
//C-  Copyright � 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_page_saver.cpp,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_page_saver.h"
#include "GOS.h"
#include "ByteStream.h"
#include "DataPool.h"
#include "qlib.h"
#include "debug.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

#include "qt_fix.h"

//***************************************************************************
//*************************** QDPageNameDialog dialog ***********************
//***************************************************************************

class QDPageNameDialog : public QeDialog
{
   Q_OBJECT
private:
   QeLineEdit	* text;
protected:
   virtual void	done(int);
protected slots:
   void		slotBrowse(void);
public:
   GUTF8String	pageName(void) const { return GStringFromQString(text->text()); }
   QDPageNameDialog(const char * message, const char * page_name,
		    QWidget * parent=0, const char * name=0);
   ~QDPageNameDialog(void) {};
};

void
QDPageNameDialog::slotBrowse(void)
{
   GUTF8String page_full_name=GStringFromQString(text->text());
   QString page_dir=QFileInfo(QStringFromGString(page_full_name)).dirPath();
   if (!QFileInfo(page_dir).isDir()) page_dir=QeFileDialog::lastSaveDir;
   if (!QFileInfo(page_dir).isDir()) page_dir=QDir::currentDirPath();
   page_full_name=GURL::expand_name(GOS::basename(page_full_name), page_dir);
   
   static char * filters[]={ "*.djvu", "*.djv", "All files (*)", 0 };
   QeFileDialog fd(page_dir, filters[0], this, "djvu_fd", TRUE);
   fd.setFilters((const char **) filters);
   fd.setCaption(tr("Select DjVu page file name..."));
   fd.setSelection(QStringFromGString(page_full_name));
      
   if (fd.exec()==QDialog::Accepted) text->setText(fd.selectedFile());
}

void
QDPageNameDialog::done(int rc)
{
   if (rc==Accepted)
   {
      try
      {
	    // Try to create the named file.
	 GUTF8String fname=GStringFromQString(text->text());
	 GP<ByteStream> gstr_in=ByteStream::create(GURL::Filename::UTF8(fname),"a");
      } catch(const GException & exc)
      {
         QString qmesg(exc.get_cause());
	 showError(this, "DjVu Error", qmesg);
	 return;
      }
   }
   QeDialog::done(rc);
}

QDPageNameDialog::QDPageNameDialog(const char * message, const char * page_name,
				   QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   QWidget * start=startWidget();

   setCaption(tr("Page file name"));
   setResizable(true, false);

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 5, "vlay");

   QeLabel * label=new QeLabel(message, start, "message_label");
   vlay->addWidget(label);
   label->setMaximumHeight(label->sizeHint().height());

   QHBoxLayout * hlay=new QHBoxLayout();
   vlay->addLayout(hlay);

   label=new QeLabel(tr("File name: "), start, "fname_label");
   hlay->addWidget(label);
   label->setMaximumHeight(label->sizeHint().height());

   text=new QeLineEdit(start, "text");
   text->setText(page_name);
   hlay->addWidget(text, 1);

   QePushButton * browse_butt=new QePushButton(tr("&Browse"), start, "browse_butt");
   hlay->addWidget(browse_butt);
   browse_butt->setMaximumHeight(browse_butt->sizeHint().height());
   
   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QePushButton * ok_butt=new QePushButton(tr("&OK"), start, "ok_butt");
   butt_lay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton(tr("&Cancel"), start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);
   ok_butt->setDefault(TRUE);
   ok_butt->setMaximumHeight(ok_butt->sizeHint().height());
   cancel_butt->setMaximumHeight(cancel_butt->sizeHint().height());
   
   vlay->activate();

      // Connecting signals and slots
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
   connect(browse_butt, SIGNAL(clicked(void)), this, SLOT(slotBrowse(void)));
}

//***************************************************************************
//*************************** QDFileFormatDialog dialog *********************
//***************************************************************************

class QDFileFormatDialog : public QeDialog
{
   Q_OBJECT
private:
   QeRadioButton	* separate_butt;
   QeRadioButton	* bundled_butt;
   QeRadioButton	* merged_butt;
public:
   enum { BUNDLED, SEPARATE, MERGED };

   int		format(void) const;
   
   QDFileFormatDialog(QWidget * parent=0, const char * name=0);
   ~QDFileFormatDialog(void) {}
};

int
QDFileFormatDialog::format(void) const
{
   return
      separate_butt->isChecked() ? SEPARATE :
      bundled_butt->isChecked() ? BUNDLED : MERGED;
}

QDFileFormatDialog::QDFileFormatDialog(QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   QWidget * start=startWidget();
   
   setCaption(tr("DjVu page file format"));

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 5, "vlay");

   QeLabel * label=new QeLabel(tr("It seems that contents of this page are currently ")+
			       tr("scattered over more than one file. ")+
			       tr("So you can now do either of the following:"), start);
   label->setMinimumWidth(300);
   label->setAlignment(WordBreak);
   vlay->addWidget(label);
   
   vlay->addSpacing(10);

   QGridLayout * glay=new QGridLayout(3, 2, 10);
   vlay->addLayout(glay);
   glay->setColStretch(1, 1);
   
   separate_butt=new QeRadioButton("", start, "separate_butt");
   separate_butt->setChecked(FALSE);
   glay->addWidget(separate_butt, 0, 0);
   label=new QeLabel(tr("Create all these files. This is useful if you plan ")+
		     tr("to save more than one page and then to insert them ")+
		     tr("into another document: the shared files will remain shared."),
		     start, "separate_label");
   label->setAlignment(WordBreak);
   glay->addWidget(label, 0, 1);

   bundled_butt=new QeRadioButton("", start, "bundled_butt");
   bundled_butt->setChecked(TRUE);
   glay->addWidget(bundled_butt, 1, 0);
   label=new QeLabel(tr("Pack all these files into one bundle ")+
		     tr("(so called BUNDLED format). This is convenient because ")+
		     tr("you will have only one file and will still be able ")+
		     tr("to split it into many when necessary."), start, "bundled_label");
   label->setAlignment(WordBreak);
   glay->addWidget(label, 1, 1);

   merged_butt=new QeRadioButton("", start, "merged_butt");
   merged_butt->setChecked(FALSE);
   glay->addWidget(merged_butt, 2, 0);
   label=new QeLabel(tr("Merge chunks from all files and store them ")+
		     tr("into one file. ")+
		     tr("Use this if you need the simplest structure of the file ")+
		     tr("and you do not plan to separate the chunks again."),
		     start, "merged_label");
   label->setAlignment(WordBreak);
   glay->addWidget(label, 2, 1);

   QButtonGroup * grp=new QButtonGroup(start, "buttongroup");
   grp->hide();
   grp->insert(separate_butt);
   grp->insert(bundled_butt);
   grp->insert(merged_butt);

   vlay->addSpacing(15);
   
   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QePushButton * ok_butt=new QePushButton(tr("&OK"), start, "ok_butt");
   butt_lay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton(tr("&Cancel"), start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);
   ok_butt->setDefault(TRUE);
   
   vlay->activate();

      // Connecting signals and slots
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}

//***************************************************************************
//*************************** QDFilesListDialog dialog **********************
//***************************************************************************

class QDFilesListDialog : public QeDialog
{
   Q_OBJECT
private:
public:
   QDFilesListDialog(const GP<DjVmDoc> & doc, const QString & dir,
		     QWidget * parent=0, const char * name=0);
   ~QDFilesListDialog(void) {}
};

QDFilesListDialog::QDFilesListDialog(const GP<DjVmDoc> & doc, const QString & dir,
				     QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   QWidget * start=startWidget();
   
   setCaption(tr("DjVu: Files list"));

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 5, "vlay");

   QString msg=tr("The following files will be created in directory \"")+
	       dir+"\":\n\n";
   GP<DjVmDir> dir=doc->get_djvm_dir();
   GPList<DjVmDir::File> files_list=dir->get_files_list();
   for(GPosition pos=files_list;pos;++pos)
      msg+="\""+QStringFromGString(files_list[pos]->get_save_name())+"\", ";
   //msg.setat(msg.length()-2, 0);
   msg[msg.length()-1]=0;
   msg+=tr(".\n\nAre you sure you want to proceed?\n");
   
   QeLabel * label=new QeLabel(msg, start);
   label->setMinimumWidth(300);
   label->setAlignment(WordBreak);
   vlay->addWidget(label);
   
   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QePushButton * ok_butt=new QePushButton(tr("&Yes"), start, "ok_butt");
   butt_lay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton(tr("&No"), start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);
   ok_butt->setDefault(TRUE);
   
   vlay->activate();

      // Connecting signals and slots
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}

//****************************************************************************
//******************************* QDPageSaver ********************************
//****************************************************************************

static bool
addToDjVm(const GP<DjVmDoc> & doc, const GP<DjVuFile> & file,
	  bool page, GMap<GURL, void *> & map)
      // Will return TRUE if the file has been added to DjVmDoc
{
   GURL url=file->get_url();

   if (!map.contains(url))
   {
      map[url]=0;

      if (!file->contains_chunk("NDIR"))
      {
	 GP<DataPool> data=file->get_djvu_data(false, true);
      
	 GPList<DjVuFile> files_list=file->get_included_files(false);
	 for(GPosition pos=files_list;pos;++pos)
	 {
	    GP<DjVuFile> f=files_list[pos];
	    if (!addToDjVm(doc, f, false, map))
	       data=DjVuFile::unlink_file(data, f->get_url().fname());
	 }

	 GUTF8String name=url.fname();
	 GP<DjVmDir::File> frec=DjVmDir::File::create(name, name, name,
						      page ? DjVmDir::File::PAGE :
						      DjVmDir::File::INCLUDE);
	 doc->insert_file(frec, data, -1);
	 return true;
      }
   }
   return false;
}

GP<DjVmDoc>
QDPageSaver::getDjVmDoc(void)
      // Will create a DJVM BUNDLED document with the page contents, but
      // without NDIR chunks
{
   GMap<GURL, void *> map;
   GP<DjVmDoc> doc=DjVmDoc::create();
   addToDjVm(doc, djvu_file, true, map);
   return doc;
}

static QString
getDir(const char * name)
{
   QString dir=name;
   while(!QFileInfo(dir).isDir()) dir=QFileInfo(dir).dirPath();
   return dir;
}

void
QDPageSaver::saveSeparate(void)
{
   GURL file_url=djvu_file->get_url();

   QString save_dir=QeFileDialog::lastSaveDir;
   if (!QFileInfo(save_dir).isDir())
      if (file_url.is_local_file_url())
	 save_dir=getDir(file_url.UTF8Filename());
   if (!QFileInfo(save_dir).isDir()) save_dir=QDir::currentDirPath();
   
   QeFileDialog fd(save_dir, "*", parent, "djvu_fd", TRUE);
   fd.setCaption(QeFileDialog::tr("Select directory for page files..."));
   fd.setMode(QeFileDialog::Directory);
     
   if (fd.exec()==QDialog::Accepted)
   {
      QeFileDialog::lastSaveDir=save_dir;
      GP<DjVmDoc> doc=getDjVmDoc();
      QDFilesListDialog dlg(doc, fd.selectedFile(), parent);
      if (dlg.exec()==QDialog::Accepted)
	 doc->expand(GURL::Filename::UTF8(GStringFromQString(fd.selectedFile())), 0);
   }
}

void
QDPageSaver::saveBundled(void)
{
   static char * filters[]={ "*.djvu", "*.djv", "All files (*)", 0 };
 
   GURL file_url=djvu_file->get_url();

   QString save_dir=QeFileDialog::lastSaveDir;
   if (!QFileInfo(save_dir).isDir())
      if (file_url.is_local_file_url())
	 save_dir=getDir(file_url.UTF8Filename());
   if (!QFileInfo(save_dir).isDir()) save_dir=QDir::currentDirPath();
   
   QeFileDialog fd(save_dir, filters[0], parent, "djvu_fd", TRUE);
   fd.setFilters((const char **) filters);
   fd.setCaption(QeFileDialog::tr("Select DjVu page output file name..."));
   {
     GUTF8String gfname=GURL::expand_name(file_url.fname(),save_dir);
     fd.setSelection(QStringFromGString(gfname));
   }
      
   if (fd.exec()==QDialog::Accepted)
   {
      QeFileDialog::lastSaveDir=save_dir;
      GP<DjVmDoc> doc=getDjVmDoc();
      GURL selected=GURL::Filename::UTF8(GStringFromQString(fd.selectedFile()));
      DataPool::load_file(selected);
      GP<ByteStream> str_out=ByteStream::create(selected, "wb");
      doc->write(str_out);
   }
}

void
QDPageSaver::saveMerged(void)
{
   static char * filters[]={ "*.djvu", "*.djv", "All files (*)", 0 };
 
   GURL file_url=djvu_file->get_url();

   QString save_dir=QeFileDialog::lastSaveDir;
   if (!QFileInfo(save_dir).isDir())
      if (file_url.is_local_file_url())
	 save_dir=getDir(file_url.UTF8Filename());
   if (!QFileInfo(save_dir).isDir()) save_dir=QDir::currentDirPath();
   
   QeFileDialog fd(save_dir, filters[0], parent, "djvu_fd", TRUE);
   fd.setFilters((const char **) filters);
   fd.setCaption(QeFileDialog::tr("Select DjVu page output file name..."));
   {
     GUTF8String gfname=GURL::expand_name(file_url.fname(),save_dir);
     fd.setSelection(QStringFromGString(gfname));
   }
      
   if (fd.exec()==QDialog::Accepted)
   {
      QeFileDialog::lastSaveDir=save_dir;
      GP<DataPool> data=djvu_file->get_djvu_data(true, true);
      GP<ByteStream> str_in=data->get_stream();

      GURL selected=GURL::Filename::UTF8(GStringFromQString(fd.selectedFile()));
      DataPool::load_file(selected);
      GP<ByteStream> gstr_out=ByteStream::create(selected,"wb");
      ByteStream &str_out=*gstr_out;
      static char sample[]="AT&T";
      char buffer[5];
      data->get_data(buffer, 0, 4);
      if (memcmp(buffer, sample, 4))
	 str_out.writall("AT&T", 4);
      str_out.copy(*str_in);
   }
}

static int
getFilesNum(const GP<DjVuFile> & file, GMap<GURL, void *> & map)
      // Will return the number of files (starting from 'file' and
      // including 'file'), which do not contain NDIR chunk.
{
   int cnt=0;
   
   GURL url=file->get_url();

   if (!map.contains(url))
   {
      map[url]=0;

      if (!file->contains_chunk("NDIR"))
      {
	 cnt++;
	 GPList<DjVuFile> files_list=file->get_included_files(false);
	 for(GPosition pos=files_list;pos;++pos)
	    cnt+=getFilesNum(files_list[pos], map);
      }
   }

   return cnt;
}

int
QDPageSaver::getFilesNum(void)
      // Will return the number of files w/o NDIR chunk. Basically, these
      // are the files, which need to be saved.
{
   GMap<GURL, void *> map;
   return ::getFilesNum(djvu_file, map);
}

void
QDPageSaver::save(void)
{
   DEBUG_MSG("QDPageSaver::save(): saving the stuff\n");
   DEBUG_MAKE_INDENT(3);

   if (!djvu_file->is_all_data_present())
      G_THROW("Cannot save page because not all data is available.");
   
   int format=QDFileFormatDialog::MERGED;

      // First see if we have a choice of formats...
   if (getFilesNum()>1)
   {
      QDFileFormatDialog dlg(parent);
      if (dlg.exec()!=QDialog::Accepted) return;
      format=dlg.format();
   }

      // Now do the actual saving...
   switch(format)
   {
      case QDFileFormatDialog::SEPARATE:
	 saveSeparate();
	 break;

      case QDFileFormatDialog::BUNDLED:
	 saveBundled();
	 break;

      case QDFileFormatDialog::MERGED:
	 saveMerged();
	 break;
   }
}

QDPageSaver::QDPageSaver(const GP<DjVuFile> & _djvu_file, QWidget * _parent) :
      djvu_file(_djvu_file), parent(_parent)
{
}

#include "qd_page_saver_moc.inc"
