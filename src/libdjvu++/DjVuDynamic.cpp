//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: DjVuDynamic.cpp,v 1.1 2001-07-12 23:33:16 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GException.h"
#include "DjVuDynamic.h"
#include "GURL.h"
#include "GString.h"

#if  defined(WIN32) || HAS_DLOPEN 

#if defined(WIN32)
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

class DjVuDynamicLib : public GPEnabled
{
public:
  DjVuDynamicLib(void);
  ~DjVuDynamicLib();
  static GP<DjVuDynamicLib> create(
    const GUTF8String &name,const bool nothrow=false);
  static GP<DjVuDynamicLib> create(
    const GURL &name,const bool nothrow=false);
  void *lookup(const GUTF8String &name);
private:
  GUTF8String name;
#ifdef WIN32
  HINSTANCE handle;
#else
  void *handle;
#endif
  GMap<GUTF8String,void *> map;
};

DjVuDynamicLib::DjVuDynamicLib(void)
: handle(0) {}

GP<DjVuDynamicLib> 
DjVuDynamicLib::create(const GUTF8String &name, const bool nothrow)
{
  DjVuDynamicLib * const lib=new DjVuDynamicLib;
  GP<DjVuDynamicLib> retval=lib;
  lib->name=name;
#ifdef WIN32
  lib->handle=LoadLibrary((const char *)GNativeString(name));
#else
  lib->handle=dlopen((const char *)GNativeString(name),RTLD_LAZY);
#endif
  if(!lib->handle)
  {
    if(!nothrow)
    {
#ifndef WIN32
      const GUTF8String mesg=GNativeString(dlerror());
      if(mesg.length())
      {
        G_THROW( (ERR_MSG("DjVuDynamicLib.failed_open2") "\t")+name+"\t"+mesg);
      }
#endif
      G_THROW( (ERR_MSG("DjVuDynamicLib.failed_open") "\t")+name);
    }
    retval=0;
  }
  return retval;
}

GP<DjVuDynamicLib> 
DjVuDynamicLib::create(const GURL &url, const bool nothrow)
{
  return create(url.UTF8Filename(),nothrow);
}

DjVuDynamicLib::~DjVuDynamicLib()
{
  if(handle)
  {
#ifdef WIN32
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
  }
}

void *
DjVuDynamicLib::lookup(const GUTF8String &name)
{
  GPosition pos=map.contains(name);
  if(handle && !pos)
  {
#ifdef WIN32
    map[name]=GetProcessAddress(handle,(const char *)name);
#else
    map[name]=dlsym(handle,(const char *)name);
#endif
    pos=map.contains(name);
  }
  return pos?map[pos]:0;
}

void *
DjVuDynamic(
  const GUTF8String &libname,
  const GUTF8String &symname,
  const bool nothrow )
{
  static GMap<GUTF8String, GP<DjVuDynamicLib> > map;
  GP<DjVuDynamicLib> lib;
  {
    GPosition pos=map.contains(libname);
    if(! pos)
    {
      map[libname]=lib=DjVuDynamicLib::create(libname,nothrow);
    }else
    {
      lib=map[pos];
    }
  }
  return lib?(lib->lookup(symname)):0;
}

void *
DjVuDynamic(
  const GURL &url,
  const GUTF8String &symname,
  const bool nothrow )
{
  return (!nothrow||(url.is_valid()&&url.is_file()))?
    DjVuDynamic(url.get_string(),symname,nothrow):0;
}
#else
void *
DjVuDynamic(
  const GUTF8STring &,const GUTF8String &,const bool)
{
  return 0;
}

void *
DjVuDynamic(
  const GURL &,const GUTF8String &,const bool)
{
  return 0;
}
#endif // defined(WIN32) || HAS_DLOPEN 

