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
//C- $Id: TestJB2.cpp,v 1.5 1999-03-16 20:21:31 leonb Exp $



#include <stdio.h>
#include "JB2Image.h"
#include "GBitmap.h"
#include "ByteStream.h"
#include "GException.h"

#ifdef ENCODE
#include "JB2Filter.h"
#include "RunImage.h"
#endif

#ifdef WIN32
#define BINARY "b"
#else
#define BINARY
#endif


int 
usage()
{
  printf("Usage: TestJB2 <jb2file> [<pbm-or-rle-or-q-file>]\n");
#ifdef ENCODE
  printf("   or: TestJB2 -e <pbm-or-rle-file> <jb2file>\n");
#endif
  exit(0);
}

int
main(int argc, char **argv)
{
  TRY
    {
      if (argc < 2)
        usage();
#ifdef ENCODE
      if (argv[1][0]=='-' && argv[1][1]=='e')
        {
          // Encoding mode
          if (argc != 4)
            usage();
          // Read image and compute runs
          StdioByteStream input(argv[2],"r"BINARY);
          RunImage rimg;
          GBitmap bm(input);
          rimg.add_bitmap_runs(bm);
          bm.GBitmap::~GBitmap();
          // Perform CC analysis
          printf("%d ccs\n", rimg.make_ccs_by_analysis());
          printf("%d merged ccs\n", rimg.merge_overlapping_ccs(85));
          // Extract JB2 Image
          GP<JB2Image> jimg = rimg.get_jb2image();
          jimg->set_dimension(bm.columns(), bm.rows());
          rimg.RunImage::~RunImage();
          // Apply JB2 Image filters
          GP<JB2Image> jimgf = JB2Filter(jimg);
          // Code JB2 Image
          StdioByteStream output(argv[3],"w"BINARY);
          jimgf->encode(output);
        }
      else
#endif
        {
          // Decoding or Recoding mode
          //
          if (argc>3)
            usage();
          StdioByteStream input (argv[1],"r"BINARY);
          GP<JB2Image> image = new JB2Image;
          image->decode(input);
          GP<GBitmap> bm = image->get_bitmap(1);
          if (argc>2)
            {
              int len = strlen(argv[2]);
              if (! strcmp(argv[2]+len-4, ".pbm")) {
                StdioByteStream output(argv[2],"w"BINARY);
                bm->save_pbm(output);
              } else if (! strcmp(argv[2]+len-4, ".rle")) {
                StdioByteStream output(argv[2],"w"BINARY);
                bm->save_rle(output);
              } else if (! strcmp(argv[2]+len-2, ".r")) {
                StdioByteStream output(argv[2],"w"BINARY);
                bm->save_rle(output);
              } else if (! strcmp(argv[2]+len-2, ".q")) {
                StdioByteStream output(argv[2],"w"BINARY);
                image->encode(output);
              } else {
                THROW("Unrecognized suffix for output file");
              }
            }
        }
    }
  CATCH(ex)
    {
      ex.perror();
    }
  ENDCATCH;
  return 0;
}
