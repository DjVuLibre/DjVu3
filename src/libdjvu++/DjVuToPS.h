//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.5)
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
// 
// $Id: DjVuToPS.h,v 1.14 2001-07-24 17:52:04 bcr Exp $
// $Name:  $

#ifndef _DJVU_TO_PS_H_
#define _DJVU_TO_PS_H_

/** @name DjVuToPS.h
    Files #"DjVuToPS.h"# and #"DjVuToPS.cpp"# implement code that can be
    used to convert a \Ref{DjVuImage} or \Ref{DjVuDocument} to PostScript
    format. The conversion is carried out by the \Ref{DjVuToPS} class.
    
    @memo PostScript file generator
    @author Andrei Erofeev <eaf@geocities.com>
    @version
    #$Id: DjVuToPS.h,v 1.14 2001-07-24 17:52:04 bcr Exp $#
*/
//@{

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuDocument.h"

class GRect;

/** DjVuImage to PostScript converter.
    Use this class to print \Ref{DjVuImage}s and \Ref{DjVuDocument}s.
    The behavior is customizable. See \Ref{DjVuToPS::Options} for the
    description of available options.*/
class DjVuToPS : public GPEnabled
{
protected:
      /** Default constructor. Initializes the class. */
   DjVuToPS(void);
public:
      /// Default creator.
   static GP<DjVuToPS> create(void) {return new DjVuToPS();}

      /** \Ref{DjVuToPS} options. Use this class to customize the way
	  in which DjVu to PS conversion will be done. You can adjust
	  the following things:
	  \begin{description}
	     \item[Format] ({\em EPS} or {\em PS}). Use the {\em EPS}
		format if you plan to embed the output image into another
		document. Print {\em PS} otherwise.
	     \item[Language level] ({\em 1} or {\em 2}). Any PostScript
	        printer or interpreter should understand PostScript Level 1
		files. Unfortunately we cannot efficiently compress and encode
		data when generating Level 1 files. PostScript Level 2 allows
		to employ an RLE compression and ASCII85 encoding scheme,
		which makes output files significantly smaller. Most of
		the printers and word processors nowadays support PostScript
		Level 2.
	     \item[Orientation] ({\em PORTRAIT} or {\em LANDSCAPE})
	     \item[Zoom factor] ({\em FIT_PAGE} or {\em ONE_TO_ONE}).
		{\em ONE_TO_ONE} mode is useful, if you want the output to
		be of the same size as the original image (before compression).
		This requires that the #dpi# setting inside the \Ref{DjVuImage}
		is correct. In most of the cases the {\em FIT_PAGE} zoom is
		would be your best choice.
	     \item[Mode] ({\em COLOR}, {\em FORE}, {\em BACK}, or {\em BW})
		Specifies how the \Ref{DjVuImage}s will be rendered (all layers,
		foreground layer, background layer, and the mask respectively)
	     \item[Color] ({\em TRUE} or {\em FALSE}). If {\em FALSE},
		color images will be converted to GrayScale before printing.
	     \item[Gamma] Printer gamma correction. #0.3-5.0#
	     \item[Number of copies] Specifies how many copies should be
		printed. This does {\bf not} affect the size of the output file.
	  \end{description}
	*/
   class Options
   {
   public:
	 /** Specifies the mode in which a \Ref{DjVuImage} will be rendered.
	     \begin{description}
	        \item[COLOR] All image layers will be rendered
		\item[FORE] Only foreground image layers will be rendered
		\item[BACK] Only foreground image layers will be rendered
		\item[BW] Only image mask will be printed
	     \end{description} */
      enum Mode { COLOR, FORE, BACK, BW };
	 /** Specifies how the output will be scaled.
	     \begin{description}
	        \item[FIT_PAGE] The output will be scaled (with aspect ratio
		     unchanged) to occupy as much space as possible).
		\item[Any positive number] Zoom factor from 5% to 999%.
		     The \Ref{DjVuImage} will be scaled according to this
		     zoom factor and the image's #dpi# (which gives the
		     size of the original image before compression).
	     \end{description}*/
      enum Zoom { FIT_PAGE=0 };
	 /** Selects the output format ({\bf PostScript} or
	     {\bf Encapsulated PostScript}) */
      enum Format { PS, EPS };
	 /** Specifies the orientation of the output image */
      enum Orientation { PORTRAIT, LANDSCAPE };
   private:
      Format	format;
      int	level;
      Orientation orientation;
      Mode	mode;
      Zoom	zoom;
      bool	color;
      float	gamma;
      int	copies;
      bool	frame;
   public:
	 /// Sets output image format to #PS# or #EPS#
      void	set_format(Format format);
	 /// Sets PostScript level (#1# or #2#)
      void	set_level(int level);
	 /// Sets orientation (#LANDSCAPE# or #PORTRAIT#)
      void	set_orientation(Orientation orientation);
	 /// Sets \Ref{DjVuImage} rendering mode (#COLOR#, #BW#, #FORE#, #BACK#)
      void	set_mode(Mode mode);
	 /** Sets zoom factor to #FIT_PAGE# or any positive %%
	     The zoom factor does {\bf not} affect the amount of data
	     sent to the printer. It is used by the PostScript code
	     to additionally scale the image. The reason why is that
	     we do not know the resolution of the printer when we
	     generate the PostScript file. So, we will render the \Ref{DjVuImage}
	     into the rectangle passed to the \Ref{DjVuToPS::print}()
	     function. */
      void	set_zoom(Zoom zoom);
	 /// Affects automatic conversion of \Ref{DjVuImage} to GreyScale mode.
      void	set_color(bool color);
	 /// Sets printer's gamma correction factor #0.3-5.0#
      void	set_gamma(float gamma);
	 /** Specifies the number of copies to be printed. This parameter
	     does {\bf not} affect the size of output file. */
      void	set_copies(int copies);
	 /** Specifies if a gray frame around the image should be printed. */
      void	set_frame(bool on);

	 /// Returns output image format (#PS# or #EPS#)
      Format	get_format(void) const;
	 /// Returns PostScript level (#1# or #2#)
      int	get_level(void) const;
	 /// Returns output image orientation (#PORTRAIT# or #LANDSCAPE#)
      Orientation get_orientation(void) const;
	 /** Returns \Ref{DjVuImage} rendering mode (#COLOR#, #FORE#,
	     #BACK#, #BW#) */
      Mode	get_mode(void) const;
	 /// Returns output zoom factor (#FIT_PAGE# or #ONE_TO_ONE#)
      Zoom	get_zoom(void) const;
	 /** Returns #FALSE# if \Ref{DjVuImage} will be converted to
	     GreyScale prior to printing */
      bool	get_color(void) const;
	 /// Returns printer gamma correction factor
      float	get_gamma(void) const;
	 /** Returns the number of copies, which will be printed by printer
	     This parameter does {\bf not} affect the size of output file */
      int	get_copies(void) const;
	 /** Returns #TRUE# if there will be a gray frame printed
	     around the image. */
      bool	get_frame(void) const;
      
      Options(void);
   };

      /** Describes current page processing stage. This is passed to
	  the #info_cb()# callback. See \Ref{set_info_cb}() for details. */
   enum Stage { DECODING, PRINTING };
public:
   class DecodePort : public DjVuPort
   {
   public:
      GEvent	decode_event;		// These two are both triggered when
      bool	decode_event_received;	// smth interesting received from decoder.

      float	decode_done;
      GURL	decode_page_url;

	 // DjVuPort virtual functions... Used here to decoded pages
	 // of a DjVuDocument (if we're asked to print a document)
	 // It's not used when printing a DjVuImage
      virtual void	notify_file_flags_changed(const DjVuFile * source,
						  long set_mask, long clr_mask);
      virtual void	notify_decode_progress(const DjVuPort * source, float done);
   private:
     DecodePort(void) : decode_event_received(false), decode_done(0.0) {};
   friend class DjVuToPS;
   };
private:
   
   static char	bin2hex[256][2];
   
   void		(* refresh_cb)(void * refresh_cl_data);
   void *	refresh_cl_data;
   void		(* prn_progress_cb)(float done, void * prn_progress_cl_data);
   void *	prn_progress_cl_data;
   void		(* dec_progress_cb)(float done, void * dec_progress_cl_data);
   void *	dec_progress_cl_data;
   void		(* info_cb)(int page_num, int page_cnt, int tot_pages,
			    Stage stage, void * info_cl_data);
   void *	info_cl_data;

   GP<DecodePort>	port;
protected:
      /* Will output the formated string to the specified \Ref{ByteStream}
	 like #fprintf# would do it for a #FILE#. */
   static void		write(ByteStream & str, const char * format, ...);

      /* Will read data between #src_start# and #src_end# pointers
	 (excluding byte pointed by #src_end#), encode it using
	 {\bf ASCII85} algorithm, and output the result into the destination
	 buffer pointed by #dst#.

	 The function returns pointer to the first unused byte in
	 the destination buffer. */
   static unsigned char * ASCII85_encode(unsigned char * dst,
					 const unsigned char * src_start,
					 const unsigned char * src_end);
      /* Will read data between #src_start# and #src_end# pointers
	 (excluding byte pointed by #src_end#), RLE encode it,
	 and output the result into the destination buffer pointed by #dst#.

	 #counter# is used to count the number of output bytes.
	 
	 The function returns pointer to the first unused byte in
	 the destination buffer. */
   static unsigned char * RLE_encode(unsigned char * dst,
				     const unsigned char * src_start,
				     const unsigned char * src_end);
   
      /* Will store the {\em document prolog}, which is basically a
	 block of document-level comments in PS DSC 3.0 format.

	 @param str Stream where PostScript data should be written
	 @param pages The total number of pages that will be output into
		this PostScript stream.
	 @param grect Bounding rectangle of the image to be printed.
		This parameter is useful in EPS mode only. */
   void		store_doc_prolog(ByteStream & str,
				 int pages, const GRect & grect);
      /* Will store the {\em document setup}, which is a set of
	 PostScript commands and functions used to inspect and prepare
	 the PostScript interpreter environment before displaying images. */
   void		store_doc_setup(ByteStream & str);
      /* Will store the {\em document trailer}, which is a clean-up code
	 used to return the PostScript interpeter back to the state, in which
	 it was before displaying this document. */
   void		store_doc_trailer(ByteStream & str);
      /* Will store PostScript code necessary to prepare page for
	 the coming \Ref{DjVuImage}. This is basically a scaling
	 code plus initialization of some buffers. */
   void		store_page_setup(ByteStream & str, int page_num,
				 int dpi, const GRect & print_rect);
      /* Will output #showpage# command, and restore PostScript state. */
   void		store_page_trailer(ByteStream & str);
      /* Just outputs the specifies image. The function assumes, that
	 all add-ons (like {\em document setup}, {\em page setup}) are
	 already there. It will just output the image. Since
	 output of this function will generate PostScript errors when
	 used without output of auxiliary functions, it should be
	 used carefully. */
   void		print_image(ByteStream & str, const GP<DjVuImage> & dimg,
			    const GRect & print_rect,
			    const GRect & img_rect);
public:
      /** Options affecting the print result. Please refer to
	  \Ref{DjVuToPS::Options} for details. */
   Options	options;

      /** @name Callbacks */
      //@{
      /** Refresh callback is a function, which will be called fairly
	  often while the image (document) is being printed. It can
	  be used to refresh a GUI, if necessary.

	  @param refresh_cb Callback function to be called periodically
	  @param refresh_cl_data Pointer, which will be passed to #refresh_cb()# */
   void		set_refresh_cb(void (* refresh_cb)(void *), void * refresh_cl_data);
      /** Callback used to report the progress of printing.
	  The progress is a float number from 0 to 1.

	  If an existing \Ref{DjVuImage} is printed, this callback will
	  be called at least twice: in the beginning and at the end of
	  printing.

	  If a \Ref{DjVuDocument} is being printed, this callback will be
	  used to report printing progress of every page. To learn
	  the number of the page being printed you can use \Ref{set_info_cb}()
	  function.

	  See \Ref{set_dec_progress_cb}() to find out how to learn the
	  decoding progress.
	  
	  @param prn_progress_cb Callback function to be called
	  @param prn_progress_cl_data Pointer, which will be passed to the
	         #prn_progress_cb()#. */
   void		set_prn_progress_cb(void (* prn_progress_cb)(float done, void *),
				    void * prn_progress_cl_data);
      /** Callback used to report the progress of decoding.
	  The progress is a float number from 0 to 1.

	  This callback is only used when printing a \Ref{DjVuDocument}
	  in a multithreaded environment. In all other cases it will
	  not be called.

	  Whenever you \Ref{print}() a page range from a \Ref{DjVuDocument},
	  the #DjVuToPS# has to decode the mentioned pages before writing
	  them to the output \Ref{ByteStream}. This callback can be
	  helpful to find out the status of the decoding.

	  See \Ref{set_prn_progress_cb}() to find out how to learn the
	  printing progress.

	  See \Ref{set_info_cb}() to learn how to find out the number
	  of the page being processed, the total number of pages
	  and the number of processed pages.
	  
	  @param dec_progress_cb Callback function to be called
	  @param dec_progress_cl_data Pointer, which will be passed to the
	         #dec_progress_cb()#. */
   void		set_dec_progress_cb(void (* dec_progress_cb)(float done, void *),
				    void * dec_progress_cl_data);

      /** Callback used to report the current printing stage of a \Ref{DjVuDocument}.

	  When printing a \Ref{DjVuDocument} ({\bf not} a \Ref{DjVuImage}),
	  the #DjVuToPS# class will decode and output every page mentioned
	  in the {\em page range}. Before decoding and outputing, it
	  will call this #info_cb()# callback in order to let you know
	  about what is going on. This can be quite useful in a GUI program
	  to keep the user informed.

	  {\bf Note}: This function is not called when you print
	  a \Ref{DjVuImage}.

	  Description of the arguments passed to #info_cb#:
	  \begin{description}
	     \item[page_num] The number of the page being processed
	     \item[page_cnt] Pages counter. Tells how many pages have
	           already been processed.
	     \item[tot_pages] Total number of pages that will be output
	     	   if the printing succeeds.
	     \item[stage] Describes the current processing stage, which can
	           be either #DECODING# or #PRINTING#.
	  \end{description}

	  @param info_cb Callback function to be called
	  @param info_cl_data Pointer, which will be passed to #info_cb()#. */
   void		set_info_cb(void (* info_cb)(int page_num, int page_cnt,
					     int tot_pages, Stage stage,
					     void * info_cl_data),
			    void * info_cl_data);
      //@}
   
      /** Will print the specified \Ref{DjVuImage} #dimg# into the
	  \Ref{ByteStream} #str#. The function will first scale
	  the image to fit the #img_rect#, then extract #prn_rect#
	  from the obtained bitmap and will output it in the
	  PostScript format. The function generates a legal PostScript
	  (or Encapsulated PostScript) file taking care of all comments
	  conforming to Document Structure Conventions v. 3.0.

	  {\bf Warning:} The zoom factor specified in \Ref{Options} does
	  not affect the amount of data stored into the PostScript file.
	  It will be used by the PostScript code to additionally scale
	  the image. We cannot pre-scale it here, because we do not know
	  the future resolution of the printer. The #img_rect# and
	  #prn_rect# alone define how much data will be sent to printer.

	  Using #img_rect# one can upsample or downsample the image prior
	  to sending it to the printer.

	  @param str \Ref{ByteStream} where PostScript output will be sent
	  @param dimg \Ref{DjVuImage} to print
	  @param img_rect Rectangle to which the \Ref{DjVuImage} will be scaled.
	         Note that this parameters defines the amount of data
		 that will actually be sent to the printer. The PostScript
		 code can futher resize the image according to the
		 #zoom# parameter from the \Ref{Options} structure.
	  @param prn_rect Part of img_rect to send to printer.
	  @param override_dpi Optional parameter allowing you to override
	         dpi setting that would otherwise be extracted from #dimg# */
   void		print(ByteStream & str, const GP<DjVuImage> & dimg,
		      const GRect & prn_rect, const GRect & img_rect,
		      int override_dpi=-1);

      /** Will output the specifies pages from the \Ref{DjVuDocument}
	  into the \Ref{ByteStream} in PostScript format.

	  The function will generate a multipage PostScript document
	  conforming to PS DSC 3.0 by storing into it every page
	  mentioned in the #page_range#.

	  If #page_range# is empty, all pages from the \Ref{DjVuDocument}
	  #doc# will be printed.

	  The #page_range# is a set of ranges separated by commas. Every range
	  has this form: {\em start_page}[-{\em end_page}]. {\em end_page}
	  is optional and can be less than the {\em start_page}, in which
	  case the pages will be printed in the reverse order.

	  Examples:
	  \begin{itemize}
	     \item {\bf 1-10} - Will print pages 1 to 10.
	     \item {\bf 10-1} - Will print pages 1 to 10 in reverse order.
	     \item {\bf 1-10,12-20} - Will print pages 1 to 20 with page 11 skipped.
	  \end{itemize}
      */
   void		print(ByteStream & str, const GP<DjVuDocument> & doc,
		      const char * page_range=0);
   
};

//****************************************************************************
//******************************** Options ***********************************
//****************************************************************************

inline DjVuToPS::Options::Format
DjVuToPS::Options::get_format(void) const
{
   return format;
}

inline int
DjVuToPS::Options::get_level(void) const
{
   return level;
}

inline DjVuToPS::Options::Orientation
DjVuToPS::Options::get_orientation(void) const
{
   return orientation;
}

inline DjVuToPS::Options::Mode
DjVuToPS::Options::get_mode(void) const
{
   return mode;
}

inline DjVuToPS::Options::Zoom
DjVuToPS::Options::get_zoom(void) const
{
   return zoom;
}

inline bool
DjVuToPS::Options::get_color(void) const
{
   return color;
}

inline float
DjVuToPS::Options::get_gamma(void) const
{
   return gamma;
}

inline int
DjVuToPS::Options::get_copies(void) const
{
   return copies;
}

inline bool
DjVuToPS::Options::get_frame(void) const
{
   return frame;
}

//****************************************************************************
//******************************** DjVuToPS **********************************
//****************************************************************************

inline void
DjVuToPS::set_refresh_cb(void (* _refresh_cb)(void *),
			 void * _refresh_cl_data)
{
   refresh_cb=_refresh_cb;
   refresh_cl_data=_refresh_cl_data;
}

inline void
DjVuToPS::set_prn_progress_cb(void (* _prn_progress_cb)(float done, void *),
			      void * _prn_progress_cl_data)
{
   prn_progress_cb=_prn_progress_cb;
   prn_progress_cl_data=_prn_progress_cl_data;
}

inline void
DjVuToPS::set_dec_progress_cb(void (* _dec_progress_cb)(float done, void *),
			      void * _dec_progress_cl_data)
{
   dec_progress_cb=_dec_progress_cb;
   dec_progress_cl_data=_dec_progress_cl_data;
}

inline void
DjVuToPS::set_info_cb(void (* _info_cb)(int page_num, int page_cnt,
					int tot_pages, Stage stage, void *),
		      void * _info_cl_data)
{
   info_cb=_info_cb;
   info_cl_data=_info_cl_data;
}

//@}
// ------------
#endif
