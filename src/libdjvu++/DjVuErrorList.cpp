//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- 
//C- This software (the "Original Code") is subject to, and may be
//C- distributed under, the GNU General Public License, Version 2.
//C- The license should have accompanied the Original Code or you
//C- may obtain a copy of the license from the Free Software
//C- Foundation at http://www.fsf.org .
//C- 
//C- With respect to the Original Code, and subject to any third
//C- party intellectual property claims, LizardTech grants recipient
//C- a worldwide, royalty-free, non-exclusive license under patent
//C- claims infringed by making, using, or selling Original Code
//C- which are now or hereafter owned or controlled by LizardTech,
//C- but solely to the extent that any such patent is reasonably
//C- necessary to enable you to make, have made, practice, sell, or 
//C- otherwise dispose of Original Code (or portions thereof) and
//C- not to any greater extent that may be necessary to utilize
//C- further modifications or combinations.
//C- 
//C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
//C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
//C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: DjVuErrorList.cpp,v 1.6 2000-11-02 01:08:34 bcr Exp $
// $Name:  $


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
 
