//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: prefs.h,v 1.1 2001-08-08 17:38:05 docbill Exp $
// $Name:  $


#ifndef HDR_PREFS
#define HDR_PREFS

#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"

class DjVuPrefs
{
private:
   void		* database;

   GUTF8String	getString(const char * name);
   void		setString(const char * name, const char * value);
   int		getInt(const char * name);
   void		setInt(const char * name, int value);
public:
   enum HLButtType	{ HLB_SHIFT=0, HLB_SHIFT_SHIFT=1, HLB_ALT=2,
			  HLB_ALT_ALT=3, HLB_CTRL=4, HLB_CTRL_CTRL=5,
			  HLB_ITEMS=6 };
   enum MagButtType	{ MAG_SHIFT=0, MAG_ALT=1, MAG_CTRL=2,
			  MAG_MID=3, MAG_ITEMS=4 };
   static const int	legal_mag_size[]={ 50, 75, 100, 125, 150, 175, 200,
					   225, 250, 275, 300 };
   static const int	legal_mag_scale[]={ 15, 20, 25, 30, 35, 40, 45, 50 };
   static const int	legal_mag_size_num=sizeof(legal_mag_size)/sizeof(legal_mag_size[0]);
   static const int	legal_mag_scale_num=sizeof(legal_mag_scale)/sizeof(legal_mag_scale[0]);
   
   static char	* hlb_names[];
   static char	* mag_names[];

   char		bBeginner;

   int		nDefaultZoom;
   double	dScreenGamma;
   double	dPrinterGamma;
   bool		printColor;
   bool		printPortrait;
   bool		printToFile;
   bool		printPS;
   bool		printLevel2;
   bool		printFitPage;
   bool		printAllPages;
   GUTF8String	printCommand;
   GUTF8String	printFile;
   bool		hlinksPopup;
   bool		hlinksBorder;
   HLButtType	hlb_num;
   int		pcacheSize;
   int		mcacheSize;
   bool		toolBarOn;
   int		toolBarDelay;
   bool		toolBarAlwaysVisible;
   bool		fastZoom;
   bool		optimizeLCD;
   
   int		magnifierSize;
   int		magnifierScale;
   MagButtType	magnifierHotKey;

   bool		fastThumb;

   bool		mimeDontAsk;
   bool		mimeDontCheck;
  
   void		save(void);
   void		load(void);

   DjVuPrefs(void);
   virtual ~DjVuPrefs(void);
};

#endif
