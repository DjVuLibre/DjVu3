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
// $Id: DjVmDir0.h,v 1.11 2000-11-02 01:08:34 bcr Exp $
// $Name:  $

 
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
    of a multipage all-in-one-file DjVu document (usually referred to as
    {\bf DjVm} document.

    This is {\bf not} a navigation directory, which lists all the pages
    in a multipage document. The navigation directory is supported by class
    \Ref{DjVuNavDir}. This is the directory of a DjVm archive.
    

    @memo Directory of DjVu all-in-one-file DjVu documents.
    @author Andrei Erofeev <eaf@geocities.com>
    @version #$Id: DjVmDir0.h,v 1.11 2000-11-02 01:08:34 bcr Exp $# */

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
   class FileRec;
private:
   GMap<GString, GP<FileRec> >	name2file;
   GPArray<FileRec>		num2file;
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

      /** Describes a file record inside a DjVm document (archive) */
class DjVmDir0::FileRec : public GPEnabled
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
