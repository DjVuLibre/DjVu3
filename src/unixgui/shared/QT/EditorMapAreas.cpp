//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: EditorMapAreas.cpp,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif


#include "MapAreas.h"
#include "MapDraw.h"
#include "debug.h"
#include "exc_msg.h"
#include "qlib.h"
#include "qxlib.h"
#include "qt_imager.h"
#include "qx_imager.h"
#include "exc_misc.h"
#include "qd_hlink_edit_dialog.h"

#include <qapplication.h>
#include <qpaintdevicemetrics.h>
#include <qbitmap.h>

int
MapArea::edit(GP<DjVuDocEditor> & doc,
	      const GP<DjVuImage> & dimg, float gamma)
{
   DEBUG_MSG("MapArea::edit(): editing the map area contents\n");
   DEBUG_MAKE_INDENT(3);
   if (!pane)
   {
      DEBUG_MSG("pane==0 => returning\n");
      return EDIT_CANCELLED;
   }

   QDHLinkEditDialog edit(doc, dimg, gmap_area, gamma, pane, "hlink_editor", TRUE);
   if (edit.exec()==QDialog::Accepted)
   {
      bool smth_changed=false;
      GMapArea::BorderType border_type_new=edit.borderType();
      smth_changed|=border_type_new!=gmap_area->border_type;
      
      GUTF8String url_new=GStringFromQString(edit.url());
      smth_changed|=url_new!=gmap_area->url;

      GUTF8String target_new=GStringFromQString(edit.target());
      smth_changed|=target_new!=gmap_area->target;

      GUTF8String comment_new=GStringFromQString(edit.comment());
      smth_changed|=comment_new!=gmap_area->comment;

      int border_width_new=edit.borderWidth();
      smth_changed|=border_width_new!=gmap_area->border_width;

      u_int32 border_color_new=edit.borderColor();
      smth_changed|=border_color_new!=gmap_area->border_color;

      bool border_always_visible_new=edit.borderAlwaysVisible();
      smth_changed|=border_always_visible_new!=gmap_area->border_always_visible;

      u_int32 hilite_color_new=edit.hiliteColor();
      smth_changed|=hilite_color_new!=gmap_area->hilite_color;

      gmap_area->url=url_new;
      gmap_area->target=target_new;
      gmap_area->comment=comment_new;
      gmap_area->border_type=border_type_new;
      gmap_area->border_width=border_width_new;
      gmap_area->border_color=border_color_new;
      gmap_area->border_always_visible=border_always_visible_new;
      gmap_area->hilite_color=hilite_color_new;

      initBorder();	// Will redraw the border too
      if (gmap_area->hilite_color!=0xffffffff) repaint();

      return smth_changed ? EDIT_SMTH_CHANGED : EDIT_NOTHING_CHANGED;
   }
   return EDIT_CANCELLED;
}

