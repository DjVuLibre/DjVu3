//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.
//C- Copyright (c) 2000 LizardTech Inc.
//C- All rights reserved.
//C-
//C- This software may only be used by you under license from LizardTech
//C- Inc. A copy of LizardTech's Source Code Agreement is available at
//C- LizardTech's Internet website having the URL <http://www.djvu.com/open>.
//C- If you received this software without first entering into a license with
//C- LizardTech, you have an infringing copy of this software and cannot use it
//C- without violating LizardTech's intellectual property rights.
//C-
//C- $Id: DjVuToPS.cpp,v 1.14 2000-09-18 17:10:14 bcr Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuToPS.h"
#include "debug.h"

#include <stdarg.h>
#include <time.h>

#ifdef UNIX
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#endif

// ***************************************************************************
// ****************************** Options ************************************
// ***************************************************************************

DjVuToPS::Options::Options(void)
{
   format=PS;
   orientation=PORTRAIT;
   level=2;
   mode=COLOR;
   zoom=FIT_PAGE;
   color=true;
   gamma=2.60;
   copies=1;
   frame=false;
}

void
DjVuToPS::Options::set_format(Format _format)
{
   if (_format!=PS && _format!=EPS)
      G_THROW("PostScript format must be either PS or EPS.");
   
   format=_format;
   if (format==EPS)
   {
      orientation=PORTRAIT;
      copies=1;
   }
}

void
DjVuToPS::Options::set_level(int _level)
{
   if (_level!=1 && _level!=2)
      G_THROW("Invalid PostScript level "+GString(_level)+". Must be either 1 or 2.");
   
   level=_level;
}

void
DjVuToPS::Options::set_orientation(Orientation _orientation)
{
   if (_orientation!=PORTRAIT && _orientation!=LANDSCAPE)
      G_THROW("Invalid orientation passed to printing code.");
   if (format==EPS && _orientation==LANDSCAPE)
      G_THROW("LANDSCAPE mode is not supported by EPS format.");
   
   orientation=_orientation;
}

void
DjVuToPS::Options::set_mode(Mode _mode)
{
   if (_mode!=COLOR && _mode!=FORE && _mode!=BACK && _mode!=BW)
      G_THROW("Invalid image mode passed to printing code.");
   
   mode=_mode;
}

void
DjVuToPS::Options::set_zoom(Zoom _zoom)
{
   if (_zoom!=FIT_PAGE && !(_zoom>=5 && _zoom<=999))
      G_THROW("Invalid zoom factor passed to printing code.");
   
   zoom=_zoom;
}

void
DjVuToPS::Options::set_color(bool _color)
{
   color=_color;
}

void
DjVuToPS::Options::set_gamma(float _gamma)
{
   if (_gamma<0.3-0.0001 || _gamma>5.0+0.0001)
      G_THROW("Invalid gamma specified. Must be between 0.3 and 5.0");
   gamma=_gamma;
}

void
DjVuToPS::Options::set_copies(int _copies)
{
   if (_copies<=0)
      G_THROW("The number of copies must be positive.");
   if (format==EPS && _copies!=1)
      G_THROW("Only one copy can be printed in EPS format.");
   
   copies=_copies;
}

void
DjVuToPS::Options::set_frame(bool _frame)
{
   frame=_frame;
}

// ***************************************************************************
// ******************************* DjVuToPS **********************************
// ***************************************************************************

char	DjVuToPS::bin2hex[256][2];

DjVuToPS::DjVuToPS(void)
{
   DEBUG_MSG("DjVuToPS::DjVuToPS(): initializing...\n");
   DEBUG_MAKE_INDENT(3);

   DEBUG_MSG("Initializing dig2hex[]\n");
      
      // Creating tables for bin=>text translation
   static char * dig2hex="0123456789ABCDEF";
   int i;
   for(i=0;i<256;i++)
   {
      bin2hex[i][0]=dig2hex[i/16];
      bin2hex[i][1]=dig2hex[i%16];
   }

   refresh_cb=0;
   refresh_cl_data=0;
   
   prn_progress_cb=0;
   prn_progress_cl_data=0;

   dec_progress_cb=0;
   dec_progress_cl_data=0;

   info_cb=0;
   info_cl_data=0;
}

void
DjVuToPS::write(ByteStream & str, const char * format, ...)
{
   va_list args;
   va_start(args, format);
  
   GString tmp;
   tmp.format(format, args);
   str.writall((const char *) tmp, tmp.length());
}

// ************************* DOCUMENT LEVEL *********************************

void
DjVuToPS::store_doc_prolog(ByteStream & str, int pages, const GRect & grect)
{
   DEBUG_MSG("DjVuToPS::store_doc_prolog(): storing the document prolog\n");
   DEBUG_MAKE_INDENT(3);

   if (options.get_format()==Options::EPS)
      write(str,
	    "%%!PS-Adobe-3.0 EPSF-3.0\n"
	    "%%%%BoundingBox: 0 0 %d %d\n",
	    grect.width(), grect.height());
   else
      write(str, "%%!PS-Adobe-3.0\n");
   
   time_t time_=time(0);
#ifdef UNIX
   passwd * pswd;
   group  * grp;
   const char * gecos="Name unknown", * u_name="unknown", * g_name="unknown";
   pswd=getpwuid(getuid());
   if (pswd)
   {
      char * ptr;
      for(ptr=pswd->pw_gecos;*ptr!=0;ptr++)
	 if (*ptr==',') { *ptr=0;break; }
      if (pswd->pw_name && strlen(pswd->pw_name))
	 u_name=pswd->pw_name;
      if (pswd->pw_gecos && strlen(pswd->pw_gecos))
	 gecos=pswd->pw_gecos;
   }
   grp=getgrgid(getgid());
   if (grp && grp->gr_name && strlen(grp->gr_name))
      g_name=grp->gr_name;
#endif
   write(str,
	 "%%%%Title: DjVu PostScript document\n"
	 "%%%%Copyright: Copyright (c) 1998-2000 LizardTech.  All Rights Reserved\n"
	 "%%%%Creator: DjVu (code by Andrei Erofeev)\n"
#ifdef UNIX
	 "%%%%For: %s (%s.%s)\n"
#endif
	 "%%%%CreationDate: %s"
	 "%%%%Pages: %d\n"
	 "%%%%PageOrder: Ascend\n"
	 "%%%%DocumentData: Clean7Bit\n"
	 "%%%%Orientation: %s\n", 
#ifdef UNIX
	 gecos, u_name, g_name, 
#endif
	 ctime(&time_), pages,
	 options.get_orientation()==Options::PORTRAIT ?
	 "Portrait" : "Landscape" );
   if (options.get_color())
      write(str, "%%%%Requirements: color\n");
   write(str, "%%%%LanguageLevel: %d\n", options.get_level());
   if (options.get_level()==1 && options.get_color())
      write(str, "%%%%Extensions: CMYK\n");
   if (options.get_level()==2)
      write(str,
	    "%%%%DocumentNeededFonts: Helvetica\n"
	    "%%%%DocumentFonts: Helvetica\n");
   
   write(str,
	 "%%%%EndComments\n"
	 "%%%%EndProlog\n"
	 "\n");
}

void
DjVuToPS::store_doc_setup(ByteStream & str)
{
   DEBUG_MSG("DjVuToPS::store_doc_setup(): storing the document setup\n");
   DEBUG_MAKE_INDENT(3);

   write(str,
	 "%%%%BeginSetup\n"
	 "%% Change this number if you want more than one copy\n"
	 "/#copies %d def\n"
	 "\n"
	 "%% Remember the original state\n"
	 "/doc-origstate save def\n"
	 "\n"
	 "%% Build a temporary dictionary\n"
	 "40 dict begin\n"
	 "\n", options.get_copies());

   write(str,
"%% Very simple rectstroke & rectfill emulation (in case if we have level 1)\n"
"/MakeBox        %% stack : x y width height => None\n"
"{\n"
"   newpath\n"
"   4 2 roll moveto\n"
"   1 index 0 rlineto\n"
"   0 exch rlineto\n"
"   neg 0 rlineto\n"
"   closepath\n"
"} bind def\n"
"\n"
"/level 1 def\n"
"\n"
"/languagelevel where\n"
"{\n"
"   pop          %% get rid of dictionary\n"
"   /level languagelevel def\n"
"} if\n"
"\n"
"level 2 lt\n"
"{\n"
"   /rectfill    %% stack : x y width height => None\n"
"   {\n"
"      MakeBox fill\n"
"   } bind def\n"
"   /rectstroke  %% stack : x y width height => None\n"
"   {\n"
"      MakeBox stroke\n"
"   } bind def\n"
"} if\n\n");

   if (options.get_level()==2)
   {
      write(str,
	    "%%\n"
"%% Define function for displaying error messages\n"
"%%\n"
"/msg_y 0 def\n"
"/ShowError		%% stack: error_msg\n"
"{\n"
"   msg_y 0 eq\n"
"   {\n"
"      /Helvetica findfont 12 scalefont setfont\n"
"      /msg_y page-y page-height 2 div add def\n"
"   } if\n"
"   dup stringwidth pop\n"
"   page-x page-width 2 div add exch 2 div sub\n"
"   msg_y moveto\n"
"   show\n"
"   /msg_y msg_y 15 sub def\n"
"} bind def\n"
"\n"
"%%\n"
"%% Determine if the interpreter supports filters\n"
"%%\n"
"systemdict /resourcestatus known\n"
"{\n"
"   (ASCII85Decode) /Filter resourcestatus\n"
"   {\n"
"      pop pop\n"
"      (RunLengthDecode) /Filter resourcestatus\n"
"      {\n"
"         pop pop\n"
"      }\n"
"      {\n"
"         (Sorry, but the image can not be printed.) ShowError\n"
"	 (This PostScript interpreter does not support RunLengthDecode filter.) ShowError\n"
"	 showpage stop\n"
"      } ifelse\n"
"   }\n"
"   {\n"
"      (Sorry, but the image can not be printed.) ShowError\n"
"      (This PostScript interpreter does not support ASCII85Decode filter.) ShowError\n"
"      showpage stop\n"
"   } ifelse\n"
"} if\n\n");
      if (options.get_color())
         write(str, 
"%%\n"
"%% Check if we have one-argument variant of image operator here\n"
"%%\n"
"systemdict /setcolorspace known not\n"
"{\n"
"   (Sorry, but the image can not be printed.) ShowError\n"
"   (This PostScript interpreter does not support image dictionary.) ShowError\n"
"   showpage stop\n"
"} if\n"
"\n"
"%%\n"
"%% Define procedures for reading color image\n"
"%%\n"
"/readR () def\n"
"/readG () def\n"
"/readB () def\n"
"/ReadData\n"
"{\n"
"   currentfile /ASCII85Decode filter dup /RunLengthDecode filter\n"
"   bufferR readstring pop /readR exch def\n"
"   dup status { flushfile } { pop } ifelse\n"
"   currentfile /ASCII85Decode filter dup /RunLengthDecode filter\n"
"   bufferG readstring pop /readG exch def\n"
"   dup status { flushfile } { pop } ifelse\n"
"   currentfile /ASCII85Decode filter dup /RunLengthDecode filter\n"
"   bufferB readstring pop /readB exch def\n"
"   dup status { flushfile } { pop } ifelse\n"
"} bind def\n"
"\n"
"/ReadR\n"
"{\n"
"   readR length 0 eq { ReadData } if\n"
"   readR /readR () def\n"
"} bind def\n"
"\n"
"/ReadG\n"
"{\n"
"   readG length 0 eq { ReadData } if\n"
"   readG /readG () def\n"
"} bind def\n"
"\n"
"/ReadB\n"
"{\n"
"   readB length 0 eq { ReadData } if\n"
"   readB /readB () def\n"
"} bind def\n"
"\n");
   } else
   {
	 // level!=2
      if (options.get_color())
      {
	 write(str,
"%%\n"
"%% Declare buffers for reading image\n"
"%%\n"
"/buffer8 () def\n"
"/buffer24 () def\n");
	 write(str, 
"%%\n"
"%% Check if colorimage is supported and provide emulation if not\n"
"%%\n"
"systemdict /colorimage known\n"
"{\n"
"   /ColorProc\n"
"   {\n"
"      currentfile buffer24 readhexstring pop\n"
"   } bind def\n"
"   /ColorImage\n"
"   {\n"
"      colorimage\n"
"   } bind def\n"
"}\n"
"{\n"
"   /ColorProc\n"
"   {\n"
"      currentfile buffer24 readhexstring pop\n"
"      /data exch def /datalen data length def\n"
"      /cnt 0 def\n"
"      0 1 datalen 3 idiv 1 sub\n"
"      {\n"
"         buffer8 exch\n"
"	 data cnt get 20 mul /cnt cnt 1 add def\n"
"	 data cnt get 32 mul /cnt cnt 1 add def\n"
"	 data cnt get 12 mul /cnt cnt 1 add def\n"
"	 add add 64 idiv put\n"
"      } for\n"
"      buffer8 0 datalen 3 idiv getinterval\n"
"   } bind def\n"
"   /ColorImage\n"
"   {\n"
"      pop pop image\n"
"   } bind def\n"
"} ifelse\n"
"\n");
      } // if (do_color)
   } // if (not do_level2)

   write(str, "%%%%EndSetup\n\n");
}

void
DjVuToPS::store_doc_trailer(ByteStream & str)
{
   DEBUG_MSG("DjVuToPS::store_doc_trailer(): Storing document trailer\n");
   write(str, 
"%%%%Trailer\n"
"\n"
"%% Stop using temporary dictionary\n"
"end\n"
"\n"
"%% Restore original state\n"
"doc-origstate restore\n"
"\n"
"%%%%EOF\n");
}

// ***********************************************************************
// ***************************** PAGE LEVEL ******************************
// ***********************************************************************

unsigned char *
DjVuToPS::ASCII85_encode(unsigned char * dst, const unsigned char * src_start,
			 const unsigned char * src_end)
      // This is a static function
{
   int symbols=0;
   const unsigned char * ptr;
   for(ptr=src_start;ptr<src_end;ptr+=4)
   {
      unsigned int num=0;
      if (ptr+3<src_end)
      {
	 num|=ptr[0]; num<<=8;
	 num|=ptr[1]; num<<=8;
	 num|=ptr[2]; num<<=8;
	 num|=ptr[3];
      }
      else
      {
	 num|=ptr[0]; num<<=8;
	 if (ptr+1<src_end) num|=ptr[1]; num<<=8;
	 if (ptr+2<src_end) num|=ptr[2]; num<<=8;
      }
      int a1, a2, a3, a4, a5;
      a5=num % 85; num/=85;
      a4=num % 85; num/=85;
      a3=num % 85; num/=85;
      a2=num % 85;
      a1=num / 85;
      *dst++=a1+33;
      *dst++=a2+33;
      *dst++=a3+33;
      *dst++=a4+33;
      *dst++=a5+33;
      symbols+=5;
      if (symbols>70) { *dst++='\n'; symbols=0; }
   }
   return dst;
}

unsigned char *
DjVuToPS::RLE_encode(unsigned char * dst,
		     const unsigned char * src_start,
		     const unsigned char * src_end)
      // This is a static function
{
   const unsigned char * ptr;
   for(ptr=src_start;ptr<src_end;ptr++)
   {
      if (ptr==src_end-1)
      {
	 *dst++=0; *dst++=*ptr;
      } else if (ptr[0]!=ptr[1])
      {
	    // Guess how many non repeating bytes we have
	 const unsigned char * ptr1;
	 for(ptr1=ptr+1;ptr1<src_end-1;ptr1++)
	    if (ptr1[0]==ptr1[1] || ptr1-ptr>=128) break;
	 int pixels=ptr1-ptr;
	 *dst++=pixels-1;
	 for(int cnt=0;cnt<pixels;cnt++)
	    *dst++=*ptr++;
	 ptr--;
      } else
      {
	    // Get the number of repeating bytes
	 const unsigned char * ptr1;
	 for(ptr1=ptr+1;ptr1<src_end-1;ptr1++)
	    if (ptr1[0]!=ptr1[1] || ptr1-ptr+1>=128) break;
	 int pixels=ptr1-ptr+1;
	 *dst++=257-pixels;
	 *dst++=*ptr;
	    
	 ptr=ptr1;
      }
   }
   return dst;
}

#define COLOR_TO_GRAY(r, g, b) (((r)*20+(g)*32+(b)*12)/64)

void
DjVuToPS::store_page_setup(ByteStream & str, int page_num,
			   int dpi, const GRect & grect)
{
   write(str,
	 "%%%%Page: %d %d\n"
	 "%%%%BeginPageSetup\n"
	 "/page-origstate save def\n\n",
	 page_num+1,
	 page_num+1);

   if (options.get_format()==Options::EPS)
   {
      write(str, 
"%% Creating dummy unary matrix to be used in all transformations\n"
"/a11 1 def\n"
"/a12 0 def\n"
"/a13 0 def\n"
"/a21 0 def\n"
"/a22 -1 def\n"
"/a23 %d def\n\n", grect.height());
   }else
   {
      write(str, 
"%% Coordinate system positioning\n"
"\n"
"/portrait %s def	%% Specifies image orientation\n"
"/fit-page %s def	%% If true, the image will be scaled to fit the page\n"
"/zoom %d def		%% Zoom factor in percents used to pre-scale image\n"
"/image-dpi %d def\n"
"clippath pathbbox\n"
"2 index sub exch\n"
"3 index sub\n"
"/page-width  exch def\n"
"/page-height exch def\n"
"/page-y exch def\n"
"/page-x exch def\n"
"\n"
"/image-x %d def\n"
"/image-y %d def\n"
"/image-width  %d def\n"
"/image-height %d def\n\n",
	    options.get_orientation()==Options::PORTRAIT ? "true" : "false",
	    options.get_zoom()==Options::FIT_PAGE ? "true" : "false",
	    options.get_zoom(),
	    dpi, 0/*grect.xmin*/, 0/*grect.ymin*/,
	    grect.width(), grect.height());

      write(str, 
"portrait\n"
"{\n"
"    %% Portrait orientation\n"
"    \n"
"    fit-page\n"
"    {\n"
"       image-height page-height div\n"
"       image-width page-width div\n"
"       gt\n"
"       {\n"
"	  page-height image-height div /coeff exch def\n"
"       }\n"
"       {\n"
"	  page-width image-width div /coeff exch def\n"
"       } ifelse\n"
"    }"
"    {\n"
"       /coeff 72 image-dpi div zoom mul 100 div def\n"
"    } ifelse\n"
"    /start-x page-x page-width image-width coeff mul sub 2 div add def\n"
"    /start-y page-y page-height add page-height image-height coeff mul sub 2 div sub def\n"
"    /a11 coeff def\n"
"    /a12 0 def\n"
"    /a13 start-x image-x coeff mul sub def\n"
"    /a21 0 def\n"
"    /a22 coeff neg def\n"
"    /a23 start-y image-y coeff neg mul sub def\n"
"}\n"
"{\n"
"    %% Landscape orientation\n"
"    \n"
"    fit-page\n"
"    {\n"
"       image-height page-width div\n"
"       image-width page-height div\n"
"       gt\n"
"       {\n"
"	  page-width image-height div /coeff exch def\n"
"       }\n"
"       {\n"
"	  page-height image-width div /coeff exch def\n"
"       } ifelse\n"
"    }\n"
"    {\n"
"       /coeff 72 image-dpi div zoom mul 100 div def\n"
"    } ifelse\n"
"    /start-x page-x page-width add page-width image-height coeff mul sub 2 div sub def\n"
"    /start-y page-y page-height add page-height image-width coeff mul sub 2 div sub def\n"
"    /a11 0 def\n"
"    /a12 coeff neg def\n"
"    /a13 start-x image-y coeff neg mul sub def\n"
"    /a21 coeff neg def\n"
"    /a22 0 def\n"
"    /a23 start-y image-x coeff neg mul sub def \n"
"} ifelse\n\n");
   }

   write(str,
"%% Creating \"_a\" matrix being reverse of \"a\"\n"
"/det a11 a22 mul a21 a12 mul sub def\n"
"/_a11 a22 det div def\n"
"/_a12 a12 neg det div def\n"
"/_a13 a22 a13 neg mul a12 neg a23 neg mul add det div def\n"
"/_a21 a21 det neg div def\n"
"/_a22 a11 neg det neg div def\n"
"/_a23 a21 a13 neg mul a11 neg a23 neg mul add det neg div def\n"
"\n"
"%% Now matrix \"a\" is suitable for using in \"concat\", while\n"
"%% matrix \"_a\" (being actually reverse of \"a\") can be used in \"image\"\n\n");

   write(str, "%%%%EndPageSetup\n\n");
}

void
DjVuToPS::store_page_trailer(ByteStream & str)
{
   write(str,
	 "%%%%PageTrailer\n"
	 "page-origstate restore\n"
	 "showpage\n\n");
}

void
DjVuToPS::print_image(ByteStream & str, const GP<DjVuImage> & dimg,
		      const GRect & prn_rect, const GRect & img_rect)
{
   DEBUG_MSG("DjVuToPS::print_image(): Printing DjVuImage to a stream\n");
   DEBUG_MAKE_INDENT(3);

   if (!dimg)
      G_THROW("Attempt to print an empty image.");
   if (prn_rect.isempty())
      G_THROW("Attempt to print an empty rectangle.");
   if (img_rect.isempty())
      G_THROW("Attempt to scale image to point while printing.");

   if (prn_progress_cb)
      prn_progress_cb(0, prn_progress_cl_data);
   float print_done=0;

      // Display image
   GP<GPixmap> pm;
   GP<GBitmap> bm;
      
   int band_bytes=125000;
   int band_height=band_bytes/prn_rect.width();
   int buffer_size=band_height*prn_rect.width();
   int ps_chunk_height=30960/prn_rect.width()+1;
      
   if (options.get_level()==2) buffer_size=buffer_size*23/10;
   else buffer_size=buffer_size*21/10;

   bool do_color=options.get_color();
   if (!dimg->is_legal_photo() && !dimg->is_legal_compound() ||
       options.get_mode()==Options::BW) do_color=false;
      
   if (do_color) buffer_size*=3;

   if (options.get_level()==2)
   {
      if (do_color)
	 write(str, 
"%%\n"
"%% Defining buffers\n"
"%%\n"
"/bufferR %d string def\n"
"/bufferG %d string def\n"
"/bufferB %d string def\n"
"\n"
"%%\n"
"%% Drawing image\n"
"%%\n"
"/DeviceRGB setcolorspace\n"
"9 dict\n"
"dup /ImageType 1 put\n"
"dup /Width %d put\n"
"dup /Height %d put\n"
"dup /BitsPerComponent 8 put\n"
"dup /Decode [0 1 0 1 0 1] put\n"
"dup /ImageMatrix [ _a11 _a21 _a12 _a22 _a13 _a23 ] put\n"
"dup /MultipleDataSources true put\n"
"dup /DataSource [ { ReadR } { ReadG } { ReadB } ] put\n"
"dup /Interpolate false put\n"
"image\n",
	       ps_chunk_height*prn_rect.width(),
	       ps_chunk_height*prn_rect.width(),
	       ps_chunk_height*prn_rect.width(),
	       prn_rect.width(), prn_rect.height());
      else
	 write(str,
	       "%%\n"
	       "%% Drawing image\n"
	       "%%\n"
	       "%d %d 8 [ _a11 _a21 _a12 _a22 _a13 _a23 ]\n"
	       "currentfile /ASCII85Decode filter /RunLengthDecode filter image\n",
	       prn_rect.width(), prn_rect.height());
   } else
   {
	 // Not level2
	 
	 // First define the buffers, that have been declared in %%PageSetup
      write(str,
	    "%%\n"
	    "%% Defining buffers\n"
	    "%%\n"
	    "/buffer8 %d string def\n", prn_rect.width());
      if (do_color)
	 write(str, "/buffer24 %d string def\n", 3*prn_rect.width());

	    // Then - draw the image itself
      if (do_color)
      {
	 write(str,
	       "%%\n"
	       "%% Drawing image\n"
	       "%%\n"
	       "%d %d 8 [ _a11 _a21 _a12 _a22 _a13 _a23 ]\n"
	       "{ ColorProc } false 3 ColorImage\n",
	       prn_rect.width(), prn_rect.height());
      } else
	 write(str,
	       "%%\n"
	       "%% Drawing image\n"
	       "%%\n"
	       "%d %d 8 [ _a11 _a21 _a12 _a22 _a13 _a23 ]\n"
	       "{ currentfile buffer8 readhexstring pop } image\n",
	       prn_rect.width(), prn_rect.height());
   }
      
   unsigned char * buffer=0;
   unsigned char * rle_in=0;
   unsigned char * rle_out=0;
   G_TRY {
	 // Start storing image in bands
      if (!(buffer=new unsigned char[buffer_size]) ||
	  !(rle_in=new unsigned char[ps_chunk_height*prn_rect.width()]) ||
	  !(rle_out=new unsigned char[2*ps_chunk_height*prn_rect.width()]))
	 G_THROW("Not enough memory to print the image.");
	 
      unsigned char * rle_out_end=rle_out;
      GRect grectBand=prn_rect;
      grectBand.ymin=grectBand.ymax;
      while(grectBand.ymin>prn_rect.ymin)
      {
	 GP<GPixmap> pm=0;
	 GP<GBitmap> bm=0;
	    
	    // Compute next band
	 grectBand.ymax=grectBand.ymin;
	 grectBand.ymin=grectBand.ymax-band_bytes/grectBand.width();
	 if (grectBand.ymin<prn_rect.ymin)
	    grectBand.ymin=prn_rect.ymin;
	    
	 DEBUG_MSG("printing band (" << grectBand.xmin << ", " <<
		   grectBand.ymin << ", " << grectBand.width() << ", " <<
		   grectBand.height() << ")\n");
	    
	    // Compute information for chosen display mode
	 switch(options.get_mode())
	 {
	    case Options::COLOR:
	       pm=dimg->get_pixmap(grectBand, img_rect, options.get_gamma());
	       break;
	    case Options::BACK:
	       pm=dimg->get_bg_pixmap(grectBand, img_rect, options.get_gamma());
	       break;
	    case Options::FORE:
	       pm=dimg->get_fg_pixmap(grectBand, img_rect, options.get_gamma());
	       break;
	    case Options::BW:	// For compiler not to warn
	       break;
	 }
	 if (!pm) bm=dimg->get_bitmap(grectBand, img_rect, sizeof(int));
	    
	 unsigned char * buf_ptr=buffer;
	 if (options.get_level()==2)
	 {
	    if (pm)
	    {
	       if (do_color)
	       {
		  int y=grectBand.height()-1;
		  while(y>=0)
		  {
		     unsigned char * ptr, * ptr1;
		     int row, y1;
			// Doing R component of current chunk
		     for(row=0, ptr=rle_in, y1=y;row<ps_chunk_height && y1>=0;row++, y1--)
		     {
			GPixel *pix=(*pm)[y1];
			for (int x=grectBand.width(); x>0; x--, pix++)
			   *ptr++=pix->r & 0xfe;
		     }
		     ptr1=RLE_encode(rle_out, rle_in, ptr); *ptr1++=0x80;
		     buf_ptr=ASCII85_encode(buf_ptr, rle_out, ptr1);
		     *buf_ptr++='~'; *buf_ptr++='>'; *buf_ptr++='\n';
			
			// Doing G component of current chunk
		     for(row=0, ptr=rle_in, y1=y;row<ps_chunk_height && y1>=0;row++, y1--)
		     {
			GPixel *pix=(*pm)[y1];
			for (int x=grectBand.width(); x>0; x--, pix++)
			   *ptr++=pix->g & 0xfe;
		     }
		     ptr1=RLE_encode(rle_out, rle_in, ptr); *ptr1++=0x80;
		     buf_ptr=ASCII85_encode(buf_ptr, rle_out, ptr1);
		     *buf_ptr++='~'; *buf_ptr++='>'; *buf_ptr++='\n';
			
			// Doing B component of current chunk
		     for(row=0, ptr=rle_in, y1=y;row<ps_chunk_height && y1>=0;row++, y1--)
		     {
			GPixel *pix=(*pm)[y1];
			for (int x=grectBand.width(); x>0; x--, pix++)
			   *ptr++=pix->b & 0xfe;
		     }
		     ptr1=RLE_encode(rle_out, rle_in, ptr); *ptr1++=0x80;
		     buf_ptr=ASCII85_encode(buf_ptr, rle_out, ptr1);
		     *buf_ptr++='~'; *buf_ptr++='>'; *buf_ptr++='\n';
			
		     y=y1;

		     if (refresh_cb) refresh_cb(refresh_cl_data);
		  } //while (y>=0)
	       } else
	       {
		     // Don't use color
		  int y=grectBand.height()-1;
		  while(y>=0)
		  {
		     unsigned char * ptr=rle_in;
		     for(int row=0;row<ps_chunk_height && y>=0;row++, y--)
		     {
			GPixel *pix=(*pm)[y];
			for (int x=grectBand.width(); x>0; x--, pix++)
			   *ptr++=COLOR_TO_GRAY(pix->r & 0xfe,
						pix->g & 0xfe,
						pix->b & 0xfe);
		     }
		     rle_out_end=RLE_encode(rle_out_end, rle_in, ptr);
		     unsigned char * encode_to=rle_out+(rle_out_end-rle_out)/4*4;
		     int bytes_left=rle_out_end-encode_to;
		     buf_ptr=ASCII85_encode(buf_ptr, rle_out, encode_to);
		     memcpy(rle_out, encode_to, bytes_left);
		     rle_out_end=rle_out+bytes_left;
		     if (refresh_cb) refresh_cb(refresh_cl_data);
		  }
	       }
	    } // if (pm)
	    if (bm)
	    {
		  // Prepare array for gray translations
	       unsigned char gray[256];
	       int grays=bm->get_grays();
	       unsigned long color=0xff0000;
	       int decrement=color/(grays-1);
	       for(int i=0;i<grays;i++)
	       {
		  gray[i]=color>>16;
		  color-=decrement;
	       }
		  
	       int y=grectBand.height()-1;
	       while(y>=0)
	       {
		  unsigned char * ptr=rle_in;
		  for(int row=0;row<ps_chunk_height && y>=0;row++, y--)
		  {
		     unsigned char *pix=(*bm)[y];
		     for (int x=grectBand.width(); x>0; x--, pix++)
			*ptr++=gray[*pix];
		  }
		  rle_out_end=RLE_encode(rle_out_end, rle_in, ptr);
		  unsigned char * encode_to=rle_out+(rle_out_end-rle_out)/4*4;
		  int bytes_left=rle_out_end-encode_to;
		  buf_ptr=ASCII85_encode(buf_ptr, rle_out, encode_to);
		  memcpy(rle_out, encode_to, bytes_left);
		  rle_out_end=rle_out+bytes_left;
		  if (refresh_cb) refresh_cb(refresh_cl_data);
	       }
	    }
	 } else
	 {
	       // Level 1
	    if (pm)
	    {
	       int symbols=0;
	       for (int y=grectBand.height()-1; y>=0; y--)
	       {
		  GPixel *pix=(*pm)[y];
		  for (int x=grectBand.width(); x>0; x--, pix++)
		  {
		     if (do_color)
		     {
			char * data;
			data=bin2hex[pix->r];
			*buf_ptr++=*data++;
			*buf_ptr++=*data;
			data=bin2hex[pix->g];
			*buf_ptr++=*data++;
			*buf_ptr++=*data;
			data=bin2hex[pix->b];
			*buf_ptr++=*data++;
			*buf_ptr++=*data;
			symbols+=6;
		     }
		     else
		     {
			char * data=bin2hex[COLOR_TO_GRAY(pix->r, pix->g, pix->b)];
			*buf_ptr++=*data++;
			*buf_ptr++=*data;
			symbols+=2;
		     }
		     if (symbols>70) { *buf_ptr++='\n'; symbols=0; }
		  }
		  if (refresh_cb) refresh_cb(refresh_cl_data);
	       }
	    } // if (pm)
	    if (bm)
	    {
		  // Prepare array for gray translations
	       unsigned char gray[256];
	       int grays=bm->get_grays();
	       unsigned long color=0xff0000;
	       int decrement=color/(grays-1);
	       for(int i=0;i<grays;i++)
	       {
		  gray[i]=color>>16;
		  color-=decrement;
	       }

	       int symbols=0;
	       for(int y=grectBand.height()-1; y>=0; y--)
	       {
		  unsigned char *pix=(*bm)[y];
		  for (int x=grectBand.width(); x>0; x--, pix++)
		  {
		     char * data=bin2hex[gray[*pix]];
		     *buf_ptr++=*data++;
		     *buf_ptr++=*data;
		     symbols+=2;
		     if (symbols>70) { *buf_ptr++='\n'; symbols=0; }
		  }
		  if (refresh_cb) refresh_cb(refresh_cl_data);
	       }
	    } // if (bm)
	 } // if (level1)
	 str.writall(buffer, buf_ptr-buffer);

	 if (prn_progress_cb)
	 {
	    float done=(double) (prn_rect.ymax-grectBand.ymin)/prn_rect.height();
	    if ((int) (20*print_done)!=(int) (20*done))
	    {
	       print_done=done;
	       prn_progress_cb(done, prn_progress_cl_data);
	    }
	 }
      } // while(grectBand.ymin>grect.ymin)
      if (options.get_level()==1)
      {
	 write(str, "\n");
      } else
	 if (!do_color)
	 {
	       // It was grayscale mode. Write remaining bytes into the file
	       // And close the stream
	    unsigned char * buf_ptr=buffer;
	    *rle_out_end++=0x80;		// Add EOF marker
	    buf_ptr=ASCII85_encode(buf_ptr, rle_out, rle_out_end);
	    *buf_ptr++='~'; *buf_ptr++='>'; *buf_ptr++='\n';
	    str.writall(buffer, buf_ptr-buffer);
	 }
	 
      delete buffer; buffer=0;
      delete rle_in; rle_in=0;
      delete rle_out; rle_out=0;
   } G_CATCH_ALL {
      delete buffer; buffer=0;
      delete rle_in; rle_in=0;
      delete rle_out; rle_out=0;
      G_RETHROW;
   } G_ENDCATCH;
   char frc=options.get_frame() ? ' ' : '%';
   write(str, 
"\n"
"%% Drawing gray rectangle surrounding the image\n"
"%cgsave\n"
"%c[ a11 a21 a12 a22 a13 a23 ] concat\n"
"%c0.7 setgray\n"
"%c1 coeff div setlinewidth\n"
"%c0 0 %d %d rectstroke\n"
"%cgrestore\n\n", frc, frc, frc, frc, frc,
                 prn_rect.width(), prn_rect.height(), frc);

   if (prn_progress_cb)
      prn_progress_cb(1, prn_progress_cl_data);
}

void
DjVuToPS::print(ByteStream & str, const GP<DjVuImage> & dimg,
		const GRect & prn_rect_in, const GRect & img_rect,
		int override_dpi)
{
   DEBUG_MSG("DjVuToPS::print(): Printing DjVu page to a stream\n");
   DEBUG_MAKE_INDENT(3);

   GRect prn_rect;
   prn_rect.intersect(prn_rect_in, img_rect);
   
   DEBUG_MSG("prn_rect=(" << prn_rect.xmin << ", " << prn_rect.ymin << ", " <<
	     prn_rect.width() << ", " << prn_rect.height() << ")\n");
   DEBUG_MSG("img_rect=(" << img_rect.xmin << ", " << img_rect.ymin << ", " <<
	     img_rect.width() << ", " << img_rect.height() << ")\n");

   if (!dimg)
      G_THROW("Attempt to print an empty image.");
   if (prn_rect.isempty())
      G_THROW("Attempt to print an empty rectangle.");
   if (img_rect.isempty())
      G_THROW("Attempt to scale image to point while printing.");

      // Store document-level directives
   store_doc_prolog(str, 1, prn_rect);
   store_doc_setup(str);

      // Setup the page
   int image_dpi;
   if (override_dpi>0) image_dpi=override_dpi;
   else image_dpi=dimg->get_dpi();
   if (image_dpi<=0) image_dpi=300;
   float scale=(float) img_rect.width()/dimg->get_width();
   store_page_setup(str, 0, (int) (image_dpi*scale), prn_rect);

      // Draw the image
   print_image(str, dimg, prn_rect, img_rect);

      // Close the page
   store_page_trailer(str);

      // Close the document
   store_doc_trailer(str);
}

// ***********************************************************************
// *************************** DOCUMENT LEVEL ****************************
// ***********************************************************************

void
DjVuToPS::DecodePort::notify_file_flags_changed(const DjVuFile * source,
						long set_mask, long clr_mask)
      // WARNING! This function is called from another thread
{
   if (set_mask & (DjVuFile::DECODE_OK | DjVuFile::DECODE_FAILED | DjVuFile::DECODE_STOPPED))
   {
      if (source->get_url()==decode_page_url)
      {
	 decode_event_received=true;
	 decode_event.set();
      }
   }
}

void
DjVuToPS::DecodePort::notify_decode_progress(const DjVuPort * source,
					     float done)
      // WARNING! This function is called from another thread
{
   if (source->inherits("DjVuFile"))
   {
      DjVuFile * file=(DjVuFile *) source;
      if (file->get_url()==decode_page_url)
	 if ((int) (decode_done*20)!=(int) (done*20))
	 {
	    decode_done=done;
	    decode_event_received=true;
	    decode_event.set();
	 }
   }
}

void
DjVuToPS::print(ByteStream & str, const GP<DjVuDocument> & doc,
		const char * page_range)
{
   DEBUG_MSG("DjVuToPS::print(): Printing DjVu document to a stream\n");
   DEBUG_MAKE_INDENT(3);

   DEBUG_MSG("page_range='" << page_range << "'\n");

   port=new DecodePort();
   DjVuPort::get_portcaster()->add_route((const DjVuDocument *) doc, port);

   int doc_pages=doc->get_pages_num();
   
   char buffer[128];
   if (!page_range)
   {
      page_range=buffer;
      sprintf(buffer, "1-%d", doc_pages);
   }

      // Allocate the arrays telling what pages to print
   GList<int> pages_todo;

      // Now parse the page_range[] contents
   const char * start=page_range;
   while(true)
   {
      const char * end;
      for(end=start;*end;end++)
	 if (*end==',') break;
      if (end>start)
      {
	 const char * dash;
	 for(dash=start;dash<end;dash++)
	    if (*dash=='-') break;

	 const char * ptr;
	 int start_page=strtol(start, (char **) &ptr, 10);
	 if (ptr<dash || start_page<=0)
	    G_THROW("Illegal page number '"+GString(start, dash-start)+"'");
	 if (start_page>doc_pages)
	    G_THROW("Page number "+GString(start_page)+" is too big.");

	 if (dash<end)
	 {
	    if (dash==end-1)
	       G_THROW("Missing 'To' page number in range '"+
		     GString(start, end-start)+"'");
	    
	    for(ptr=dash+1;ptr<end;ptr++)
	       if (*ptr=='-')
		  G_THROW("Illegal range '"+GString(start, end-start)+"'");
	    
	    int end_page=strtol(dash+1, (char **) &ptr, 10);
	    if (ptr<end || end_page<=0)
	       G_THROW("Illegal page number '"+GString(dash+1, end-dash-1)+"'");

	    if (end_page>doc_pages)
	       G_THROW("Page number "+GString(end_page)+" is too big.");

	    if (start_page<end_page)
	       for(int page_num=start_page;page_num<=end_page;page_num++)
		  pages_todo.append(page_num-1);
	    else
	       for(int page_num=start_page;page_num>=end_page;page_num--)
		  pages_todo.append(page_num-1);
	 } else pages_todo.append(start_page-1);
      }
      if (*end) start=end+1;
      else break;
   }

   if (pages_todo.size()>1 && options.get_format()==Options::EPS)
      G_THROW("Can't print more than one page in EPS format.");
   
   int page_cnt;
   GPosition pos;
   for(pos=pages_todo, page_cnt=0;pos;++pos, page_cnt++)
   {
      int page_num=pages_todo[pos];
      DEBUG_MSG("processing page #" << page_num << "(" << page_cnt <<
		"/" << pages_todo.size() << ")\n");

      port->decode_event_received=false;
      port->decode_done=false;
      
      GP<DjVuFile> djvu_file=doc->get_djvu_file(page_num);
      GP<DjVuImage> dimg;
      if (djvu_file && !djvu_file->is_decode_ok())
      {
	    // This is the best place to call info_cb(). Note, that
	    // get_page() will start decoding if necessary, and will not
	    // return until the decoding is over in a single threaded
	    // environment.
	    // That's why we called get_djvu_file() first.
	 if (info_cb)
	    info_cb(page_num, page_cnt, pages_todo.size(),
		    DECODING, info_cl_data);

	    // Do NOT decode the page syncronously here!!!
	    // The plugin will deadlock otherwise.
	 dimg=doc->get_page(page_num, false);
	 djvu_file=dimg->get_djvu_file();
	 port->decode_page_url=djvu_file->get_url();
	 if (!djvu_file->is_decode_ok())
	 {
	    DEBUG_MSG("decoding this page\n");
	    if (dec_progress_cb)
	       dec_progress_cb(0, dec_progress_cl_data);
	 
	    while(!djvu_file->is_decode_ok())
	    {
	       while(!port->decode_event_received &&
		     !djvu_file->is_decode_ok())
	       {
		  port->decode_event.wait(100);
		  if (refresh_cb) refresh_cb(refresh_cl_data);
	       }
	       port->decode_event_received=false;
	       if (djvu_file->is_decode_failed() ||
		   djvu_file->is_decode_stopped())
		  G_THROW("Failed to decode page "+GString(page_num)+".");
	       if (dec_progress_cb)
		  dec_progress_cb(port->decode_done, dec_progress_cl_data);
	    }

	    if (dec_progress_cb)
	       dec_progress_cb(1, dec_progress_cl_data);
	 }
      } else dimg=doc->get_page(page_num, false);

      if (!dimg)
	 G_THROW("Failed to get image for page "+page_num);
      
      if (info_cb)
	 info_cb(page_num, page_cnt, pages_todo.size(),
		 PRINTING, info_cl_data);

      if (page_cnt==0)
      {
	    // Store document-level PostScript code
	 store_doc_prolog(str, pages_todo.size(),
			  GRect(0, 0, dimg->get_width(), dimg->get_height()));
	 store_doc_setup(str);
      }
   
	 // Setup the page
      int image_dpi=dimg->get_dpi();
      if (image_dpi<=0) image_dpi=300;
      GRect img_rect(0, 0, dimg->get_width(), dimg->get_height());
      store_page_setup(str, page_cnt, image_dpi, img_rect);

	 // Draw the image
      print_image(str, dimg, img_rect, img_rect);

	 // Close the page
      store_page_trailer(str);
   }

      // Close the PostScript document.
   store_doc_trailer(str);
}
