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
  DjVuPrint("new %d = %x\n", sz, x);
  return x;
}
void operator delete(void *x) {
  DjVuPrint("delete %x\n", x);
  free(x);
}
#endif


#define PRS(expr)  DjVuPrint("%s :=\"%s\"\n", #expr, (const char*)(expr))
#define PRI(expr)  DjVuPrint("%s :=%d\n", #expr, (int)(expr))
#define PRC(expr)  DjVuPrint("%s :=%d '%c'\n", #expr, (char)(expr), (char)(expr))

void
PMAPSI(const GMap<GString,int> &map)
{
  GString gs;
  DjVuPrint("( ");
  for (GPosition pos=map.firstpos(); pos; ++pos)
    DjVuPrint("%s:%d ", (const char*)map.key(pos), map[pos]);
  DjVuPrint(")\n");
}

void
PMAPIS(const GMap<int,GString> &map)
{
  DjVuPrint("( ");
  for (GPosition pos=map.firstpos(); pos; ++pos)
    DjVuPrint("%d:%s ", map.key(pos), (const char*)map[pos]);
  DjVuPrint(")\n");
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
