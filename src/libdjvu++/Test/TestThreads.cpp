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
//C- $Id: TestThreads.cpp,v 1.5 1999-03-15 18:28:54 leonb Exp $

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "GThreads.h"



GThread th;
GEvent ev;
GCriticalSection sec;
static int flag = 0;

#include <stdarg.h>


void 
xprintf(char *fmt, ...)
{
  char buf[256];
  va_list ap;
  va_start(ap, fmt);
  vsprintf(buf, fmt, ap);
  va_end(ap);
  write(2,buf,strlen(buf));
}

void 
readfile()
{
  char c;
  int fd = open(__FILE__, O_RDONLY, 0666);
  if (fd < 0)
    { fprintf(stderr,"Cannot open file '" __FILE__ "'."); exit(1); }
  while (read(fd, &c, 1)==1) 
    { 
      GThread::yield(); 
    }
  close(fd);
}


void 
serial()
{
  GCriticalSectionLock lock(&sec);
  if (flag)
    xprintf("*** critical section test failed\n");
  flag = 1;
  readfile();
  { // embedded lock
    GCriticalSectionLock lock(&sec);
    readfile();
  }
  flag = 0;
}


void 
alloc()
{
  int size = 1;
  char *mem = (char*)malloc(1);
  FILE *f = fopen(__FILE__,"r");
  if (f == 0)
    { fprintf(stderr,"Cannot open file '" __FILE__ "'."); exit(0); }
  while (!feof(f))
    {
      mem = (char*)realloc(mem, size+1);
      mem[size] = fgetc(f);
      size += 1;
    }
  fclose(f);
  free(mem);
}


void
second(void *arg)
{
  xprintf("%d enter\n", (int)arg);
  for (int i=0; i<10; i++)
    serial();
  for (int i=0; i<10; i++)
    alloc();
  if (arg)
    {
      xprintf("%d waiting (no timeout)\n",(int)arg);
      ev.wait();
      xprintf("%d waiting (timeout 3s)\n",(int)arg);
      ev.wait(3000);
      xprintf("%d leave\n", (int)arg);
    }
}


GEvent sleeper;

int
main()
{
  th.create(second,(void*)1);
  second(0);
  xprintf("0 sleeping 3s\n");
  sleeper.wait(3000);
  xprintf("0 waking up\n");
  ev.set();
  xprintf("0 sleeping 6s\n");
  sleeper.wait(6000);
  xprintf("0 leave\n");
}

