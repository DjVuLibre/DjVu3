//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: DjVmDoc.cpp,v 1.34 2001-01-04 22:04:54 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVmDoc.h"
#include "DataPool.h"
#include "IFFByteStream.h"
#include "GOS.h"
#include "debug.h"

static const char octets[4]={0x41,0x54,0x26,0x54};

DjVmDoc::DjVmDoc(void)
{
   DEBUG_MSG("DjVmDoc::DjVmDoc(): Constructing empty DjVm document.\n");
   DEBUG_MAKE_INDENT(3);

   dir=new DjVmDir();
}

void
DjVmDoc::insert_file(const GP<DjVmDir::File> & f,
		     GP<DataPool> data_pool, int pos)
{
   DEBUG_MSG("DjVmDoc::insert_file(): inserting file '" << f->id <<
	     "' at pos " << pos << "\n");
   DEBUG_MAKE_INDENT(3);

   if (!f)
     G_THROW("DjVmDoc.no_zero_file");       //  Can't insert ZERO file.
   if (data.contains(f->id))
     G_THROW("DjVmDoc.no_duplicate");       //  Attempt to insert the same file twice.

   char buffer[4];
   if (data_pool->get_data(buffer, 0, 4)==4 &&
       !memcmp(buffer, octets, 4))
   {
      data_pool=new DataPool(data_pool, 4, -1);
   } 
   data[f->id]=data_pool;
   dir->insert_file(f, pos);
}

void		
DjVmDoc::insert_file(ByteStream &data, DjVmDir::File::FILE_TYPE file_type,
                     const char *name, const char *id, 
                     const char *title, int pos)
{
   GP<DjVmDir::File> file=new DjVmDir::File(name, id, title, file_type);
   GP<DataPool> pool = new DataPool;
      // Cannot connect to a bytestream.
      // Must copy data into the datapool.
   int nbytes;
   char buffer[1024];
   while ((nbytes = data.read(buffer, sizeof(buffer))))
      pool->add_data(buffer, nbytes);
   pool->set_eof();
      // Call low level insert
   insert_file(file, pool, pos);
}

void
DjVmDoc::delete_file(const char * id)
{
   DEBUG_MSG("DjVmDoc::delete_file(): deleting file '" << id << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!data.contains(id))
      G_THROW(GString(".cant_delete\t") + id);      //  There is no file with ID 'xxx' to delete.
   
   data.del(id);
   dir->delete_file(id);
}

GP<DataPool>
DjVmDoc::get_data(const char * id)
{
   GPosition pos;
   if (!data.contains(id, pos))
      G_THROW(GString("DjVmDoc.cant_find\t") + id);       //  Can't find file with ID 'xxx'.
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
      file->offset=0xffffffff;
      GPosition data_pos;
      if (!data.contains(file->id, data_pos))
	       G_THROW("DjVmDoc.no_data\t" + file->id);     //  Strange: there is no data for file 'xxx'
      file->size=data[data_pos]->get_length();
      if (!file->size)
         G_THROW("DjVmDoc.zero_file");                //  Strange: File size is zero.
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
      file->offset=offset;
      offset+=file->size;	// file->size has been set in the first pass
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

      GP<DataPool> pool=data[file->id];
      GP<ByteStream> str_in=pool->get_stream();

	 // First check that the file is in IFF format
      G_TRY {
	       IFFByteStream iff_in(*str_in);
	       int size;
	       GString chkid;
	       size=iff_in.get_chunk(chkid);
	       if (size<0 || size>0x7fffffff)
	          G_THROW("DjVmDoc.not_IFF\t" + file->id);    //  File 'xxx' is not in IFF format.
      } G_CATCH_ALL {
	       G_THROW("DjVmDoc.not_IFF\t" + file->id);       //  File 'xxx' is not in IFF format.
      } G_ENDCATCH;

	 // Now copy the file contents
      str_in=pool->get_stream();	// Rewind doesn't work
      if ((iff.tell() & 1)!=0) { char ch=0; iff.write(&ch, 1); }
      iff.copy(*str_in);
   }

   iff.close_chunk();
   iff.flush();

   DEBUG_MSG("done storing DjVm file.\n");
}

void
DjVmDoc::read(const GP<DataPool> & pool)
{
   DEBUG_MSG("DjVmDoc::read(): reading the BUNDLED doc contents from the pool\n");
   DEBUG_MAKE_INDENT(3);
   
   GP<ByteStream> str=pool->get_stream();
   
   IFFByteStream iff(*str);
   GString chkid;
   iff.get_chunk(chkid);
   if (chkid!="FORM:DJVM")
      G_THROW("DjVmDoc.no_form_djvm");      //  Can't find form DJVM in the input data.

   iff.get_chunk(chkid);
   if (chkid!="DIRM")
      G_THROW("DjVmDoc.no_dirm_chunk");     //  The first chunk of a DJVM document must be DIRM: must be an old format.
   dir->decode(iff);
   iff.close_chunk();

   data.empty();

   if (dir->is_indirect())
      G_THROW("DjVmDoc.cant_read_indr");    //  Can't read indirect DjVm documents from DataPools or ByteStreams.

   GPList<DjVmDir::File> files_list=dir->get_files_list();
   for(GPosition pos=files_list;pos;++pos)
   {
      DjVmDir::File * f=files_list[pos];
      
      DEBUG_MSG("reading contents of file '" << f->id << "'\n");
      data[f->id]=new DataPool(pool, f->offset, f->size);
   }
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

   GP<DataPool> pool=new DataPool(name);
   GP<ByteStream> str=pool->get_stream();
   IFFByteStream iff(*str);
   GString chkid;
   iff.get_chunk(chkid);
   if (chkid!="FORM:DJVM")
      G_THROW("DjVmDoc.no_form_djvm2");       //  Can't find form DJVM. The document is not in new multipage format.

   iff.get_chunk(chkid);
   if (chkid!="DIRM")
      G_THROW("DjVmDoc.no_dirm_chunk");       //  The first chunk of a DJVM document must be DIRM: must be an old format.
   dir->decode(iff);
   iff.close_chunk();

   if (dir->is_bundled()) read(pool);
   else
   {
      GString full_name=GOS::expand_name(name);
      GString dir_name=GOS::dirname(full_name);

      data.empty();

      GPList<DjVmDir::File> files_list=dir->get_files_list();
      for(GPosition pos=files_list;pos;++pos)
      {
	 DjVmDir::File * f=files_list[pos];
      
	 DEBUG_MSG("reading contents of file '" << f->id << "'\n");

	 data[f->id]=new DataPool(GOS::expand_name(f->name, dir_name));
      }
   }
}

void
DjVmDoc::write_index(ByteStream & str)
{
   DEBUG_MSG("DjVmDoc::write_index(): Storing DjVm index file\n");
   DEBUG_MAKE_INDENT(3);

   GPList<DjVmDir::File> files_list=dir->get_files_list();
   for(GPosition pos=files_list;pos;++pos)
   {
      GP<DjVmDir::File> file=files_list[pos];
      file->offset=0;

      GPosition data_pos;
      if (!data.contains(file->id, data_pos))
	       G_THROW("DjVmDoc.no_data\t" + file->id);     //  Strange: there is no data for file 'xxx'
      file->size=data[data_pos]->get_length();
      if (!file->size) G_THROW("DjVmDoc.zero_file");  //  Strange: File size is zero.
   }

   IFFByteStream iff(str);

   iff.put_chunk("FORM:DJVM", 1);
   iff.put_chunk("DIRM");
   dir->encode(iff);
   iff.close_chunk();
   iff.close_chunk();
   iff.flush();
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
	       G_THROW("DjVmDoc.no_data\t" + file->id);     //  Strange: there is no data for file 'xxx'.

      GString file_name=GOS::expand_name(file->name, dir_name);
      DEBUG_MSG("storing file '" << file_name << "'\n");

      GP<ByteStream> str_in=data[data_pos]->get_stream();
      DataPool::load_file(file_name);
      StdioByteStream str_out(file_name, "wb");
      str_out.writall(octets, 4);
      str_out.copy(*str_in);
   }

   if (idx_name && strlen(idx_name))
   {
      GString idx_full_name=GOS::expand_name(idx_name, dir_name);
   
      DEBUG_MSG("storing index file '" << idx_full_name << "'\n");

      DataPool::load_file(idx_full_name);
      StdioByteStream str(idx_full_name, "wb");
      write_index(str);
   }
}
