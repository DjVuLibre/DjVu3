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
//C- $Id: DjVuAnno.cpp,v 1.1.2.1 1999-04-12 16:48:21 eaf Exp $


#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuAnno.h"

DjVuAnno::DjVuAnno()
{
}

void 
DjVuAnno::decode(ByteStream &bs)
{
  GCriticalSectionLock lock(&mutex);
  char buf[512];
  int len = sizeof(buf);
  while (len>0) {
    len = bs.readall((void*)buf, sizeof(buf));
    raw = raw + GString(buf, len);
  } 
}

void 
DjVuAnno::encode(ByteStream &bs)
{
  GCriticalSectionLock lock(&mutex);
  bs.writall((const void*)raw, raw.length());
}

unsigned int 
DjVuAnno::get_memory_usage() const
{
  return sizeof(DjVuAnno) + raw.length();
}
