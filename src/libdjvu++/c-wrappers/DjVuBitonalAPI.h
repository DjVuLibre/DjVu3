/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuBitonalAPI.h,v 1.15 2000-07-05 16:50:53 bcr Exp $
 */

#ifndef _DJVUBITONAL_H_
#define _DJVUBITONAL_H_

/** @name DjVuBitonalAPI.h

    @memo #DjVuBitonalAPI.h# defines the functions used to convert multiple
    bitonal images to DjVu multipage documents.
    @author
    Bill C Riemers 
*/

/*@{*/

/* 
 * $Log: DjVuBitonalAPI.h,v $
 * Revision 1.15  2000-07-05 16:50:53  bcr
 * Updated the documentation.
 *
 * Revision 1.14  2000/07/04 00:51:50  mrosen
 * updated documentation
 *
 * Revision 1.13  2000/03/09 22:27:59  bcr
 * Updated the documentation, again.
 *
 * Revision 1.12  2000/03/08 22:59:46  bcr
 * Updated the documentation.  I'm using Leon's libdjvu++ documentation
 * as a template.
 *
 * Revision 1.11  2000/03/05 18:13:37  bcr
 * More comment changes.
 *
 * Revision 1.10  2000/03/05 06:29:16  bcr
 * More documentation updates.
 *
 */

#include "DjVuDecodeAPI.h"

/* Predeclarations. */

/** ++ #bitonaltodjvu_type# is used to define the bitonal
    compression type.  Possible values consist of normal,
    conservative, lossless, aggressive, and pseudo.
*/
enum bitonaltodjvu_type_enum
{
  djvu_normal,
  djvu_conservative,
  djvu_lossless,
  djvu_aggressive,
  djvu_pseudo
};

typedef enum bitonaltodjvu_type_enum bitonaltodjvu_type;
 
struct djvu_parse;

#ifdef __cplusplus
extern "C"
{
#ifndef __cplusplus
};
#endif
#endif


/** @memo #djvu_jb2_options# fine tunes compression quality and speed.

    This structure defines the options used by \Ref{djvu_bitonal}.
    Each field correspends to command line options of the corresponding
    utility programs.
*/
struct djvu_jb2_options_struct
{
  /** #pages_per_dict# allows n number of pages to be matched together
      and joined with a common shared dictionary.  This value should never
      be too high or too low. Best values are between 10 to 20.  A value
      of 1 has the special meaning of not using a shared dictionary, and
      should be when fewer than ten or so pages are being processed, or
      when you expect very few matching shapes (e.g. hand drawn text). */
  int pages_per_dict;

  /** The #compression# field specifies how aggressive compression is.
      Values of lossless, normal, conservative, aggressive, and pseudo
      may be used.  #pseudo# has special meaning, that the data will
      not be JB2 compressed at all, but instead the G4 MMR data in the
      input file will be copied directly into the DjVu(tm) file. */
  bitonaltodjvu_type compression;

  /** Halftone detection is used for dithered images.  The default
      of zero, means halftone detection is on.  When the #disable_halftone#
      option is non-zero, the JB2 encoder will not check for dithering.
      If the document is not dithered, the result of #disable_halftone# is
      to reduce the compression time.  You should not set #disable_halftone#
      for documents that really are dithered. */
  int disable_halftone;

  /** #tolerance_percent# and #tolerance4_size# are more options
      for specifying the quality of JB2 compression.  Primarily these
      are used for tuning purposes, and normal users should not adjust
      these values, but instead use the compression mode above. */
  int tolerance_percent, tolerance4_size;

#ifdef __cplusplus
inline djvu_jb2_options_struct();
#endif /* __cplusplus */

};
typedef struct djvu_jb2_options_struct djvu_jb2_options;


/** @memo #bitonaltodjvu_options# lists options corresponding to
    \URL[bitonaltodjvu]{../SDKTools/bitonaltodjvu.html}.

    The values of the #bitonaltodjvu_options# correspond to
    the command line options of the \URL[bitonaltodjvu]{../SDKTools/bitonaltodjvu.html}
    utility.
*/

struct bitonaltodjvu_options_struct
{
  /** #process# contains the \Ref{djvu_process_options} which
      defines the pages to be parsed, input, and output, and contains the
      pointer for storing errors. */
  djvu_process_options process;

  /** #transform# contains the \Ref{djvu_transform_options}
      which  defines the transformations which take place before compressing
      each page. */
  djvu_transform_options transform;

  /** #jb2# contains the \Ref{djvu_jb2_options} which defines
      the JB2 compression options. */
  djvu_jb2_options jb2;

#ifdef __cplusplus
  inline bitonaltodjvu_options_struct();
#endif /* __cplusplus */

};
typedef struct bitonaltodjvu_options_struct bitonaltodjvu_options;


/** @name DjVuBitonalAPI C function calls
 */

/*@{*/

DJVUAPI
#if 0
;
#endif
/** ++ #bitonaltodjvu_options_alloc# is the primary allocation routine for 
    \Ref{bitonaltodjvu_options}.  If a \Ref{djvu_parse} structure has been
    declared, it may be passed to the #bitonaltodjvu_options_alloc# routine.
    Otherwise a NULL value should be passed as the parse value.  Even if the
    values specified are illegal, an options structure will be returned. */
bitonaltodjvu_options *
bitonaltodjvu_options_alloc(
  struct djvu_parse *parse,int argc,const char * const argv[]);

DJVUAPI
#if 0
;
#endif
/** ++ #bitonaltodjvu_options_free# deallocates the fields of the
    \Ref{bitonaltodjvu_options} structure.  You should always use the
    free option, even if you did not use alloc so the data pointed to by
    priv pointer is freed. */
void bitonaltodjvu_options_free(bitonaltodjvu_options *);

DJVUAPI
#if 0
;
#endif
/** ++ #bitonaltodjvu# converts the bitonal input files to a multipage
    DjVu document according to the options structure.
    Depending on the type of the input data, the function uses the
    a non-zero return value indicates a fatal error. */
int bitonaltodjvu(bitonaltodjvu_options[1]);

DJVUAPI
#if 0
;
#endif
/** ++ #bitonaltodjvu_haserror# returns a non-zero value indicates there are
    error messages on the stack.  Error messages are generated for both fatal
    and some non-fatal errors. */
int bitonaltodjvu_haserror(const bitonaltodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ #bitonaltodjvu_haswarning# returns a non-zero value indicates there are
    warning messages on the stack.  Warning messages are generated for both
    non-fatal problems, and some types of abnormal usage. */
int bitonaltodjvu_haswarning(const bitonaltodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ #bitonaltodjvu_error# pops the first error message off the stack.  Each
    call erases the previous return value. */
const char * bitonaltodjvu_error(bitonaltodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ #bitonaltodjvu_warning# pops the first warning message off the stack.
    Each call erases the previous return value. */
const char * bitonaltodjvu_warning(bitonaltodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ #bitonaltodjvu_perror# prints all the errors to stderr, and removes
  them from the stack.  If mesg is not NULL, the mesg string will be printed
  followed by a colon and a blank befor each error message.  */
void bitonaltodjvu_perror(bitonaltodjvu_options [1],const char *mesg);

DJVUAPI
#if 0
;
#endif
/** ++ This will print usage instructions to the specified fileno. */
void bitonaltodjvu_usage(int fd,const char *prog);

/*@}*/

#ifdef __cplusplus
#ifndef __cplusplus
{
#endif
};

inline djvu_jb2_options_struct::djvu_jb2_options_struct() :
  pages_per_dict(10), compression(djvu_normal), disable_halftone(0),
  tolerance_percent(-1), tolerance4_size(-1) {}

inline bitonaltodjvu_options_struct::bitonaltodjvu_options_struct() :
  process(), transform(), jb2() {}

#endif

/*@}*/

#endif /* _DJVUBITONAL_H_ */

