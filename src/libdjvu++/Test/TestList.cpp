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
//C- $Id: TestList.cpp,v 1.11 2000-09-18 17:10:33 bcr Exp $

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
PCONTI(GList<int> &ga)
{
  printf("( ");
  for (GPosition pos=ga; pos; ++pos)
    printf("%d ", ga[pos]);
  printf(")\n");
}

void
PCONTS(GList<GString> &ga)
{
  printf("( ");
  for (GPosition pos=ga; pos; ++pos)
    printf("\"%s\" ", (const char*)(ga[pos]));
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
  GPosition pos = gl1;
  PRS(gl1[pos]);
  ++pos;
  PRS(gl1[pos]);

  GList<GString> gl2 = gl1;
  gl2.prepend("zero");
  PCONTS(gl2);
  gl2 = gl1;
  PCONTS(gl2);
  PFIRST(&gl2);
  gl2 = gl1;
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
  pos = gl1.firstpos();
  n = gl1.search("two",pos);
  PRI(n);
  printf("( ");
  for (pos=gl2.lastpos(); pos; --pos)
    printf("'%s' ", (const char*)gl2[pos]);
  printf(")\n");
  return 0;
}
