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
// $Id: ByteStream.cpp,v 1.43 2001-02-07 23:26:34 bcr Exp $
// $Name:  $

// - Author: Leon Bottou, 04/1997

#ifdef __GNUC__
#pragma implementation
#endif
#include "ByteStream.h"
#include "GOS.h"

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#endif
#ifndef UNDER_CE
#include <errno.h>
#endif

//// CLASS BYTESTREAM


ByteStream::~ByteStream()
{
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
      G_THROW("ByteStream.bad_arg");       //  Illegal argument in ByteStream::seek
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
        G_THROW("ByteStream.read_error");       //  ByteStream::readall, read error.
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
ByteStream::writall(const void *buffer, size_t size)
{
  size_t total = 0;
  while (size > 0)
    {
      size_t nitems = write(buffer, size);
      if (nitems == 0)
        G_THROW("ByteStream.write_error");      //  Unknown error in ByteStream::write
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
        G_THROW("ByteStream.write\t8");               //  ByteStream::write8, write error.
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
  G_THROW("ByteStream.write\t16");                    //  ByteStream::write16, write error.
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
        G_THROW("ByteStream.write\t24");              //  ByteStream::write24, write error.
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
        G_THROW("ByteStream.write\t32");              //  ByteStream::write32, write error.
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



//// CLASS STDIOBYTESTREAM

StdioByteStream::StdioByteStream(FILE *f, const char *mode, bool closeme)
{
  fp = f;
  pos = 0;
  must_close = closeme?1:0;
  can_read = can_write = 0;
  for (const char *s=mode; s && *s; s++)
    switch(*s) 
      {
      case 'r': can_read=1;  break;
      case 'w': can_write=1; break;
      case 'a': can_write=1; break;
      case '+': can_read=can_write=1; break;
      case 'b': break;
      default: G_THROW("ByteStream.bad_mode");        //  Illegal mode in StdioByteStream
      }
  tell();
}

StdioByteStream::StdioByteStream(const char *filename, const char *mode)
{
  pos = 0;
  must_close = 1;
  can_read = can_write = 0;
  FILE *dash = 0;
  for (const char *s=mode; s && *s; s++)
    switch(*s) 
      {
      case 'r': can_read=1;  dash=stdin; break;
      case 'w': can_write=1; dash=stdout; break;
      case 'a': can_write=1; dash=stdout; break;
      case '+': can_read=can_write=1; dash=0; break;
      case 'b': break;
      default: G_THROW("ByteStream.bad_mode");        //  Illegal mode in StdioByteStream
      }
  if (strcmp(filename,"-") != 0) 
    {
      fp = fopen(GOS::expand_name(filename), mode);
      if (!fp)
      {
#ifndef UNDER_CE
	 char buffer[4096];
	 sprintf(buffer, "ByteStream.open_fail\t%s\t%s",
		               filename, strerror(errno));
	 G_THROW(buffer);                                   //  Failed to open '%s': %s
#else
   G_THROW("ByteStream.open_fail2");                  //  StdioByteStream::StdioByteStream, failed to open file.
#endif


      }
    } 
  else 
    {
      must_close = 0;
      fp = dash;
      if (!fp)
        G_THROW("ByteStream.bad_mode2");              //  Illegal mode for stdin/stdout file descriptor
    }
  tell();
}

StdioByteStream::~StdioByteStream()
{
  if (must_close && fp)
     fclose(fp);
}

size_t 
StdioByteStream::read(void *buffer, size_t size)
{
  if (!can_read)
    G_THROW("ByteStream.no_read");                    //  StdioByteStream not opened for reading
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
        G_THROW("ByteStream.read_error2");            //  StdioByteStream::read, read error.
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
StdioByteStream::write(const void *buffer, size_t size)
{
  if (!can_write)
    G_THROW("ByteStream.no_write");                   //  StdioByteStream not opened for writing
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
        G_THROW("ByteStream.write_error2");           //  StdioByteStream::write, write error.
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
StdioByteStream::flush()
{
  if (fflush(fp) < 0)
#ifndef UNDER_CE
        G_THROW(strerror(errno));                     //  (No error in the DjVuMessageFile)
#else
        G_THROW("ByteStream.flush_error");            //  StdioByteStream::flush, flush error.
#endif
}

long 
StdioByteStream::tell() const
{
  long x = ftell(fp);
  if (x >= 0)
  {
    StdioByteStream *sbs=const_cast<StdioByteStream *>(this);
    (sbs->pos) = x;
  }else
  {
    x=pos;
  }
  return x;
}

int
StdioByteStream::seek(long offset, int whence, bool nothrow)
{
  if (whence==SEEK_SET && offset>=0 && offset==ftell(fp))
    return 0;
  if (fseek(fp, offset, whence)) 
    {
      if (nothrow) return -1;
#ifndef UNDER_CE
        G_THROW(strerror(errno));                     //  (No error in the DjVuMessageFile)
#else
        G_THROW("ByteStream.seek_error");             //  StdioByteStream::seek, seek error.
#endif
    }
  tell();
  return 0;
}




///////// MEMORYBYTESTREAM

MemoryByteStream::MemoryByteStream()
  : where(0), bsize(0), nblocks(0), gblocks(blocks,0)
{
}

MemoryByteStream::MemoryByteStream(const void *buffer, size_t sz)
  : where(0), bsize(0), nblocks(0), gblocks(blocks,0)
{
  MemoryByteStream::write(buffer, sz);
  where = 0;
}

MemoryByteStream::MemoryByteStream(const char *buffer)
  : where(0), bsize(0), nblocks(0), gblocks(blocks,0)
{
  MemoryByteStream::write(buffer, strlen(buffer));
  where = 0;
}

void 
MemoryByteStream::empty()
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

MemoryByteStream::~MemoryByteStream()
{
  empty();
}

size_t 
MemoryByteStream::write(const void *buffer, size_t sz)
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
MemoryByteStream::readat(void *buffer, size_t sz, int pos)
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
MemoryByteStream::read(void *buffer, size_t sz)
{
  sz = readat(buffer,sz,where);
  where += sz;
  return sz;
}

long 
MemoryByteStream::tell() const
{
  return where;
}

int
MemoryByteStream::seek(long offset, int whence, bool nothrow)
{
  int nwhere = 0;
  switch (whence)
    {
    case SEEK_SET: nwhere = 0; break;
    case SEEK_CUR: nwhere = where; break;
    case SEEK_END: nwhere = bsize; break;
    default: G_THROW("bad_arg\tMemoryByteStream::seek()");      // Illegal argument in MemoryByteStream::seek()
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


///////// STATICBYTESTREAM

StaticByteStream::StaticByteStream(const char *buffer, size_t sz)
  : data(buffer), bsize(sz), where(0), buf(0)
{
}

StaticByteStream::StaticByteStream(const char *buffer)
  : data(buffer), bsize(0), where(0), buf(0)
{
  bsize = strlen(data);
}

#if defined(UNIX) && defined(PROT_READ) && defined(MAP_SHARED)
StaticByteStream::StaticByteStream(const int fd,const bool closeme)
  : data(0), bsize(0), where(0), buf(0)
{
  if(fd>=0)
  {
    init(fd);
    if(closeme)
    {
      close(fd);
    }
  }else
  {
    G_THROW("ByteStream.open_fail2");
  } 
}

StaticByteStream::StaticByteStream(FILE *f,const bool closeme)
  : data(0), bsize(0), where(0), buf(0)
{
  if(f)
  {
    init(fileno(f));
    if(closeme)
    {
      fclose(f);
    }
  }
}

void
StaticByteStream::init(const int fd)
{
  struct stat statbuf;
  if(!fstat(fd,&statbuf))
  {
    if(statbuf.st_size)
    {
      bsize=statbuf.st_size;
      data=(char *)(buf=mmap(0,statbuf.st_size,PROT_READ,MAP_SHARED,fd,0));
    }
  }
}

StaticByteStream::~StaticByteStream()
{
  if(buf)
  {
    munmap(buf,bsize);
  }
}
#endif

size_t 
StaticByteStream::read(void *buffer, size_t sz)
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

size_t 
StaticByteStream::write(const void *buffer, size_t sz)
{
  G_THROW("ByteStream.write_error3");                           //  Attempt to write into a StaticByteStream
  return 0;
}

int
StaticByteStream::seek(long offset, int whence, bool nothrow)
{
  int nwhere = 0;
  switch (whence)
    {
    case SEEK_SET: nwhere = 0; break;
    case SEEK_CUR: nwhere = where; break;
    case SEEK_END: nwhere = bsize; break;
    default: G_THROW("bad_arg\tStaticByteStream::seek()");      //  Illegal argument to StaticByteStream::seek()
    }
  nwhere += offset;
  if (nwhere<0)
    G_THROW("ByteStream.seek_error2");                          //  Attempt to seek before the beginning of the file
  where = nwhere;
  return 0;
}

long 
StaticByteStream::tell() const
{
  return where;
}

