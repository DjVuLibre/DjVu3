//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
//C-

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
readfile()
{
  char c;
  int fd = open(__FILE__, O_RDONLY, 0666);
  if (fd < 0)
    { DjVuPrintErrorUTF8("Cannot open file '" __FILE__ "'."); exit(1); }
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
    DjVuPrintErrorUTF8("*** critical section test failed\n");
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
    { DjVuPrintErrorUTF8("%s","Cannot open file '" __FILE__ "'."); exit(0); }
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
  DjVuPrintErrorUTF8("%d enter\n", (int)arg);
  for (int i=0; i<10; i++)
    serial();
  for (int i=0; i<10; i++)
    alloc();
  if (arg)
    {
      DjVuPrintErrorUTF8("%d waiting (no timeout)\n",(int)arg);
      ev.wait();
      DjVuPrintErrorUTF8("%d waiting (timeout 3s)\n",(int)arg);
      ev.wait(3000);
      DjVuPrintErrorUTF8("%d waiting 10 seconds\n", (int)arg);
      sleeper.wait(10000);
      DjVuPrintErrorUTF8("%d leave\n", (int)arg);
    }
}


int
main()
{
  GThread th;
  GThread th2;
  th.create(second,(void*)1);
  second(0);
  DjVuPrintErrorUTF8("0 sleeping 3s\n");
  sleeper.wait(3000);
  DjVuPrintErrorUTF8("0 waking up\n");
  ev.set();
  DjVuPrintErrorUTF8("0 sleeping 6s\n");
  sleeper.wait(6000);
  DjVuPrintErrorUTF8("0 destroying thread 1 identifier\n");
  th.GThread::~GThread();
  DjVuPrintErrorUTF8("0 sleeping 10s\n");
  sleeper.wait(10000);
  DjVuPrintErrorUTF8("0 leave\n");
}

