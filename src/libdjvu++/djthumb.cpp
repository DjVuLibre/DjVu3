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
//C- $Id: djthumb.cpp,v 1.2 2000-01-13 22:15:31 eaf Exp $

// DJTHUMB -- DjVu thumbnails generator

/** @name djthumb

    {\bf Synopsis}
    \begin{verbatim}
        djthumb [options] <djvu_file_in> <djvu_file_out>
    \end{verbatim}

    {\bf Description} --- File #"djthumb.cpp"# illustrates how to use
    \Ref{DjVuDocEditor} class for thumbnails generation. Any multipage
    DjVu document can have integrated thumbnails. These are small icons
    stored inside the document (one per every page), which can be displayed
    by DjVu plugins in a narrow column along the left side of DjVu page.

    {\em {\bf Note}: As of DjVu release 3.0 only UNIX DjVu plugin is capable of
    viewing them. Windows support for thumbnails is expected in the next
    release. }

    {\bf Arguments} --- #<djvu_file_in># and #<djvu_file_out>#
    are names of input and output files. The #options# can be:
    \begin{description}
       \item[-verbose] Do verbose processing (progress indicator and
          status messages).
       \item[-size <icon_size>] Specifies the size of thumbnail images to be
          generated. This parameter affects only the quality of the images.
	  DjVu plugin can scale thumbnails arbitrary and regardless of
	  their original size.
    \end{description}
    
    @memo
    Thumbnails generator
    @author
    Andrei Erofeev <eaf@geocities.com> -- initial implementation
    @version
    #$Id: djthumb.cpp,v 1.2 2000-01-13 22:15:31 eaf Exp $# */
//@{
//@}

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "GString.h"
#include "DjVuDocEditor.h"
#include "DjVmDoc.h"
#include "GOS.h"

static const char * progname;

static void
usage(void)
{
   fprintf(stderr, "\
DJTHUMB -- DjVu thumbnails generating utility\n\
   Copyright (c) AT&T 1999.  All rights reserved\n\
\n\
Usage:\n\
	%s [options] <djvu_file_in> <djvu_file_out>\n\
\n\
Options:\n\
	-v[erbose]	- Verbose operation.\n\
	-s[ize] <size>	- How big (in pixels) thumbnail images\n\
			  should be (default is 128 pixels).\n\
\n\
The program will generate thumbnails for every page of <djvu_file_in>
and will store results into <djvu_file_out>.\n\n", progname);
}

static int pages_num;

static bool
progress_cb(int page_num, void *)
{
   fprintf(stderr, "Processing page %d of %d (%d%%)...\r",
	   page_num, pages_num, 100*page_num/pages_num);
   return false;	// Proceed
}

int
main(int argc, char ** argv)
{
      // Get the program name
   const char * ptr;
   for(progname=ptr=argv[0];*ptr;ptr++)
      if (*ptr=='/') progname=ptr+1;
   
   if (argc<3)
   {
      usage();
      exit(1);
   }

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
      const char * name_in=0;
      const char * name_out=0;
      int size=128;
      bool verbose=false;
   
      for(int i=1;i<argc;i++)
      {
	 if (argv[i][0]!='-')
	 {
	    if (!name_in) name_in=argv[i];
	    else if (!name_out) name_out=argv[i];
	    else fprintf(stderr, "Unexpected string '%s' ignored.\n", argv[i]);
	 } else if (!strncmp(argv[i], "-v", 2)) verbose=true;
	 else if (!strncmp(argv[i], "-s", 2))
	 {
	    if (++i<argc)
	    {
	       int _size=atoi(argv[i]);
	       if (_size<32) fprintf(stderr, "Image size (%d) is too small\n", _size);
	       else if (_size>256) fprintf(stderr, "Image size (%d) is too big\n", _size);
	       else size=_size;
	    } else fprintf(stderr, "Flag '%s' is missing its value.\n", argv[i-1]);
	 } else fprintf(stderr, "Unknown flag '%s' encountered\n", argv[i]);
      }

      if (verbose)
	 fprintf(stderr, "Image size=%d pixels.\n", size);

      int size_in=-1, size_out=-1;
      struct stat st;
      if (stat(name_in, &st)>=0) size_in=st.st_size;
      
      GP<DjVuDocEditor> doc=new DjVuDocEditor;
      doc->init(name_in);
      pages_num=doc->get_pages_num();
      if (pages_num==1)
	 THROW("Thumbnails cannot be generated for one-page documents.");
      
      doc->generate_thumbnails(size, verbose ? progress_cb : 0, 0);
      
      doc->save_as(name_out, true);

      if (stat(name_out, &st)>=0) size_out=st.st_size;
      
      if (verbose && size_in>0 && size_out>0)
	 fprintf(stderr, "Document size: was %d, became %d, increase %d%%\n",
		 size_in, size_out, 100*(size_out-size_in)/size_in);
   } CATCH(exc) {
      exc.perror();
      exit(1);
   } ENDCATCH;
}
