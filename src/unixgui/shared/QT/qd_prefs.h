//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_prefs.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_PREFS
#define HDR_QD_PREFS

#ifdef __GNUC__
#pragma interface
#endif

#include <qframe.h>
#include <qgroupbox.h>

#include "prefs.h"
#include "qt_fix.h"

// These are components, which will be used by QDViewerPrefs or
// QDEditorPrefs in their "Preferences" dialogs. It's up to these two
// classes to position the following boxes inside the dialog. Each of
// the boxes is self contained

// Gamma correction preview
class QDGammaDispl : public QFrame
{
   Q_OBJECT
private:
   double	gamma;
protected:
   virtual void	resizeEvent(QResizeEvent *);
   virtual void	paintEvent(QPaintEvent *);
   virtual bool	event(QEvent *);
public slots:
   void		setGamma(int _gamma);
public:
   QDGammaDispl(QWidget * parent=0, const char * name=0);
   ~QDGammaDispl(void) {}
};

// "Gamma Correction: box
class QDGammaPrefs : public QeGroupBox
{
   Q_OBJECT
private:
   class QeCheckBox	* match_butt;
   class QeSlider	* displ_slider, * print_slider;
   class QDGammaDispl	* preview;
private slots:
   void		printValueChanged(int v);
   void		displValueChanged(int v);
   void		matchToggled(bool state);
public:
   double	displGamma(void) const;
   double	printGamma(void) const;
   bool		match(void) const;

   QDGammaPrefs(DjVuPrefs * prefs, QWidget * parent=0, const char * name=0);
   ~QDGammaPrefs(void) {}
};

// "Magnifying Glass" box
class QDLensPrefs : public QeGroupBox
{
   Q_OBJECT
private:
   class QeComboBox	* size_menu, * scale_menu, * hkey_menu;
private slots:
   void		slotHotKeyChanged(const QString & text);
signals:
   void		sigHotKeyChanged(DjVuPrefs::MagButtType key);
public slots:
   void		slotHlHotKeyChanged(DjVuPrefs::HLButtType key);
public:
   DjVuPrefs::MagButtType hotKey(void) const;
   int		size(void) const;
   int		scale(void) const;

   QDLensPrefs(DjVuPrefs * prefs, QWidget * parent=0, const char * name=0);
   ~QDLensPrefs(void) {}
};

// "Optimization" box
class QDOptimPrefs : public QeGroupBox
{
   Q_OBJECT
private:
   class QeCheckBox	* fastzoom_butt, * lcd_butt, * fastthumb_butt;
public:
   bool		fastZoom(void) const;
   bool		fastThumb(void) const;
   bool		optimizeLCD(void) const;

   QDOptimPrefs(DjVuPrefs * prefs, QWidget * parent=0, const char * name=0);
   ~QDOptimPrefs(void) {}
};

// "Cache Preferences" box
class QDCachePrefs : public QeGroupBox
{
   Q_OBJECT
private:
   class QeComboBox		* mcache_menu;
   class QeLabel		* pcache_label;
   class QeComboBox		* pcache_menu;
   class QPushButton		* pcache_butt;
private slots:
   void		slotClearCache(void);
public:
   int		mcacheSize(void) const;
   int		pcacheSize(void) const;
   void		enablePCache(bool en);
   void		resetPCache(void);
   
   QDCachePrefs(DjVuPrefs * prefs, bool pcache_on,
		QWidget * parent=0, const char * name=0);
   ~QDCachePrefs(void) {}
};

// "Toolbar Preferences Box"
class QDTbarPrefs : public QeGroupBox
{
   Q_OBJECT
private:
   class QeCheckBox		* on_butt;
   class QeCheckBox		* vis_butt;
   class QeLabel		* delay_label;
   class QeSpinBox		* delay_spin;
private slots:
   void		slotTBToggled(bool);
public:
   bool		enabled(void) const;
   bool		visible(void) const;
   int		delay(void) const;
   
   QDTbarPrefs(DjVuPrefs * prefs, QWidget * parent=0, const char * name=0);
   ~QDTbarPrefs(void) {}
};

#endif
