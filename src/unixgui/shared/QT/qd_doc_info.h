//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_doc_info.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_QD_DOC_INFO
#define HDR_QD_DOC_INFO

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuDocument.h"
#include "qd_port.h"

#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qtabdialog.h>

#include "qt_fix.h"

class QDDocInfo : public QTabDialog
{
   Q_OBJECT
private:
   QePushButton		* page_goto_butt;
   QePushButton		* page_info_butt, * thumb_info_butt, * file_info_butt;
   QeLabel		* size_label;
   QListView		* page_list, * file_list, * thumb_list;

//   QTabDialog	* tab_dialog;
   
   TArray<void *>	page_items, file_items, thumb_items;

   GP<DjVuDocument>	doc;

   QDPort		port;

   void		preloadNextPage(void);
   void		preloadNextThumb(void);
   void		updatePage(const GP<DjVuFile> & file);
   void		updateThumb(const GP<DjVuFile> & file);
private slots:
   void		slotItemSelected(QListViewItem * item);
   void		slotItemDblClicked(QListViewItem * item);
   void		slotShowInfo(void);
   void		slotGotoPage(void);
   void		slotRightButtonPressed(QListViewItem *, const QPoint &, int);
      // Slots to receive translated requests from QDPort
   void		slotNotifyError(const GP<DjVuPort> &, const GUTF8String &);
   void		slotNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long);
signals:
   void		sigGotoPage(int page_num);
public:
   virtual void	show(void);

      // Note, that structure of DjVuDocument should already be known
   QDDocInfo(const GP<DjVuDocument> & doc, QWidget * parent=0,
	     const char * name=0, bool modal=FALSE);
   ~QDDocInfo(void) {}
};

#endif
