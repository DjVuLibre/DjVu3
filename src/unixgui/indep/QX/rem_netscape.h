//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: rem_netscape.h,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifndef HDR_REM_NETSCAPE
#define HDR_REM_NETSCAPE

#ifdef __GNUC__
#pragma interface
#endif

#include <sys/types.h>

// RemoteNetscape - class for controlling Netscape remotely. X11 specific

// We've got pretty weird types here. All the stuff is to avoid using X11
// and QT headers in one file

class RemoteNetscape
{
private:
   void		* displ;			// Actual type is Display *
   u_long	ref_window, top_window;		// Actual type is Window
   
   int		have_lock;

   void		ObtainLock(void);
   void		ReleaseLock(void);
   void		SendCommand(const char * cmd);
public:
   u_long	GetTopWindow(void);		// Actual type is Window
   void		SendPage(void);

   void		SetRefWindow(u_long _ref_window)	// Window
   {
      ref_window=_ref_window; top_window=0;
   };
   
   RemoteNetscape(void) : displ(0), ref_window(0), top_window(0), have_lock(0) {};
   RemoteNetscape(void * _displ, u_long _ref_window) : displ(_displ),
      ref_window(_ref_window), top_window(0), have_lock(0) {};
   ~RemoteNetscape(void);
};

#endif
