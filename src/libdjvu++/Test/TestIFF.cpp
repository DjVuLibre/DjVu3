//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: TestIFF.cpp,v 1.2 1999-02-01 18:32:35 leonb Exp $

/*****************************************************************************
 *
 *   $Revision: 1.2 $
 *   $Date: 1999-02-01 18:32:35 $
 *   @(#) $Id: TestIFF.cpp,v 1.2 1999-02-01 18:32:35 leonb Exp $
 *
 *****************************************************************************/

static char RCSVersion[]="@(#) $Id: TestIFF.cpp,v 1.2 1999-02-01 18:32:35 leonb Exp $";

#include "GIFFManager.h"

void main(int argc, char ** argv)
{
   TRY
   {
      StdioByteStream str("page.djvu", "r");
      GIFFManager mng;
      mng.LoadFile(str);
      
      mng.AddSection("1st", GArray<char>(1), 0);
      mng.AddSection("LEV1.LEV2.data", GArray<char>(1));
      mng.AddSection("LEV1.ASS", GArray<char>(1));
      mng.AddSection("last", GArray<char>(1));
      StdioByteStream str1("out.djvu", "w");
      mng.SaveFile(str1);
   } CATCH(exc)
   {
      fprintf(stderr, "%s\n", exc.cause);
   } ENDCATCH;
}
