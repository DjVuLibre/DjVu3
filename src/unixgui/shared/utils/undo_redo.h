//C-  -*- C++ -*-
//C-
//C- DjVu® Unix Viewer (v. 3.5)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C-
// 
// $Id: undo_redo.h,v 1.18 2001-10-17 19:11:41 docbill Exp $
// $Name:  $


#ifndef HDR_UNDO_REDO
#define HDR_UNDO_REDO

#include "DjVuGlobal.h"
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
