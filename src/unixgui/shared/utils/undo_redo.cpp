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
// $Id: undo_redo.cpp,v 1.11 2001-08-23 23:04:34 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "undo_redo.h"

#include "exc_msg.h"

void
UndoRedo::check(void)
{
   if (!initialized)
      throw ERROR_MESSAGE("UndoRedo::check",
			  "The UndoRedo class has not been initialized yet.");
}

void
UndoRedo::addAction(const char * description,
		    const GP<DjVuAnno> & new_anno,
		    int flags)
{
   check();
   
   undo_list.prepend(new Frame(description, cur_anno, flags));
   redo_list.empty();
   cur_anno=new_anno->copy();
}

GP<DjVuAnno>
UndoRedo::undo(int depth)
{
   check();
   
   if (undo_list.size()<=depth)
      throw ERROR_MESSAGE("UndoRedo::undo", "The undo list is not that long.");
   
   for(int i=0;i<=depth;i++)
   {
      GPosition pos;
      undo_list.first(pos);
      GP<Frame> item=undo_list[pos];
      undo_list.del(pos);
      GP<DjVuAnno> tmp_anno=cur_anno;
      cur_anno=item->anno;
      item->anno=tmp_anno;
      redo_list.prepend(item);
   }
   
   return cur_anno->copy();
}

GP<DjVuAnno>
UndoRedo::redo(int depth)
{
   check();
   
   if (redo_list.size()<=depth)
      throw ERROR_MESSAGE("UndoRedo::redo", "The redo list is not that long.");
   
   for(int i=0;i<=depth;i++)
   {
      GPosition pos;
      redo_list.first(pos);
      GP<Frame> item=redo_list[pos];
      redo_list.del(pos);
      GP<DjVuAnno> tmp_anno=cur_anno;
      cur_anno=item->anno;
      item->anno=tmp_anno;
      undo_list.prepend(item);
   }
   
   return cur_anno->copy();
}

GUTF8String
UndoRedo::getUndoDescription(int depth)
{
   check();
   
   if (undo_list.size()<=depth)
      throw ERROR_MESSAGE("UndoRedo::getUndoDescription", "The undo list is not that long.");
   GPosition pos;
   undo_list.nth(depth, pos);
   return undo_list[pos]->description;
}

GUTF8String
UndoRedo::getRedoDescription(int depth)
{
   check();
   
   if (redo_list.size()<=depth)
      throw ERROR_MESSAGE("UndoRedo::getRedoDescription", "The redo list is not that long.");
   GPosition pos;
   redo_list.nth(depth, pos);
   return redo_list[pos]->description;
}

int
UndoRedo::getUndoFlags(int depth)
{
   check();
   
   if (undo_list.size()<=depth)
      throw ERROR_MESSAGE("UndoRedo::getUndoFlags", "The undo list is not that long.");
   GPosition pos;
   undo_list.nth(depth, pos);
   return undo_list[pos]->flags;
}

int
UndoRedo::getRedoFlags(int depth)
{
   check();
   
   if (redo_list.size()<=depth)
      throw ERROR_MESSAGE("UndoRedo::getRedoFlags", "The redo list is not that long.");
   GPosition pos;
   redo_list.nth(depth, pos);
   return redo_list[pos]->flags;
}
