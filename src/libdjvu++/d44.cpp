//C-  -*- C++ -*-
//C-
//C- This software contains software code or other information proprietary to
//C- AT&T Corp. ("AT&T").  Unless you accept a license to use this software,
//C- you shall not use, execute, compile, modify, redistribute, reverse
//C- compile, disassemble, or otherwise reverse engineer the AT&T software or
//C- any derived work of the AT&T software.  The text of a license can be
//C- found in file "ATTLICENSE" or at the Internet website having the URL
//C- "http://www.djvu.att.com/open".
//C-
//C- This software is provided to you "AS IS".  YOU "AS IS".  YOU ASSUME TOTAL
//C- RESPONSIBILITY AND RISK FOR YOUR USE OF THEM INCLUDING THE RISK OF ANY
//C- DEFECTS OR INACCURACIES THEREIN.  AT&T DOES NOT MAKE, AND EXPRESSLY
//C- DISCLAIMS, ANY EXPRESS OR IMPLIED WARRANTIES OF ANY KIND WHATSOEVER,
//C- INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//C- OR FITNESS FOR A PARTICULAR PURPOSE, WARRANTIES OF TITLE OR
//C- NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS OR TRADEMARK RIGHTS,
//C- ANY WARRANTIES ARISING BY USAGE OF TRADE, COURSE OF DEALING OR COURSE OF
//C- PERFORMANCE, OR ANY WARRANTY THAT THE AT&T SOURCE CODE RELEASE OR AT&T
//C- CAPSULE ARE "ERROR FREE" WILL MEET YOUR REQUIREMENTS.
//C-
//C-     (C) AT&T Corp. All rights reserved.
//C-     AT&T is a registered trademark of AT&T Corp.
//C-
//C- $Id: d44.cpp,v 1.12 1999-03-16 20:21:30 leonb Exp $



/** @name d44

    {\bf Synopsis}\\
    \begin{verbatim}
        d44 [-verbose] [-chunks n] iw44file [pnmfile]
    \end{verbatim}

    {\bf Description} ---
    File #"d44.cpp"# illustrates the use of classes \Ref{IWBitmap} and
    \Ref{IWPixmap} for decompressing a color image or a gray level 
    encoded using DjVu IW44 wavelets.  Such files are typically created
    using program \Ref{c44}. 

    {\bf Arguments} --- Argument #iw4file# is the name of the input file
    containing IW44 encoded data.  These files usually have suffix #".djvu"#,
    #".djv"#, #".iw44"# or #".iw4"#.  The output file will be a PPM file for a
    color image or a PGM file for a gray level image.  These formats can be
    converted into other file formats using the NetPBM package
    (\URL{http://www.arc.umn.edu/GVL/Software/netpbm.html}) or the ImageMagick
    package (\URL{http://www.wizards.dupont.com/cristy}).  Argument #pnmfile#
    is the name of the output file.  A single dash #"-"# can be used to
    represent the standard output.  If this argument is omitted, a filename is
    generated by replacing the suffix of #iw4file# with suffix #".pgm"# or
    #".ppm"#.

    {\bf Options} ---
    The following options are recognized.
    \begin{description}
    \item[-chunks n]
    Decodes only the first #n# chunks of the file.
    This option allows you to render the successive images displayed
    while downloading this IW44 file into a web browser.
    \item[-verbose]
    Prints a message describing the decompression times and the memory
    requirements.  Decompression is a two stage process. The IW44 data is
    first decoded and the wavelet coefficients are stored into a memory
    efficient data structure. This data structure is then used to render the
    final image.  
    \begin{verbatim}
    % d44 -v lag.djvu - | xv -
    image: color 510 x 684
    times: 421ms (decoding) + 340ms (rendering)
    memory: 678kB (24% active coefficients)
    \end{verbatim}
    \end{description}

    @memo
    DjVu IW44 wavelet decoder.
    @author
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id: d44.cpp,v 1.12 1999-03-16 20:21:30 leonb Exp $# 
*/
//@{
//@}

#include <stdio.h>
#include "GString.h"
#include "GException.h"
#include "IWImage.h"
#include "GOS.h"
#include "ATTLicense.h"


// global data

int flag_verbose = 0;
int flag_chunks = 9999;
int flag_addsuffix = 0;
GString pnmfile;
GString iw4file;



// parse arguments

void 
usage()
{
  printf("D44 -- Image decompression utility using Interpolating Wavelets (4,4)\n"
         "%s\n"
         "Usage: d44 [options] iw4file [pnmfile]\n"
         "Options:\n"
         "    -verbose     -- report decoding time and memory use\n"
         "    -chunks n    -- select number of chunks to decode\n"
         "\n", ATTLicense::get_usage_text());
  exit(1);
}


void
parse(int argc, char **argv)
{
  for (int i=1; i<argc; i++)
    {
      if (argv[i][0] == '-' && argv[i][1])
        {
          if (argv[i][1] == 'v')
            {
              flag_verbose = 1;
            }
          else if (argv[i][1] == 'c')
            {
              if (++i >= argc)
                THROW("d44: missing argument after option -chunks");
              char *ptr;
              flag_chunks = strtol(argv[i], &ptr, 10);
              if (*ptr || flag_chunks<=0 || flag_chunks>999)
                THROW("d44: illegal argument after option -chunks");
            }
          else
            usage();
        }
      else if (!iw4file)
        iw4file = argv[i];
      else if (!pnmfile)
        pnmfile = argv[i];
      else
        usage();
    }
  if (!iw4file)
    usage();
  if (!pnmfile)
    {
      GString dir = GOS::dirname(iw4file);
      GString base = GOS::basename(iw4file);
      int dot = base.rsearch('.');
      if (dot >= 1)
        base = base.substr(0,dot);
      pnmfile = GOS::expand_name(base,dir);
      flag_addsuffix = 1;
    }
}


int
main(int argc, char **argv)
{
  TRY
    {
      // Parse arguments
      ATTLicense::process_cmdline(argc,argv);
      parse(argc, argv);
      // Check input file
      StdioByteStream ibs(iw4file,"rb");
      GString chkid;
      // Determine file type
      { 
        IFFByteStream iff(ibs);
        if (! iff.get_chunk(chkid))
          THROW("d44: malformed IW4 file");
        ibs.seek(0);
      }
      // Go decoding
      if (chkid == "FORM:BM44")
        {
          IFFByteStream iff(ibs);
          IWBitmap iw;
          int stime = GOS::ticks();
          iw.decode_iff(iff, flag_chunks);
          int dtime = GOS::ticks() - stime;
          GP<GBitmap> pbm = iw.get_bitmap();
          int rtime = GOS::ticks() - stime - dtime;
          if (flag_verbose)
            fprintf(stderr,
                    "image: gray %d x %d\n"
                    "times: %dms (decoding) + %dms (rendering)\n"
                    "memory: %dkB (%d%% active coefficients)\n",
                    iw.get_width(), iw.get_height(), dtime, rtime, 
                    (iw.get_memory_usage()+512)/1024, iw.get_percent_memory());
          if (flag_addsuffix)
            pnmfile = pnmfile + ".pgm";
          remove(pnmfile);
          StdioByteStream obs(pnmfile,"wb");
          pbm->save_pgm(obs);
        }
      else if (chkid == "FORM:PM44")
        {
          IFFByteStream iff(ibs);
          IWPixmap iw;
          int stime = GOS::ticks();
          iw.decode_iff(iff, flag_chunks);
          int dtime = GOS::ticks() - stime;
          GP<GPixmap> ppm = iw.get_pixmap();
          int rtime = GOS::ticks() - stime - dtime;
          if (flag_verbose)
            fprintf(stderr,
                    "image: color %d x %d\n"
                    "times: %dms (decoding) + %dms (rendering)\n"
                    "memory: %dkB (%d%% active coefficients)\n",
                    iw.get_width(), iw.get_height(), dtime, rtime, 
                    (iw.get_memory_usage()+512)/1024, iw.get_percent_memory());
          if (flag_addsuffix)
            pnmfile = pnmfile + ".ppm";
          remove(pnmfile);
          StdioByteStream obs(pnmfile,"wb");
          ppm->save_ppm(obs);
        }
      else
        {
          THROW("d44: expected BM44 or PM44 chunk in IW4 file");
        }
    }
  CATCH(ex)
    {
      ex.perror("Exception while executing D44");
      exit(1);
    }
  ENDCATCH;
  return 0;
}


