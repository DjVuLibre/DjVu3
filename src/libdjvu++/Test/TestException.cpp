//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
//C-


#include <stdio.h>
#include <locale.h>
#include "GException.h"
#include "GString.h"
#include "DjVu.h"
#include "DjVuMessage.h"


#ifdef _MSC_VER
#include <io.h>
#endif

int
main()
{
   setlocale(LC_ALL,"");
   DjVuMessage::use_locale();

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
      DjVuPrintMessage("a=%d, b=%d, c=%d\n",a,b,a+b);
    } 
  G_CATCH(ex) 
    {
      DjVuPrintError("*** %s\n", (const char *)DjVuMessage::LookUpNative( ex.get_cause() ));
    }
  G_ENDCATCH;
  

  G_TRY 
    {
      int a = 2;
      int b = 2+a;
      DjVuPrintMessage("a=%d, b=%d, c=%d\n",a,b,a+b);
      G_THROW( ERR_MSG("TextException.test") );
    }
  G_CATCH(ex)
    {
      DjVuPrintError("*** %s\n", (const char *)DjVuMessage::LookUpNative( ex.get_cause() ));
    }
  G_ENDCATCH;

  G_TRY
    {
      GUTF8String gs = "abcdef";
      DjVuPrintMessage("gs[0]=%c\n",gs[0]);
      DjVuPrintMessage("gs[-1]=%c\n",gs[-1]);
      DjVuPrintMessage("gs[-12]=%c\n",gs[-12]);
    }
  G_CATCH(ex)
    {
      DjVuPrintError("*** %s\n", (const char *)DjVuMessage::LookUpNative( ex.get_cause() ));
    }
  G_ENDCATCH;


  G_TRY
    {
      G_TRY
        {
          GUTF8String gs = "abcdef";
          DjVuPrintMessage("gs[0]=%c\n",gs[0]);
          DjVuPrintMessage("gs[-1]=%c\n",gs[-1]);
          DjVuPrintMessage("gs[-12]=%c\n",gs[-12]);
        }
      G_CATCH(ex)
        {
          DjVuPrintError("*** %s\n", (const char *)DjVuMessage::LookUpNative( ex.get_cause() ));
          DjVuPrintMessage("Rethrown\n");
          G_RETHROW;
        }
      G_ENDCATCH;
    }
  G_CATCH(ex)
    {
      DjVuPrintMessage("Recatched\n");
      DjVuPrintError("*** %s\n", (const char *)DjVuMessage::LookUpNative( ex.get_cause() ));
    }
  G_ENDCATCH;


  return 0;
}
