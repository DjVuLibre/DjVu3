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
//C- $Id: TestException.cpp,v 1.6 1999-03-17 19:25:00 leonb Exp $


#include <stdio.h>
#include "GException.h"
#include "GString.h"


#ifdef _MSC_VER
#include <io.h>
#endif

int
main()
{
#ifdef _MSC_VER
  // Redirect STDERR on STDOUT
  fflush(NULL);
  close(2);
  _dup2(1,2);
#endif

  TRY 
    {
      int a = 2;
      int b = 2+a;
      printf("a=%d, b=%d, c=%d\n",a,b,a+b);
    } 
  CATCH(ex) 
    {
      fprintf(stderr,"*** %s\n", ex.get_cause());
    }
  ENDCATCH;
  

  TRY 
    {
      int a = 2;
      int b = 2+a;
      printf("a=%d, b=%d, c=%d\n",a,b,a+b);
      THROW("Test exception");
    }
  CATCH(ex)
    {
      fprintf(stderr,"*** %s\n", ex.get_cause());
    }
  ENDCATCH;

  TRY
    {
      GString gs = "abcdef";
      printf("gs[0]=%c\n",gs[0]);
      printf("gs[-1]=%c\n",gs[-1]);
      printf("gs[-12]=%c\n",gs[-12]);
    }
  CATCH(ex)
    {
      fprintf(stderr,"*** %s\n", ex.get_cause());
    }
  ENDCATCH;


  TRY
    {
      TRY
        {
          GString gs = "abcdef";
          printf("gs[0]=%c\n",gs[0]);
          printf("gs[-1]=%c\n",gs[-1]);
          printf("gs[-12]=%c\n",gs[-12]);
        }
      CATCH(ex)
        {
          fprintf(stderr,"*** %s\n", ex.get_cause());
          printf("Rethrown\n");
          RETHROW;
        }
      ENDCATCH;
    }
  CATCH(ex)
    {
      printf("Recatched\n");
      fprintf(stderr,"*** %s\n", ex.get_cause());
    }
  ENDCATCH;


  return 0;
}
