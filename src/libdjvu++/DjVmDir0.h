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
//C- $Id: DjVmDir0.h,v 1.4 1999-08-18 15:36:03 eaf Exp $
 
#ifndef _DJVMDIR0_H
#define _DJVMDIR0_H

#ifdef __GNUC__
#pragma interface
#endif

#include "GContainer.h"
#include "GString.h"
#include "ByteStream.h"
#include "GSmartPointer.h"

/** @name DjVmDir0.h

    Files #"DjVmDir0.h"# and #"DjVmDir0.cpp"# contain implementation of
    \Ref{DjVmDir0} class responsible for reading and writing the directory
    of a mutipage all-in-one-file DjVu document (usually referred to as
    {\bf DjVm} document.

    This is {\bf not} a navigation directory, which lists all the pages
    in a multipage document. The navigation directory is supported by class
    \Ref{DjVuNavDir}. This is the directory of a DjVm archive.
    

    @memo Directory of DjVu all-in-one-file DjVu documents.
    @author Andrei Erofeev <eaf@geocities.com>
    @version #$Id: DjVmDir0.h,v 1.4 1999-08-18 15:36:03 eaf Exp $# */

//@{

/** Directory of all-in-one-file DjVu documents (also known as DjVm documents).
    This class can read and write the directory (table of contents, in other
    words) of a DjVm document. This table of contents lists all {\bf files}
    included into the document, not {\bf pages} like \Ref{DjVuNavDir} does.
    It is normally stored in the document inside {\bf DIR0} chunk where
    {\bf "0"} refers to the version number.

    An object of this class can be created either as a result of the decoding
    of an existing DjVm file, or manually by calling the default constructor
    and adding later directory entries by means of \Ref{add_file}() function.

    You normally will not want to create or decode this directory manually.
    \Ref{DjVmFile} class will do it for you. */
class DjVmDir0 : public GPEnabled
{
public:
      /** Describes a file record inside a DjVm document (archive) */
   class FileRec : public GPEnabled
   {
   public:
	 /// Name of the file.
      GString		name;
	 /// 1 if the file is in IFF format.
      bool		iff_file;
	 /// Offset of the file in the archive.
      int		offset;
	 /// Size of the file
      int		size;

      friend int	operator==(const FileRec & f1, const FileRec & f2);

	 /// Constructs the #FileRec# object
      FileRec(const char * name, bool iff_file,
	      int offset=-1, int size=-1);
	 /// Default constructor
      FileRec(void);
      virtual ~FileRec(void);
   };
private:
   GMap<GString, GP<FileRec> >	name2file;
   DPArray<FileRec>		num2file;
protected:
public:
      /// Returns the number of files in the DjVm archive
   int		get_files_num(void) const;
   
      /// Returns the file record with name #name#
   GP<FileRec>	get_file(const char * name);

      /// Returns the file record number #file_num#
   GP<FileRec>	get_file(int file_num);

      /** Creates a new file record with name #name# at offset
	  #offset# and size #size#, which is in IFF format if
	  #iff_file# is #TRUE#. */
   void		add_file(const char * name, bool iff_file,
			 int offset=-1, int size=-1);

      /// Returns the size of the directory if it were encoded in #DIR0# chunk
   int		get_size(void) const;

      /** Encodes the directory in #DIR0# chunk into the specified
	  \Ref{ByteStream} */
   void		encode(ByteStream & bs);

      /** Decodes the directory from #DIR0# chunk from the specified
	  \Ref{ByteStream} */
   void		decode(ByteStream & bs);

      /// Copy constructor
   DjVmDir0(const DjVmDir0 & d);

      /// Default constructor
   DjVmDir0(void) {};
   virtual ~DjVmDir0(void) {};
};

inline
DjVmDir0::FileRec::FileRec(const char * name_in, bool iff_file_in,
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

//@}

#endif
