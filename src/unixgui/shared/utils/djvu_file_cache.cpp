//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// $Name:  $
static char RCSVersion[]="@(#) $Id: djvu_file_cache.cpp,v 1.1 2001-08-08 17:38:05 docbill Exp $";

#ifdef __GNUC__
#pragma implementation
#endif

#include "djvu_file_cache.h"

static GP<DjVuFileCache> cache=NULL;

DjVuFileCache * get_file_cache(void)
{
   if (!cache) cache=DjVuFileCache::create();
   if (!cache) G_THROW("Failed to initialize DjVuFile cache.");
   return cache;
}

