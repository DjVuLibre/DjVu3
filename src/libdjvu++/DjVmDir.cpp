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
//C- $Id: DjVmDir.cpp,v 1.26 2000-11-02 00:03:46 bcr Exp $


#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVmDir.h"
#include "BSByteStream.h"
#include "debug.h"

#include <ctype.h>


/* Test that a file id is legal (static). */

bool
DjVmDir::File::is_legal_id(const char *id)
{
  // This is a minimal check.
  if (id==0 || id[0]==0 || id[0]=='#')
    return false;
  return true;
}

/* DjVmDir::File */

DjVmDir::File::File(void) 
  : offset(0), size(0), flags(0), page_num(-1)
{ 
}

DjVmDir::File::File(const char *name, const char *id,
		    const char *title, FILE_TYPE file_type)
  : name(name), id(id), title(title), offset(0), size(0), page_num(-1)
{ 
  if (! File::is_legal_id(id) )
    G_THROW("DjVmDir.bad_file\t" + GString(id));    //  DjVm File ID 'xxxx' contains illegal character(s)
  flags=(file_type & TYPE_MASK);
}

DjVmDir::File::File(const char *name, const char *id,
		    const char *title, bool page)
  : name(name), id(id), title(title), offset(0), size(0), page_num(-1)
{ 
  if (! File::is_legal_id(id) )
    G_THROW("DjVmDir.bad_file\t" + GString(id));    //  DjVm File ID 'xxxx' contains illegal character(s)
  flags=page ? PAGE : INCLUDE;
}
   
GString
DjVmDir::File::get_str_type(void) const
{
   GString type;
   switch(flags & TYPE_MASK)
   {
      case INCLUDE: type="INCLUDE"; break;
      case PAGE: type="PAGE"; break;
      case THUMBNAILS: type="THUMBNAILS"; break;
      case SHARED_ANNO: type="SHARED_ANNO"; break;
      default:
	 G_THROW("DjVmDir.get_str_type");   //  Internal error: please modify DjVmDir::File::get_str_type()
	                                    //  to contain all possible File types.
   }
   return type;
}


const int DjVmDir::version=1;

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
      G_THROW("DjVmDir.version_error\t" + GString(version) + "\t" + GString(ver));
                                           // Unable to read DJVM directories of versions higher than xxx
                                           // Data version number is yyy.
   DEBUG_MSG("bundled directory=" << bundled << "\n");
   
   DEBUG_MSG("reading the directory records...\n");
   int files=str.read16();
   DEBUG_MSG("number of files=" << files << "\n");

   if (files)
   {
      DEBUG_MSG("reading offsets (and sizes for ver==0)\n");
      for(int file=0;file<files;file++)
      {
	 GP<File> file=new File();
	 files_list.append(file);
	 if (bundled)
	 {
	    file->offset=str.read32();
	    if (ver==0)
              file->size=str.read24();
	    if (file->offset==0)
	       G_THROW("DjVmDir.no_indirect");    //  Directory error: no indirect entries allowed in bundled document.
	 } else
         {
           file->offset=file->size=0;
         }
      }

      BSByteStream bs_str(str);
      if (ver>0)
      {
	 DEBUG_MSG("reading and decompressing sizes...\n");
	 for(GPosition pos=files_list;pos;++pos)
	    files_list[pos]->size=bs_str.read24();
      }
	 
      DEBUG_MSG("reading and decompressing flags...\n");
      for(pos=files_list;pos;++pos)
	 files_list[pos]->flags=bs_str.read8();

      if (!ver)
      {
	 DEBUG_MSG("converting flags from version 0...\n");
	 for(pos=files_list;pos;++pos)
	 {
	    unsigned char flags_0=files_list[pos]->flags;
	    unsigned char flags_1;
	    flags_1=(flags_0 & File::IS_PAGE_0) ? File::PAGE : File::INCLUDE;
	    if (flags_0 & File::HAS_NAME_0)
              flags_1|=File::HAS_NAME;
	    if (flags_0 & File::HAS_TITLE_0)
              flags_1|=File::HAS_TITLE;
	    files_list[pos]->flags=flags_1;
	 }
      }
   
      DEBUG_MSG("reading and decompressing names...\n");
      GTArray<char> strings;
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

	 file->id=ptr;
    ptr+=file->id.length()+1;
	 if (file->flags & File::HAS_NAME)
	 {
	    file->name=ptr;
       ptr+=file->name.length()+1;
	 } else
       file->name=file->id;
	 if (file->flags & File::HAS_TITLE)
	 {
	    file->title=ptr;
       ptr+=file->title.length()+1;
	 } else
       file->title=file->id;
   /* msr debug:  multipage file, file->title is null.  
	 DEBUG_MSG(file->name << ", " << file->id << ", " << file->title << ", " <<
		   file->offset << ", " << file->size << ", " <<
		   file->is_page() << "\n"); */
      }

	 // Check that there is only one file with SHARED_ANNO flag on
      int shared_anno_cnt=0;
      for(pos=files_list;pos;++pos)
      {
	 if (files_list[pos]->is_shared_anno())
         {
	    shared_anno_cnt++;
         }
      }
      if (shared_anno_cnt>1)
        G_THROW("DjVmDir.corrupt");      //  DjVu document is corrupt. There may be only one file
	                                       //  with shared annotations in a multipage document.

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
	          G_THROW("DjVmDir.dupl_name\t" + file->name );   //  Error in 'DIRM' chunk: two records for the same NAME 'xxx'
	       name2file[file->name]=file;
      }

	 // Generate id2file map
      for(pos=files_list;pos;++pos)
      {
	       GP<File> file=files_list[pos];
	       if (id2file.contains(file->id))
	          G_THROW("DjVmDir.dupl_id\t" + file->id);        //  Error in 'DIRM' chunk: two records for the same ID 'xxx'
	       id2file[file->id]=file;
      }

	 // Generate title2file map
      for(pos=files_list;pos;++pos)
      {
	       GP<File> file=files_list[pos];
	       if (file->title.length())
	       {
	          if (title2file.contains(file->title))
	             G_THROW("DjVmDir.dupl_title\t" + file->title); //  Error in 'DIRM' chunk: two records for the same TITLE 'xxx'
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
	 // Check that there is only one file with shared annotations
      int shared_anno_cnt=0;
      for(pos=files_list;pos;++pos)
      {
	 if (files_list[pos]->is_shared_anno())
	    shared_anno_cnt++;
      }
      if (shared_anno_cnt>1)
	 G_THROW("DjVmDir.multi_save");       //  Cannot save a multipage document containing
                                        //  more than one file with shared annotations.
      
      if (bundled)
      {
	    // We need to store offsets uncompressed. That's because when
	    // we save a DjVmDoc, we first compress the DjVmDir with dummy
	    // offsets and after computing the real offsets we rewrite the
	    // DjVmDir, which should not change its size during this operation
	       DEBUG_MSG("storing offsets for every record\n");
	       for(pos=files_list;pos;++pos)
	       {
	          GP<File> file=files_list[pos];
	          if (!file->offset)
	             G_THROW("DjVmDir.bad_dir");      //  The directory contains both indirect and bundled records.
	          str.write32(file->offset);
	       }
      }

      BSByteStream bs_str(str, 50);
      DEBUG_MSG("storing and compressing sizes for every record\n");
      for(pos=files_list;pos;++pos)
      {
	 GP<File> file=files_list[pos];
	 if ((file->offset)?bundled:(!bundled))
	    G_THROW("DjVmDir.bad_dir");               //  The directory contains both indirect and bundled records.
	 bs_str.write24(file->size);
      }
	 
      DEBUG_MSG("storing and compressing flags for every record\n");
      for(pos=files_list;pos;++pos)
      {
	 GP<File> file=files_list[pos];
	 if (file->name!=file->id)
           file->flags|=File::HAS_NAME;
	 else
           file->flags&=~File::HAS_NAME;
	 if (file->title!=file->id)
           file->flags|=File::HAS_TITLE;
	 else
           file->flags&=~File::HAS_TITLE;
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

   return (page_num<page2file.size())?page2file[page_num]:0;
}

GP<DjVmDir::File>
DjVmDir::name_to_file(const GString & name) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GPosition pos;
   return (name2file.contains(name, pos))?name2file[pos]:0;
}

GP<DjVmDir::File>
DjVmDir::id_to_file(const char * id) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GPosition pos;
   return (id2file.contains(id, pos))?id2file[pos]:0;
}

GP<DjVmDir::File>
DjVmDir::title_to_file(const char * title) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GPosition pos;
   return (title2file.contains(title, pos))?title2file[pos]:0;
}

GPList<DjVmDir::File>
DjVmDir::get_files_list(void) const
{
//bcr: I doubt this lock is needed....
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
   for(pos=files_list, cnt=0;pos&&(files_list[pos]!=f);++pos, cnt++);
   return (pos)?cnt:(-1);
}

int
DjVmDir::get_page_pos(int page_num) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   
   GP<File> file=page_to_file(page_num);
   return (file)?get_file_pos(file):(-1);
}

GP<DjVmDir::File>
DjVmDir::get_shared_anno_file(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GP<File> file;
   for(GPosition pos=files_list;pos;++pos)
   {
      GP<File> frec=files_list[pos];
      if (frec->is_shared_anno())
      {
	 file=frec;
	 break;
      }
   }
   return file;
}

void
DjVmDir::insert_file(const GP<File> & file, int pos_num)
{
   DEBUG_MSG("DjVmDir::insert_file(): name='" << file->name << "', pos=" << pos_num << "\n");
   DEBUG_MAKE_INDENT(3);
   
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   
   if (pos_num<0)
     pos_num=files_list.size();

      // Modify maps
   if (! File::is_legal_id(file->id))
     G_THROW("DjVmDir.bad_file\t" + file->id);    //  DjVm File ID 'xxx' contains illegal character(s)
   if (id2file.contains(file->id))
     G_THROW("DjVmDir.dupl_id2\t" + file->id);    //  File with ID 'xxx' already exists in the DJVM directory.
   if (name2file.contains(file->name))
      G_THROW("DjVmDir.dupl_name2\t" + file->name); //  File with NAME 'xxx' already exists in the DJVM directory.
   name2file[file->name]=file;
   id2file[file->id]=file;
   if (file->title.length())
     {
       if (title2file.contains(file->title))  // duplicate titles make become ok some day
         G_THROW("DjVmDir.dupl_title2\t" + file->title);  //File with TITLE 'xxx' already exists in the DJVM directory.
       title2file[file->title]=file;
     }

      // Make sure that there is no more than one file with shared annotations
   if (file->is_shared_anno())
   {
      for(GPosition pos=files_list;pos;++pos)
	 if (files_list[pos]->is_shared_anno())
	    G_THROW("DjVmDir.multi_save2");         //  Current DjVu multipage format does not support
                                              //  more than one file with shared annotations.
   }
   
      // Add the file to the list
   int cnt;
   GPosition pos;
   for(pos=files_list, cnt=0;pos&&(cnt!=pos_num);++pos, cnt++);
   if (pos)
     files_list.insert_before(pos, file);
   else
     files_list.append(file);

   if (file->is_page())
   {
	 // This file is also a page
	 // Count its number
      int page_num=0;
      for(pos=files_list;pos;++pos)
      {
	 GP<File> &f=files_list[pos];
	 if (f==file)
           break;
	 if (f->is_page())
           page_num++;
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
         {
	    for(int page=0;page<page2file.size();page++)
            {
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
            }
         }
	 files_list.del(pos);
	 break;
      }
   }
}

void
DjVmDir::set_file_name(const char * id, const char * name)
{
   DEBUG_MSG("DjVmDir::set_file_name(): id='" << id << "', name='" << name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GPosition pos;
   
      // First see, if the name is unique
   for(pos=files_list;pos;++pos)
   {
      GP<File> file=files_list[pos];
      if (file->id!=id && file->name==name)
        G_THROW("DjVmDir.name_in_use\t" + GString(name));   //  Name 'xxx' is already in use");
   }

      // Check if ID is valid
   if (!id2file.contains(id, pos))
      G_THROW("DjVmDir.no_info\t" + GString(id));       //  Nothing is known about file with ID 'xxx'
   GP<File> file=id2file[pos];
   name2file.del(file->name);
   file->name=name;
   name2file[name]=file;
}

void
DjVmDir::set_file_title(const char * id, const char * title)
{
   DEBUG_MSG("DjVmDir::set_file_title(): id='" << id << "', title='" << title << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GPosition pos;
   
      // First see, if the title is unique
   for(pos=files_list;pos;++pos)
   {
      GP<File> file=files_list[pos];
      if (file->id!=id && file->title==title)
        G_THROW("DjVmDir.title_in_use\t" + GString(title));  //  Title 'xxx' is already in use
   }

      // Check if ID is valid
   if (!id2file.contains(id, pos))
      G_THROW("DjVmDir.no_info\t" + GString(id));       //  Nothing is known about file with ID 'xxx'
   GP<File> file=id2file[pos];
   title2file.del(file->title);
   file->title=title;
   title2file[title]=file;
}
