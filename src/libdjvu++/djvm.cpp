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
//C- $Id: djvm.cpp,v 1.1.1.1 1999-10-22 19:29:23 praveen Exp $

/** @name djvm

    {\bf Synopsis}\\
    \begin{verbatim}
        djvm [options] djvudocument [djvufiles] [pagenum]
    \end{verbatim}

    {\bf Description} ---

    File #"djvm.cpp"# illustrates how classes
    \Ref{DjVmDoc}, and \Ref{DjVuDocument} to create, modify and
    convert multipage documents.

    {\bf Arguments} ---
    Depending on the task to be performed, the number and types of arguments
    differ:
    \begin{itemize}
       \item To create a new document

             #djvm -c[reate] <doc.djvu> <page_1.djvu> ... <page_n.djvu>#

	     This will package pages represented by files #<page_1.djvu>#
	     ... #<page_n.djvu># into all-in-one-file multipage DjVu document
	     #<doc.djvu>#. The pages do not have to include a file with #NDIR#
	     chunk, that is they don't have to be part of a single document.
	     \Ref{DjVuDocument} will take care of it and will create a new
	     navigation directory automatically.
	     
       \item To expand an existing document

             #djvm -e[xpand] <doc.djvu>#

	     This will expand the given #<doc.djvu># into current directory.
	     Basically, this does conversion into a format ideal for web
	     publishing: when every page is in one or more separate files.
	     After expansion pages will still be linked by a navigation directory.
	     
       \item To insert a page

       	     #djvm -i[nsert] <doc.djvu> <page.djvu> <page_num>#

	     This will insert page represented by file #<page.djvu># into
	     document #<doc.djvu># at the position number #<page_num># (starts
	     from #1#).
	     
       \item To delete a page

             #djvm -d[elete] <doc.djvu> <page_num>#

	     This will remove page number #<page_num># from document
	     #<doc.djvu>#. If there is only one page left in #<doc.djvu>#
	     after deletion, it will automatically be converted into a
	     single page DjVu file format. Page numbering starts from #1#.

       \item To view document contents

             #djvm -l[ist] <doc.djvu>#

	     This will list all files composing the given document #<doc.djvu>#
    \end{itemize}
	     
    @memo
    DjVu all-in-one-file multipage documents creator.
    @author
    Andrei Erofeev <eaf@research.att.com>
    @version
    #$Id: djvm.cpp,v 1.1.1.1 1999-10-22 19:29:23 praveen Exp $# */
//@{
//@}

#include "GException.h"
#include "DjVuDocument.h"
#include "GOS.h"

#include <stdlib.h>
#include <iostream.h>

static char * progname;

static void
usage(void)
{
  fprintf(stderr,


}











static void
create(int argc, char ** argv)
      // djvm -c[reate] <doc.djvu> <page_1.djvu> ... <page_n.djvu>
      // doc.djvu will be overwritten
{
   if (argc<4) { usage(); exit(1); }
   GP<DjVuDocument> doc=new DjVuDocument(0);
   for(int i=3;i<argc;i++)
   {
      GP<DjVuFile> file=new DjVuFile(GOS::filename_to_url(argv[i]));
      doc->insert_page(file, -1);
   }
   doc->save_as_djvm(argv[2]);
}

static void
expand(int argc, char ** argv)
      // djvm -e[xpand] <doc.djvu>
{
   if (argc!=3) { usage(); exit(1); }
   GP<DjVuDocument> doc=new DjVuDocument(GOS::filename_to_url(argv[2]), 0);
   doc->save_as_djvu(GOS::cwd());
}

static void
insert(int argc, char ** argv)
      // djvm -i[nsert] <doc.djvu> <page.djvu> <page_num>
{
   if (argc!=5) { usage(); exit(1); }
   GP<DjVuDocument> doc=new DjVuDocument(GOS::filename_to_url(argv[2]), 0);
   GP<DjVuFile> file=new DjVuFile(GOS::filename_to_url(argv[3]));
   int page_num=atoi(argv[4])-1;
   if (page_num<0) { fprintf(stderr, "Page number must be positive.\n"); exit(1); }
   doc->insert_page(file, page_num);
   doc->save_as_djvm(argv[2]);
}

static void
del(int argc, char ** argv)
      // djvm -d[elete] <doc.djvu> <page_num>
{
   if (argc!=4) { usage(); exit(1); }
   GP<DjVuDocument> doc=new DjVuDocument(GOS::filename_to_url(argv[2]), 0);
   int page_num=atoi(argv[3])-1;
   if (page_num<0) { fprintf(stderr, "Page number must be positive.\n"); exit(1); }
   doc->delete_page(page_num);
   doc->save_as_djvm(argv[2]);
}

static void
list(int argc, char ** argv)
      // djvm -l[ist] <doc.djvu>
{
   if (argc!=3) { usage(); exit(1); }
   StdioByteStream str(argv[2], "rb");
   DjVmFile djvm_file;
   djvm_file.read(str);
   GP<DjVmDir0> djvm_dir=djvm_file.get_djvm_dir();
   int files=djvm_dir->get_files_num();
   for(int file_num=0;file_num<files;file_num++)
   {
      GP<DjVmDir0::FileRec> f=djvm_dir->get_file(file_num);
      printf("%d\t[%s]\t%s\n", f->size, f->iff_file ? "IFF file" :
	     "RAW file", (const char *) f->name);
   }
}

int
main(int argc, char ** argv)
{
   char * ptr;
   for(progname=ptr=argv[0];*ptr;ptr++)
      if (*ptr=='/') progname=ptr+1;
   
   TRY {
      if (argc<2) { usage(); exit(1); }
      if (!strncmp(argv[1], "-c", 2)) create(argc, argv);
      else if (!strncmp(argv[1], "-e", 2)) expand(argc, argv);
      else if (!strncmp(argv[1], "-i", 2)) insert(argc, argv);
      else if (!strncmp(argv[1], "-d", 2)) del(argc, argv);
      else if (!strncmp(argv[1], "-l", 2)) list(argc, argv);
      else { usage(); exit(1); }
   } CATCH(exc) {
      exc.perror();
      exit(1);
   } ENDCATCH;
   exit(0);
}
