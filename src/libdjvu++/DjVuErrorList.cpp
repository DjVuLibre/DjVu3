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
//C- $Id: DjVuErrorList.cpp,v 1.3 2000-05-01 16:15:21 bcr Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuErrorList.h"
#include "GException.h"
#include "GContainer.h"
#include "GOS.h"
#include <string.h>

DjVuErrorList::DjVuErrorList() {}

void
DjVuErrorList::set_stream(GP<ByteStream> xibs)
{ ibs=xibs; }

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
   TRY
   {
     if (url.is_empty() )
     {
       if(ibs)
         retval=new DataPool(*ibs);
     }else if (url.is_local_file_url())
     {
       GString fname=GOS::url_to_filename(url);
       if (GOS::basename(fname)=="-") fname="-";
       retval=new DataPool(fname);
     }
   }
   CATCH(exc)
   {
     retval=0;
   } ENDCATCH;
   return retval;
}
 
