//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: cin_data.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $

 
#ifndef HDR_CIN_DATA
#define HDR_CIN_DATA

#ifdef __GNUC__
#pragma interface
#endif

#include "ByteStream.h"
#include "GContainer.h"
#include "GString.h"

class CINData
{
private:
   class Item : public GPEnabled
   {
   public:
      GUTF8String		name;
      GTArray<char>	data;

      Item(const char * name, const char * buffer, int length);
   };
   static GPList<Item>	* items;
   static GPList<Item>	* get_items(void);
public:
   static GP<ByteStream>get(const char * name);
   static void		add(const char * name, const char * data, int length);
};

#endif
