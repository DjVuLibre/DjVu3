/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuOptions.h,v 1.3 2000-02-03 19:07:38 bcr Exp $
 */

#ifndef _DJVU_OPTIONS_H_
#define _DJVU_OPTIONS_H_

/** These options are available in DjVuIO.cpp as part of libdjvuio++.a */
extern const char djvu_config_string[];
extern const char djvu_filelist_string[];
extern const char djvu_verbose_string[];
extern const char djvu_force_string[];
extern const char djvu_dpi_string[];
extern const char djvu_input_format_string[];
extern const char djvu_output_format_string[];
extern const char djvu_prefix_string[];
extern const char djvu_page_number_string[];
extern const char djvu_profile_string[];
extern const char djvu_page_size_string[];
extern const char djvu_render_size_string[];
extern const char djvu_rotate_string[];
extern const char djvu_vflip_string[];
extern const char djvu_hflip_string[];
extern const char djvu_invert_string[];
extern const char djvu_help_string[];
#define djvu_config_option {djvu_config_string+2,1,0,'f'}
#define djvu_verbose_option {djvu_verbose_string+2,2,0,'v'}
#define djvu_filelist_option {djvu_filelist_string+2,1,0,'T'}
#define djvu_force_option {djvu_force_string+2,0,0,'F'}
#define djvu_dpi_option {djvu_dpi_string+2,1,0,'d'}
#define djvu_input_format_option {djvu_input_format_string+2,1,0,'I'}
#define djvu_output_format_option {djvu_output_format_string+2,1,0,'O'}
#define djvu_prefix_option {djvu_prefix_string+2,1,0,'P'}
#define djvu_page_number_option {djvu_page_number_string+2,1,0,'p'}
#define djvu_profile_option {djvu_profile_string+2,1,0,'c'}
#define djvu_page_size_option {djvu_page_size_string+2,1,0,0xff}
#define djvu_render_size_option {djvu_render_size_string+2,1,0,0xfe}
#define djvu_rotate_option {djvu_rotate_string+2,1,0,'r'}
#define djvu_vflip_option {djvu_vflip_string+2,2,0,'V'}
#define djvu_hflip_option {djvu_hflip_string+2,2,0,'H'}
#define djvu_invert_option {djvu_invert_string+2,0,0,'x'}
#define djvu_help_option {djvu_help_string+2,0,0,'h'}

/** These are defined in DjVuProcess.cpp as part of libdjvudecode.a */

extern const char djvu_page_range_string[];
extern const char djvu_trace_string[];
#define djvu_page_range_option {djvu_page_range_string+2,1,0,'p'}
#define djvu_trace_option {djvu_trace_string+2,0,0,'Z'}

/** These are defined in DjVuTransform.cpp as part of libdjvudecode.a */

extern const char djvu_togray_string[];
extern const char djvu_tobitonal_string[];
extern const char djvu_resize_string[];
extern const char djvu_upsample_string[];
extern const char djvu_subsample_string[];
extern const char djvu_crop_string[];
#define djvu_togray_option {djvu_togray_string+2,2,0,'G'}
#define djvu_tobitonal_option {djvu_tobitonal_string+2,2,0,'B'}
#define djvu_resize_option {djvu_resize_string+2,1,0,'R'}
#define djvu_upsample_option {djvu_upsample_string+2,1,0,'u'}
#define djvu_subsample_option {djvu_subsample_string+2,1,0,'s'}
#define djvu_crop_option {djvu_crop_string+2,1,0,'S'}

/** These are defined in DjVuJB2Parse.cpp as part of libdjvuphoto.a */

extern const char djvu_normal_string[];
extern const char djvu_conservative_string[];
extern const char djvu_lossless_string[];
extern const char djvu_aggressive_string[];
extern const char djvu_halftone_off_string[];
extern const char djvu_pages_per_dict_string[];
extern const char djvu_pseudo_string[];
extern const char djvu_tolerance_percent_string[];
#define djvu_normal_option {djvu_normal_string+2,0,0,'n'}
#define djvu_conservative_option {djvu_conservative_string+2,0,0,'C'}
#define djvu_lossless_option {djvu_lossless_string+2,0,0,'l'}
#define djvu_aggressive_option {djvu_aggressive_string+2,0,0,'a'}
#define djvu_halftone_off_option {djvu_halftone_off_string+2,2,0,'t'}
#define djvu_pages_per_dict_option {djvu_pages_per_dict_string+2,1,0,'D'}
#define djvu_pseudo_option {djvu_pseudo_string+2,2,0,'g'}
#define djvu_tolerance_percent_option {djvu_tolerance_percent_string+2,1,0,0xfd}
#define djvu_tolerance4_size_option {djvu_tolerance4_size_string+2,1,0,0xfc}

/** These are defined in DjVuIW44Parse.cpp as part of libdjvuphoto.a */

extern const char djvu_sizes_string[];
extern const char djvu_decibels_string[];
extern const char djvu_slices_string[];
extern const char djvu_quality_string[];
extern const char djvu_crcbnormal_string[];
extern const char djvu_crcbfull_string[];
extern const char djvu_crcbhalf_string[];
extern const char djvu_crcbnone_string[];
extern const char djvu_crcbdelay_string[];
extern const char djvu_gamma_string[];
extern const char djvu_jpeg_string[];
#define djvu_crcbfull_option {djvu_crcbfull_string+2,0,0,'A'}
#define djvu_quality_option {djvu_quality_string+2,1,0,'q'}
#define djvu_slices_option {djvu_slices_string+2,1,0,'N'}
#define djvu_sizes_option {djvu_sizes_string+2,1,0,'k'}
#define djvu_decibels_option {djvu_decibels_string+2,1,0,'b'}
#define djvu_crcbnormal_option {djvu_crcbnormal_string+2,0,0,'z'}
#define djvu_crcbhalf_option {djvu_crcbhalf_string+2,0,0,0xfc}
#define djvu_crcbnone_option {djvu_crcbnone_string+2,0,0,0xfb}
#define djvu_crcbdelay_option {djvu_crcbdelay_string+2,1,0,0xfa}
#define djvu_gamma_option {djvu_gamma_string+2,1,0,0xf9}
#define djvu_jpeg_option {djvu_jpeg_string+2,2,0,'j'}

/** These are defined in DjVuForegroundParse.cpp as part of libdjvudocument.a */

extern const char djvu_color_jb2_string[];
extern const char djvu_fg_quality_string[];

#define djvu_color_jb2_option {djvu_color_jb2_string+2,0,0,0xf7}
#define djvu_fg_quality_option {djvu_fg_quality_string+2,1,0,0xf6}

/** These are defined in DjVuSegParse.cpp as part of libdjvudocument.a */

extern const char djvu_threshold_level_string[];
extern const char djvu_shape_filter_level_string[];
extern const char djvu_pix_filter_level_string[];
extern const char djvu_inversion_level_string[];
extern const char djvu_inhibit_foreback_level_string[];
extern const char djvu_edge_size_string[];
extern const char djvu_render_size_string[];
extern const char djvu_blurring_size_string[];
extern const char djvu_fg_subsampling_string[];
extern const char djvu_bg_subsampling_string[];
extern const char djvu_resolution_multiplier_string[];
extern const char djvu_high_variation_foreground_string[];
extern const char djvu_refine_string[];
extern const char djvu_sub_chrom_string[];

#define djvu_sub_chrom_option {djvu_sub_chrom_string+2,0,0,0xf8}
#define djvu_threshold_level_option {djvu_threshold_level_string+2,1,0,0xf5}
#define djvu_shape_filter_level_option {djvu_shape_filter_level_string+2,1,0,0xf4}
#define djvu_pix_filter_level_option {djvu_pix_filter_level_string+2,1,0,0xf3}
#define djvu_inversion_level_option {djvu_inversion_level_string+2,1,0,0xf2}
#define djvu_inhibit_foreback_level_option {djvu_inhibit_foreback_level_string+2,1,0,0xf1}
#define djvu_edge_size_option {djvu_edge_size_string+2,1,0,0xf0}
#define djvu_segm_render_size_option {djvu_render_size_string+2,1,0,0xef}
#define djvu_blurring_size_option {djvu_blurring_size_string+2,1,0,0xee}
#define djvu_fg_subsampling_option {djvu_fg_subsampling_string+2,1,0,0xec}
#define djvu_bg_subsampling_option {djvu_bg_subsampling_string+2,1,0,0xeb}
#define djvu_resolution_multiplier_option {djvu_resolution_multiplier_string+2,1,0,0xea}
#define djvu_high_variation_foreground_option {djvu_high_variation_foreground_string+2,2,0,0xe9}
#define djvu_refine_option {djvu_refine_string+2, 2, 0, 0xe8}

#endif

