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
//C- $Id: DjVmDoc.cpp,v 1.3 1999-08-25 22:29:37 eaf Exp $

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
   GP<DataPool> pool=new DataPool();
   pool->add_data((const char *) d, d.size());
   pool->set_eof();

   insert_file(f, new DataRange(pool), pos);
}

void
DjVmDoc::insert_file(DjVmDir::File * f, GP<DataRange> data_range, int pos)
{
   DEBUG_MSG("DjVmDoc::insert_file(): inserting file '" << f->id <<
	     "' at pos " << pos << "\n");
   DEBUG_MAKE_INDENT(3);

   if (!f) THROW("Can't insert ZERO file.");
   if (data.contains(f->id)) THROW("Attempt to insert the same file twice.");

   char buffer[4];
   if (data_range->get_data(buffer, 0, 4)==4 &&
       !memcmp(buffer, "AT&T", 4))
      data_range=new DataRange(data_range->get_pool(),
			       data_range->get_start()+4,
			       data_range->get_length()-4);
   data[f->id]=data_range;
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

GP<DataRange>
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
      file->size=data[data_pos]->get_length();
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
      
      ByteStream * str_in=0;
      TRY {
	 str_in=data[file->id]->get_stream();

	    // First check that the file is in IFF format
	 TRY {
	    IFFByteStream iff_in(*str_in);
	    int size;
	    GString chkid;
	    size=iff_in.get_chunk(chkid);
	    if (size<0 || size>1024*1024)
	       THROW("File '"+file->id+"' is not in IFF format.");
	 } CATCH(exc) {
	    THROW("File '"+file->id+"' is not in IFF format.");
	 } ENDCATCH;

	    // Now copy the file contents
	 str_in->seek(0, SEEK_SET);
	 if ((iff.tell() & 1)!=0) { char ch=0; iff.write(&ch, 1); }
	 iff.copy(*str_in);
      } CATCH(exc) {
	 delete str_in; str_in=0;
	 RETHROW;
      } ENDCATCH;
      delete str_in; str_in=0;
   }

   DEBUG_MSG("done storing DjVm file.\n");
}

void
DjVmDoc::read(const GP<DataPool> & pool)
{
   DEBUG_MSG("DjVmDoc::read(): reading the BUNDLED doc contents from the pool\n");
   DEBUG_MAKE_INDENT(3);
   
   ByteStream * str=0;
   TRY {
      GP<DataRange> range=new DataRange(pool);
      str=range->get_stream();
   
      IFFByteStream iff(*str);
      GString chkid;
      iff.get_chunk(chkid);
      if (chkid!="FORM:DJVM")
	 THROW("Can't find form DJVM in the input data.");

      iff.get_chunk(chkid);
      if (chkid!="DIRM")
	 THROW("The first chunk of a DJVM document must be DIRM: must be an old format.");
      dir->decode(iff);
      iff.close_chunk();

      data.empty();

      GPList<DjVmDir::File> files_list=dir->get_files_list();
      if (files_list[files_list]->offset==0)
	 THROW("Can't read indirect DjVm documents from DataPools or ByteStreams.");
      
      for(GPosition pos=files_list;pos;++pos)
      {
	 DjVmDir::File * f=files_list[pos];
      
	 DEBUG_MSG("reading contents of file '" << f->id << "'\n");
	 data[f->id]=new DataRange(pool, f->offset, f->size);
      }
   } CATCH(exc) {
      delete str; str=0;
      RETHROW;
   } ENDCATCH;
   delete str; str=0;
}

void
DjVmDoc::read(ByteStream & str_in)
{
   DEBUG_MSG("DjVmDoc::read(): reading the BUNDLED doc contents from the stream\n");
   DEBUG_MAKE_INDENT(3);

   GP<DataPool> pool=new DataPool();
   char buffer[1024];
   int length;
   while((length=str_in.read(buffer, 1024)))
      pool->add_data(buffer, length);
   pool->set_eof();

   read(pool);
}

void
DjVmDoc::read(const char * name)
{
   DEBUG_MSG("DjVmDoc::read(): reading the doc contents from the HDD\n");
   DEBUG_MAKE_INDENT(3);

   ByteStream * str=0;
   TRY {
      GP<DataPool> pool=new DataPool(name);
      GP<DataRange> range=new DataRange(pool);
      str=range->get_stream();
      IFFByteStream iff(*str);
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
      if (files_list[files_list]->offset!=0) read(pool);
      else
      {
	 GString full_name=GOS::expand_name(name);
	 GString dir_name=GOS::basename(full_name);

	 data.empty();
      
	 for(GPosition pos=files_list;pos;++pos)
	 {
	    DjVmDir::File * f=files_list[pos];
      
	    DEBUG_MSG("reading contents of file '" << f->id << "'\n");

	       // We could have initialized DataPool() with the file_name,
	       // but this would lead to too many open files
	    GP<DataPool> pool=new DataPool();
	    StdioByteStream str(GOS::expand_name(f->name, dir_name), "rb");
	    char buffer[1024];
	    int length;
	    while((length=str.read(buffer, 1024)))
	       pool->add_data(buffer, length);
	    pool->set_eof();
	    data[f->id]=new DataRange(pool);
	 }
      }
   } CATCH(exc) {
      delete str; str=0;
      RETHROW;
   } ENDCATCH;
   delete str; str=0;
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

      ByteStream * str_in=0;
      TRY {
	 str_in=data[data_pos]->get_stream();
	 GOS::deletefile(file_name);
	 StdioByteStream str_out(file_name, "wb");
	 str_out.writall("AT&T", 4);
	 str_out.copy(*str_in);
      } CATCH(exc){
	 delete str_in; str_in=0;
	 RETHROW;
      } ENDCATCH;
      delete str_in; str_in=0;
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

   GOS::deletefile(idx_full_name);
   StdioByteStream str(idx_full_name, "wb");
   IFFByteStream iff(str);

   iff.put_chunk("FORM:DJVM", 1);
   iff.put_chunk("DIRM");
   dir->encode(iff);
   iff.close_chunk();
}
