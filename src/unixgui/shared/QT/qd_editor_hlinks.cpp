//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_editor_hlinks.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_editor_hlinks.h"

#include "djvu_editor_res.h"
#include "exc_msg.h"
#include "debug.h"
#include "qlib.h"

#include <math.h>
#define Pi 3.14159265359
#define Pi2 (3.14159265359*2)

const char *DjEditEnterURLMessage="Enter URL here";

void
QDEditor::createMapAreas(bool allow_draw)
{
   DEBUG_MSG("QDEditor::createMapAreas(): Creating the map areas\n");
   DEBUG_MAKE_INDENT(3);
   
   QDBase::createMapAreas(allow_draw);

   DEBUG_MSG("enabling edit controls\n");

   GRect grect;
   grect.intersect(rectVisible, rectDocument);

   if (cur_editor_mode!=PREVIEW)
      for(GPosition pos=map_areas;pos;++pos)
      {
	 GP<MapArea> ma=map_areas[pos];
	 ma->setForceAlwaysActive(true);
	 ma->enableEditControls(true, false);
	 if (allow_draw) ma->repaint();
      }
}

void
QDEditor::HL_CopyToANT(void)
{
   GPList<GMapArea> gmap_areas;
   for(GPosition pos=map_areas;pos;++pos)
      gmap_areas.append(map_areas[pos]->gmap_area->get_copy());
   page_anno->ant->map_areas=gmap_areas;
}

/*****************************************************************************
************************** HL editing callbacks ******************************
*****************************************************************************/

void
QDEditor::HL_Edit(void)
{
   if (cur_map_area)
   {
      if (cur_editor_mode==PREVIEW)
	 setEditorMode(EDIT);
      
      if (cur_map_area->edit(djvu_doc,
           dimg, prefs.dScreenGamma)==MapArea::EDIT_SMTH_CHANGED)
      {
	 HL_CopyToANT();
	 copyAnnoBack();
      }
   }
}

void
QDEditor::HL_Del(const MapArea * obj)
{
   if (!obj)
   {
      obj=cur_map_area; cur_map_area=0;
   }
   if (obj)
   {
      if (cur_editor_mode==PREVIEW)
	 setEditorMode(EDIT);
      
      GPosition pos;
      if (map_areas.search((MapArea *) obj, pos))
      {
	 GP<MapArea> ma=map_areas[pos];
	 map_areas.del(pos);
	 ma->repaint();
	 
	 HL_CopyToANT();
	 copyAnnoBack();
      }
   }
}

void
QDEditor::HL_CancelNewMapArea(void)
{
   if (new_map_area)
   {
      GPosition pos;
      if (map_areas.search(new_map_area, pos)) map_areas.del(pos);
      new_map_area->repaint();
      new_map_area=0;
   }
}

void
QDEditor::HL_Create(int what)
{
   if (what!=IDC_HLINKS_NEW_RECT &&
       what!=IDC_HLINKS_NEW_POLY &&
       what!=IDC_HLINKS_NEW_OVAL &&
       what!=IDC_HILITE_NEW_RECT) return;

      // Didn't finish creating one map area and jump to another :(
      // Get rid of the area being created and start all over again.
   if (new_map_area) HL_CancelNewMapArea();

   new_marea_type=what;
   new_map_area=0;
   new_marea_points=0;
   
   setEditorMode(CREATE);
   updateEditToolBar();
   showStatus("Click to start creating the map area. ESC to abort.");
}

/*****************************************************************************
**************************** Action callbacks ********************************
*****************************************************************************/

bool
QDEditor::HL_Motion(int xPos, int yPos)
{
   if (!map_areas.size() || cur_editor_mode==PREVIEW) return false;
   
   if (cur_editor_mode==CREATE)
   {
      if (new_map_area && new_marea_type==IDC_HLINKS_NEW_POLY)
      {
	 GRect grect;
	 grect.intersect(rectVisible, rectDocument);
	 new_map_area->drawOutline(grect);
	 new_map_area->move(rectDocument, xPos, yPos,
			    MapPoly::VERTEX, -1);
	 new_map_area->drawOutline(grect);
      }
   } else
   {
      MapArea * new_cur_map_area=0;
      int new_cur_marea_vic_code=MapArea::FAR;
      int new_cur_marea_vic_data=0;

      for(GPosition pos=map_areas;pos;++pos)
      {
	 GP<MapArea> ma=map_areas[pos];
	 if (ma->isPointerClose(xPos, yPos, &new_cur_marea_vic_code, &new_cur_marea_vic_data))
	 {
	    new_cur_map_area=ma;
	    break;
	 }
      }
      if (cur_map_area!=new_cur_map_area ||
	  new_cur_marea_vic_code!=cur_marea_vic_code ||
	  new_cur_marea_vic_data!=cur_marea_vic_data)
      {
	 if (!new_cur_map_area) showStatus(" ");
	 else
	 {
	    switch(new_cur_marea_vic_code)
	    {
	       case MapArea::INSIDE:
		  {
		     GUTF8String comment=new_cur_map_area->getComment();
		     GUTF8String url=new_cur_map_area->getURL();
		     GUTF8String info;
		     if (comment.length())
		     {
			if (url.length())
			   info=comment+" ("+url+")";
			else info=comment;
		     } else info=url;
		     if (info.length())
                       showStatus(QStringFromGString(info));
		  }
		  break;
	       case MapPoly::VERTEX:
		  showStatus("Press mouse button to start moving the vertex.");
		  break;
	       default:
		  showStatus("Press mouse button to start resizing the area.");
	       break;
	    }
	 }
	 cur_map_area=new_cur_map_area;
	 cur_marea_vic_code=new_cur_marea_vic_code;
	 cur_marea_vic_data=new_cur_marea_vic_data;
	 setCursor();
      }
   }
   return true;
}

bool
QDEditor::HL_Btn1DblClick(int xPos, int yPos)
{
   DEBUG_MSG("QDEditor::HL_Btn1DblClick() called\n");
   DEBUG_MAKE_INDENT(3);

   if (cur_editor_mode==CREATE &&
       new_marea_type==IDC_HLINKS_NEW_POLY &&
       new_map_area && new_marea_points>=2)
   {
      int x=xPos, y=yPos;
      ma_mapper.unmap(x, y);
	 
      GRect grect;
      grect.intersect(rectVisible, rectDocument);
      new_map_area->drawOutline(grect);
	 
	 // Remove the last point (set with the 1st click)
      GArray<int> xx(new_marea_points-1), yy(new_marea_points-1);
      for(int i=0;i<new_marea_points;i++)
      {
	 xx[i]=new_marea_x[i]; yy[i]=new_marea_y[i];
      }
      GP<MapArea> obj;
      try
      {
	 GP<GMapPoly> poly=GMapPoly::create(xx, yy, new_marea_points);
	 poly->url=DjEditEnterURLMessage;
	 poly->target=GMapArea::TARGET_SELF;
	 poly->comment="";
	 poly->border_type=GMapArea::XOR_BORDER;
	 obj=new MapPoly(poly);
      } catch(const GException & exc)
      {
	 new_map_area->drawOutline(grect);
	 showError(this, exc);
	 return true;
      }
	 
      GPosition pos;
      if (map_areas.search(new_map_area, pos)) map_areas.del(pos);
      new_map_area=0;
      map_areas.append(obj);
	 
      obj->attachWindow(pane, &ma_mapper);
      obj->layout(GRect(0, 0, dimg->get_width(), dimg->get_height()));
      obj->enableEditControls(true, false);
      obj->setForceAlwaysActive(true);
      obj->repaintBorder();

      if(obj->edit(djvu_doc, dimg, prefs.dScreenGamma)
          == MapArea::EDIT_CANCELLED)
      {
        HL_Del(obj);
      }else
      {
	 showStatus(" ");
	 HL_CopyToANT();
	 copyAnnoBack();
      }
      return true;
   } else if (cur_editor_mode==EDIT)
   {
      if (cur_map_area)
      {
	 HL_Edit();
	 return true;
      }
   }
   return false;
}

bool
QDEditor::HL_Btn1Down(int xPos, int yPos)
{
   DEBUG_MSG("QDEditor::HL_Btn1Down() called\n");
   DEBUG_MAKE_INDENT(3);
   
   if (cur_editor_mode==CREATE)
   {
      GRect irect;
      if (irect.intersect(rectVisible, rectDocument) &&
	  irect.contains(xPos, yPos))
      {
	 int x=xPos, y=yPos;
	 ma_mapper.unmap(x, y);
	 if (!new_map_area)
	 {
	    switch(new_marea_type)
	    {
	       case IDC_HLINKS_NEW_RECT:
	       case IDC_HILITE_NEW_RECT:
	       {
		  GP<GMapRect> rect=GMapRect::create(GRect(x, y, 1, 1));
		  if (new_marea_type==IDC_HILITE_NEW_RECT)
		  {
		     rect->url="";
		     rect->border_type=GMapArea::NO_BORDER;
		     rect->hilite_color=0xffff00;
		     rect->border_always_visible=true;
		  } else
		  {
		     rect->url=DjEditEnterURLMessage;
		     rect->target=GMapArea::TARGET_SELF;
		     rect->comment="";
		     rect->border_type=GMapArea::XOR_BORDER;
		  }
		  new_map_area=new MapRect(rect);
		  new_map_area->setOutlineMode(true, true, false);
		  showStatus("Click again to finish creating the rectangle. ESC to undo.");
		  break;
	       }
	       case IDC_HLINKS_NEW_POLY:
	       {
		  GArray<int> xx(1), yy(1);
		  xx[0]=x; yy[0]=y; xx[1]=x+1; yy[1]=y+1;
		  GP<GMapPoly> poly=GMapPoly::create(xx, yy, 2, 1);
		  poly->url=DjEditEnterURLMessage;
		  poly->target=GMapArea::TARGET_SELF;
		  poly->comment="";
		  poly->border_type=GMapArea::XOR_BORDER;
		  new_map_area=new MapPoly(poly);
		  new_map_area->setOutlineMode(true, true, false);
		  showStatus("Click again to add one more vertex. ESC to undo the last one.");
		  break;
	       }
	       case IDC_HLINKS_NEW_OVAL:
	       {
		  GP<GMapOval> oval=GMapOval::create(GRect(x, y, 1, 1));
		  oval->url=DjEditEnterURLMessage;
		  oval->target=GMapArea::TARGET_SELF;
		  oval->comment="";
		  oval->border_type=GMapArea::XOR_BORDER;
		  new_map_area=new MapOval(oval);
		  new_map_area->setOutlineMode(true, true, false);
		  showStatus("Click again to finish creating the oval. ESC to undo.");
		  break;
	       }
	    }
	    new_marea_x.resize(0); new_marea_x[0]=x;
	    new_marea_y.resize(0); new_marea_y[0]=y;
	    new_marea_points=1;
	 
	    new_map_area->attachWindow(pane, &ma_mapper);
	    new_map_area->layout(GRect(0, 0, dimg->get_width(), dimg->get_height()));
	    new_map_area->enableEditControls(true, false);
	    new_map_area->startMoving(xPos+1, yPos+1);
	 
	    map_areas.append(new_map_area);
	 
	    GRect grect;
	    grect.intersect(rectVisible, rectDocument);
	    new_map_area->drawOutline(grect);
	 } else // if (!new_map_area)
	 {
	       // new_map_area!=0
	    if (new_marea_type==IDC_HLINKS_NEW_POLY)
	    {
	       GRect grect;
	       grect.intersect(rectVisible, rectDocument);
	       new_map_area->drawOutline(grect);
	    
	       GArray<int> xx(new_marea_points+1), yy(new_marea_points+1);
	       for(int i=0;i<new_marea_points;i++)
	       {
		  xx[i]=new_marea_x[i]; yy[i]=new_marea_y[i];
	       }
	       xx[new_marea_points]=x;
	       yy[new_marea_points]=y;
	       xx[new_marea_points+1]=x+1;
	       yy[new_marea_points+1]=y+1;
	       GP<MapArea> obj;
	       try
	       {
		  GP<GMapPoly> poly=GMapPoly::create(xx, yy, new_marea_points+2, 1);
		  poly->url=DjEditEnterURLMessage;
		  poly->target=GMapArea::TARGET_SELF;
		  poly->comment="";
		  poly->border_type=GMapArea::XOR_BORDER;
		  obj=new MapPoly(poly);
		  obj->setOutlineMode(true, true, false);
	       } catch(...)
	       {
		  new_map_area->drawOutline(grect);
		  return true;
	       }
	       new_marea_x.resize(new_marea_points);
	       new_marea_y.resize(new_marea_points);
	       new_marea_x[new_marea_points]=x;
	       new_marea_y[new_marea_points++]=y;
	       showStatus("Click again to add one more vertex. Double click to finish. ESC to undo last vertex.");
	    
	       GPosition pos;
	       if (map_areas.search(new_map_area, pos)) map_areas.del(pos);
	       new_map_area=obj;
	       map_areas.append(new_map_area);
	    
	       new_map_area->attachWindow(pane, &ma_mapper);
	       new_map_area->layout(GRect(0, 0, dimg->get_width(), dimg->get_height()));
	       new_map_area->enableEditControls(true, false);
	       new_map_area->startMoving(xPos+1, yPos+1);
	    
	       new_map_area->drawOutline(grect);
	    }
	 }
	 return true;
      }	// If the point is inside the document
   } else if (cur_editor_mode==EDIT)
   {
      if (cur_map_area && cur_marea_vic_code!=MapArea::FAR)
      {
	 cur_map_area->startMoving(xPos, yPos);
	    // Do some optimization here... Maybe we don't need to redraw
	    // everything...
	 if (cur_map_area->getHiliteColor()!=0xffffffff)
	    cur_map_area->setOutlineMode(true, true, true);
	 else
	 {
	    cur_map_area->setOutlineMode(true, true, false);
	    cur_map_area->repaintBorder();
	 }
	 return true;
      }
   }
   return false;
}

bool
QDEditor::HL_Btn1Motion(int xPos, int yPos)
{
   DEBUG_MSG("QDEditor::HL_Btn1Motion() called\n");
   DEBUG_MAKE_INDENT(3);
   
   if (cur_editor_mode==CREATE)
   {
      if (new_map_area &&
	  (new_marea_type==IDC_HLINKS_NEW_RECT ||
	   new_marea_type==IDC_HILITE_NEW_RECT ||
	   new_marea_type==IDC_HLINKS_NEW_OVAL))
      {
	 // Moving bottom right corner of an oval or rectangle under creation
	 GRect grect;
	 grect.intersect(rectVisible, rectDocument);
	 new_map_area->drawOutline(grect);
	 new_map_area->move(rectDocument, xPos, yPos,
			    MapArea::BOTTOM_RIGHT, 0);
	 new_map_area->drawOutline(grect);
      }
      return true;
   } else if (cur_editor_mode==EDIT)
   {
      if (cur_map_area && cur_marea_vic_code!=MapArea::FAR &&
	  cur_map_area->isMoving())
      {
	 // Moving the current hyperlink
	 GRect grect;
	 grect.intersect(rectVisible, rectDocument);
	 cur_map_area->drawOutline(grect);
	 cur_map_area->move(rectDocument, xPos, yPos, cur_marea_vic_code, cur_marea_vic_data);
	 cur_map_area->drawOutline(grect);
	 return true;
      }
   }
   return false;
}

bool
QDEditor::HL_Btn1Up(int xPos, int yPos)
{
   DEBUG_MSG("QDEditor::HL_Btn1Up() called\n");
   DEBUG_MAKE_INDENT(3);
   
   if (cur_editor_mode==CREATE)
   {
      if (new_map_area &&
	  (new_marea_type==IDC_HLINKS_NEW_RECT ||
	   new_marea_type==IDC_HILITE_NEW_RECT ||
	   new_marea_type==IDC_HLINKS_NEW_OVAL))
      {
	    // Finishing creation of either rectangle or oval
	 GP<MapArea> obj=new_map_area;
	 new_map_area=0;

	 obj->finishMoving();
	 obj->setOutlineMode(false, false, true);
	 obj->setForceAlwaysActive(true);

	 if (obj->edit(djvu_doc, dimg, prefs.dScreenGamma)
              ==MapArea::EDIT_CANCELLED)
         {
           HL_Del(obj);
	 }else
	 {
	    showStatus(" ");
	    HL_CopyToANT();
	    copyAnnoBack();
	 }
      }
      return true;
   } else if (cur_editor_mode==EDIT)
   {
      if (cur_map_area && cur_marea_vic_code!=MapArea::FAR &&
	  cur_map_area->isMoving())
      {
	 // Finishing the moving of a hyperlink object
	 GRect grect;
	 grect.intersect(rectVisible, rectDocument);
	 cur_map_area->finishMoving();
	 cur_map_area->setOutlineMode(false, false, true);
	 cur_map_area->setForceAlwaysActive(true);
	 
	 HL_CopyToANT();
	 copyAnnoBack();
	 return true;
      }
   }
   return false;
}

void
QDEditor::HL_AbortMotion(void)
{
   DEBUG_MSG("QDEditor::HL_AbortMotion(): stopping motion...\n");
   DEBUG_MAKE_INDENT(3);
   
   if (cur_editor_mode==CREATE)
   {
      if (!new_map_area) setEditorMode(EDIT);
      else
      {
	 GRect grect;
	 grect.intersect(rectVisible, rectDocument);
	 new_map_area->drawOutline(grect);
	 
	 if (new_marea_type==IDC_HLINKS_NEW_RECT ||
	     new_marea_type==IDC_HILITE_NEW_RECT ||
	     new_marea_type==IDC_HLINKS_NEW_OVAL ||
	     new_marea_type==IDC_HLINKS_NEW_POLY && new_marea_points==1)
	 {
	    GPosition pos;
	    if (map_areas.search(new_map_area, pos)) map_areas.del(pos);
	    new_map_area=0;
	    showStatus("Click to start creating the hyperlink. ESC to abort.");
	 } else
	    if (new_marea_type==IDC_HLINKS_NEW_POLY)
	    {
	       GArray<int> xx(new_marea_points-1), yy(new_marea_points-1);
	       for(int i=0;i<new_marea_points;i++)
	       {
		  xx[i]=new_marea_x[i]; yy[i]=new_marea_y[i];
	       }
	       GP<MapArea> obj;
	       try
	       {
		  GP<GMapPoly> poly=GMapPoly::create(xx, yy, new_marea_points, 1);
		  poly->url=DjEditEnterURLMessage;
		  poly->target=GMapArea::TARGET_SELF;
		  poly->comment="";
		  poly->border_type=GMapArea::XOR_BORDER;
		  obj=new MapPoly(poly);
		  obj->setOutlineMode(true, true, false);
	       } catch(...)
	       {
		  new_map_area->drawOutline(grect);
		  return;
	       }
	       new_marea_points--;
	       if (new_marea_points>=2)
		  showStatus("Click again to add one more vertex. Double click to finish. ESC to undo last vertex.");
	       else
		  showStatus("Click again to add one more vertex. ESC to undo the last one.");
	       
	       GPosition pos;
	       if (map_areas.search(new_map_area, pos)) map_areas.del(pos);
	       new_map_area=obj;
	       map_areas.append(new_map_area);
	       
	       new_map_area->attachWindow(pane, &ma_mapper);
	       new_map_area->layout(GRect(0, 0, dimg->get_width(), dimg->get_height()));
	       new_map_area->enableEditControls(true, false);
	       int x=xx[new_marea_points], y=yy[new_marea_points];
	       ma_mapper.map(x, y);
	       new_map_area->startMoving(x+1, y+1);
	       new_map_area->setOutlineMode(true, true, true);
	    }
      } // if (!new_map_area)
   } else if (cur_editor_mode==EDIT)
   {
      if (cur_map_area && cur_marea_vic_code!=MapArea::FAR &&
	  cur_map_area->isMoving())
      {
	 GRect grect;
	 grect.intersect(rectVisible, rectDocument);
	 cur_map_area->drawOutline(grect);
	 cur_map_area->abortMoving();
	 cur_map_area->setOutlineMode(false, false, true);
	 cur_map_area->setForceAlwaysActive(true);
      }
   }
}
