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
// $Id: djthumb.cpp,v 1.14 2001-04-21 00:16:57 bcr Exp $
// $Name:  $

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
    #$Id: djthumb.cpp,v 1.14 2001-04-21 00:16:57 bcr Exp $# */
//@{
//@}

#include <stdio.h>
#include <stdlib.h>
#ifndef UNDER_CE
#include <sys/stat.h>
#endif
#include "GString.h"
#include "DjVuDocEditor.h"
#include "DjVmDoc.h"
#include "GOS.h"

static const char * progname;

static void
usage(void)
{
   DjVuPrintError("\
DJTHUMB -- DjVu thumbnails generating utility\n\
   Copyright Â© 1999-2000 LizardTech, Inc. All Rights Reserved.\n\
\n\
Usage:\n\
	%s [options] <djvu_file_in> <djvu_file_out>\n\
\n\
Options:\n\
	-v[erbose]	- Verbose operation.\n\
	-s[ize] <size>	- How big (in pixels) thumbnail images\n\
			  should be (default is 128 pixels).\n\
\n\
The program will generate thumbnails for every page of <djvu_file_in> \
and will store results into <djvu_file_out>.\n\n", progname);
}

static int pages_num;

static bool
progress_cb(int page_num, void *)
{
   DjVuPrintError("Processing page %d of %d (%d%%)...\r",
	   page_num, pages_num, 100*page_num/pages_num);
   return false;	// Proceed
}

int
main(int argc, char ** argv)
{
  DArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
  {
    GUTF8String g(argv[i]);
    dargv[i]=g.getNative2UTF8();
  }
      // Get the program name
   progname=dargv[0]=GOS::basename(dargv[0]);
   
   if (argc<3)
   {
      usage();
      exit(1);
   }

#ifdef DEBUG
   {
      const GUTF8String debug(GOS::getenv("DEBUG"));
      if (debug.length())
      {
//	 int level=debug.is_int()?atoi((const char *)debug):1;
         int level=debug.is_int()?debug.toInt():1;
         if (level<1) level=1;
         if (level>32) level=32;
//	 DEBUG_SET_LEVEL(level);
      }
   }
#endif
   
   G_TRY {
      GUTF8String name_in;
      GUTF8String name_out;
      int size=128;
      bool verbose=false;
   
      for(int i=1;i<argc;i++)
      {
	 if (dargv[i][0]!='-')
	 {
	    if (!name_in.length()) name_in=dargv[i];
	    else if (!name_out.length()) name_out=dargv[i];
	    else DjVuPrintError("Unexpected string '%s' ignored.\n", (const char *)dargv[i]);
	 } else if (dargv[i].ncmp("-v", 2)) verbose=true;
	 else if (dargv[i].ncmp("-s", 2))
	 {
	    if (++i<argc)
	    {
	       int _size=atoi(dargv[i]);
	       if (_size<32) DjVuPrintError("Image size (%d) is too small\n", _size);
	       else if (_size>256) DjVuPrintError("Image size (%d) is too big\n", _size);
	       else size=_size;
	    } else DjVuPrintError("Flag '%s' is missing its value.\n", (const char *)dargv[i-1]);
	 } else DjVuPrintError("Unknown flag '%s' encountered\n", (const char *)dargv[i]);
      }

      if (verbose)
	 DjVuPrintError("Image size=%d pixels.\n", size);

      int size_in=-1, size_out=-1;
      struct stat st;
      if (stat(name_in.getUTF82Native(), &st)>=0) size_in=st.st_size;
      
      GP<DjVuDocEditor> edoc=DjVuDocEditor::create_wait(GURL::Filename::UTF8(name_in));
      pages_num=edoc->get_pages_num();
      if (pages_num==1)
	 G_THROW("Thumbnails cannot be generated for one-page documents.");
     
      int page_num=0;
      do 
      { 
        edoc->generate_thumbnails(size, page_num);
        progress_cb(page_num,0);
      }while (page_num>=0);
      
      edoc->save_as(GURL::Filename::UTF8(name_out), true);

      if (stat(name_out.getUTF82Native(), &st)>=0) size_out=st.st_size;
      
      if (verbose && size_in>0 && size_out>0)
	 DjVuPrintError("Document size: was %d, became %d, increase %d%%\n",
		 size_in, size_out, 100*(size_out-size_in)/size_in);
   } G_CATCH(exc) {
      exc.perror();
      exit(1);
   } G_ENDCATCH;
return 0;
}
