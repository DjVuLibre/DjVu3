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
// $Id: qd_print_dialog.h,v 1.3 2001-07-25 17:10:43 mchen Exp $
// $Name:  $


#ifndef HDR_QD_PRINT_DIALOG
#define HDR_QD_PRINT_DIALOG

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuImage.h"
#include "DjVuDocument.h"
#include "DjVuToPS.h"
#include "GRect.h"

#include <qdialog.h>
#include <qstring.h>
#include <sys/types.h>

#include "prefs.h"
#include "qt_n_in_one.h"
#include "qt_fix.h"

class QDPrintDialog : public QeDialog
{
   Q_OBJECT
public:
   enum What { PRINT_PAGE, PRINT_DOC, PRINT_CUSTOM, PRINT_WIN };
private:
   class QeRadioButton	* ps_butt, * eps_butt;
   class QeRadioButton	* portrait_butt, * landscape_butt;
   class QeRadioButton	* color_butt, * grey_butt;
   class QeRadioButton	* level1_butt, * level2_butt;
   class QeComboBox	* what_menu;
   class QeSpinBox	* copies_spin;
   class QeLabel	* custompages_label;
   class QeLineEdit	* custompages_text;
   class QeCheckBox	* save_butt;
   class QeButtonGroup	* format_bg, * orient_bg, * color_bg;
   class QeButtonGroup	* scale_bg, * what_bg;
   class QeComboBox	* zoom_menu;
   class QeSpinBox	* zoom_spin;

   QeNInOne		* dst_widget;
   QWidget		* printer_widget, * file_widget;
   class QeRadioButton	* printer_butt, * file_butt;
   class QeLineEdit	* printer_text, * file_text;

   QeNInOne		* prog_widget;
   class QeProgressBar	* progress;
   class QePushButton	* cancel_butt;

   GP<DjVuDocument>	doc;
   GP<DjVuImage>	dimg;

   int			cur_page_num;
   bool			printing, interrupt_printing;

   DjVuPrefs		* prefs;
   int			displ_mode;
   int			cur_zoom;

   GRect		print_rect;

   static const QString	id2str(int id);
   static int		str2id(const QString &);
   
   static void		refresh_cb(void * cl_data);
   static void		prnProgress_cb(float done, void * cl_data);
   static void		decProgress_cb(float done, void * cl_data);
   static void		info_cb(int page_num, int page_cnt, int tot_pages,
				DjVuToPS::Stage stage,void * cl_data);

   void			adjustScaling(void);
   void			adjustWhat(void);
   void			setSensitivity(void);
private slots:
   void		slotBrowse(void);
   void		slotFormatChanged(void);
   void		slotDstChanged(void);
   void		slotWhatChanged(const QString & text);
   void		slotZoomChanged(const QString & text);
protected slots:
   virtual void	done(int);
public:
   virtual bool	eventFilter(QObject * obj, QEvent * ev);
   
   void		setPSFormat(bool ps);
   void		setPortrait(bool portrait);
   void		setColorMode(bool color);
   void		setPSLevel(int level);
   void		setZoom(int zoom);	// Negative number means "reduce to fit"
   void		setCurZoom(int zoom);	// number 5..999
   void		setPrint(What what);
   void		setFileName(const QString &name);
   void		setCommand(const QString &cmd);
   void		printToFile(int file);
   
   QDPrintDialog(const GP<DjVuDocument> & doc, const GP<DjVuImage> & cur_dimg,
		 DjVuPrefs * prefs, int displ_mode, int cur_zoom,
		 const GRect & prn_rect, QWidget * parent=0,
		 const char * name=0, bool modal=FALSE);
   ~QDPrintDialog(void) {}
};

#endif
