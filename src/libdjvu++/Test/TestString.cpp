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
//C- $Id: TestString.cpp,v 1.8 2000-09-18 17:10:34 bcr Exp $

#include <new.h>
#include <stdio.h>
#include <stdlib.h>
#include "GString.h"



#define PRS(expr)  printf("%s :=\"%s\"\n", #expr, (const char*)(expr))
#define PRI(expr)  printf("%s :=%d\n", #expr, (int)(expr))
#define PRC(expr)  printf("%s :=%d '%c'\n", #expr, (char)(expr), (char)(expr))

// #define THOROUGH
#ifdef THOROUGH
void *operator new(size_t sz) {
  void *x = malloc(sz);
  printf("new %d = %x\n", sz, x);
  return x;
}
void operator delete(void *x) {
  printf("delete %x\n", x);
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
