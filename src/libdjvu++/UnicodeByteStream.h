//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//
// $Id: UnicodeByteStream.h,v 1.1 2001-01-17 00:14:55 bcr Exp $
// $Name:  $

#ifndef _UNICODEBYTESTREAM_H_
#define _UNICODEBYTESTREAM_H_

#ifdef __GNUC__
#pragma interface
#endif

/** @name UnicodeByteStream.h

    Files #"UnicodeByteStream.h"# and #"UnicodeByteStream.cpp"# implement a parser for
    files structured W3C Extensible Markup Language (XML) 1.0 (Second Edition).
    
    Class \Ref{UnicodeByteStream} provides a way to read or write XML files.
    files.  Member functions provide an easy mean to position the underlying
    \Ref{ByteStream}.

    {\bf References} --- W3C Extensible Markup Language (XML) 1.0
    (Second Edition)
    \URL{http://www.w3.org/TR/2000/REC-xml-20001006.html}

    @memo 
    XML file parser.
    @author
    Bill C Riemers <bcr@lizardtech.org>
    @version
    #$Id: UnicodeByteStream.h,v 1.1 2001-01-17 00:14:55 bcr Exp $# */
//@{

#include "DjVuGlobal.h"
#include "GUnicode.h"
#include "ByteStream.h"



/** ByteStream interface for an Unicode file. 

    Class #UnicodeByteStream# augments the #ByteStream# interface with
    functions for navigating Unicode documents.  It works in relation
    with a ByteStream specified at construction time. 

    {\bf Reading an Unicode file} --- You can read an Unicode file by
    constructing an #UnicodeByteStream# object attached to the ByteStream
    containing the Unicode file.
    
    {\bf Writing an Unicode file} --- You can write an Unicode file by
    constructing an #UnicodeByteStream# object attached to the seekable
    ByteStream object that will contain the XML file.

    Writing an XML file requires a seekable ByteStream (see
    \Ref{ByteStream::is_seekable}).  This is not much of a problem because you
    can always create the XML file into a \Ref{MemoryByteStream} and then use
    \Ref{ByteStream::copy} to transfer the XML file into a non seekable
    ByteStream.  */

class UnicodeByteStream : public ByteStream
{
public:
  /** Constructs an UnicodeByteStream object attached to ByteStream #bs#.
      Any ByteStream can be used when reading an XML file.  Writing
      an XML file however requires a seekable ByteStream. */
  UnicodeByteStream(GP<ByteStream> bs,
    const GUnicode::EncodeType encodetype=GUnicode::UTF8);
  UnicodeByteStream(UnicodeByteStream &bs);
  // --- BYTESTREAM INTERFACE
  ~UnicodeByteStream();
  /// Sets the encoding type and seek's to position 0.
  void set_encodetype(const GUnicode::EncodeType et=GUnicode::UTF8);
  /// Simmular to fgets(), except read aheads effect the tell() position.
  virtual GUnicode gets(size_t const t=0,unsigned long const stopat='\n',bool const inclusive=true); 
  /// Resets the gets buffering as well as physically seeking.
  virtual int seek(long offset, int whence = SEEK_SET, bool nothrow=false);
  /** Physically reads the specified bytes, and adds them to the gets
      read ahead buffer. */
  virtual size_t read(void *buffer, size_t size);
  /// Not correctly implimented...
  virtual size_t write(const void *buffer, size_t size);
  /// tell will tell you the read position, including read ahead for gets()...
  virtual long tell(void) const;
  /// Does a flush, and clears the read ahead buffer.
  virtual void flush(void);
protected:
  /// The real byte stream.
  GUnicode::EncodeType encodetype;
  GUnicode buffer;
  GP<ByteStream> bs;
private:
  // Cancel C++ default stuff
  UnicodeByteStream(const UnicodeByteStream &);
  UnicodeByteStream & operator=(const UnicodeByteStream &);
};

inline void
UnicodeByteStream::set_encodetype(const GUnicode::EncodeType et)
{
  encodetype=et;
  seek(0,SEEK_SET);
}

inline size_t
UnicodeByteStream::read(void *buf, size_t size)
{
  int retval=bs->read(buf,size);
  if(retval)
  {
    buffer+=GUnicode((unsigned char const *)buf,retval,encodetype);
  }
  return retval;
}

inline size_t
UnicodeByteStream::write(const void *buf, size_t size)
{
  buffer=GUnicode();
  return bs->write(buf,size);
}

inline long 
UnicodeByteStream::tell(void) const
{
  return bs->tell();
}

inline UnicodeByteStream & 
UnicodeByteStream::operator=(const UnicodeByteStream &uni)
{
  bs=uni.bs;
  buffer=uni.buffer;
  encodetype=uni.encodetype;
  return *this;
}

inline int 
UnicodeByteStream::seek
(long offset, int whence, bool nothrow)
{
  int retval=bs->seek(offset,whence,nothrow);
  buffer=GUnicode();
  return retval;
}

inline void 
UnicodeByteStream::flush(void)
{
  bs->flush();
  buffer=GUnicode();
}


class XMLByteStream : public UnicodeByteStream
{
public:
  XMLByteStream(GP<ByteStream> bs);
  XMLByteStream(UnicodeByteStream &bs);
  // --- BYTESTREAM INTERFACE
  ~XMLByteStream();
};

//@}

#endif

