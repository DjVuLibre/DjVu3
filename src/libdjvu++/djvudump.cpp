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
//C- $Id: djvudump.cpp,v 1.10 2000-02-01 22:49:54 leonb Exp $



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
    @author
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id: djvudump.cpp,v 1.10 2000-02-01 22:49:54 leonb Exp $# */
//@{
//@}

#include <stdio.h>
#include <ctype.h>
#include "GException.h"
#include "GString.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "DjVuImage.h"
#include "DjVmDir.h"


struct DjVmInfo
{
  GP<DjVmDir> dir;
  GPMap<int,DjVmDir::File> map;
};



// ---------- ROUTINES FOR SUMMARIZING CHUNK DATA


void
display_djvu_info(IFFByteStream &iff, GString, size_t size, DjVmInfo& )
{
  struct DjVuInfo info;
  info.decode(iff);
  if (size >= 4)
    printf("DjVu %dx%d", info.width, info.height);
  if (size >= 5)
    printf(", v%d", info.version);
  if (size >= 8)
    printf(", %d dpi", info.dpi);
  if (size >= 8)
    printf(", gamma=%3.1f", info.gamma);
}

void
display_djbz(IFFByteStream &iff, GString, size_t, DjVmInfo& )
{
  printf("JB2 shared dictionary");
}

void
display_fgbz(IFFByteStream &iff, GString, size_t, DjVmInfo& )
{
  printf("JB2 colors data");
}

void
display_sjbz(IFFByteStream &iff, GString, size_t, DjVmInfo& )
{
  printf("JB2 bilevel data");
}

void
display_smmr(IFFByteStream &iff, GString, size_t, DjVmInfo& )
{
  printf("G4/MMR stencil data");
}

void
display_iw4(IFFByteStream &iff, GString, size_t, DjVmInfo& )
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
      printf("IW4 data #%d, %d slices", primary.serial+1, primary.slices);
      if (primary.serial==0)
        if (iff.readall((void*)&secondary, sizeof(secondary)) == sizeof(secondary))
          {
            printf(", v%d.%d (%s), %dx%d", secondary.major&0x7f, secondary.minor,
                   (secondary.major & 0x80 ? "b&w" : "color"),
                   (secondary.xhi<<8)+secondary.xlo,
                   (secondary.yhi<<8)+secondary.ylo  );
          }
    }
}

void
display_djvm_dirm(IFFByteStream & iff, GString head, size_t, DjVmInfo& djvminfo)
{
  GP<DjVmDir> dir = new DjVmDir();
  dir->decode(iff);
  GPList<DjVmDir::File> list = dir->get_files_list();
  if (dir->is_indirect())
    {
      printf("Document directory (indirect, %d files %d pages)", 
	     dir->get_files_num(), dir->get_pages_num());
      for (GPosition p=list; p; ++p)
	printf("\n%s%s -> %s", (const char*)head, 
	       (const char*)list[p]->id, (const char*)list[p]->name );
    }
  else
    {
      printf("Document directory (bundled, %d files %d pages)", 
	     dir->get_files_num(), dir->get_pages_num());
      djvminfo.dir = dir;
      djvminfo.map.empty();
      for (GPosition p=list; p; ++p)
	djvminfo.map[list[p]->offset] = list[p];
    }
}

void
display_th44(IFFByteStream & iff, GString, size_t, DjVmInfo & djvminfo)
{
  static int th_page_num=0;
  if (djvminfo.dir && th_page_num>=djvminfo.dir->get_pages_num())
     printf("Thumbnails for page ???");
  else printf("Thumbnails for page %d", th_page_num+1);
  th_page_num++;
}

void
display_incl(IFFByteStream & iff, GString, size_t, DjVmInfo& )
{
   GString name;
   char ch;
   while(iff.read(&ch, 1) && ch!='\n')
     name += ch;
   printf("Indirection chunk --> {%s}", (const char *) name);
}

void
display_anno(IFFByteStream &iff, GString, size_t, DjVmInfo& )
{
   printf("Page annotation");
   GString id;
   iff.short_id(id);
   if (id=="ANTa" || id=="ANTz")
     printf(" (hyperlinks, etc.)");
   if (id=="TXTa" || id=="TXTz")
     printf(" (text, etc.)");
}

struct displaysubr
{
  const char *id;
  void (*subr)(IFFByteStream &, GString, size_t, DjVmInfo& );
} 
disproutines[] = 
{
  { "DJVU.INFO", display_djvu_info },
  { "DJVU.Smmr", display_smmr },
  { "DJVU.Sjbz", display_sjbz },
  { "DJVU.Djbz", display_djbz },
  { "DJVU.FG44", display_iw4 },
  { "DJVU.BG44", display_iw4 },
  { "DJVU.FGbz", display_fgbz },
  { "DJVI.Sjbz", display_sjbz },
  { "DJVI.Djbz", display_djbz },
  { "DJVI.FGbz", display_fgbz },
  { "DJVI.FG44", display_iw4 },
  { "DJVI.BG44", display_iw4 },
  { "BM44.BM44", display_iw4 },
  { "PM44.PM44", display_iw4 },
  { "DJVM.DIRM", display_djvm_dirm },
  { "THUM.TH44", display_th44 },
  { "INCL", display_incl },
  { "ANTa", display_anno },
  { "ANTz", display_anno },
  { "TXTa", display_anno },
  { "TXTz", display_anno },
  { 0, 0 },
};




// ---------- ROUTINES FOR DISPLAYING CHUNK STRUCTURE

void
display_chunks(IFFByteStream &iff, const GString &head)
{
  size_t size;
  GString id, fullid;
  GString head2 = head + "  ";
  GPMap<int,DjVmDir::File> djvmmap;
  DjVmInfo djvminfo;
  int rawoffset;
  
  while ((size = iff.get_chunk(id, &rawoffset)))
    {
      GString msg;
      msg.format("%s%s [%d] ", (const char *)head, (const char *)id, size);
      printf("%s", (const char *)msg);
      // Display DJVM is when adequate
      if (djvminfo.dir)
	{
	  GP<DjVmDir::File> rec = djvminfo.map[rawoffset];
	  printf("{%s}", rec ? (const char*)rec->id : "???");
	}
      // Test chunk type
      iff.full_id(fullid);
      for (int i=0; disproutines[i].id; i++)
        if (fullid == disproutines[i].id || id == disproutines[i].id)
          {
            int n = msg.length();
	    while (n++ < 22) putchar(' ');
	    if (!iff.composite()) printf("    ");
            (*disproutines[i].subr)(iff, head2, size, djvminfo);
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
          "DJVUDUMP -- Describes IFF85 files\n"
          "  Copyright (c) AT&T 1999.  All rights reserved\n"
          "Usage: djvudump <iff_filenames>\n" );
  exit(1);
}

int 
main(int argc, char **argv)
{
  TRY
    {
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
