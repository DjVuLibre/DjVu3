//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: col_db.cpp,v 1.1 2001-05-29 22:05:30 bcr Exp $
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
