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
//C- $Id: DjVmDir.cpp,v 1.5 1999-09-23 19:05:24 eaf Exp $


#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVmDir.h"
#include "BSByteStream.h"
#include "debug.h"

#include <ctype.h>

/* Directory file format

   char8		(version number | (bundled or not) << 7)
   char16		number of records
   for every record (only if bundled)
      char32		offset
      char24		size
   bzz compressed block:
      for every record
         char8 flags
      for every record
         ASCIIZ id
         ASCIIZ name, if it's different from id (see flags)
         ASCIIZ title, if it's different from id (see flags)
*/

void 
DjVmDir::decode(ByteStream & str)
{
   DEBUG_MSG("DjVmDir::decode(): decoding contents of 'DIRM' chunk...\n");
   DEBUG_MAKE_INDENT(3);
   
   GCriticalSectionLock lock(&class_lock);

   GPosition pos;

   files_list.empty();
   page2file.resize(-1);
   name2file.empty();
   id2file.empty();
   title2file.empty();

   int ver=str.read8();
   bool bundled=(ver & 0x80)!=0;
   ver&=0x7f;

   DEBUG_MSG("DIRM version=" << ver << ", our version=" << version << "\n");
   if (ver>version)
      THROW("Unable to read DJVM directories of versions higher than "+
	    GString(version)+".\nData version number is "+GString(ver)+".");
   DEBUG_MSG("bundled directory=" << bundled << "\n");
   
   DEBUG_MSG("reading the directory records...\n");
   int files=str.read16();
   DEBUG_MSG("number of files=" << files << "\n");

   if (files)
   {
      DEBUG_MSG("reading offsets and sizes\n");
      for(int file=0;file<files;file++)
      {
	 GP<File> file=new File();
	 files_list.append(file);
	 if (bundled)
	 {
	    file->offset=str.read32();
	    file->size=str.read24();
	    if (file->offset==0 || file->size==0)
	       THROW("Directory error: no indirect entries allowed in bundled document.");
	 } else file->offset=file->size=0;
      }

      BSByteStream bs_str(str);
      DEBUG_MSG("reading and decompressing flags...\n");
      for(pos=files_list;pos;++pos)
	 files_list[pos]->flags=bs_str.read8();
   
      DEBUG_MSG("reading and decompressing names...\n");
      TArray<char> strings;
      char buffer[1024];
      int length;
      while((length=bs_str.read(buffer, 1024)))
      {
	 int strings_size=strings.size();
	 strings.resize(strings_size+length-1);
	 memcpy((char*) strings+strings_size, buffer, length);
      }
      DEBUG_MSG("size of decompressed names block=" << strings.size() << "\n");
   
	 // Copy names into the files
      const char * ptr=strings;
      for(pos=files_list;pos;++pos)
      {
	 GP<File> file=files_list[pos];

	 file->id=ptr; ptr+=file->id.length()+1;
	 if (file->flags & File::HAS_NAME)
	 {
	    file->name=ptr; ptr+=file->name.length()+1;
	 } else file->name=file->id;
	 if (file->flags & File::HAS_TITLE)
	 {
	    file->title=ptr; ptr+=file->title.length()+1;
	 } else file->title=file->id;
      
	 DEBUG_MSG(file->name << ", " << file->id << ", " << file->title << ", " <<
		   file->offset << ", " << file->size << ", " <<
		   file->is_page() << "\n");
      }

	 // Now generate page=>file array for direct access
      int pages=0;
      for(pos=files_list;pos;++pos)
	 pages+=files_list[pos]->is_page() ? 1 : 0;
      DEBUG_MSG("got " << pages << " pages\n");
      page2file.resize(pages-1);
      int page_num=0;
      for(pos=files_list;pos;++pos)
      {
	 GP<File> file=files_list[pos];
	 if (file->is_page())
	 {
	    page2file[page_num]=file;
	    file->page_num=page_num++;
	 }
      }

	 // Generate name2file map
      for(pos=files_list;pos;++pos)
      {
	 GP<File> file=files_list[pos];
	 if (name2file.contains(file->name))
	    THROW("Error in 'DIRM' chunk: two records for the same NAME '"
		  + file->name + "'");
	 name2file[file->name]=file;
      }

	 // Generate id2file map
      for(pos=files_list;pos;++pos)
      {
	 GP<File> file=files_list[pos];
	 if (id2file.contains(file->id))
	    THROW("Error in 'DIRM' chunk: two records for the same ID '"
		  + file->id + "'");
	 id2file[file->id]=file;
      }

	 // Generate title2file map
      for(pos=files_list;pos;++pos)
      {
	 GP<File> file=files_list[pos];
	 if (file->title.length())
	 {
	    if (title2file.contains(file->title))
	       THROW("Error in 'DIRM' chunk: two records for the same TITLE '"
		     + file->title + "'");
	    title2file[file->title]=file;
	 }
      }
   }
}

void
DjVmDir::encode(ByteStream & str) const
{
   DEBUG_MSG("DjVmDir::encode(): encoding contents of the 'DIRM' chunk\n");
   DEBUG_MAKE_INDENT(3);
   
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   GPosition pos;

   bool bundled=files_list.size() ? (files_list[files_list]->offset!=0) : true;
   DEBUG_MSG("encoding version number=" << version << ", bundled=" << bundled << "\n");
   str.write8(version | ((int) bundled<< 7));
   
   DEBUG_MSG("storing the number of records=" << files_list.size() << "\n");
   str.write16(files_list.size());

   if (files_list.size())
   {
      if (bundled)
      {
	 DEBUG_MSG("storing offsets and sizes for every record\n");
	 for(pos=files_list;pos;++pos)
	 {
	    GP<File> file=files_list[pos];
	    if (bundled ^ (file->offset!=0))
	       THROW("The directory contains both indirect and bundled records.");
      
	    str.write32(file->offset);
	    str.write24(file->size);
	 }
      }

      BSByteStream bs_str(str, 50);
      DEBUG_MSG("storing and compressing flags for every record\n");
      for(pos=files_list;pos;++pos)
      {
	 GP<File> file=files_list[pos];
	 if (file->name!=file->id) file->flags|=File::HAS_NAME;
	 else file->flags&=~File::HAS_NAME;
	 if (file->title!=file->id) file->flags|=File::HAS_TITLE;
	 else file->flags&=~File::HAS_TITLE;
	 bs_str.write8(file->flags);
      }

      DEBUG_MSG("storing and compressing names...\n");
      for(pos=files_list;pos;++pos)
      {
	 GP<File> file=files_list[pos];
	 bs_str.writall((const void*)(const char*)file->id, file->id.length()+1);
	 if (file->flags & File::HAS_NAME)
	    bs_str.writall((const void*)(const char*)file->name, file->name.length()+1);
	 if (file->flags & File::HAS_TITLE)
	    bs_str.writall((const void*)(const char*)file->title, file->title.length()+1);
      }
   }
   
   DEBUG_MSG("done\n");
}

GP<DjVmDir::File>
DjVmDir::page_to_file(int page_num) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   if (page_num<page2file.size()) return page2file[page_num];
   else return 0;
}

GP<DjVmDir::File>
DjVmDir::name_to_file(const GString & name) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GPosition pos;
   if (name2file.contains(name, pos)) return name2file[pos];
   else return 0;
}

GP<DjVmDir::File>
DjVmDir::id_to_file(const char * id) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GPosition pos;
   if (id2file.contains(id, pos)) return id2file[pos];
   else return 0;
}

GP<DjVmDir::File>
DjVmDir::title_to_file(const char * title) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GPosition pos;
   if (title2file.contains(title, pos)) return title2file[pos];
   else return 0;
}

GPList<DjVmDir::File>
DjVmDir::get_files_list(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return files_list;
}

int
DjVmDir::get_files_num(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return files_list.size();
}

int
DjVmDir::get_pages_num(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return page2file.size();
}

int
DjVmDir::get_file_pos(const File * f) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   int cnt;
   GPosition pos;
   for(pos=files_list, cnt=0;pos;++pos, cnt++)
      if (files_list[pos]==f) break;
   if (pos) return cnt;
   else return -1;
}

int
DjVmDir::get_page_pos(int page_num) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   
   GP<File> file=page_to_file(page_num);
   if (file) return get_file_pos(file);
   else return -1;
}

void
DjVmDir::insert_file(File * file, int pos_num)
{
   DEBUG_MSG("DjVmDir::insert_file(): name='" << file->name << "', pos=" << pos_num << "\n");
   DEBUG_MAKE_INDENT(3);
   
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   
   if (pos_num<0) pos_num=files_list.size();

      // Modify maps
   if (name2file.contains(file->name))
      THROW("File with NAME '"+file->name+"' already exists in the DJVM directory.");
   name2file[file->name]=file;
   if (id2file.contains(file->id))
      THROW("File with ID '"+file->id+"' already exists in the DJVM directory.");
   id2file[file->id]=file;
   if (file->title.length())
   {
      if (title2file.contains(file->title))
	 THROW("File with TITLE '"+file->title+"' already exists in the DJVM directory.");
      title2file[file->title]=file;
   }
   
      // Add the file to the list
   int cnt;
   GPosition pos;
   for(pos=files_list, cnt=0;pos;++pos, cnt++)
      if (cnt==pos_num) break;
   if (pos) files_list.insert_before(pos, file);
   else files_list.append(file);

   if (file->is_page())
   {
	 // This file is also a page
	 // Count its number
      int page_num=0;
      for(pos=files_list;pos;++pos)
      {
	 GP<File> & f=files_list[pos];
	 if (f==file) break;
	 if (f->is_page()) page_num++;
      }

      int i;
      page2file.resize(page2file.size());
      for(i=page2file.size()-1;i>page_num;i--)
	 page2file[i]=page2file[i-1];
      page2file[page_num]=file;
      for(i=page_num;i<page2file.size();i++)
	 page2file[i]->page_num=i;
   }
}

void
DjVmDir::delete_file(const char * id)
{
   DEBUG_MSG("Deleting file with id='" << id << "'\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   
   for(GPosition pos=files_list;pos;++pos)
   {
      GP<File> & f=files_list[pos];
      if (!strcmp(f->id, id))
      {
	 name2file.del(f->name);
	 id2file.del(f->id);
	 title2file.del(f->title);
	 if (f->is_page())
	    for(int page=0;page<page2file.size();page++)
	       if (page2file[page]==f)
	       {
		  int i;
		  for(i=page;i<page2file.size()-1;i++)
		     page2file[i]=page2file[i+1];
		  page2file.resize(page2file.size()-2);
		  for(i=page;i<page2file.size();i++)
		     page2file[i]->page_num=i;
		  break;
	       }
	 files_list.del(pos);
	 break;
      }
   }
}
