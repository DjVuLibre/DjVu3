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
//C- $Id: DjVmFile.h,v 1.3 1999-08-08 23:27:05 leonb Exp $
 
#ifndef _DJVMFILE_H
#define _DJVMFILE_H

#ifdef __GNUC__
#pragma interface
#endif

#include "ByteStream.h"
#include "GSmartPointer.h"
#include "GContainer.h"
#include "Arrays.h"
#include "GString.h"
#include "DjVmDir0.h"

/** @name DjVmFile.h
    Files #"DjVmFile.h"# and #"DjVmFile.cpp"# contain implementation of the
    \Ref{DjVmFile} class used to create and read all-in-one-file multipage
    DjVu documents (usually refered to as DjVm documents).

    This is a useful class to get a bunch of IFF or non-IFF files and store
    them inside one DjVm archive. It can also read an existing DjVm file
    into memory and make it available for you.

    Internally it maintains a directory (table of contents) of the archive
    implemented by \Ref{DjVmDir0} class. The directory contains the file's
    name, its size, offset in the archive and type.

    @memo DjVm documents reader/writer.
    @author Andrei Erofeev <eaf@geocities.com>
    @version #$Id: DjVmFile.h,v 1.3 1999-08-08 23:27:05 leonb Exp $#
*/

//@{

/** #DjVmFile# - class for reading and writing all-in-one-file DjVu multipage
    documents (also known as DjVm documents).

    Besides reading and writing the DjVm documents, it allows you to compose
    them manually by means of \Ref{add_file}() and \Ref{del_file}() functions.
*/
class DjVmFile : public GPEnabled
{
   class File : public GPEnabled
   {
   public:
      GString		name;
      TArray<char>	data;

      File(const char * name, const TArray<char> & data);
      virtual ~File(void);
   };
private:
   GPList<File>		files;
   GP<DjVmDir0>		djvm_dir;

   int		get_djvm_file_size(void);
public:
      /** Adds a file with the specified #name# and contaning the specified
	  #data# of length #length# to the list of files at
	  position #pos#. If #pos# is negative, then the file is
	  appended to the list. */
   void		add_file(const char * name, const TArray<char> & data, int pos=-1);
   
      /** Same as above, but the file contents are read from the stream */
   void		add_file(const char * name, ByteStream & istr, int pos=-1);
   
      /** Removes file with the specified #name# from the list */
   void		del_file(const char * name);

      /** Returns directory of the DjVm document as it will be saved (or has
	  been read) to (or from) the file */
   GP<DjVmDir0>	get_djvm_dir(void);
   
      /** Returns contents of file with name #name# from the list. */
   TArray<char>	& get_file(const char * name);

      /** Returns the first IFF file, which has the top-level form with name
	  #form_name#. This is useful when displaying contents of
	  a DjVm archive: you want to decode and display the first page,
	  but you don't know what page is the first because the navigation
	  directory (implemented by \Ref{DjVuNavDir}) has not been decoded yet.*/
   GString	get_first_file(const char * form_name);

      /** Write the whole DjVm file into the stream */
   void		write(ByteStream & ostr);
      /** Stores the contents of the whole DjVm file into the array */
   void		write(TArray<char> & data);
      /** Reads the DjVm file from the stream */
   void		read(ByteStream & istr);
      /** Reads the DjVm file from the array */
   void		read(const TArray<char> & data);

      /** Expands into given directory. */
   void		expand(const char * dir_name);

      /// Default constructor
   DjVmFile(void);
   virtual ~DjVmFile(void);
};

//@}

inline
DjVmFile::File::~File(void) {};

inline
DjVmFile::DjVmFile(void) {};

inline
DjVmFile::~DjVmFile(void) {};

#endif
