//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1997-2001 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: ByteStream.cpp,v 1.63 2001-04-06 00:11:23 bcr Exp $
// $Name:  $

// - Author: Leon Bottou, 04/1997

#ifdef __GNUC__
#pragma implementation
#endif
#include "ByteStream.h"
#include "GOS.h"
#include "GURL.h"

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#else
#ifdef macintosh
#include <unistd.h>

_MSL_IMP_EXP_C int _dup(int);
_MSL_IMP_EXP_C int _dup2(int,int);
_MSL_IMP_EXP_C int _close(int);

							
__inline int dup(int _a ) 			{ return _dup(_a);}
__inline int dup2(int _a, int _b ) 	{ return _dup2(_a, _b);}

#else
#ifndef UNDER_CE
#include <io.h>
#endif
#endif
#endif
#ifndef UNDER_CE
#include <errno.h>
#endif

/** ByteStream interface for stdio files. 
    The virtual member functions #read#, #write#, #tell# and #seek# are mapped
    to the well known stdio functions #fread#, #fwrite#, #ftell# and #fseek#.
    @see Unix man page fopen(3), fread(3), fwrite(3), ftell(3), fseek(3) */

class ByteStream::Stdio : public ByteStream {
public:
  Stdio(void);

  /** Constructs a ByteStream for accessing the file named #url#.
      Arguments #url# and #mode# are similar to the arguments of the well
      known stdio function #fopen#. In addition a url of #-# will be
      interpreted as the standard output or the standard input according to
      #mode#.  This constructor will open a stdio file and construct a
      ByteStream object accessing this file. Destroying the ByteStream object
      will flush and close the associated stdio file.  Returns an error code
      if the stdio file cannot be opened. */
  GString init(const GURL &url, const char * const mode);

  /** Constructs a ByteStream for accessing the stdio file #f#.
      Argument #mode# indicates the type of the stdio file, as in the
      well known stdio function #fopen#.  Destroying the ByteStream
      object will not close the stdio file #f# unless closeme is true. */
  GString init(FILE * const f, const char * const mode="rb", const bool closeme=false);

  /** Initializes from stdio */
  GString init(const char mode[]);

  // Virtual functions
  ~Stdio();
  virtual size_t read(void *buffer, size_t size);
  virtual size_t write(const void *buffer, size_t size);
  virtual void flush(void);
  virtual int seek(long offset, int whence = SEEK_SET, bool nothrow=false);
  virtual long tell(void) const;
private:
  // Cancel C++ default stuff
  Stdio(const Stdio &);
  Stdio & operator=(const Stdio &);
private:
  // Implementation
  bool can_read;
  bool can_write;
  bool must_close;
protected:
  FILE *fp;
  long pos;
};

inline GString
ByteStream::Stdio::init(FILE * const f,const char mode[],const bool closeme)
{
  fp=f;
  must_close=closeme;
  return init(mode);
}


/** ByteStream interface managing a memory buffer.  
    Class #ByteStream::Memory# manages a dynamically resizable buffer from
    which data can be read or written.  The buffer itself is organized as an
    array of blocks of 4096 bytes.  */

class ByteStream::Memory : public ByteStream
{
public:
  /** Constructs an empty ByteStream::Memory.
      The buffer is initially empty. You must first use function #write#
      to store data into the buffer, use function #seek# to rewind the
      current position, and function #read# to read the data back. */
  Memory();
  /** Constructs a Memory by copying initial data.  The
      Memory buffer is initialized with #size# bytes copied from the
      memory area pointed to by #buffer#. */
  GString init(const void * const buffer, const size_t size);
  // Virtual functions
  ~Memory();
  virtual size_t read(void *buffer, size_t size);
  virtual size_t write(const void *buffer, size_t size);
  virtual int    seek(long offset, int whence=SEEK_SET, bool nothrow=false);
  virtual long   tell(void) const;
  /** Erases everything in the Memory.
      The current location is reset to zero. */
  void empty();
  /** Returns the total number of bytes contained in the buffer.  Valid
      offsets for function #seek# range from 0 to the value returned by this
      function. */
  virtual int size(void) const;
  /** Returns a reference to the byte at offset #n#. This reference can be
      used to read (as in #mbs[n]#) or modify (as in #mbs[n]=c#) the contents
      of the buffer. */
  char &operator[] (int n);
  /** Copies all internal data into \Ref{TArray} and returns it */
private:
  // Cancel C++ default stuff
  Memory(const Memory &);
  Memory & operator=(const Memory &);
  // Current position
  int where;
protected:
  /** Reads data from a random position. This function reads at most #sz#
      bytes at position #pos# into #buffer# and returns the actual number of
      bytes read.  The current position is unchanged. */
  virtual size_t readat(void *buffer, size_t sz, int pos);
  /** Number of bytes in internal buffer. */
  int bsize;
  /** Number of 4096 bytes blocks. */
  int nblocks;
  /** Pointers (possibly null) to 4096 bytes blocks. */
  char **blocks;
  /** Pointers (possibly null) to 4096 bytes blocks. */
  GPBuffer<char *> gblocks;
};



inline int
ByteStream::Memory::size(void) const
{
  return bsize;
}

inline char &
ByteStream::Memory::operator[] (int n)
{
  return blocks[n>>12][n&0xfff];
}



/** Read-only ByteStream interface to a memory area.  
    Class #ByteStream::Static# implements a read-only ByteStream interface for a
    memory area specified by the user at construction time. Calls to function
    #read# directly access this memory area.  The user must therefore make
    sure that its content remain valid long enough.  */

class ByteStream::Static : public ByteStream
{
public:
  /** Creates a Static object for allocating the memory area of
      length #sz# starting at address #buffer#. */
  Static(const void * const buffer, const size_t sz);
  // Virtual functions
  virtual size_t read(void *buffer, size_t sz);
  virtual int    seek(long offset, int whence = SEEK_SET, bool nothrow=false);
  virtual long tell(void) const;
  /** Returns the total number of bytes contained in the buffer, file, etc.
      Valid offsets for function #seek# range from 0 to the value returned
      by this function. */
  virtual int size(void) const;
protected:
  const char *data;
  int bsize;
private:
  int where;
};


inline int
ByteStream::Static::size(void) const
{
  return bsize;
}

#ifdef UNIX
/** Read-only ByteStream interface to a memmap area.
    Class #MemoryMapByteStream# implements a read-only ByteStream interface
    for a memory map to a file. */

class MemoryMapByteStream : public ByteStream::Static
{
public:
  MemoryMapByteStream(void);
  virtual ~MemoryMapByteStream();  
private:
  GString init(const int fd, const bool closeme);
  GString init(FILE *const f,const bool closeme);
  friend ByteStream;
};
#endif

//// CLASS BYTESTREAM


ByteStream::~ByteStream()
{
}

int 
ByteStream::scanf(const char *fmt, ...)
{
  G_THROW("ByteStream.not_implimented"); // This is a place holder function.
  return 0;
}

size_t 
ByteStream::read(void *buffer, size_t sz)
{
  G_THROW("ByteStream.cant_read");      //  Cannot read from a ByteStream created for writing
  return 0;
}

size_t 
ByteStream::write(const void *buffer, size_t sz)
{
  G_THROW("ByteStream.cant_write");      //  Cannot write from a ByteStream created for reading
  return 0;
}

void
ByteStream::flush()
{
}

int
ByteStream::seek(long offset, int whence, bool nothrow)
{
  int nwhere = 0;
  int ncurrent = tell();
  switch (whence)
    {
    case SEEK_SET:
      nwhere=0; break;
    case SEEK_CUR:
      nwhere=ncurrent; break;
    case SEEK_END: 
    {
      if(offset)
      {
        if (nothrow)
          return -1;
        G_THROW("ByteStream.backwards");
      }
      char buffer[1024];
      int bytes;
      while((bytes=read(buffer, sizeof(buffer))))
        EMPTY_LOOP;
      return 0;
    }
    default:
      G_THROW("ByteStream.bad_arg");       //  Illegal argument in seek
    }
  nwhere += offset;
  if (nwhere < ncurrent) 
  {
    //  Seeking backwards is not supported by this ByteStream
    if (nothrow)
      return -1;
    G_THROW("ByteStream.backward");
  }
  while (nwhere>ncurrent)
  {
    char buffer[1024];
    const int xbytes=(ncurrent+(int)sizeof(buffer)>nwhere)
      ?(nwhere - ncurrent):(int)sizeof(buffer);
    const int bytes = read(buffer, xbytes);
    ncurrent += bytes;
    if (!bytes)
      G_THROW("EOF");
    //  Seeking works funny on this ByteStream (ftell() acts strange)
    if (ncurrent!=tell())
      G_THROW("ByteStream.seek");
  }
  return 0;
}

size_t 
ByteStream::readall(void *buffer, size_t size)
{
  size_t total = 0;
  while (size > 0)
    {
      int nitems = read(buffer, size);
      // Replaced perror() below with G_THROW(). It still makes little sense
      // as there is no guarantee, that errno is right. Still, throwing
      // exception instead of continuing to loop is better.
      // - eaf
      if(nitems < 0) 
#ifndef UNDER_CE
        G_THROW(strerror(errno));               //  (No error in the DjVuMessageFile)
#else
        G_THROW("ByteStream.read_error");       //  readall, read error.
#endif
      if (nitems == 0)
        break;
      total += nitems;
      size -= nitems; 
      buffer = (void*)((char*)buffer + nitems);
    }
  return total;
}

size_t
ByteStream::format(const char *fmt, ... )
{
  va_list args;
  va_start(args, fmt); 
  GString message;
  return writestring(message.format(fmt, args));
}

size_t
ByteStream::writestring(const GString &s)
{
  const GString msg((cp==NATIVE)?s.getUTF82Native():s);
  return writall((const char *)msg,msg.length());
}

size_t 
ByteStream::writall(const void *buffer, size_t size)
{
  size_t total = 0;
  while (size > 0)
    {
      size_t nitems = write(buffer, size);
      if (nitems == 0)
        G_THROW("ByteStream.write_error");      //  Unknown error in write
      total += nitems;
      size -= nitems; 
      buffer = (void*)((char*)buffer + nitems);
    }
  return total;
}

size_t 
ByteStream::copy(ByteStream &bsfrom, size_t size)
{
  size_t total = 0;
  char buffer[1024];
  for(;;)
    {
      size_t bytes = sizeof(buffer);
      if (size>0 && bytes>size-total)
        bytes = size - total;
      if (bytes == 0)
        break;
      bytes = bsfrom.read((void*)buffer, bytes);
      if (bytes == 0)
        break;
      writall((void*)buffer, bytes);
      total += bytes;
    }
  return total;
}


void 
ByteStream::write8 (unsigned int card)
{
  unsigned char c[1];
  c[0] = (card) & 0xff;
  if (write((void*)c, sizeof(c)) != sizeof(c))
#ifndef UNDER_CE
        G_THROW(strerror(errno));                     //  (No error in the DjVuMessageFile)
#else
        G_THROW("ByteStream.write\t8");               //  write8, write error.
#endif
}

void 
ByteStream::write16(unsigned int card)
{
  unsigned char c[2];
  c[0] = (card>>8) & 0xff;
  c[1] = (card) & 0xff;
  if (writall((void*)c, sizeof(c)) != sizeof(c))
#ifndef UNDER_CE
        G_THROW(strerror(errno));                     //  (No error in the DjVuMessageFile)
#else
  G_THROW("ByteStream.write\t16");                    //  write16, write error.
#endif
}

void 
ByteStream::write24(unsigned int card)
{
  unsigned char c[3];
  c[0] = (card>>16) & 0xff;
  c[1] = (card>>8) & 0xff;
  c[2] = (card) & 0xff;
  if (writall((void*)c, sizeof(c)) != sizeof(c))
#ifndef UNDER_CE
        G_THROW(strerror(errno));                     //  (No error in the DjVuMessageFile)
#else
        G_THROW("ByteStream.write\t24");              //  write24, write error.
#endif
}

void 
ByteStream::write32(unsigned int card)
{
  unsigned char c[4];
  c[0] = (card>>24) & 0xff;
  c[1] = (card>>16) & 0xff;
  c[2] = (card>>8) & 0xff;
  c[3] = (card) & 0xff;
  if (writall((void*)c, sizeof(c)) != sizeof(c))
#ifndef UNDER_CE
        G_THROW(strerror(errno));                     //  (No error in the DjVuMessageFile)
#else
        G_THROW("ByteStream.write\t32");              //  write32, write error.
#endif
}

unsigned int 
ByteStream::read8 ()
{
  unsigned char c[1];
  if (readall((void*)c, sizeof(c)) != sizeof(c))
    G_THROW("EOF");
  return c[0];
}

unsigned int 
ByteStream::read16()
{
  unsigned char c[2];
  if (readall((void*)c, sizeof(c)) != sizeof(c))
    G_THROW("EOF");
  return (c[0]<<8)+c[1];
}

unsigned int 
ByteStream::read24()
{
  unsigned char c[3];
  if (readall((void*)c, sizeof(c)) != sizeof(c))
    G_THROW("EOF");
  return (((c[0]<<8)+c[1])<<8)+c[2];
}

unsigned int 
ByteStream::read32()
{
  unsigned char c[4];
  if (readall((void*)c, sizeof(c)) != sizeof(c))
    G_THROW("EOF");
  return (((((c[0]<<8)+c[1])<<8)+c[2])<<8)+c[3];
}



//// CLASS ByteStream::Stdio

ByteStream::Stdio::Stdio(void)
: can_read(false),can_write(false),must_close(true),fp(0),pos(0)
{}

ByteStream::Stdio::~Stdio()
{
  if (fp && must_close)
    fclose(fp);
}

GString
ByteStream::Stdio::init(const char mode[])
{
  char const *mesg=0; 
  if(!fp)
    must_close=false;
  for (const char *s=mode; s && *s; s++)
  {
    switch(*s) 
    {
      case 'r':
        can_read=true;
        if(!fp) fp=stdin;
        break;
      case 'w': 
      case 'a':
        can_write=true;
        if(!fp) fp=stdout;
        break;
      case '+':
        can_read=can_write=true;
        break;
      case 'b':
        break;
      default:
        mesg="ByteStream.bad_mode"; //  Illegal mode in Stdio
    }
  }
  GString retval;
  if(!mesg)
  {
    tell();
  }else
  {
    retval=mesg;
  }
  if(mesg &&(fp && must_close))
  {
    fclose(fp);
    fp=0;
    must_close=false;
  }
  return retval;
}

static FILE *
urlfopen(const GURL &url,const char mode[])
{
  return fopen((const char *)url.NativeFilename(),mode);
}

#ifdef UNIX
static int
urlopen(const GURL &url, const int mode, const int perm)
{
  return open((const char *)url.NativeFilename(),mode,perm);
}
#endif /* UNIX */

GString
ByteStream::Stdio::init(const GURL &url, const char mode[])
{
  GString retval;
  if (url.fname() != '-')
  {
#ifdef macintosh
    fp = urlfopen(url,mode);
#else
    /* MBCS */
    fp=urlfopen(url,mode);
#if 0
    if (!fp)
    {
      GString utf8Filename(GOS::expand_name(filename));
      GString utf8Basename(GOS::basename(filename));
      GString nativeBasename(GOS::encode_mbcs_reserved(utf8Basename.getUTF82Native()));
      char *fpath;
      const size_t fpath_length=1+utf8Filename.length()-utf8Basename.length();
      GPBuffer<char> gfpath(fpath,fpath_length);
      gfpath.clear();
      strncpy(fpath,(char*)(const char*)utf8Filename, utf8Filename.length() - utf8Basename.length());
      nativeFilename = GString(fpath).getUTF82Native();
      nativeFilename+=nativeBasename;
      fp = fopen((char*)(const char *)nativeFilename, mode);
    }
#endif
#endif
    if (!fp)
    {
#ifndef UNDER_CE
      //  Failed to open '%s': %s
      G_THROW("ByteStream.open_fail\t"+url.get_string()+"\t"+GString(strerror(errno)).getNative2UTF8());
#else
      G_THROW("ByteStream.open_fail2");                  //  StdioByteStream::StdioByteStream, failed to open file.
#endif
    }
    /*MBCS*/
    if (!fp)
    {
#ifndef UNDER_CE
      retval=GString("ByteStream.open_fail\t")+url+strerror(errno);
         //  Failed to open '%s': %s
#else
      retval="ByteStream.open_fail2"; //  Stdio, failed to open file.
#endif
    }
  }
  return retval.length()?retval:init(mode);
}

size_t 
ByteStream::Stdio::read(void *buffer, size_t size)
{
  if (!can_read)
    G_THROW("ByteStream.no_read");                    //  Stdio not opened for reading
  size_t nitems;
  do
  {
    clearerr(fp);
    nitems = fread(buffer, 1, size, fp); 
    if (nitems<=0 && ferror(fp))
    {
#ifdef EINTR
      if (errno!=EINTR)
#endif
#ifndef UNDER_CE
        G_THROW(strerror(errno));                     //  (No error in the DjVuMessageFile)
#else
        G_THROW("ByteStream.read_error2");            //  read, read error.
#endif
    }else
    {
      break;
    }
  }while(true);
  pos += nitems;
  return nitems;
}

size_t 
ByteStream::Stdio::write(const void *buffer, size_t size)
{
  if (!can_write)
    G_THROW("ByteStream.no_write");                   //  Stdio not opened for writing
  size_t nitems;
  do
  {
    clearerr(fp);
    nitems = fwrite(buffer, 1, size, fp);
    if (nitems<=0 && ferror(fp))
    {
#ifdef EINTR
      if (errno!=EINTR)
#endif
#ifndef UNDER_CE
        G_THROW(strerror(errno));                     //  (No error in the DjVuMessageFile)
#else
        G_THROW("ByteStream.write_error2");           //  write, write error.
#endif
    }else
    {
      break;
    }
  }while(true);
  pos += nitems;
  return nitems;
}

void
ByteStream::Stdio::flush()
{
  if (fflush(fp) < 0)
#ifndef UNDER_CE
        G_THROW(strerror(errno));                     //  (No error in the DjVuMessageFile)
#else
        G_THROW("ByteStream.flush_error");            //  flush, flush error.
#endif
}

long 
ByteStream::Stdio::tell(void) const
{
  long x = ftell(fp);
  if (x >= 0)
  {
    Stdio *sbs=const_cast<Stdio *>(this);
    (sbs->pos) = x;
  }else
  {
    x=pos;
  }
  return x;
}

int
ByteStream::Stdio::seek(long offset, int whence, bool nothrow)
{
  if (whence==SEEK_SET && offset>=0 && offset==ftell(fp))
    return 0;
  if (fseek(fp, offset, whence)) 
    {
      if (nothrow) return -1;
#ifndef UNDER_CE
        G_THROW(strerror(errno));                     //  (No error in the DjVuMessageFile)
#else
        G_THROW("ByteStream.seek_error");             //  seek, seek error.
#endif
    }
  tell();
  return 0;
}




///////// ByteStream::Memory

ByteStream::Memory::Memory()
  : where(0), bsize(0), nblocks(0), gblocks(blocks,0)
{
}

GString
ByteStream::Memory::init(void const * const buffer, const size_t sz)
{
  GString retval;
  G_TRY
  {
    writall(buffer, sz);
    where = 0;
  }
  G_CATCH(ex) // The only error that should be thrown is out of memory...
  {
    retval=ex.get_cause();
  }
  G_ENDCATCH;
  return retval;
}

void 
ByteStream::Memory::empty()
{
  for (int b=0; b<nblocks; b++)
  {
    delete [] blocks[b];
    blocks[b]=0;
  }
  bsize = 0;
  where = 0;
  nblocks = 0;
}

ByteStream::Memory::~Memory()
{
  empty();
}

size_t 
ByteStream::Memory::write(const void *buffer, size_t sz)
{
  int nsz = (int)sz;
  if (nsz <= 0)
    return 0;
  // check memory
  if ( (where+nsz) > ((bsize+0xfff)&~0xfff) )
    {
      // reallocate pointer array
      if ( (where+nsz) > (nblocks<<12) )
        {
          const int old_nblocks=nblocks;
          nblocks = (((where+nsz)+0xffff)&~0xffff) >> 12;
          gblocks.resize(nblocks);
          char const ** eblocks=(char const **)(blocks+old_nblocks);
          for(char const * const * const new_eblocks=blocks+nblocks;
            eblocks <new_eblocks; eblocks++) 
          {
            *eblocks = 0;
          }
        }
      // allocate blocks
      for (int b=(where>>12); (b<<12)<(where+nsz); b++)
      {
        if (! blocks[b])
          blocks[b] = new char[0x1000];
      }
    }
  // write data to buffer
  while (nsz > 0)
    {
      int n = (where|0xfff) + 1 - where;
      n = ((nsz < n) ? nsz : n);
      memcpy( (void*)&blocks[where>>12][where&0xfff], buffer, n);
      buffer = (void*) ((char*)buffer + n);
      where += n;
      nsz -= n;
    }
  // adjust size
  if (where > bsize)
    bsize = where;
  return sz;
}

size_t 
ByteStream::Memory::readat(void *buffer, size_t sz, int pos)
{
  if ((int) sz > bsize - pos)
    sz = bsize - pos;
  int nsz = (int)sz;
  if (nsz <= 0)
    return 0;
  // read data from buffer
  while (nsz > 0)
    {
      int n = (pos|0xfff) + 1 - pos;
      n = ((nsz < n) ? nsz : n);
      memcpy(buffer, (void*)&blocks[pos>>12][pos&0xfff], n);
      buffer = (void*) ((char*)buffer + n);
      pos += n;
      nsz -= n;
    }
  return sz;
}

size_t 
ByteStream::Memory::read(void *buffer, size_t sz)
{
  sz = readat(buffer,sz,where);
  where += sz;
  return sz;
}

long 
ByteStream::Memory::tell(void) const
{
  return where;
}

int
ByteStream::Memory::seek(long offset, int whence, bool nothrow)
{
  int nwhere = 0;
  switch (whence)
    {
    case SEEK_SET: nwhere = 0; break;
    case SEEK_CUR: nwhere = where; break;
    case SEEK_END: nwhere = bsize; break;
    default: G_THROW("bad_arg\tByteStream::Memory::seek()");      // Illegal argument in ByteStream::Memory::seek()
    }
  nwhere += offset;
  if (nwhere<0)
    G_THROW("ByteStream.seek_error2");                          //  Attempt to seek before the beginning of the file
  where = nwhere;
  return 0;
}



/** This function has been moved into Arrays.cpp
    In order to avoid dependencies from ByteStream.o
    to Arrays.o */
#ifdef DO_NOT_MOVE_GET_DATA_TO_ARRAYS_CPP
TArray<char>
ByteStream::get_data(void)
{
   TArray<char> data(0, size()-1);
   readat((char*)data, size(), 0);
   return data;
}
#endif


///////// ByteStream::Static

ByteStream::Static::Static(const void * const buffer, const size_t sz)
  : data((const char *)buffer), bsize(sz), where(0)
{
}

size_t 
ByteStream::Static::read(void *buffer, size_t sz)
{
  int nsz = (int)sz;
  if (nsz > bsize - where)
    nsz = bsize - where;
  if (nsz <= 0)
    return 0;
  memcpy(buffer, data+where, nsz);
  where += nsz;
  return nsz;
}

int
ByteStream::Static::seek(long offset, int whence, bool nothrow)
{
  int nwhere = 0;
  switch (whence)
    {
    case SEEK_SET: nwhere = 0; break;
    case SEEK_CUR: nwhere = where; break;
    case SEEK_END: nwhere = bsize; break;
    default: G_THROW("bad_arg\tByteStream::Static::seek()");      //  Illegal argument to ByteStream::Static::seek()
    }
  nwhere += offset;
  if (nwhere<0)
    G_THROW("ByteStream.seek_error2");                          //  Attempt to seek before the beginning of the file
  where = nwhere;
  return 0;
}

long 
ByteStream::Static::tell(void) const
{
  return where;
}

GP<ByteStream>
ByteStream::create(void)
{
  return new Memory();
}

GP<ByteStream>
ByteStream::create(void const * const buffer, const size_t size)
{
  Memory *mbs=new Memory();
  GP<ByteStream> retval=mbs;
  mbs->init(buffer,size);
  return retval;
}

GP<ByteStream>
ByteStream::create(const GURL &url,char const * const mode)
{
  GP<ByteStream> retval;
#ifdef UNIX
  if (!mode || (GString("rb") == mode))
  {
    const int fd=urlopen(url,O_RDONLY,0777);
    if(fd>=0)
    {
      MemoryMapByteStream *rb=new MemoryMapByteStream();
      retval=rb;
      GString errmessage=rb->init(fd,true);
      if(errmessage.length())
      {
        retval=0;
      }
    }
  }
  if(!retval)
#endif
  {
    Stdio *sbs=new Stdio();
    retval=sbs;
    GString errmessage=sbs->init(url,mode?mode:"rb");
    if(errmessage.length())
    {
      G_THROW(errmessage);
    }
  }
  return retval;
}

GP<ByteStream>
ByteStream::create(char const * const mode)
{
  GP<ByteStream> retval;
  Stdio *sbs=new Stdio();
  retval=sbs;
  GString errmessage=sbs->init(mode?mode:"rb");
  if(errmessage.length())
  {
    G_THROW(errmessage);
  }
  return retval;
}

GP<ByteStream>
ByteStream::create(const int fd,char const * const mode,const bool closeme)
{
  GP<ByteStream> retval;
  const char *default_mode="rb";
#ifdef UNIX

//  if((!mode&&(fd!=1)&&(fd!=2))||(mode&&!strcmp(mode,"rb")))
  if ((!mode&&(fd!=1)&&(fd!=2)) || (mode&&(GString("rb") == mode)))
  {
    MemoryMapByteStream *rb=new MemoryMapByteStream();
    retval=rb;
    GString errmessage=rb->init(fd,closeme);
    if(errmessage.length())
    {
      retval=0;
    }
  }
  default_mode="rb";
  if(!retval)
#endif
  {
    int fd2=fd;
    FILE *f=0;
    switch(fd)
    {
      case 0:
        if(!closeme && !mode)
        {
          f=stdin;
          fd2=(-1);
          break;
        }
      case 1:
        if(!closeme && (!mode || GString(mode) == "a"))
        {
          f=stdout;
          default_mode="wb";
          fd2=(-1);
          break;
        }
      case 2:
        if(!closeme && (!mode || GString(mode) == "a"))
        {
          default_mode="a";
          f=stderr;
          fd2=(-1);
          break;
        }
      default:
#ifndef UNDER_CE
        if(!closeme)
        {
          fd2=dup(fd);
        } 
        f=fdopen(fd2,(char*)(mode?mode:default_mode));
#else
        if(!closeme)
        {
          fd2=(-1);
        }
#endif
        break;
    }
    if(!f)
    {
#ifndef UNDER_CE
      if(fd2>= 0)
        close(fd2);
#endif
      G_THROW("ByteStream.open_fail2");
    }
    Stdio *sbs=new Stdio();
    retval=sbs;
    GString errmessage=sbs->init(f,mode?mode:default_mode,(fd2>=0));
    if(errmessage.length())
    {
      G_THROW(errmessage);
    }
  }
  return retval;
}

GP<ByteStream>
ByteStream::create(FILE * const f,char const * const mode,const bool closeme)
{
  GP<ByteStream> retval;
#ifdef UNIX
  if (!mode || (GString("rb") == mode))
  {
    MemoryMapByteStream *rb=new MemoryMapByteStream();
    retval=rb;
    GString errmessage=rb->init(fileno(f),false);
    if(errmessage.length())
    {
      retval=0;
    }else
    {
      fclose(f);
    }
  }
  if(!retval)
#endif
  {
    Stdio *sbs=new Stdio();
    retval=sbs;
    GString errmessage=sbs->init(f,mode?mode:"rb",closeme);
    if(errmessage.length())
    {
      G_THROW(errmessage);
    }
  }
  return retval;
}

GP<ByteStream>
ByteStream::create_static(const void * const buffer, size_t sz)
{
  return new Static(buffer, sz);
}

#ifdef UNIX
MemoryMapByteStream::MemoryMapByteStream(void)
: ByteStream::Static(0,0)
{}

GString
MemoryMapByteStream::init(FILE *const f,const bool closeme)
{
  GString retval;
  retval=init(fileno(f),false);
  if(closeme)
  {
    fclose(f);
  }
  return retval;
}

GString
MemoryMapByteStream::init(const int fd,const bool closeme)
{
  GString retval;
#if defined(PROT_READ) && defined(MAP_SHARED)
  struct stat statbuf;
  if(!fstat(fd,&statbuf))
  {
    if(statbuf.st_size)
    {
      bsize=statbuf.st_size;
      data=(char *)mmap(0,statbuf.st_size,PROT_READ,MAP_SHARED,fd,0);
    }
  }else
  {
    if(closeme)
    {
      close(fd);
    }
    retval="ByteStream.open_fail2";
  }
#else
  retval="ByteStream.open_fail2";
#endif
  if(closeme)
  {
    close(fd);
  }
  return retval;
}

MemoryMapByteStream::~MemoryMapByteStream()
{
  if(data)
  {
    munmap(const_cast<char *>(data),bsize);
  }
}

#endif

ByteStream::Wrapper::~Wrapper() {}

void
DjVuPrintError(const char *fmt, ... )
{
  static GP<ByteStream> errout=ByteStream::create(2,0,false);
  if(errout)
  {
    errout->cp=ByteStream::NATIVE;
    va_list args;
    va_start(args, fmt); 
    GString message;
    errout->writestring(message.format(fmt, args));
  }
}

void
DjVuPrintMessage(const char *fmt, ... )
{
  static GP<ByteStream> strout=ByteStream::create(1,0,false);
  if(strout)
  {
    strout->cp=ByteStream::NATIVE;
    va_list args;
    va_start(args, fmt);
    GString message;
    strout->writestring(message.format(fmt,args));
  }
}

