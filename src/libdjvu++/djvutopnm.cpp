//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: djvutopnm.cpp,v 1.1 1999-01-22 00:40:19 leonb Exp $

// File "$Id: djvutopnm.cpp,v 1.1 1999-01-22 00:40:19 leonb Exp $"
// Author: Yann Le Cun 08/1997

#include <stdio.h>
#include "GException.h"
#include "GSmartPointer.h"
#include "GRect.h"
#include "GPixmap.h"
#include "GBitmap.h"
#include "JB2Codec.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "DejaVuCodec.h"


static int flag_verbose = 0;
static int flag_subsample = -1;

static void
convert(const char *from, const char *to, int subsample=1)
{
  // Open djvu file
  StdioByteStream in (from,"rb");
  // Decode djvu file
  DejaVuDecoder   decoder(in);
  GP<DejaVuImage> dimg = new DejaVuImage;
  decoder.decode(dimg);
  // Verbose
  if (flag_verbose)
    fprintf(stderr,"Internal DjVuImage is %dkB\n", 
            (dimg->get_memory_usage()+512)/1024 );
  // Get pixmap or bitmap
  GRect rect(0,0, dimg->get_width()/subsample, dimg->get_height()/subsample);
  GP<GPixmap> pm = dimg->get_color_pixmap(rect, subsample);
  // Save image
  StdioByteStream out (to,"wb");
  if (pm) 
    {
      pm->save_ppm(out);
    }
  else 
    {
      GP<GBitmap> bm = dimg->get_bitmap(rect, subsample);
      if (!bm)
        THROW("Cannot decode this file");
      if (subsample==1)
        bm->save_pbm(out);
      else 
        bm->save_pgm(out);
    }
}


void
usage()
{
  printf("DJVUTOPNM -- DjVu decompression utility\n"
         "             [djvutopnm (c) AT&T Labs 1997 (Leon Bottou, HA6156)]\n\n"
         "Usage: djvutopnm [-v] -<n> [<djvufile> [<pnmfile>]]\n\n"
         "Options:\n"
         "  -v   Reports memory used by internal DjVu image.\n"
         "  -<n> Subsampling ratio. Legal values for argument <n>\n"
         "       are integers in range 1..15. [Required option]\n\n"
         "The ouput file is usually a PPM file, except when the DjVu file\n"
         "contains only a bilevel image.  The output file then is either a PBM\n"
         "file (subsampling ratio is 1) or a PGM file (otherwise).\n"
         "If <pnmfile> is a single dash or omitted, the decompressed image\n"
         "is sent to the standard output.  If <djvufile> is a single dash or\n"
         "omitted, the djvu file is read from the standard input.\n\n"
         );
  exit(-1);
}


int
main(int argc, const char **argv)
{
  TRY
    {
      while (argc>1 && argv[1][0]=='-' && argv[1][1])
        {
          if (argv[1][1]=='v')
            flag_verbose = 1;
          else if (argv[1][1]>='0' && argv[1][1]<='9')
            flag_subsample = atoi(&argv[1][1]);
          argc -= 1;
          argv += 1;
        }
      if (flag_subsample<1 || flag_subsample>15)
        usage();
      if (argc == 1) 
        convert("-","-", flag_subsample); 
      else if (argc == 2) 
        convert(argv[1],"-", flag_subsample);
      else if (argc == 3) 
        convert(argv[1],argv[2], flag_subsample);
      else
        usage();
    }
  CATCH(ex)
    {
      ex.perror("Exception while executing D44");
      return -1;
    }
  ENDCATCH;
  return 0;
}
