//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_editor_thumb.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include <qpopupmenu.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qcstring.h>

#include "qt_fix.h"
#include "qd_editor_thumb.h"
#include "qlib.h"
#include "DataPool.h"

#define IDC_ETH_INSERT_BEFORE	0
#define IDC_ETH_INSERT_AFTER	1
#define IDC_ETH_IMPORT_BEFORE	2
#define IDC_ETH_IMPORT_AFTER	3
#define IDC_ETH_REMOVE	        4
#define IDC_ETH_MOVE	        5
#define IDC_ETH_SAVE            6
#define IDC_ETH_CLOSE		7
#define IDC_ETH_COPY            8
#define IDC_ETH_CUT             9
#define IDC_ETH_PASTE_BEFORE    10
#define IDC_ETH_PASTE_AFTER     11

int
QDEditorThumbnails::getSelectedItemsNum(void) const
{
   int cnt=0;
   for(int i=0;i<(int) count();i++)
      if (isSelected(i))
	 cnt++;
   return cnt;
}

void
QDEditorThumbnails::processCommand(int cmd)
{
   if (cmd==IDC_ETH_CLOSE)
   {
      emit sigCloseThumbnails();
      return;
   }
   
   int page_num=currentItem();
   GList<int> page_list;
   for(int i=0;i<(int) count();i++)
      if (isSelected(i))
	 page_list.append(i);
   
   if (page_num>=0 && page_num<(int) count())
      switch(cmd)
      {
	 case IDC_ETH_INSERT_BEFORE:
	    emit sigInsert(page_num);
	    break;
	 
	 case IDC_ETH_INSERT_AFTER:
	    if (++page_num>=(int) count()) page_num=-1;
	    emit sigInsert(page_num);
	    break;
	 
	 case IDC_ETH_IMPORT_BEFORE:
	    emit sigImport(page_num);
	    break;
	 
	 case IDC_ETH_IMPORT_AFTER:
	    if (++page_num>=(int) count()) page_num=-1;
	    emit sigImport(page_num);
	    break;
	    
	 case IDC_ETH_REMOVE:
	    emit sigRemove(page_list);
	    break;
	 
	 case IDC_ETH_MOVE:
	    emit sigMove(page_list);
	    break;

	 case IDC_ETH_SAVE:
	    emit sigSaveAs(page_list);
	    break;
	    
	 case IDC_ETH_COPY:
	    emit sigCopy(page_list);
	    break;
	    
         case IDC_ETH_CUT:
	    emit sigCut(page_list);
	    break;
	    
	 case IDC_ETH_PASTE_BEFORE:
	    emit sigPasteBefore();
	    break;

         case IDC_ETH_PASTE_AFTER:
	    emit sigPasteAfter();
	    break;

         case IDC_ETH_CLOSE:
         default:
	    emit sigCloseThumbnails();
	    break;
      }
}

void
QDEditorThumbnails::slotPopup(int cmd)
{
   popup_menu_id=cmd;
}

void
QDEditorThumbnails::mouseDoubleClickEvent(QMouseEvent * ev)
{
   int clicked_item=findItem(ev->y());
   if (clicked_item>=0)
   {
      clearSelection();
      setSelected(clicked_item, TRUE);
   }

   QDThumbnails::mouseDoubleClickEvent(ev);
}

void
QDEditorThumbnails::mousePressEvent(QMouseEvent * ev)
{
   try
   {
	 // Let the QListBox select the item under the mouse pointer

      int clicked_item=findItem(ev->y());

      if (ev->button()==LeftButton)
      {
	 if (ev->state() & ControlButton)
	 {
	       // Don't do anything here. QListBox will work for us.
	 } else if (ev->state() & ShiftButton)
	 {
	       // Here we have some code that selects all items
	       // between last_clicked_item and clicked_item.
	       // Unfortunately, QListBox doesn't support this mode.
	    if (clicked_item>=0)
	    {
	       if (last_clicked_item<=0 ||
		   !isSelected(last_clicked_item))
	       {
		  for(int i=0;i<(int) count();i++)
		     if (isSelected(i))
		     {
			last_clicked_item=i;
			break;
		     }
	       }
	       if (last_clicked_item>=0)
	       {
		  int i;
		  if (clicked_item>last_clicked_item)
		     for(i=last_clicked_item;i<clicked_item;i++)
			setSelected(i, TRUE);
		  else
		     for(i=clicked_item;i<last_clicked_item;i++)
			setSelected(i, TRUE);

		     // Deselect the just clicked item
		     // It will be immediately reselected by
		     // QListBox::mousePressEvent()
		  setSelected(clicked_item, FALSE);
	       }
	    }
	 } else
	 {
	       // W/o any modifiers we want all selected items to be
	       // deselected, and the clicked item selected.
	       // This is "standard" behavior, which is unfortunately
	       // different from one implemented by QListBox.
	    clearSelection();
	 }
	 
	 QDThumbnails::mousePressEvent(ev);
      }
      
	 // And see if we need to display the popup menu
      if (ev->button()==RightButton && clicked_item>=0)
      {
	 if (!isSelected(clicked_item))
	 {
	    clearSelection();
	    setSelected(clicked_item, TRUE);
	 }

	 int selected=0;
	 for(int i=0;i<(int) count();i++)
	    if (isSelected(i))
	       selected++;

	 popup_menu->setItemEnabled(IDC_ETH_INSERT_BEFORE, selected==1);
	 popup_menu->setItemEnabled(IDC_ETH_INSERT_AFTER, selected==1);
	 popup_menu->setItemEnabled(IDC_ETH_IMPORT_BEFORE, selected==1);
	 popup_menu->setItemEnabled(IDC_ETH_IMPORT_AFTER, selected==1);

	 if (selected==1)
	 {
	    QDClipBoardDjVuData *cb=dynamic_cast<QDClipBoardDjVuData *>(QApplication::clipboard()->data());
	    bool cb_is_empty=TRUE;
	    if ( cb )
	       cb_is_empty=cb->isEmpty();
	    popup_menu->setItemEnabled(IDC_ETH_PASTE_BEFORE, !cb_is_empty);
	    popup_menu->setItemEnabled(IDC_ETH_PASTE_AFTER, !cb_is_empty);
	 }
	 else
	 {
	    popup_menu->setItemEnabled(IDC_ETH_PASTE_BEFORE, FALSE);
	    popup_menu->setItemEnabled(IDC_ETH_PASTE_AFTER, FALSE);
	 }
	 
	 popup_menu_id=-1;
	 popup_menu->exec(QCursor::pos());
	 if (popup_menu_id>=0) processCommand(popup_menu_id);
      }

      if (clicked_item>=0)
	 last_clicked_item=clicked_item;
   } catch(const GException & exc)
   {
      showError(topLevelWidget(), exc);
   }
}

static void
clearCache(GMap<GUTF8String, void *> & cache)
{
   GPosition pos;
   while((pos=cache))
   {
      delete (TArray<char> *) cache[pos];
      cache.del(pos);
   }
}

void
QDEditorThumbnails::clearThumbCache(void)
{
   clearCache(thumb_cache);
}

void
QDEditorThumbnails::dataSet(int page_num, const TArray<char> & data)
{
   GUTF8String id=doc->page_to_id(page_num);
   GPosition pos;
   if (thumb_cache.contains(id, pos))
   {
      delete (TArray<char> *) thumb_cache[pos];
      thumb_cache.del(pos);
   }
   thumb_cache[id]=new TArray<char>(data);
}

void
QDEditorThumbnails::setCurPageNum(int cur_page_num)
{
   if (!!doc && cur_page_num>=0)
     cur_page_id=doc->page_to_id(cur_page_num);
   QDThumbnails::setCurPageNum(cur_page_num);
}

void
QDEditorThumbnails::rescan(bool soft)
{
   if (soft && doc->get_pages_num()>0)
   {
      GMap<GUTF8String, void *> tmp_cache=thumb_cache;
      thumb_cache.empty();

      bool visible=isVisible();
      try
      {
	    // We need to hide the widget 'cause we will be switching the
	    // current page, which will change the visible items,
	    // which will be considered as flickering by most.
	 if (visible) hide();
#ifdef  QT1
	 int top_page_num=topCell();	// Remember what was at the top
#else
         int top_page_num=topItem();
#endif
	 GUTF8String top_page_id;
	 if (top_page_num<doc->get_pages_num())
	    top_page_id=doc->page_to_id(top_page_num);
	 QDThumbnails::rescan();
	 if (cur_page_id.length())	// Set the current page
	 {
	    GP<DjVmDir::File> f=doc->get_djvm_dir()->id_to_file(cur_page_id);
	    if (f)
              setCurPageNum(f->get_page_num());
	 }
	 if (top_page_id.length())	// Set page at the top
	 {
	    GP<DjVmDir::File> f=doc->get_djvm_dir()->id_to_file(top_page_id);
#ifdef QT1
	    setTopCell(f->get_page_num());
#else
	    setTopItem(f->get_page_num());
#endif
	 }

	    // Now return the cached data back to QDThumbnails
	 int pages_num=doc->get_pages_num();
	 for(int page_num=0;page_num<pages_num;page_num++)
	 {
	    GUTF8String id=doc->page_to_id(page_num);
	    GPosition pos;
	    if (tmp_cache.contains(id, pos))
	       setData(page_num, *(TArray<char> *) tmp_cache[pos]);
	 }
	 clearCache(tmp_cache);

	 if (visible)
	 {
	    show();
	    getDataForNextPage();	// QDThumbnails does not do it when not visible
	 }
      } catch(...)
      {
	 clearCache(tmp_cache);
	 if (visible)
         {
           show();
         }
	 throw;
      }
   } else
   {
      clearThumbCache();
      QDThumbnails::rescan();
   }
}

void
QDEditorThumbnails::setDjVuDocument(GP<DjVuDocEditor> & doc_in)
{
   doc=doc_in;
   //GP<DjVuDocument> xdoc=doc;
   QDThumbnails::setDjVuDocument((GP<DjVuDocument> &)doc);
}

QDEditorThumbnails::QDEditorThumbnails(QWidget * parent, bool _canCompress,
				       const char * name, bool _rowMajor) :
   QDThumbnails(parent, name, _rowMajor), doc(0), canCompress(_canCompress)
{

   popup_menu=new QPopupMenu(0, "qd_shop_thumb_menu");

   connect(popup_menu, SIGNAL(activated(int)), this, SLOT(slotPopup(int)));
   popup_menu->insertItem("&Insert pages before", IDC_ETH_INSERT_BEFORE);
   popup_menu->insertItem("I&nsert pages after", IDC_ETH_INSERT_AFTER);
   if (canCompress)
   {
      popup_menu->insertItem("Import pages b&efore", IDC_ETH_IMPORT_BEFORE);
      popup_menu->insertItem("Import pages a&fter", IDC_ETH_IMPORT_AFTER);
   }
   popup_menu->insertSeparator();
   popup_menu->insertItem("&Copy", IDC_ETH_COPY);
   popup_menu->insertItem("Cu&t", IDC_ETH_CUT);
   popup_menu->insertItem("&Paste before", IDC_ETH_PASTE_BEFORE);
   popup_menu->insertItem("P&aste after", IDC_ETH_PASTE_AFTER);

   popup_menu->insertSeparator();
   popup_menu->insertItem("&Move page(s)", IDC_ETH_MOVE);
   popup_menu->insertItem("&Delete page(s)", IDC_ETH_REMOVE);

   popup_menu->insertSeparator();
   popup_menu->insertItem("&Save page(s)", IDC_ETH_SAVE);
   
   popup_menu->insertSeparator();
   popup_menu->insertItem("C&lose thumbnails", IDC_ETH_CLOSE);
   
   last_clicked_item=-1;
   setMultiSelection(TRUE);
}


const char*
QDClipBoardDjVuData::format( int i ) const
{
   DEBUG_MSG("QDClipBoardDjVuData::format()\n");
   
    if ( i == 0 )
       return "image/djvu";
    else
       return 0;
}

QByteArray
QDClipBoardDjVuData::encodedData( const char* mime ) const
{
   DEBUG_MSG("QDClipBoardDjVuData::encodedData()\n");

   QByteArray a;

   G_TRY {
   
      if ( QString( mime ) == "image/djvu" )
      {
      
	 GList<GUTF8String> names;
	 GPList<ByteStream> streams;
	 long needmemory = sizeof(short); // for the short number of files
      
	 for(GPosition pos=page_list;pos;++pos)
	 {
	    int page=page_list[pos];
      
	    GP<DataPool> data;
	    GUTF8String filename;
	    if( !editor || !editor->getFileDataFileName(page, data, filename) )
	       G_THROW("Can not get data for Page "+page);
							
	    names.append(filename);
	    GP<ByteStream> stream = data->get_stream();
	    GP<ByteStream> memstream = ByteStream::create();
	    memstream->copy(*stream);
	    streams.append(memstream);
	    needmemory += filename.length()+1; // 1 for zero at the end
	    memstream->seek(0, SEEK_END);

	    needmemory += sizeof(long); // for stream size
	    needmemory += memstream->tell();
	    memstream->seek(0);
	 }
      
	 a.resize(needmemory);

	 char *cdata=a.data();
      
	 //copy everything in here
	 short pages = names.size();
	 memcpy(cdata, &pages, sizeof(pages) );
	 cdata += sizeof(pages);

	 for( GPosition namepos=names, streampos=streams; namepos; ++namepos, ++streampos)
	 {
	    // copy the name here
	    strcpy(cdata, names[namepos]);
	    cdata += names[namepos].length()+1;
	    // copy the data size here
	    GP<ByteStream> stream = streams[streampos];
	    long size=0;
	    stream->seek(0, SEEK_END);
	    size = stream->tell();
	    memcpy(cdata, &size, sizeof(size));
	    cdata += sizeof(size);

	    // copy data here
	    stream->seek(0);
	    stream->readall(cdata, size);
	    cdata += size;
	 }
      
      }
   } G_CATCH(exc) {
      a.resize(0);
   }
   G_ENDCATCH;

   return a;

}

// END OF FILE


