//C-  -*- C++ -*-
//C-
//C- This software contains software code or other information proprietary to
//C- AT&T Corp. ("AT&T").  Unless you accept a license to use this software,
//C- you shall not use, execute, compile, modify, redistribute, reverse
//C- compile, disassemble, or otherwise reverse engineer the AT&T software or
//C- any derived work of the AT&T software.  The text of a license can be
//C- found in file "ATTLICENSE" or at the Internet website having the URL
//C- "http://www.djvu.att.com/open".
//C
//C- This software is provided to you "AS IS".  YOU ASSUME TOTAL
//C- RESPONSIBILITY AND RISK FOR USE OF THE AT&T SOFTWARE.  AT&T DOES NOT
//C- MAKE, AND EXPRESSLY DISCLAIMS, ANY EXPRESS OR IMPLIED WARRANTIES OF ANY
//C- KIND WHATSOEVER, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
//C- MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, WARRANTIES OF TITLE
//C- OR NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS, ANY WARRANTIES
//C- ARISING BY USAGE OF TRADE, COURSE OF DEALING OR COURSE OF PERFORMANCE, OR
//C- ANY WARRANTY THAT THE AT&T SOFTWARE IS ERROR FREE OR WILL MEET YOUR
//C- REQUIREMENTS.
//C-
//C-     (C) AT&T Corp. All rights reserved.
//C-     AT&T is a registered trademark of AT&T Corp.
//C-
//C- $Id: TestSmartPointer.cpp,v 1.3 1999-03-15 18:28:54 leonb Exp $


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


