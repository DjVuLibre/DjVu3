//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: undo_redo.h,v 1.1 2001-08-08 17:38:05 docbill Exp $
// $Name:  $


#ifndef HDR_UNDO_REDO
#define HDR_UNDO_REDO

#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"
#include "GContainer.h"
#include "GSmartPointer.h"
#include "DjVuAnno.h"

class UndoRedo
{
private:
   class Frame : public GPEnabled
   {
   public:
      GUTF8String		description;
      GP<DjVuAnno>	anno;
      int		flags;
      
      Frame(const char * _description, const GP<DjVuAnno> & _anno, int _flags=0) :
	    description(_description), anno(_anno), flags(_flags) {};
 
      virtual ~Frame(void) {}
   };
   
   GPList<Frame>undo_list, redo_list;
   GP<DjVuAnno>	cur_anno;
   bool		initialized;

   void		check(void);
public:
   void		addAction(const char * description,
			  const GP<DjVuAnno> & new_anno,
			  int flags=0);
   GP<DjVuAnno>	undo(int depth=0);
   GP<DjVuAnno>	redo(int depth=0);
   int		getUndoFrames(void) const { return undo_list.size(); }
   GUTF8String	getUndoDescription(int depth=0);
   int		getUndoFlags(int depth=0);
   int		getRedoFrames(void) const { return redo_list.size(); }
   GUTF8String	getRedoDescription(int depth=0);
   int		getRedoFlags(int depth=0);

   void		init(const GP<DjVuAnno> & _cur_anno);
   bool		isInitialized(void) const { return initialized; }

   UndoRedo(void) {}
   UndoRedo(const GP<DjVuAnno> & cur_anno) { init(cur_anno); }
   virtual ~UndoRedo(void) {}
};

inline void
UndoRedo::init(const GP<DjVuAnno> & _cur_anno)
{
   undo_list.empty();
   redo_list.empty();
   cur_anno=_cur_anno->copy();
   initialized=true;
}

#endif
