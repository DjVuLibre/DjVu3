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
//C- $Id: DjVmDir0.h,v 1.1.2.1 1999-04-12 16:48:21 eaf Exp $
 
#ifndef _DJVMDIR0_H
#define _DJVMDIR0_H

#ifdef __GNUC__
#pragma interface
#endif

#include "GPContainer.h"
#include "GString.h"
#include "ByteStream.h"
#include "GSmartPointer.h"

// For wondering: This class is responsible for reading/writing directory
// of a DjVm archive. This is NOT navigation directory used by the plugin.
// It's the directory of the archive (list of all files there)

class DjVmDir0 : public GPEnabled
{
public:
   class FileRec : public GPEnabled
   {
   public:
      GString		name;
      int		iff_file;
      int		offset, size;

      friend int	operator==(const FileRec & f1, const FileRec & f2);
   
      FileRec(const char * name, int iff_file,
	      int offset=-1, int size=-1);
      FileRec(void);
      virtual ~FileRec(void);
   };
private:
   GMap<GString, GP<FileRec> >	name2file;
   DPArray<FileRec>		num2file;
protected:
public:
   int		get_files_num(void) const;
   GP<FileRec>	get_file(const char * name);
   GP<FileRec>	get_file(int file_num);
   void		add_file(const char * name, int iff_file,
			 int offset=-1, int size=-1);
   int		get_size(void) const;
   void		write(ByteStream * bs);
   void		read(ByteStream * bs);

   DjVmDir0(const DjVmDir0 & d);
   DjVmDir0(void);
   virtual ~DjVmDir0(void);
};

inline
DjVmDir0::FileRec::FileRec(const char * name_in, int iff_file_in,
			   int offset_in, int size_in) :
      name(name_in), iff_file(iff_file_in),
      offset(offset_in), size(size_in)
{
}

inline
DjVmDir0::FileRec::FileRec(void) : iff_file(0), offset(-1), size(-1)
{
}

inline
DjVmDir0::FileRec::~FileRec(void)
{
}

inline int
DjVmDir0::get_files_num(void) const
{
   return num2file.size();
}

inline
DjVmDir0::DjVmDir0(const DjVmDir0 & d) :
      name2file(d.name2file), num2file(d.num2file)
{
}

inline
DjVmDir0::DjVmDir0(void) {}

inline
DjVmDir0::~DjVmDir0(void) {}

#endif
