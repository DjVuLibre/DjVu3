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
//C- $Id: DjVmDir0.cpp,v 1.3 1999-08-04 21:45:11 leonb Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "ByteStream.h"
#include "DjVmDir0.h"
#include "debug.h"

int
DjVmDir0::get_size(void) const
      // WARNING! make sure, that get_size(), encode() and decode() are in sync
{
   int size=0;

   size+=2;	// number of files
   for(int i=0;i<num2file.size();i++)
   {
      FileRec & file=*num2file[i];
      size+=file.name.length()+1;	// file name
      size+=1;				// is IFF file
      size+=4;				// file offset
      size+=4;				// file size
   };

   return size;
}

void
DjVmDir0::encode(ByteStream & bs)
      // WARNING! make sure, that get_size(), encode() and decode() are in sync
{
   bs.write16(num2file.size());
   for(int i=0;i<num2file.size();i++)
   {
      FileRec & file=*num2file[i];
      bs.write((const char*)file.name, file.name.length()+1);
      bs.write8(file.iff_file);
      bs.write32(file.offset);
      bs.write32(file.size);
   }
}

void
DjVmDir0::decode(ByteStream & bs)
      // WARNING! make sure, that get_size(), encode() and decode() are in sync
{
   name2file.empty();
   num2file.empty();

   for(int i=bs.read16();i>0;i--)
   {
      GString name;
      char ch;
      while(bs.read(&ch, 1) && ch) name+=ch;
      int iff_file=bs.read8();
      int offset=bs.read32();
      int size=bs.read32();
      add_file(name, iff_file, offset, size);
   };
}

GP<DjVmDir0::FileRec>
DjVmDir0::get_file(const char * name)
{
   if (name2file.contains(name)) return name2file[name];
   return 0;
}

GP<DjVmDir0::FileRec>
DjVmDir0::get_file(int file_num)
{
   if (file_num<num2file.size()) return num2file[file_num];
   return 0;
}

void
DjVmDir0::add_file(const char * name, bool iff_file, int offset, int size)
{
   DEBUG_MSG("DjVmDir0::add_file(): name='" << name << "', iff=" << iff_file <<
	     ", offset=" << offset << "\n");
   
   if (strchr(name, '/')) THROW("File name may not contain slashes.");
   
   GP<FileRec> file=new FileRec(name, iff_file, offset, size);
   name2file[name]=file;
   num2file.resize(num2file.size());
   num2file[num2file.size()-1]=file;
}
