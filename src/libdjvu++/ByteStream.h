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
//C- $Id: ByteStream.h,v 1.1.1.2 1999-10-22 19:29:22 praveen Exp $


#ifndef _BYTESTREAM_H
#define _BYTESTREAM_H

/** @name ByteStream.h
    
    Files #"ByteStream.h"# and #"ByteStream.cpp"# define input/output classes
    similar in spirit to the well known C++ #iostream# classes.  Class
    \Ref{ByteStream} is an abstract base class for all byte streams.  It
    defines a virtual interface and also provides useful functions.  These
    files provide two subclasses. Class \Ref{StdioByteStream} provides a
    simple interface to the Ansi C buffered input/output functions. Class
    \Ref{MemoryByteStream} provides stream-like access to a dynamical array
    maintained in memory. Class \Ref{StaticByteStream} provides read-only
    stream-like access to a user allocated data buffer.

    {\bf Notes} --- These classes were partly written because we did not want to
    depend on the standard C++ library.  The main reason however is related to
    the browser interface. We want to have a tight control over the
    implementation of subclasses because we want to use a byte stream to
    represent data passed by a web browser to a plugin.  This operation
    involves multi-threading issues that many implementations of the standard
    C++ library would squarely ignore.

    @memo 
    Input/output classes
    @author
    L\'eon Bottou <leonb@research.att.com> -- initial implementation\\
    Andrei Erofeev <eaf@research.att.com> -- 
    @version
    #$Id: ByteStream.h,v 1.1.1.2 1999-10-22 19:29:22 praveen Exp $# */
//@{

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "GException.h"
#include "GContainer.h"
#include "Arrays.h"
#include "GSmartPointer.h"


/** Abstract class for a stream of bytes.  Class #ByteStream# represent an
    object from which (resp. to which) bytes can be read (resp. written) as
    with a regular file.  Virtual functions #read# and #write# must implement
    these two basic operations.  In addition, function #tell# returns an
    offset identifying the current position, and function #seek# may be used
    to change the current position.

    {\bf Note}. Both the copy constructor and the copy operator are declared
    as private members. It is therefore not possible to make multiple copies
    of instances of this class, as implied by the class semantic.  
*/
class ByteStream : public GPEnabled {
public:
  /** @name Virtual Functions.
      These functions are usually implemented by each subclass of #ByteStream#. */
  //@{
public:
  /** Virtual destructor. */
  virtual ~ByteStream();
  /** Reads data from a ByteStream.  This function {\em must} be implemented
      by each subclass of #ByteStream#.  At most #size# bytes are read from
      the ByteStream and stored in the memory area pointed to by #buffer#.
      Function #read# returns immediately if #size# is zero. The actual number
      of bytes read is returned.  Function #read# returns a number of bytes
      smaller than #size# if the end-of-file mark is reached before filling
      the buffer. Subsequent invocations will always return value #0#.
      Function #read# may also return a value greater than zero but smaller
      than #size# for internal reasons. Programs must be ready to handle these
      cases or use function \Ref{readall}. Exception \Ref{GException} is
      thrown with a plain text error message whenever an error occurs. */
  virtual size_t read(void *buffer, size_t size) = 0;
  /** Writes data to a ByteStream.  This function {\em must} be implemented by
      each subclass of #ByteStream#.  At most #size# bytes from buffer
      #buffer# are written to the ByteStream.  Function #write# returns
      immediately if #size# is zero.  The actual number of bytes written is
      returned. Function #write# may also return a value greater than zero but
      smaller than #size# for internal reasons. Programs must be ready to
      handle these cases or use function \Ref{writall}. Exception
      \Ref{GException} is thrown with a plain text error message whenever an
      error occurs. */
  virtual size_t write(const void *buffer, size_t size) = 0;
  /** Returns the offset of the current position in the ByteStream.  This
      function {\em must} be implemented by each subclass of #ByteStream#. */
  virtual long tell(void) = 0;
  /** Sets the current position for reading or writing the ByteStream.  Class
      #ByteStream# provides a default implementation able to seek forward by
      calling function #read# until reaching the desired position.  Subclasses
      implementing better seek capabilities must override this default
      implementation.  The new current position is computed by applying
      displacement #offset# to the position represented by argument
      #whence#. The following values are recognized for argument #whence#:
      \begin{description}
      \item[#SEEK_SET#] Argument #offset# indicates the position relative to
      the beginning of the ByteStream.
      \item[#SEEK_CUR#] Argument #offset# is a signed displacement relative to
      the current position.
      \item[#SEEK_END#] Argument #offset# is a displacement relative to the end
      of the file. It is then advisable to provide a negative value for #offset#.
      \end{description}
      Results are undefined whenever the new position is greater than the
      total size of the ByteStream.  An exception \Ref{GException} is thrown
      whwnever an error occurs.  However, if argument #nothrow# is set, this
      function will return #-1# if the bytestream is not able to perform the
      required seek operation. */
  virtual int seek(long offset, int whence = SEEK_SET, bool nothrow=false);
  /** Flushes all buffers in the ByteStream.  Calling this function
      guarantees that pending data have been actually written (i.e. passed to
      the operating system). Class #ByteStream# provides a default
      implementation which does nothing. */
  virtual void flush();
  //@}
  /** @name Utility Functions.  
      Class #ByteStream# implements these functions using the virtual
      interface functions only.  All subclasses of #ByteStream# inherit these
      functions. */
  //@{
public:
  /** Reads data and blocks until everything has been read.  This function is
      essentially similar to function #read#.  Unlike function #read# however,
      function #readall# will never return a value smaller than #size# unless
      an end-of-file mark is reached.  This is implemented by repeatedly
      calling function #read# until everything is read or until we reach an
      end-of-file mark.  Note that #read# and #readall# are equivalent when
      #size# is one. */
  size_t readall(void *buffer, size_t size);
  /** Writes data and blocks until everything has been written.  This function
      is essentially similar to function #write#.  Unlike function #write#
      however, function #writall# will only return after all #size# bytes have
      been written.  This is implemented by repeatedly calling function
      #write# until everything is written.  Note that #write# and #writall#
      are equivalent when #size# is one. */
  size_t writall(const void *buffer, size_t size);
  /** Copy data from another ByteStream.  A maximum of #size# bytes are read
      from the ByteStream #bsfrom# and are written to the ByteStream #*this#
      at the current position.  Less than #size# bytes may be written if an
      end-of-file mark is reached on #bsfrom#.  This function returns the
      total number of bytes copied.  Setting argument #size# to zero (the
      default value) has a special meaning: the copying process will continue
      until reaching the end-of-file mark on ByteStream #bsfrom#, regardless
      of the number of bytes transferred.  */
  size_t copy(ByteStream &bsfrom, size_t size=0);
  /** Writes a one-byte integer to a ByteStream. */
  void write8 (unsigned int card8);
  /** Writes a two-bytes integer to a ByteStream.
      The integer most significant byte is written first,
      regardless of the processor endianness. */
  void write16(unsigned int card16);
  /** Writes a three-bytes integer to a ByteStream.
      The integer most significant byte is written first,
      regardless of the processor endianness. */
  void write24(unsigned int card24);
  /** Writes a four-bytes integer to a ByteStream. 
      The integer most significant bytes are written first,
      regardless of the processor endianness. */
  void write32(unsigned int card32);
  /** Reads a one-byte integer from a ByteStream. */
  unsigned int read8 ();
  /** Reads a two-bytes integer from a ByteStream.
      The integer most significant byte is read first,
      regardless of the processor endianness. */
  unsigned int read16();
  /** Reads a three-bytes integer from a ByteStream.
      The integer most significant byte is read first,
      regardless of the processor endianness. */
  unsigned int read24();
  /** Reads a four-bytes integer from a ByteStream.
      The integer most significant bytes are read first,
      regardless of the processor endianness. */
  unsigned int read32();
  //@}
protected:
  ByteStream() {};
private:
  // Cancel C++ default stuff
  ByteStream(const ByteStream &);
  ByteStream & operator=(const ByteStream &);
};


/** ByteStream interface for stdio files. 
    The virtual member functions #read#, #write#, #tell# and #seek# are mapped
    to the well known stdio functions #fread#, #fwrite#, #ftell# and #fseek#.
    @see Unix man page fopen(3), fread(3), fwrite(3), ftell(3), fseek(3) */

class StdioByteStream : public ByteStream {
public:
  /** Constructs a ByteStream for accessing the file named #filename#.
      Arguments #filename# and #mode# are similar to the arguments of the well
      known stdio function #fopen#. In addition a filename of #-# will be
      interpreted as the standard output or the standard input according to
      #mode#.  This constructor will open a stdio file and construct a
      ByteStream object accessing this file. Destroying the ByteStream object
      will flush and close the associated stdio file.  Exception
      \Ref{GException} is thrown with a plain text error message if the stdio
      file cannot be opened. */
  StdioByteStream(const char *filename, const char *mode="rb");
  /** Constructs a ByteStream for accessing the stdio file #f#.
      Argument #mode# indicates the type of the stdio file, as in the
      well known stdio function #fopen#.  Destroying the ByteStream
      object will not close the stdio file #f#. */
  StdioByteStream(FILE *f, const char *mode="rb");
  // Virtual functions
  ~StdioByteStream();
  size_t read(void *buffer, size_t size);
  size_t write(const void *buffer, size_t size);
  void flush();
  int seek(long offset, int whence = SEEK_SET, bool nothrow=false);
  long tell();
private:
  // Cancel C++ default stuff
  StdioByteStream(const StdioByteStream &);
  StdioByteStream & operator=(const StdioByteStream &);
private:
  // Implementation
  char can_read;
  char can_write;
  char must_close;
  FILE *fp;
  long pos;
};


/** ByteStream interface managing a memory buffer.  
    Class #MemoryByteStream# manages a dynamically resizeable buffer from
    which data can be read or written.  The buffer itself is organized as an
    array of blocks of 4096 bytes.  */

class MemoryByteStream : public ByteStream {
public:
  /** Constructs an empty MemoryByteStream.
      The buffer is initially empty. You must first use function #write#
      to store data into the buffer, use function #seek# to rewind the
      current position, and function #read# to read the data back. */
  MemoryByteStream();
  /** Constructs a MemoryByteStream by copying initial data.  The
      MemoryByteStream buffer is initialized with #size# bytes copied from the
      memory area pointed to by #buffer#. */
  MemoryByteStream(const void *buffer, size_t size);
  /** Constructs a MemoryByteStream by copying an initial string.  The
      MemoryByteStream buffer is initialized with the null terminated string
      #buffer#. */
  MemoryByteStream(const char *buffer);
  // Virtual functions
  ~MemoryByteStream();
  size_t read(void *buffer, size_t size);
  size_t write(const void *buffer, size_t size);
  int    seek(long offset, int whence=SEEK_SET, bool nothrow=false);
  long   tell();
  /** Erases everything in the memorybytestream.
      The current location is reset to zero. */
  void empty();
  /** Returns the total number of bytes contained in the buffer.  Valid
      offsets for function #seek# range from 0 to the value returned by this
      function. */
  int size() const;
  /** Returns a reference to the byte at offset #n#. This reference can be
      used to read (as in #mbs[n]#) or modify (as in #mbs[n]=c#) the contents
      of the buffer. */
  char &operator[] (int n);
  /** Copies all internal data into \Ref{TArray} and returns it */
  TArray<char> get_data(void);
private:
  // Cancel C++ default stuff
  MemoryByteStream(const MemoryByteStream &);
  MemoryByteStream & operator=(const MemoryByteStream &);
  // Current position
  int where;
protected:
  /** Reads data from a random position. This function reads at most #sz#
      bytes at position #pos# into #buffer# and returns the actual number of
      bytes read.  The current position is unchanged. */
  size_t readat(void *buffer, size_t sz, int pos);
  /** Number of bytes in internal buffer. */
  int bsize;
  /** Number of 4096 bytes blocks. */
  int nblocks;
  /** Pointers (possibly null) to 4096 bytes blocks. */
  char **blocks;
};


inline int
MemoryByteStream::size() const
{
  return bsize;
}

inline char &
MemoryByteStream::operator[] (int n)
{
  return blocks[n>>12][n&0xfff];
}



/** Read-only ByteStream interface to a memory area.  
    Class #StaticByteStream# implements a read-only ByteStream interface for a
    memory area specified by the user at construction time. Calls to function
    #read# directly access this memory area.  The user must therefore make
    sure that its content remain valid long enough.  */

class StaticByteStream : public ByteStream
{
public:
  /** Creates a StaticByteStream object for allocating the memory area of
      length #sz# starting at address #buffer#. */
  StaticByteStream(const char *buffer, size_t sz);
  /** Creates a StaticByteStream object for allocating the null terminated
      memory area starting at address #buffer#. */
  StaticByteStream(const char *buffer);  
  // Virtual functions
  size_t read(void *buffer, size_t sz);
  size_t write(const void *buffer, size_t sz);
  int    seek(long offset, int whence = SEEK_SET, bool nothrow=false);
  long tell();
private:
  const char *data;
  int bsize;
  int where;
};

//@}



// ------------ THE END
#endif

