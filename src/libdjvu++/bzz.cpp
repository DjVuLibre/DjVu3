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
//C- $Id: bzz.cpp,v 1.10 1999-03-17 19:24:59 leonb Exp $

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
    #$Id: bzz.cpp,v 1.10 1999-03-17 19:24:59 leonb Exp $# */
//@{
//@}

#include "GException.h"
#include "ByteStream.h"
#include "BSByteStream.h"
#include "ATTLicense.h"

char *program = "(unknown)";

void
usage(void)
{
  fprintf(stderr, 
          "BZZ -- ZPCoded Burrows Wheeler compression\n"
          "%s\n"
          "Usage [encoding]: %s -e[<blocksize>] <infile> <outfile>\n"
          "Usage [decoding]: %s -d <infile> <outfile>\n"
          "  Argument <blocksize> must be in range [900..4096] (default 1100).\n"
          "  Arguments <infile> and <outfile> can be '-' for stdin/stdout.\n",
          ATTLicense::get_usage_text(), program, program);
  exit(1);
}

int 
main(int argc, char **argv)
{
  TRY
    {
      ATTLicense::process_cmdline(argc,argv);
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
  CATCH(ex)
    {
      ex.perror();
      exit(1);
    }
  ENDCATCH;
  return 0;
}

