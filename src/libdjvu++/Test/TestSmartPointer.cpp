//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
//C-

#include <stdio.h>
#include <stdlib.h>
#include "GSmartPointer.h"


static char genid = 'a';


class Test : public GPEnabled 
{
public:
  char id;
  Test();
  ~Test();
  void show(char *s);
};


Test::Test()
{
  id = genid++;
  DjVuPrintMessageUTF8("creating test-%c\n", id);
}

Test::~Test()
{
  DjVuPrintMessageUTF8("deleting test-%c\n", id);
}

void
Test::show(char *s)
{
  DjVuPrintMessageUTF8("%s contains test-%c with counter %d\n", 
         s, id, count);
}


GP<Test>
subr1()
{
  return new Test;
}

GP<Test>
subr2(GP<Test> arg)
{
  arg->show("arg");
  return arg;
}

GP<Test>
subr3(Test *arg)
{
  return arg;
}




int
main()
{
  GP<Test> t1 = new Test;
  {
    GP<Test> t2 = new Test;
    t2->show("t2");
  }
  t1->show("t1");
  t1 = new Test();
  t1->show("t1");
  {
    GP<Test> t3 = new Test;
    GP<Test> t4 = t1;
    t4->show("t4");
    t1 = t3;
    t3->show("t3");
    t1->show("t1");
  }
  t1->show("t1");
  GP<Test> t5 = subr1();
  t5->show("t5");
  t5 = subr2(t5);
  t5->show("t5");  
  t5 = subr3(t5);
  t5->show("t5");  
  return 0;
}


