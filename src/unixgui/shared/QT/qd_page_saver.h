//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_page_saver.h,v 1.2 2001-06-06 14:53:58 mchen Exp $
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

class QDPageSaver: public QObject
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
