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
//C- $Id: DjVuErrorList.cpp,v 1.5 2000-09-18 17:10:09 bcr Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuErrorList.h"
#include "DjVmDoc.h"
#include "GException.h"
#include "GContainer.h"
#include "GOS.h"
#include <string.h>

DjVuErrorList::DjVuErrorList() {}

GURL
DjVuErrorList::set_stream(GP<ByteStream> xibs)
{
  GString name;
  static unsigned long serial=0;
  pool=new DataPool(*xibs);
  name.format("data://%08lx/%08lx.djvu",
    ++serial,(unsigned long)(size_t)((const ByteStream *)xibs));
  pool_url=name;
  return pool_url;
}

bool
DjVuErrorList::notify_error(const DjVuPort * source, const char * msg)
{
  Errors.append(msg);
  return 1;
}

bool
DjVuErrorList::notify_status(const DjVuPort * source, const char * msg)
{
  Status.append(msg);
  return 1;
}  

const char *
DjVuErrorList::GetError(void)
{
  const char *retval=0;
  GPosition pos;
  if((pos=Errors))
  {
    PrevError=Errors[pos];
    Errors.del(pos);
    retval=(const char *)PrevError;
  }
  return retval;
}

const char *
DjVuErrorList::GetStatus(void)
{
  const char *retval=0;
  GPosition pos;
  if((pos=Status))
  {
    PrevStatus=Status[pos];
    Status.del(pos);
    retval=(const char *)PrevStatus;
  }
  return retval;
}

GP<DataPool>
DjVuErrorList::request_data(const DjVuPort * source, const GURL & url)
{
   DataPool *retval=0;
   G_TRY
   {
     if (pool && url.protocol().downcase() == "data")
     {
       if(url == pool_url)
       {
         retval=pool;
       }else if(url.base() == pool_url)
       {
         GString name=url.fname();
         GP<DjVmDoc> doc=new DjVmDoc();
         GP<ByteStream> bs=pool->get_stream();
         doc->read(*bs);
         retval=doc->get_data(name);
       }
     }else if (url.is_local_file_url())
     {
       GString fname=GOS::url_to_filename(url);
       if (GOS::basename(fname)=="-") fname="-";
       retval=new DataPool(fname);
     }
   }
   G_CATCH_ALL
   {
     retval=0;
   } G_ENDCATCH;
   return retval;
}
 
