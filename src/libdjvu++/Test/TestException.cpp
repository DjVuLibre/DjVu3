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
//C- $Id: TestException.cpp,v 1.10 2000-10-06 21:47:21 fcrary Exp $


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

  G_TRY 
    {
      int a = 2;
      int b = 2+a;
      printf("a=%d, b=%d, c=%d\n",a,b,a+b);
    } 
  G_CATCH(ex) 
    {
      fprintf(stderr,"*** %s\n", DjVuMsg.LookUp( ex.get_cause() ));
    }
  G_ENDCATCH;
  

  G_TRY 
    {
      int a = 2;
      int b = 2+a;
      printf("a=%d, b=%d, c=%d\n",a,b,a+b);
      G_THROW("TextException.test");
    }
  G_CATCH(ex)
    {
      fprintf(stderr,"*** %s\n", DjVuMsg.LookUp( ex.get_cause() ));
    }
  G_ENDCATCH;

  G_TRY
    {
      GString gs = "abcdef";
      printf("gs[0]=%c\n",gs[0]);
      printf("gs[-1]=%c\n",gs[-1]);
      printf("gs[-12]=%c\n",gs[-12]);
    }
  G_CATCH(ex)
    {
      fprintf(stderr,"*** %s\n", DjVuMsg.LookUp( ex.get_cause() ));
    }
  G_ENDCATCH;


  G_TRY
    {
      G_TRY
        {
          GString gs = "abcdef";
          printf("gs[0]=%c\n",gs[0]);
          printf("gs[-1]=%c\n",gs[-1]);
          printf("gs[-12]=%c\n",gs[-12]);
        }
      G_CATCH(ex)
        {
          fprintf(stderr,"*** %s\n", DjVuMsg.LookUp( ex.get_cause() ));
          printf("Rethrown\n");
          G_RETHROW;
        }
      G_ENDCATCH;
    }
  G_CATCH(ex)
    {
      printf("Recatched\n");
      fprintf(stderr,"*** %s\n", DjVuMsg.LookUp( ex.get_cause() ));
    }
  G_ENDCATCH;


  return 0;
}
