//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: TestArray.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $


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
PCONT(GContainer<int> &ga)
{
  int *np;
  GPosition pos(ga);
  while (np = ga.next(pos))
    printf("%d ", *np);
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
  PCONT(ga);
  ga.touch(-10);
  PRI(ga.lbound());
  PRI(ga.hbound());
  PCONT(ga);

  ga.del(1);
  PRI(ga.lbound());
  PRI(ga.hbound());
  PCONT(ga);

  ga.ins(-4, 23, 5);
  PRI(ga.lbound());
  PRI(ga.hbound());
  PCONT(ga);


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
  const GString *gsp;
  GPosition pos(ga);
  PRS(ga[pos]);
  while ((gsp = ga.next(pos)))
    PRS(*gsp);
  // Test of iterator
  ga.del(1);
  ga.ins(1,"hello",2);
  ga.first(pos);
  PRS(ga[pos]);
  while ((gsp = ga.next(pos)))
    PRS(*gsp);
  // Test of copy
  GArray<GString> gb (ga);
  PRI(gb.size());
  PRI(gb.lbound());
  PRI(gb.hbound());
  gb.first(pos);
  while ((gsp = gb.next(pos)))
    PRS(*gsp);

  // Test of copy
  GContainer<GString>& gc = ga;
  GArray<GString> gd (gc);
  PRI(gd.size());
  PRI(gd.lbound());
  PRI(gd.hbound());
  gd.first(pos);
  GString *gsc;
  while ((gsc = gd.next(pos)))
    PRS(*gsc);

}





int
main()
{
  test_string();
  test_integer();
  return 0;
}
