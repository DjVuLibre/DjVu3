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
//C- $Id: DjVmFile.cpp,v 1.1.2.2 1999-05-12 21:44:00 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVmFile.h"
#include "IFFByteStream.h"
#include "debug.h"

DjVmFile::File::File(const char * name_in, const TArray<char> & data_in) :
      name(name_in), data(data_in)
{
   DEBUG_MSG("File::File(): Creating file record w/name '" << name_in << "'\n");
}

void
DjVmFile::add_file(const char * name, const TArray<char> & data, int pos_num)
{
   DEBUG_MSG("DjVmFile:add_file(): adding file '" << name << "' at pos=" << pos_num << "\n");
   DEBUG_MAKE_INDENT(3);

   GPosition pos;
   for(pos=files;pos;++pos)
      if (files[pos]->name==name)
	 return;
   
   GP<File> file=new File(name, data);

   if (pos_num>=0 && files.nth(pos_num, pos))
      files.insert_before(pos, file);
   else files.append(file);

   djvm_dir=0;
}

void
DjVmFile::add_file(const char * name, ByteStream & istr, int pos_num)
{
      // It's crazy to use this temporary byte stream, but it will be faster
      // for big files as MemoryByteStream optimizes internal buffer resize
      // TODO: still optimize
   MemoryByteStream str;
   char buffer[1024];
   int length;
   while((length=istr.read(buffer, 1024)))
      str.writall(buffer, length);
   
   add_file(name, str.get_data(), pos_num);
}

void
DjVmFile::del_file(const char * name)
{
   DEBUG_MSG("DjVmFile::del_file(): deleting file w/name '" << name << "'\n");
   DEBUG_MAKE_INDENT(3);

   for(GPosition pos=files;pos;++pos)
      if (files[pos]->name==name)
      {
	 files.del(pos);
	 djvm_dir=0;
	 return;
      };
   
   THROW("Can't find file with name '"+GString(name)+"' in DjVm file.");
}

TArray<char> &
DjVmFile::get_file(const char * name)
{
   DEBUG_MSG("DjVmFile::get_file(): returning file contents w/name '" << name << "'\n");
   DEBUG_MAKE_INDENT(3);

   for(GPosition pos=files;pos;++pos)
      if (files[pos]->name==name)
	 return files[pos]->data;
   
   THROW("Can't find file with name '"+GString(name)+"' in DjVm file.");
   return *new TArray<char>();
}

GString
DjVmFile::get_first_file(const char * form_name)
{
   if (!djvm_dir) djvm_dir=get_djvm_dir();
   int files=djvm_dir->get_files_num();
   for(int i=0;i<files;i++)
   {
      GP<DjVmDir0::FileRec> file=djvm_dir->get_file(i);
      if (file->iff_file)
      {
	 TArray<char> data=get_file(file->name);
	 MemoryByteStream str(data, data.size());
	 IFFByteStream iff(str);

	 int chksize;
	 GString chkid;
	 if (!iff.get_chunk(chkid)) THROW("File does not appear to be in IFF format.");

	 while((chksize=iff.get_chunk(chkid)))
	 {
	    if (chkid==form_name) return file->name;
	    iff.close_chunk();
	 }
      }
   }
   return "";
}

GP<DjVmDir0>
DjVmFile::get_djvm_dir(void)
{
   DEBUG_MSG("DjVmFile::get_djvm_dir(): returning the DjVm directory...\n");
   DEBUG_MAKE_INDENT(3);

   if (djvm_dir)
   {
      DEBUG_MSG("returning cached data\n");
      return djvm_dir;
   };
   
   DEBUG_MSG("pass 1: create dummy DIR0 chunk...\n");
   DjVmDir0 dir1;
   
   GPosition pos;

   for(pos=files;pos;++pos)
   {
      GString fname=files[pos]->name;
      TArray<char> & data=files[pos]->data;
      MemoryByteStream file_str(data, data.size());
      int iff_file=0;
      TRY {
	 IFFByteStream file_iff(file_str);
	 int size;
	 GString chkid;
	 size=file_iff.get_chunk(chkid);
	 if (size>=0 && size<10*1024*1024) iff_file=1;
      } CATCH(exc) {} ENDCATCH;
      
      dir1.add_file(fname, iff_file);
   };

   DEBUG_MSG("size of DIR0 chunk is " << dir1.get_size() << "\n");
   
   DEBUG_MSG("pass 2: creating the real DIR0 chunk with valid offsets, etc.\n");
   DjVmDir0 dir2;

   int offset=16;			// FORM:DJVM
   offset+=8;				// DIR0 chunk header
   offset+=dir1.get_size();		// DIR0 chunk contents
   for(pos=files;pos;++pos)
   {
      if (offset & 1) offset++;
      
      GString fname=files[pos]->name;
      TArray<char> & data=files[pos]->data;
      int iff_file=dir1.get_file(fname)->iff_file;
      if (!iff_file)
      {
	 DEBUG_MSG(fname << " is not IFF file\n");
	 offset+=8;
	 dir2.add_file(fname, iff_file, offset, data.size());
	 offset+=data.size();
      } else
      {
	 DEBUG_MSG(fname << " is an IFF file\n");
	 int size=data.size();
	 if (!strncmp(data, "AT&T", 4)) size-=4;
	 dir2.add_file(fname, iff_file, offset, size);
	 offset+=size;
      };
   };

   djvm_dir=new DjVmDir0(dir2);
   
   return djvm_dir;
}

int
DjVmFile::get_djvm_file_size(void)
{
   DjVmDir0 & dir=*get_djvm_dir();

   GP<DjVmDir0::FileRec> file=dir.get_file(dir.get_files_num()-1);
   int size=file->offset+file->size;

   return size;
}

void
DjVmFile::write(TArray<char> & data)
{
   DEBUG_MSG("DjVmFile::write(): Storing DjVm file into memory buffer\n");
   DEBUG_MAKE_INDENT(3);

   MemoryByteStream str;
   write(str);
   data=str.get_data();
}

void
DjVmFile::write(ByteStream & ostr)
{
   DEBUG_MSG("DjVmFile::write(): Storing DjVm file into stream\n");
   DEBUG_MAKE_INDENT(3);

   DjVmDir0 & dir=*get_djvm_dir();

   DEBUG_MSG("Opening IFF stream...\n");
   IFFByteStream iff(ostr);
   
   iff.put_chunk("FORM:DJVM", 1);

   DEBUG_MSG("Storing directory into DJVM file...\n");
   iff.put_chunk("DIR0");
   dir.encode(iff);
   iff.close_chunk();

   DEBUG_MSG("Storing all files into DJVM file...\n");
   DEBUG_MAKE_INDENT(3);
   
   for(GPosition pos=files;pos;++pos)
   {
      GString fname=files[pos]->name;
      TArray<char> & data=files[pos]->data;

      if (iff.tell() & 1) { char ch=0; iff.write(&ch, 1); };
      
      if (dir.get_file(fname)->iff_file)
      {
	 DEBUG_MSG(fname << ": is an IFF file\n");
	 if (!strncmp(data, "AT&T", 4))
	    iff.writall((const char *) data+4, data.size()-4);
	 else iff.writall(data, data.size());
      } else
      {
	 DEBUG_MSG(fname << ": is NOT an IFF file\n");
	 iff.put_chunk("RAW ");
	 iff.writall(data, data.size());
	 iff.close_chunk();
      };
   };
   iff.close_chunk();
}

void
DjVmFile::read(const TArray<char> & data)
{
   DEBUG_MSG("DjVmFile::read(): reading the file's contents from memory\n");
   DEBUG_MAKE_INDENT(3);

   MemoryByteStream str(data, data.size());
   read(str);
}

void
DjVmFile::read(ByteStream & str)
{
   DEBUG_MSG("DjVmFile::read(): reading the file's contents from the stream\n");
   DEBUG_MAKE_INDENT(3);

   if (!str.is_seekable())
      THROW("Can't read DjVm file: ByteStream is not seekable.");
   
      // Now read the DIR0 chunk (DjVmDir0 directory)
   DjVmDir0 dir;
      
   {
      IFFByteStream iff(str);

      DEBUG_MSG("Reading the DIR0 chunk...\n");
      DEBUG_MAKE_INDENT(3);

      bool got_dir=0;
      GString chkid;
      iff.get_chunk(chkid);
      if (chkid!="FORM:DJVM") THROW("DJVM document expected.");
      while(iff.get_chunk(chkid))
      {
	 DEBUG_MSG("Got chunk '" << chkid << "'\n");
	 if (chkid=="DIR0")
	 {
	    dir.decode(iff);
	    got_dir=1;
	    break;
	 };
	 iff.close_chunk();
      };
      if (!got_dir) THROW("Failed to find chunk DIR0 in DJVM file.");
      djvm_dir=new DjVmDir0(dir);
   }

   DEBUG_MSG("Now reading every file mentioned in the directory...\n");
   DEBUG_MAKE_INDENT(3);

   files.empty();
   for(int i=0;i<dir.get_files_num();i++)
   {
      DjVmDir0::FileRec & file=*dir.get_file(i);
      DEBUG_MSG(file.name << " offset=" << file.offset <<
		", size=" << file.size << "\n");

      TArray<char> data(file.size+3);
      memcpy(data, "AT&T", 4);
      str.seek(file.offset, SEEK_SET);
      str.readall((char *) data+4, data.size()-4);
      files.append(new File(file.name, data));
   };
}

void
DjVmFile::expand(const char * dir_name)
{
   DEBUG_MSG("DjVmFile::expand(): Expanding into '" << dir_name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   for(GPosition pos=files;pos;++pos)
   {
      File & file=*files[pos];
      GString name=dir_name;
      if (dir_name[strlen(dir_name)-1]!='/') name+='/';
      name+=file.name;
      StdioByteStream str(name, "wb");
      str.writall(file.data, file.data.size());
   }
}
