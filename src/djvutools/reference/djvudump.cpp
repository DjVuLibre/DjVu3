//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.
//C- Copyright (c) 2000 LizardTech Inc.
//C- All rights reserved.
//C-
//C- This software may only be used by you under license from LizardTech
//C- Inc. A copy of LizardTech's Source Code Agreement is available at
//C- LizardTech's Internet website having the URL <http://www.djvu.com/open>.
//C- If you received this software without first entering into a license with
//C- LizardTech, you have an infringing copy of this software and cannot use it
//C- without violating LizardTech's intellectual property rights.
//C-
//C- $Id: djvudump.cpp,v 1.2 2000-10-18 23:40:02 fcrary Exp $

/** @name djvuinfo

    {\bf Synopsis}
    \begin{verbatim}
        djvudump <... iff_file_names ...>
    \end{verbatim}

    {\bf Description} --- File #"djvudump.cpp"# uses the facilities
    provided by \Ref{IFFByteStream.h} to display an indented
    representation of the chunk structure of an ``EA IFF 85'' file.
    Each line represent contains a chunk ID followed by the chunk
    size.  Additional information about the chunk is provided when
    program #djvuinfo.cpp# recognizes the chunk name and knows how to
    summarize the chunk data.  Furthermore, page identifiers are
    printed between curly braces when #djvudump# recognizes a bundled
    multipage document.  Lines are indented in order to reflect the
    hierarchical structure of the IFF files.

    {\bf Example}
    \begin{verbatim}
    % djvuinfo graham1.djvu 
    graham1.djvu:
      FORM:DJVU [32553] 
        INFO [5]            2325x3156, version 20, 300 dpi, gamma 2.2
	ANTa [34]	    Page annotation
	INCL [11]	    Indirection chunk (document.dir)
        Sjbz [17692]        JB2 data, no header
        BG44 [2570]         #1 - 74 slices - v1.2 (color) - 775x1052
        FG44 [1035]         #1 - 100 slices - v1.2 (color) - 194x263
        BG44 [3048]         #2 - 10 slices 
        BG44 [894]          #3 - 4 slices 
        BG44 [7247]         #4 - 9 slices 
    \end{verbatim}

    {\bf References} ---
    EA IFF 85 file format specification:\\
    \URL{http://www.cica.indiana.edu/graphics/image_specs/ilbm.format.txt}
    or \URL{http://www.tnt.uni-hannover.de/soft/compgraph/fileformats/docs/iff.pre}

    @memo
    Prints the structure of an IFF file.
xxx
    @author
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id: djvudump.cpp,v 1.2 2000-10-18 23:40:02 fcrary Exp $# */
//@{
//@}

#include <stdio.h>
#include <ctype.h>
#include "DjVuDumpHelper.h"

void
display(const char *s)
{
   DjVuDumpHelper helper;
   StdioByteStream ibs(s, "rb");
   GP<ByteStream> str_out;
   str_out=helper.dump(ibs);
   StdioByteStream obs("-", "w");
   str_out->seek(0);
   obs.copy(*str_out);
}

void
usage()
{
  fprintf(stderr,
          "DJVUDUMP -- Describes IFF85 files\n"
          "  Copyright (c) AT&T 1999.\n  Copyright (c) LizardTech 2000.\n  All rights reserved\n"
          "Usage: djvudump <iff_filenames>\n" );
  exit(1);
}

int 
main(int argc, char **argv)
{
  G_TRY
    {
      if (argc<=1)
        usage();
      for (int i=1; i<argc; i++)
        display(argv[i]);
    }
  G_CATCH(ex)
    {
      ex.perror();
      exit(1);
    }
  G_ENDCATCH;
  return 0;
}

