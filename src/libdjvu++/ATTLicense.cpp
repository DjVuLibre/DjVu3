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
//C- $Id: ATTLicense.cpp,v 1.1 1999-08-31 14:29:00 leonb Exp $

#include <stdlib.h>
#include <stdio.h>
#include "ATTLicense.h"
#include "ByteStream.h"
#include "BSByteStream.h"

static const char *the_copyright = 
"copyright (c) AT&T 1999";

static const char *the_usage = 
"\tCopyright (c) AT&T 1999.  All rights reserved.\n"
"\tUse option \'-license\' for more information\n";

static const char *the_notice = 
"Copyright (c) 1999 AT&T Corp.  All rights reserved.\n"
"\n"
"This software may only be used by you under license from AT&T\n"
"Corp. (\"AT&T\"). A copy of AT&T's Source Code Agreement is available at\n"
"AT&T's Internet website having the URL <http://www.djvu.att.com/open>.\n"
"If you received this software without first entering into a license with\n"
"AT&T, you have an infringing copy of this software and cannot use it\n"
"without violating AT&T's intellectual property rights.\n"
"\n";

#ifdef INCLUDE_ATT_LICENSE_EVERYWHERE
// This is too big to include in every binary...
static const char *the_license = 
@ATTLICENSE@
"\000";
#endif


const char*
ATTLicense::get_notice_text()
{
  return the_notice;
}

const char*
ATTLicense::get_copyright_text()
{
  return the_copyright;
}

const char*
ATTLicense::get_usage_text()
{
  return the_usage;
}

void
ATTLicense::process_cmdline(int argc, char **argv)
{
  // Check cmdline
  int i;
  for (i=1; i<argc; i++)
    {
      char *s = argv[i];
      while (*s=='-')
        s++;
      if (strcmp(s,"license")==0)
        break;
    }
  if (i>=argc)
    return;
  // Display license
#ifdef USE_UNIX_PAGER
  char *pager = getenv("PAGER");
  if (pager==0) 
    pager="more";
  FILE *f = popen(pager,"w");
  if (f)
    {
      StaticByteStream input(the_notice);
      StdioByteStream output(f, "w");
      output.copy(input);
      if (pclose(f) >= 0)
        exit(0);
    }
#endif
  fprintf(stdout,"\n%s\n", (const char*)get_notice_text());
  exit(0);
}
