//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.
//C- Copyright (c) 2000 LizardTech Inc.
//C- All rights reserved.
//C-
//C- This software may only be used by you under license from LizardTech
//C- Inc. A copy of LizardTech's Source Code Agreement is available at
//C- LizardTech's Internet website having the URL <http://www.djvu.com/open>.
//C- If you received this software without first entering into a license with
//C- LizardTech, you have an infringing copy of this software and cannot use it
//C- without violating LizardTech's intellectual property rights.
//C-
//C- $Id: DjVuInfo.cpp,v 1.10 2000-10-06 21:47:21 fcrary Exp $


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
    dpi(300), gamma(2.2), compressable(false)
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
  compressable=false;
  // Read data
  unsigned char buffer[10];
  int  size = bs.readall((void*)buffer, sizeof(buffer));
  if (size == 0)
    G_THROW("EOF");
  if (size < 5)
    G_THROW("DjVuInfo.corrupt_file");
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
  int flags=0;
  if (size>=10)
    flags = buffer[9];
  // Fixup
  if (gamma<0.3)
     gamma=0.3;
  if (gamma>5.0)
     gamma=5.0;
  if (dpi < 25 || dpi > 6000)
    dpi = 300;
  if(flags&COMPRESSABLE_FLAG)
    compressable=true;
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
  unsigned char flags=0;
  if(compressable) 
  {
    flags|=COMPRESSABLE_FLAG;
  }
  bs.write8(flags);
}

unsigned int 
DjVuInfo::get_memory_usage() const
{
  return sizeof(DjVuInfo);
}
