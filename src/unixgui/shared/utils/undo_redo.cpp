//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: undo_redo.cpp,v 1.1 2001-08-08 17:38:05 docbill Exp $
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
