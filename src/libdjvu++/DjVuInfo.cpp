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
//C- $Id: DjVuInfo.cpp,v 1.1.1.1 1999-10-22 19:29:23 praveen Exp $


#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuInfo.h"

// ----------------------------------------
// CLASS DJVUINFO


#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x


DjVuInfo::DjVuInfo()
  : width(0), height(0), 
    version(DJVUVERSION),
    dpi(300), gamma(2.2), reserved(0)
{
}

void 
DjVuInfo::decode(ByteStream &bs)
{
  // Set to default values
  width = 0;
  height = 0;
  version = DJVUVERSION;
  dpi = 300;
  gamma = 2.2;
  reserved = 0;
  // Read data
  unsigned char buffer[10];
  int  size = bs.readall((void*)buffer, sizeof(buffer));
  if (size == 0)
    THROW("EOF");
  if (size < 5)
    THROW("DjVu Decoder: Corrupted file (truncated INFO chunk)");
  // Analyze data with backward compatibility in mind!
  if (size>=2)
    width = (buffer[0]<<8) + buffer[1];
  if (size>=4)
    height = (buffer[2]<<8) + buffer[3];
  if (size>=5)
    version = buffer[4];
  if (size>=6 && buffer[5]!=0xff)
    version = (buffer[5]<<8) + buffer[4];
  if (size>=8 && buffer[7]!=0xff)
    dpi = (buffer[7]<<8) + buffer[6];
  if (size>=9)
    gamma = 0.1 * buffer[8];
  if (size>=10)
    reserved = buffer[9];
  // Fixup
  if (gamma <= 0.3 || gamma >= 5.0)
    gamma = 2.2;
  if (dpi < 25 || dpi > 6000)
    dpi = 300;
}

void 
DjVuInfo::encode(ByteStream &bs)
{
  bs.write16(width);
  bs.write16(height);
  bs.write8(version & 0xff);
  bs.write8(version >> 8);
  bs.write8(dpi & 0xff);
  bs.write8(dpi >> 8);
  bs.write8((int)(10.0*gamma+0.5) );
  bs.write8(reserved);
}

unsigned int 
DjVuInfo::get_memory_usage() const
{
  return sizeof(DjVuInfo);
}
