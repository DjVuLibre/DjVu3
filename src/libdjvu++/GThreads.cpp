//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.  All rights reserved.
//C-
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet website having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: GThreads.cpp,v 1.27 1999-07-19 14:25:27 leonb Exp $


// **** File "$Id: GThreads.cpp,v 1.27 1999-07-19 14:25:27 leonb Exp $"
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

void *
GThread::current()
{
  return (void*) GetCurrentThreadId();
}

GMonitor::GMonitor()
  : ok(0), count(1)
{
  InitializeCriticalSection(&cs);
  hev[0] = CreateEvent(NULL,FALSE,FALSE,NULL);
  hev[1] = CreateEvent(NULL,TRUE,FALSE,NULL);
  locker = GetCurrentThreadId();
  ok = 1;
}

GMonitor::~GMonitor()
{
  ok = 0;
  DeleteCriticalSection(&cs); 
  CloseHandle(hev[0]);
  CloseHandle(hev[1]);
}

void 
GMonitor::enter()
{
  DWORD self = GetCurrentThreadId();
  if (count>0 || self!=locker)
    {
      if (ok)
        EnterCriticalSection(&cs);
      locker = self;
      count = 1;
    }
  count -= 1;
}

void 
GMonitor::leave()
{
  DWORD self = GetCurrentThreadId();
  if (ok && (count>0 || self!=locker))
    THROW("Monitor was not acquired by this thread (GMonitor::broadcast)");
  count += 1;
  if (count > 0)
    {
      count = 1;
      if (ok)
        LeaveCriticalSection(&cs);
    }
}

void
GMonitor::signal()
{
  if (ok)
    {
      DWORD self = GetCurrentThreadId();
      if (count>0 || self!=locker)
        THROW("Monitor was not acquired by this thread (GMonitor::signal)");
      SetEvent(hev[0]);
    }
}

void
GMonitor::broadcast()
{
  if (ok)
    {
      DWORD self = GetCurrentThreadId();
      if (count>0 || self!=locker)
        THROW("Monitor was not acquired by this thread (GMonitor::broadcast)");
      SetEvent(hev[1]);
    }
}

void
GMonitor::wait()
{
  // Check state
  DWORD self = GetCurrentThreadId();
  if (count>0 || self!=locker)
    THROW("Monitor was not acquired by this thread (GMonitor::wait)");
  // Wait
  if (ok)
    {
      // Release
      int sav_count = count;
      count = 1;
      ResetEvent(hev[0]);
      ResetEvent(hev[1]);
      // Wait
      LeaveCriticalSection(&cs);
      WaitForMultipleObjects(2,hev,FALSE,INFINITE);
      // Re-acquire
      EnterCriticalSection(&cs);
      count = sav_count;
      locker = self;
    }
}

void
GMonitor::wait(unsigned long timeout) 
{
  // Check state
  DWORD self = GetCurrentThreadId();
  if (count>0 || self!=locker)
    THROW("Monitor was not acquired by this thread (GMonitor::wait)");
  // Wait
  if (ok)
    {
      // Release
      int sav_count = count;
      count = 1;
      ResetEvent(hev[0]);
      ResetEvent(hev[1]);
      // Wait
      LeaveCriticalSection(&cs);
      WaitForMultipleObjects(2,hev,FALSE,timeout);
      // Re-acquire
      EnterCriticalSection(&cs);
      count = sav_count;
      locker = self;
    }
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

// GMonitor is not implemented (highly suspicious)
inline GMonitor::GMonitor() {}
inline GMonitor::~GMonitor() {}
inline void GMonitor::enter() {}
inline void GMonitor::leave() {}
inline void GMonitor::wait() {}
inline void GMonitor::wait(unsigned long timeout) {}
inline void GMonitor::signal() {}
inline void GMonitor::broadcast() {}

#endif



// ----------------------------------------
// POSIXTHREADS IMPLEMENTATION
// ----------------------------------------

#if THREADMODEL==POSIXTHREADS

#if defined(CMA_INCLUDE) || defined(pthread_attr_default)
#define DCETHREADS
#define pthread_key_create pthread_keycreate
#else
#define pthread_mutexattr_default  NULL
#define pthread_condattr_default   NULL
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
  if (xentry || xarg)
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

// -- GMonitor

GMonitor::GMonitor()
  : ok(0), count(1), locker(0)
{
  pthread_mutex_init(&mutex, pthread_mutexattr_default);
  pthread_cond_init(&cond, pthread_condattr_default); 
  locker = pthread_self();
  ok = 1;
}

GMonitor::~GMonitor()
{
  ok = 0;
  pthread_cond_destroy(&cond);
  pthread_mutex_destroy(&mutex); 
}


void 
GMonitor::enter()
{
  pthread_t self = pthread_self();
  if (count>0 || !pthread_equal(locker, self))
    {
      if (ok)
        pthread_mutex_lock(&mutex);
      locker = self;
      count = 1;
    }
  count -= 1;
}

void 
GMonitor::leave()
{
  pthread_t self = pthread_self();
  if (ok && (count>0 || !pthread_equal(locker, self)))
    THROW("Monitor was not acquired by this thread (GMonitor::broadcast)");
  count += 1;
  if (count > 0)
    {
      count = 1;
      if (ok)
        pthread_mutex_unlock(&mutex);
    }
}

void
GMonitor::signal()
{
  if (ok)
    {
      pthread_t self = pthread_self();
      if (count>0 || !pthread_equal(locker, self))
        THROW("Monitor was not acquired by this thread (GMonitor::signal)");
      pthread_cond_signal(&cond);
    }
}

void
GMonitor::broadcast()
{
  if (ok)
    {
      pthread_t self = pthread_self();
      if (count>0 || !pthread_equal(locker, self))
        THROW("Monitor was not acquired by this thread (GMonitor::broadcast)");
      pthread_cond_broadcast(&cond);
    }
}

void
GMonitor::wait()
{
  // Check
  pthread_t self = pthread_self();
  if (count>0 || !pthread_equal(locker, self))
    THROW("Monitor was not acquired by this thread (GMonitor::wait)");
  // Wait
  if (ok)
    {
      // Release
      int sav_count = count;
      count = 1;
      // Wait
      pthread_cond_wait(&cond, &mutex);
      // Re-acquire
      count = sav_count;
      locker = self;
    }      
}

void
GMonitor::wait(unsigned long timeout) 
{
  // Check
  pthread_t self = pthread_self();
  if (count>0 || !pthread_equal(locker, self))
    THROW("Monitor was not acquired by this thread (GMonitor::wait)");
  // Wait
  if (ok)
    {
      // Release
      int sav_count = count;
      count = 1;
      // Wait
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
      // Re-acquire
      count = sav_count;
      locker = self;
    }      
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


// -------------------------------------- constants

// Minimal stack size
#define MINSTACK   (32*1024)
// Default stack size
#define DEFSTACK   (127*1024)
// Maxtime between checking fdesc (ms)
#define MAXFDWAIT    (200)
// Maximum time to wait in any case
#define MAXWAIT (60*60*1000)
// Maximum penalty for hog task (ms)
#define MAXPENALTY (1000)
// Trace task switches
#undef COTHREAD_TRACE

// -------------------------------------- context switch code

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
      char *sp = (char*)(((unsigned long)stacklo+128+255) & ~0xff);
      asm volatile("copy %0,%%sp\n\t"       // set stack pointer
                   "copy %1,%%r22\n\t"      // set call address
                   ".CALL\n\t"              // call pseudo instr (why?)
                   "bl $$dyncall,%%r31\n\t" // call 
                   "copy %%r31,%%r2"        // delay slot ???
                   : : "r" (sp), "r" (pc) );
#elif #cpu(alpha)
      char *sp = (char*)(((unsigned long)stackhi-16) & ~0xff);
      asm volatile ("bis $31,%0,$30\n\t"  // set new stack pointer
                    "bis $31,%1,$27\n\t"  // load function pointer
                    "jsr $26,($27),0"     // call function
                    : : "r" (sp), "r" (pc) );
#elif #cpu(powerpc)
      char *sp = (char*)(((unsigned long)stackhi-16) & ~0xff);
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
      char *sp = (char*)(((unsigned long)stackhi-16) & ~0xff);
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



// -------------------------------------- select

struct coselect {
  int nfds;
  fd_set rset;
  fd_set wset;
  fd_set eset;
};

static void 
coselect_merge(coselect *dest, coselect *from)
{
  int i;
  int nfds = from->nfds;
  if (nfds > dest->nfds)
    dest->nfds = nfds;
  for (i=0; i<nfds; i++) if (FD_ISSET(i, &from->rset)) FD_SET(i, &dest->rset);
  for (i=0; i<nfds; i++) if (FD_ISSET(i, &from->wset)) FD_SET(i, &dest->wset);
  for (i=0; i<nfds; i++) if (FD_ISSET(i, &from->eset)) FD_SET(i, &dest->eset);
}

static int
coselect_test(coselect *c)
{
  static timeval tmzero = {0,0};
  fd_set copyr = c->rset;
  fd_set copyw = c->wset;
  fd_set copye = c->eset;
  return select(c->nfds, &copyr, &copyw, &copye, &tmzero);
}


// -------------------------------------- cotask

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
  // timing information
  unsigned long over;
  // waiting information
  void *wchan;
  coselect *wselect;
  unsigned long *maxwait;
};

static cotask *maintask = 0;
static cotask *curtask  = 0;
static unsigned long globalmaxwait = 0;

// Hmmm. Waiting tasks should really be in a separate list


// -------------------------------------- time

static timeval time_base;

static unsigned long
time_elapsed(int reset=1)
{
  timeval tm;
  gettimeofday(&tm, NULL);
  long msec = (tm.tv_usec-time_base.tv_usec)/1000;
  unsigned long elapsed = (long)(tm.tv_sec-time_base.tv_sec)*1000 + msec;
  if (reset && elapsed>0)
    {
#ifdef COTHREAD_TRACE
      fprintf(stderr,"cothreads: %4ld ms in task %p\n", elapsed, curtask);
#endif
      time_base.tv_sec = tm.tv_sec;
      time_base.tv_usec += msec*1000;
    }
  return elapsed;
}

// -------------------------------------- scheduler

static int
cotask_yield()
{
  // ok
  if (! maintask)
    return 0;
  // get elapsed time and return immediately when it is too small
  unsigned long elapsed = time_elapsed();
  if (elapsed==0 && curtask->wchan==0 && curtask->prev && curtask->next)
    return 0;
  // adjust task running time
  curtask->over += elapsed;
  if (curtask->over > MAXPENALTY)
    curtask->over = MAXPENALTY;
  // start scheduling
 reschedule:
  // try unblocking tasks
  cotask *n = curtask->next;
  cotask *q = n;
  do 
    { 
      if (q->wchan)
        {
	  if (q->maxwait && *q->maxwait<=elapsed) 
            {
              *q->maxwait = 0;
              q->wchan=0; 
              q->maxwait=0; 
              q->wselect=0; 
            }
          else if (q->wselect && globalmaxwait<=elapsed && coselect_test(q->wselect))
            {
              q->wchan=0;
              if (q->maxwait)
                *q->maxwait -= elapsed;
              q->maxwait = 0; 
              q->wselect=0; 
            }
          if (q->maxwait)
            *q->maxwait -= elapsed;
        }
      q = q->next;
    } 
  while (q!=n);
  // adjust globalmaxwait
  if (globalmaxwait < elapsed)
    globalmaxwait = MAXFDWAIT;
  else
    globalmaxwait -= elapsed;
  // find best candidate
  static int count;
  unsigned long best = MAXPENALTY + 1;
  cotask *r = 0;
  count = 0;
  q = n;
  do 
    { 
      if (! q->wchan)
        {
          count += 1;
          if (best > q->over)
            {
              r = q;
              best = r->over;
            } 
        }
      q = q->next;
    } 
  while (q != n);
  // found
  if (count > 0)
    {
      // adjust over 
      q = n;
      do 
        { 
          q->over = (q->over>best ? q->over-best : 0);
          q = q->next;
        } 
      while (q != n);
      // Switch
      if (r != curtask)
        {
#ifdef COTHREAD_TRACE
          fprintf(stderr,"cothreads: ----- switch to %p [%ld]\n", r, best);
#endif
          cotask *old = curtask;
          curtask = r;
          mach_switch(&old->regs, &curtask->regs);
        }
      // return 
      if (count == 1)
        return 1;
      return 0;
    }
  // No task ready
  count = 0;
  unsigned long minwait = MAXWAIT;
  coselect allfds;
  allfds.nfds = 1;
  FD_ZERO(&allfds.rset);
  FD_ZERO(&allfds.wset);
  FD_ZERO(&allfds.eset);
  q = n;
  do 
    {
      if (q->maxwait || q->wselect)
        count += 1;
      if (q->maxwait && *q->maxwait<minwait)
        minwait = *q->maxwait;
      if (q->wselect)
        coselect_merge(&allfds, q->wselect);
      q = q->next;
    } 
  while (q != n);
  // abort on deadlock
  if (count == 0) {
    fprintf(stderr,"PANIC: Cothreads deadlock\n");
    abort();
  }
  // select
  timeval tm;
  tm.tv_sec = minwait/1000;
  tm.tv_usec = 1000*(minwait-1000*tm.tv_sec);
  select(allfds.nfds,&allfds.rset, &allfds.wset, &allfds.eset, &tm);
  // reschedule
  globalmaxwait = 0;
  elapsed = time_elapsed();
  goto reschedule;
}



// -------------------------------------- select / get_select

static int
cotask_select(int nfds, 
              fd_set *rfds, fd_set *wfds, fd_set *efds,
              struct timeval *tm)
{
  // bypass
  if (maintask==0 || (tm && tm->tv_sec==0 && tm->tv_usec<1000))
    return select(nfds, rfds, wfds, efds, tm);
  // copy parameters
  unsigned long maxwait = 0;
  coselect parm;
  // set waiting info
  curtask->wchan = (void*)&parm;
  if (rfds || wfds || efds)
    {
      parm.nfds = nfds;
      if (rfds) { parm.rset=*rfds; } else { FD_ZERO(&parm.rset); }
      if (wfds) { parm.wset=*wfds; } else { FD_ZERO(&parm.wset); }
      if (efds) { parm.eset=*efds; } else { FD_ZERO(&parm.eset); }
      curtask->wselect = &parm;
    }
  if (tm) 
    {
      maxwait = time_elapsed(0) + tm->tv_sec*1000 + tm->tv_usec/1000;
      curtask->maxwait = &maxwait;
    }
  // reschedule
  cotask_yield();
  // call select to update masks
  if (tm)
    {
      tm->tv_sec = maxwait/1000;
      tm->tv_usec = 1000*(maxwait-1000*tm->tv_sec);
    }
  static timeval tmzero = {0,0};
  return select(nfds, rfds, wfds, efds, &tmzero);
}


void 
GThread::get_select(int &nfds,
                    fd_set *rfds, fd_set *wfds, fd_set *efds, 
                    unsigned long &timeout)
{
  int ready = 1;
  unsigned long minwait = MAXWAIT;
  unsigned long elapsed = time_elapsed(0);
  coselect allfds;
  allfds.nfds=0;
  FD_ZERO(&allfds.rset);
  FD_ZERO(&allfds.wset);
  FD_ZERO(&allfds.eset);
  if (curtask)
    {
      cotask *q=curtask->next;
      while (q != curtask)
        {
          ready++;
          if (q->wchan)
            {
              if (q->wselect) 
                coselect_merge(&allfds, q->wselect);
              if (q->maxwait && *q->maxwait<minwait)
                minwait = *q->maxwait;
              ready--;
            }
          q = q->next;
        }
    }
  timeout = 0;
  nfds=allfds.nfds;
  *rfds=allfds.rset;
  *wfds=allfds.wset;
  *efds=allfds.eset;
  if (ready==1 && minwait>elapsed)
    timeout = minwait-elapsed;
}



// -------------------------------------- utilities

static void
cotask_wakeup(void *wchan, int onlyone)
{
  if (maintask && curtask)
    {
      cotask *n = curtask->next;
      cotask *q = n;
      do 
        { 
          if (q->wchan == wchan)
            {
              q->wchan=0; 
              q->maxwait=0; 
              q->wselect=0; 
              q->over = 0;
	      if (onlyone)
		return;
            }
          q = q->next;
        } 
      while (q!=n);
    }
}



// -------------------------------------- libgcc hook

#ifndef NO_LIBGCC_HOOKS
// These are exported by Leon's patched version of libgcc.a
// Let's hope that the egcs people will include the patch in
// the distributions.
extern "C" 
{
  extern void* (*__get_eh_context_ptr)(void);
  extern void* __new_eh_context(void);
}

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



// -------------------------------------- GThread

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
      gettimeofday(&time_base,NULL);
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

static void 
starttwo(GThread *thr)
{
  // Hopefully this function reacquires 
  // an exception context pointer. Therefore
  // we can register the exception handlers.
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

static GThread * volatile starter;

static void
startone(void)
{
  GThread *thr = starter;
  mach_switch(&thr->task->regs, &curtask->regs);
  // Registers may still contain an improper pointer
  // to the exception context.  We should neither 
  // register cleanups nor register handlers.
  starttwo(thr);
  abort();
}

int 
GThread::create(void (*entry)(void*), void *arg)
{
  if (task->next || task->prev)
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
  mach_start(&old->regs, (void*)startone, 
             task->stack, task->stack+task->stacksize);
  if (scheduling_callback)
    (*scheduling_callback)(CallbackCreate);
  return 0;
}

void 
GThread::terminate()
{
  if (!task)
    return;
  if (task==maintask)
    abort();
  if (task->prev && task->next)
    {
      if (scheduling_callback)
        (*scheduling_callback)(CallbackTerminate);
      task->prev->next = task->next;
      task->next->prev = task->prev;
      task->prev = 0;
      if (task == curtask)
        cotask_yield();
    }
}

int
GThread::yield()
{
  return cotask_yield();
}

int 
GThread::select(int nfds, 
                fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                struct timeval *timeout)
{
  return cotask_select(nfds, readfds, writefds, exceptfds, timeout);
}

inline void *
GThread::current()
{
  if (curtask && curtask!=maintask)
    return (void*)curtask;
  return (void*)0;
}


// -------------------------------------- GMonitor

GMonitor::GMonitor()
  : count(1), locker(0)
{
  locker = 0;
  ok = 1;
}

GMonitor::~GMonitor()
{
  ok = 0;
  cotask_wakeup((void*)this, 0);
  cotask_wakeup((void*)count, 0);    
  cotask_yield();
}

void 
GMonitor::enter()
{
  void *self = GThread::current();
  if (count>0 || self!=locker)
    {
      while (ok && count<=0)
        {
          curtask->wchan = (void*)&count;
          cotask_yield();
        }
      count = 1;
      locker = self;
    }
  count -= 1;
}

void 
GMonitor::leave()
{
  void *self = GThread::current();
  if (ok && (count>0 || self!=locker))
    THROW("Monitor was not acquired by this thread (GMonitor::leave)");
  if (++count > 0)
    cotask_wakeup((void*)&count, 1);
}

void
GMonitor::signal()
{
  void *self = GThread::current();
  if (count>0 || self!=locker)
    THROW("Monitor was not acquired by this thread (GMonitor::signal)");
  cotask_wakeup((void*)this, 1);
  if (scheduling_callback)
    (*scheduling_callback)(GThread::CallbackUnblock);
}

void
GMonitor::broadcast()
{
  void *self = GThread::current();
  if (count>0 || self!=locker)
    THROW("Monitor was not acquired by this thread (GMonitor::broadcast)");
  cotask_wakeup((void*)this, 0);
  if (scheduling_callback)
    (*scheduling_callback)(GThread::CallbackUnblock);
}

void
GMonitor::wait()
{
  // Check state
  void *self = GThread::current();
  if (count>0 || locker!=self)
    THROW("Monitor was not acquired by this thread (GMonitor::wait)");
  // Wait
  if (ok)
    {
      // Release
      int sav_count = count;
      count = 1;
      // Wait
      curtask->wchan = (void*)this;
      cotask_yield();
      // Re-acquire
      while (ok && count <= 0)
        {
          curtask->wchan = &count;
          cotask_yield();
        }
      count = sav_count;
      locker = self;
    }
}

void
GMonitor::wait(unsigned long timeout) 
{
  // Check state
  void *self = GThread::current();
  if (count>0 || locker!=self)
    THROW("Monitor was not acquired by this thread (GMonitor::wait)");
  // Wait
  if (ok)
    {
      // Release
      int sav_count = count;
      count = 1;
      // Wait
      unsigned long maxwait = time_elapsed(0) + timeout;
      curtask->maxwait = &maxwait;
      curtask->wchan = (void*)this;
      cotask_yield();
      // Re-acquire
      while (ok && count<=0)
        {
          curtask->wchan = &count;
          cotask_yield();
        }
      count = sav_count;
      locker = self;
    }
}

#endif

