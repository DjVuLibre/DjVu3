//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: djvu_base_res.h,v 1.2 2001-08-08 17:44:52 docbill Exp $
// $Name:  $


#ifndef HDR_DJVU_BASE_RES
#define HDR_DJVU_BASE_RES

#ifdef __GNUC__
#pragma interface
#endif

#define IDC_SAVE			40000
#define IDC_SAVE_DOC			40001
#define IDC_SAVE_DOC_AS			40002
#define IDC_SAVE_PAGE			40003
#define IDC_SAVE_PAGE_AS		40004
#define IDC_SAVE_PAGES_AS		40005
#define IDC_COMPRESS_DOC		40006
#define IDC_EXPORT_PAGE			40007
#define IDC_SEND_PAGE			40008
#define IDC_PRINT			40009
#define IDC_PRINT_DOC			40010
#define IDC_PRINT_PAGE			40011
#define IDC_PRINT_WIN			40012
#define IDC_PRINT_CUSTOM		40013
#define IDC_PREFERENCES			40014

#define IDC_MODE			40100
#define IDC_DISPLAY			40100
#define IDC_DISPLAY_COLOR		40101
#define IDC_DISPLAY_BLACKWHITE		40102
#define IDC_DISPLAY_BACKGROUND		40103
#define IDC_DISPLAY_FOREGROUND		40104
#define IDC_ZOOM			40200
// It's important, that PAGE==MIN-1, WIDTH==MIN-2 and STRETCH=MIN-3.
// Don't change it!
#define IDC_ZOOM_STRETCH		40201
#define IDC_ZOOM_CUSTOM			(IDC_ZOOM_STRETCH+1)
#define IDC_ZOOM_ONE2ONE		(IDC_ZOOM_CUSTOM+1)
#define IDC_ZOOM_WIDTH			(IDC_ZOOM_ONE2ONE+1)
#define IDC_ZOOM_PAGE			(IDC_ZOOM_WIDTH+1)
#define IDC_ZOOM_MIN			(IDC_ZOOM_PAGE+1)
#define IDC_ZOOM_25			(IDC_ZOOM_MIN+25)
#define IDC_ZOOM_50			(IDC_ZOOM_MIN+50)
#define IDC_ZOOM_75			(IDC_ZOOM_MIN+75)
#define IDC_ZOOM_100			(IDC_ZOOM_MIN+100)
#define IDC_ZOOM_150			(IDC_ZOOM_MIN+150)
#define IDC_ZOOM_300			(IDC_ZOOM_MIN+300)
#define IDC_ZOOM_MAX			(IDC_ZOOM_MIN+999)
#define IDC_ZOOM_ZOOMIN			(IDC_ZOOM_MAX+1)
#define IDC_ZOOM_ZOOMOUT		(IDC_ZOOM_MAX+2)

#define IDC_PANE                        42000
#define IDC_ZOOM_SELECT                 42010
#define IDC_TEXT_SELECT                 42020

#define IDC_ROTATE_90                   42100
#define IDC_ROTATE_270                  42110

#define IDC_OPEN			41300
#define IDC_CLOSE			41301
#define IDC_EXIT			41302
#define IDC_FILE			41303
#define IDC_IMPORT			41304

#define IDC_NAVIGATE			41400
#define IDC_NAV_FIRST_PAGE		41401
#define IDC_NAV_LAST_PAGE		41402
#define IDC_NAV_NEXT_PAGE		41403
#define IDC_NAV_PREV_PAGE		41404
#define IDC_NAV_NEXT_PAGE10		41405
#define IDC_NAV_PREV_PAGE10		41406
#define IDC_NAV_GOTO_PAGE		41407

#define IDC_THUMB_SHOW			41500
#define IDC_THUMB_GENERATE		41502
#define IDC_THUMB_REMOVE		41503

#define IDC_SEARCH			41600
#define IDC_VIEW			41601
#define IDC_INFO			41602
#define IDC_NEW_WINDOW			41603

#endif
