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
//C- $Id: djvmcvt.cpp,v 1.10 2000-09-18 17:10:27 bcr Exp $

/** @name djvmcvt

    {\bf Synopsis}\\
    \begin{verbatim}
        djvmcvt -b[undled] <doc_in.djvu> <doc_out.djvu>

	or
	
	djvmcvt -i[ndirect] <doc_in.djvu> <dir_out> <idx_fname.djvu>
    \end{verbatim}

    {\bf Description} ---
    File #"djvmcvt.cpp"# and the program #djvmcvt# serve the purpose of
    convertion of obsolete DjVu documents into the new formats. The program
    can also read documents in the new formats, so you can use it to
    perform conversion between #BUNDLED# and #INDIRECT# formats. This is a
    simple illustration of the capabilities of \Ref{DjVuDocument} class.

    As a matter of fact, there are two ways to make conversion between
    different formats:
    \begin{enumerate}
       \item If the input format is one of obsolete formats (#OLD_BUNDLED#
             or #OLD_INDEXED#) then the conversion can be done by
	     \Ref{DjVuDocument} only.
       \item If the input format is one of new formats (#BUNDLED# or
             #INDIRECT#) then the best candidate to perform conversion
	     is \Ref{DjVmDoc} class. It will do it at the lowest possible
	     with the least expenses.
    \end{enumerate}
    
    {\bf Arguments} ---
    Depending on the output format, the number and types of arguments
    differ. The second argument though (#<doc_in.djvu>#) is the same in both
    cases, and depending on the format of input document, it means:
    \begin{itemize}
       \item {\bf OLD_BUNDLED} format: just name of the document
       \item {\bf OLD_INDEXED} format: name of any page of the document
       \item {\bf BUNDLED} format: name of the document
       \item {\bf INDIRECT} format: name of the top-level file with the
             list of all pages of the document.
    \end{itemize}.

    So, in order to do conversion choose one of syntaxes below:
    \begin{itemize}
       \item To create a new {\em BUNDLED} document

             #djvmcvt -b[undled] <doc_in.djvu> <doc_out.djvu>#

	     This will read the document referenced by #<doc_in.djvu># as
	     descrived above, will convert it into the #BUNDLED#
	     format and will save the results into the #<doc_out.djvu># file.
	     
       \item To create a new {\em INDIRECT} document

             #djvmcvt -i[ndirect] <doc_in.djvu> <dir_out> <idx_fname.djvu>#

	     This will read the input document referenced by #<doc_in.djvu>#
	     as described above, will convert it into the #INDIRECT#
	     format and will save it into the #<dir_out># directory. Since
	     DjVu multipage documents in the #INDIRECT# formats are
	     represented by a bunch of files, you have to specify a directory
	     name where all of the files will be saved. In addition to these
	     files the program will also create a top-level file named
	     #<idx_fname.djvu># with the list of all pages and components
	     composing the given DjVu document. Whenever you need to open
	     this document later, open this top-level file.
    \end{itemize}
	     
    @memo
    DjVu multipage document converter.
    @author
    Andrei Erofeev <eaf@geocities.com>
    @version
    #$Id: djvmcvt.cpp,v 1.10 2000-09-18 17:10:27 bcr Exp $# */

#ifdef __GNUC__
#pragma implementation
#endif

#include "debug.h"
#include "DjVmDoc.h"
#include "DjVuDocument.h"
#include "GOS.h"

#include <stdio.h>

static char * progname;

static void Usage(void)
{
   fprintf(stderr, "\
DJVMCVT -- DjVu multipage document conversion utility\n\
   Copyright (c) AT&T 1999.  All rights reserved\n\
\n\
Usage:\n\
\n\
   To convert any DjVu multipage document into the new BUNDLED format:\n\
	%s -b[undled] <doc_in.djvu> <doc_out.djvu>\n\
	where <doc_out.djvu> is the name of the output file.\n\
\n\
   To convert any DjVu multipage document into the new INDIRECT format:\n\
	%s -i[ndirect] <doc_in.djvu> <dir_out> <idx_fname.djvu>\n\
	where <dir_out> is the name of the output directory, and\n\
	<idx_fname.djvu> is the name of the top-level document index file.\n\
\n\
   The <doc_in.djvu> specifies the document to be converted.\n\
   For OLD_BUNDLED and BUNDLED formats, this is the name of the document file.\n\
   For INDIRECT format, this is the name of the top-level index file.\n\
   For OLD_INDEXED format, this is the name of any page file.\n\
\n", progname, progname);
}

static void
do_bundled(int argc, char ** argv)
      // <progname> -b[undled] <file_in> <file_out>
{
   if (argc!=4) { Usage(); exit(1); }
   GP<DjVuDocument> doc = new DjVuDocument;
   doc->init(GOS::filename_to_url(argv[2]));
   StdioByteStream str(argv[3], "wb");
   doc->write(str);
}

static void
do_indirect(int argc, char ** argv)
      // <progname> -i[ndirect] <file_in> <dir_out> <idx_fname>
{
   if (argc!=5) { Usage(); exit(1); }
   GP<DjVuDocument> doc = new DjVuDocument;
   doc->init(GOS::filename_to_url(argv[2]));
   doc->expand(argv[3], argv[4]);
}

int 
main(int argc, char ** argv)
{
   char * ptr;
   for(progname=ptr=argv[0];*ptr;ptr++)
      if (*ptr=='/') progname=ptr+1;

   if (argc<2) { Usage(); exit(1); }

   bool bundled=true;
   G_TRY {
      if (!strncmp(argv[1], "-b", 2)) bundled=true;
      else if (!strncmp(argv[1], "-i", 2)) bundled=false;
      else { Usage(); exit(1); }

      if (bundled) do_bundled(argc, argv);
      else do_indirect(argc, argv);
   } G_CATCH(exc) {
      fprintf(stderr, "%s\n", exc.get_cause());
      exit(1);
   } G_ENDCATCH;

   exit(0);
}
