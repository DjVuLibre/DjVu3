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
//C- $Id: djvm.cpp,v 1.11 2000-05-01 16:15:23 bcr Exp $

/** @name djvm

    {\bf Synopsis}\\
    \begin{verbatim}
        djvm [options] <djvu_doc_name> [djvufiles] [pagenum]
    \end{verbatim}

    {\bf Description} ---
    File #"djvm.cpp"# and program #djvm# illustrate how class \Ref{DjVuDocEditor}
    can be used to create and modify DjVu multipage documents. The program
    demonstrates how to pack several DjVu single-page files together,
    how to remove pages from a multipage document or how to insert new.
    
    {\bf Arguments} ---
    Depending on the task to be performed, the number and types of arguments
    differ:
    \begin{itemize}
       \item To create a new document

             #djvm -c[reate] <doc.djvu> <page_1.djvu> ... <page_n.djvu>#

	     This will package pages represented by files #<page_1.djvu>#
	     ... #<page_n.djvu># into #BUNDLED# multipage DjVu document
	     #<doc.djvu># (see \Ref{DjVuDocument} for the explanation of
	     the #BUNDLED# format). The specified page files may include other
	     files (by means of #INCL# chunks). They will also be packed
	     into the document.

       \item To insert a page

       	     #djvm -i[nsert] <doc.djvu> <page.djvu> [<page_num>]#

	     This will insert page represented by file #<page.djvu># into
	     document #<doc.djvu># as page number #<page_num># (page
	     numbering starts from #1#). Negative or missing #<page_num>#
	     means to append the page.

	     The #<page.djvu># file can actually be another
	     multipage DjVu document. In this cases, all pages from that
	     document will be inserted into #<doc.djvu># starting from
	     page #<page_num>#.

       \item To delete a page

             #djvm -d[elete] <doc.djvu> <page_num>#

	     This will remove page number #<page_num># from document
	     #<doc.djvu>#. If there is only one page left in #<doc.djvu>#
	     after deletion, it will automatically be converted to a
	     single page DjVu file format. Page numbering starts from #1#.

       \item To view document contents

             #djvm -l[ist] <doc.djvu>#

	     This will list all files composing the given document #<doc.djvu>#.
	     The files list includes the names of page files plus names
	     of any files included into the page files by means of
	     #INCL# chunk.
    \end{itemize}

    @memo
    DjVu multipage documents creator.
    @author
    Andrei Erofeev <eaf@geocities.com>
    @version
    #$Id: djvm.cpp,v 1.11 2000-05-01 16:15:23 bcr Exp $# */
//@{
//@}

#include "GException.h"
#include "DjVuDocEditor.h"
#include "GOS.h"

#include <stdlib.h>
#include <iostream.h>

static char * progname;

static void
usage(void)
{
   cerr << "\
DJVM -- DjVu multipage document manipulation utility\n\
   Copyright (c) AT&T 1999.  All rights reserved\n\
\n\
Usage:\n\
   To compose a multipage document:\n\
      " << progname << " -c[reate] <doc.djvu> <page_1.djvu> ... <page_n.djvu>\n\
      where <doc.djvu> is the name of the BUNDLED document to be\n\
      created, <page_n.djvu> are the names of the page files to\n\
      be packed together.\n\
\n\
   To insert a new page into an existing document:\n\
      " << progname << " -i[nsert] <doc.djvu> <page.djvu> [<page_num>]\n\
      where <doc.djvu> is the name of the BUNDLED DjVu document to be\n\
      modified, <page.djvu> is the name of the single-page DjVu document\n\
      file to be inserted as page <page_num> (page numbers start from 1).\n\
      Negative or omitted <page_num> means to append the page.\n\
      <page.djvu> can be another multipage DjVu document, in which case\n\
      all pages of that document will be inserted into <doc.djvu>\n\
      starting starting at page <page_num>\n\
\n\
   To delete a page from an existing document:\n\
      " << progname << " -d[elete] <doc.djvu> <page_num>\n\
      where <doc.djvu> is the name of the docyment to be modified\n\
      and <page_num> is the number of the page to be deleted\n\
\n\
   To list document contents:\n\
      " << progname << " -l[ist] <doc.djvu>\n\
\n\
   Pages being inserted may reference other files by means of INCL chunks.\n\
   Moreover, files shared between pages will be stored into the document\n\
   only once.\n\
\n";
}

static void
create(int argc, char ** argv)
      // djvm -c[reate] <doc.djvu> <page_1.djvu> ... <page_n.djvu>
      // doc.djvu will be overwritten
{
   if (argc<4) { usage(); exit(1); }

      // Initialize the DjVuDocEditor class
   GP<DjVuDocEditor> doc=new DjVuDocEditor();
   doc->init();

      // Insert pages
   GList<GString> list;
   for(int i=3;i<argc;i++)
      list.append(argv[i]);
   doc->insert_group(list);

      // Save in BUNDLED format
   doc->save_as(argv[2], true);
}

static void
insert(int argc, char ** argv)
      // djvm -i[nsert] <doc.djvu> <page.djvu> <page_num>
{
   if (argc!=4 && argc!=5) { usage(); exit(1); }

      // Initialize DjVuDocEditor class
   GP<DjVuDocEditor> doc=new DjVuDocEditor();
   doc->init(argv[2]);

      // Insert page
   int page_num=-1;
   if (argc==5) page_num=atoi(argv[4])-1;
   doc->insert_page(argv[3], page_num);

      // Save the document
   doc->save();
}

static void
del(int argc, char ** argv)
      // djvm -d[elete] <doc.djvu> <page_num>
{
   if (argc!=4) { usage(); exit(1); }

      // Initialize DjVuDocEditor class
   GP<DjVuDocEditor> doc=new DjVuDocEditor();
   doc->init(argv[2]);

      // Delete the page
   int page_num=atoi(argv[3])-1;
   if (page_num<0) { fprintf(stderr, "Page number must be positive.\n"); exit(1); }
   doc->remove_page(page_num);

      // Save the document
   doc->save();
}

static void
list(int argc, char ** argv)
      // djvm -l[ist] <doc.djvu>
{
   if (argc!=3) { usage(); exit(1); }

   GP<DjVmDoc> doc=new DjVmDoc();
   doc->read(argv[2]);
   
   GP<DjVmDir> dir=doc->get_djvm_dir();
   if (dir)
   {
      GPList<DjVmDir::File> files_list=dir->get_files_list();
      printf("Size     Type        Name\n");
      printf("--------------------------\n");
      for(GPosition pos=files_list;pos;++pos)
      {
	 GP<DjVmDir::File> file=files_list[pos];
	 printf("%6d   ", file->size);
	 if (file->is_page())
	 {
	    char buffer[128];
	    sprintf(buffer, "PAGE #%d", file->get_page_num());
	    printf("%s", buffer);
	    for(int i=strlen(buffer);i<9;i++)
	       putchar(' ');
	 } else if (file->is_include())
	    printf("%s", "INCLUDE  ");
	 } else if (file->is_shared_anno())
	    printf("%s", "SHARED_ANNO  ");
	 else if (file->is_thumbnails())
	    printf("%s", "THUMBNAIL");
	 else printf("%s", "UNKNOWN  ");
	 printf("   %s\n", (const char *) file->name);
      }
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
