//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_thumb.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_THUMB
#define HDR_QD_THUMB

#ifdef __GNUC__
#pragma interface
#endif

#include "qd_port.h"
#include "GContainer.h"

#include <qlistbox.h>

#include "qt_fix.h"

class QDThumbnails : public QListBox
{
   Q_OBJECT
   friend class QDThumbItem;
private:
   class Pending : public GPEnabled
   {
   public:
      int		page;
      GP<DataPool>	pool;
      Pending(int _page, const GP<DataPool> & _pool) :
	    page(_page), pool(_pool) {}
   };
   GP<DjVuDocument>	doc;
   QDPort		port;
   QDMessenger		messenger;
   class QPopupMenu	* popup_menu;
   int			popup_menu_id;

   bool			need_rescan;

   GArray<QPixmap>	pixmaps_arr;
   GList<int>		pixmaps_list;

   GPList<Pending>	pending_list;

   QCursor	normalCursor;

   bool		fast_mode;
   int		cur_page_num;

   bool         rowMajor;
   int          min_list_width;
   int          max_list_width;
   
   QPixmap	getImage(int page_num, int width, int height);
   void		processCommand(int cmd);

   static void	trigger_cb(void * cl_data);
private slots:
   void		slotPopup(int);
   void		slotNotifyFileFlagsChanged(const GP<DjVuFile> & source,
					   long set_mask, long clr_mask);
   void		slotNotifyDocFlagsChanged(const GP<DjVuDocument> & source,
					  long set_mask, long clr_mask);
   void		slotTriggerCB(int);
   void         slotGotoPage(QListBoxItem *);
protected:
   virtual void	resizeEvent(QResizeEvent * ev);
   virtual void	mousePressEvent(QMouseEvent * ev);
   virtual void	dataSet(int page_num, const TArray<char> & data) {}
   void		setData(int page_num, const TArray<char> & data);
   bool		getDataForPage(int page_num);
   void		getDataForNextPage(void);
signals:
   void		sigGotoPage(int page_num);
   void		sigCloseThumbnails(void);
public slots:
   void		slotReloadPage(int page_num) { reloadPage(page_num); }
public:
   virtual QSize sizeHint(void) const;
   virtual void	show(void);
   
   void		rescan(void);
      // Will clear data for page 'page_num' and request it from DjVuDocument
      // again. fast_mode will be used to determine if DjVuDocument should
      // decode the file or just return predecoded data (if any)
   void		reloadPage(int page_num);

   void		rereadGamma(void);
   void		setFastMode(bool en);
   void		setDjVuDocument(GP<DjVuDocument> & doc);
   virtual void	setCurPageNum(int cur_page_num);

   QDThumbnails(QWidget * parent=0, const char * name=0, bool _rowMajor=FALSE);
   ~QDThumbnails(void) {}
};

#endif
