//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.  All rights reserved.
//C-
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet website having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: djvutopnm.cpp,v 1.27 1999-11-03 23:31:08 bcr Exp $


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
    \item[-subsample N] Same as above.
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
    Compound djVu files and Photo DjVu files are always rendered as PPM
    files. Bilevel DjVu files are rendered as PBM files if the
    subsampling factor is 1.  Otherwise, they are rendered as PGM files
    because the resolution change gives better results with anti-aliasing.
    Three options alter this default behavior.
    \begin{description}
    \item[-black] Renders only the foreground layer mask.  This mode does not
       work with IW44 files because these files have no foreground layer mask.
       The output file will be a PBM file if the subsampling factor is 1.
       Otherwise the output file will be an anti-aliased PGM file.
    \item[-layer black] Same as above.
    \item[-foreground] Renders only the foreground layer on a white
       background.  This mode works only with Compound DjVu files. The output
       file always is a PPM file.
    \item[-layer foreground] Same as above.
    \item[-background] Renders only the background layer. This mode works only
       with Compound DjVu files and IW44 files. The output file always is a PPM
       file.
    \item[-layer background] Same as above.
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
    \item[-page N] This can be used to decode a specific page of a multipage
       document. Page numbers start from #1#. If this option is omitted,
       #1# is assumed.
    \end{description}

    @memo
    Decodes and renders a DjVu file.
    @author
    Yann Le Cun <yann@research.att.com>\\
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id: djvutopnm.cpp,v 1.27 1999-11-03 23:31:08 bcr Exp $# */
//@{
//@}

#include <stdio.h>
#include "GException.h"
#include "GSmartPointer.h"
#include "GRect.h"
#include "GPixmap.h"
#include "GBitmap.h"
#include "DjVuImage.h"
#include "DjVuDocument.h"
#include "GOS.h"
#include "parseoptions.h"

static double flag_scale = -1;
static int    flag_size = -1;
static int    flag_subsample = -1;
static int    flag_segment = -1;
static int    flag_verbose = 0;
static int    flag_mode = 0;

GRect fullrect;
GRect segmentrect;

static void
convert(const char *from, const char *to, int page_num)
{
  unsigned long start, stop;

  // Create DjVuDocument
  GURL from_url=GOS::filename_to_url(from);
  GP<DjVuDocument> doc=new DjVuDocument;
  doc->init(from_url);
  if (! doc->wait_for_complete_init())
    THROW("Decoding failed. Nothing can be done.");        
  
  // Create DjVuImage
  start=GOS::ticks();
  GP<DjVuImage> dimg=doc->get_page(page_num);
  if (! dimg->wait_for_complete_decode() )
    THROW("Decoding failed. Nothing can be done.");    
  stop=GOS::ticks();

  // Verbose
  if (flag_verbose)
    {
      fprintf(stderr,"%s", (const char*)dimg->get_long_description());
      fprintf(stderr,"Decoding time:    %lu ms\n", stop - start);
    }

  // Check
  DjVuInfo *info = dimg->get_info();
  int colorp = dimg->is_legal_photo();
  int blackp = dimg->is_legal_bilevel();
  int compoundp = dimg->is_legal_compound();
  if (flag_verbose)
    {
      if (compoundp)
        fprintf(stderr, "This is a legal Compound DjVu image\n");
      else if (colorp)
        fprintf(stderr, "This is a legal Photo DjVu image\n");
      else if (blackp)
        fprintf(stderr, "This is a legal Bilevel DjVu image\n");
      // Without included files
      fprintf(stderr, "Direct memory usage is %4.1f Kb\n", 
              (double)(dimg->get_djvu_file()->get_memory_usage())/1024 );
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
      int w = (int)(dimg->get_width() * flag_scale / info->dpi);
      int h = (int)(dimg->get_height() * flag_scale / info->dpi);
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
      bm = dimg->get_bitmap(segmentrect, fullrect);
      break;
    case 'f':
      pm = dimg->get_fg_pixmap(segmentrect, fullrect);
      break;
    case 'b':
      pm = dimg->get_bg_pixmap(segmentrect, fullrect);
      break;
    default:
      pm = dimg->get_pixmap(segmentrect, fullrect);
      if (! pm)
        bm = dimg->get_bitmap(segmentrect, fullrect);
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
  fprintf(stderr,
          "DJVUTOPNM -- DjVu decompression utility\n"
          "  Copyright (c) AT&T 1999.  All rights reserved\n"
          "Usage: djvutopnm [options] [<djvufile> [<pnmfile>]]\n\n"
          "Options:\n"
          "  -verbose           Prints various informational messages.\n"
          "  -scale N           Selects display scale (default: 100%%).\n"
          "  -size  WxH         Selects size of rendered image.\n"
          "  -segment WxH+X+Y   Selects which segment of the rendered image\n"
          "  -black             Only renders the stencil(s).\n"
          "  -foreground        Only renders the foreground layer.\n"
          "  -background        Only renders the background layer.\n"
          "  -layer <layer>     Same as the above three options.\n"
          "  -#                  Subsampling factor from full resolution.\n"
          "  -subsample N       Same as above.\n"
	  "  -page <page>       Decode page <page> (for multipage documents).\n"
          "\n"
          "The output will be a PBM, PGM or PPM file depending of its content."
          "If <pnmfile> is a single dash or omitted, the decompressed image\n"
          "is sent to the standard output.  If <djvufile> is a single dash or\n"
          "omitted, the djvu file is read from the standard input.\n\n");
  exit(1);
}


void
geometry(const char *r, GRect &rect)
{
  int w,h;
  char *s;
  rect.xmin = rect.ymin = 0;
  w = strtol(r, &s,10);
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




static const djvu_option long_options[] = {
{"verbose",0,0,'v'},
{"help",0,0,'h'},
{"scale",1,0,0},
{"size",1,0,0},
{"segment",1,0,0},
{"layer",0,0,0},
{"black",0,0,'s'},
{"foreground",0,0,'f'},
{"background",0,0,'b'},
{"page",1,0,0},
{"subsample",3,0,'1'},
{"subsample",3,0,'2'},
{"subsample",3,0,'3'},
{"subsample",3,0,'4'},
{"subsample",3,0,'5'},
{"subsample",3,0,'6'},
{"subsample",3,0,'7'},
{"subsample",3,0,'8'},
{"subsample",3,0,'9'},
{0,0,0,0}
};


int
main(int argc, char *argv[], char *[])
{
  TRY
    {
      DjVuParseOptions Opts("djvutopnm");
      int nextarg=Opts.ParseArguments(argc,argv,long_options,1)-1;
      argc -= nextarg;
      argv+=nextarg;

      // Process options
      int page_num=-1;
      if(Opts.GetInteger("help",0))
      {
        Opts.perror();
        usage();
      }
      flag_verbose=Opts.GetInteger("verbose",0);
      const char *segment=Opts.GetValue("segment");
      if(segment)
      {
        geometry(segment, segmentrect);
        flag_segment = 1;
      }
      page_num=Opts.GetInteger("page",-1);
      if(page_num != -1)
      {
	if (page_num<=0) THROW("Page number must be positive.");
	page_num--;
      }
      if (page_num<0) page_num=0;

      // We need one and only one of the following three values.  The
      // For that we should use the GetBest() method.  Otherwise we will
      // run into problems if one is defined in a profile, but the other
      // is specified on the command line.
      int duplicates[5];
      duplicates[0]=Opts.GetVarToken("scale");
      duplicates[1]=Opts.GetVarToken("size");
      duplicates[2]=Opts.GetVarToken("subsample");
      duplicates[3]=0;
      switch(Opts.GetBest(duplicates))
      {
        case 0: // scale
        {
          const char *s=Opts.GetValue(duplicates[0]);
          char *r;
          flag_scale = strtod(s,&r);
          if (*r == '%') 
            r++;
          if (*r)
            THROW("Illegal argument for option '-scale'");
          break;
        }
        case 1: // size
        {
          const char *s=Opts.GetValue(duplicates[1]);
          geometry(s, fullrect);
          flag_size = 1;
          if (fullrect.xmin || fullrect.ymin)
            THROW("Illegal -size specification");
          break;
        }
        case 2: // subsample
        {
          int i=Opts.GetInteger(duplicates[2],0);
          if (i<0)
          {
            Opts.perror();
            usage();
          }
          flag_subsample = i;
          break;
        }
        default:  // none of them are defined.
	  break;
      }
      duplicates[0]=Opts.GetVarToken("layer");
      duplicates[1]=Opts.GetVarToken("black");
      duplicates[2]=Opts.GetVarToken("foreground");
      duplicates[3]=Opts.GetVarToken("background");
      duplicates[4]=0;
      switch(Opts.GetBest(duplicates))
      {
        case 0:  // layer
        {
          const char *s=Opts.GetValue(duplicates[0]);
          if(!strcmp(s,"black"))
          {
            flag_mode = 's';
          }else if(!strcmp(s,"foreground"))
          {
            flag_mode = 'f';
          }else if(!strcmp(s,"background"))
          {
            flag_mode = 'b';
          }else
          {
            THROW("Illegal -layer specification.");
          }
          break;
        }
        case 1: // black
          if(Opts.GetInteger(duplicates[1],0))
            flag_mode = 's';
          break;
        case 2: // foreground
          if(Opts.GetInteger(duplicates[2],0))
            flag_mode = 'f';
          break;
        case 3: // background
          if(Opts.GetInteger(duplicates[3],0))
            flag_mode = 'b';
          break;
        default: // none of these options have been specified.
	  break;
      }
      if(Opts.HasError())
      {
        Opts.perror();
        usage();
      }
      // Process remaining arguments
      if (argc == 1) 
        convert("-","-", page_num); 
      else if (argc == 2) 
        convert(argv[1],"-", page_num);
      else if (argc == 3) 
        convert(argv[1],argv[2], page_num);
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


