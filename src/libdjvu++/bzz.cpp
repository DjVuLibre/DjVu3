//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: bzz.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $

// BZZ -- a frontend for BSByteStream

/** @name bzz

    \begin{description}
    \item[Compression:]
    #bzz -e[<blocksize>] <infile> <outfile>#
    \item[Decompression:]
    #bzz -d <infile> <outfile>#
    \end{description}    

    Program bzz is a simple front-end for the Burrows Wheeler encoder
    implemented in \Ref{BSByteStream.h}.  Argument #blocksize# is expressed in
    kilobytes and must be in range 200 to 4096.  The default value is 2048.
    Arguments #infile# and #outfile# are the input and output filenames. A
    single dash (#"-"#) can be used to represent the standard input or output.

    @memo
    General purpose compression/decompression program
    @author
    Leon Bottou <leonb@research.att.com> -- initial implementation
    @version
    #$Id: bzz.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $# */
//@{
//@}

#include "GException.h"
#include "ByteStream.h"
#include "BSByteStream.h"

char *program = "(unknown)";

void
usage(void)
{
  fprintf(stderr, 
          "usage (encoding): %s -e[<blocksize>] <infile> <outfile>\n"
          "usage (decoding): %s -d <infile> <outfile>\n"
          "  Argument <blocksize> must be in range [900..4096] (default 1100).\n"
          "  Arguments <infile> and <outfile> can be '-' for stdin/stdout.\n",
          program, program);
  exit(10);
}

int 
main(int argc, char **argv)
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
  TRY
    {
      StdioByteStream in(infile,"rb");
      StdioByteStream out(outfile,"wb");
      if (blocksize)
        {
          BSByteStream bsb(&out, blocksize);
          bsb.copy(in);
        }
      else 
        {
          BSByteStream bsb(&in);
          out.copy(bsb);
        }
    }
  CATCH(ex)
    {
      ex.perror();
      exit(10);
    }
  ENDCATCH;
  return 0;
}

