dnl  $Id: configure.ac,v 1.5 2001-08-24 21:50:10 docbill Exp $
dnl
dnl   Name:     libdjvu++-configure.ac
dnl
dnl   Purpose:  Autoconf initialization script to automatically generate 
dnl             configure script via Autoconf for libdjvu++.
dnl

dnl ---------------------------------------------------------------------------
dnl 
dnl   Section       Description
dnl   -------       -----------
dnl
dnl      1          Initialization and Setup.
dnl      2          Checks for programs.
dnl      3          Check for system.
dnl      4          Check and process command line options.
dnl      5          Define include paths and libraries.
dnl      6          Write output.
dnl

dnl                Checks for libraries
dnl                Checks for header files
dnl                Checks for types
dnl                Checks for structures
dnl                Checks for compiler characteristics
dnl                Checks for library functions
dnl                Checks for system services
dnl
dnl
dnl
dnl ---------------------------------------------------------------------------


dnl ---------------------------------------------------------------------------
dnl
dnl Section 1:  Initialization and Setup
dnl
dnl ---------------------------------------------------------------------------

dnl Require autoconf version 2.50.
AC_PREREQ(2.50)



dnl Process command-line arguments and perform initializations and verifications.
dnl Set package name and version.
AC_INIT(libdjvu, 3.0)

dnl Set the auxilary config directory.
dnl @Warning  This must be called prior to AM_INIT_AUTOMAKE to so that
dnl           automake will find install.sh in the the config directory.
AC_CONFIG_AUX_DIR(config)


dnl Initialize automake macros -- export AC's package name and version to AM.
AM_INIT_AUTOMAKE(@PACKAGE_NAME@, @PACKAGE_VERSION@) 

dnl Check--are we the correct place?
AC_CONFIG_SRCDIR(DjVuImage.h)


dnl Prepare platform specific header file (config.h).
dnl Note: Since using automake use AM_CONFIG_HEADER, instead of AC_CONFIG_HEADERS.
dnl @bug Do not activiate at present, since DjVu code base does not have
dnl      HAVE_CONFIG_H include guards in source files.  Pass UNIX via -Dflag
dnl      for present.
dnl AM_CONFIG_HEADER(config.h)

dnl Write CVS revision info to top of script file.
AC_REVISION($Id: configure.ac,v 1.5 2001-08-24 21:50:10 docbill Exp $)

dnl Write the copyright to the configure file.
AC_COPYRIGHT([                
C- -*- C -*-
C-
C- DjVu® Reference Library (v. 3.0)
C- 
C- Copyright © 2000 LizardTech, Inc. All Rights Reserved.
C- 
C- This software (the "Original Code") is subject to, and may be
C- distributed under, the GNU General Public License, Version 2.
C- The license should have accompanied the Original Code or you
C- may obtain a copy of the license from the Free Software
C- Foundation at http://www.fsf.org .
C- 
C- With respect to the Original Code, and subject to any third
C- party intellectual property claims, LizardTech grants recipient
C- a worldwide, royalty-free, non-exclusive license under patent
C- claims infringed by making, using, or selling Original Code
C- which are now or hereafter owned or controlled by LizardTech,
C- but solely to the extent that any such patent is reasonably
C- necessary to enable you to make, have made, practice, sell, or 
C- otherwise dispose of Original Code (or portions thereof) and
C- not to any greater extent that may be necessary to utilize
C- further modifications or combinations.
C- 
C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
])


dnl ---------------------------------------------------------------------------
dnl
dnl Section 2:  Check for programs.
dnl
dnl ---------------------------------------------------------------------------

dnl Check for c compiler. This required only for libjpeg.
AC_PROG_CC

dnl Check for c++ compiler.
AC_PROG_CXX

dnl Check for libtool.
AC_PROG_LIBTOOL

dnl Do check tests in c++.
AC_LANG(C++) 


dnl Export echo macros to Autoamke.
AC_SUBST(ECHO_N)
AC_SUBST(ECHO_C)
AC_SUBST(ECHO_T)


dnl ---------------------------------------------------------------------------
dnl
dnl Section 4:  Check for system.
dnl
dnl ---------------------------------------------------------------------------


dnl @todo Move DEFUN of AC_UNIX to a DjVu macro (.m4) in m4 directory.
dnl @todo What happens if we are using gcc on a cygwin?
dnl @todo How do we configure for a unix target box?
#
# AC_UNIX: Function to determine if on unix and define UNIX.
#          Notes:
#          gcc defines the macro: __unix__.
#          SUNPRO_CC defines the macros:  __unix, unix.
#          SUNPRO_C  defines the macro:__unix.
# ------
AC_DEFUN([AC_UNIX],
[AC_BEFORE([AC_COMPILE_IFELSE], [$0])dnl
AC_BEFORE([$0], [AC_RUN_IFELSE])dnl
AC_MSG_CHECKING([whether on a UNIX platform ([__unix__, __unix, unix])])
AC_EGREP_CPP(yes,
[#ifdef __unix__
  yes
#endif
#ifdef __unix
  yes
#endif
#ifdef unix
  yes
#endif
],
[AC_MSG_RESULT([yes])
AC_DEFINE(UNIX, 1, [Define if on a UNIX platform] )],
[AC_MSG_RESULT([no])])
])# AC_UNIX

dnl @bug Debug code: Use this DEFINE to UNIX without using the AC_UNIX macro.
dnl AC_DEFINE(UNIX, 1, [Define if on a UNIX platform])

dnl Check if this is a unix platform.
AC_UNIX

dnl ---------------------------------------------------------------------------
dnl
dnl Section 4:  Check and process command line options.
dnl
dnl ---------------------------------------------------------------------------

dnl AC_ARG_ENABLE(threads, [ --with-libjpeg 

AC_ARG_WITH(threads,
            AC_HELP_STRING([--with-threads=(no|jri|co|posix|dce)],
                           [use threads implemenation (default is NO)]),
            ac_cv_use_threads=$withval,
            ac_cv_use_threads=no)

AC_ARG_WITH(libjpeg, 
            AC_HELP_STRING([--with-libjpeg],
                           [use libjpeg decoder (bulky) (default is YES)]),
            ac_cv_use_libjpeg=$withval, 
            ac_cv_use_libjpeg=yes)


dnl ---------------------------------------------------------------------------
dnl
dnl Section 5:  Define include paths and libraries.
dnl
dnl ---------------------------------------------------------------------------
            
DJVUAPI_INCDIR='$(top_srcdir)/../include'
AC_SUBST(DJVUAPI_INCDIR)

LIBDJVUXX='libdjvu.la'
LIBDJVUXX_INCDIR='$(top_srcdir)/../libdjvu++'
LIBDJVUXX_BUILDDIR='$(top_builddir)/../libdjvu++'
LIBDJVUXX_LDPATH='$(LIBDJVUXX_BUILDDIR)'

AC_SUBST(LIBDJVUXX)
AC_SUBST(LIBDJVUXX_INCDIR)
AC_SUBST(LIBDJVUXX_BUILDDIR)
AC_SUBST(LIBDJVUXX_LDPATH)


all_includes='-I$(DJVUAPI_INCDIR) -I$(LIBDJVUXX_INCDIR)'
AC_SUBST(all_includes)

dnl all_includes='-I$(top_srcdir)/dcop -I$(top_srcdir)/libltdl -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdeui -I$(top_srcdir)/kssl $(QT_INCLUDES) $(X_INCLUDES) $(KDE_INCLUDES) $(USER_INCLUDES)'
dnl AC_SUBST(all_includes)


dnl ---------------------------------------------------------------------------
dnl
dnl Section 6:  Write output
dnl
dnl ---------------------------------------------------------------------------

dnl List the output files to write.
AC_CONFIG_FILES([ \
	Makefile \
	Test/Makefile ])

dnl Write output - The last command.
AC_OUTPUT                     




