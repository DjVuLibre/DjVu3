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
// $Id: DjVmDoc.h,v 1.17 2000-11-03 02:08:36 bcr Exp $
// $Name:  $

#ifndef _DJVMDOC_H
#define _DJVMDOC_H

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "ByteStream.h"
#include "GSmartPointer.h"
#include "GContainer.h"
#include "GString.h"
#include "DjVmDir.h"
#include "DataPool.h"

/** @name DjVmDoc.h
    Files #"DjVmDoc.h"# and #"DjVmDoc.cpp"# contain implementation of the
    \Ref{DjVmDoc} class used to read and write new DjVu multipage documents.

    @memo DjVu multipage documents reader/writer.
    @author Andrei Erofeev <eaf@geocities.com>
    @version #$Id: DjVmDoc.h,v 1.17 2000-11-03 02:08:36 bcr Exp $#
*/

//@{

/** Read/Write DjVu multipage documents.

    The "new" DjVu multipage documents can be of two types: {\em bundled} and
    {\em indirect}. In the first case all pages are packed into one file,
    which is very like an archive internally. In the second case every page
    is stored in a separate file. Plus there can be other components,
    included into one or more pages, which also go into separate files. In
    addition to pages and components, in the case of the {\em indirect} format
    there is one more top-level file with the document directory (see
    \Ref{DjVmDir}), which is basically an index file containing the
    list of all files composing the document.

    This class can read documents of both formats and can save them under any
    format.  It is therefore ideal for converting between {\em bundled} and
    {\em indirect} formats.  It cannot be used however for reading obsolete
    formats.  The best way to convert obsolete formats consists in reading
    them with class \Ref{DjVuDocument} class and saving them using
    \Ref{DjVuDocument::write} or \Ref{DjVuDocument::expand}.

    This class can also be used to create and modify multipage documents at
    the low level without decoding every page or component (See
    \Ref{insert_file}() and \Ref{delete_file}()). 
*/

class DjVmDoc : public GPEnabled
{
private:
   GP<DjVmDir>			dir;
   GMap<GString, GP<DataPool> >	data;

      // Internal function.
   
public:
      /** Inserts a file into the document.
          @param data  ByteStream containing the file data.
          @param file_type Describes the type of the file to be inserted.
	  	 See \Ref{DjVmDir::File} for details.
          @param name  Name of the file in the document (e.g. an URL).
          @param id    Identifier of the file (as used in INCL chunks).
          @param title Optional title of the file (shown in browsers).
          @param pos   Position of the file in the document (default is append).
      */
   void		insert_file(ByteStream &data, DjVmDir::File::FILE_TYPE file_type,
                            const char *name, const char *id, 
                            const char *title=0, int pos=-1);

      /** Inserts a file described by \Ref{DjVmDir::File} structure with
	  data #data# at position #pos#. If #pos# is negative, the file
          will be appended to the document. Otherwise it will be inserted
          at position #pos#. */
   void		insert_file(const GP<DjVmDir::File> & f,
			    GP<DataPool> data, int pos=-1);

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
   GP<DataPool>	get_data(const char * id);

      /** Reading routines */
      //@{
      /** Reads contents of a {\em bundled} multipage DjVu document from
	  the stream. */
   void		read(ByteStream & str);
      /** Reads contents of a {\em bundled} multipage DjVu document from
	  the \Ref{DataPool}. */
   void		read(const GP<DataPool> & data_pool);
      /** Reads the DjVu multipage document in either {\em bundled} or
	  {\em indirect} format.

	  {\bf Note:} For {\em bundled} documents the file is not
	  read into memory. We just open it and access data directly there.
	  Thus you should not modify the file contents.

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
      /** Stored index (top-level) file of the DjVu document in the {\em
	  indirect} format into the specified stream. */
   void		write_index(ByteStream & str);
      /** Writes the multipage DjVu document in the {\em indirect} format
	  into the given directory. Every page and included file will be
          stored as a separate file. Besides, one top-level file with
          the document directory (named #idx_name#) will be created unless
	  #idx_name# is empty.

          @param dir_name Name of the directory where files should be
		 created
	  @param idx_name Name of the top-level file with the \Ref{DjVmDir}
		 with the list of files composing the given document.
		 If empty, the file will not be created. */
   void		expand(const char * dir_name, const char * idx_name);
      //@}

      /// Constructor
   DjVmDoc(void);
};

inline GP<DjVmDir>
DjVmDoc::get_djvm_dir(void)
{
   return dir;
}


//@}


#endif
