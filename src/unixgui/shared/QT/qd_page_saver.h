//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_page_saver.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_PAGE_SAVER
#define HDR_QD_PAGE_SAVER

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuFile.h"
#include "DjVmDoc.h"

#include <qdialog.h>

#include "qt_fix.h"

class QDPageSaver
{
private:
   GP<DjVuFile>		djvu_file;
   QWidget		* parent;

   GP<DjVmDoc>	getDjVmDoc(void);
   int		getFilesNum(void);
   
   void		saveSeparate(void);
   void		saveBundled(void);
   void		saveMerged(void);
public:
   void		save(void);

      // Note, that DjVuFile should already have ALL data
   QDPageSaver(const GP<DjVuFile> & file, QWidget * parent);
   ~QDPageSaver(void) {}
};

#endif
