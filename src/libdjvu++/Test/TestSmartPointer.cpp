//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: TestSmartPointer.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $


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
  printf("creating test-%c\n", id);
}

Test::~Test()
{
  printf("deleting test-%c\n", id);
}

void
Test::show(char *s)
{
  printf("%s contains test-%c with counter %d\n", 
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
}


