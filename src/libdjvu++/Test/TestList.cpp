//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
//C-

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include "GContainer.h"
#include "GString.h"
#include "DjVuMessage.h"

// #define THOROUGH
#ifdef THOROUGH
void * operator new(size_t sz) {
  void *x = malloc(sz);
  DjVuPrintMessage("new %d = %x\n", sz, x);
  return x;
}
void operator delete(void *x) {
  DjVuPrintMessage("delete %x\n", x);
  free(x);
}
#endif


#define PRS(expr)  DjVuPrintMessage("%s :=\"%s\"\n", #expr, (const char*)(expr))
#define PRI(expr)  DjVuPrintMessage("%s :=%d\n", #expr, (int)(expr))
#define PRC(expr)  DjVuPrintMessage("%s :=%d '%c'\n", #expr, (char)(expr), (char)(expr))

void
PCONTI(GList<int> &ga)
{
  DjVuPrintMessage("( ");
  for (GPosition pos=ga; pos; ++pos)
    DjVuPrintMessage("%d ", ga[pos]);
  DjVuPrintMessage(")\n");
}

void
PCONTS(GList<GUTF8String> &ga)
{
  DjVuPrintMessage("( ");
  for (GPosition pos=ga; pos; ++pos)
    DjVuPrintMessage("\"%s\" ", (const char*)(ga[pos]));
  DjVuPrintMessage(")\n");
}



void
PFIRST(const GList<GUTF8String> * const gl)
{
  GPosition pos(*gl);
  PRS((*gl)[pos]);
}


int
main(int,char *argv[],char *[])
{
   setlocale(LC_ALL,"");
   DjVuMessage::set_programname(GNativeString(argv[0]));
   
  GList<GUTF8String> gl1;

  gl1.append("one");
  gl1.append("two");
  gl1.append("three");
  PRI(gl1.size());
  PCONTS(gl1);
  GPosition pos = gl1;
  PRS(gl1[pos]);
  ++pos;
  PRS(gl1[pos]);

  GList<GUTF8String> gl2 = gl1;
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
  
  GList<GUTF8String> gl3 = gl1;
  PRI(gl1.size());
  PCONTS(gl1);
  pos = gl1.firstpos();
  n = gl1.search("two",pos);
  PRI(n);
  DjVuPrintMessage("( ");
  for (pos=gl2.lastpos(); pos; --pos)
    DjVuPrintMessage("'%s' ", (const char*)gl2[pos]);
  DjVuPrintMessage(")\n");
  return 0;
}
