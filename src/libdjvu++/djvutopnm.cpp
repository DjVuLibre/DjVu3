//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: djvutopnm.cpp,v 1.12 1999-02-18 22:46:07 leonb Exp $


/** @name djvutopnm

    {\bf Synopsis}
    \begin{verbatim}
        djvutopnm [options] djvufilename [pnmfilename]
    \end{verbatim}

    {\bf Description} --- File #"djvutopnm.cpp"# illustrates how to decode and
    render a DjVu file using class #DjVuImage#.  This program decodes all
    variants of DjVu files, including the wavelet files produced by \Ref{c44},
    and produces PNM files (see \Ref{PNM and RLE file formats}).

    {\bf Arguments} --- Argument #djvufilename# is the name of the input DjVu
    file.  A single dash #"-"# represents the standard input.  Argument
    #pnmfilename# is the name of the output PNM file.  Omitting this argument
    or specifying a single dash #"-"# represents the standard output.

    {\bf Output Resolution} --- Three options control the resolution of the 
    output PNM image.  At most one of these three options can be specified.
    The default resolution, used when no other option is specified, is equivalent
    to specifying #-scale 100#.
    \begin{description}
    \item[-N] This option (e.g #"-3"# or #"-19"#) specifies a subsampling
       factor #N#.  Rendering the full DjVu image would create an image whose
       dimensions are #N# times smaller than the DjVu image size.
    \item[-scale N] This option takes advantage of the #dpi# field stored in
       the #"INFO"# chunk of the DjVu image (cf. \Ref{DjVuInfo}).  Argument
       #N# is a magnification percentage relative to the adequate resolution
       for a 100dpi device such as a screen.
    \item[-size WxH] This option provides total control on the resolution and
       the aspect ratio of the image.  The vertical and horizontal resolutions
       will be separately adjusted in such a way that the complete DjVu image
       is rendered into a PNM file of width #W# and height #H#.
    \end{description}
    
    {\bf Rendering Mode Selection} --- The default rendering mode merges all
    the layers of the DjVu image and outputs an adequate PNM file. IW44 files
    and Color DjVu files are always rendered as PPM files. Black and White
    DjVu files are rendered as PBM files if the subsampling factor is 1.
    Otherwise, they are rendered as PGM files because the resolution change
    gives better results with anti-aliasing.  Three options alter this
    default behavior.
    \begin{description}
    \item[-black] Renders only the foreground layer mask.  This mode does not
       work with IW44 files because these files have no foreground layer mask.
       The output file will be a PBM file if the subsampling factor is 1.
       Otherwise the output file will be an anti-aliased PGM file.
    \item[-foreground] Renders only the foreground layer on a white
       background.  This mode works only with Color DjVu files. The output
       file always is a PPM file.
    \item[-background] Renders only the background layer. This mode works only
       with Color DjVu files and IW44 files. The output file always is a PPM
       file.
    \end{description}

    {\bf Other Options} --- The following two options are less commonly used:
    \begin{description}
    \item[-segment WxH+X+Y] Selects an image segment to render. Conceptually,
       #djvutopnm# renders the full page using the specified resolution, and
       then extracts a subimage of width #W# and height #H#, starting at
       position (#X#,#Y#) relative to the bottom left corner of the page.
       Both operations of course happen simultaneously.  Rendering a small
       subimage is much faster than rendering the complete image.  Note that
       the output PNM file will always have size #WxH#.
    \item[-v] Causes #djvutopnm# to print abundant information about the
       structure of the DjVu file, the compression ratios, the memory usage,
       and the decoding and rendering times.
    \end{description}

    @memo
    Decodes and renders a DjVu file.
    @author
    Yann Le Cun <yann@research.att.com>\\
    Leon Bottou <leonb@research.att.com>
    @version
    #$Id: djvutopnm.cpp,v 1.12 1999-02-18 22:46:07 leonb Exp $# */
//@{
//@}

#include <stdio.h>
#include "GException.h"
#include "GSmartPointer.h"
#include "GRect.h"
#include "GPixmap.h"
#include "GBitmap.h"
#include "DjVuImage.h"
#include "GOS.h"


static double flag_scale = -1;
static int    flag_size = -1;
static int    flag_subsample = -1;
static int    flag_segment = -1;
static int    flag_verbose = 0;
static int    flag_mode = 0;

GRect fullrect;
GRect segmentrect;

static void
convert(const char *from, const char *to)
{
  unsigned long start, stop;

  // Open djvu file
  StdioByteStream in (from,"rb");
  // Decode djvu file
  DjVuImage dimg;
  start = GOS::ticks();
  dimg.decode(in);
  stop = GOS::ticks();
  // Verbose
  if (flag_verbose)
    {
      fprintf(stderr,"%s", (const char*)dimg.get_long_description());
      fprintf(stderr,"Decoding time:    %lu ms\n", stop - start);
      fprintf(stderr,"DjVuImage memory: %.1f kB\n", dimg.get_memory_usage()/1024.0);
    }
  // Check
  DjVuInfo *info = dimg.get_info();
  int colorp = dimg.is_legal_color();
  int blackp = dimg.is_legal_bilevel();
  int compoundp = dimg.is_legal_compound();
  if (flag_verbose)
    {
      if (compoundp)
        fprintf(stderr, "This is a legal Compound DjVu image\n");
      else if (colorp)
        fprintf(stderr, "This is a legal Color DjVu image\n");
      else if (blackp)
        fprintf(stderr, "This is a legal Bilevel DjVu image\n");
    }    
  if (!compoundp && !colorp && !blackp)
    { 
      fprintf(stderr,"Warning: This is not a well formed DjVu image\n");
      if (!info)
        THROW("Cannot find INFO chunk. Aborting."); 
    }
  // Setup rectangles
  if (flag_size<0 && flag_scale<0 && flag_subsample<0)
    flag_scale = 100;
  if (flag_subsample>0)
    flag_scale = (double) info->dpi / flag_subsample;
  if (flag_scale>0)
    {
      int w = (int)(dimg.get_width() * flag_scale / info->dpi);
      int h = (int)(dimg.get_height() * flag_scale / info->dpi);
      if (w<1) w=1;
      if (h<1) h=1;
      fullrect = GRect(0,0, w, h);
    }
  if (flag_segment < 0)
    segmentrect = fullrect;
  // Render
  GP<GPixmap> pm;
  GP<GBitmap> bm;
  start = GOS::ticks();
  switch(flag_mode)
    {
    case 's':
      bm = dimg.get_bitmap(segmentrect, fullrect);
      break;
    case 'f':
      pm = dimg.get_fg_pixmap(segmentrect, fullrect);
      break;
    case 'b':
      pm = dimg.get_bg_pixmap(segmentrect, fullrect);
      break;
    default:
      pm = dimg.get_pixmap(segmentrect, fullrect);
      if (! pm)
        bm = dimg.get_bitmap(segmentrect, fullrect);
      break;
    }
  stop = GOS::ticks();
  if (flag_verbose)
    {
      fprintf(stderr,"Rendering time:   %lu ms\n", stop - start);
    }
  // Save image
  if (pm) 
    {
      StdioByteStream out (to,"wb");
      pm->save_ppm(out);
    }
  else if (bm) 
    {
      StdioByteStream out (to,"wb");
      if (bm->get_grays() == 2)
        bm->save_pbm(out);
      else 
        bm->save_pgm(out);
    }
  else 
    {
      THROW("Cannot render the requested image");
    }
}


void
usage()
{
  fprintf(stderr,"%s",
          "DJVUTOPNM -- DjVu decompression utility\n"
          "             [djvutopnm (c) AT&T Labs 1997 (Leon Bottou, HA6156)]\n\n"
          "Usage: djvutopnm [options] [<djvufile> [<pnmfile>]]\n\n"
          "Options:\n"
          "  -v                  Prints various informational messages.\n"
          "  -scale N            Selects display scale (default: 100%).\n"
          "  -size  WxH          Selects size of rendered image.\n"
          "  -segment WxH+X+Y    Selects which segment of the rendered image\n"
          "  -black              Only renders the stencil(s).\n"
          "  -foreground         Only renders the foreground layer.\n"
          "  -background         Only renders the background layer.\n"
          "  -N                  Subsampling factor from full resolution.\n"
          "\n"
          "The output will be a PBM, PGM or PPM file depending of its content."
          "If <pnmfile> is a single dash or omitted, the decompressed image\n"
          "is sent to the standard output.  If <djvufile> is a single dash or\n"
          "omitted, the djvu file is read from the standard input.\n\n"
          );
  exit(1);
}


void
geometry(char *s, GRect &rect)
{
  int w,h;
  rect.xmin = rect.ymin = 0;
  w = strtol(s, &s,10);
  if (w<=0 || *s++!='x') goto error;
  h = strtol(s,&s,10);
  if (h<=0 || (*s && *s!='+' && *s!='-')) goto error;
  if (*s) 
    rect.xmin = strtol(s,&s,10);
  if (*s)
    rect.ymin = strtol(s,&s,10);
  if (*s) 
    {
    error:
      THROW("Syntax error in geometry specification");
    }
  rect.xmax = rect.xmin + w;
  rect.ymax = rect.ymin + h;
}





int
main(int argc, char **argv)
{
  TRY
    {
      // Process options
      while (argc>1 && argv[1][0]=='-' && argv[1][1])
        {
          char *s = argv[1];
          if (!strcmp(argv[1],"-v"))
            {
              flag_verbose = 1;
            }
          else if (!strcmp(s,"-scale"))
            {
              if (argc<=2)
                THROW("No argument for option '-scale'");
              if (flag_subsample>=0 || flag_scale>=0 || flag_size>=0)
                THROW("Duplicate scaling specification");
              argc -=1; argv +=1; s = argv[1];
              flag_scale = strtod(s,&s);
              if (*s == '%') 
                s++;
              if (*s)
                THROW("Illegal argument for option '-scale'");
            }
          else if (! strcmp(s,"-size"))
            {
              if (argc<=2)
                THROW("No argument for option '-size'");
              if (flag_subsample>=0 || flag_scale>=0 || flag_size>=0)
                THROW("Duplicate scaling specification");
              argc -=1; argv +=1; s = argv[1];
              geometry(s, fullrect);
              flag_size = 1;
              if (fullrect.xmin || fullrect.ymin)
                THROW("Illegal size specification");
            }
          else if (! strcmp(s,"-segment"))
            {
              if (argc<=2)
                THROW("No argument for option '-segment'");
              if (flag_segment>=0)
                THROW("Duplicate segment specification");
              argc -=1; argv +=1; s = argv[1];
              geometry(s, segmentrect);
              flag_segment = 1;
            }
          else if (! strcmp(s,"-black"))
            {
              if (flag_mode)
                THROW("Duplicate rendering mode specification");
              flag_mode = 's';
            }
          else if (! strcmp(s,"-foreground"))
            {
              if (flag_mode)
                THROW("Duplicate rendering mode specification");
              flag_mode = 'f';
            }
          else if (! strcmp(s,"-background"))
            {
              if (flag_mode)
                THROW("Duplicate rendering mode specification");
              flag_mode = 'b';
            }
          else if (s[1]>='1' && s[1]<='9')
            {
              int arg = strtol(s+1,&s,10);
              if (arg<0 || *s) usage();
              if (flag_subsample>=0 || flag_scale>=0 || flag_size>=0)
                THROW("Duplicate scaling specification");
              flag_subsample = arg;
            }
          else
            {
              usage();
            }
          argc -= 1;
          argv += 1;
        }
      // Process remaining arguments
      if (argc == 1) 
        convert("-","-"); 
      else if (argc == 2) 
        convert(argv[1],"-");
      else if (argc == 3) 
        convert(argv[1],argv[2]);
      else
        usage();
    }
  CATCH(ex)
    {
      ex.perror("Exception while executing DJVUTOPNM");
      exit(1);
    }
  ENDCATCH;
  return 0;
}


