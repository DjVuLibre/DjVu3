//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: saved_data.h,v 1.1 2001-08-08 17:38:05 docbill Exp $
// $Name:  $


#ifndef HDR_SAVED_DATA
#define HDR_SAVED_DATA

#ifdef __GNUC__
#pragma interface
#endif

struct SavedData
{
   int  cmd_mode;
   int  cmd_zoom;
   int  imgx;
   int  imgy;

   int		isEmpty(void) const
   {
      return cmd_mode==0 && cmd_zoom==0 && imgx==0 && imgy==0;
   };
   SavedData(void) : cmd_mode(0), cmd_zoom(0), imgx(0), imgy(0) {};
};

#endif
