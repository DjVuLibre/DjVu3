//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1998,1999 AT&T Corp.
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
//C- $Id: TestArray.cpp,v 1.11 2000-09-18 17:10:33 bcr Exp $

#include <stdio.h>
#include <stdlib.h>
#include "GContainer.h"
#include "GString.h"


#define PRS(expr)  printf("%s :=\"%s\"\n", #expr, (const char*)(expr))
#define PRI(expr)  printf("%s :=%d\n", #expr, (int)(expr))
#define PRC(expr)  printf("%s :=%d '%c'\n", #expr, (char)(expr), (char)(expr))

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

void
PARR(GArray<int> &ga)
{
  for(int i=ga.lbound();i<=ga.hbound();i++)
    printf("%d ", ga[i]);
  printf("\n");
}

void
test_integer()
{
  GArray<int> ga;

  ga.resize(0,12);
  ga[0] = 1;
  ga[2] = 3;
  ga.resize(12);
  PRI(ga.lbound());
  PRI(ga.hbound()); 
  PARR(ga);
  ga.touch(-10);
  PRI(ga.lbound());
  PRI(ga.hbound());
  PARR(ga);

  ga.del(1);
  PRI(ga.lbound());
  PRI(ga.hbound());
  PARR(ga);

  ga.ins(-4, 23, 5);
  PRI(ga.lbound());
  PRI(ga.hbound());
  PARR(ga);
#if 0
  ga.sort(-2,5);
  PARR(ga);
#endif
}




void
test_string()
{
  GArray<GString> ga(0,9);
  // Test of allocation
  ga[0] = "zero";
  ga[2] = "two";
  PRS(ga[0]);
  PRS(ga[1]);
  PRS(ga[2]);
  ga.resize(5);
  PRS(ga[5]);
  PRI(ga.lbound());
  PRI(ga.hbound()); 
  PRS(ga[0]);
  PRS(ga[1]);
  PRS(ga[2]);
  PRS(ga[5]);
  ga.touch(-2);
  PRI(ga.lbound());
  PRI(ga.hbound());
  PRS(ga[0]);
  PRS(ga[1]);
  PRS(ga[2]);
  PRS(ga[5]);
  ga[3] = "three";
  PRS(ga[3]);
  ga[-2] = "minustwo";
  ga[4] = "four";
  ga[1] = "one";
  // Test of iterator
  int i;
  for(i=ga.lbound();i<=ga.hbound();i++) PRS(ga[i]);
  // Test of iterator
  ga.del(1);
  ga.ins(1,"hello",2);
  for(i=ga.lbound();i<=ga.hbound();i++) PRS(ga[i]);
  // Test of copy
  GArray<GString> gb (ga);
  PRI(gb.size());
  PRI(gb.lbound());
  PRI(gb.hbound());
  for(i=gb.lbound();i<=gb.hbound();i++) PRS(gb[i]);
#if 0
  gb.sort();
  printf("sorted\n");
  for(i=gb.lbound();i<=gb.hbound();i++) PRS(gb[i]);
#endif
}





int
main()
{
   G_TRY {
      test_string();
      test_integer();
   } G_CATCH(exc) {
      exc.perror();
      return 1;
   } G_ENDCATCH;
   return 0;
}
