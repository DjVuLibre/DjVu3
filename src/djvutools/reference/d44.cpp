//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2000 LizardTech, Inc. All Rights Reserved.
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
// $Id: d44.cpp,v 1.6 2000-12-18 17:13:40 bcr Exp $
// $Name:  $

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
    #$Id: d44.cpp,v 1.6 2000-12-18 17:13:40 bcr Exp $# 
*/
//@{
//@}

#include "GString.h"
#include "GException.h"
#include "IWImage.h"
#include "GOS.h"
#include "IFFByteStream.h"
#include "GPixmap.h"
#include "GBitmap.h"
#include <stdlib.h>
#ifdef UNDER_CE
#include <windows.h>
#endif

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
         "  Copyright � 1999-2000 LizardTech, Inc. All Rights Reserved.\n"
         "Usage: d44 [options] iw4file [pnmfile]\n"
         "Options:\n"
         "    -verbose     -- report decoding time and memory use\n"
         "    -chunks n    -- select number of chunks to decode\n"
         "\n");
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
                G_THROW("d44: missing argument after option -chunks");
              char *ptr;
              flag_chunks = strtol(argv[i], &ptr, 10);
              if (*ptr || flag_chunks<=0 || flag_chunks>999)
                G_THROW("d44: illegal argument after option -chunks");
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
#ifndef UNDER_CE
main(int argc, char **argv)
#else
mymain(int argc, char **argv)
#endif
{
  G_TRY
    {
      // Parse arguments
      parse(argc, argv);
      // Check input file
      StdioByteStream ibs(iw4file,"rb");
      GString chkid;
      // Determine file type
      { 
        IFFByteStream iff(ibs);
        if (! iff.get_chunk(chkid))
          G_THROW("d44: malformed IW4 file");
        ibs.seek(0);
      }
      // Go decoding
      if (chkid == "FORM:DJVM")
	      G_THROW("This is multipage DJVU file. Please break it into pieces.");
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
#ifndef UNDER_CE
          remove(pnmfile);
#else
          WCHAR tszPnmFile[MAX_PATH] ;
          MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,pnmfile,strlen(pnmfile)+1,tszPnmFile,sizeof(tszPnmFile)) ;
          DeleteFile(tszPnmFile) ;
#endif
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
#ifndef UNDER_CE
          remove(pnmfile);
#else
          WCHAR tszPnmFile[MAX_PATH] ;
          MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,pnmfile,strlen(pnmfile)+1,tszPnmFile,sizeof(tszPnmFile)) ;
          DeleteFile(tszPnmFile) ;
#endif
          StdioByteStream obs(pnmfile,"wb");
          ppm->save_ppm(obs);
        }
      else
        {
          G_THROW("d44: expected BM44 or PM44 chunk in IW4 file");
        }
    }
  G_CATCH(ex)
    {
      ex.perror("Exception while executing D44");
      exit(1);
    }
  G_ENDCATCH;
  return 0;
}

#ifdef UNDER_CE

int WINAPI WinMain (HINSTANCE hInstance,
		             HINSTANCE hPrevInstance,
                     LPTSTR lpCmdLine,
                     int nCmdShow)
{
   char *argv[3] ;
   argv[0]="-" ;
   argv[1]="-verbose" ;
   argv[2]="enigmaphoto.djvu" ;
   mymain (3, argv) ;
   return (0) ;
}
#endif
