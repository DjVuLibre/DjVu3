//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: TestException.cpp,v 1.3 1999-02-16 17:19:15 leonb Exp $


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
