//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_search_dialog.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_SEARCH_DIALOG
#define HDR_QD_SEARCH_DIALOG

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuDocument.h"
#include "DjVuText.h"

#include <qdialog.h>

#include "qt_fix.h"

class QDSearchDialog : public QeDialog
{
   Q_OBJECT
private:
   static bool	all_pages;
   static bool	case_sensitive;
   static bool	search_backwards;
   static bool	whole_word;

   class QeLineEdit	* text;
   class QeCheckBox	* all_pages_butt;
   class QeCheckBox	* case_butt;
   class QeCheckBox	* back_butt;
   class QeCheckBox	* whole_word_butt;
   class QePushButton	* search_butt;
   class QePushButton	* clear_butt;
   class QLabel		* status_label;

   bool			in_search;
   bool			stop;
   
   int			page_pos;
   int			page_num;
   int			seen_page_num;
   GP<DjVuDocument>	doc;
private slots:
   void		slotSearch(void);
   void		slotTextChanged(const QString &);
protected:
   virtual void	done(int);
signals:
   void		sigDisplaySearchResults(int page_num, const GList<DjVuTXT::Zone *> & zone_list);
public slots:
   void		slotSetPageNum(int page_num);
public:
   QDSearchDialog(int page_num, const GP<DjVuDocument> & doc, QWidget * parent=0,
		  const char * name=0, bool modal=FALSE);
   ~QDSearchDialog(void) {};
};

#endif
