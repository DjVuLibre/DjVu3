//C-  -*- C++ -*-
//C-
//C- This software contains software code or other information proprietary to
//C- AT&T Corp. ("AT&T").  Unless you accept a license to use this software,
//C- you shall not use, execute, compile, modify, redistribute, reverse
//C- compile, disassemble, or otherwise reverse engineer the AT&T software or
//C- any derived work of the AT&T software.  The text of a license can be
//C- found in file "ATTLICENSE" or at the Internet website having the URL
//C- "http://www.djvu.att.com/open".
//C
//C- This software is provided to you "AS IS".  YOU ASSUME TOTAL
//C- RESPONSIBILITY AND RISK FOR USE OF THE AT&T SOFTWARE.  AT&T DOES NOT
//C- MAKE, AND EXPRESSLY DISCLAIMS, ANY EXPRESS OR IMPLIED WARRANTIES OF ANY
//C- KIND WHATSOEVER, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
//C- MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, WARRANTIES OF TITLE
//C- OR NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS, ANY WARRANTIES
//C- ARISING BY USAGE OF TRADE, COURSE OF DEALING OR COURSE OF PERFORMANCE, OR
//C- ANY WARRANTY THAT THE AT&T SOFTWARE IS ERROR FREE OR WILL MEET YOUR
//C- REQUIREMENTS.
//C-
//C-     (C) AT&T Corp. All rights reserved.
//C-     AT&T is a registered trademark of AT&T Corp.
//C-
//C- $Id: TestMap.cpp,v 1.4 1999-03-15 18:28:54 leonb Exp $



#include "GContainer.h"
#include "GString.h"
#include <stdlib.h>
#include <stdio.h>


// #define THOROUGH
#ifdef THOROUGH
void * operator new(size_t sz) {
  void *x = malloc(sz);
  printf("new %d = %x\n", sz, x);
  return x;
}
void operator delete(void *x) {
  printf("delete %x\n", x);
  free(x);
}
#endif


#define PRS(expr)  printf("%s :=\"%s\"\n", #expr, (const char*)(expr))
#define PRI(expr)  printf("%s :=%d\n", #expr, (int)(expr))
#define PRC(expr)  printf("%s :=%d '%c'\n", #expr, (char)(expr), (char)(expr))

void
PMAPSI(const GMap<GString,int> &map)
{
  int i;
  GString gs;
  printf("( ");
  for (GPosition pos=map.firstpos(); pos; ++pos)
    printf("%s:%d ", (const char*)map.key(pos), map[pos]);
  printf(")\n");
}

void
PMAPIS(const GMap<int,GString> &map)
{
  int i;
  GString gs;
  printf("( ");
  for (GPosition pos=map.firstpos(); pos; ++pos)
    printf("%d:%s ", map.key(pos), (const char*)map[pos]);
  printf(")\n");
}

int
main()
{
  GMap<GString,int> forw;

  forw["one"] = 1 ;
  forw["two"] = 2;
  forw["three"] = 33;
  forw["douze"] = -1;
  PRI(forw["douze"]);
  PRI(forw.size());
  forw.del("douze");
  PRI(forw.size());
  GPosition pf;
  PRI(forw.contains("one",pf));
  PRI(forw[pf]);
  PRI(forw["one"]);
  PRI(forw.contains("thre",pf));
  forw["three"] = 3;
  PRI(forw["three"]);
  
  PMAPSI(forw);
  GMap<GString,int> copy(forw);
  PMAPSI(copy);
  
  GMap<int,GString> rev;
  int i;
  GString gs;
  for (pf=copy.firstpos(); pf; ++pf)
    rev[ copy[pf] ] = copy.key(pf);
  PMAPIS(rev);
  forw.del("two");
  PMAPSI(forw);
 
  return 0;
}
