//C-  -*- C++ -*-
//C-
//C- This software contains software code or other information proprietary to
//C- AT&T Corp. ("AT&T").  Unless you accept a license to use this software,
//C- you shall not use, execute, compile, modify, redistribute, reverse
//C- compile, disassemble, or otherwise reverse engineer the AT&T software or
//C- any derived work of the AT&T software.  The text of a license can be
//C- found in file "ATTLICENSE" or at the Internet website having the URL
//C- "http://www.djvu.att.com/open".
//C-
//C- This software is provided to you "AS IS".  YOU "AS IS".  YOU ASSUME TOTAL
//C- RESPONSIBILITY AND RISK FOR YOUR USE OF THEM INCLUDING THE RISK OF ANY
//C- DEFECTS OR INACCURACIES THEREIN.  AT&T DOES NOT MAKE, AND EXPRESSLY
//C- DISCLAIMS, ANY EXPRESS OR IMPLIED WARRANTIES OF ANY KIND WHATSOEVER,
//C- INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//C- OR FITNESS FOR A PARTICULAR PURPOSE, WARRANTIES OF TITLE OR
//C- NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS OR TRADEMARK RIGHTS,
//C- ANY WARRANTIES ARISING BY USAGE OF TRADE, COURSE OF DEALING OR COURSE OF
//C- PERFORMANCE, OR ANY WARRANTY THAT THE AT&T SOURCE CODE RELEASE OR AT&T
//C- CAPSULE ARE "ERROR FREE" WILL MEET YOUR REQUIREMENTS.
//C-
//C-     (C) AT&T Corp. All rights reserved.
//C-     AT&T is a registered trademark of AT&T Corp.
//C-
//C- $Id: TestList.cpp,v 1.5 1999-03-16 20:21:31 leonb Exp $



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
PCONTI(GContainer<int> &ga)
{
  int *np;
  printf("( ");
  GPosition pos(ga);
  while( (np = ga.next(pos)))
    printf("%d ", *np);
  printf(")\n");
}

void
PCONTS(GContainer<GString> &ga)
{
  GString *np;
  printf("( ");
  GPosition pos(ga);
  while ((np = ga.next(pos)))
    printf("\"%s\" ", (const char*)(*np));
  printf(")\n");
}



void
PFIRST(const GList<GString> * const gl)
{
  GPosition pos(*gl);
  PRS((*gl)[pos]);
}


int
main()
{
  GList<GString> gl1;

  gl1.append("one");
  gl1.append("two");
  gl1.append("three");
  PRI(gl1.size());
  PCONTS(gl1);
  GPosition pos(gl1);
  PRS(gl1[pos]);
  gl1.next(pos);
  PRS(gl1[pos]);

  GList<GString> gl2 = gl1;
  gl2.prepend("zero");
  PCONTS(gl2);
  gl2 = gl1;
  PCONTS(gl2);
  PFIRST(&gl2);
  GContainer<GString>& gc1 = gl1;
  gl2 = gc1;
  gl2.prepend("zero");
  PCONTS(gl2);
  
  int n = gl2.nth(2, pos);
  PRI(n);
  PRS(gl2[pos]);

  n = gl2.search("three",pos);
  PRI(n);
  PRS(gl2[pos]);
  n = gl2.search("one",pos);      
  PRI(n);
  PRS(gl2[pos]);

  gl2.insert_after(pos, "after three");
  gl2.insert_before(pos, "before three");
  gl2.del(pos);
  PCONTS(gl2);
  
  GList<GString> gl3 = gl1;
  PRI(gl1.size());
  PCONTS(gl1);
  gl1.first(pos);
  n = gl1.search("two",pos);
  PRI(n);
  gl2.last(pos);
  const GString *ps;
  printf("( ");
  while ((ps = gl2.prev(pos)))
    printf("'%s' ",(const char*)*ps);
  printf(")\n");

  return 0;
}
