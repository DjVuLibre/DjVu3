//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
//C-

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "GContainer.h"
#include "GString.h"

#define PRS(expr)  DjVuPrintMessageUTF8("%s :=\"%s\"\n", #expr, (const char*)(expr))
#define PRI(expr)  DjVuPrintMessageUTF8("%s :=%d\n", #expr, (int)(expr))
#define PRC(expr)  DjVuPrintMessageUTF8("%s :=%d '%c'\n", #expr, (char)(expr), (char)(expr))

// #define THOROUGH
#ifdef THOROUGH
void * operator new(size_t sz) {
  void *x = malloc(sz);
  DjVuPrintMessageUTF8("new %d = %x\n", sz, x);
  return x;
}
void operator delete(void *x) {
  DjVuPrintMessageUTF8("delete %x\n", x);
  free(x);
}
#endif

void
PARR(GArray<int> &ga)
{
  for(int i=ga.lbound();i<=ga.hbound();i++)
    DjVuPrintMessageUTF8("%d ", ga[i]);
  DjVuPrintMessageUTF8("\n");
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
  GArray<GUTF8String> ga(0,9);
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
  GArray<GUTF8String> gb (ga);
  PRI(gb.size());
  PRI(gb.lbound());
  PRI(gb.hbound());
  for(i=gb.lbound();i<=gb.hbound();i++) PRS(gb[i]);
#if 0
  gb.sort();
  DjVuPrintMessageUTF8("sorted\n");
  for(i=gb.lbound();i<=gb.hbound();i++) PRS(gb[i]);
#endif
}





int
main()
{
  setlocale(LC_ALL,"");
   G_TRY {
      test_string();
      test_integer();
   } G_CATCH(exc) {
      exc.perror();
      return 1;
   } G_ENDCATCH;
   return 0;
}
