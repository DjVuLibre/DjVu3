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
// $Id: col_db.cpp,v 1.2 2001-07-25 17:10:42 mchen Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "col_db.h"
#include "GException.h"

ColorDB::ColorItem	ColorDB::color[]=
{
   { "black",		0,   0,   0 },
   { "blue",		0,   0,   255 },
   { "turquoise",	64,  224, 208 },
   { "green", 		0,   255, 0 },
   { "pink",		255, 192, 203 },
   { "magenta",		255, 0,   255 },
   { "red",		255, 0,   0 },
   { "brown",		165, 42,  42 },
   { "yellow",		255, 255, 0 },
   { "white",		255, 255, 255 },
   { "dark blue",	0,   0,   139 },
   { "violet",		238, 130, 238 },
   { "dark red",	139, 0,   0 },
   { "dark yellow",	139, 139, 0 },
   { "dark gray",	128, 128, 128 },
   { "gray",		211, 211, 211 }
};

char ** ColorDB::name;
unsigned char * ColorDB::red;
unsigned char * ColorDB::green;
unsigned char * ColorDB::blue;
GMap<u_int32, int>	* ColorDB::c32_to_num;
GMap<GUTF8String, int>	* ColorDB::string_to_num;
   
int ColorDB::colors=sizeof(ColorDB::color)/sizeof(ColorDB::color[0]);

static ColorDB color_db;

void ColorDB::InitializeMaps(void)
{
   if (!c32_to_num) c32_to_num=new GMap<u_int32, int>();
   if (!string_to_num) string_to_num=new GMap<GUTF8String, int>();
}

ColorDB::ColorDB(void)
{
   red=new unsigned char[colors];
   green=new unsigned char[colors];
   blue=new unsigned char[colors];
   name=new char*[colors];
   
   if (!red || !green || !blue || !name)
      G_THROW("ColorDB::ColorDB(): Not enough memory to initialize color database.");
   
   InitializeMaps();
   
   for(int i=0;i<colors;i++)
   {
      red[i]=color[i].red;
      green[i]=color[i].green;
      blue[i]=color[i].blue;
      name[i]=color[i].name;
      
      u_int32 color32=
	 ((u_int32) red[i] << 16) |
	 ((u_int32) green[i] << 8) |
	 (u_int32) blue[i];
      (*c32_to_num)[color32]=i;
      (*string_to_num)[name[i]]=i;
   };
}
