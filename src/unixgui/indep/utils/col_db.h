//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: col_db.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_COL_DB
#define HDR_COL_DB

#ifdef __GNUC__
#pragma interface
#endif

#include "int_types.h"
#include "GContainer.h"
#include "GString.h"

class ColorDB
{
private:
   static GMap<u_int32, int>	* c32_to_num;
   static GMap<GUTF8String, int>	* string_to_num;
   
   static void	InitializeMaps(void);
public:
   class ColorItem
   {
   public:
      char		* name;
      unsigned char	red, green, blue;
   };
   static ColorItem	color[];
   static int		colors;
   static char 		** name;
   static unsigned char	* red, * green, * blue;
   
   static int	GetColorNum(u_int32 color);
   static int	GetColorNum(const char * color);
   static int	C32_to_Num(u_int32 color);
   static u_int32	Num_to_C32(int color_num);
   static const char *	Num_to_Name(int color_num);
   
   
   static unsigned char	C32_GetRed(u_int32 color);
   static unsigned char	C32_GetGreen(u_int32 color);
   static unsigned char	C32_GetBlue(u_int32 color);

   static u_int32	RGB_to_C32(unsigned char r,
				   unsigned char g,
				   unsigned char b);
   
   ColorDB(void);
};

inline int ColorDB::GetColorNum(u_int32 color)
{
   if (!c32_to_num) InitializeMaps();
   return c32_to_num->contains(color) ? (*c32_to_num)[color] : -1;
}

inline int ColorDB::GetColorNum(const char * color)
{
   if (!string_to_num) InitializeMaps();
   return string_to_num->contains(color) ? (*string_to_num)[color] : -1;
}

inline int ColorDB::C32_to_Num(u_int32 color)
{
   if (!c32_to_num) InitializeMaps();
   return c32_to_num->contains(color) ? (*c32_to_num)[color] : -1;
}

inline u_int32 ColorDB::Num_to_C32(int color_num)
{
   if (color_num<0 || color_num>=colors) color_num=0;
   return
      ((u_int32) red[color_num] << 16) |
      ((u_int32) green[color_num] << 8) |
      (u_int32) blue[color_num];
}

inline const char * ColorDB::Num_to_Name(int color_num)
{
   if (color_num<0 || color_num>=colors) color_num=0;
   return name[color_num];
}

inline unsigned char ColorDB::C32_GetRed(u_int32 color)
{
   return (color >> 16) & 0xff;
}

inline unsigned char ColorDB::C32_GetGreen(u_int32 color)
{
   return (color >> 8) & 0xff;
}

inline unsigned char ColorDB::C32_GetBlue(u_int32 color)
{
   return color & 0xff;
}

inline u_int32 ColorDB::RGB_to_C32(unsigned char r,
				   unsigned char g,
				   unsigned char b)
{
   return (r << 16) | (g << 8) | b;
}

#endif
