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
//C- $Id: DjVmDir.h,v 1.3 1999-08-24 22:02:33 eaf Exp $

#ifndef _DJVMDIR_H
#define _DJVMDIR_H


/** @name DjVmDir.h
    Files #"DjVmDir.h"# and #"DjVmDir.cpp"# contain implementation of
    DjVu multipage document directory represented by class \Ref{DjVmDir}.

    @memo Implements DjVu multipage document directory
    @author Andrei Erofeev <eaf@geocities.com>
    @version
    #$Id: DjVmDir.h,v 1.3 1999-08-24 22:02:33 eaf Exp $# */
//@{


#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"
#include "Arrays.h"
#include "GContainer.h"
#include "ByteStream.h"
#include "GThreads.h"

/** Implements DjVu multipage document directory.
    There are currently two multipage DjVu formats supported: {\em bundled}
    and {\em indirect}. In the first format all files composing a given
    document are packaged (or bundled) into one file, in the second one
    every page and component is stored in a separate file and there is
    one more file, which contains the list of all others.

    In both cases there is somewhere a #DIRM# chunk with the
    {\em multipage DjVu document directory}. This directory lists all files
    composing the given document, helps to access every file, identify pages
    and maintain user-specified shortcuts.

    Every directory record contains the following:
    \begin{itemize}
       \item {\em id} - An internal unique ID assigned by encoder and used
             by other programs to refer to different files in the document.
	     #INCL# (from INCLude) chunks use IDs to refer to files as well.
       \item {\em name} - Name of the file. It must also be unique and is
             set either by encoder or by user at the stage when the document
	     is composed. By keeping the name in {\em bundled} document
	     we guarantee, that it can be expanded later into {\em indirect}
	     document and files will still have the same names.
       \item {\em title} - A user-specified unique or empty string, which
             may be used to access (view) any particular page. These are
	     supposed to be shortcuts like #"chapter1"# or #"appendix"#.
       \item {\em is_page} - Boolean variable telling if a given file is
             a page, or it's just an included component.
       \item {\em offset} and {\em size} - These two numbers are relevant
             in the {\em bundled} case only when everything is packed into
	     one single file and these numbers specify the offset and size
	     of any given component inside this file.
   \end{itemize}

   The #DjVmDir# class does compression and decompression of the directory
   data when it's written or read from the #DIRM# chunk.

   Normally you don't have to create this class yourself. It's done
   automatically when \Ref{DjVmDoc} class initializes itself. It may be useful
   though to be able to access records in the directory because some classes
   (like \Ref{DjVuDocument} and \Ref{DjVmDoc}) return a pointer to #DjVmDir#
   in some cases.
*/

class DjVmDir : public GPEnabled
{
public:
   static const int	version=0;
   
      /** This class represents file records stored in \Ref{DjVmDir}. */
   class File : public GPEnabled
   {
      friend class DjVmDir;
   public:
	 /// Name of the file
      GString	name;
	 /// Internally used ID of the file
      GString	id;
	 /// User specified shortcut to the file
      GString	title;
      
      int	offset, size;

	 /// Returns #TRUE# if the file is a page.
      bool	is_page(void) const { return (flags & IS_PAGE)!=0; }
	 /// If the file is a page, returns its number. #-1# otherwise.
      int	get_page_num(void) const { return page_num; }

      File(void) : flags(0) {};
      File(const char * _name, const char * _id, const char * _title, bool _page) :
	    name(_name), id(_id), title(_title), flags(0), page_num(-1)
      {
	 if (_page) flags|=IS_PAGE;
      };
   private:
      enum FLAGS { IS_PAGE=1, HAS_NAME=2, HAS_TITLE=4 };
      char	flags;
      int	page_num;
   };

      /// The constructor
   DjVmDir(void);
   virtual ~DjVmDir();
   
      /** Decodes the directory from the specified stream. */
   void decode(ByteStream & stream);
      /** Encodes the directory into the specified stream */
   void encode(ByteStream & stream) const;

      /** Returns #TRUE# if the directory is from an {\em indirect} document
	  (where every page is stored in a separate file). */
   bool	is_indirect(void) const;
      /** Returns #TRUE# if the directory is from a {\em bundled} document
	  (where everything is bundled into one file. */
   bool	is_bundled(void) const;

      /// Translates page numbers to file records
   GP<DjVmDir::File>	page_to_file(int page_num) const;
      /// Translates file names to file records
   GP<DjVmDir::File>	name_to_file(const GString & name) const;
      /// Translates file IDs to file records
   GP<DjVmDir::File>	id_to_file(const char * id) const;
      /// Translates file shortcuts to file records
   GP<DjVmDir::File>	title_to_file(const char * title) const;
      /// Returns position of the file in the directory.
   int			get_file_pos(const File * f) const;
      /** Returns position of the file corresponding to the given page in
	  the directory */
   int			get_page_pos(int page_num) const;
      /// Returns the list of file records
   GPList<DjVmDir::File>get_files_list(void) const;
      /// Returns the number of file records
   int			get_files_num(void) const;
      /** Returns the number of file records which have the #IS_PAGE# flag
	  set to #TRUE#. */
   int			get_pages_num(void) const;

      /** Inserts the specified file record at the specified position.
	  #pos=-1# means to append */
   void		insert_file(File * file, int pos=-1);
      /** Removes a file record with the given #id# from the directory */
   void		delete_file(const char * id);
private:
   GCriticalSection 		class_lock;
   GPList<File>			files_list;
   DPArray<File>		page2file;
   GMap<GString, GP<File> >	name2file;
   GMap<GString, GP<File> >	id2file;
   GMap<GString, GP<File> >	title2file;
};

inline DjVmDir::DjVmDir(void) {}

inline DjVmDir::~DjVmDir() {}

inline bool
DjVmDir::is_indirect(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   
   GP<File> file=files_list[files_list];
   return file && file->offset==0;
}

inline bool
DjVmDir::is_bundled(void) const
{
   return !is_indirect();
}

//@}

// ----- THE END
#endif
