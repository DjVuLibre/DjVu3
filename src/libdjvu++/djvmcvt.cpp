/*****************************************************************************
 *
 *   $Revision: 1.1 $
 *   $Date: 1999-08-18 14:44:10 $
 *   @(#) $Id: djvmcvt.cpp,v 1.1 1999-08-18 14:44:10 eaf Exp $
 *
 *****************************************************************************/

static char RCSVersion[]="@(#) $Id: djvmcvt.cpp,v 1.1 1999-08-18 14:44:10 eaf Exp $";

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
Usage:\n\
\n\
   To convert any DjVu document into the new BUNDLED format:\n\
	%s -b[undled] <file_in> <file_out>\n\
   To convert any DjVu document into the new INDIRECT format:\n\
	%s -i[ndirect] <file_in> <dir_out> <idx_fname>\n\
   To extract a chunk into a separate file:\n\
\n", progname, progname);
}

static void
do_bundled(int argc, char ** argv)
      // <progname> -b[undled] <file_in> <file_out>
{
   if (argc!=4) { Usage(); exit(1); }

   DjVuDocument doc(GOS::filename_to_url(argv[2]));
   GP<DjVmDoc> djvm_doc=doc.get_djvm_doc();
   StdioByteStream str(argv[3], "wb");
   djvm_doc->write(str);
}

static void
do_indirect(int argc, char ** argv)
      // <progname> -i[ndirect] <file_in> <dir_out> <idx_fname>
{
   if (argc!=5) { Usage(); exit(1); }

   DjVuDocument doc(GOS::filename_to_url(argv[2]));
   GP<DjVmDoc> djvm_doc=doc.get_djvm_doc();
   djvm_doc->expand(argv[3], argv[4]);
}

int main(int argc, char ** argv)
{
   char * ptr;
   for(progname=ptr=argv[0];*ptr;ptr++)
      if (*ptr=='/') progname=ptr+1;

   if (argc<2) { Usage(); exit(1); };

   bool bundled=true;
   TRY {
      if (!strncmp(argv[1], "-b", 2)) bundled=true;
      else if (!strncmp(argv[1], "-i", 2)) bundled=false;
      else { Usage(); exit(1); };

      if (bundled) do_bundled(argc, argv);
      else do_indirect(argc, argv);
   } CATCH(exc) {
      fprintf(stderr, "%s\n", exc.get_cause());
      exit(1);
   } ENDCATCH;

   return 0;
}
