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
//C- $Id: TestException.cpp,v 1.4 1999-03-15 18:28:53 leonb Exp $


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
