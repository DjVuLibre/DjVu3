//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- 
//C- This software (the "Original Code") is subject to, and may be
//C- distributed under, the GNU General Public License, Version 2.
//C- The license should have accompanied the Original Code or you
//C- may obtain a copy of the license from the Free Software
//C- Foundation at http://www.fsf.org .
//C- 
//C- With respect to the Original Code, and subject to any third
//C- party intellectual property claims, LizardTech grants recipient
//C- a worldwide, royalty-free, non-exclusive license under patent
//C- claims infringed by making, using, or selling Original Code
//C- which are now or hereafter owned or controlled by LizardTech,
//C- but solely to the extent that any such patent is reasonably
//C- necessary to enable you to make, have made, practice, sell, or 
//C- otherwise dispose of Original Code (or portions thereof) and
//C- not to any greater extent that may be necessary to utilize
//C- further modifications or combinations.
//C- 
//C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
//C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
//C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: DjVmDir0.cpp,v 1.11 2000-11-03 02:08:36 bcr Exp $
// $Name:  $

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

#ifndef NEED_DECODER_ONLY
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
#endif

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
      bool iff_file=bs.read8()?true:false;
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
   
   if (strchr(name, '/')) G_THROW("DjVmDir0.no_slash");   //  File name may not contain slashes.
   
   GP<FileRec> file=new FileRec(name, iff_file, offset, size);
   name2file[name]=file;
   num2file.resize(num2file.size());
   num2file[num2file.size()-1]=file;
}
