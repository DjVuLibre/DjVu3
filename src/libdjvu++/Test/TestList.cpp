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
