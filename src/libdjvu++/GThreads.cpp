//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GThreads.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $


// **** File "$Id: GThreads.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $"
// This file defines machine independent classes
// for running and synchronizing threads.
// - Author: Leon Bottou, 01/1998

#ifdef __GNUC__
#pragma implementation
#endif

#include "GThreads.h"
#include "GException.h"
#include <stdlib.h>
#include <stdio.h>

// ----------------------------------------
// Consistency check

#if THREADMODEL!=NOTHREADS
#ifdef USE_EXCEPTION_EMULATION
#warning "Compiler must support thread safe exceptions"
#endif //USE_EXCEPTION_EMULATION
#if defined(__GNUC__)
#if (__GNUC__<2) || ((__GNUC__==2) && (__GNUC_MINOR__<=8))
#warning "GCC 2.8 exceptions are not thread safe."
#warning "Use properly configured EGCS-1.1 or greater."
#endif // (__GNUC__<2 ...
#endif // defined(__GNUC__)
#endif // THREADMODEL!=NOTHREADS


// ----------------------------------------
// NOTHREADS
// ----------------------------------------

#if THREADMODEL==NOTHREADS
int
GThread::create( void (*entry)(void*), void *arg)
{
  (*entry)(arg);
  return 0;
}
#endif


// ----------------------------------------
// WIN32 IMPLEMENTATION
// ----------------------------------------

#if THREADMODEL==WINTHREADS
#include <process.h>

static unsigned __stdcall 
start(void *arg)
{
  GThread *gt = (GThread*)arg;
  try 
    {
      TRY
        {
          gt->xentry( gt->xarg );
        }
      CATCH(ex)
        {
          ex.perror();
          fprintf(stderr, "GThreads: uncaught exception.");
          abort();
        }
      ENDCATCH;
    }
  catch(...)
    {
      fprintf(stderr, "GThreads: unrecognized uncaught exception.");
      abort();
    }
  return 0;
}

GThread::GThread(int stacksize)
  : hthr(0), thrid(0), xentry(0), xarg(0)
{
}

GThread::~GThread()
{
  if (hthr)
    CloseHandle(hthr);
  hthr = 0;
  thrid = 0;
}

int  
GThread::create(void (*entry)(void*), void *arg)
{
  if (hthr)
    return -1;
  xentry = entry;
  xarg = arg;
  unsigned uthread = 0;
  hthr = (HANDLE)_beginthreadex(NULL, 0, start, (void*)this, 0, &uthread);
  thrid = (DWORD) uthread;
  if (hthr)
    return 0;
  return -1;
}

void 
GThread::terminate()
{
  OutputDebugString("Terminating thread.\n");
  TerminateThread(hthr,0);
}

int
GThread::yield()
{
  Sleep(0);
  return 0;
}

void*
GThread::current()
{
  return (void*) GetCurrentThreadId();
}

#endif



// ----------------------------------------
// MACTHREADS IMPLEMENTATION (from Praveen)
// ----------------------------------------

#if THREADMODEL==MACTHREADS

GThread::GThread(int stacksize) 
  : thid(0)
{
}

GThread::~GThread(void)
{
}

pascal void
start(void *arg)
{
  GThread *gt = (GThread*)arg;
  try 
    {
      TRY
        {
          (gt->xentry)(gt->xarg);
        }
      CATCH(ex)
        {
          ex.perror();
          fprintf(stderr, "GThreads: uncaught exception.");
          abort();
        }
      ENDCATCH;
    }
  catch(...)
    {
      fprintf(stderr, "GThreads: unrecognized uncaught exception.");
      abort();
    }
  return 0;
}

int
GThread::create(void (*entry)(void*), void *arg)
{
  if (thid)
    return -1;
  xentry = entry;
  xarg = arg;
  int err = NewThread( kCooperativeThread, 
                       start , this, 0,
                       kCreateIfNeeded, 
                       (void**)nil, &thid );
  if( err != noErr )
    return err;
  return 0;
}

void
GThread::terminate()
{
  if (thid)
    DisposeThread( thid, NULL, false );
}

int
GThread::yield()
{
  YieldToAnyThread();
  return 0;
}

void*
GThread::current()
{
  unsigned long thid;
  GetCurrentThread(&thid);
  return (void*) thid;
}


// GCriticalSection: unimportant since mac threads are cooperative
GCriticalSection::GCriticalSection() {}
GCriticalSection::~GCriticalSection() {}
void GCriticalSection::lock() {}
void GCriticalSection::unlock() {}

// GEvent: this is highly suspicious (LYB!)
GEvent::GEvent() {}
GEvent::~GEvent() {}
void GEvent::set() {}
void GEvent::wait() {}
void GEvent::wait(int timeout) {}

#endif



// ----------------------------------------
// POSIXTHREADS IMPLEMENTATION
// ----------------------------------------

#if THREADMODEL==POSIXTHREADS

#if defined(CMA_INCLUDE) || defined(pthread_attr_default)
#define DCETHREADS
#define pthread_key_create pthread_keycreate
static pthread_t nullthread;
#else
#define pthread_mutexattr_default  NULL
#define pthread_condattr_default   NULL
static const pthread_t nullthread = 0;
#endif

static void *
start(void *arg)
{
  GThread *gt = (GThread*)arg;
#ifdef DCETHREADS
#ifdef CANCEL_ON
  pthread_setcancel(CANCEL_ON);
  pthread_setasynccancel(CANCEL_ON);
#endif
#else // !DCETHREADS
#ifdef PTHREAD_CANCEL_ENABLE
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
#endif
#ifdef PTHREAD_CANCEL_ASYNCHRONOUS
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
#endif
#endif
  // Catch exceptions
#ifdef __EXCEPTIONS
  try 
    {
#endif 
      TRY
        {
          (gt->xentry)(gt->xarg);
        }
      CATCH(ex)
        {
          ex.perror();
          fprintf(stderr, "GThreads: uncaught exception.");
          abort();
        }
      ENDCATCH;
#ifdef __EXCEPTIONS
    }
  catch(...)
    {
      fprintf(stderr, "GThreads: unrecognized uncaught exception.");
      abort();
    }
#endif 
  return 0;
}

// GThread

GThread::GThread(int stacksize) : 
    hthr(nullthread),
    xentry(0), 
    xarg(0)
{
}

GThread::~GThread()
{
}

int  
GThread::create(void (*entry)(void*), void *arg)
{
  if (hthr)
    return -1;
  xentry = entry;
  xarg = arg;
#ifdef DCETHREADS
  int ret = pthread_create(&hthr, pthread_attr_default, start, (void*)this);
  if (ret >= 0)
    pthread_detach(&hthr);
#else
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  int ret = pthread_create(&hthr, &attr, start, (void*)this);
  pthread_attr_destroy(&attr);
#endif
  return ret;
}

void 
GThread::terminate()
{
  pthread_cancel(hthr);
}

int
GThread::yield()
{
#ifdef DCETHREADS
  pthread_yield();
#else
  // should use sched_yield() when available.
  static struct timeval timeout = { 0, 0 };
  ::select(0, 0,0,0, &timeout);
#endif
  return 0;
}

void*
GThread::current()
{
  pthread_t self = pthread_self();
#if defined(pthread_getunique_np)
  return (void*) pthread_getunique_np( & self );
#elif defined(cma_thread_get_unique)
  return (void*) cma_thread_get_unique( & self );  
#else
  return (void*) self;
#endif
}



// -- GCriticalSection

GCriticalSection::GCriticalSection() 
  : count(0), locker(nullthread) 
{	
  pthread_mutex_init(&mutex, pthread_mutexattr_default);
  ok = 1;
}

GCriticalSection::~GCriticalSection()
{	
  ok = 0;
  pthread_mutex_destroy(&mutex); 
}

void 
GCriticalSection::lock() 
{
  pthread_t self = pthread_self();
  if (count<=0 || !pthread_equal(locker, self))
    {
      if (ok)
        pthread_mutex_lock(&mutex);
      locker = self;
    }
  count += 1;
}

void 
GCriticalSection::unlock() 
{
  pthread_t self = pthread_self();
  if (! pthread_equal(locker, self))
    THROW("GCriticalSection has been misused");
  count -= 1;
  if (count == 0) 
    {
      count  = 0;
      locker = nullthread;
      if (ok)
        pthread_mutex_unlock(&mutex);
    }
}

// -- GEvent

GEvent::GEvent() 
  : status(0) 
{
  pthread_cond_init(&cond, pthread_condattr_default); 
  pthread_mutex_init(&mutex, pthread_mutexattr_default);
}

GEvent::~GEvent() 
{
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
}

void
GEvent::set() 
{
  if (status) 
    return;
  pthread_mutex_lock(&mutex);
  status = 1;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
}

void
GEvent::wait()
{
  pthread_mutex_lock(&mutex);
  if (! status)
    pthread_cond_wait(&cond, &mutex);
  status = 0;
  pthread_mutex_unlock(&mutex);
}

void
GEvent::wait(int timeout) 
{
  pthread_mutex_lock(&mutex);
  if (! status)
    {	
      struct timeval  abstv;
      struct timespec absts;
      gettimeofday(&abstv, NULL); // grrr
      absts.tv_sec = abstv.tv_sec + timeout/1000;
      absts.tv_nsec = abstv.tv_usec*1000  + (timeout%1000)*1000000;
      if (absts.tv_nsec > 1000000000) {
        absts.tv_nsec -= 1000000000;
        absts.tv_sec += 1;
      }
      pthread_cond_timedwait(&cond, &mutex, &absts);
    }
  status = 0;
  pthread_mutex_unlock(&mutex);
}

#endif


// ----------------------------------------
// CUSTOM COOPERATIVE THREADS
// ----------------------------------------

#if THREADMODEL==COTHREADS

#ifndef __GNUG__
#error "COTHREADS require G++"
#endif
#if (__GNUC__<2) || ((__GNUC__==2) && (__GNUC_MINOR__<=90))
#warning "COTHREADS require EGCS-1.1.1 with Leon's libgcc patch."
#warning "You may have trouble with thread-unsafe exceptions..."
#define NO_LIBGCC_HOOKS
#endif

#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>


// -- context switch

struct mach_state { 
  jmp_buf buf; 
};

static void
mach_switch(mach_state *st1, mach_state *st2)
{ 
#if #cpu(sparc)
  asm("ta 3"); // save register windows
#endif
  if (! setjmp(st1->buf))
    longjmp(st2->buf, 1);
}

static void 
mach_start(mach_state *st1, void *pc, char *stacklo, char *stackhi)
{ 
#if #cpu(sparc)
  asm("ta 3"); // save register windows
#endif
  if (! setjmp(st1->buf))
    {
      // The following code must perform two tasks:
      // -- set stack pointer to a proper value between #stacklo# and #stackhi#.
      // -- branch to or call the function at address #pc#.
      // This function never returns ... so there is no need to save anything
#if #cpu(mips)
      char *sp = (char*)(((unsigned long)stackhi-16) & ~0xff);
      asm volatile ("move $sp,%0\n\t"  // set new stack pointer
                    "move $25,%1\n\t"  // call subroutine via $25
                    "jal  $25\n\t"     // call subroutine via $25
                    "nop"              // delay slot
                    : : "r" (sp), "r" (pc) );
#elif #cpu(i386)
      char *sp = (char*)(((unsigned long)stackhi-16) & ~0xff);
      asm volatile ("movl %0,%%esp\n\t" // set new stack pointer
                    "call *%1"          // call function
                    : : "r" (sp), "r" (pc) );
#elif #cpu(sparc)
      char *sp = (char*)(((unsigned long)stackhi-16) & ~0xff);
      asm volatile ("mov %0,%%sp\n\t"   // set new stack pointer
                    "call %1,0\n\t"     // call function
                    "nop"               // delay slot
                    : : "r" (sp), "r" (pc) );
#elif #cpu(hppa)
      char *sp = (char*)(((unsigned long)stacklo+127+256) & ~0xff);
      asm volatile("copy %0,%%sp\n\t"       // set stack pointer
                   "copy %1,%%r22\n\t"      // set call address
                   ".CALL\n\t"              // call 
                   "bl $$dyncall,%%r31\n\t" // call 
                   "nop"                    // delay slot ???
                   : : "r" (sp), "r" (pc) );
#elif #cpu(alpha)
      char *sp = (char*)(((unsigned long)stackhi-16) & ~0xff);
      asm volatile ("bis $31,%0,$30\n\t"  // set new stack pointer
                    "bis $31,%1,$27\n\t"  // load function pointer
                    "jsr $26,($27),0"     // call function
                    : : "r" (sp), "r" (pc) );
#elif #cpu(powerpc)
      char *sp = (char*)(((unsigned long)stackhi-64) & ~0xff);
      asm volatile ("mr 1,%0\n\t"         // set new stack pointer
                    "mr 0,%1\n\t"         // load func pointer into r0
                    "mtlr 0\n\t"          // load link register with r0
                    "blrl"                // branch
                    : : "r" (sp), "r" (pc) );
#elif #cpu(m68k) && defined(COTHREAD_UNTESTED)
      char *sp = (char*)(((unsigned long)stackhi-16) & ~0xff);
      asm volatile ("move%.l %0,%Rsp\n\t" // set new stack pointer
                    "jmp %a1"             // branch to address %1
                    : : "r" (sp), "a" (pc) );
#elif #cpu(arm) && defined(COTHREAD_UNTESTED)
      char *sp = (char*)(((unsigned long)stackhi-64) & ~0xff);
      asm volatile ("mov%?\t%|sp, %0\n\t" // set new stack pointer
                    "mov%?\t%|pc, %1"     // branch to address %1
                    : : "r" (sp), "r" (pc) );
#else
#error "COTHREADS not supported on this machine."
#error "Try -DTHREADMODEL=NOTHREADS."
#endif
      // We should never reach this point
      abort();
      // Note that this call to abort() makes sure
      // that function mach_start() is compiled as a non-leaf
      // function. It is indeed a non-leaf function since the
      // piece of assembly code calls a function, but the compiler
      // would not know without the call to abort() ...
    }
}

#ifdef CHECK
// This code can be used to verify that task switching works.
char stack[16384];
mach_state st1, st2;
void th2() {
  puts("2b"); mach_switch(&st2, &st1);
  puts("4b"); mach_switch(&st2, &st1);
  puts("6b"); mach_switch(&st2, &st1);
}
void th2relay() {
  th2(); puts("ooops\n");
}
void th1() {
  mach_start(&st1, (void*)th2relay, stack, stack+sizeof(stack));
  puts("3a"); mach_switch(&st1, &st2);
  puts("5a"); mach_switch(&st1, &st2);
}
int main() { 
  puts("1a"); th1(); puts("6a"); 
}
#endif


// -- data structures

struct cotask {
  struct cotask *next;
  struct cotask *prev;
  // context
  mach_state regs;
  // stack information
  char *stack;
  int stacksize;
  // egcs exception support
  void *ehctx;
  // waiting on event
  int *wchan;
  // waiting on select
  int nfds;
  fd_set *rdfds;
  fd_set *wrfds;
  fd_set *exfds;
  // waiting timeout
  struct timeval wakup;
};

static cotask *maintask = 0;
static cotask *curtask = 0;

// Minimum and default stack size
#define MINSTACK (32*1024)
#define DEFSTACK (127*1024)

#ifndef NO_LIBGCC_HOOKS
  // These are exported by Leon's patched version of libgcc.a
  // Let's hope that the egcs people will include the patch in
  // the distributions.
extern "C" 
{
  extern void* (*__get_eh_context_ptr)(void);
  extern void* __new_eh_context(void);
}
#endif




// -- cotask support

static int
cotask_switch(cotask *thr)
{
  if (thr != curtask)
    {
      cotask *old = curtask;
      curtask = thr;
      curtask->wchan = 0;
      mach_switch(&old->regs, &curtask->regs);
      return 0;
    }
  return 1;
}

static void
cotask_release_all(int *wchan)
{
  int again = 0;
  if (maintask && curtask)
    again = 1;
  while (again)
    {
      again = 0;
      cotask *p = curtask;
      do {
        if (p->wchan == wchan)
          {
            cotask_switch(p);
            again = 1;
            break;
          }
        p = p->next;
      } while (p != curtask);
    }
}

#ifndef NO_LIBGCC_HOOKS
// This function is called via the pointer __get_eh_context_ptr
// by the internal mechanisms of egcs.  It must return the 
// per-thread event handler context.  This is necessary to
// implement thread safe exceptions on some machine and/or 
// when flag -fsjlj-exception is set.
static void *
cotask_get_eh_context()
{
  if (curtask)
    return curtask->ehctx;
  else if (maintask)
    return maintask->ehctx;
  fprintf(stderr, "Panic COTHREADS : exception context not found\n");
  abort();
}
#endif

static int
tmcmp(const struct timeval *tp1, const struct timeval *tp2)
{
  if (tp1->tv_sec < tp2->tv_sec)
    return -1;
  if (tp1->tv_sec > tp2->tv_sec)
    return  1;
  if (tp1->tv_usec < tp2->tv_usec)
    return -1;
  if (tp1->tv_usec > tp2->tv_usec)
    return  1;
  return 0;
}

static void
tmadd(struct timeval *tp1, const struct timeval *tp2)
{
  tp1->tv_sec += tp2->tv_sec;
  tp1->tv_usec += tp2->tv_usec;
  if (tp1->tv_usec > 1000000)
    {
      tp1->tv_usec -= 1000000;
      tp1->tv_sec += 1;
    }
}

static void
tmsub(struct timeval *tp1, const struct timeval *tp2)
{
  if (tp1->tv_usec < tp2->tv_usec)
    {
      tp1->tv_usec += 1000000;
      tp1->tv_sec -= 1;
    }
  tp1->tv_usec -= tp2->tv_usec;
  tp1->tv_sec -= tp2->tv_sec;
}

static struct timeval tmzero = {0,0};

static void
fdadd(fd_set *set1, fd_set *set2, int nfds)
{
  if (set2)
    for (int i=0; i<nfds; i++)
      if (FD_ISSET(i, set2))
        FD_SET(i, set1);
}

static int 
pollselect(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds)
{
  fd_set copyr; FD_ZERO(&copyr); if (rfds) copyr = *rfds;
  fd_set copyw; FD_ZERO(&copyw); if (wfds) copyw = *wfds;
  fd_set copye; FD_ZERO(&copye); if (efds) copye = *efds;
  return ::select(nfds, &copyr, &copyw, &copye, &tmzero);
}



// -- GThread

static void (*scheduling_callback)(int) = 0;

void 
GThread::set_scheduling_callback(void (*call)(int))
{
  if (scheduling_callback)
    THROW("GThread::set_scheduling_callback called twice");
  scheduling_callback = call;
}


GThread::GThread(int stacksize)
  : task(0), xentry(0), xarg(0)
{
  // check argument
  if (stacksize < 0)
    stacksize = DEFSTACK;
  if (stacksize < MINSTACK)
    stacksize = MINSTACK;
  // initialization
  if (! maintask)
    {
      static cotask comaintask;
      maintask = &comaintask;
      memset(maintask, 0, sizeof(cotask));
      maintask->next = maintask;
      maintask->prev = maintask;
#ifndef NO_LIBGCC_HOOKS
      maintask->ehctx =  (*__get_eh_context_ptr)();
      __get_eh_context_ptr = cotask_get_eh_context;
#endif
      curtask = maintask;
    }
  // allocation
  task = new cotask;
  memset(task, 0, sizeof(cotask));
  task->stacksize = stacksize;
  task->stack = new char[stacksize];
#ifndef NO_LIBGCC_HOOKS
  task->ehctx = __new_eh_context();
#endif
}

GThread::~GThread()
{
  if (task==0 || task==maintask)
    return;
  if (task->next && task->prev)
    terminate();
  if (task->stack) 
    delete [] task->stack;
  task->stack = 0;
  if (task->ehctx) 
    free(task->ehctx);
  task->ehctx = 0;
  delete task;
  task = 0;
}

static GThread *starter;
void GThread::start(void)
{
  GThread *thr = starter;
  mach_switch(&starter->task->regs, &curtask->regs);
#ifdef __EXCEPTIONS
  try 
    {
#endif 
      TRY
        {
          thr->xentry( thr->xarg );
        }
      CATCH(ex)
        {
          ex.perror();
          fprintf(stderr, "GThreads: uncaught exception.");
          abort();
        }
      ENDCATCH;
#ifdef __EXCEPTIONS
    }
  catch(...)
    {
      fprintf(stderr, "GThreads: unrecognized uncaught exception.");
      abort();
    }
#endif 
  thr->terminate();
  GThread::yield();
  abort();
}

int 
GThread::create(void (*entry)(void*), void *arg)
{
  if (task->prev)
    return -1;
  xentry = entry;
  xarg = arg;
  task->wchan = 0;
  task->next = curtask;
  task->prev = curtask->prev;
  task->next->prev = task;
  task->prev->next = task;
  cotask *old = curtask;
  starter = this;
  mach_start(&old->regs, 
             (void*)GThread::start, 
             task->stack, task->stack+task->stacksize);
  if (scheduling_callback)
    (*scheduling_callback)(0);
  return 0;
}

void 
GThread::terminate()
{
  if (!task)
    return;
  if (task==maintask)
    abort();
  if (task->next)
    {
      task->prev->next = task->next;
      task->next->prev = task->prev;
      task->next = 0;
      if (task == curtask)
        yield();
    }
}

int
GThread::yield()
{
  // Before initializing
  if (! maintask)
    return -1;
  // Initialize variables
  cotask *next = curtask->next;
  if (! next)
    // This is happening when a cotask calls terminate() itself
    next = curtask->prev->next;
  // Scheduling loop
 reschedule:
  struct timeval cur;
  cur.tv_sec = 0;
  cur.tv_usec = 0;
  cotask *p = next;
  do 
    {
      if (p->wchan == 0)
        return cotask_switch(p);
      // check if waiting event set
      if (*p->wchan > 0)
        return cotask_switch(p);
      // check if waiting timeout elapsed
      if (p->wakup.tv_sec > 0)
        {
          if (cur.tv_sec==0)
            gettimeofday(&cur, NULL);
          if (tmcmp(&cur, &p->wakup) >= 0)
            return cotask_switch(p);
        }
      // check if waiting file descriptors ready
      if (p->nfds > 0)
        if (pollselect(p->nfds, p->rdfds, p->wrfds, p->exfds))
          return cotask_switch(p);            
      p = p->next;
    }
  while (p != next);
  // all tasks are waiting
  struct timeval wakup;
  cotask *waiter = 0 ;
  fd_set rdfds, wrfds, exfds;
  int nfds = 0;
  FD_ZERO(&rdfds);
  FD_ZERO(&wrfds);
  FD_ZERO(&exfds);
  // loop over task
  p = next;
  do 
    {
      // compute union of waiting file descriptors
      if (p->wchan && p->nfds>0)
        {
          if (p->nfds > nfds)
            nfds = p->nfds;
          fdadd(&rdfds, p->rdfds, nfds);
          fdadd(&wrfds, p->wrfds, nfds);
          fdadd(&exfds, p->exfds, nfds);
        }
      // compute earliest wakup time
      if (p->wchan && p->wakup.tv_sec>0)
        if (waiter==0 || tmcmp(&p->wakup, &wakup) <= 0) 
          {
            waiter = p;
            wakup = p->wakup;
          }
      // next task
      p = p->next;
    } 
  while (p!=next);
  // check for deadlock
  if (waiter==0 && nfds==0)
    {
      fprintf(stderr,"panic: COTHREADS deadlock\n");
      abort();
    }
  // check if timeout has expired
  if (waiter)
    {
      gettimeofday(&cur, NULL);
      if (tmcmp(&wakup, &cur) <= 0)
        goto reschedule;
      tmsub(&wakup, &cur);
    }
  // whole process waits.
  ::select(nfds, &rdfds, &wrfds, &exfds, (waiter ? &wakup : 0));
  goto reschedule;
}


void*
GThread::current()
{
  if (curtask && curtask!=maintask)
    return (void*)curtask;
  return (void*)0;
}

int 
GThread::select(int nfds, 
                fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                struct timeval *timeout)
{
  if (! maintask)
    return ::select(nfds, readfds, writefds, exceptfds, timeout);
  // Compute wakup date
  struct timeval wakup = tmzero; 
  if (timeout)
    {
      gettimeofday(&wakup, NULL);
      tmadd(&wakup, timeout);
    }
  // Check if data is ready now
  if (pollselect(nfds, readfds, writefds, exceptfds) == 0)
    {
      // Wait
      int event = 0;
      curtask->nfds = nfds;
      curtask->rdfds = readfds;
      curtask->wrfds = writefds;
      curtask->exfds = exceptfds;
      curtask->wakup = wakup;
      curtask->wchan = &event;
      yield();
    }
  // Compute remaining waiting time (as select does)
  if (timeout)
    {
      *timeout = wakup;
      gettimeofday(&wakup, 0);
      if (tmcmp(timeout, &wakup) > 0)
        tmsub(timeout, &wakup);
      else
        *timeout = tmzero;
    }
  // Run the real select in order to modify the masks.
  return ::select(nfds, readfds, writefds, exceptfds, &tmzero);
}

// -- GCriticalSection

GCriticalSection::GCriticalSection() 
  : count(1), locker(0) 
{	
  ok = 1;
}

GCriticalSection::~GCriticalSection()
{	
  ok = 0;
  cotask_release_all(&count);
}

void 
GCriticalSection::lock() 
{
  if (count>0) {
    count = 0;
    locker = curtask;
  } else if (locker==curtask) {
    count -= 1;
  } else if (ok) {
    curtask->nfds = 0;
    curtask->wakup.tv_sec = 0;
    curtask->wchan = &count;
    GThread::yield();
    count = 0;
    locker = curtask;
  }
}

void 
GCriticalSection::unlock() 
{
  if (locker == curtask)
    {
      count += 1;
      if (count>0)
        {
          locker = 0;
          if (scheduling_callback)
            (*scheduling_callback)(1);
        }
    }
}


// -- GEvent

GEvent::GEvent() 
  : status(0) 
{
  ok = 1;
}

GEvent::~GEvent() 
{
  ok = 0;
  cotask_release_all(&status);
}

void
GEvent::set() 
{
  status = 1;
  if (scheduling_callback)
    (*scheduling_callback)(2);
}

void
GEvent::wait()
{
  if (ok && status<=0)
    {
      curtask->nfds = 0;
      curtask->wakup.tv_sec = 0;
      curtask->wchan = &status;
      GThread::yield();
    }
  status = 0;
}

void
GEvent::wait(int timeout) 
{
  if (ok && status<=0) 
    {
      gettimeofday(&curtask->wakup, NULL);
      curtask->wakup.tv_sec += timeout/1000;
      curtask->wakup.tv_usec += (timeout%1000)*1000;
      if (curtask->wakup.tv_usec > 1000000) 
        {
          curtask->wakup.tv_usec -= 1000000;
          curtask->wakup.tv_sec += 1;
        }
      curtask->nfds = 0;
      curtask->wchan = &status;
      GThread::yield();
    }
  status = 0;
}

#endif

