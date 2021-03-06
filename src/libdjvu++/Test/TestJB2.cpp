//C-  -*- C++ -*-
//C-
//C-  Copyright � 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
//C-

#include "JB2Image.h"
#include "GBitmap.h"
#include "ByteStream.h"
#include "GException.h"
#include "GURL.h"
#include <stdio.h>
#include <locale.h>

#ifdef ENCODE
#include "JB2Filter.h"
#include "RunImage.h"
#endif

int 
usage()
{
  DjVuPrintMessageUTF8("Usage: TestJB2 <jb2file> [<pbm-or-rle-or-q-file>]\n");
#ifdef ENCODE
  DjVuPrintMessageUTF8("   or: TestJB2 -e <pbm-or-rle-file> <jb2file>\n");
#endif
  exit(0);
}

int
main(int argc, char **argv)
{
  setlocale(LC_ALL,"");
   
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
          GP<ByteStream> input=ByteStream::create(GURL::Filename::Native(argv[2]),"rb");
          GBitmap bm(*input);
          input=0;
          RunImage rimg;
          rimg.add_bitmap_runs(bm);
          bm.GBitmap::~GBitmap();
          // Perform CC analysis
          DjVuPrintMessageUTF8("%d ccs\n", rimg.make_ccs_by_analysis());
          DjVuPrintMessageUTF8("%d merged ccs\n", rimg.merge_overlapping_ccs(85));
          // Extract JB2 Image
          GP<JB2Image> jimg = rimg.get_jb2image();
          jimg->set_dimension(bm.columns(), bm.rows());
          rimg.RunImage::~RunImage();
          // Apply JB2 Image filters
          GP<JB2Image> jimgf = JB2Filter(jimg);
          // Code JB2 Image
          GP<ByteStream> output=ByteStream::create(GURL::Filename::Native(argv[3]),"wb");
          jimgf->encode(*output);
        }
      else
#endif
        {
          // Decoding or Recoding mode
          //
          if (argc>3)
            usage();
          GP<ByteStream> input=ByteStream::create(GURL::Filename::Native(argv[1]),"rb");
          GP<JB2Image> image = JB2Image::create();
          image->decode(input);
          input=0;
          GP<GBitmap> bm = image->get_bitmap(1);
          if (argc>2)
            {
              int len = strlen(argv[2]);
              if (! strcmp(argv[2]+len-4, ".pbm")) {
                GP<ByteStream> output=ByteStream::create(GURL::Filename::Native(argv[2]),"wb");
                bm->save_pbm(*output);
              } else if (! strcmp(argv[2]+len-4, ".rle")) {
                GP<ByteStream> output=ByteStream::create(GURL::Filename::Native(argv[2]),"wb");
                bm->save_rle(*output);
              } else if (! strcmp(argv[2]+len-2, ".r")) {
                GP<ByteStream> output=ByteStream::create(GURL::Filename::Native(argv[2]),"wb");
                bm->save_rle(*output);
              } else if (! strcmp(argv[2]+len-2, ".q")) {
                GP<ByteStream> output=ByteStream::create(GURL::Filename::Native(argv[2]),"wb");
                image->encode(output);
              } else {
                G_THROW( ERR_MSG("TestJB2.bad_suffix") );
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
