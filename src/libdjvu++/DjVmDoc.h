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
//C- $Id: DjVmDoc.h,v 1.1 1999-08-17 21:28:44 eaf Exp $
 
#ifndef _DJVMDOC_H
#define _DJVMDOC_H

#ifdef __GNUC__
#pragma interface
#endif

#include "ByteStream.h"
#include "GSmartPointer.h"
#include "GPContainer.h"
#include "Arrays.h"
#include "GString.h"
#include "DjVmDir.h"

/** @name DjVmDoc.h
    Files #"DjVmDoc.h"# and #"DjVmDoc.cpp"# contain implementation of the
    \Ref{DjVmDoc} class used to read and write new DjVu multipage documents.

    Since the class can read and write documents in any of the two formats,
    it's ideal for performing the convertion between the formats.

    @memo DjVu multipage documents reader/writer.
    @author Andrei Erofeev <eaf@geocities.com>
    @version #$Id: DjVmDoc.h,v 1.1 1999-08-17 21:28:44 eaf Exp $#
*/

//@{

/** #DjVmDoc# - class for reading and writing DjVu multipage documents in
    one of the new formats. 

    The "new" DjVu multipage documents can be of two types: {\em bundled} and
    {\em indirect}. In the first case all pages are packed into one file,
    which is very like an archive internally. In the second case every page
    is stored in a separate file. Plus there can be other components,
    included into one or more pages, which also go into separate files. In
    addition to pages and components, in the case of the {\em indirect} format
    there is one more top-level file with the document directory (see
    \Ref{DjVmDir}), which is basically an index file containing the
    list of all files composing the document.

    Since the class can read documents of both formats and can save them
    in any format, it's ideal for performing conversion between {\em bundled}
    and {\em indirect} formats. If it's necessary to convert a document
    in an obsolete format though, the best way to do it is to use
    \Ref{DjVuDocument} class, which still supports both obsolete and new formats.

    This class can also be used to create and modify multipage documents
    at the low level without decoding every page or component (See
    \Ref{insert_file}() and \Ref{delete_file}()). For a more convenient
    editing tool please refer to \Ref{DjVuDocEditor} and \Ref{DjVuFile}.
*/
class DjVmDoc : public GPEnabled
{
private:
   GP<DjVmDir>			dir;
   GMap<GString, TArray<char> >	data;
public:
      /** Inserts a file described by \Ref{DjVmDir::File} structure with
	  data #data# at position #pos#. If #pos# is negative, the file
          will be appended to the document. Otherwise it will be inserted
          at position #pos#. */
   void		insert_file(DjVmDir::File * f, const TArray<char> & data, int pos=-1);
   
      /** Removes file with the specified #id# from the document. Every
	  file inside a new DjVu multipage document has its unique ID
	  (refer to \Ref{DjVmDir} for details), which is passed to this
          function. */
   void		delete_file(const char * id);

      /** Returns the directory of the DjVm document (the one which will
	  be encoded into #DJVM# chunk of the top-level file or the bundle). */
   GP<DjVmDir>	get_djvm_dir(void);
   
      /** Returns contents of file with ID #id# from the document.
	  Please refer to \Ref{DjVmDir} for the explanation of what
          IDs mean. */
   TArray<char>	get_data(const char * id);

      /** Reading routines */
      //@{
      /** Reads contents of a {\em bundled} multipage DjVu document from
	  the stream. */
   void		read(ByteStream & str);
      /** Reads the DjVu multipage document in either {\em bundled} or
	  {\em indirect} format.

	  @param name For {\em bundled} documents this is the name
	         of the document. For {\em indirect} documents this is
		 the name of the top-level file of the document (containing
		 the \Ref{DjVmDir} with the list of all files).
		 The rest of the files are expected to be in the
		 same directory and will be read by this function as well. */
   void		read(const char * name);
      //@}

      /** Writing routines */
      //@{
      /** Writes the multipage DjVu document in the {\em bundled} format into
	  the stream. */
   void		write(ByteStream & str);
      /** Writes the multipage DjVu document in the {\em indirect} format
	  into the given directory. Every page and included file will be
          stored as a separate file. Besides, one top-level file with
          the document directory (named #idx_name#) will be created.

          @param dir_name Name of the directory where files should be
		 created
	  @param idx_name Name of the top-level file with the \Ref{DjVmDir}
		 with the list of files composing the given document. */
   void		expand(const char * dir_name, const char * idx_name);
      //@}

      /// Constructor
   DjVmDoc(void);
   virtual ~DjVmDoc(void);
};

inline GP<DjVmDir>
DjVmDoc::get_djvm_dir(void)
{
   return dir;
}

inline
DjVmDoc::~DjVmDoc(void) {}

//@}


#endif
