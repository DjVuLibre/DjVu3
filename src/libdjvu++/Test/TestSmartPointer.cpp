//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.
//C- Copyright (c) 2000 LizardTech Inc.
//C- All rights reserved.
//C-
//C- This software may only be used by you under license from LizardTech
//C- Inc. A copy of LizardTech's Source Code Agreement is available at
//C- LizardTech's Internet website having the URL <http://www.lizardtech.com/open>.
//C- If you received this software without first entering into a license with
//C- LizardTech, you have an infringing copy of this software and cannot use it
//C- without violating LizardTech's intellectual property rights.
//C-
//C- $Id: TestSmartPointer.cpp,v 1.10 2000-11-02 02:17:06 bcr Exp $

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


