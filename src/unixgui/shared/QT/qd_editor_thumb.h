//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_editor_thumb.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_EDITOR_THUMB
#define HDR_QD_EDITOR_THUMB

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuDocEditor.h"
#include "qd_thumb.h"
#include "qd_editor.h"

#include <qdragobject.h>
#include <qmime.h>
#include <qcstring.h>

#include "qt_fix.h"

class QDEditorThumbnails : public QDThumbnails
{
   Q_OBJECT
private:
   GP<DjVuDocEditor>	doc;
   GUTF8String		cur_page_id;
   GMap<GUTF8String, void *>thumb_cache;
   class QPopupMenu	* popup_menu;
   int			popup_menu_id;
   int			last_clicked_item;
   bool         canCompress;
   
   void		clearThumbCache(void);
   void		processCommand(int cmd);
private slots:
   void		slotPopup(int);
protected:
   virtual void	dataSet(int page_num, const TArray<char> & data);
   virtual void	mousePressEvent(QMouseEvent * ev);
   virtual void	mouseDoubleClickEvent(QMouseEvent * ev);
signals:
   void		sigInsert(int page_num);
   void		sigImport(int page_num);
   void		sigRemove(const GList<int> & page_list);
   void		sigMove(const GList<int> & page_list);
   void		sigSaveAs(const GList<int> & page_list);
   void		sigCopy(const GList<int> & page_list);
   void		sigCut(const GList<int> & page_list);
   void		sigPasteBefore(void);
   void		sigPasteAfter(void);
   void		sigCloseThumbnails(void);
public:   
   void		rescan(bool soft=false);

   int		getSelectedItemsNum(void) const;
   
   void		setDjVuDocument( GP<DjVuDocEditor> & doc );
   virtual void	setCurPageNum(int cur_page_num);
   
   QDEditorThumbnails(QWidget * parent=0, bool canCompress=FALSE,
		      const char * name=0, bool _rowMajor=FALSE);
   ~QDEditorThumbnails(void) {}
};



class QDClipBoardDjVuData : public QMimeSource
{

public:
   QDClipBoardDjVuData(QDEditor *_editor, const GP<DjVuDocEditor> & _djvu_doc, const GList<int> &_page_list) :
      editor(_editor), djvu_doc(_djvu_doc), page_list(_page_list) { _data=encodedData("image/djvu"); }

   const char* format( int i=0 ) const;
   QByteArray encodedData( const char* mime ) const;
   QByteArray getEncodedData(void) { return _data; }
   bool isEmpty() { return _data.isEmpty(); }

private:
   QDEditor *editor;
   GP<DjVuDocEditor> djvu_doc;
   GList<int> page_list;
   QByteArray _data;
};


#endif
