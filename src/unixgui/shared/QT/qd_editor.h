//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_editor.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_EDITOR
#define HDR_QD_EDITOR

#ifdef __GNUC__
#pragma interface
#endif

#include "qd_base.h"
#include "qd_port.h"
#include "qd_toolbutt.h"
#include "DjVuDocEditor.h"

#include <qpopupmenu.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qlabel.h>
#include <qprogressbar.h>

#include "qt_fix.h"

// QDEditorMainWindow: top level shell which creates toolbar, menu system ...
// QDEditor: custom popup menu, handles all the editing including hyperlink
// editing Has no idea about threads: it uses QDLoader to load a DjVuPage
// if necessary. 

class QWorkspace;
class QDEditor;

class QDEditorMainWindow : public QMainWindow
{
   Q_OBJECT

   friend class QDEditor;
   
public:

   virtual bool	close(bool forceKill=FALSE);

   QDEditorMainWindow(GP<DjVuDocEditor> &doc, int page_num,
		      QWidget * parent=0, bool canCompress=FALSE, const char * name=0);
   QDEditorMainWindow(const char *fname, int page_num=0,
		      QWidget * parent=0, bool canCompress=FALSE, const char * name=0);
   ~QDEditorMainWindow(void);

private:

   bool         canCompress;
   
   // work space for MDIs
   QWorkspace *ws;
   GPList<QDEditor> editorList;
   bool         closing;
   
   QToolBar	* edit_toolbar;
   QDToolButton * open_tbutt, * oval_tbutt, * poly_tbutt;
   QDToolButton * print_tbutt, * rect_tbutt, * save_tbutt;
   QDToolButton * hlrect_tbutt;
   QDToolButton	* edit_tbutt, * preview_tbutt;

   QPopupMenu   * windows_pane;
      
   class QeNInOne	* wpaper_switch;
   QeLabel		* wpaper;
   QeMenuBar	* menu;
   QLabel	* status_bar;
   QProgressBar	* progress_bar;

   QToolBar     * view_toolbar;
   class QDTBarNavPiece	* nav_tbar;
   class QDTBarModePiece * mode_tbar;

   void         resetMenuAndEditingToolBar(void);
   void		processCommand(int cmd);
   void	        createShell(void);

   void		openDocument(const char *filters[]);
   void		compressDocument(void);
   
   void		contents(void);
   void		about(void);

   void         updateMainMenus(bool viewingMode);
   
   void		setMode(int cmd_mode, bool do_redraw=1,
			int mode_src=QDBase::MODE_DEFAULT);
   void		setZoom(int cmd_zoom, bool do_layout=1,
			int zoom_src=QDBase::ZOOM_DEFAULT);

protected:
   //virtual void	resizeEvent(QResizeEvent * ev);
   
private slots:
 
   void         slotToolButtonClicked(void);
   void		slotMenuCB(int id) { processCommand(id); };
   void		slotAboutToShowMenu(void);
   void         slotAboutToShowWindowsMenu(void);
   void         slotWindowsMenuActivated( int id );
   void         slotWindowActivated(QWidget *w);

      // Toolbar related slots
   void		slotSetZoom(int cmd_zoom);
   void		slotSetMode(int cmd_mode);
   void		slotGotoPage(int page_num);
   void		slotDoCmd(int cmd);
   
};



class QDEditor : public QDBase
{
   Q_OBJECT

   friend class QDEditorMainWindow;

public:

   int		allowToClose;
   virtual bool	close(bool forceKill=FALSE);
   
private:

   QPopupMenu	* popup_menu;
   int		popup_menu_id;

   static int   firstTime;

   QDEditorMainWindow *main_window;

   QWidget	* search_parent;

   class QDEditorThumbnails	* thumbnails;
   

   GP<DjVuDocEditor>	djvu_doc;
   bool                 open_ok;
   bool			doc_modified;
   bool                 close_again;

   GP<DjVuFile>		shared_anno_file;
   GP<DjVuAnno>		shared_anno;
   GP<DjVuAnno>		page_anno;
   GP<DjVuText>         page_text;

   QDPort		port;

      // Extended hlinks information
   enum EDITOR_MODE { PREVIEW=0, EDIT=1, CREATE=2 };
   EDITOR_MODE		cur_editor_mode;
   int			cur_marea_vic_code, cur_marea_vic_data;
   int			new_marea_type;
   GP<MapArea>		new_map_area;
   int			new_marea_points;
   GArray<int>		new_marea_x, new_marea_y;

   GPQCursor	cur_ptr;

   bool		open_after_save, close_after_save;
   
   void		processCommand(int cmd);
   void		createPopupMenu(void);
   void		setupMenu(QMenuData * menu, QMenuData * pane=0);
   void		runPopupMenu(QMouseEvent * ev);

   void		compressDocument(void);
   
      //void		logChanges(int flags, const char * descr);
   virtual void	updateEditToolBar(void);
   void		setCaption(void);

   void		gotoPage(int page);

   void		copyAnnoBack(bool mark_doc_as_modified=true);
   void		createSharedAnno(void);

   void		insertPage(int page_num,const char filter[]);
   void		removePage(int page_num);
   void		removePages(const GList<int> & page_list);
   void		movePage(int page_num);
   void		movePages(const GList<int> & page_list);

   void		generateThumbnails(void);
   void		removeThumbnails(void);
   bool		checkThumbnails(void);

   void		HL_Edit(void);
   void		HL_Del(const MapArea * obj=0);
   void		HL_Create(int what);
   void		HL_CancelNewMapArea(void);
   bool		HL_Motion(int x, int y);
   bool		HL_Btn1Motion(int x, int y);
   bool		HL_Btn1Down(int x, int y);
   bool		HL_Btn1DblClick(int x, int y);
   bool		HL_Btn1Up(int x, int y);
   void		HL_AbortMotion(void);
   void		HL_CopyToANT(void);

   void		setDocModified(bool on);
   void		setEditorMode(EDITOR_MODE mode);
   void		savePagesAs(void);

   void         OCRProcessDocument(void);
   void         OCRProcessPage(void);
   
private slots:

   void		slotPopupCB(int id) { popup_menu_id=id; };
   void		slotDummy(void) {};

      // Structure manipulators (connected to thumbnails)
   void		slotInsertPage(int page_num);
   void		slotImportPage(int page_num);
   void		slotRemovePages(const GList<int> & page_list);
   void		slotMovePages(const GList<int> & page_list);
   void		slotSavePagesAs(const GList<int> & page_list);
   void		slotCut(const GList<int> & page_list);
   void		slotCopy(const GList<int> & page_list);
   void		slotPasteBefore(void);
   void		slotPasteAfter(void);
   void		paste(int atPage);
   void		slotCloseThumbnails(void);

      // Search stuff
   void		slotSearchClosed(void);
   void		slotDisplaySearchResults(int page_num, const GList<DjVuTXT::Zone *> & zones_list);

      // for thumbnail
   void		slotGotoPage(int page_num);
   //void		slotDoCmd(int cmd);

      // To receive status from QDBase
   void		slotShowStatus(const QString &msg);

      // Slots for QDPort connections
   void		slotNotifyError(const GP<DjVuPort> & source, const GUTF8String &msg);
   void		slotNotifyStatus(const GP<DjVuPort> & source, const QString &msg);
   
   void		slotCompressOK(int);

   void         slotDocModified(void) { setDocModified(true); }
   
protected:
   virtual bool eventFilter(QObject *obj, QEvent *ev);
   virtual void	createMapAreas(bool);
   virtual void	setCursor(void);
   
   virtual QWidget * createThumbnails(bool _rowMajor);

   virtual void	fillToolBar(class QDToolBar * toolbar) {}
   virtual void	updateToolBar(void);
   virtual void	setDjVuImage(const GP<DjVuImage> & _dimg, int do_redraw);
   virtual void	decodeAnno(bool allow_redraw);
   void init(GP<DjVuDocEditor> &doc_in, int page_num);
signals:
   void		sigPageChanged(int page_num);
public:
   void		show(void);
   void		savePageAs(int page_num=-1);
   void		savePagesAs(const GList<int> & pages_list);
   void		saveDocument(void);
   void		saveDocumentAs(void);
   void		print(int what);

   void		search(void);

   virtual void	imageUpdated(void);
   
   void		setDjVuDocument(GP<DjVuDocEditor> &doc_in, int page_num=0);
   void		setDjVuDocument(const GURL &url, int page_num=0);

   bool         getFileDataFileName(int page, GP<DataPool> &data, GUTF8String &name);
   
   virtual QSize sizeHint(void) const;

   QDEditor(GP<DjVuDocEditor> &doc,
	    int page_num, QDEditorMainWindow *_main_window, QWidget * parent=0, const char * name=0);
   QDEditor(const char *fname,
	    int page_num=0, QDEditorMainWindow *_main_window=0, QWidget * parent=0, const char * name=0);
   ~QDEditor(void);

protected:
   //virtual void	resizeEvent(QResizeEvent * ev);

};

// convenient class
class GDjVuDocEditor
{
   GP<DjVuDocEditor> doc;
public:
   GDjVuDocEditor(DjVuDocEditor *_doc) : doc(_doc) {}
   bool operator!(void) const {return !(const DjVuDocEditor *)doc;}
   DjVuDocEditor* operator-> (void) {return doc;}
   const DjVuDocEditor* operator-> (void) const {return doc;}
   operator GP<DjVuDocument> (void) { return (DjVuDocEditor *)doc;}
   operator DjVuDocEditor *(void) { return doc;}
   operator const DjVuDocEditor *(void) const { return doc; }
};


#endif // HDR_QD_EDITOR
