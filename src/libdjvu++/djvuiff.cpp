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
//C- $Id: djvuiff.cpp,v 1.1 1999-06-22 13:54:42 leonb Exp $



/** @name djvuinfo

    {\bf Synopsis}
    \begin{verbatim}
        djvuinfo <... iff_file_names ...>
    \end{verbatim}

    {\bf Description} ---
    File #"djvuinfo.cpp"# uses the facilities provided by \Ref{IFFByteStream.h}
    to display an indented representation of the chunk structure of an
    ``EA IFF 85'' file.  Each line represent contains a chunk ID followed by the
    chunk size.  Additional information about the chunk is provided when
    program #djvuinfo.cpp# recognizes the chunk name and knows how to summarize
    the chunk data. In addition, when it recognizes that the file is a DjVu
    multipage document (an archive of pages), it prints page names too.
    Lines are indented in order to reflect the hierarchical
    structure of the IFF files.

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
    @author
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id: djvuiff.cpp,v 1.1 1999-06-22 13:54:42 leonb Exp $# */
//@{
//@}

#include <stdio.h>
#include <ctype.h>
#include "GException.h"
#include "GString.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "DjVuImage.h"
#include "DjVmDir0.h"
#include "ATTLicense.h"


// ---------- ROUTINES FOR SUMMARIZING CHUNK DATA

static GP<DjVmDir0> dir0;

void
display_djvu_info(IFFByteStream &iff, const GString &head, size_t size)
{
  struct DjVuInfo info;
  info.decode(iff);
  if (size >= 4)
    printf("%dx%d", info.width, info.height);
  if (size >= 5)
    printf(", version %d", info.version);
  if (size >= 8)
    printf(", %d dpi", info.dpi);
  if (size >= 8)
    printf(", gamma %3.1f", info.gamma);
}

void
display_sjbz_info(IFFByteStream &iff, const GString &head, size_t size)
{
  printf("JB2 data, no header");
}

void
display_iw4_info(IFFByteStream &iff, const GString &head, size_t size)
{
  struct PrimaryHeader {
    unsigned char serial;
    unsigned char slices;
  } primary;

  struct SecondaryHeader {
    unsigned char major;
    unsigned char minor;
    unsigned char xhi, xlo;
    unsigned char yhi, ylo;
  } secondary;
  
  if (iff.readall((void*)&primary, sizeof(primary)) == sizeof(primary))
    {
      printf("#%d - %d slices ", primary.serial+1, primary.slices);
      if (primary.serial==0)
        if (iff.readall((void*)&secondary, sizeof(secondary)) == sizeof(secondary))
          {
            printf("- v%d.%d (%s) - %dx%d", secondary.major&0x7f, secondary.minor,
                   (secondary.major & 0x80 ? "b&w" : "color"),
                   (secondary.xhi<<8)+secondary.xlo,
                   (secondary.yhi<<8)+secondary.ylo  );
          }
    }
}

void
display_dir0_info(IFFByteStream & iff, const GString &, size_t)
{
   dir0=new DjVmDir0();
   dir0->decode(iff);
   printf("Document directory (%d files)", dir0->get_files_num());
}

void
display_djvi_info(IFFByteStream & iff, const GString &, size_t)
{
   if (dir0)
   {
      long offset=iff.tell()-12;
      int files=dir0->get_files_num();
      for(int i=0;i<files;i++)
      {
	 GP<DjVmDir0::FileRec> file=dir0->get_file(i);
	 if (file->offset==offset)
	 {
	    printf("Directory name: \"%s\"", (const char *) file->name);
	    break;
	 }
      }
   }
}

void
display_ndir_info(IFFByteStream &, const GString &, size_t)
{
   printf("Navigation directory");
}

void
display_incl_info(IFFByteStream & iff, const GString &, size_t)
{
   GString name;
   char ch;
   while(iff.read(&ch, 1))
      if (!isspace(ch))
      {
	 name+=ch;
	 break;
      }
   while(iff.read(&ch, 1) && ch!='\n') name+=ch;
   
   printf("Indirection chunk (%s)", (const char *) name);
}

void
display_anta_info(IFFByteStream &, const GString &, size_t)
{
   printf("Page annotation");
}

struct displaysubr
{
  const char *id;
  void (*subr)(IFFByteStream &iff, const GString &head, size_t size);
} 
disproutines[] = 
{
  { "DJVU.INFO", display_djvu_info },
  { "DJVU.Sjbz", display_sjbz_info },
  { "DJVU.FG44", display_iw4_info },
  { "DJVU.BG44", display_iw4_info },
  { "BM44.BM44", display_iw4_info },
  { "PM44.PM44", display_iw4_info },
  { "DJVM.DIR0", display_dir0_info },
  { "FORM:DJVI", display_djvi_info },
  { "FORM:DJVU", display_djvi_info },
  { "FORM:PM44", display_djvi_info },
  { "FORM:BM44", display_djvi_info },
  { "NDIR", display_ndir_info },
  { "INCL", display_incl_info },
  { "INCF", display_incl_info },
  { "ANTa", display_anta_info },
  { 0, 0 },
};


// ---------- ROUTINES FOR DISPLAYING CHUNK STRUCTURE

void
display_chunks(IFFByteStream &iff, const GString &head)
{
  size_t size;
  GString id, fullid;
  GString head2 = head + "  ";
  
  while ((size = iff.get_chunk(id)))
    {
      GString msg;
      msg.format("%s%s [%d] ", (const char *)head, (const char *)id, size);
      printf("%s", (const char *)msg);
      // Test chunk type
      iff.full_id(fullid);
      for (int i=0; disproutines[i].id; i++)
        if (fullid == disproutines[i].id || id == disproutines[i].id)
          {
            int n = msg.length();
	    while (n++ < 26) putchar(' ');
	    if (!iff.composite()) printf("    ");
            (*disproutines[i].subr)(iff, head2, size);
            break;
          }
      // Default display of composite chunk
      printf("\n");
      if (iff.composite())
        display_chunks(iff, head2);
      // Terminate
      iff.close_chunk();
    }
}




void
display(const char *s)
{
  StdioByteStream bs(s,"rb");
  IFFByteStream iff(bs);
  printf("%s:\n", s);
  GString head = "  ";
  display_chunks(iff, head);
}


void
usage()
{
  fprintf(stderr,
          "DJVUINFO -- Describes IFF85 files\n"
          "%s\nUsage: djvuinfo <iff_filenames>\n",
          ATTLicense::get_usage_text() );
  exit(1);
}

int 
main(int argc, char **argv)
{
  TRY
    {
      ATTLicense::process_cmdline(argc,argv);
      if (argc<=1)
        usage();
      for (int i=1; i<argc; i++)
        display(argv[i]);
    }
  CATCH(ex)
    {
      ex.perror();
      exit(1);
    }
  ENDCATCH;
  return 0;
}
