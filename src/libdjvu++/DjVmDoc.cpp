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
//C- $Id: DjVmDoc.cpp,v 1.2 1999-08-17 23:48:04 leonb Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVmDoc.h"
#include "IFFByteStream.h"
#include "GOS.h"
#include "debug.h"

DjVmDoc::DjVmDoc(void)
{
   DEBUG_MSG("DjVmDoc::DjVmDoc(): Constructing empty DjVm document.\n");
   DEBUG_MAKE_INDENT(3);

   dir=new DjVmDir();
}

void
DjVmDoc::insert_file(DjVmDir::File * f, const TArray<char> & d, int pos)
{
   DEBUG_MSG("DjVmDoc::insert_file(): inserting file '" << f->id <<
	     "' at pos " << pos << "\n");
   DEBUG_MAKE_INDENT(3);

   if (!f) THROW("Can't insert ZERO file.");
   if (data.contains(f->id)) THROW("Attempt to insert the same file twice.");

   if (d.size()>=4 && !memcmp(d, "AT&T", 4))
   {
      TArray<char> d1(d.size()-4);
      memcpy(d1, (const char *) d+4, d1.size());
      data[f->id]=d1;
   } else data[f->id]=d;
   dir->insert_file(f, pos);
}

void
DjVmDoc::delete_file(const char * id)
{
   DEBUG_MSG("DjVmDoc::delete_file(): deleting file '" << id << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!data.contains(id))
      THROW(GString("There is no file with ID '")+id+"' to delete.");
   
   data.del(id);
   dir->delete_file(id);
}

TArray<char>
DjVmDoc::get_data(const char * id)
{
   GPosition pos;
   if (!data.contains(id, pos))
      THROW(GString("Can't find file with ID '")+id+"'.");
   return data[pos];
}

void
DjVmDoc::write(ByteStream & str)
{
   DEBUG_MSG("DjVmDoc::write(): Storing document into the byte stream.\n");
   DEBUG_MAKE_INDENT(3);


   DEBUG_MSG("pass 1: create dummy DIRM chunk and calculate offsets...\n");

   GPosition pos;

   GPList<DjVmDir::File> files_list=dir->get_files_list();
   for(pos=files_list;pos;++pos)
   {
      GP<DjVmDir::File> file=files_list[pos];
      file->offset=file->size=0xffffffff;
   }
   
   MemoryByteStream tmp_str;
   IFFByteStream tmp_iff(tmp_str);
   tmp_iff.put_chunk("FORM:DJVM", 1);
   tmp_iff.put_chunk("DIRM");
   dir->encode(tmp_iff);
   tmp_iff.close_chunk();
   tmp_iff.close_chunk();
   int offset=tmp_iff.tell();

   for(pos=files_list;pos;++pos)
   {
      if ((offset & 1)!=0) offset++;
      
      GP<DjVmDir::File> & file=files_list[pos];
      GPosition data_pos;
      if (!data.contains(file->id, data_pos))
	 THROW("Strange: there is no data for file '"+file->id+"'\n");
      file->offset=offset;
      file->size=data[data_pos].size();
      offset+=file->size;
   }

   DEBUG_MSG("pass 2: store the file contents.\n");

   IFFByteStream iff(str);
   iff.put_chunk("FORM:DJVM", 1);
   iff.put_chunk("DIRM");
   dir->encode(iff);
   iff.close_chunk();

   for(pos=files_list;pos;++pos)
   {
      GP<DjVmDir::File> & file=files_list[pos];
      TArray<char> & d=data[file->id];
      
	 // First check that the file is in IFF format
      TRY {
	 MemoryByteStream str_in((const void*)(const char*)d, d.size()<32 ? d.size() : 32);
	 IFFByteStream iff_in(str_in);
	 int size;
	 GString chkid;
	 size=iff_in.get_chunk(chkid);
	 if (size<0 || size>1024*1024)
	    THROW("File '"+file->id+"' is not in IFF format.");
      } CATCH(exc) {
	 THROW("File '"+file->id+"' is not in IFF format.");
      } ENDCATCH;

	 // Now store file contents into the stream
      if ((iff.tell() & 1)!=0) { char ch=0; iff.write(&ch, 1); }
      iff.writall((const void*)(const char*)d, d.size());
   }

   DEBUG_MSG("done storing DjVm file.\n");
}

static int
sort_func(const void * ptr1, const void * ptr2)
{
   DjVmDir::File * file1=*(DjVmDir::File **) ptr1;
   DjVmDir::File * file2=*(DjVmDir::File **) ptr2;

   return file1->offset<file2->offset ? -1 :
	  file1->offset>file2->offset ? 1 : 0;
}

void
DjVmDoc::read(ByteStream & str)
{
   DEBUG_MSG("DjVmDoc::read(): reading the BUNDLED doc contents from the stream\n");
   DEBUG_MAKE_INDENT(3);

   IFFByteStream iff(str);
   GString chkid;
   iff.get_chunk(chkid);
   if (chkid!="FORM:DJVM")
      THROW("Can't find form DJVM in the input stream.");

   iff.get_chunk(chkid);
   if (chkid!="DIRM")
      THROW("The first chunk of a DJVM document must be DIRM: must be an old format.");
   dir->decode(iff);
   iff.close_chunk();

   data.empty();

      // Now the challenge is to read all files not seeking backwards
      // (because the ByteStream may not support it)
   GPList<DjVmDir::File> files_list=dir->get_files_list();
   if (files_list[files_list]->offset==0)
      THROW("Can't read indirect DjVm documents.");
   
   TArray<void *> files_arr(files_list.size()-1);
   int file_num=0;
   for(GPosition pos=files_list;pos;++pos)
      files_arr[file_num++]=files_list[pos];
   qsort(files_arr, files_arr.size(), sizeof(void *), sort_func);

      // Now all files should be sorted based on their offsets in the document
      // Read them
   for(file_num=0;file_num<files_arr.size();file_num++)
   {
      DjVmDir::File * f=(DjVmDir::File *) files_arr[file_num];
      
      DEBUG_MSG("reading contents of file '" << f->id << "'\n");
      
      TArray<char> d(f->size-1);
      iff.seek(f->offset, SEEK_SET);
      iff.readall(d, d.size());
      data[f->id]=d;
   }
}

void
DjVmDoc::read(const char * name)
{
   DEBUG_MSG("DjVmDoc::read(): reading the doc contents from the HDD\n");
   DEBUG_MAKE_INDENT(3);

   StdioByteStream str(name, "rb");
   IFFByteStream iff(str);
   GString chkid;
   iff.get_chunk(chkid);
   if (chkid!="FORM:DJVM")
      THROW("Can't find form DJVM in the input stream.");

   iff.get_chunk(chkid);
   if (chkid!="DIRM")
      THROW("The first chunk of a DJVM document must be DIRM: must be an old format.");
   dir->decode(iff);
   iff.close_chunk();

   GPList<DjVmDir::File> files_list=dir->get_files_list();
   if (files_list[files_list]->offset!=0)
   {
      str.seek(0, SEEK_SET);
      read(str);
   } else
   {
      GString full_name=GOS::expand_name(name);
      GString dir_name=GOS::basename(full_name);

      data.empty();
      
      for(GPosition pos=files_list;pos;++pos)
      {
	 DjVmDir::File * f=(DjVmDir::File *) files_list[pos];
      
	 DEBUG_MSG("reading contents of file '" << f->id << "'\n");
      
	 TArray<char> d(f->size-1);
	 StdioByteStream str(GOS::expand_name(f->name, dir_name), "rb");
	 str.readall(d, d.size());
	 data[f->id]=d;
      }
   }
}

void
DjVmDoc::expand(const char * dir_name, const char * idx_name)
{
   DEBUG_MSG("DjVmDoc::expand(): Expanding into '" << dir_name << "'\n");
   DEBUG_MAKE_INDENT(3);

   GPosition pos;
   
      // First - store each file
   GPList<DjVmDir::File> files_list=dir->get_files_list();
   for(pos=files_list;pos;++pos)
   {
      GP<DjVmDir::File> & file=files_list[pos];
      
      GPosition data_pos;
      if (!data.contains(file->id, data_pos))
	 THROW("Strange: there is no data for file '"+file->id+"'.");

      GString file_name=GOS::expand_name(file->name, dir_name);
      DEBUG_MSG("storing file '" << file_name << "'\n");
      
      StdioByteStream str(file_name, "wb");
      TArray<char> & d=data[data_pos];
      str.writall("AT&T", 4);
      str.writall((const void*)(const char*)d, d.size());
   }

   GString idx_full_name=GOS::expand_name(idx_name, dir_name);
   
   DEBUG_MSG("storing index file '" << idx_full_name << "'\n");
      // Now save the index
   for(pos=files_list;pos;++pos)
   {
      GP<DjVmDir::File> file=files_list[pos];
      file->offset=0;
      file->size=0;
   }
   
   StdioByteStream str(idx_full_name, "wb");
   IFFByteStream iff(str);

   iff.put_chunk("FORM:DJVM", 1);
   iff.put_chunk("DIRM");
   dir->encode(iff);
   iff.close_chunk();
}
