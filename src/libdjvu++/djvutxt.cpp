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
//C- $Id: djvutxt.cpp,v 1.1 2000-03-22 23:42:05 eaf Exp $

// DJVUTXT -- DjVu TXT extractor

/** @name djvutxt

    {\bf Synopsis}
    \begin{verbatim}
        djvutxt [-page <page_num>] <djvu_file_in> [<txt_file_out>]
    \end{verbatim}

    {\bf Description} --- File #"djvutxt.cpp"# illustrates how to use
    \Ref{DjVuDocument}, \Ref{DjVuImage}, \Ref{DjVuAnno>, and \Ref{DjVuTXT}
    to retrieve textual information stored inside a #TXT*# chunk of a DjVu
    document.

    #TXT*# chunks should have been created with the help of an OCR
    engine, and are used to allow indexing and searching of the DjVu
    document. The chunks contain the ASCII text itself, and layout information
    allowing the DjVu plugins to highlight found text.

    This utility can be used to extract text from #TXT*# chunks and
    output it to a file or standard output.
    
    {\bf Arguments}:
    \begin{itemize}
       \item {\bf <djvu_file_in>} - Name of input DjVu file.
       \item {\bf <txt_file_out>} - Name of the file where ASCII text
             will be stored. #-# means standard output.
    \end{itemize}

    {\bf -page} option can be used to select a particular page from the
    {\bf <djvu_file_in>} for processing.
    
    @memo #TXT*# chunks extractor
    @author
    Andrei Erofeev <eaf@geocities.com> -- initial implementation
    @version
    #$Id: djvutxt.cpp,v 1.1 2000-03-22 23:42:05 eaf Exp $# */
//@{
//@}

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "GString.h"
#include "DjVuDocument.h"
#include "GOS.h"

static const char * progname;

static void
usage(void)
{
   fprintf(stderr, "\
DJVUTXT -- DjVu TXT* chunks extractor\n\
   Copyright (c) AT&T 1999, 2000.  All rights reserved\n\
\n\
Usage:\n\
	%s [-page <page_num>] <djvu_file_in> [<txt_file_out>]\n\
\n\
The program will decode and output to <txt_file_out> ASCII text from\n\
every TXT* (TXTa or TXTz) chunk found in the source <djvu_file_in>\n\
DjVu document.\n\
\n\
TXT* chunks contain information about the text present within the given\n\
page, and its location. These chunks should have been generated with\n\
the help of an OCR engine, and encoded using capabilities provided by\n\
this library.\n\n", progname);
}

static void
doPage(const GP<DjVuDocument> & doc, int page_num,
       ByteStream & str_out)
{
   GP<DjVuImage> dimg=doc->get_page(page_num);
   if (!dimg)
      THROW("Failed to decode page.");
   GP<MemoryByteStream> anno_str=dimg->get_anno();
   if (anno_str)
   {
      GP<DjVuAnno> anno=new DjVuAnno();
      anno->decode(*anno_str);
      GP<DjVuTXT> txt=anno->txt;
      if (txt)
	 str_out.write((const char *) txt->textUTF8,
		       txt->textUTF8.length());
   }
}

int
main(int argc, char ** argv)
{
      // Get the program name
   const char * ptr;
   for(progname=ptr=argv[0];*ptr;ptr++)
      if (*ptr=='/') progname=ptr+1;
   
#ifdef DEBUG
   {
      const char * debug=getenv("DEBUG");
      if (debug)
      {
	 int level=atoi(debug);
	 if (level<1) level=1;
	 if (level>32) level=32;
	 DEBUG_SET_LEVEL(level);
      }
   }
#endif
   
   TRY {
      GString name_in, name_out;
      int page_num=-1;

      for(int i=1;i<argc;i++)
      {
	 if (!strcmp(argv[i], "-") || argv[i][0]!='-')
	 {
	    if (!name_in.length())
	    {
	       if (!strcmp(argv[i], "-"))
	       {
		  fprintf(stderr, "Can't read from standard input.\n\n");
		  usage();
		  exit(1);
	       } else name_in=argv[i];
	    } else
	    {
	       if (!name_out.length())
		  name_out=argv[i];
	       else
	       {
		  usage();
		  exit(1);
	       }
	    }
	 } else
	 {
	    if (argv[i][0]=='-' && argv[i][1]=='-')
	       argv[i]++;
	    if (!strncmp(argv[i], "-p", 2))
	    {
	       if (i+1>=argc)
	       {
		  fprintf(stderr, "-page option must be followed by a number.\n\n");
		  usage();
		  exit(1);
	       }
	       i++;
	       page_num=atoi(argv[i])-1;
	       if (page_num<0)
	       {
		  fprintf(stderr, "Page number must be positive.\n\n");
		  usage();
		  exit(1);
	       }
	    } else if (!strncmp(argv[i], "-h", 2) ||
		       !strncmp(argv[i], "-h", 2))
	    {
	       usage();
	       exit(1);
	    } else fprintf(stderr, "Unrecognized option '%s' encountered.\n\n", argv[i]);
	 }
      }

      if (name_in.length()==0)
      {
	 fprintf(stderr, "The name of the input file is missing.\n\n");
	 usage();
	 exit(1);
      }
      if (name_out.length()==0)
	 name_out="-";

      GP<DjVuDocument> doc=new DjVuDocument();
      doc->init(GOS::filename_to_url(name_in));
      StdioByteStream str_out(name_out, "w");
      if (page_num<0)
	 for(page_num=0;page_num<doc->get_pages_num();page_num++)
	    doPage(doc, page_num, str_out);
      else
	 doPage(doc, page_num, str_out);
   } CATCH(exc) {
      exc.perror();
      exit(1);
   } ENDCATCH;
}
