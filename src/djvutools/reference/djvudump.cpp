//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: djvudump.cpp,v 1.13 2001-03-30 23:31:25 bcr Exp $
// $Name:  $

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
    #$Id: djvudump.cpp,v 1.13 2001-03-30 23:31:25 bcr Exp $# */
//@{
//@}

#include "DjVuDumpHelper.h"
#include "ByteStream.h"
#include "GException.h"
#include "GOS.h"
#include "GURL.h"

#include <stdio.h>
#include <ctype.h>
#ifdef WIN32
#include <stdlib.h>
#endif

void
display(const GURL &url)
{
   DjVuDumpHelper helper;
   GP<ByteStream> ibs=ByteStream::create(url, "rb");
   GP<ByteStream> str_out;
   str_out=helper.dump(ibs);
   GP<ByteStream> obs=ByteStream::create("w");
   str_out->seek(0);
   obs->copy(*str_out);
}

void
usage()
{
  fprintf(stderr,
          "DJVUDUMP -- Describes IFF85 files\n"
          "  Copyright � 1999-2000 LizardTech, Inc. All Rights Reserved.\n"
          "Usage: djvudump <iff_filenames>\n" );
  exit(1);
}

int 
main(int argc, char **argv)
{
  DArray<GString> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
  {
    GString g(argv[i]);
    dargv[i]=g.getNative2UTF8();
  }
  G_TRY
    {
      if (argc<=1)
        usage();
      for (int i=1; i<argc; i++)
      {
        const GURL::Filename::UTF8 url(dargv[i]);
        display(url);
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

