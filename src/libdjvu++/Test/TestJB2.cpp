//C-  -*- C++ -*-
//C-
//C- This software contains software code or other information proprietary to
//C- AT&T Corp. ("AT&T").  Unless you accept a license to use this software,
//C- you shall not use, execute, compile, modify, redistribute, reverse
//C- compile, disassemble, or otherwise reverse engineer the AT&T software or
//C- any derived work of the AT&T software.  The text of a license can be
//C- found in file "ATTLICENSE" or at the Internet website having the URL
//C- "http://www.djvu.att.com/open".
//C
//C- This software is provided to you "AS IS".  YOU ASSUME TOTAL
//C- RESPONSIBILITY AND RISK FOR USE OF THE AT&T SOFTWARE.  AT&T DOES NOT
//C- MAKE, AND EXPRESSLY DISCLAIMS, ANY EXPRESS OR IMPLIED WARRANTIES OF ANY
//C- KIND WHATSOEVER, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
//C- MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, WARRANTIES OF TITLE
//C- OR NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS, ANY WARRANTIES
//C- ARISING BY USAGE OF TRADE, COURSE OF DEALING OR COURSE OF PERFORMANCE, OR
//C- ANY WARRANTY THAT THE AT&T SOFTWARE IS ERROR FREE OR WILL MEET YOUR
//C- REQUIREMENTS.
//C-
//C-     (C) AT&T Corp. All rights reserved.
//C-     AT&T is a registered trademark of AT&T Corp.
//C-
//C- $Id: TestJB2.cpp,v 1.4 1999-03-15 18:28:53 leonb Exp $



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
