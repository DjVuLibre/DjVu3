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
//C- $Id: DjVuDumpHelper.cpp,v 1.6 2000-09-21 22:37:06 bcr Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuDumpHelper.h"
#include "DjVmDir.h"
#include "DjVuInfo.h"
#include "IFFByteStream.h"

#ifdef putchar
#undef putchar
#endif

struct DjVmInfo
{
  GP<DjVmDir> dir;
  GPMap<int,DjVmDir::File> map;
};

static void
printf(ByteStream & str, const char * fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   GString tmp;
   tmp.format(fmt, args);
   str.writall((const char *) tmp, tmp.length());
}

inline static void
putchar(ByteStream & str, char ch)
{
   str.write(&ch, 1);
}

// ---------- ROUTINES FOR SUMMARIZING CHUNK DATA

static void
display_djvu_info(ByteStream & out_str, IFFByteStream &iff,
		  GString, size_t size, DjVmInfo&, int)
{
  DjVuInfo info;
  info.decode(iff);
  if (size >= 4)
    printf(out_str, "DjVu %dx%d", info.width, info.height);
  if (size >= 5)
    printf(out_str, ", v%d", info.version);
  if (size >= 8)
    printf(out_str, ", %d dpi", info.dpi);
  if (size >= 8)
    printf(out_str, ", gamma=%3.1f", info.gamma);
}

static void
display_djbz(ByteStream & out_str, IFFByteStream &iff,
	     GString, size_t, DjVmInfo&, int)
{
  printf(out_str, "JB2 shared dictionary");
}

static void
display_fgbz(ByteStream & out_str, IFFByteStream &iff,
	     GString, size_t, DjVmInfo&, int)
{
  printf(out_str, "JB2 colors data");
}

static void
display_sjbz(ByteStream & out_str, IFFByteStream &iff,
	     GString, size_t, DjVmInfo&, int)
{
  printf(out_str, "JB2 bilevel data");
}

static void
display_smmr(ByteStream & out_str, IFFByteStream &iff,
	     GString, size_t, DjVmInfo&, int)
{
  printf(out_str, "G4/MMR stencil data");
}

static void
display_iw4(ByteStream & out_str, IFFByteStream &iff,
	    GString, size_t, DjVmInfo&, int)
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
      printf(out_str, "IW4 data #%d, %d slices", primary.serial+1, primary.slices);
      if (primary.serial==0)
        if (iff.readall((void*)&secondary, sizeof(secondary)) == sizeof(secondary))
          {
            printf(out_str, ", v%d.%d (%s), %dx%d", secondary.major&0x7f, secondary.minor,
                   (secondary.major & 0x80 ? "b&w" : "color"),
                   (secondary.xhi<<8)+secondary.xlo,
                   (secondary.yhi<<8)+secondary.ylo  );
          }
    }
}

static void
display_djvm_dirm(ByteStream & out_str, IFFByteStream & iff,
		  GString head, size_t, DjVmInfo& djvminfo, int)
{
  GP<DjVmDir> dir = new DjVmDir();
  dir->decode(iff);
  GPList<DjVmDir::File> list = dir->get_files_list();
  if (dir->is_indirect())
    {
      printf(out_str, "Document directory (indirect, %d files %d pages)", 
	     dir->get_files_num(), dir->get_pages_num());
      for (GPosition p=list; p; ++p)
	printf(out_str, "\n%s%s -> %s", (const char*)head, 
	       (const char*)list[p]->id, (const char*)list[p]->name );
    }
  else
    {
      printf(out_str, "Document directory (bundled, %d files %d pages)", 
	     dir->get_files_num(), dir->get_pages_num());
      djvminfo.dir = dir;
      djvminfo.map.empty();
      for (GPosition p=list; p; ++p)
	djvminfo.map[list[p]->offset] = list[p];
    }
}

static void
display_th44(ByteStream & out_str, IFFByteStream & iff,
	     GString, size_t, DjVmInfo & djvminfo, int counter)
{
   int start_page=-1;
   if (djvminfo.dir)
   {
      GPList<DjVmDir::File> files_list=djvminfo.dir->get_files_list();
      for(GPosition pos=files_list;pos;++pos)
      {
	 GP<DjVmDir::File> frec=files_list[pos];
	 if (iff.tell()>=frec->offset &&
	     iff.tell()<frec->offset+frec->size)
	 {
	    while(pos && !files_list[pos]->is_page())
	       ++pos;
	    if (pos)
	       start_page=files_list[pos]->get_page_num();
	    break;
	 }
      }
   }
   if (start_page>=0)
      printf(out_str, "Thumbnail icon for page %d", start_page+counter+1);
   else
      printf(out_str, "Thumbnail icon");
}

static void
display_incl(ByteStream & out_str, IFFByteStream & iff,
	     GString, size_t, DjVmInfo&, int)
{
   GString name;
   char ch;
   while(iff.read(&ch, 1) && ch!='\n')
     name += ch;
   printf(out_str, "Indirection chunk --> {%s}", (const char *) name);
}

static void
display_anno(ByteStream & out_str, IFFByteStream &iff,
	     GString, size_t, DjVmInfo&, int)
{
   printf(out_str, "Page annotation");
   GString id;
   iff.short_id(id);
   if (id=="ANTa" || id=="ANTz")
     printf(out_str, " (hyperlinks, etc.)");
   if (id=="TXTa" || id=="TXTz")
     printf(out_str, " (text, etc.)");
}

struct displaysubr
{
  const char *id;
  void (*subr)(ByteStream &, IFFByteStream &, GString,
	       size_t, DjVmInfo&, int counter);
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

static void
display_chunks(ByteStream & out_str, IFFByteStream &iff,
	       const GString &head, DjVmInfo djvminfo)
{
  size_t size;
  GString id, fullid;
  GString head2 = head + "  ";
  GPMap<int,DjVmDir::File> djvmmap;
  int rawoffset;
  GMap<GString, int> counters;

  while ((size = iff.get_chunk(id, &rawoffset)))
    {
      if (!counters.contains(id)) counters[id]=0;
      else counters[id]++;
      
      GString msg;
      msg.format("%s%s [%d] ", (const char *)head, (const char *)id, size);
      printf(out_str, "%s", (const char *)msg);
      // Display DJVM is when adequate
      if (djvminfo.dir)
	{
	  GP<DjVmDir::File> rec = djvminfo.map[rawoffset];
	  if (rec)
	     printf(out_str, "{%s}", (const char*) rec->id);
	}
      // Test chunk type
      iff.full_id(fullid);
      for (int i=0; disproutines[i].id; i++)
        if (fullid == disproutines[i].id || id == disproutines[i].id)
          {
            int n = msg.length();
	    while (n++ < 14+(int) head.length()) putchar(out_str, ' ');
	    if (!iff.composite()) printf(out_str, "    ");
            (*disproutines[i].subr)(out_str, iff, head2,
				    size, djvminfo, counters[id]);
            break;
          }
      // Default display of composite chunk
      printf(out_str, "\n");
      if (iff.composite())
	display_chunks(out_str, iff, head2, djvminfo);
      // Terminate
      iff.close_chunk();
    }
}

GP<MemoryByteStream>
DjVuDumpHelper::dump(const GP<DataPool> & pool)
{
   GP<ByteStream> str=pool->get_stream();
   return dump(*str);
}

GP<MemoryByteStream>
DjVuDumpHelper::dump(ByteStream & str)
{
   GP<MemoryByteStream> out_str=new MemoryByteStream;
   GString head="  ";
   IFFByteStream iff(str);
   DjVmInfo djvminfo;
   display_chunks(*out_str, iff, head, djvminfo);
   return out_str;
}