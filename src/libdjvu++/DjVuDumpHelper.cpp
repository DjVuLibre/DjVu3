//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: DjVuDumpHelper.cpp,v 1.20 2001-04-12 00:24:59 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuDumpHelper.h"
#include "DataPool.h"
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

inline static void
putchar(ByteStream & str, char ch)
{
   str.write(&ch, 1);
}

// ---------- ROUTINES FOR SUMMARIZING CHUNK DATA

static void
display_djvu_info(ByteStream & out_str, IFFByteStream &iff,
		  GUTF8String, size_t size, DjVmInfo&, int)
{
  GP<DjVuInfo> ginfo=DjVuInfo::create();
  DjVuInfo &info=*ginfo;
  info.decode(iff);
  if (size >= 4)
    out_str.format( "DjVu %dx%d", info.width, info.height);
  if (size >= 5)
    out_str.format( ", v%d", info.version);
  if (size >= 8)
    out_str.format( ", %d dpi", info.dpi);
  if (size >= 8)
    out_str.format( ", gamma=%3.1f", info.gamma);
}

static void
display_djbz(ByteStream & out_str, IFFByteStream &iff,
	     GUTF8String, size_t, DjVmInfo&, int)
{
  out_str.format( "JB2 shared dictionary");
}

static void
display_fgbz(ByteStream & out_str, IFFByteStream &iff,
	     GUTF8String, size_t, DjVmInfo&, int)
{
  out_str.format( "JB2 colors data");
}

static void
display_sjbz(ByteStream & out_str, IFFByteStream &iff,
	     GUTF8String, size_t, DjVmInfo&, int)
{
  out_str.format( "JB2 bilevel data");
}

static void
display_smmr(ByteStream & out_str, IFFByteStream &iff,
	     GUTF8String, size_t, DjVmInfo&, int)
{
  out_str.format( "G4/MMR stencil data");
}

static void
display_iw4(ByteStream & out_str, IFFByteStream &iff,
	    GUTF8String, size_t, DjVmInfo&, int)
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
      out_str.format( "IW4 data #%d, %d slices", primary.serial+1, primary.slices);
      if (primary.serial==0)
        if (iff.readall((void*)&secondary, sizeof(secondary)) == sizeof(secondary))
          {
            out_str.format( ", v%d.%d (%s), %dx%d", secondary.major&0x7f, secondary.minor,
                   (secondary.major & 0x80 ? "b&w" : "color"),
                   (secondary.xhi<<8)+secondary.xlo,
                   (secondary.yhi<<8)+secondary.ylo  );
          }
    }
}

static void
display_djvm_dirm(ByteStream & out_str, IFFByteStream & iff,
		  GUTF8String head, size_t, DjVmInfo& djvminfo, int)
{
  GP<DjVmDir> dir = DjVmDir::create();
  dir->decode(iff.get_bytestream());
  GPList<DjVmDir::File> list = dir->get_files_list();
  if (dir->is_indirect())
  {
    out_str.format( "Document directory (indirect, %d files %d pages)", 
	                  dir->get_files_num(), dir->get_pages_num());
    for (GPosition p=list; p; ++p)
      out_str.format( "\n%s%s -> %s", (const char*)head, 
                      (const char*)list[p]->id, (const char*)list[p]->name );
  }
  else
  {
    out_str.format( "Document directory (bundled, %d files %d pages)", 
	                  dir->get_files_num(), dir->get_pages_num());
    djvminfo.dir = dir;
    djvminfo.map.empty();
    for (GPosition p=list; p; ++p)
      djvminfo.map[list[p]->offset] = list[p];
  }
}

static void
display_th44(ByteStream & out_str, IFFByteStream & iff,
	     GUTF8String, size_t, DjVmInfo & djvminfo, int counter)
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
      out_str.format( "Thumbnail icon for page %d", start_page+counter+1);
   else
      out_str.format( "Thumbnail icon");
}

static void
display_incl(ByteStream & out_str, IFFByteStream & iff,
	     GUTF8String, size_t, DjVmInfo&, int)
{
   GUTF8String name;
   char ch;
   while(iff.read(&ch, 1) && ch!='\n')
     name += ch;
   out_str.format( "Indirection chunk --> {%s}", (const char *) name);
}

static void
display_anno(ByteStream & out_str, IFFByteStream &iff,
	     GUTF8String, size_t, DjVmInfo&, int)
{
   out_str.format( "Page annotation");
   GUTF8String id;
   iff.short_id(id);
   out_str.format( " (hyperlinks, etc.)");
}

static void
display_text(ByteStream & out_str, IFFByteStream &iff,
	     GUTF8String, size_t, DjVmInfo&, int)
{
   out_str.format( "Hidden text");
   GUTF8String id;
   iff.short_id(id);
   out_str.format( " (text, etc.)");
}

struct displaysubr
{
  const char *id;
  void (*subr)(ByteStream &, IFFByteStream &, GUTF8String,
	       size_t, DjVmInfo&, int counter);
};
 
static displaysubr disproutines[] = 
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
  { "TXTa", display_text },
  { "TXTz", display_text },
  { 0, 0 },
};

// ---------- ROUTINES FOR DISPLAYING CHUNK STRUCTURE

static void
display_chunks(ByteStream & out_str, IFFByteStream &iff,
	       const GUTF8String &head, DjVmInfo djvminfo)
{
  size_t size;
  GUTF8String id, fullid;
  GUTF8String head2 = head + "  ";
  GPMap<int,DjVmDir::File> djvmmap;
  int rawoffset;
  GMap<GUTF8String, int> counters;
  
  while ((size = iff.get_chunk(id, &rawoffset)))
  {
    if (!counters.contains(id)) counters[id]=0;
    else counters[id]++;
    
    GUTF8String msg;
    msg.format("%s%s [%d] ", (const char *)head, (const char *)id, size);
    out_str.format( "%s", (const char *)msg);
    // Display DJVM is when adequate
    if (djvminfo.dir)
    {
      GP<DjVmDir::File> rec = djvminfo.map[rawoffset];
      if (rec)
        out_str.format( "{%s}", (const char*) rec->id);
    }
    // Test chunk type
    iff.full_id(fullid);
    for (int i=0; disproutines[i].id; i++)
      if (fullid == disproutines[i].id || id == disproutines[i].id)
      {
        int n = msg.length();
        while (n++ < 14+(int) head.length()) putchar(out_str, ' ');
        if (!iff.composite()) out_str.format( "    ");
        (*disproutines[i].subr)(out_str, iff, head2,
                                size, djvminfo, counters[id]);
        break;
      }
      // Default display of composite chunk
      out_str.format( "\n");
      if (iff.composite())
        display_chunks(out_str, iff, head2, djvminfo);
      // Terminate
      iff.close_chunk();
  }
}

GP<ByteStream>
DjVuDumpHelper::dump(const GP<DataPool> & pool)
{
   return dump(pool->get_stream());
}

GP<ByteStream>
DjVuDumpHelper::dump(GP<ByteStream> gstr)
{
   GP<ByteStream> out_str=ByteStream::create();
   GUTF8String head="  ";
   GP<IFFByteStream> iff=IFFByteStream::create(gstr);
   DjVmInfo djvminfo;
   display_chunks(*out_str, *iff, head, djvminfo);
   return out_str;
}

