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
//C- $Id: IFFByteStream.h,v 1.16 2000-06-06 18:59:39 bcr Exp $


#ifndef _IFFBYTESTREAM_H_
#define _IFFBYTESTREAM_H_


/** @name IFFByteStream.h

    Files #"IFFByteStream.h"# and #"IFFByteStream.cpp"# implement a parser for
    files structured according the Electronic Arts ``EA IFF 85 Interchange
    File Format''.  IFF files are composed of a sequence of data {\em chunks}.
    Each chunk is identified by a four character {\em chunk identifier}
    describing the type of the data stored in the chunk.  A few special chunk
    identifiers, for instance #"FORM"#, are reserved for {\em composite
    chunks} which themselves contain a sequence of data chunks.  This
    conventions effectively provides IFF files with a convenient hierarchical
    structure.  Composite chunks are further identified by a secondary chunk
    identifier.
    
    We found convenient to define a {\em extended chunk identifier}.  In the
    case of a regular chunk, the extended chunk identifier is simply the
    chunk identifier, as in #"PM44"#. In the case of a composite chunk, the
    extended chunk identifier is composed by concatenating the main chunk
    identifier, a colon, and the secondary chunk identifier, as in
    #"FORM:DJVU"#.

    Class \Ref{IFFByteStream} provides a way to read or write IFF structured
    files.  Member functions provide an easy mean to position the underlying
    \Ref{ByteStream} at the beginning of each chunk and to read or write the
    data until reaching the end of the chunk.  The utility program
    \Ref{djvuinfo} demonstrates how to use class #IFFByteStream#.

    {\bf IFF Files and ZP-Coder} ---
    Class #IFFByteStream# repositions the underlying ByteStream whenever a new
    chunk is accessed.  It is possible to code chunk data with the ZP-Coder
    without worrying about the final file position. See class \Ref{ZPCodec}
    for more details.
    
    {\bf AT&T IFF Files} --- We had initially planned to exactly follow the
    IFF specifications.  Then we realized that certain versions of MSIE
    recognize any IFF file as a Microsoft AIFF sound file and pop a message
    box "Cannot play that sound".  It appears that the structure of AIFF files
    is entirely modeled after the IFF standard, with small variations
    regarding the endianness of numbers and the padding rules.  We eliminate
    this problem by casting the AT&T protection spell.  Our IFF files always
    start with the four octets #"AT&T"# followed by the fully conformant IFF
    byte stream.  Class #IFFByteStream# silently skips these four
    octets when it encounters them.

    {\bf References} --- EA IFF 85 Interchange File Format specification:\\
    \URL{http://www.cica.indiana.edu/graphics/image_specs/ilbm.format.txt} or
    \URL{http://www.tnt.uni-hannover.de/soft/compgraph/fileformats/docs/iff.pre}

    @memo 
    IFF file parser.
    @author
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id: IFFByteStream.h,v 1.16 2000-06-06 18:59:39 bcr Exp $# */
//@{

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "GException.h"
#include "GString.h"
#include "ByteStream.h"



/** ByteStream interface for an IFF file. 

    Class #IFFByteStream# augments the #ByteStream# interface with
    functions for navigating from chunk to chunk.  It works in relation
    with a ByteStream specified at construction time. 

    {\bf Reading an IFF file} --- You can read an IFF file by constructing an
    #IFFByteStream# object attached to the ByteStream containing the IFF file.
    Calling function \Ref{get_chunk} positions the file pointer at the
    beginning of the first chunk.  You can then use \Ref{ByteStream::read} to
    access the chunk data.  Function #read# will return #0# if you attempt to
    read past the end of the chunk, just as if you were trying to read past
    the end of a file. You can at any time call function \Ref{close_chunk} to
    terminate reading data in this chunk.  The following chunks can be
    accessed by calling #get_chunk# and #close_chunk# repeatedly until you
    reach the end of the file.  Function #read# is not very useful when
    accessing a composite chunk.  You can instead make nested calls to
    functions #get_chunk# and #close_chunk# in order to access the chunks
    located inside the composite chunk.
    
    {\bf Writing an IFF file} --- You can write an IFF file by constructing an
    #IFFByteStream# object attached to the seekable ByteStream object that
    will contain the IFF file.  Calling function \Ref{put_chunk} creates a
    first chunk header and positions the file pointer at the beginning of the
    chunk.  You can then use \Ref{ByteStream::write} to store the chunk data.
    Calling function \Ref{close_chunk} terminates the current chunk.  You can
    append more chunks by calling #put_chunk# and #close_chunk# repeatedly.
    Function #write# is not very useful for writing a composite chunk.  You
    can instead make nested calls to function #put_chunk# and #close_chunk# in
    order to create chunks located inside the composite chunk.

    Writing an IFF file requires a seekable ByteStream (see
    \Ref{ByteStream::is_seekable}).  This is not much of a problem because you
    can always create the IFF file into a \Ref{MemoryByteStream} and then use
    \Ref{ByteStream::copy} to transfer the IFF file into a non seekable
    ByteStream.  */

class IFFByteStream : public ByteStream
{
public:
  /** Constructs an IFFByteStream object attached to ByteStream #bs#.
      Any ByteStream can be used when reading an IFF file.  Writing
      an IFF file however requires a seekable ByteStream. */
  IFFByteStream(ByteStream &bs);
  // --- BYTESTREAM INTERFACE
  ~IFFByteStream();
  size_t read(void *buffer, size_t size);
  size_t write(const void *buffer, size_t size);
  long tell();
  void flush();
  // -- NAVIGATING CHUNKS
  /** Enters a chunk for reading.  Function #get_chunk# returns zero when the
      last chunk has already been accessed.  Otherwise it parses a chunk
      header, positions the IFFByteStream at the beginning of the chunk data,
      stores the extended chunk identifier into string #chkid#, and returns
      the non zero chunk size.  The file offset of the chunk data may be
      retrieved using function #tell#.  The chunk data can then be read using
      function #read# until reaching the end of the chunk.  Advanced users may
      supply two pointers to integer variables using arguments #rawoffsetptr#
      and #rawsizeptr#. These variables will be overwritten with the offset
      and the length of the file segment containing both the chunk header and
      the chunk data. */
  int get_chunk(GString &chkid, int *rawoffsetptr=0, int *rawsizeptr=0);
  /** Enters a chunk for writing.  Function #put_chunk# prepares a chunk
      header and positions the IFFByteStream at the beginning of the chunk
      data.  Argument #chkid# defines a extended chunk identifier for this
      chunk.  The chunk data can then be written using function #write#.  The
      chunk is terminated by a matching call to function #close_chunk#.  When
      #insertatt# is non zero, function #put_chunk# inserts the four letters
      #"AT&T"# before the chunk header, as discussed in
      \Ref{IFFByteStream.h}. */
  void put_chunk(const char *chkid, int insertatt=0);
  /** Leaves the current chunk.  This function leaves the chunk previously
      entered by a matching call to #get_chunk# and #put_chunk#.  The
      IFFByteStream is then ready to process the next chunk at the same
      hierarchical level. */
  void close_chunk();
  /** This is identical to the above, plus it adds a seek to the start of
      the next chunk.  This way we catch EOF errors with the current chunk.*/
  void seek_close_chunk();
  /** Returns true when it is legal to call #read# or #write#. */
  int ready();
  /** Returns true when the current chunk is a composite chunk. */
  int composite();
  /** Returns the current chunk identifier of the current chunk.  String
      #chkid# is overwritten with the {\em extended chunk identifier} of the
      current chunk.  The extended chunk identifier of a regular chunk is
      simply the chunk identifier, as in #"PM44"#.  The extended chunk
      identifier of a composite chunk is the concatenation of the chunk
      identifier, of a semicolon #":"#, and of the secondary chunk identifier,
      as in #"FORM:DJVU"#. */
  void short_id(GString &chkid);
  /** Returns the qualified chunk identifier of the current chunk.  String
      #chkid# is overwritten with the {\em qualified chunk identifier} of the
      current chunk.  The qualified chunk identifier of a composite chunk is
      equal to the extended chunk identifier.  The qualified chunk identifier
      of a regular chunk is composed by concatenating the secondary chunk
      identifier of the closest #"FORM"# or #"PROP"# composite chunk
      containing the current chunk, a dot #"."#, and the current chunk
      identifier, as in #"DJVU.INFO"#.  According to the EA IFF 85 identifier
      scoping rules, the qualified chunk identifier uniquely defines how the
      chunk data should be interpreted. */
  void full_id(GString &chkid);
  /** Checks a potential chunk identifier.  This function categorizes the
      chunk identifier formed by the first four characters of string #chkid#.
      It returns #0# if this is a legal identifier for a regular chunk.  It
      returns #+1# if this is a reserved composite chunk identifier.  It
      returns #-1# if this is an illegal or otherwise reserved identifier
      which should not be used.  */
  static int check_id(const char *id);
private:
  // private datatype
  struct IFFContext
  {
    IFFContext *next;
    long offStart;
    long offEnd;
    char idOne[4];
    char idTwo[4];
    char bComposite;
  };
  // Implementation
  IFFContext *ctx;
  ByteStream *bs;
  long offset;
  long seekto;
  int dir;
  // Cancel C++ default stuff
  IFFByteStream(const IFFByteStream &);
  IFFByteStream & operator=(const IFFByteStream &);
};


//@}


#endif
