//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: ByteStream.h,v 1.2 1999-02-01 18:32:31 leonb Exp $


#ifndef _BYTESTREAM_H
#define _BYTESTREAM_H

/** @name ByteStream.h
    
    Files #"ByteStream.h"# and #"ByteStream.cpp"# define input/output classes
    similar in spirit to the well known C++ #iostream# classes.  Class
    \Ref{ByteStream} is an abstract base class for all byte streams.  It
    defines a virtual interface and also provides utseful functions.  These
    files provide two subclasses. Class \Ref{StdioByteStream} provides a
    simple interface to the Ansi C buffered input/output functions. Class
    \Ref{MemoryByteStream} provides stream-like access to a dynamical array
    maintained in memory.

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
    Leon Bottou <leonb@research.att.com> -- initial implementation\\
    Andrei Erofeev <eaf@research.att.com> -- 
    @version
    #$Id: ByteStream.h,v 1.2 1999-02-01 18:32:31 leonb Exp $# */
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
#include "GSmartPointer.h"


/** Abstract class for a stream of bytes.  Class #ByteStream# represent an
    object from which (resp. to which) bytes can be read (resp. written) as
    with a regular file.  Virtual functions #read# and #write# must implement
    these two basic operations.  In addition, function #tell# returns an
    offset identifying the current position, and function #seek# may be used
    to change the current position.

    {\bf Note}. Both the copy constructor and the copy operator are declared
    as private members. It is threfore not possible to make multiple copies
    of instances of this class, as implied by the class semantic.  
*/
class ByteStream {
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
      Function #read# returns immediatly if #size# is zero. The actual number
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
      immediatly if #size# is zero.  The actual number of bytes written is
      returned. Function #write# may also return a value greater than zero but
      smaller than #size# for internal reasons. Programs must be ready to
      handle these cases or use function \Ref{writall}. Exception
      \Ref{GException} is thrown with a plain text error message whenever an
      error occurs. */
  virtual size_t write(const void *buffer, size_t size) = 0;
  /** Returns the offset of the current position in the ByteStream.  This
      function {\em must} be implemented by each subclass of #ByteStream#. */
  virtual long tell(void) = 0;
  /** Tests whether function #seek# can seek backwards. Class
      #ByteStream# provides a default implementation which always returns false. 
      Subclasses implementing backward seek capabilities must override this
      default implementation and return true. */
  virtual int  is_seekable(void) const;
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
      total size of the ByteStream. Exception \Ref{GException} is thrown with
      a plain text error message whenever an error occurs. */
  virtual void seek(long offset, int whence = SEEK_SET);
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
      an end-of-file mark is reached.  This is implemented by repeatitively
      calling function #read# until everything is read or until we reach an
      end-of-file mark.  Note that #read# and #readall# are equivalent when
      #size# is one. */
  size_t readall(void *buffer, size_t size);
  /** Writes data and blocks until everything has been written.  This function
      is essentially similar to function #write#.  Unlike function #write#
      however, function #writall# will only return after all #size# bytes have
      been written.  This is implemented by repeatitively calling function
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
  int is_seekable(void) const;
  void seek(long offset, int whence = SEEK_SET);
  long tell();
private:
  // Cancel C++ default stuff
  StdioByteStream(const StdioByteStream &);
  StdioByteStream & operator=(const StdioByteStream &);
private:
  // Implementation
  char can_read;
  char can_write;
  char can_seek;
  char must_close;
  FILE *fp;
  long pos;
};


/** ByteStream interface for a memory buffer.
    Class #MemoryByteStream# manages a dynamically resizeable buffer 
    from which data can be read or written.
 */

class MemoryByteStream : public ByteStream {
public:
  /** Constructs an empty MemoryByteStream.
      The buffer is initially empty. You must first use function #write#
      to store data into the buffer, use function #seek# to rewind the
      current position, and function #read# to read the data back. */
  MemoryByteStream();
  /** Constructs a MemoryByteStream with initial data.  The MemoryByteStream
      buffer is initialized with #size# bytes copied from the memory area
      pointed to by #buffer#. */
  MemoryByteStream(const void *buffer, size_t size);
  /** Constructs a MemoryByteStream with an initial string.
      The MemoryByteStream buffer is initialized with the zero terminated
      string #buffer#. */
  MemoryByteStream(const char *buffer);
  // Virtual functions
  ~MemoryByteStream();
  size_t read(void *buffer, size_t size);
  size_t write(const void *buffer, size_t size);
  int is_seekable(void) const;
  void seek(long offset, int whence = SEEK_SET);
  long tell();
  /** Returns the total number of bytes contained in the buffer.  Valid
      offsets for function #seek# range from 0 to the value returned by this
      function. */
  int size();
  /** Converts the MemoryByteStream into a character pointer.
      The returned pointer points to the internal buffer. It remains valid
      as long as nothing is written into the MemoryByteStream. */
  operator char*();
  /** Converts the MemoryByteStream into a void pointer.
      The returned pointer points to the internal buffer. It remains valid
      as long as nothing is written into the MemoryByteStream. */
  operator void*();
  /** Returns a reference to the byte at offset #n#. This reference can be
      used to read (as in #mbs[n]#) or modify (as in #mbs[n]=c#) the contents
      of the buffer. */
  char &operator[] (int n);
private:
  // Cancel C++ default stuff
  MemoryByteStream(const MemoryByteStream &);
  MemoryByteStream & operator=(const MemoryByteStream &);
private:
  // Implementation
  int where;
  GArray<char> data;
};

//@}

inline int
MemoryByteStream::size()
{
  return data.size();
}

inline
MemoryByteStream::operator char*()
{
  return (char*)data;
}

inline
MemoryByteStream::operator void*()
{
  return (void*)(char*)data;
}

inline char &
MemoryByteStream::operator[] (int n)
{
  return data[n];
}

// ------------ THE END
#endif

