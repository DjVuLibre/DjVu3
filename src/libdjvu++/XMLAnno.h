//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//
// $Id: XMLAnno.h,v 1.1 2001-01-17 00:14:55 bcr Exp $
// $Name:  $

#ifndef _LT_XMLANNO__
#define _LT_XMLANNO__

#ifdef __GNUC__
#pragma interface
#endif

#include "XMLTags.h"
#include "GContainer.h"
#include "GString.h"
#include "GURL.h"

class lt_XMLContents;
class DjVuFile;
class DjVuDocument;

class lt_XMLAnno : public GPEnabled
{
public:
  lt_XMLAnno(void) {}
  void parse(const char xmlfile[]);
  void parse(GP<ByteStream> &bs);
  void parse(const lt_XMLTags &tags);
  void save(void);
  void empty(void);
protected:
  void ChangeAnno(const lt_XMLTags &map,const GURL url,const GString id,
    const GString width,const GString height);
  GPList<DjVuFile> files;
  GMap<GString,GP<DjVuDocument> > docs;
};

#endif /* _LT_XMLANNO__ */


