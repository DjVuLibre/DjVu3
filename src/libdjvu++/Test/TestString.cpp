//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
//C-

#include <new.h>
#include <stdio.h>
#include <stdlib.h>
#include "GString.h"



#define PRS(expr)  DjVuPrint("%s :=\"%s\"\n", #expr, (const char*)(expr))
#define PRI(expr)  DjVuPrint("%s :=%d\n", #expr, (int)(expr))
#define PRC(expr)  DjVuPrint("%s :=%d '%c'\n", #expr, (char)(expr), (char)(expr))

// #define THOROUGH
#ifdef THOROUGH
void *operator new(size_t sz) {
  void *x = malloc(sz);
  DjVuPrint("new %d = %x\n", sz, x);
  return x;
}
void operator delete(void *x) {
  DjVuPrint("delete %x\n", x);
  free(x);
}
#endif


int
main()
{
  GString gs1;
  PRS(gs1);
  GString gs2 = "abcdefghijklmnopqrstuvwxyz";
  PRS(gs2);
  GString gs3 = gs2;
  PRS(gs3);
  GString gs4("abcdefghijk",4);
  PRS(gs4);
  GString gs5(gs2,-4, 5);
  PRS(gs5);
  gs5 = gs2;
  PRS(gs5);
  gs5 = "ghijkl";
  PRS(gs5);
  PRI(gs5.length());
  PRI(!gs5 ? 1 : 0);
  PRI(!gs1 ? 1 : 0);
  PRC(gs2[3]);
  PRC(gs2[4]);
  PRC(gs2[-1]);
  gs3 = gs2;
  PRS(gs3);
  gs2.getbuf(40);
  PRS(gs2);
  gs2.setat(24,'Z');
  PRS(gs2);
  PRS(gs3);
  GString gsu = gs2.upcase();
  PRS(gsu);
  gsu = gs2.downcase();
  PRS(gsu);
  gsu.empty();
  PRS(gsu);
  gsu.format("hello %s from %d", "folks", 314);
  PRS(gsu);
  PRI(gsu.search('f'));
  PRI(gsu.rsearch('f'));
  PRI(gsu.search('f',8));
  PRI(gsu.search('z'));
  PRI(gsu.search("folks"));
  PRI(gsu.rsearch(" f"));
  PRI(gsu.rsearch(" f",8));
  PRI(gsu.rsearch(" f",3));
  gs2=gsu;
  gs2.getbuf();
  PRS(gs2);
  PRS(gs3);
  PRI(gs2==gsu);
  PRI(gs2!=gsu);
  PRI(gs2!=gs3);
  PRI(gs2<gs3);
  PRI(gs2>gs3);

  gs2 += "efgh";
  PRS(gs2);
  
  gs1 = gsu + " " + gs3;
  PRS(gs1);
  
  GString gsa ("abcdef");
  GString gsb = GString("abc") + GString("def");
  PRS(gsa);
  PRS(gsb);
  PRI(gsa==gsb);
  PRI("abcdef"!=gsb);
  PRI(gsa<gsb);
  PRI(gsa<=gsb);
  return 0;
}
