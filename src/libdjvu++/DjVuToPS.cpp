//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.5)
//C- 
//C- Copyright � 2001 LizardTech, Inc. All Rights Reserved.
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
// 
// $Id: DjVuToPS.cpp,v 1.29 2001-07-31 16:15:38 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuToPS.h"
#include "IFFByteStream.h"
#include "BSByteStream.h"
#include "DjVuImage.h"
#include "DjVuText.h"
#include "DataPool.h"
#include "IW44Image.h"
#include "JB2Image.h"
#include "GBitmap.h"
#include "GPixmap.h"
#include "debug.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#ifdef UNIX
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#endif

static const size_t ps_string_size=15000;

// ***************************************************************************
// ****************************** Options ************************************
// ***************************************************************************

DjVuToPS::Options::Options(void)
: format(PS), orientation(PORTRAIT), level(2), mode(COLOR), zoom(FIT_PAGE),
  color(true), calibrate(true), gamma((double)2.2), copies(1), frame(false),
  text(false) {}

void
DjVuToPS::Options::set_format(const Format xformat)
{
  switch(xformat)
  {
  case EPS:
	orientation=PORTRAIT;
	copies=1;
	// fall through
  case PS:
	format=xformat;
	break;
  default:
    G_THROW(ERR_MSG("DjVuToPS.bad_format"));
    break;
  }
}

void
DjVuToPS::Options::set_level(const int xlevel)
{
  if (xlevel<1 || xlevel>3)
    G_THROW((ERR_MSG("DjVuToPS.bad_level") "\t")+GUTF8String(xlevel));
  level=xlevel;
}

void
DjVuToPS::Options::set_orientation(const Orientation xorientation)
{
  if (xorientation!=PORTRAIT && xorientation!=LANDSCAPE)
    G_THROW(ERR_MSG("DjVuToPS.bad_orient"));
  if (format==EPS && xorientation==LANDSCAPE)
    G_THROW(ERR_MSG("DjVuToPS.no_landscape"));  
  orientation=xorientation;
}

void
DjVuToPS::Options::set_mode(const Mode xmode)
{
  if (xmode!=COLOR && xmode!=FORE && xmode!=BACK && xmode!=BW)
    G_THROW(ERR_MSG("DjVuToPS.bad_mode"));
  mode=xmode;
}

void
DjVuToPS::Options::set_zoom(const Zoom xzoom)
{
  if (xzoom!=FIT_PAGE && !(xzoom>=5 && xzoom<=999))
    G_THROW(ERR_MSG("DjVuToPS.bad_zoom"));
  zoom=xzoom;
}

void
DjVuToPS::Options::set_color(const bool xcolor)
{
  color=xcolor;
}

void 
DjVuToPS::Options::set_sRGB(const bool xcalibrate)
{
  calibrate=xcalibrate;
}

void
DjVuToPS::Options::set_gamma(double xgamma)
{
  if  (xgamma<(double)(0.3-0.0001) || xgamma>(double)(5.0+0.0001))
    G_THROW(ERR_MSG("DjVuToPS.bad_gamma"));
  gamma=xgamma;
}

void
DjVuToPS::Options::set_copies(int xcopies)
{
  if (xcopies<=0)
    G_THROW(ERR_MSG("DjVuToPS.bad_number"));
  if (format==EPS && xcopies!=1)
    G_THROW(ERR_MSG("DjVuToPS.bad_EPS_num"));
  copies=xcopies;
}

void
DjVuToPS::Options::set_frame(bool xframe)
{
  frame=xframe;
}

void
DjVuToPS::Options::set_text(bool xtext)
{
  text=xtext;
}

// ***************************************************************************
// ******************************* DjVuToPS **********************************
// ***************************************************************************

char DjVuToPS::bin2hex[256][2];

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
DjVuToPS::write(ByteStream &str, const char *format, ...)
{
  /* Will output the formated string to the specified \Ref{ByteStream}
     like #fprintf# would do it for a #FILE#. */
  va_list args;
  va_start(args, format);
  GUTF8String tmp;
  tmp.format(format, args);
  str.writall((const char *) tmp, tmp.length());
}

// ************************* DOCUMENT LEVEL *********************************

void
DjVuToPS::store_doc_prolog(ByteStream &str, int pages, int dpi, const GRect &grect)
{
  /* Will store the {\em document prolog}, which is basically a
     block of document-level comments in PS DSC 3.0 format.
     @param str Stream where PostScript data should be written
     @param pages Total number of pages
     @param dpi (EPS mode only) 
     @param grect (EPS mode only) */
  DEBUG_MSG("DjVuToPS::store_doc_prolog(): storing the document prolog\n");
  DEBUG_MAKE_INDENT(3);
  if (options.get_format()==Options::EPS)
    write(str,
          "%%!PS-Adobe-3.0 EPSF-3.0\n"
          "%%%%BoundingBox: 0 0 %d %d\n",
          (grect.width()*100+dpi-1)/dpi, 
          (grect.height()*100+dpi-1)/dpi );
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
        "%%%%Copyright: Copyright (c) 1998-1999 AT&T\n"
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
  if (options.get_level()<2 && options.get_color())
    write(str, "%%%%Extensions: CMYK\n");
  if (options.get_level()>=2)
    write(str,
          "%%%%DocumentNeededFonts: Helvetica\n"
          "%%%%DocumentFonts: Helvetica\n");
  write(str,
        "%%%%EndComments\n"
        "%%%%EndProlog\n"
        "\n");
}

void
DjVuToPS::store_doc_setup(ByteStream &str)
{
  /* Will store the {\em document setup}, which is a set of
     PostScript commands and functions used to inspect and prepare
     the PostScript interpreter environment before displaying images. */
  DEBUG_MSG("DjVuToPS::store_doc_setup(): storing the document setup\n");
  DEBUG_MAKE_INDENT(3);
  write(str,
        "%%%%BeginSetup\n"
        "%% Change this number if you want more than one copy\n"
        "/#copies %d def\n"
        "%% Remember the original state\n"
        "/doc-origstate save def\n"
        "\n", options.get_copies());
  write(str, 
        "%% rectstroke & rectfill emulation (in case if we have level 1)\n"
        "/level 1 def\n"
        "/languagelevel where { pop /level languagelevel def} if\n"
        "level 2 lt {\n"
        "   /rectfill %% stack : x y width height => None\n"
        "   { newpath 4 2 roll moveto 1 index 0 rlineto\n"
        "   0 exch rlineto neg 0 rlineto closepath fill\n"
        "   } bind def\n"
        "   /rectstroke  %% stack : x y width height => None\n"
        "   { newpath 4 2 roll moveto 1 index 0 rlineto\n"
        "   0 exch rlineto neg 0 rlineto closepath stroke\n"
        "   } bind def\n"
        "} if\n");
  
  if (options.get_level()>=2)
    {
      write(str,
            "%% Define function for displaying error messages\n"
            "/msg_y 0 def\n"
            "/E { msg_y 0 eq {\n"
            "      /Helvetica findfont 12 scalefont setfont\n"
            "      /msg_y page-y page-height 2 div add def\n"
            "   } if\n"
            "   dup stringwidth pop\n"
            "   page-x page-width 2 div add exch 2 div sub\n"
            "   msg_y moveto show\n"
            "   /msg_y msg_y 15 sub def\n"
            "} bind def\n"
            "%% Determine if the interpreter supports filters\n"
            "systemdict /resourcestatus known {\n"
            "   (ASCII85Decode) /Filter resourcestatus {\n"
            "      pop pop\n"
            "      (RunLengthDecode) /Filter resourcestatus {\n"
            "         pop pop\n"
            "      } {\n"
            "         (Sorry, but the image can not be printed.) E\n"
            "              (This PostScript interpreter does not support RunLengthDecode filter.) E\n"
            "              showpage stop\n"
            "      } ifelse\n"
            "   } {\n"
            "      (Sorry, but the image can not be printed.) E\n"
            "      (This PostScript interpreter does not support ASCII85Decode filter.) E\n"
            "      showpage stop\n"
            "   } ifelse\n"
            "} if\n");
      if (options.get_color())
        write(str, 
              "%% Define procedures for reading color image\n"
              "/readR () def\n"
              "/readG () def\n"
              "/readB () def\n"
              "/ReadData {\n"
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
              "/ReadR {\n"
              "   readR length 0 eq { ReadData } if\n"
              "   readR /readR () def\n"
              "} bind def\n"
              "/ReadG {\n"
              "   readG length 0 eq { ReadData } if\n"
              "   readG /readG () def\n"
              "} bind def\n"
              "/ReadB {\n"
              "   readB length 0 eq { ReadData } if\n"
              "   readB /readB () def\n"
              "} bind def\n");
      
      if (options.get_sRGB())
        {
          write(str,
                "%% See www.srgb.org\n"
                "/DjVuColorSpace [ %s\n"
                "<< /DecodeLMN [\n"
                "      {dup 0.03928 le {12.92321 div}{0.055 add 1.055 div 2.4 exp}ifelse}\n"
                "      bind dup dup ]\n"
                "   /MatrixLMN [\n"
                "      0.412457 0.212673 0.019334\n"
                "      0.357576 0.715152 0.119192\n"
                "      0.180437 0.072175 0.950301 ]\n"
                "   /WhitePoint [ 0.9505 1 1.0890 ] %% D65 \n"
                "   /BlackPoint[0 0 0] >> ] def\n",
                (options.get_color()) 
                ? "/CIEBasedABC" 
                : "/CIEBasedA" );
        }
      else if (options.get_color())
        {
          write(str,"/DjVuColorSpace /DeviceRGB def\n");
        }
      else
        {
          write(str,"/DjVuColorSpace /DeviceGray def\n");
        }
    } 
  else 
    {
      // level<2
      if (options.get_color())
        {
          write(str, 
                "%% Declare buffers for reading image (level1)\n"
                "/buffer8 () def\n"
                "/buffer24 () def\n"
                "%% Provide emulation for colorimage (level1)\n"
                "systemdict /colorimage known {\n"
                "   /ColorProc {\n"
                "      currentfile buffer24 readhexstring pop\n"
                "   } bind def\n"
                "   /ColorImage {\n"
                "      colorimage\n"
                "   } bind def\n"
                "} {\n"
                "   /ColorProc {\n"
                "      currentfile buffer24 readhexstring pop\n"
                "      /data exch def /datalen data length def\n"
                "      /cnt 0 def\n"
                "      0 1 datalen 3 idiv 1 sub {\n"
                "         buffer8 exch\n"
                "                data cnt get 20 mul /cnt cnt 1 add def\n"
                "                data cnt get 32 mul /cnt cnt 1 add def\n"
                "                data cnt get 12 mul /cnt cnt 1 add def\n"
                "                add add 64 idiv put\n"
                "      } for\n"
                "      buffer8 0 datalen 3 idiv getinterval\n"
                "   } bind def\n"
                "   /ColorImage {\n"
                "      pop pop image\n"
                "   } bind def\n"
                "} ifelse\n");
        } // color
    } // level<2
  write(str, 
        "%% Useful functions\n"
        "/g {gsave 0 0 0 0 5 index 5 index setcachedevice\n"
        "    true [1 0 0 1 0 0] 5 4 roll imagemask grestore } bind def\n"
        "/gn {gsave 0 0 0 0 6 index 6 index setcachedevice\n"
        "    true [1 0 0 1 0 0] 3 2 roll 5 1 roll \n"
        "    { 1 sub 0 index 2 add 1 index  1 add roll} imagemask grestore pop} bind def\n"
        "/c {setcolor rmoveto glyphshow} bind def\n"
        "/s {rmoveto glyphshow} bind def\n"
        "/S {rmoveto gsave show grestore} bind def\n"
        "/F {/Helvetica findfont 2 1 roll  scalefont setfont } bind def\n"
        "%%%%EndSetup\n\n");
}

void
DjVuToPS::store_doc_trailer(ByteStream &str)
{
  /* Will store the {\em document trailer}, which is a clean-up code
     used to return the PostScript interpeter back to the state, in which
     it was before displaying this document. */
  DEBUG_MSG("DjVuToPS::store_doc_trailer(): Storing document trailer\n");
  write(str, 
        "%%%%Trailer\n"
        "doc-origstate restore\n"
        "%%%%EOF\n");
}

// ***********************************************************************
// ***************************** PAGE LEVEL ******************************
// ***********************************************************************

unsigned char *
DjVuToPS::ASCII85_encode(unsigned char * dst, const unsigned char * src_start,
                         const unsigned char * src_end)
{
  /* Will read data between #src_start# and #src_end# pointers (excluding byte
     pointed by #src_end#), encode it using {\bf ASCII85} algorithm, and
     output the result into the destination buffer pointed by #dst#.  The
     function returns pointer to the first unused byte in the destination
     buffer. */
  int symbols=0;
  const unsigned char * ptr;
  for(ptr=src_start;ptr<src_end;ptr+=4)
    {
      unsigned int num=0;
      if (ptr+3<src_end)
        {
          num |= ptr[0] << 24; 
          num |= ptr[1] << 16; 
          num |= ptr[2] << 8; 
          num |= ptr[3];
        }
      else
        {
          num |= ptr[0] << 24; 
          if (ptr+1<src_end) 
            num |= ptr[1] << 16; 
          if (ptr+2<src_end) 
            num |= ptr[2] << 8; 
        }
      int a1, a2, a3, a4, a5;
      a5=num % 85; num/=85;
      a4=num % 85; num/=85;
      a3=num % 85; num/=85;
      a2=num % 85;
      a1=num / 85;
      *dst++ = a1+33;
      *dst++ = a2+33;
      if (ptr+1<src_end)
        *dst++ = a3+33;
      if (ptr+2<src_end)
        *dst++ = a4+33;
      if (ptr+3<src_end)
        *dst++ = a5+33;
      symbols += 5;
      if (symbols > 70 && ptr+4<src_end)
        { 
          *dst++='\n'; 
          symbols=0; 
        }
    }
  return dst;
}

unsigned char *
DjVuToPS::RLE_encode(unsigned char * dst,
                     const unsigned char * src_start,
                     const unsigned char * src_end)
{
  /* Will read data between #src_start# and #src_end# pointers (excluding byte
     pointed by #src_end#), RLE encode it, and output the result into the
     destination buffer pointed by #dst#.  #counter# is used to count the
     number of output bytes.  The function returns pointer to the first unused
     byte in the destination buffer. */
  const unsigned char * ptr;
  for(ptr=src_start;ptr<src_end;ptr++)
    {
      if (ptr==src_end-1)
        {
          *dst++=0; *dst++=*ptr;
        } 
      else if (ptr[0]!=ptr[1])
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
        } 
      else
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
DjVuToPS::store_page_setup(ByteStream &str, int page_num,
                           int dpi, const GRect &grect)
{
  /* Will store PostScript code necessary to prepare page for
     the coming \Ref{DjVuImage}. This is basically a scaling
     code plus initialization of some buffers. */
  write(str,
        "%%%%Page: %d %d\n"
        "%%%%BeginPageSetup\n"
        "/page-origstate save def\n"
        "%% Coordinate system positioning\n", 
        page_num+1, page_num+1);
  if (options.get_format()==Options::EPS)
    write(str, 
          "/image-dpi %d def\n"
          "/image-x %d def\n"
          "/image-y %d def\n"
          "/image-width  %d def\n"
          "/image-height %d def\n\n"
          "/coeff 100 image-dpi div def\n"
          "/a11 coeff def\n"
          "/a12 0 def\n"
          "/a13 0 def\n"
          "/a21 0 def\n"
          "/a22 coeff def\n"
          "/a23 0 def\n",
          dpi, 0, 0, grect.width(), grect.height() );
  else
    {
      write(str, 
            "/portrait %s def        %% Specifies image orientation\n"
            "/fit-page %s def        %% If true, the image will be scaled to fit the page\n"
            "/zoom %d def        %% Zoom factor in percents used to pre-scale image\n"
            "/image-dpi %d def\n"
            "clippath pathbbox\n"
            "2 index sub exch\n"
            "3 index sub\n"
            "/page-width  exch def\n"
            "/page-height exch def\n"
            "/page-y exch def\n"
            "/page-x exch def\n"
            "/image-x %d def\n"
            "/image-y %d def\n"
            "/image-width  %d def\n"
            "/image-height %d def\n\n",
            (options.get_orientation() == Options::PORTRAIT) ? "true" : "false",
            (options.get_zoom() == Options::FIT_PAGE) ? "true" : "false",
            options.get_zoom(), dpi, 0, 0, grect.width(), grect.height());
      write(str, 
            "portrait {\n"
            "  fit-page {\n"
            "    image-height page-height div\n"
            "    image-width page-width div\n"
            "    gt {\n"
            "           page-height image-height div /coeff exch def\n"
            "    } {\n"
            "           page-width image-width div /coeff exch def\n"
            "    } ifelse\n"
            "  } {\n"
            "    /coeff 72 image-dpi div zoom mul 100 div def\n"
            "  } ifelse\n"
            "  /start-x page-x page-width image-width coeff mul sub 2 div add def\n"
            "  /start-y page-y page-height image-height coeff mul sub 2 div add def\n"
            "  /a11 coeff def\n"
            "  /a12 0 def\n"
            "  /a13 start-x def\n"
            "  /a21 0 def\n"
            "  /a22 coeff def\n"
            "  /a23 start-y def\n"
            "} { %% landscape\n"
            "  fit-page {\n"
            "    image-height page-width div\n"
            "    image-width page-height div\n"
            "    gt {\n"
            "           page-width image-height div /coeff exch def\n"
            "    } {\n"
            "           page-height image-width div /coeff exch def\n"
            "    } ifelse\n"
            "  } {\n"
            "    /coeff 72 image-dpi div zoom mul 100 div def\n"
            "  } ifelse\n"
            "  /start-x page-x page-width add page-width image-height coeff mul sub 2 div sub def\n"
            "  /start-y page-y page-height image-width coeff mul sub 2 div add def\n"
            "  /a11 0 def\n"
            "  /a12 coeff neg def\n"
            "  /a13 start-x image-y coeff neg mul sub def\n"
            "  /a21 coeff def\n"
            "  /a22 0 def\n"
            "  /a23 start-y image-x coeff mul add def \n"
            "} ifelse\n");
    }
  write(str, 
        "[a11 a21 a12 a22 a13 a23] concat 0 0 %d %d rectclip\n"
        "%%%%EndPageSetup\n\n", grect.width(), grect.height());
}

void
DjVuToPS::store_page_trailer(ByteStream &str)
{
  /* Will output #showpage# command, and restore PostScript state. */
  write(str,
        "%%%%PageTrailer\n"
        "page-origstate restore\n"
        "showpage\n\n");
}

static int
compute_red(int w, int h, int rw, int rh)
{
  for (int red=1; red<16; red++)
    if (((w+red-1)/red==rw) && ((h+red-1)/red==rh))
      return red;
  return 16;
}

static int
get_bg_red(const GP<DjVuImage> &dimg) 
{
  GP<GPixmap> pm = 0;
  // Access image size
  int width = dimg->get_width();
  int height = dimg->get_height();
  if (width<=0 || height<=0) return 0;
  // CASE1: Incremental BG IW44Image
  GP<IW44Image> bg44 = dimg->get_bg44();
  if (bg44)
    {
      int w = bg44->get_width();
      int h = bg44->get_height();
      // Avoid silly cases
      if (w==0 || h==0 || width==0 || height==0)
        return 0;
      return compute_red(width,height,w,h);
    }
  // CASE 2: Raw background pixmap
  GP<GPixmap>  bgpm = dimg->get_bgpm();
  if (bgpm)
    {
      int w = bgpm->columns();
      int h = bgpm->rows();
      // Avoid silly cases
      if (w==0 || h==0 || width==0 || height==0)
        return 0;
      return compute_red(width,height,w,h);
    }
  return 0;
}

static GP<GPixmap>
get_bg_pixmap(const GP<DjVuImage> &dimg, const GRect &rect)
{
  GP<GPixmap> pm = 0;
  // Access image size
  int width = dimg->get_width();
  int height = dimg->get_height();
  GP<DjVuInfo> info = dimg->get_info();
  if (width<=0 || height<=0 || !info) return 0;
  // CASE1: Incremental BG IW44Image
  GP<IW44Image> bg44 = dimg->get_bg44();
  if (bg44)
    {
      int w = bg44->get_width();
      int h = bg44->get_height();
      // Avoid silly cases
      if (w==0 || h==0 || width==0 || height==0)
        return 0;
      pm = bg44->get_pixmap(1,rect);
      return pm;
    }
  // CASE 2: Raw background pixmap
  GP<GPixmap>  bgpm = dimg->get_bgpm();
  if (bgpm)
    {
      int w = bgpm->columns();
      int h = bgpm->rows();
      // Avoid silly cases
      if (w==0 || h==0 || width==0 || height==0)
        return 0;
      pm->init(*bgpm, rect);
      return pm;
    }
  // FAILURE
  return 0;
}

void 
DjVuToPS::make_gamma_ramp(const GP<DjVuImage> &dimg)
{
  double targetgamma = options.get_gamma();
  double whitepoint = (options.get_sRGB() ? 255 : 280);
  for (int i=0; i<256; i++)
    ramp[i] = i;
  if (! dimg->get_info()) 
    return;
  if (targetgamma < 0.1)
    return;
  double filegamma = dimg->get_info()->gamma;
  double correction = filegamma / targetgamma;
  if (correction<0.1 || correction>10)
    return;
  {
    for (int i=0; i<256; i++)
    {
      double x = (double)(i)/255.0;
      if (correction != 1.0) 
        x = pow(x, correction);        
      int j = (int) floor(whitepoint * x + 0.5);
      ramp[i] = (j>255) ? 255 : (j<0) ? 0 : j;
    }
  }
}

void
DjVuToPS::print_fg_2layer(ByteStream &str, const GP<DjVuImage> &dimg,
                          const GRect &prn_rect, unsigned char *blit_list)
{
  // Pure-jb2 or color-jb2 case.
  GPixel p;
  int currentx=0;
  int currenty=0;
  GP<DjVuPalette>pal = dimg->get_fgbc();
  GP<JB2Image> jb2 = dimg->get_fgjb();
  int num_blits = jb2->get_blit_count();
  int current_blit;
  for(current_blit=0; current_blit<num_blits; current_blit++)
    {
      if (blit_list[current_blit])
        {
          JB2Blit *blit = jb2->get_blit(current_blit);
          if ((pal) && !(options.get_mode()==Options::BW))
            {
              pal->index_to_color(pal->colordata[current_blit], p);
              if (options.get_color())
                {
                  write(str,"/%d %d %d %f %f %f c\n",
                        blit->shapeno, blit->left-currentx, blit->bottom-currenty,
                        ramp[p.r]/255., ramp[p.g]/255., ramp[p.b]/255.);
                } 
              else
                {
                  write(str,"/%d %d %d %f c\n",
                        blit->shapeno, blit->left-currentx, blit->bottom-currenty,
                        ramp[COLOR_TO_GRAY(p.r, p.g, p.b)]/255.);
                }
            }
          else
            {
              write(str,"/%d %d %d s\n", 
                    blit->shapeno, blit->left-currentx, blit->bottom-currenty);
            }
          currentx = blit->left;
          currenty = blit->bottom;
        }
    }
}

void
DjVuToPS::print_fg_3layer(ByteStream &str, const GP<DjVuImage> &dimg,
                          const GRect &cprn_rect, unsigned char *blit_list)
{
  GRect prn_rect;
  GP<GPixmap> brush = dimg->get_fgpm();
  int br = brush->rows();
  int bc = brush->columns();
  int red = compute_red(dimg->get_width(),dimg->get_height(),bc,br);
  prn_rect.ymin = (cprn_rect.ymin)/red;
  prn_rect.xmin = (cprn_rect.xmin)/red;
  prn_rect.ymax = (cprn_rect.ymax+red-1)/red;
  prn_rect.xmax = (cprn_rect.xmax+red-1)/red;
  int color_nb = ((options.get_color()) ? 3 : 1);
  GP<JB2Image> jb2 = dimg->get_fgjb();
  int pw = bc;
  int ph = 2;

  write(str,
        "/P {\n" 
        "  11 dict dup begin 4 1 roll\n"
        "    /PatternType 1 def\n"
        "    /PaintType 1 def\n"
        "    /TilingType 1 def\n"
        "    /H exch def\n"
        "    /W exch def\n"
        "    /Red %d def\n"
        "    /PatternString exch def\n"
        "    /XStep W Red mul def\n"
        "    /YStep H Red mul def\n"
        "    /BBox [0 0 XStep YStep] def\n"
        "    /PaintProc { begin\n"
        "       Red dup scale\n"
        "       << /ImageType 1 /Width W /Height H\n"
        "          /BitsPerComponent 8 /Interpolate false\n"
        "          /Decode [%s] /ImageMatrix [1 0 0 1 0 0]\n"
        "          /DataSource PatternString >> image\n"
        "       end } bind def\n"
        "     0 0 XStep YStep rectclip\n"
        "     end matrix makepattern\n"
        "  /Pattern setcolorspace setpattern\n"
        "  0 0 moveto\n"
        "} def\n", red, (color_nb == 1) ? "0 1" : "0 1 0 1 0 1" );

  unsigned char *s;
  GPBuffer<unsigned char> gs(s,pw*ph*color_nb);
  unsigned char *s_ascii_encoded;
  GPBuffer<unsigned char> gs_ascii_encoded(s_ascii_encoded,pw*ph*2*color_nb);
    {
      for (int y=prn_rect.ymin; y<prn_rect.ymax; y+=ph)
        for (int x=prn_rect.xmin; x<prn_rect.xmax; x+=pw)
          {
            int w = ((x+pw > prn_rect.xmax) ? prn_rect.xmax-x : pw);
            int h = ((y+ph > prn_rect.ymax) ? prn_rect.ymax-y : ph);
            int currentx = x * red;
            int currenty = y * red;
            // Find first intersecting blit
            int current_blit;
            int num_blits = jb2->get_blit_count();
            GRect rect1(currentx,currenty, w*red, h*red);
            for(current_blit=0; current_blit<num_blits; current_blit++)
              if (blit_list[current_blit])
                {
                  JB2Blit *blit = jb2->get_blit(current_blit);
                  GRect rect2(blit->left, blit->bottom,
                              jb2->get_shape(blit->shapeno).bits->columns(),
                              jb2->get_shape(blit->shapeno).bits->rows());
                  if (rect2.intersect(rect1,rect2)) 
                    break;
                }
            if (current_blit >= num_blits)
              continue;
            // Setup pattern
            write(str,"gsave %d %d translate\n", currentx, currenty);
            write(str,"<~");
            unsigned char *q = s;
            for(int current_row = y; current_row<y+h; current_row++)
              { 
                GPixel *row_pix = (*brush)[current_row];
                for(int current_col = x; current_col<x+w; current_col++)
                  { 
                    GPixel &p = row_pix[current_col];
                    if (color_nb>1)
                      {
                        *q++ = ramp[p.r];
                        *q++ = ramp[p.g];
                        *q++ = ramp[p.b];
                      }
                    else
                      {
                        *q++ = ramp[COLOR_TO_GRAY(p.r,p.g,p.b)];
                      }
                  }
              }
            unsigned char *stop_ascii = ASCII85_encode(s_ascii_encoded,s,s+w*h*color_nb);
            *stop_ascii++='\0';
            write(str,"%s",s_ascii_encoded);
            write(str,"~> %d %d P\n", w, h);
            // Keep performing blits
            for(; current_blit<num_blits; current_blit++)
              if (blit_list[current_blit])
                {
                  JB2Blit *blit = jb2->get_blit(current_blit);
                  GRect rect2(blit->left, blit->bottom,
                              jb2->get_shape(blit->shapeno).bits->columns(),
                              jb2->get_shape(blit->shapeno).bits->rows()); 
                  if (rect2.intersect(rect1,rect2)) 
                    {   
                      write(str,"/%d %d %d s\n",
                            blit->shapeno, blit->left-currentx, blit->bottom-currenty);
                      currentx = blit->left;
                      currenty = blit->bottom;
                    }
                }
            write(str,"grestore\n");
          }
      // Cleanup
    }
}

void
DjVuToPS::print_fg(ByteStream &str, const GP<DjVuImage> &dimg,
                   const GRect &prn_rect)
{
  GP<JB2Image> jb2=dimg->get_fgjb();
  int num_blits = jb2->get_blit_count();
  int num_shapes = jb2->get_shape_count();
  unsigned char *dict_shapes = 0;
  unsigned char *blit_list = 0;
  GPBuffer<unsigned char> gdict_shapes(dict_shapes,num_shapes);
  GPBuffer<unsigned char> gblit_list(blit_list,num_blits);
  for(int i=0; i<num_shapes; i++)
  {
    dict_shapes[i]=0;
  }
  for(int current_blit=0; current_blit<num_blits; current_blit++)
  {
    JB2Blit *blit = jb2->get_blit(current_blit);
    JB2Shape *shape = & jb2->get_shape(blit->shapeno);
    blit_list[current_blit] = 0;
    if (! shape->bits) 
      continue;
    GRect rect2(blit->left, blit->bottom, 
      shape->bits->columns(), shape->bits->rows());
    if (rect2.intersect(rect2, prn_rect))
    {
      dict_shapes[blit->shapeno] = 1;
      blit_list[current_blit] = 1;
    }
  }
  write(str,
    "%% Printing Foreground\n"
    "gsave DjVuColorSpace setcolorspace\n" );
      // Define font
  write(str,
    "/$DjVuLocalFont 7 dict def\n"
    "$DjVuLocalFont begin\n"
    "/FontType 3 def \n"
    "/FontMatrix [1 0 0 1 0 0] def\n"
    "/FontBBox [0 0 1 .5] def\n"
    "/CharStrings %d dict def\n"
    "/Encoding 2 array def\n"
    "0 1 1 {Encoding exch /.notdef put} for \n"
    "CharStrings begin\n"
    "/.notdef {} def\n",
    num_shapes+1);
  for(int current_shape=0; current_shape<num_shapes; current_shape++)
  {
    if (dict_shapes[current_shape])
    {
      JB2Shape *shape = & jb2->get_shape(current_shape);
      GP<GBitmap> bitmap = shape->bits;
      int rows = bitmap->rows();
      int columns = bitmap->columns();
      int nbytes = (columns+7)/8*rows+1;
      int nrows = rows;
      int nstrings=0;
      if (nbytes>ps_string_size)   //max string length
      {
        nrows=ps_string_size/((columns+7)/8);
        nbytes=(columns+7)/8*nrows+1;
      }
      unsigned char *s_start;
      GPBuffer<unsigned char> gs_start(s_start,nbytes);
      unsigned char *s_ascii;
      GPBuffer<unsigned char> gs_ascii(s_ascii,nbytes*2);
      write(str,"/%d {",current_shape);

      unsigned char *s = s_start;
      for(int current_row=0; current_row<rows; current_row++)
      {  
        unsigned char * row_bits = (*bitmap)[current_row];
        unsigned char acc = 0;
        unsigned char mask = 0;
        for(int current_col=0; current_col<columns; current_col++)
        {
          if (mask == 0)
            mask = 0x80;
          if (row_bits[current_col])
            acc |= mask;
          mask >>= 1;
          if (mask == 0)
          {
            *s=acc;
            s++;
            acc = mask = 0;
          }
        }
        if (mask != 0)
        {
          *s=acc;
          s++;
        }
        if (!((current_row+1)%nrows))
        {
          unsigned char *stop_ascii = ASCII85_encode(s_ascii,s_start,s); 
          *stop_ascii++='\0';
          write(str,"<~%s~> ",s_ascii);
          s=s_start;
          nstrings++;
        }
      }
      if (s!=s_start)
      {
        unsigned char *stop_ascii = ASCII85_encode(s_ascii,s_start,s);
        *stop_ascii++='\0';
        write(str,"<~%s~> ",s_ascii);
          nstrings++;
      }
      if (nstrings==1)
        write(str," %d %d g} def\n", columns, rows);                  
      else
        write(str," %d %d %d gn} def\n", columns, rows,nstrings);
    }
  }
  write(str, 
    "end\n"
    "/BuildGlyph {\n"
    "  exch /CharStrings get exch\n"
    "  2 copy known not\n"
    "  {pop /.notdef} if\n"
    "  get exec \n"
    "} bind def\n"
    "end\n"
    "/LocalDjVuFont $DjVuLocalFont definefont pop\n"
    "/LocalDjVuFont findfont setfont\n" );
  write(str,
    "-%d -%d translate\n"
    "0 0 moveto\n",
    prn_rect.xmin, prn_rect.ymin);
  // Print the foreground layer
  if (dimg->get_fgpm() && !(options.get_mode()==Options::BW)) 
    print_fg_3layer(str, dimg, prn_rect, blit_list);
  else
    print_fg_2layer(str, dimg, prn_rect, blit_list);        
  write(str, "/LocalDjVuFont undefinefont grestore\n");
}


void 
DjVuToPS::print_bg(ByteStream &str, const GP<DjVuImage> &dimg,
                   const GRect &cprn_rect)
{
  GP<GPixmap> pm;
  GRect prn_rect;
  double print_done = 0;
  int red = 0;
  write(str, "%% Printing background\n");
  if (! (red = get_bg_red(dimg)))
    return;
  write(str, 
        "gsave -%d -%d translate\n"
        "/bgred %d def bgred bgred scale\n",
        cprn_rect.xmin % red, 
        cprn_rect.ymin % red, 
        red);
  prn_rect.ymin = (cprn_rect.ymin)/red;
  prn_rect.ymax = (cprn_rect.ymax+red-1)/red;
  prn_rect.xmin = (cprn_rect.xmin)/red;
  prn_rect.xmax = (cprn_rect.xmax+red-1)/red;
  // Display image
  int band_bytes = 125000;
  int band_height = band_bytes/prn_rect.width();
  int buffer_size = band_height*prn_rect.width();
  int ps_chunk_height = 30960/prn_rect.width()+1;
  buffer_size = buffer_size*23/10;
  bool do_color = options.get_color();
  if (!dimg->is_legal_photo() && 
      !dimg->is_legal_compound() ||
      options.get_mode()==Options::BW) 
    do_color = false;
  if (do_color) 
    buffer_size *= 3;
  if (do_color)
    write(str, 
          "/bufferR %d string def\n"
          "/bufferG %d string def\n"
          "/bufferB %d string def\n"
          "DjVuColorSpace setcolorspace\n"
          "<< /ImageType 1\n"
          "   /Width %d\n"
          "   /Height %d\n"
          "   /BitsPerComponent 8\n"
          "   /Decode [0 1 0 1 0 1]\n"
          "   /ImageMatrix [1 0 0 1 0 0]\n"
          "   /MultipleDataSources true\n"
          "   /DataSource [ { ReadR } { ReadG } { ReadB } ]\n"
          "   /Interpolate false >> image\n",
          ps_chunk_height*prn_rect.width(),
          ps_chunk_height*prn_rect.width(),
          ps_chunk_height*prn_rect.width(),
          prn_rect.width(), prn_rect.height());
  else
    write(str, 
          "DjVuColorSpace setcolorspace\n"
          "<< /ImageType 1\n"
          "   /Width %d\n"
          "   /Height %d\n"
          "   /BitsPerComponent 8\n"
          "   /Decode [0 1]\n"
          "   /ImageMatrix [1 0 0 1 0 0]\n"
          "   /DataSource currentfile /ASCII85Decode filter /RunLengthDecode filter\n"
          "   /Interpolate false >> image\n",
          prn_rect.width(), prn_rect.height());
  
  unsigned char *buffer;
  GPBuffer<unsigned char> gbuffer(buffer,buffer_size);
  unsigned char *rle_in;
  GPBuffer<unsigned char> grle_in(rle_in,ps_chunk_height*prn_rect.width());
  unsigned char *rle_out;
  GPBuffer<unsigned char> grle_out(rle_out,2*ps_chunk_height*prn_rect.width());
    {
      // Start storing image in bands
      unsigned char * rle_out_end = rle_out;
      GRect grectBand = prn_rect;
      grectBand.ymax = grectBand.ymin;
      while(grectBand.ymax < prn_rect.ymax)
        {
          GP<GPixmap> pm = 0;
          // Compute next band
          grectBand.ymin=grectBand.ymax;
          grectBand.ymax=grectBand.ymin+band_bytes/grectBand.width();
          if (grectBand.ymax>prn_rect.ymax)
            grectBand.ymax=prn_rect.ymax;
          pm = get_bg_pixmap(dimg, grectBand);
          unsigned char *buf_ptr = buffer;
          if (pm)
            {
              if (do_color)
                {
                  int y=0;
                  while(y<grectBand.height())
                    {
                      int row, y1;
                      unsigned char *ptr, *ptr1;
                      // Doing R component of current chunk
                      for (row=0,ptr=rle_in,y1=y; row<ps_chunk_height && y1<grectBand.height(); row++,y1++)
                        {
                          GPixel *pix = (*pm)[y1];
                          for (int x=grectBand.width(); x>0; x--,pix++)
                            *ptr++ = ramp[pix->r];
                        }
                      ptr1 = RLE_encode(rle_out, rle_in, ptr); 
                      *ptr1++ = 0x80;
                      buf_ptr = ASCII85_encode(buf_ptr, rle_out, ptr1);
                      *buf_ptr++ = '~'; *buf_ptr++ = '>'; *buf_ptr++ = '\n';
                      // Doing G component of current chunk
                      for (row=0,ptr=rle_in,y1=y; row<ps_chunk_height && y1<grectBand.height(); row++,y1++)
                        {
                          GPixel *pix = (*pm)[y1];
                          for (int x=grectBand.width(); x>0; x--,pix++)
                            *ptr++ = ramp[pix->g];
                        }
                      ptr1 = RLE_encode(rle_out, rle_in, ptr); 
                      *ptr1++ = 0x80;
                      buf_ptr = ASCII85_encode(buf_ptr, rle_out, ptr1);
                      *buf_ptr++ = '~'; 
                      *buf_ptr++ = '>'; 
                      *buf_ptr++ = '\n';
                      // Doing B component of current chunk
                      for (row=0, ptr=rle_in, y1=y;row<ps_chunk_height && y1<grectBand.height(); row++,y1++)
                        {
                          GPixel *pix = (*pm)[y1];
                          for (int x=grectBand.width(); x>0; x--,pix++)
                            *ptr++ = ramp[pix->b];
                        }
                      ptr1 = RLE_encode(rle_out, rle_in, ptr);
                      *ptr1++ = 0x80;
                      buf_ptr = ASCII85_encode(buf_ptr, rle_out, ptr1);
                      *buf_ptr++ = '~'; 
                      *buf_ptr++ = '>'; 
                      *buf_ptr++ = '\n';
                      y=y1;
                      if (refresh_cb) 
                        refresh_cb(refresh_cl_data);
                    } //while (y>=0)
                } 
              else
                {
                  // Don't use color
                  int y=0;
                  while(y<grectBand.height())
                    {
                      unsigned char *ptr = rle_in;
                      for(int row=0; row<ps_chunk_height && y<grectBand.height(); row++,y++)
                        {
                          GPixel *pix = (*pm)[y];
                          for (int x=grectBand.width(); x>0; x--,pix++)
                            *ptr++ = ramp[COLOR_TO_GRAY(pix->r,pix->g,pix->b)];
                        }
                      rle_out_end = RLE_encode(rle_out_end, rle_in, ptr);
                      unsigned char *encode_to = rle_out+(rle_out_end-rle_out)/4*4;
                      int bytes_left = rle_out_end-encode_to;
                      buf_ptr = ASCII85_encode(buf_ptr, rle_out, encode_to);
                      *buf_ptr++ = '\n';
                      memcpy(rle_out, encode_to, bytes_left);
                      rle_out_end = rle_out+bytes_left;
                      if (refresh_cb) 
                        refresh_cb(refresh_cl_data);
                    }
                }
            } // if (pm)
          str.writall(buffer, buf_ptr-buffer);
          if (prn_progress_cb)
            {
              double done=(double) (grectBand.ymax-prn_rect.ymin)/prn_rect.height();
              if ((int) (20*print_done)!=(int) (20*done))
                {
                  print_done=done;
                  prn_progress_cb(done, prn_progress_cl_data);
                }
            }
        } // while(grectBand.yax<grect.ymax)
      if (! do_color)
        {
          unsigned char * buf_ptr = buffer;
          *rle_out_end++ = 0x80;
          buf_ptr = ASCII85_encode(buf_ptr, rle_out, rle_out_end);
          *buf_ptr++='~'; 
          *buf_ptr++='>'; 
          *buf_ptr++='\n';
          str.writall(buffer, buf_ptr-buffer);
        }
    } 
  //restore the scaling
  write(str, "grestore\n");
}

void
DjVuToPS::print_image_lev1(ByteStream &str, const GP<DjVuImage> &dimg,
                           const GRect &prn_rect)
{         
  double print_done=0;
  GRect all(0,0, dimg->get_width(),dimg->get_height());
  GP<GPixmap> pm;
  GP<GBitmap> bm;
  GRect test(0,0,1,1);
  if (options.get_mode() == Options::FORE)
    pm = dimg->get_fg_pixmap(test, all);
  else if (options.get_mode() == Options::BACK)
    pm = dimg->get_bg_pixmap(test, all);
  else if (options.get_mode() != Options::BW)
    pm = dimg->get_pixmap(test, all);
  if (! pm)
    bm = dimg->get_bitmap(test,all);
  if (! pm && ! bm)
    return;
  write(str,
        "%% Printing image (level=1)\n"
        "gsave\n");
  // Display image
  int band_bytes=125000;
  int band_height = band_bytes/prn_rect.width();
  int buffer_size = band_height*prn_rect.width();
  buffer_size = buffer_size*21/10;
  bool do_color = false;
  bool do_color_or_gray = false;
  if (pm && (options.get_mode() != Options::BW))
    do_color_or_gray = true;
  if (do_color_or_gray && options.get_color())
    do_color = true;
  if (do_color) 
    buffer_size *= 3;
  if (do_color)
    write(str, "/buffer24 %d string def\n", 3*prn_rect.width());
  if (do_color_or_gray)
    write(str, "/buffer8 %d string def\n", prn_rect.width());
  else
    write(str, "/buffer8 %d string def\n", (prn_rect.width()+7)/8);
  if (do_color)
    {
      write(str,
            "%d %d 8 [ 1 0 0 1 0 0 ]\n"
            "{ ColorProc } false 3 ColorImage\n",
            prn_rect.width(), prn_rect.height());
    } 
  else if (do_color_or_gray)
    {
      write(str,
            "%d %d 8 [ 1 0 0 1 0 0 ]\n"
            "{ currentfile buffer8 readhexstring pop } image\n",
            prn_rect.width(), prn_rect.height());
    } 
  else
    {
      write(str,
            "%d %d 1 [ 1 0 0 1 0 0 ]\n"
            "{ currentfile buffer8 readhexstring pop } image\n",
            prn_rect.width(), prn_rect.height());
    }
  unsigned char * buffer;
  GPBuffer<unsigned char> gbuffer(buffer,buffer_size);
    {
      // Start storing image in bands
      GRect grectBand = prn_rect;
      grectBand.ymax = grectBand.ymin;
      while(grectBand.ymax < prn_rect.ymax)
        {
          // Compute next band
          grectBand.ymin = grectBand.ymax;
          grectBand.ymax = grectBand.ymin+band_bytes/grectBand.width();
          if (grectBand.ymax > prn_rect.ymax)
            grectBand.ymax = prn_rect.ymax;
          GRect all(0,0, dimg->get_width(),dimg->get_height());
          pm = 0;
          bm = 0;
          if (do_color_or_gray)
            {
              if (options.get_mode() == Options::FORE)
                pm = dimg->get_fg_pixmap(grectBand, all);
              else if (options.get_mode() == Options::BACK)
                pm = dimg->get_bg_pixmap(grectBand, all);
              else
                pm = dimg->get_pixmap(grectBand, all);
            }
          else 
            {
              bm = dimg->get_bitmap(grectBand, all);
            }
          // Store next band
          unsigned char *buf_ptr = buffer;
          int symbols=0;
          for (int y=0; y<grectBand.height(); y++)
            {
              if (pm && do_color_or_gray)
                {
                  GPixel *pix = (*pm)[y];
                  for (int x=grectBand.width(); x>0; x--, pix++)
                    {
                      if (do_color)
                        {
                          char *data;
                          data = bin2hex[ramp[pix->r]];
                          *buf_ptr++ = data[0];
                          *buf_ptr++ = data[1];
                          data = bin2hex[ramp[pix->g]];
                          *buf_ptr++ = data[0];
                          *buf_ptr++ = data[1];
                          data = bin2hex[ramp[pix->b]];
                          *buf_ptr++ = data[0];
                          *buf_ptr++ = data[1];
                          symbols += 6;
                        }
                      else
                        {
                          char *data;
                          data = bin2hex[ramp[COLOR_TO_GRAY(pix->r, pix->g, pix->b)]];
                          *buf_ptr++ = data[0];
                          *buf_ptr++ = data[1];
                          symbols += 2;
                        }
                      if (symbols>70) 
                        { 
                          *buf_ptr++ = '\n'; 
                          symbols=0; 
                        }
                    }
                }
              else if (bm)
                {
                  unsigned char *pix = (*bm)[y];
                  unsigned char acc = 0;
                  unsigned char mask = 0;
                  char *data;
                  for (int x=grectBand.width(); x>0; x--, pix++)
                    {
                      if (mask == 0)
                        mask = 0x80;
                      if (! *pix)
                        acc |= mask;
                      mask >>= 1;
                      if (mask == 0)
                        {
                          data = bin2hex[acc];
                          acc = 0;
                          *buf_ptr++ = data[0];
                          *buf_ptr++ = data[1];
                          symbols += 2;
                          if (symbols>70) 
                            { 
                              *buf_ptr++ = '\n'; 
                              symbols = 0; 
                            }
                        }
                    }
                  if (mask != 0) 
                    {
                      data = bin2hex[acc];
                      *buf_ptr++ = data[0];
                      *buf_ptr++ = data[1];
                      symbols += 2;
                    }
                }
              if (refresh_cb) 
                refresh_cb(refresh_cl_data);
            }
          str.writall(buffer, buf_ptr-buffer);
          if (prn_progress_cb)
            {
              double done=(double) (grectBand.ymax-prn_rect.ymin)/prn_rect.height();
              if ((int) (20*print_done)!=(int) (20*done))
                {
                  print_done=done;
                  prn_progress_cb(done, prn_progress_cl_data);
                }
            }
        }
      write(str, "\n");
    } 
  write(str, "grestore\n");
}

void
DjVuToPS::print_image_lev2(ByteStream &str, const GP<DjVuImage> &dimg,
                           const GRect &prn_rect)
{         
  double print_done=0;
  GRect all(0,0, dimg->get_width(),dimg->get_height());
  GP<GPixmap> pm;
  GRect test(0,0,1,1);
  if (options.get_mode() == Options::FORE)
    pm = dimg->get_fg_pixmap(test, all);
  else if (options.get_mode() == Options::BACK)
    pm = dimg->get_bg_pixmap(test, all);
  else if (options.get_mode() != Options::BW)
    pm = dimg->get_pixmap(test, all);
  if (! pm)
    return;
  write(str,
        "%% Printing image (level=2)\n"
        "gsave\n");
  // Display image
  int band_bytes=125000;
  int band_height = band_bytes/prn_rect.width();
  int buffer_size = band_height*prn_rect.width();
  int ps_chunk_height = 30960/prn_rect.width()+1;
  buffer_size = buffer_size*21/10 + 32;
  bool do_color = options.get_color();
  if (do_color)
    {
      buffer_size *= 3;
      write(str, 
            "/bufferR %d string def\n"
            "/bufferG %d string def\n"
            "/bufferB %d string def\n"
            "DjVuColorSpace setcolorspace\n"
            "<< /ImageType 1\n"
            "   /Width %d\n"
            "   /Height %d\n"
            "   /BitsPerComponent 8\n"
            "   /Decode [0 1 0 1 0 1]\n"
            "   /ImageMatrix [1 0 0 1 0 0]\n"
            "   /MultipleDataSources true\n"
            "   /DataSource [ { ReadR } { ReadG } { ReadB } ]\n"
            "   /Interpolate false >> image\n",
            ps_chunk_height*prn_rect.width(),
            ps_chunk_height*prn_rect.width(),
            ps_chunk_height*prn_rect.width(),
            prn_rect.width(), prn_rect.height());
    } 
  else
    {
      write(str, 
            "DjVuColorSpace setcolorspace\n"
            "<< /ImageType 1\n"
            "   /Width %d\n"
            "   /Height %d\n"
            "   /BitsPerComponent 8\n"
            "   /Decode [0 1]\n"
            "   /ImageMatrix [1 0 0 1 0 0]\n"
            "   /DataSource currentfile /ASCII85Decode filter /RunLengthDecode filter\n"
            "   /Interpolate false >> image\n",
            prn_rect.width(), prn_rect.height());
    } 
  unsigned char *buffer;
  GPBuffer<unsigned char> gbuffer(buffer,buffer_size);
  unsigned char *rle_in;
  GPBuffer<unsigned char> grle_in(rle_in,ps_chunk_height*prn_rect.width());
  unsigned char *rle_out;
  GPBuffer<unsigned char> grle_out(rle_out,2*ps_chunk_height*prn_rect.width());
    {
      // Start storing image in bands
      unsigned char * rle_out_end = rle_out;
      GRect grectBand = prn_rect;
      grectBand.ymax = grectBand.ymin;
      while(grectBand.ymax < prn_rect.ymax)
        {
          // Compute next band
          grectBand.ymin = grectBand.ymax;
          grectBand.ymax = grectBand.ymin+band_bytes/grectBand.width();
          if (grectBand.ymax > prn_rect.ymax)
            grectBand.ymax = prn_rect.ymax;
          GRect all(0,0, dimg->get_width(),dimg->get_height());
          pm = 0;
          if (options.get_mode() == Options::FORE)
            pm = dimg->get_fg_pixmap(grectBand, all);
          else if (options.get_mode() == Options::BACK)
            pm = dimg->get_bg_pixmap(grectBand, all);
          else
            pm = dimg->get_pixmap(grectBand, all);
          // Store next band
          unsigned char *buf_ptr = buffer;
          if (do_color && pm)
            {
              int y=0;
              while(y<grectBand.height())
                {
                  int row, y1;
                  unsigned char *ptr, *ptr1;
                  // Doing R component of current chunk
                  for (row=0,ptr=rle_in,y1=y; row<ps_chunk_height && y1<grectBand.height(); row++,y1++)
                    {
                      GPixel *pix = (*pm)[y1];
                      for (int x=grectBand.width(); x>0; x--,pix++)
                        *ptr++ = ramp[pix->r];
                    }
                  ptr1 = RLE_encode(rle_out, rle_in, ptr); 
                  *ptr1++ = 0x80;
                  buf_ptr = ASCII85_encode(buf_ptr, rle_out, ptr1);
                  *buf_ptr++ = '~'; *buf_ptr++ = '>'; *buf_ptr++ = '\n';
                  // Doing G component of current chunk
                  for (row=0,ptr=rle_in,y1=y; row<ps_chunk_height && y1<grectBand.height(); row++,y1++)
                    {
                      GPixel *pix = (*pm)[y1];
                      for (int x=grectBand.width(); x>0; x--,pix++)
                        *ptr++ = ramp[pix->g];
                    }
                  ptr1 = RLE_encode(rle_out, rle_in, ptr); 
                  *ptr1++ = 0x80;
                  buf_ptr = ASCII85_encode(buf_ptr, rle_out, ptr1);
                  *buf_ptr++ = '~'; 
                  *buf_ptr++ = '>'; 
                  *buf_ptr++ = '\n';
                  // Doing B component of current chunk
                  for (row=0, ptr=rle_in, y1=y;row<ps_chunk_height && y1<grectBand.height(); row++,y1++)
                    {
                      GPixel *pix = (*pm)[y1];
                      for (int x=grectBand.width(); x>0; x--,pix++)
                        *ptr++ = ramp[pix->b];
                    }
                  ptr1 = RLE_encode(rle_out, rle_in, ptr);
                  *ptr1++ = 0x80;
                  buf_ptr = ASCII85_encode(buf_ptr, rle_out, ptr1);
                  *buf_ptr++ = '~'; 
                  *buf_ptr++ = '>'; 
                  *buf_ptr++ = '\n';
                  y=y1;
                  if (refresh_cb) 
                    refresh_cb(refresh_cl_data);
                } //while (y>=0)
            } 
          else if (pm)
            {
              // Don't use color
              int y=0;
              while(y<grectBand.height())
                {
                  unsigned char *ptr = rle_in;
                  for(int row=0; row<ps_chunk_height && y<grectBand.height(); row++,y++)
                    {
                      GPixel *pix = (*pm)[y];
                      for (int x=grectBand.width(); x>0; x--,pix++)
                        *ptr++ = ramp[COLOR_TO_GRAY(pix->r,pix->g,pix->b)];
                    }
                  rle_out_end = RLE_encode(rle_out_end, rle_in, ptr);
                  unsigned char *encode_to = rle_out+(rle_out_end-rle_out)/4*4;
                  int bytes_left = rle_out_end-encode_to;
                  buf_ptr = ASCII85_encode(buf_ptr, rle_out, encode_to);
                  *buf_ptr++ = '\n';
                  memcpy(rle_out, encode_to, bytes_left);
                  rle_out_end = rle_out+bytes_left;
                  if (refresh_cb) 
                    refresh_cb(refresh_cl_data);
                }
              if (grectBand.ymax >= prn_rect.ymax)
                {
                  *rle_out_end++ = 0x80; // Add EOF marker
                  buf_ptr = ASCII85_encode(buf_ptr, rle_out, rle_out_end);
                  *buf_ptr++ = '~'; 
                  *buf_ptr++ = '>'; 
                  *buf_ptr++ = '\n';
                }
            }
          str.writall(buffer, buf_ptr-buffer);
          if (prn_progress_cb)
            {
              double done=(double) (grectBand.ymax-prn_rect.ymin)/prn_rect.height();
              if ((int) (20*print_done)!=(int) (20*done))
                {
                  print_done=done;
                  prn_progress_cb(done, prn_progress_cl_data);
                }
            }
        }
      write(str, "\n");
    } 
  write(str, "grestore\n");
}

void 
get_anno_sub(IFFByteStream &iff, IFFByteStream &out)
{
  GUTF8String chkid;
  while (iff.get_chunk(chkid))
    {
      if (iff.composite())
        get_anno_sub(iff, out);
      else if (chkid == "ANTa" || chkid == "ANTz" ||
               chkid == "TXTa" || chkid == "TXTz"   )
        {
          out.put_chunk(chkid);
          out.copy(*iff.get_bytestream());
          out.close_chunk();
        }
      iff.close_chunk();
    }
}

GP<ByteStream>
get_anno(GP<DjVuFile> f)
{
  if (! f->anno) 
    {
      GP<ByteStream> bs = f->get_init_data_pool()->get_stream();
      GP<ByteStream> anno = ByteStream::create();
      GP<IFFByteStream> in = IFFByteStream::create(bs);
      GP<IFFByteStream> out = IFFByteStream::create(anno);
      get_anno_sub(*in, *out);
      f->anno = anno;
    }
  f->anno->seek(0);
  return f->anno;
}

GP<DjVuTXT>
get_text(GP<DjVuFile> file)
{ 
  GUTF8String chkid;
  GP<IFFByteStream> iff = IFFByteStream::create(get_anno(file));
  while (iff->get_chunk(chkid))
    {
      if (chkid == "TXTa") 
        {
          const GP<DjVuTXT> txt = DjVuTXT::create();
          txt->decode(iff->get_bytestream());
          return txt;
        }
      else if (chkid == "TXTz") 
        {
          const GP<DjVuTXT> txt = DjVuTXT::create();
          GP<ByteStream> bsiff = BSByteStream::create(iff->get_bytestream());
          txt->decode(bsiff);
          return txt;
        }
      iff->close_chunk();
    }
  return 0;
}

class zone_names_struct 
{
public:
  const char *name;
  DjVuTXT::ZoneType ztype;
  const char separator;
  zone_names_struct(const char xname[],DjVuTXT::ZoneType xztype,const char xseparator)
	  : name(xname), ztype(xztype), separator(xseparator) {}
}; 

static const zone_names_struct zone_names[] = 
{
  zone_names_struct("page",   DjVuTXT::PAGE,0),
  zone_names_struct("column", DjVuTXT::COLUMN,    DjVuTXT::end_of_column ),
  zone_names_struct("region", DjVuTXT::REGION,    DjVuTXT::end_of_region ),
  zone_names_struct("para",   DjVuTXT::PARAGRAPH, DjVuTXT::end_of_paragraph ),
  zone_names_struct("line",   DjVuTXT::LINE,      DjVuTXT::end_of_line ),
  zone_names_struct("word",   DjVuTXT::WORD,      ' '),
  zone_names_struct("char",   DjVuTXT::CHARACTER, 0 ),
  zone_names_struct(0, (DjVuTXT::ZoneType)0 ,0)
};

void
print_c_string(const char *data, int length, ByteStream &out)
{
  out.write(" \"",2);
  while (*data && length>0) 
    {
      int span = 0;
      while (span<length && data[span]>=0x20 && 
             data[span]<0x7f && data[span]!='"' && data[span]!='\\' )
        span++;
      if (span > 0) 
        {
          out.write(data, span);
          data += span;
          length -= span;
        }
      else
        {
          char buffer[5];
          sprintf(buffer,"\\%03o", *data);
          out.write(buffer,4);
          data += 1;
          length -= 1;
        }
    }
  out.write("\"",1);
}

void
print_txt_sub(DjVuTXT &txt, DjVuTXT::Zone &zone, ByteStream &out,int &lastx,int &lasty)
{
  // Zone header
  int zinfo;
  for (zinfo=0; zone_names[zinfo].name; zinfo++)
    if (zone.ztype == zone_names[zinfo].ztype)
      break;  
  // Zone children
  if (zone.children.isempty()) 
    {
      const char *data = (const char*)txt.textUTF8 + zone.text_start;
      int length = zone.text_length;
      if (data[length-1] == zone_names[zinfo].separator)
        length -= 1;
      out.write8('(');
      print_c_string(data,length,out);
      out.write8(')');      
      GUTF8String message;
      int tmpx= zone.rect.xmin-lastx;
      int tmpy= zone.rect.ymin-lasty;
      message.format(" %d %d S \n",
                     tmpx, tmpy);
      lastx=zone.rect.xmin;
      lasty=zone.rect.ymin;
      out.write((const char*)message, message.length());
    }
  else
    {
      if (zone.ztype==DjVuTXT::LINE)
        {
          GUTF8String message;
          message.format("%d F\n",zone.rect.ymax-zone.rect.ymin);
          out.write((const char*)message,message.length());
        }
      for (GPosition pos=zone.children; pos; ++pos)
        print_txt_sub(txt, zone.children[pos], out,lastx,lasty);
    }
}

void
print_txt(const GP<DjVuTXT> &txt, ByteStream &out)
{
  if (txt)
    {
      GUTF8String message("gsave -1 -1 0 0 clip 0 0 moveto\n");
      out.write((const char*)message,message.length());
      int lastx=0;
      int lasty=0;
      print_txt_sub(*txt, txt->page_zone, out,lastx,lasty);
      message="grestore \n";
      out.write((const char*)message,message.length());
    }
}

void
DjVuToPS::print_image(ByteStream &str, const GP<DjVuImage> &dimg,
                      const GRect &prn_rect, const GP<DjVuTXT> &txt)
{
  /* Just outputs the specifies image. The function assumes, that
     all add-ons (like {\em document setup}, {\em page setup}) are
     already there. It will just output the image. Since
     output of this function will generate PostScript errors when
     used without output of auxiliary functions, it should be
     used carefully. */
  DEBUG_MSG("DjVuToPS::print_image(): Printing DjVuImage to a stream\n");
  DEBUG_MAKE_INDENT(3);
  if (!dimg)
    G_THROW(ERR_MSG("DjVuToPS.empty_image"));
  if (prn_rect.isempty())
    G_THROW(ERR_MSG("DjVuToPS.empty_rect"));
  if (prn_progress_cb)
    prn_progress_cb(0, prn_progress_cl_data);
  // Compute information for chosen display mode
  print_txt(txt, str);
  make_gamma_ramp(dimg);
  if (options.get_level() < 2)
    {
      print_image_lev1(str, dimg, prn_rect);
    }
  else if (options.get_level() < 3 && dimg->get_fgpm())
    {
      switch(options.get_mode())
        {
        case Options::COLOR:
        case Options::FORE:
          print_image_lev2(str, dimg, prn_rect);
          break;
        case Options::BW:
          print_fg(str, dimg, prn_rect);
          break;
        case Options::BACK:
          print_bg(str, dimg, prn_rect);
          break;
        }
    }
  else 
    {
      switch(options.get_mode())
        {
        case Options::COLOR:
          print_bg(str, dimg, prn_rect);
          print_fg(str, dimg, prn_rect);
          break;
        case Options::FORE:
        case Options::BW:
          print_fg(str, dimg, prn_rect);
          break;
        case Options::BACK:
          print_bg(str, dimg, prn_rect);
          break;
        }
    }
  if (options.get_frame())
    {
      write(str, 
            "%% Drawing gray rectangle surrounding the image\n"
            "gsave\n"
            "0.7 setgray\n"
            "1 coeff div setlinewidth\n"
            "0 image-height %d -%d rectstroke\n"
            "grestore\n", 
            prn_rect.width(), prn_rect.height());
    }

  if (prn_progress_cb)
    prn_progress_cb(1, prn_progress_cl_data);
}

void
DjVuToPS::print(ByteStream &str, const GP<DjVuImage> &dimg,
                const GRect &prn_rect_in, const GRect &img_rect,
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
    G_THROW(ERR_MSG("DjVuToPS.empty_image"));
  if (prn_rect.isempty())
    G_THROW(ERR_MSG("DjVuToPS.empty_rect"));
  if (img_rect.isempty())
    G_THROW(ERR_MSG("DjVuToPS.bad_scale"));
  GRectMapper mapper;
  mapper.set_input(img_rect);
  GRect full_rect(0, 0, dimg->get_width(), dimg->get_height());
  mapper.set_output(full_rect);
  mapper.map(prn_rect);
  int image_dpi =  dimg->get_dpi();
  if (override_dpi>0) 
    image_dpi = override_dpi;
  if (image_dpi <= 0) 
    image_dpi = 300;
  store_doc_prolog(str, 1, (int)(image_dpi), prn_rect);
  store_doc_setup(str);
  store_page_setup(str, 0, (int)(image_dpi), prn_rect);
  print_image(str, dimg, prn_rect,0);
  store_page_trailer(str);
  store_doc_trailer(str);
}


// ***********************************************************************
// *************************** DOCUMENT LEVEL ****************************
// ***********************************************************************

class DjVuToPS::DecodePort : public DjVuPort
{
protected:
  DecodePort(void);
public:
  static inline GP<DecodePort> create(void) {return new DecodePort; }

  GEvent decode_event;
  bool decode_event_received;
  double decode_done;
  GURL decode_page_url;
  virtual void notify_file_flags_changed(const DjVuFile *source, 
                                         long set_mask, long clr_mask);
  virtual void notify_decode_progress(const DjVuPort *source, 
                                      double done);
};

DjVuToPS::DecodePort::DecodePort(void)
: decode_event_received(false), decode_done((double)0) {}

void
DjVuToPS::DecodePort::notify_file_flags_changed(const DjVuFile * source,
                                                long set_mask, long clr_mask)
{
  // WARNING! This function is called from another thread
  if (set_mask & (DjVuFile::DECODE_OK | DjVuFile::DECODE_FAILED | DjVuFile::DECODE_STOPPED))
    {
      if (source->get_url() == decode_page_url)
        {
          decode_event_received=true;
          decode_event.set();
        }
    }
}

void
DjVuToPS::DecodePort::notify_decode_progress(const DjVuPort * source,
                                             double done)
{
  // WARNING! This function is called from another thread
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
DjVuToPS::print(ByteStream &str, const GP<DjVuDocument> &doc)
{
  GUTF8String dummy;
  print(str,doc,dummy);
}

void
DjVuToPS::print(ByteStream &str, const GP<DjVuDocument> &doc,
				GUTF8String page_range)
{
  DEBUG_MSG("DjVuToPS::print(): Printing DjVu document to a stream\n");
  DEBUG_MAKE_INDENT(3);
  DEBUG_MSG("page_range='" << (const char *)page_range << "'\n");
  port = DecodePort::create();
  DjVuPort::get_portcaster()->add_route((const DjVuDocument *) doc, port);
  int doc_pages = doc->get_pages_num();
  if (!page_range.length())
  {
    page_range.format("1-%d", doc_pages);
  }
  // Allocate the arrays telling what pages to print
  GList<int> pages_todo;
  // Now parse the page_range[] contents
  for(int start=0;start < (int)page_range.length();)
  {
    int end=page_range.search(',',start);
	if(end < 0)
	  end=page_range.length();
    if (end>start)
    {
      int dash=page_range.search('-',start);
	  if(dash < 0)
        dash=end;
      int pos;
      int start_page = page_range.toLong(start,pos,10);
      if (pos<dash || start_page<=0)
        G_THROW((ERR_MSG("DjVuToPS.bad_page") "\t")+page_range.substr(start, dash-start));
      if (start_page > doc_pages)
        G_THROW((ERR_MSG("DjVuToPS.big_page") "\t")+page_range.substr(start_page,-1));
      if (dash<end)
      {
        if (dash == end-1)
          G_THROW((ERR_MSG("DjVuToPS.no_to") "\t")+page_range.substr(start, end-start));
        for(pos=dash+1; pos<end; pos++)
		{
          if (page_range[pos] == '-')
            G_THROW((ERR_MSG("DjVuToPS.bad_range") "\t")+page_range.substr(start, end-start));
		}
        int end_page = page_range.toLong(dash+1,pos, 10);
        if (pos<end || end_page<=0)
          G_THROW((ERR_MSG("DjVuToPS.bad_page") "\t")+page_range.substr(dash+1, end-dash-1));
        if (end_page > doc_pages)
          G_THROW((ERR_MSG("DjVuToPS.big_page") "\t")+page_range.substr(end_page,-1));
        if (start_page < end_page)
		{
          for(int page_num=start_page; page_num<=end_page; page_num++)
            pages_todo.append(page_num-1);
		}else
		{
          for(int page_num=start_page; page_num>=end_page; page_num--)
            pages_todo.append(page_num-1);
		}
      } else
	  {
        pages_todo.append(start_page-1);
	  }
    }
    start=end+1;
  }
  if (pages_todo.size()>1 && options.get_format()==Options::EPS)
    G_THROW(ERR_MSG("DjVuToPS.only_one_page"));
  int page_cnt;
  GPosition pos;
  for(pos=pages_todo, page_cnt=0;pos;++pos, page_cnt++)
    {
      int page_num=pages_todo[pos];
      DEBUG_MSG("processing page #" << page_num << "(" << page_cnt <<
                "/" << pages_todo.size() << ")\n");
      port->decode_event_received=false;
      port->decode_done=0;
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
                    G_THROW((ERR_MSG("DjVuToPS.no_image") "\t")+GUTF8String(page_num));
                  if (dec_progress_cb)
                    dec_progress_cb(port->decode_done, dec_progress_cl_data);
                }
              if (dec_progress_cb)
                dec_progress_cb(1, dec_progress_cl_data);
            }
        } else 
          dimg=doc->get_page(page_num, false);
      if (!dimg)
        G_THROW((ERR_MSG("DjVuToPS.no_image") "\t")+GUTF8String(page_num));
      if (info_cb)
        info_cb(page_num, page_cnt, pages_todo.size(),
                PRINTING, info_cl_data);
      if (page_cnt==0)
        {
          // Store document-level PostScript code
          store_doc_prolog(str, pages_todo.size(), dimg->get_dpi(),
                           GRect(0, 0, dimg->get_width(), dimg->get_height()));
          store_doc_setup(str);
        }
      // Setup the page
      int image_dpi=dimg->get_dpi();
      if (image_dpi<=0) image_dpi=300;
      GRect img_rect(0, 0, dimg->get_width(), dimg->get_height());
      store_page_setup(str, page_cnt, image_dpi, img_rect);
      //get the text
      GP<DjVuTXT>  txt=0;

      if (options.get_text())
          txt = get_text(djvu_file);

      // Draw the image
      print_image(str, dimg, img_rect,txt);
      // Close the page
      store_page_trailer(str);
    }
  // Close the PostScript document.
  store_doc_trailer(str);
}


