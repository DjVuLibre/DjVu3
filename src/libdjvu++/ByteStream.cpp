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
//C- $Id: ByteStream.cpp,v 1.9 1999-03-17 19:24:56 leonb Exp $

// File "$Id: ByteStream.cpp,v 1.9 1999-03-17 19:24:56 leonb Exp $"
// - Author: Leon Bottou, 04/1997

#ifdef __GNUC__
#pragma implementation
#endif

#include "ByteStream.h"
#include <errno.h>

//// CLASS BYTESTREAM


ByteStream::~ByteStream()
{
}

void
ByteStream::flush()
{
}

int 
ByteStream::is_seekable() const
{
  return 0;
}

void
ByteStream::seek(long offset, int whence)
{
  int nwhere = 0;
  int ncurrent = tell();
  switch (whence)
    {
    case SEEK_SET: nwhere=0 ; break;
    case SEEK_CUR: nwhere=ncurrent; break;
    case SEEK_END: THROW("Seek mode SEEK_END is not supported by this ByteStream");
    default: THROW("Illegal argument in ByteStream::seek");
    }
  nwhere += offset;
  if (nwhere < ncurrent)
    THROW("Seeking backwards is not supported by this ByteStream");
  while (nwhere>ncurrent)
    {
      char buffer[512];
      int bytes = sizeof(buffer);
      if (ncurrent + bytes > nwhere)
        bytes = nwhere - ncurrent;
      bytes = read(buffer, bytes);
      ncurrent += bytes;
      if (ncurrent != tell())
        THROW("Seeking is not supported by this ByteStream "
              "(tell returns strange results)");
      if (bytes==0)
        THROW("EOF");
    }
  return;
}

size_t 
ByteStream::readall(void *buffer, size_t size)
{
  size_t total = 0;
  while (size > 0)
    {
      size_t nitems = read(buffer, size);
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
        THROW("Unknown error in ByteStream::write");
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
    THROW(strerror(errno));
}

void 
ByteStream::write16(unsigned int card)
{
  unsigned char c[2];
  c[0] = (card>>8) & 0xff;
  c[1] = (card) & 0xff;
  if (writall((void*)c, sizeof(c)) != sizeof(c))
    THROW(strerror(errno));
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
    THROW(strerror(errno));
}

unsigned int 
ByteStream::read8 ()
{
  unsigned char c[1];
  if (readall((void*)c, sizeof(c)) != sizeof(c))
    THROW("EOF");
  return c[0];
}

unsigned int 
ByteStream::read16()
{
  unsigned char c[2];
  if (readall((void*)c, sizeof(c)) != sizeof(c))
    THROW("EOF");
  return (c[0]<<8)+c[1];
}

unsigned int 
ByteStream::read32()
{
  unsigned char c[4];
  if (readall((void*)c, sizeof(c)) != sizeof(c))
    THROW("EOF");
  return (((((c[0]<<8)+c[1])<<8)+c[2])<<8)+c[3];
}



//// CLASS STDIOBYTESTREAM

StdioByteStream::StdioByteStream(FILE *f, const char *mode)
{
  fp = f;
  pos = 0;
  must_close = 0;
  can_read = can_write = 0;
  can_seek = 1;
  for (const char *s=mode; s && *s; s++)
    switch(*s) 
      {
      case 'r': can_read=1; break;
      case 'w': can_write=1; break;
      case 'a': can_write=1; can_seek=0; break;
      case '+': can_read=can_write=1; break;
      case 'b': break;
      default: THROW("Illegal mode in StdioByteStream");
      }
}

StdioByteStream::StdioByteStream(const char *filename, const char *mode)
{
  pos = 0;
  must_close = 1;
  can_read = can_write = 0;
  can_seek = 1;
  FILE *dash = 0;
  for (const char *s=mode; s && *s; s++)
    switch(*s) 
      {
      case 'r': can_read=1; dash=stdin; break;
      case 'w': can_write=1; dash=stdout; break;
      case 'a': can_write=1; can_seek=0; dash=stdout; break;
      case '+': can_read=can_write=1; dash=0; break;
      case 'b': break;
      default: THROW("Illegal mode in StdioByteStream");
      }
  if (strcmp(filename,"-") != 0) 
    {
      fp = fopen(filename, mode);
      if (!fp)
      {
	 char buffer[4096];
	 sprintf(buffer, "Cannot open '%s': %s",
		 filename, strerror(errno));
	 THROW(buffer);
      };
    } 
  else 
    {
      must_close = 0;
      can_seek = 0;
      fp = dash;
      if (!fp)
        THROW("Illegal mode for stdin/stdout file descriptor");
    }
}

StdioByteStream::~StdioByteStream()
{
  if (must_close)
    if (fclose(fp) < 0)
      THROW(strerror(errno));
}

size_t 
StdioByteStream::read(void *buffer, size_t size)
{
  if (!can_read)
    THROW("StdioByteStream not opened for writing");
  size_t nitems = fread(buffer, 1, size, fp);
  if (nitems<=0 && ferror(fp))
    THROW(strerror(errno));
  pos += nitems;
  return nitems;
}

size_t 
StdioByteStream::write(const void *buffer, size_t size)
{
  if (!can_write)
    THROW("StdioByteStream not opened for writing");
  size_t nitems = fwrite(buffer, 1, size, fp);
  if (nitems<=0 && ferror(fp))
    THROW(strerror(errno));
  pos += nitems;
  return nitems;
}

void
StdioByteStream::flush()
{
  if (fflush(fp) < 0)
    THROW(strerror(errno));
}

long 
StdioByteStream::tell()
{
  long x = ftell(fp);
  if (x >= 0)
    pos = x;
  return pos;
}

int 
StdioByteStream::is_seekable() const
{
  return can_seek;
}

void
StdioByteStream::seek(long offset, int whence)
{
  if (!can_seek)
    ByteStream::seek(offset, whence);
  else if (fseek(fp, offset, whence))
    THROW(strerror(errno));
}




///////// MEMORYBYTESTREAM

MemoryByteStream::MemoryByteStream()
  : where(0), bsize(0), nblocks(0), blocks(0)
{
}

MemoryByteStream::MemoryByteStream(const void *buffer, size_t sz)
  : where(0), bsize(0), nblocks(0), blocks(0)
{
  MemoryByteStream::write(buffer, sz);
  where = 0;
}

MemoryByteStream::MemoryByteStream(const char *buffer)
  : where(0), bsize(0), nblocks(0), blocks(0)
{
  MemoryByteStream::write(buffer, strlen(buffer));
  where = 0;
}

MemoryByteStream::~MemoryByteStream()
{
  for (int b=0; b<nblocks; b++)
    delete [] blocks[b];
  bsize = 0;
  where = 0;
  nblocks = 0;
  delete [] blocks;
  blocks = 0;
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
      int b;
      // reallocate pointer array
      if ( (where+nsz) > (nblocks<<12) )
        {
          int new_nblocks = (((where+nsz)+0xffff)&~0xffff) >> 12;
          char **new_blocks = new char* [new_nblocks];
          for (b=0; b<nblocks; b++) 
            new_blocks[b] = blocks[b];
          for (; b<new_nblocks; b++) 
            new_blocks[b] = 0;
           char **old_blocks = blocks;
          blocks = new_blocks;
          nblocks = new_nblocks;
          delete [] old_blocks;
        }
      // allocate blocks
      for (b=(where>>12); (b<<12)<(where+nsz); b++)
        if (! blocks[b])
          blocks[b] = new char[0x1000];
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
MemoryByteStream::tell()
{
  return where;
}

int 
MemoryByteStream::is_seekable() const
{
  return 1;
}

void
MemoryByteStream::seek(long offset, int whence)
{
  int nwhere = 0;
  switch (whence)
    {
    case SEEK_SET: nwhere = 0; break;
    case SEEK_CUR: nwhere = where; break;
    case SEEK_END: nwhere = bsize; break;
    default: THROW("Illegal argument in MemoryByteStream::seek()");
    }
  nwhere += offset;
  if (nwhere<0 || nwhere>bsize)
    THROW("Seek out of bound in MemoryByteStream");
  where = nwhere;
}


///////// STATICBYTESTREAM


StaticByteStream::StaticByteStream(const char *buffer, size_t sz)
  : data(buffer), bsize(sz), where(0)
{
}

StaticByteStream::StaticByteStream(const char *buffer)
  : data(buffer), bsize(0), where(0)
{
  bsize = strlen(data);
}

size_t 
StaticByteStream::read(void *buffer, size_t sz)
{
  if ((int) sz > bsize - where)
    sz = bsize - where;
  if ((int) sz <= 0)
    return 0;
  memcpy(buffer, data+where, sz);
  where += sz;
  return sz;
}

size_t 
StaticByteStream::write(const void *buffer, size_t sz)
{
  THROW("Attempt to write into a StaticByteStream");
  return 0;
}

int 
StaticByteStream::is_seekable(void) const
{
  return 1;
}

void 
StaticByteStream::seek(long offset, int whence)
{
  int nwhere = 0;
  switch (whence)
    {
    case SEEK_SET: nwhere = 0; break;
    case SEEK_CUR: nwhere = where; break;
    case SEEK_END: nwhere = bsize; break;
    default: THROW("Illegal argument in StaticByteStream::seek()");
    }
  nwhere += offset;
  if (nwhere<0 || nwhere>bsize)
    THROW("Seek out of bound in StaticByteStream");
  where = nwhere;
}

long 
StaticByteStream::tell()
{
  return where;
}

