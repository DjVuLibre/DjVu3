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
//C- $Id: IFFByteStream.cpp,v 1.10 1999-11-22 03:35:17 bcr Exp $

// File "$Id: IFFByteStream.cpp,v 1.10 1999-11-22 03:35:17 bcr Exp $"
// -- Implementation of IFFByteStream
// - Author: Leon Bottou, 06/1998

#ifdef __GNUC__
#pragma implementation
#endif

#include <assert.h>
#include "IFFByteStream.h"



// Constructor
IFFByteStream::IFFByteStream(ByteStream &xbs)
  : ctx(0), bs(&xbs), dir(0)
{
  offset = seekto = bs->tell();
}


// Destructor
IFFByteStream::~IFFByteStream()
{
  while (ctx)
    close_chunk();
}



// IFFByteStream::ready
// -- indicates if bytestream is ready for reading
//    returns number of bytes available

int 
IFFByteStream::ready()
{
  if (ctx && dir < 0)
    return ctx->offEnd - offset;
  else if (ctx)
    return 1;
  else
    return 0;
}


// IFFByteStream::composite
// -- indicates if bytestream is ready for putting or getting chunks

int 
IFFByteStream::composite()
{
  if (ctx && !ctx->bComposite)
    return 0;
  else
    return 1;
}




// IFFByteStream::check_id
// -- checks if an id is legal

int 
IFFByteStream::check_id(const char *id)
{
  int i;
  // check absence of null bytes
  for (i=0; i<4; i++)
    if (id[i]<0x20 || id[i]>0x7e)
      return -1;
  // check composite chunks
  static char *szComposite[] = { "FORM", "LIST", "PROP", "CAT ", 0 };
  for (i=0; szComposite[i]; i++) 
    if (!memcmp(id, szComposite[i], 4))
      return 1;
  // check reserved chunks
  static char *szReserved[] = { "FOR", "LIS", "CAT", 0 };
  for (i=0; szReserved[i]; i++) 
    if (!memcmp(id, szReserved[i], 3) && id[3]>='1' && id[3]<='9')
      return -1;
  // regular chunk
  return 0;
}



// IFFByteStream::get_chunk
// -- get next chunk header

int  
IFFByteStream::get_chunk(GString &chkid, int *rawoffsetptr, int *rawsizeptr)
{
  int bytes;
  char buffer[8];
  
  // Check that we are allowed to read a chunk
  if (dir > 0)
    THROW("Attempt to write and read the same IFFByteStream");
  if (ctx && !ctx->bComposite)
    THROW("IFFByteStream not ready for reading chunk");
  dir = -1;

  // Seek to end of previous chunk if necessary
  if (seekto > offset)
    {
      bs->seek(seekto);
      offset = seekto;
    }

  // Skip padding byte
  if (ctx && offset == ctx->offEnd)
    return 0;
  if (offset & 1)
    {
      bytes = bs->read( (void*)buffer, 1);
      if (bytes==0 && !ctx)
        return 0;
      offset += bytes;
    }
  
  // Record raw offset
  int rawoffset = offset;
  
  // Read chunk id (skipping sequences inserted here to defeat 
  // Internet Explorer assumptions about IFF)
  do {
    if (ctx && offset == ctx->offEnd)
      return 0;
    if (ctx && offset+4 > ctx->offEnd)
      THROW("Corrupted IFF file (EndOfChunk while reading chunk ID)");
    bytes = bs->readall( (void*)&buffer[0], 4);
    offset = seekto = offset + bytes;
    if (bytes==0 && !ctx)
      return 0;
    if (bytes != 4)
      THROW("EOF");
  } while (buffer[0]==0x41 && buffer[1]==0x54 && 
           buffer[2]==0x26 && buffer[3]==0x54  );
  
  // Read chunk size
  if (ctx && offset+4 > ctx->offEnd)
    THROW("Corrupted IFF file (EndOfChunk while reading chunk size)");
  bytes = bs->readall( (void*)&buffer[4], 4);
  offset = seekto = offset + bytes;
  if (bytes != 4)
    THROW("EOF");
  long size = ((unsigned char)buffer[4]<<24) |
              ((unsigned char)buffer[5]<<16) |
              ((unsigned char)buffer[6]<<8)  |
              ((unsigned char)buffer[7]);
  if (ctx && offset+size > ctx->offEnd)
    THROW("Corrupted IFF file (Mangled chunk boundaries)");
  
  // Check if composite 
  int composite = check_id(buffer);
  if (composite < 0)
    THROW("Corrupted IFF file (Illegal chunk id)");
  
  // Read secondary id of composite chunk
  if (composite)
  {
    if (ctx && ctx->offEnd<offset+4)
      THROW("Corrupted IFF file (EndOfChunk while reading composite chunk header)");
    bytes = bs->readall( (void*)&buffer[4], 4);
    offset += bytes;
    if (bytes != 4)
      THROW("EOF");
    if (check_id(&buffer[4]))
      THROW("Corrupted IFF file (Illegal secondary chunk id)");
  }

  // Create context record
  IFFContext *nctx = new IFFContext;
  nctx->next = ctx;
  nctx->offStart = seekto;
  nctx->offEnd = seekto + size;
  if (composite)
  {
    memcpy( (void*)(nctx->idOne), (void*)&buffer[0], 4);
    memcpy( (void*)(nctx->idTwo), (void*)&buffer[4], 4);
    nctx->bComposite = 1;
  }
  else
  {
    memcpy( (void*)(nctx->idOne), (void*)&buffer[0], 4);
    memset( (void*)(nctx->idTwo), 0, 4);
    nctx->bComposite = 0;
  }
  
  // Install context record
  ctx = nctx;
  chkid = GString(ctx->idOne, 4);
  if (composite)
    chkid = chkid + ":" + GString(ctx->idTwo, 4);

  // Return
  if (rawoffsetptr)
    *rawoffsetptr = rawoffset;
  if (rawsizeptr)
    *rawsizeptr = ( ctx->offEnd - rawoffset + 1) & ~0x1;
  return size;
}



// IFFByteStream::put_chunk
// -- write new chunk header

void  
IFFByteStream::put_chunk(const char *chkid, int insert_att)
{
  int bytes;
  char buffer[8];

  // Check that we are allowed to write a chunk
  if (dir < 0)
    THROW("Attempt to read and write the same IFFByteStream");
  if (ctx && !ctx->bComposite)
    THROW("IFFByteStream not ready for writing chunks");
  dir = +1;

  // Check primary id
  int composite = check_id(chkid);
  if ((composite<0) || (composite==0 && chkid[4])
      || (composite && (chkid[4]!=':' || check_id(&chkid[5]) || chkid[9])) )
    THROW("Illegal chunk id (IFFByteStream::put_chunk)");

  // Write padding byte
  assert(seekto <= offset);
  memset((void*)buffer, 0, 8);
  if (offset & 1)
    offset += bs->write((void*)&buffer[4], 1);

  // Insert "AT&T" to fool MSIE
  if (insert_att)
  {
    // Don't change the way you fool Internet Explorer! 
    // I rely on these "AT&T" letters in some places
    // (like DjVmFile.cpp and djvm.cpp) -- eaf
    strcpy(buffer,"AT&T");
    offset += bs->writall((void*)&buffer[0], 4);
  }

  // Write chunk header
  memcpy((void*)&buffer[0], (void*)&chkid[0], 4);
  bytes = bs->writall((void*)&buffer[0], 8);
  offset = seekto = offset + bytes;
  if (composite)
  {
    memcpy((void*)&buffer[4], (void*)&chkid[5], 4);
    bytes = bs->writall((void*)&buffer[4], 4);
    offset = offset + bytes;    
  }

  // Create new context record
  IFFContext *nctx = new IFFContext;
  nctx->next = ctx;
  nctx->offStart = seekto;
  nctx->offEnd = 0;
  if (composite)
  {
    memcpy( (void*)(nctx->idOne), (void*)&buffer[0], 4);
    memcpy( (void*)(nctx->idTwo), (void*)&buffer[4], 4);
    nctx->bComposite = 1;
  }
  else
  {
    memcpy( (void*)(nctx->idOne), (void*)&buffer[0], 4);
    memset( (void*)(nctx->idTwo), 0, 4);
    nctx->bComposite = 0;
  }
  // Install context record and leave
  ctx = nctx;
}



void 
IFFByteStream::close_chunk()
{
  // Check that this is ok
  if (!ctx)
    THROW("Cannot close chunk when no chunk is open");
  // Patch size field in new chunk
  if (dir > 0)
    {
      ctx->offEnd = offset;
      long size = ctx->offEnd - ctx->offStart;
      char buffer[4];
      buffer[0] = (unsigned char)(size>>24);
      buffer[1] = (unsigned char)(size>>16);
      buffer[2] = (unsigned char)(size>>8);
      buffer[3] = (unsigned char)(size);
      bs->seek(ctx->offStart - 4);
      bs->writall((void*)buffer, 4);
      bs->seek(offset);
    }
  // Arrange for reader to seek at next chunk
  seekto = ctx->offEnd;
  // Remove ctx record
  IFFContext *octx = ctx;
  ctx = octx->next;
  assert(ctx==0 || ctx->bComposite);
  delete octx;
}

// This is the same as above, but adds a seek to the close
// Otherwise an EOF in this chunk won't be reported until we
// try to open the next chunk, which makes error recovery
// very difficult.
void  
IFFByteStream::seek_close_chunk(void)
{
  close_chunk();
  if ((dir <= 0)&&((!ctx)||(ctx->bComposite))&&(seekto > offset))
  {
    bs->seek(seekto);
    offset = seekto;
  }
}



// IFFByteStream::short_id
// Returns the id of the current chunk

void 
IFFByteStream::short_id(GString &chkid)
{
  if (!ctx)
    THROW("No chunk id available yet");
  if (ctx->bComposite)
    chkid = GString(ctx->idOne, 4) + ":" + GString(ctx->idTwo, 4);
  else
    chkid = GString(ctx->idOne, 4);
}


// IFFByteStream::full_id
// Returns the full chunk id of the current chunk

void 
IFFByteStream::full_id(GString &chkid)
{
  short_id(chkid);
  if (ctx->bComposite)
    return;
  // Search parent FORM or PROP chunk.
  for (IFFContext *ct = ctx->next; ct; ct=ct->next)
    if (memcmp(ct->idOne, "FOR", 3)==0 || 
        memcmp(ct->idOne, "PRO", 3)==0  )
      {
        chkid = GString(ct->idTwo, 4) + "." + chkid;
        break;
      }
}



// IFFByteStream::read
// -- read bytes from IFF file chunk

size_t 
IFFByteStream::read(void *buffer, size_t size)
{
  if (! (ctx && dir < 0))
    THROW("IFFByteStream not ready for reading bytes");
  // Seek if necessary
  if (seekto > offset) {
    bs->seek(seekto);
    offset = seekto;
  }
  // Ensure that read does not extend beyond chunk
  if (offset > ctx->offEnd)
    THROW("IFFByteStream (internal error) offset beyond chunk boundary");
  if (offset + (long)size >  ctx->offEnd)
    size = (size_t) (ctx->offEnd - offset);
  // Read bytes
  size_t bytes = bs->read(buffer, size);
  offset += bytes;
  return bytes;
}


// IFFByteStream::write
// -- write bytes to IFF file chunk

size_t 
IFFByteStream::write(const void *buffer, size_t size)
{
  if (! (ctx && dir > 0))
    THROW("IFFByteStream not ready for writing bytes");
  if (seekto > offset)
    THROW("Cannot write until previous chunk is complete");
  size_t bytes = bs->write(buffer, size);
  offset += bytes;
  return bytes;
}



// IFFByteStream::flush
// -- flushes all buffers

void 
IFFByteStream::flush()
{
  bs->flush();
}


// IFFByteStream::tell
// -- tell position

long 
IFFByteStream::tell()
{
  if (seekto > offset)
    return seekto;
  else
    return offset;
}

