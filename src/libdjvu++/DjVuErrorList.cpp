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
// $Id: DjVuErrorList.cpp,v 1.11.2.1 2001-03-28 01:04:27 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuErrorList.h"
#include "DjVmDoc.h"
#include "GException.h"
#include "GContainer.h"
#include "GOS.h"
#include "DataPool.h"
#include <string.h>

DjVuErrorList::DjVuErrorList() {}

GURL
DjVuErrorList::set_stream(GP<ByteStream> xibs)
{
  GString name;
  static unsigned long serial=0;
  pool=DataPool::create(*xibs);
  name.format("data://%08lx/%08lx.djvu",
    ++serial,(unsigned long)(size_t)((const ByteStream *)xibs));
  pool_url=GURL::UTF8(name);
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
   GP<DataPool> retval;
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
         GP<DjVmDoc> doc=DjVmDoc::create();
         GP<ByteStream> bs=pool->get_stream();
         doc->read(*bs);
         retval=doc->get_data(name);
       }
     }else if (url.is_local_file_url())
     {
//       GString fname=GOS::url_to_filename(url);
//       if (GOS::basename(fname)=="-") fname="-";
       retval=DataPool::create(url);
     }
   }
   G_CATCH_ALL
   {
     retval=0;
   } G_ENDCATCH;
   return retval;
}
 
