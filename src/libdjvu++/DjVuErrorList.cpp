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
//C- $Id: DjVuErrorList.cpp,v 1.1 1999-12-05 21:46:17 bcr Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuErrorList.h"
#include "GException.h"
#include "GContainer.h"
#include <string.h>

DjVuErrorList::DjVuErrorList() {}

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

