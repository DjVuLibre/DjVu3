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
//C- $Id: TestThreads.cpp,v 1.10 2000-07-24 16:33:38 bcr Exp $

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "GThreads.h"



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

GEvent sleeper;


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
      xprintf("%d waiting 10 seconds\n", (int)arg);
      sleeper.wait(10000);
      xprintf("%d leave\n", (int)arg);
    }
}


int
main()
{
  GThread th;
  GThread th2;
  th.create(second,(void*)1);
  second(0);
  xprintf("0 sleeping 3s\n");
  sleeper.wait(3000);
  xprintf("0 waking up\n");
  ev.set();
  xprintf("0 sleeping 6s\n");
  sleeper.wait(6000);
  xprintf("0 destroying thread 1 identifier\n");
  th.GThread::~GThread();
  xprintf("0 sleeping 10s\n");
  sleeper.wait(10000);
  xprintf("0 leave\n");
}

