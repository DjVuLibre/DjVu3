//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
//C-

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
  GString gs;
  printf("( ");
  for (GPosition pos=map.firstpos(); pos; ++pos)
    printf("%s:%d ", (const char*)map.key(pos), map[pos]);
  printf(")\n");
}

void
PMAPIS(const GMap<int,GString> &map)
{
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
  GString gs;
  for (pf=copy.firstpos(); pf; ++pf)
    rev[ copy[pf] ] = copy.key(pf);
  PMAPIS(rev);
  forw.del("two");
  PMAPSI(forw);
 
  return 0;
}
