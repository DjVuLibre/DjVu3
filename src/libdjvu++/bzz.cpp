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
//C- $Id: bzz.cpp,v 1.8 1999-03-15 18:28:53 leonb Exp $

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
    #$Id: bzz.cpp,v 1.8 1999-03-15 18:28:53 leonb Exp $# */
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

