//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_base_ant.cpp,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "ByteStream.h"
#include "qd_base_ant.h"
#include "djvu_base_res.h"
#include "debug.h"
#include "qlib.h"

#include <qapplication.h>

#include "qt_fix.h"

#include <math.h>
#define Pi 3.14159265359
#define Pi2 (3.14159265359*2)

void
QDBase::eraseSearchResults(void)
{
   for(GPosition pos=map_areas;pos;)
   {
      GP<MapArea> ma=map_areas[pos];
      if (ma->getComment()==search_results_name)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 map_areas.del(this_pos);
	 ma->repaint();
      } else ++pos;
   }
}

void
QDBase::displaySearchResults(const GList<DjVuTXT::Zone *> & zones_list)
{
   eraseSearchResults();

   for(GPosition pos=zones_list;pos;++pos)
   {
      GP<GMapRect> gma=GMapRect::create(zones_list[pos]->rect);
      gma->comment=search_results_name;
      gma->hilite_color=0xff000000;
      gma->border_type=GMapArea::NO_BORDER;
      gma->url="";
      GP<MapArea> ma=new MapRect(gma);
      map_areas.append(ma);
      ma->attachWindow(pane, &ma_mapper);
      ma->layout(GRect(0, 0, dimg->get_width(), dimg->get_height()));
      ma->repaint();
   }

      // Position the page to see the first map area
   DjVuTXT::Zone * first_zone=zones_list[zones_list];
   int xc=(first_zone->rect.xmin+first_zone->rect.xmax)/2;
   int yc=(first_zone->rect.ymin+first_zone->rect.ymax)/2;
   ma_mapper.map(xc, yc);

   scroll(xc-pane->width()/2, yc-pane->height()/2, true);
}

void
QDBase::eraseMapAreas(bool search_results_too, bool allow_draw)
{
   DEBUG_MSG("QDBase::eraseMapAreas(): Erasing the map areas\n");
   DEBUG_MAKE_INDENT(3);

   GRect grect;
   grect.intersect(rectDocument, rectVisible);
   
   for(GPosition pos=map_areas;pos;)
   {
      GP<MapArea> ma=map_areas[pos];
      if (search_results_too || ma->getComment()!=search_results_name)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 map_areas.del(this_pos);
	 
	 if (!grect.isempty() && allow_draw) ma->repaint();
      } else ++pos;
   }
   cur_map_area=0;
   delete map_area_tip; map_area_tip=0;
}

void
QDBase::createMapAreas(bool allow_draw)
{
   DEBUG_MSG("QDBase::createMapAreas(): Creating the map areas\n");
   DEBUG_MAKE_INDENT(3);

   eraseMapAreas(false, allow_draw);

   if (!dimg) return;

   if (override_flags.hilite_rects.size())
   {
      GP<DjVuInfo> info;
      if (dimg) info=dimg->get_info();
      if (info)
	 for(GPosition pos=override_flags.hilite_rects;pos;++pos)
	 {
	    GP<GMapRect> gmap_rect=override_flags.hilite_rects[pos];
	    GRect grect=*gmap_rect;
	    if (grect.xmin<0) grect.xmin=0;
	    if (grect.ymin<0) grect.ymin=0;
	    if (grect.xmax>info->width) grect.xmax=info->width;
	    if (grect.ymax>info->height) grect.ymax=info->height;
	    if (grect.width()>0 && grect.height()>0)
	    {
	       *gmap_rect=grect;
	       map_areas.append(new MapRect(gmap_rect));
	    }
	 }
   }
   
   if (!override_flags.url.is_empty())
   {
      GRect rect(0, 0, dimg->get_width(), dimg->get_height());
      if (!rect.isempty())
      {
	 GP<GMapRect> gma=GMapRect::create(rect);
	 gma->url=override_flags.url.get_string();
	 gma->border_type=GMapArea::SHADOW_OUT_BORDER;
	 gma->border_always_visible=false;
	 gma->border_width=4;
	 GP<MapArea> ma=new MapRect(gma);
	 map_areas.append(ma);
      }
   } else if (anno && anno->ant)
   {
      DjVuANT * ant=anno->ant;
   
      GPosition pos;
      for(pos=ant->map_areas;pos;++pos)
      {
	 GP<MapArea> ma;
	 GMapArea * gmap_area=ant->map_areas[pos];
	 GUTF8String shape_name = gmap_area->get_shape_name();
	 if (shape_name == GMapArea::RECT_TAG)
	    ma=new MapRect((GMapRect *) gmap_area);
	 else if (shape_name == GMapArea::OVAL_TAG)
	    ma=new MapOval((GMapOval *) gmap_area);
	 else if (shape_name == GMapArea::POLY_TAG)
	    ma=new MapPoly((GMapPoly *) gmap_area);
	 if ( ma )
	 {
	    ma->setActiveOutlineMode(!ma->isAlwaysActive() && prefs.hlinksBorder, true);
	    map_areas.append(ma);
	 }
      }
   }

   for(GPosition pos=map_areas;pos;++pos)
   {
      map_areas[pos]->attachWindow(pane, &ma_mapper);
      map_areas[pos]->layout(GRect(0, 0, dimg->get_width(), dimg->get_height()));
   }
}

void
QDBase::decodeAnno(bool allow_redraw)
      // Will create DjVuAnno and call processAnno()
{
   DEBUG_MSG("QDBase::decodeAnno(): decoding...\n");
   DEBUG_MAKE_INDENT(3);
   
   if ( !dimg ) return;
   G_TRY {
      bool anno_processed=false;
      anno=dimg->get_decoded_anno();
      
      if (anno && anno->ant)
      {
	 processAnno(allow_redraw);
	 anno_processed=true;
      }
      if (!anno_processed)
      {
	 // If we didn't find anything to process, clean annotations
	 // from the previous (if any) page.
	 cleanAnno(allow_redraw);
      }
   } G_CATCH(exc) {
      anno=0;
      showError(this, exc);
   } G_ENDCATCH;
}

void
QDBase::processAnno(bool allow_redraw)
      // Will process DjVuAnno structure, which should have already
      // been created by decodeAnno() or anybody else.
{
   DEBUG_MSG("QDBase::processAnno(): processing...\n");
   DEBUG_MAKE_INDENT(3);

   G_TRY {
      eraseMapAreas(false, allow_redraw);
   
      if (!dimg || !anno || !anno->ant) return;

      DjVuANT * ant=anno->ant;
      
      int do_redraw=0, do_layout=0;

      if (!ignore_ant_mode_zoom)
      {
	 switch(ant->mode)
	 {
	    case DjVuANT::MODE_COLOR:
	       setMode(IDC_DISPLAY_COLOR, 0, MODE_ANT); do_redraw=1; break;
	    case DjVuANT::MODE_FORE:
	       setMode(IDC_DISPLAY_FOREGROUND, 0, MODE_ANT); do_redraw=1; break;
	    case DjVuANT::MODE_BACK:
	       setMode(IDC_DISPLAY_BACKGROUND, 0, MODE_ANT); do_redraw=1; break;
	    case DjVuANT::MODE_BW:
	       setMode(IDC_DISPLAY_BLACKWHITE, 0, MODE_ANT); do_redraw=1; break;
	    default:
	       break;
	 }
	 
	 switch(ant->zoom)
	 {
	    case DjVuANT::ZOOM_STRETCH:
	       setZoom(IDC_ZOOM_STRETCH, 0, ZOOM_ANT); do_layout=1; break;
	    case DjVuANT::ZOOM_ONE2ONE:
	       setZoom(IDC_ZOOM_ONE2ONE, 0, ZOOM_ANT); do_layout=1; break;
	    case DjVuANT::ZOOM_WIDTH:
	       setZoom(IDC_ZOOM_WIDTH, 0, ZOOM_ANT); do_layout=1; break;
	    case DjVuANT::ZOOM_PAGE:
	       setZoom(IDC_ZOOM_PAGE, 0, ZOOM_ANT); do_layout=1; break;
	    default:
	       if (ant->zoom!=DjVuANT::ZOOM_UNSPEC)
	       {
		  setZoom(IDC_ZOOM_MIN+ant->zoom, 0, ZOOM_ANT); do_layout=1;
	       }
	 }
      }

      u_int32 bg_color=ant->bg_color;
      if (bg_color==0xffffffff) bg_color=0xffffff;
      if (bg_color!=back_color)
      {
	 do_redraw=1;
	 setBackgroundColor(bg_color, false);	// Don't redraw
      }

      if (ant->hor_align!=DjVuANT::ALIGN_CENTER &&
	  ant->hor_align!=DjVuANT::ALIGN_UNSPEC ||
	  ant->ver_align!=DjVuANT::ALIGN_CENTER &&
	  ant->ver_align!=DjVuANT::ALIGN_UNSPEC)
	 do_layout=1;
   
      createMapAreas(false); do_redraw=1;
   
      if (do_layout) layout(0);	// Disallow redraw
      if (allow_redraw && (do_layout || do_redraw)) redraw();
   } G_CATCH(exc) {
      showError(this, exc);
   } G_ENDCATCH;
}

void
QDBase::cleanAnno(bool allow_redraw)
{
   anno=0;

      // Is required to get rid of highlighted areas from annotations
      // and keep areas from URL or EMBED flags
   createMapAreas(false);

      // Restore background color
   if (back_color!=0xffffff)
      setBackgroundColor(0xffffff, false);

      // Switch zoom and mode back, if necessary
   if (zoom_src==ZOOM_ANT) setZoom(prefs.nDefaultZoom, 0, ZOOM_ANT);
   if (mode_src==MODE_ANT) setMode(IDC_DISPLAY_COLOR, 0, MODE_ANT);

      // Alignment will be picked up by layout() automatically.
   
   if (allow_redraw) redraw();
}
