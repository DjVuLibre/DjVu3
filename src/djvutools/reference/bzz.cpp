//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: bzz.cpp,v 1.9 2001-01-04 22:04:54 bcr Exp $
// $Name:  $


// BZZ -- a frontend for BSByteStream

/** @name bzz

    \begin{description}
    \item[Compression:]
    #bzz -e[<blocksize>] <infile> <outfile>#
    \item[Decompression:]
    #bzz -d <infile> <outfile>#
    \end{description}    

    Program bzz is a simple front-end for the Burrows Wheeler encoder
    implemented in \Ref{BSByteStream.h}.  Although this compression model is
    not currently used in DjVu files, it may be used in the future for
    encoding textual data chunks.  Argument #blocksize# is expressed in
    kilobytes and must be in range 200 to 4096.  The default value is 2048.
    Arguments #infile# and #outfile# are the input and output filenames. A
    single dash (#"-"#) can be used to represent the standard input or output.

    @memo
    General purpose compression/decompression program
    @author
    L\'eon Bottou <leonb@research.att.com> -- initial implementation
    @version
    $Id: bzz.cpp,v 1.9 2001-01-04 22:04:54 bcr Exp $ */
//@{
//@}

#include "GException.h"
#include "ByteStream.h"
#include "BSByteStream.h"
#include <stdlib.h>

char *program = "(unknown)";

void
usage(void)
{
  fprintf(stderr, 
          "BZZ -- ZPCoded Burrows Wheeler compression\n"
          "  Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.\n"
          "Usage [encoding]: %s -e[<blocksize>] <infile> <outfile>\n"
          "Usage [decoding]: %s -d <infile> <outfile>\n"
          "  Argument <blocksize> must be in range [900..4096] (default 1100).\n"
          "  Arguments <infile> and <outfile> can be '-' for stdin/stdout.\n",
          program, program);
  exit(1);
}

int 
main(int argc, char **argv)
{
  G_TRY
    {
      // Get program name
      program = strrchr(argv[0],'/');
      if (program) 
        program += 1; 
      else 
        program = argv[0];
      // Obtain default mode from program name
      int blocksize = -1;
      if (!strcmp(program,"bzz"))
        blocksize = 1100;
      else if (!strcmp(program,"unbzz"))
        blocksize = 0;
      // Parse arguments
      if (argc>=2 && argv[1][0]=='-')
        {
          if (argv[1][1]=='d' && argv[1][2]==0)
            {
              blocksize = 0;
            }
          else if (argv[1][1]=='e')
            {
              blocksize = 2048;
              if (argv[1][2])
                blocksize = atoi(argv[1]+2);
            }
          else 
            usage();
          argv++;
          argc--;
        }
      if (blocksize < 0)
        usage();
      // Obtain filenames
      char *infile = "-";
      char *outfile = "-";
      if (argc >= 2)
        infile = argv[1];
      if (argc >= 3)
        outfile = argv[2];
      if (argc >= 4)
        usage();
      // Action
      StdioByteStream in(infile,"rb");
      StdioByteStream out(outfile,"wb");
      if (blocksize)
        {
          BSByteStream bsb(out, blocksize);
          bsb.copy(in);
        }
      else 
        {
          BSByteStream bsb(in);
          out.copy(bsb);
        }
    }
  G_CATCH(ex)
    {
      ex.perror();
      exit(1);
    }
  G_ENDCATCH;
  return 0;
}

