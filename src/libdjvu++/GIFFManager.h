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
// $Id: GIFFManager.h,v 1.10 2000-11-02 01:08:34 bcr Exp $
// $Name:  $


#ifndef _GIFFMANAGER_H
#define _GIFFMANAGER_H

#ifdef __GNUC__
#pragma interface
#endif

#include "IFFByteStream.h"
#include "GContainer.h"
#include "Arrays.h"
#include "GSmartPointer.h"
#include "GString.h"

/** @name GIFFManager.h

    Files #"GIFFManager.h"# and #"GIFFManager.cpp"# define more convenient
    interface to IFF files. You may want to use the {\Ref GIFFManager} class
    instead of coping with {\Ref IFFByteStream} especially when you have to
    insert or move chunks, which is a kind of tricky with sequential access
    provided by {\Ref IFFByteStream}.

    You will mostly deal with {\Ref GIFFManager} class, but sometimes you may
    want to use {\Ref GIFFChunk}s as well thus bypassing {\Ref GIFFManager}'s
    interface and working with the chunks hierarchy yourself.
    
    Interface to IFF files.
    @author 
    Andrei Erofeev <eaf@geocities.com> -- Initial implementation.
    @version 
    #$Id: GIFFManager.h,v 1.10 2000-11-02 01:08:34 bcr Exp $# */

/** #GIFFChunk# is the base class for other IFF chunks understood by
    {\Ref GIFFManager}. It provides some basic interface, and is not supposed
    to be used on its own. */

class GIFFChunk : public GPEnabled
{
private:
   char			name[5];
   GString		type;
   GPList<GIFFChunk>	chunks;
   TArray<char>		data;

   static void	decode_name(const char * name, GString * short_name, int * number);
public:
      /// Returns the name of the chunk (without possible #FORM:# or similar prefixes)
   GString	get_name(void) const;
      /// Returns full chunk name, with possible container specification
   GString	get_full_name(void) const;
      /// Returns the chunk type, like #CAT# for chunk #CAT:DJVU#
   GString	get_type(void) const;
      /// Returns TRUE if the chunk may contain other chunks or FALSE otherwise
   bool		is_container(void) const;
      /** Sets the chunk name. The {\em name} may not contain dots or brackets,
	  but {\bf may} contain colons. */
   void		set_name(const char * name);
      /** Parses the {\em name} probably containing colon and compares it
	  with its own name returning TRUE if they are the same */
   bool		check_name(const char * name);

      /** Adds the {\em chunk} to the chunks list at position {\em order}.
	  Set {\em order} to #-1# to append the chunk to the list.
          {\bf Note!} By adding chunk #PROP# you will convert this chunk
          to type #LIST# {\em automatically}. */
   void		add_chunk(const GP<GIFFChunk> & chunk, int order=-1);
      /** Removes the chunk with given {\em name}. The {\em name} may not
	  contain dots, but MAY contain colons and brackets (the latter -
	  for specifying the chunk number) */
   void		del_chunk(const char * name);
      /** Returns the chunk with given {\em name}. The {\em name} may not
	  contain dots, but MAY contain colons and brackets (the latter -
	  for specifying the chunk number). If {\em position} is not zero
	  then the chunk position in its parent will be put into #*position# */
   GP<GIFFChunk>get_chunk(const char * name, int * position=0);
      /** Returns the number of chunks with given {\em name}. The {\em name}
	  may not contain dots and brackets. If {\em name} is ZERO, the
	  total number of chunks will be returned. */
   int		get_chunks_number(const char * name=0);
      /** Returns the data array for plain chunks */
   TArray<char>	get_data(void) const;
   
      /** Saves the chunk into the {\Ref IFFByteStream}.
	  Set {\em use_trick} to #1# if this is a top-level chunk */
   void		save(IFFByteStream & istr, bool use_trick=0);
      
      /// Default constructor
   GIFFChunk(void);
      /** Constructs the chunk with the given name. The {\em name} may not
	  contain dots colons or brackets */
   GIFFChunk(const char * name);
      /** Constructs the {\em plain chunk} containing raw data */
   GIFFChunk(const char * name, const TArray<char> & data);
      /// Destructor
   virtual ~GIFFChunk(void);
};

inline GString
GIFFChunk::get_name(void) const { return GString(name, 4); }

inline GString
GIFFChunk::get_type(void) const { return type; };

inline GString
GIFFChunk::get_full_name(void) const { return get_type()+":"+get_name(); };

inline bool
GIFFChunk::is_container(void) const { return type.length()!=0; };

inline TArray<char>
GIFFChunk::get_data(void) const { return data; };

inline
GIFFChunk::GIFFChunk(void) { name[0]=0; }

inline
GIFFChunk::GIFFChunk(const char * name) { set_name(name); }

inline
GIFFChunk::GIFFChunk(const char * name, const TArray<char> & data_in) :
      data(data_in)
{
   set_name(name);
}

inline
GIFFChunk::~GIFFChunk(void) {};

//************************************************************************

/** Intuitive interface to IFF files.

    It's too terrible to keep reading/writing IFF files chunk after chunk
    using {\Ref IFFByteStream}s. This class allows you to operate with chunks
    as with structures or arrays without even caring about the byte streams.

    Some of the examples are below:
    \begin{verbatim}
       GP<GIFFChunk> chunk;
       chunk=manager1.get_chunk("BG44[2]");
       manager2.add_chunk(".FORM:DJVU.BG44[-1]", chunk);
    \end{verbatim}

    {\bf Chunk name}
    \begin{itemize}
       \item Every chunk name may contain optional prefix #FORM:#, #LIST:#,
             #PROP:# or #CAT:#. If the prefix is omitted and the chunk happens
	     to contain other chunks, #FORM:# will be assumed.
       \item Every chunk name may be {\em short} or {\em complete}.
             {\em short} chunk names may not contain dots as they're a
	     subchunks names with respect to a given chunk.
	     {\em complete} chunk names may contain dots. But there may be
	     or may not be the {\em leading dot} in the name. If the
	     {\em leading dot} is present, then the name is assumed to contain
	     the name of the top-level chunk as well. Otherwise it's treated
	     {\em with respect} to the top-level chunk. You may want to use
	     the leading dot only when you add a chunk to an empty document,
	     since a command like #manager.addChunk(".FORM:DJVU.BG44", chunk)#
	     will create the top level chunk of the requested type (#FORM:DJVU#)
	     and will add chunk #BG44# to it {\em automatically}.
       \item You may use {\em brackets} in the name to specify the chunk's
             position. The meaning of the number inside the brackets depends
	     on the function you call. In most of the cases this is the number
	     of the chunk with the given name in the parent chunk. But sometimes
	     (as in #addChunk(name, buffer, length)#) the brackets at the
	     end of the #name# actually specify the {\em position} of the
	     chunk in the parent. For example, to insert #INCL# chunk into
	     #DJVU# form at position #1# (make it the second) you may want to
	     use #manager.addChunk(".DJVU.INCL[1]", data, size)#. At the same
	     time, to get 2-nd chunk with name #BG44# from form #DJVU# you
	     should do smth like #chunk=manager.getChunk("BG44[1]")#. Note, that
	     here the manager will search for chunk #BG44# in form #DJVU# and
	     will take the second {\em found} one.
    \end{itemize} */

class GIFFManager
{
private:
   GP<GIFFChunk>	top_level;

   static const char *	check_leading_dot(const char * name);
public:
      /// Sets the name of the top level chunk to {\em name}
   void		set_name(const char * name);
      /** Adds the chunk {\em chunk} to chunk with name {\em parent_name} at
	  position {\em pos}. {\em parent_name} may contain dots, brackets
	  and colons. All missing chunks in the chain will be created.

	  {\bf Examples:}
	  \begin{verbatim}
	     ;; To set the top-level chunk to 'ch'
	     m.addChunk(".", ch);
	     ;; To add 'ch' to the top-level chunk "DJVU" creating it if necessary
	     m.addChunk(".DJVU", ch);
	     ;; Same as above regardless of top-level chunk name
	     m.addChunk("", ch);
	     ;; To add 'ch' to 2nd FORM DJVU in top-level form DJVM
	     m.addChunk(".FORM:DJVM.FORM:DJVU[1]", ch);
	     ;; Same thing regardless of the top-level chunk name
	     m.addChunk("FORM:DJVU[1]", ch);
	  \end{verbatim} */
   void		add_chunk(const char * parent_name, const GP<GIFFChunk> & chunk, int pos=-1);
      /** If {\em name}={\em name1}.{\em name2} where {\em name2} doesn't
	  contain dots, then #addChunk()# will create plain chunk with
	  name {\em name2} with data {\em buffer} of size {\em length} and
	  will add it to chunk {\em name1} in the same way as
	  #addChunk(name, chunk, pos)# function would do it. The #pos# in
	  this case is either #-1# (append) or is extracted from between
          brackets if the {\em name} ends with them.
	  
          {\bf Examples:}
          \begin{verbatim}
             ;; To insert INCL chunk at position 2 (make it 3rd)
             m.addChunk("INCL[2]", data, length);
             ;; To append chunk BG44 to 2nd DjVu file inside DjVm archive:
             m.addChunk(".DJVM.DJVU[1].BG44", data, length);
          \end{verbatim} */
   void		add_chunk(const char * name, const TArray<char> & data);
      /** Will remove chunk with name {\em name}. You may use dots, colons
	  and brackets to specify the chunk uniquely.

	  {\bf Examples:}
	  \begin{verbatim}
	     ;; To remove 2nd DjVu document from DjVm archive use
	     m.delChunk(".DJVM.DJVU[1]");
	     ;; Same thing without top-level chunk name specification
	     m.delChunk("DJVU[1]");
	     ;; Same thing for the first DJVU chunk
	     m.delChunk("DJVU");
	  \end{verbatim}
      */
   void		del_chunk(const char * name);
      /** Will return the number of chunks with given name. The {\em name} may
	  not end with brackets, but may contain them inside. It may also
	  contain dots and colons. If {\em name} is ZERO, the total number
	  of chunks will be returned.

	  {\bf Examples:}
	  \begin{verbatim}
	     ;; To get the number of DJVU forms inside DjVm document
	     m.getChunksNumber(".DJVM.DJVU");
	     ;; Same thing without top-level chunk name specification
	     m.getChunksNumber("DJVU");
	  \end{verbatim}
      */
   int		get_chunks_number(const char * name=0);

      /** Returns the chunk with name {\em name}. The {\em name} may contain dots
	  colons and slashes. If {\em position} is not zero, #*position# will
	  be assigned the position of the found chunk in the parent chunk.

	  {\bf Examples:}
	  \begin{verbatim}
	     ;; To get the directory chunk of DjVm document
	     m.getChunk(".DJVM.DIR0");
	     ;; To get chunk corresponding to 2nd DJVU form
	     m.getChunk(".DJVU[1]");
	  \end{verbatim} */
   GP<GIFFChunk>get_chunk(const char * name, int * position=0);

      /** Loads the composite {\em chunk}'s contents from stream {\em istr}. */
   void		load_chunk(IFFByteStream & istr, GP<GIFFChunk> chunk);
      /** Loads the file contents from stream {\em str} */
   void		load_file(ByteStream & str);
      /** Loads the file contents from the data array {\em data} */
   void		load_file(const TArray<char> & data);
      /** Saves all the chunks into stream {\em str} */
   void		save_file(ByteStream & str);
      /** Saves all the chunks into the data array {\em data} */
   void		save_file(TArray<char> & data);

      /// Default constructor
   GIFFManager(void);
      /** Constructs the {\Ref GIFFManager} and assigns name {\em name} to
	  the top-level chunk. you may use chunk type names (before colon)
	  to set the top-level chunk type, or omit it to work with #FORM# */
   GIFFManager(const char * name);
      /// Destructor
   virtual ~GIFFManager(void);
};

inline void
GIFFManager::set_name(const char * name)
{
   top_level->set_name(name);
}

inline
GIFFManager::GIFFManager(void) : top_level(new GIFFChunk()) {};

inline
GIFFManager::GIFFManager(const char * name) :
      top_level(new GIFFChunk(name)) {};

inline
GIFFManager::~GIFFManager(void) {};

#endif
