//C-  -*- C++ -*-
//C-
//C- DjVu® Unix Viewer (v. 3.5)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: qt_fix.h,v 1.2 2001-07-25 17:10:42 mchen Exp $
// $Name:  $


#ifdef __GNUC__
#pragma interface
#endif

#include "qwidget.h"

void ActivateLayouts(QWidget * start);

#if (defined(QT_FIX_DO_ALL) || defined(QAPPLICATION_H)) && !defined(QeAPPLICATION_DEFINED)
#define QeAPPLICATION_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qapplication.h>
#endif
#include <qlist.h>
#include <qtimer.h>
#include "debug.h"

extern struct QeApplication * qeApp;

class QeApplication : public QApplication
{
   Q_OBJECT
private:
   QList<QWidget>	widgetsToKill;
   QTimer		kill_timer;

   void		construct(void);
private slots:
   void		killTimeout(void);
signals:
   void		gotX11Event(XEvent *);
public slots:
   void		slotKillSender(void);
public:
   float	gamma;
   QString	geometry;
   
      // killWidget() kills the mentioned widget when QApplication is about
      // to block. It will not kill a widget if it has already been deleted
      // by someone.
   void		killWidget(QWidget * widget)
   {
      if (widget)
      {
	 DEBUG_MSG("QeApplication::killWidgets(): scheduling " << widget << " for kill\n");
	 widgetsToKill.append(widget);
	 if (!kill_timer.isActive()) kill_timer.start(0, TRUE);
      }
   }

      // Resizes and moved widget in accordance with X11 geometry data.
   void		setWidgetGeometry(QWidget * w);
   
#ifdef UNIX
   bool		x11EventResult;
   virtual bool	x11EventFilter(XEvent * ev);
   QeApplication(Display * displ) : QApplication(displ) { construct(); }
#endif
   
   QeApplication(int & argc, char ** argv) :
      QApplication(argc, argv)
   {
      construct();
      gamma=2.2;
   }
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QDIALOG_H)) && !defined(QeDIALOG_DEFINED)
#define QeDIALOG_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qdialog.h>
#endif
class QeDialog : public QDialog
// Simple hack: No need now to do resize(0, 0) AND the dialog will always try
// to get its minimum size even after removing/adding new widgets
// Also: pass it non-zero parent, and it will become transient to it
// regardless of modal mode
{
   Q_OBJECT
private:
   bool		modal;
   QWidget	* parent;	// Parent passed to the constructor
   QWidget	* aux_widget;
   bool		wresizable, hresizable;
   
   virtual bool	event(QEvent * ev);	// Used with aux_widget
   virtual bool	eventFilter(QObject * obj, QEvent * ev); // Used w/o aux_widget
   virtual bool	x11Event(XEvent * ev);

   void		setResizeDecorations(bool enabled);
signals:
   void		sigClosed(void);
   void		sigCancelled(void);
public slots:
   void		slotParentDestroyed(void);
public:
   static void	makeTransient(QWidget * dialog, QWidget * parent);

   virtual void	resize(int w, int h);
   virtual void	setGeometry(int x, int y, int w, int h);
   virtual void	show(void);
   virtual void	done(int rc)
   {
      if (rc==Accepted) emit sigClosed();
      else emit sigCancelled();
      QDialog::done(rc);
   }
   
   QWidget	* startWidget(void) { return aux_widget ? aux_widget : this; }

   void		setResizable(bool wres, bool hres);
   void		setResizable(bool res) { setResizable(res, res); }
   
   QeDialog(QWidget * parent=0, const char * name=0,
	    bool modal=FALSE, WFlags f=0);
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QLABEL_H)) && !defined(QeLABEL_DEFINED)
#define QeLABEL_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qlabel.h>
#endif
class QeLabel : public QLabel
{
   Q_OBJECT
private:
   inline void	setMinSize(void)
   {
      setMinimumSize(sizeHint());
      ActivateLayouts(this);
   }
   inline bool	isText(void) const { return !pixmap() && !movie(); }
   virtual void	resizeEvent(QResizeEvent * ev);
public:
   QeLabel(QWidget * parent=0, const char * name=0, WFlags f=0) :
	 QLabel(parent, name, f) {}
   QeLabel(const QString & text, QWidget * parent=0, const char * name=0, WFlags f=0) :
	 QLabel(text, parent, name, f) {}
   QeLabel(QWidget * buddy, const QString & text, QWidget * parent, const char * name=0, WFlags f=0) :
	 QLabel(buddy, text, parent, name, f) {}
public slots:
   virtual void	show(void)
   {
      if (!isText() || !(alignment() & WordBreak)) setMinSize();
      QLabel::show();
   }
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QPUSHBUTTON_H)) && !defined(QePUSHBUTTON_DEFINED)
#define QePUSHBUTTON_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qpushbutton.h>
#endif
class QePushButton : public QPushButton
{
   Q_OBJECT
private:
   int		infl_w, infl_h;
   inline void	setMinSize(void)
   {
      setMinimumSize(sizeHint());
      ActivateLayouts(this);
   }
public:
   void		inflateWidth(int _infl_w) { infl_w=_infl_w; }
   void		inflateHeight(int _infl_h) { infl_h=_infl_h; }
   virtual QSize	sizeHint(void) const
   {
      QSize size=QPushButton::sizeHint();
      return QSize(size.width()+2*infl_w, size.height()+2*infl_h);
   }
   QePushButton(QWidget * parent=0, const char * name=0) :
	 QPushButton(parent, name), infl_w(0), infl_h(0)
      { setAutoResize(TRUE); setMinSize(); }
   QePushButton(const QString & text, QWidget * parent=0, const char * name=0) :
	 QPushButton(text, parent, name), infl_w(0), infl_h(0)
      { setAutoResize(TRUE); setMinSize(); }
public slots:
   virtual void	show(void)
   {
      setMinSize();
      QPushButton::show();
   }
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QCHECKBOX_H)) && !defined(QeCHECKBOX_DEFINED)
#define QeCHECKBOX_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qcheckbox.h>
#endif
class QeCheckBox : public QCheckBox
{
   Q_OBJECT
private:
   inline void	setMinSize(void)
   {
      setMinimumSize(sizeHint());
      ActivateLayouts(this);
   }
public:
   QeCheckBox(QWidget * parent=0, const char * name=0) :
	 QCheckBox(parent, name) { setMinSize(); }
   QeCheckBox(const QString & text, QWidget * parent, const char * name=0) :
	 QCheckBox(text, parent, name) { setMinSize(); }
public slots:
   virtual void	show(void)
   {
      setMinSize();
      QCheckBox::show();
   }
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QRADIOBUTTON_H)) && !defined(QeRADIOBUTTON_DEFINED)
#define QeRADIOBUTTON_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qradiobutton.h>
#endif
class QeRadioButton : public QRadioButton
{
   Q_OBJECT
private:
   void		setMinSize(void)
   {
      setMinimumSize(sizeHint());
      ActivateLayouts(this);
   }
public:
   QeRadioButton(QWidget * parent=0, const char * name=0) :
	 QRadioButton(parent, name) { setMinSize(); }
   QeRadioButton(const QString & text, QWidget * parent, const char * name=0) :
	 QRadioButton(text, parent, name) { setMinSize(); }
public slots:
   virtual void	show(void)
   {
      setMinSize();
      QRadioButton::show();
   }
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QLINEEDIT_H)) && !defined(QeLINEEDIT_DEFINED)
#define QeLINEEDIT_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qlineedit.h>
#endif
class QeLineEdit : public QLineEdit
{
   Q_OBJECT
private:
   inline void	setMinMaxSize(void)
   {
      setMinimumSize(sizeHint());
      setMaximumHeight(sizeHint().height());
      ActivateLayouts(this);
   }
public slots:
   virtual void	show(void)
   {
      setMinMaxSize();
      QLineEdit::show();
   }
public:
   QeLineEdit(QWidget * parent=0, const char * name=0) :
	 QLineEdit(parent, name) { setMinMaxSize(); }
signals:
   void textChanged(const QString & text);
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QSLIDER_H)) && !defined(QeSLIDER_DEFINED)
#define QeSLIDER_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qslider.h>
#endif
class QeSlider : public QSlider
{
   Q_OBJECT
private:
   inline void	setMinMaxSize(void)
   {
      setMinimumSize(sizeHint());
      ActivateLayouts(this);
   }
public:
   QeSlider(QWidget * parent=0, const char * name=0) :
	 QSlider(parent, name) { setMinMaxSize(); }
   QeSlider(Orientation or, QWidget * parent=0, const char * name=0) :
	 QSlider(or, parent, name) { setMinMaxSize(); }
   QeSlider(int min, int max, int step, int value,
	    Orientation or, QWidget * parent=0, const char * name=0) :
	 QSlider(min, max, step, value, or, parent, name) { setMinMaxSize(); }
public slots:
   virtual void	show(void)
   {
      setMinMaxSize();
      QSlider::show();
   }
};
#endif


#if (defined(QT_FIX_DO_ALL) || defined(QCOMBOBOX_H)) && !defined(QeCOMBOBOX_DEFINED)
#define QeCOMBOBOX_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qcombobox.h>
#endif
class QeComboBox : public QComboBox
{
   Q_OBJECT
private:
   inline void	setMinMaxSize(void)
   {
      setMinimumWidth(sizeHint().width());
      setMinimumHeight(sizeHint().height());
      setMaximumHeight(sizeHint().height());
      ActivateLayouts(this);
   }
public:
   void		setCurrentItem(const QString & text);
   void		setCurrentItem(int item_num)
   { QComboBox::setCurrentItem(item_num); }

   QeComboBox(QWidget * parent=0, const char * name=0) :
   QComboBox(parent, name) { setMinMaxSize(); }
   QeComboBox(bool rw, QWidget * parent=0, const char * name=0) :
   QComboBox(rw, parent, name) { setMinMaxSize(); }
signals:
   void activated(const QString & text);
   void highlighted(const QString & text);
   void textChanged(const QString & text);

public slots:
   virtual void	show(void)
   {
      setMinMaxSize();
      QComboBox::show();
   }
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QSPINBOX_H)) && !defined(QeSPINBOX_DEFINED)
#define QeSPINBOX_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qspinbox.h>
#endif
class QeSpinBox : public QSpinBox
{
   Q_OBJECT
private:
   inline void	setMinMaxSize(void)
   {
      setMinimumWidth(sizeHint().width());
      setMinimumHeight(sizeHint().height());
      setMaximumHeight(sizeHint().height());
      ActivateLayouts(this);
   }
protected:
   virtual bool	eventFilter(QObject *, QEvent *);
public:
   QeSpinBox(QWidget * parent=0, const char * name=0) :
	 QSpinBox(parent, name) { setMinMaxSize(); }
   QeSpinBox(int min, int max, int step=-1,
	     QWidget * parent=0, const char * name=0) :
	 QSpinBox(min, max, step, parent, name) { setMinMaxSize(); }
public slots:
   virtual void	show(void)
   {
      setMinMaxSize();
      QSpinBox::show();
   }
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QPROGRESSBAR_H)) && !defined(QePROGRESSBAR_DEFINED)
#define QePROGRESSBAR_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qprogressbar.h>
#endif
class QeProgressBar : public QProgressBar
{
   Q_OBJECT
private:
   QString	prefix;
   int		prefix_changed;
protected:
   bool		setIndicator(QString & string, int progress, int totalSteps)
   {
      bool rc=QProgressBar::setIndicator(string, progress, totalSteps) ||
	      prefix_changed;
      if (prefix.length()) string=prefix+string;
      prefix_changed=0;
      return rc;
   }
public:
   void		setPrefix(const QString & _prefix)
   {
      prefix_changed=(prefix!=_prefix);
      prefix=_prefix;
   }
   
   QeProgressBar(QWidget * parent=0, const char * name=0) :
	 QProgressBar(parent, name), prefix_changed(0) {}
   QeProgressBar(int totalSteps, QWidget * parent=0, const char * name=0) :
	 QProgressBar(totalSteps, parent, name), prefix_changed(0) {}
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QGROUPBOX_H)) && !defined(QeGROUPBOX_DEFINED)
#define QeGROUPBOX_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qgroupbox.h>
#endif
class QeGroupBox : public QGroupBox
{
   Q_OBJECT
private:
   void		init(void)
   {
      QFont fnt=font();
      fnt.setBold(TRUE);
      setFont(fnt);
   }
public:
   QeGroupBox(QWidget * parent=0, const char * name=0) :
	 QGroupBox(parent, name) { init(); }
   QeGroupBox(const QString & title, QWidget * parent=0, const char * name=0) :
	 QGroupBox(title, parent, name) { init(); }
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QBUTTONGROUP_H)) && !defined(QeBUTTONGROUP_DEFINED)
#define QeBUTTONGROUP_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qbuttongroup.h>
#endif
class QeButtonGroup : public QButtonGroup
{
   Q_OBJECT
private:
   void		init(void)
   {
      QFont fnt=font();
      fnt.setBold(TRUE);
      setFont(fnt);
   }
public:
   QeButtonGroup(QWidget * parent=0, const char * name=0) :
	 QButtonGroup(parent, name) { init(); }
   QeButtonGroup(const QString & title, QWidget * parent=0, const char * name=0) :
	 QButtonGroup(title, parent, name) { init(); }
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QMENUBAR_H)) && !defined(QeMENUBAR_DEFINED)
#define QeMENUBAR_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qmenubar.h>
#endif
class QeMenuBar : public QMenuBar
{
   Q_OBJECT
public:
   QeMenuBar(QWidget * parent=0, const char * name=0) :
	 QMenuBar(parent, name)
   {
      setSeparator(InWindowsStyle);
   }
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QFILEDIALOG_H)) && !defined(QeFILEDIALOG_DEFINED)
#define QeFILEDIALOG_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qfiledialog.h>
#endif
#include "GString.h"
class QeFileDialog : public QFileDialog
{
   Q_OBJECT
private:
   bool		forWriting;
protected slots:
   virtual void	done(int rc);
public:
   static QString	lastSaveDir;
   static QString	lastLoadDir;
   
   void		setForWriting(bool fwr)
   {
      forWriting=fwr;
      setMode(fwr ? QFileDialog::AnyFile : QFileDialog::ExistingFile);
   }
   
   QeFileDialog(const QString & dirName, const char * filter=0,
		QWidget * parent=0, const char * name=0, bool modal=FALSE);
   QeFileDialog(QWidget * parent=0, const char * name=0, bool modal=FALSE);
};

#endif

#ifndef __QT_FIX_H_
#define __QT_FIX_H_
#include "GString.h"
#include "qstring.h"
inline GUTF8String
GStringFromQString(const QString & x)
{
  GUTF8String retval=(const char *)x.utf8();
  return retval;
}
inline QString
QStringFromGString(const GUTF8String & x)
{
  QString retval=QString::fromUtf8((const char *)x);
  return retval;
}
//#endif /* QT1 */
#endif /* __QT_FIX_H_ */

