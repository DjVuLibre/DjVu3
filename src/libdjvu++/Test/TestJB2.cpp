//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
//C-

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
  G_TRY
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
                G_THROW("TestJB2.bad_suffix");
              }
            }
        }
    }
  G_CATCH(ex)
    {
      ex.perror();
    }
  G_ENDCATCH;
  return 0;
}
