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
//C- $Id: DjVmFile.h,v 1.1.2.1 1999-04-12 16:48:21 eaf Exp $
 
#ifndef _DJVMFILE_H
#define _DJVMFILE_H

#ifdef __GNUC__
#pragma interface
#endif

#include "ByteStream.h"
#include "GSmartPointer.h"
#include "GPContainer.h"
#include "Arrays.h"
#include "GString.h"
#include "DjVmDir0.h"

/** @name DjVmFile.h
    File #"DjVmFile.h"# contains the implementation of the \Ref{DjVmFile} class
    used to create and read DjVm files (self-contained, possibly multipage,
    DjVu documents).

    @memo DjVm documents interface.
    @author Andrei Erofeev
    @version #$Id: DjVmFile.h,v 1.1.2.1 1999-04-12 16:48:21 eaf Exp $#
*/

//@{

/** #DjVmFile# - class for reading/writing DJVM self-contained (probably
    multipage) documents.

    It allows to compose/decompose the file items using \Ref{add_file}()
    and \Ref{del_file}() functions; and read/write the file from/into
    a give \Ref{ByteStream} by means of \Ref{read}() and \Ref{write}()
    functions.
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
      /** Adds the file with the specified {\em name} and contaning the specified
	  {\em data} of length {\em length} to the list of files at
	  position {\em pos}. If {\em pos} is negative, then the file is
	  appended to the list. */
   void		add_file(const char * name, const TArray<char> & data, int pos=-1);
      /** Same as above, but the file contents are read from the stream */
   void		add_file(const char * name, ByteStream & istr, int pos=-1);
      /** Removes file with the specified {\em name} from the list */
   void		del_file(const char * name);

      /** Returns directory of the DjVm document as it will be saved (or has
	  been read) to (or from) the file */
   GP<DjVmDir0>	get_djvm_dir(void);
      /** Returns contents of file with name {\em name} from the list. */
   TArray<char>	& get_file(const char * name);

      /** Returns the first IFF file, which has the top-level form with name
	  {\em form_name} */
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
      /// The destructor
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
