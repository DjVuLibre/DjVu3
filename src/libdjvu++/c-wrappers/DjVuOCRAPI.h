/*C-  -*- C++ -*-
 *C-
 *C- Copyright (c) 2000 LizardTech Inc.
 *C- All rights reserved.
 *C- 
 *C- $Id: DjVuOCRAPI.h,v 1.2 2000-09-18 17:10:36 bcr Exp $
 */

#ifndef _DJVUOCR_H_
#define _DJVUOCR_H_

#include "DjVuAPI.h"

#ifdef __cplusplus
extern "C"
{
#ifndef __cplusplus
};
#endif
#endif

enum djvu_ocr_flags_enum
{
   DJVUOCR_LOCATE_FORCESINGLE=0x1,
   DJVUOCR_RECOGNIZE_DQDM=0x2,
   DJVUOCR_RECOGNIZE_DEGRADED=0x4,
   DJVUOCR_NUMERIC_STYLE=0x8,
   DJVUOCR_PRINT=0x10,
   DJVUOCR_LINES_ONLY=0x20,
   DJVUOCR_DESKEW=0x40
};
struct djvu_rtk;
typedef int djvu_run_rtk (struct djvu_rtk *opts);

#ifdef __cplusplus
#ifndef __cplusplus
{
#endif
}
#endif

#endif /* _DJVUOCR_H_ */

