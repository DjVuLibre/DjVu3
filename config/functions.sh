#!/bin/ksh
#C-
#C- DjVu® Reference Library (v. 3.5)
#C- 
#C- Copyright © 2000-2001 LizardTech, Inc. All Rights Reserved.
#C- The DjVu Reference Library is protected by U.S. Pat. No.
#C- 6,058,214 and patents pending.
#C- 
#C- This software is subject to, and may be distributed under, the
#C- GNU General Public License, Version 2. The license should have
#C- accompanied the software or you may obtain a copy of the license
#C- from the Free Software Foundation at http://www.fsf.org .
#C- 
#C- The computer code originally released by LizardTech under this
#C- license and unmodified by other parties is deemed the "LizardTech
#C- Original Code."
#C- 
#C- With respect to the LizardTech Original Code ONLY, and subject
#C- to any third party intellectual property claims, LizardTech
#C- grants recipient a worldwide, royalty-free, non-exclusive license
#C- under patent claims now or hereafter owned or controlled by
#C- LizardTech that are infringed by making, using, or selling
#C- LizardTech Original Code, but solely to the extent that any such
#C- patent(s) is/are reasonably necessary to enable you to make, have
#C- made, practice, sell, or otherwise dispose of LizardTech Original
#C- Code (or portions thereof) and not to any greater extent that may
#C- be necessary to utilize further modifications or combinations.
#C- 
#C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
#C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
#C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
#C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id: functions.sh,v 1.73 2001-08-08 19:02:17 docbill Exp $
# $Name:  $

#
# Support functions for configuration scripts
# Configuration scripts should source this file 
# and call the relevant functions.
#
# The basic variables needed for all configuration files are preset,
# and the names are stored in "$CONFIG_VARS".  The previous config.cache
# file is also read.
#
# The functions defined are:
#
# echon ...
#   Print the arguments to standard output without a line feed.
#
# pathclean PATH
#   Replace things like /./ with / in the PATH specified.
#
# sortlist LIST
#   Sorts each element of the white space separated list in alphabetical order.
#
# mkdirp PATH
#   Equivalent to the GNU command "mkdir -p PATH"
#
# testfile FILENAME
#   Output standard input to the specified FILENAME and the logfile.
#
# run COMMAND
#   Evaluates the specified command, and writes the output to $temp.out and
#   the logfile.
#
# get_option_value OPTION
#   Extracts the value of a long option string.  For example the following
#   would return "fi".
#      get_option_value --foo=fi
#
# process_general_option OPTION
#   This processes the options --prefix=*,--with-debug,-g,--with-flag=*
#
# list_general_options
#   Outputs usage instructions for general options to standard error.
#
# check_compiler
#   Queries the C and C++ compilers
#
# require_compiler
#   Queries the C and C++ compilers, and exits if not found.
#
# check_debug_option ARG
#   Seems to be a dummy function that is the same as:
#	process_general_option -g
#   if and only if a none empty argument is supplied.
#
# check_thread_option
#   (to be documented)
#   
# check_rpo_option
#   (to be documented)
#
# check_library
#   (to be documented)
#
# require_library
#   Same as check_library, but exits with error if a library is not found.
#
# check_make_stlib
#   (to be documented)
#
# require_make_stlib
#   (to be documented)
#
# generate_makefile
#   (to be documented)
#
# generate_main_makefile
#   (to be documented)
#
# finish_config
#   Writes the reconfigure script and cache file.


# --- prefix for temporary files
tempdir=/tmp
temp="${tempdir}"/c$$

# --- Make sure that all temp files are removed
trap "rm -rf 2>>/dev/null $temp $temp.*" 0

if [ -z "" -a -z "broken"  ]
then
  echo 2>&1 "Your /bin/sh is broken.  Please upgrade."
  echo 2>&1 "Old versions of bash also contain bug which"
  echo 2>&1 "may allow hackers to crack your system."
  exit 1
fi

# --- Save command line options
CONFIGURATION_OPTIONS="$*"


### ------------------------------------------------------------------------
### Support functions

# Usage: echon ...ARGUMENTS...
if [ "`echo -n`" = "-n" ]
then
  echon() { 
    echo "$*" "\c" 
  }
else
  echon() { 
    echo -n "$*"
  }
fi


# Renmove excess /./ in pathnames
pathclean()
{
    echo $1 | sed \
                -e 's://:/:g' \
                -e 's:/\./:/:g' \
                -e 's:\(.\)/\.$:\1:g'
}

# Sort a list and make sure the elements are unique
sortlist()
{
  (for i 
  do
    echo "$i"
  done)|${sort} -u
}

nonl()
{
  echo $*
}

# Escape characters.
escape()
{
  nonl `(for i
  do
    echo "$i"
  done)|${sed} -e 's,@%%@,@%%@p,g' -e 's,!,@%%@e,g' -e 's, ,@%%@s,g' -e 's,	,@%%@t,g' -e 's,\\$,@%%@d,g' -e 's,",@%%@q,g' -e 's,'"'"',@%%@a,g' -e 's,\\\\,@%%@z,g'`
}

unescape()
{
  (for i
   do
    echo "$i"
  done)|sed  -e 's,@%%@a,'"'"',g' -e 's,@%%@q,",g' -e 's,@%%@d,\$,g' -e 's,@%%@t,	,g' -e 's,@%%@s, ,g' -e 's,@%%@e,!,g' -e 's,@%%@p,@%%@,g' -e 's,@%%@z,\\,g'
}

# Make a test file with echo on CONFIG_LOG
testfile()
{
    echo "------- ($*)" >> "$CONFIG_LOG"
    "${tee}" "$*"            >> "$CONFIG_LOG"
}


# Run command with echo on CONFIG_LOG
run()
{
    ln -f -s `unescape $TOPBUILDDIR` "$tempdir/TOPDIR"
    ( cd "$tempdir" 2>>/dev/null 1>>/dev/null; $* ) > $temp.out 2>&1 
    status=$?
    echo "------- (cmd)"   >> "$CONFIG_LOG"
    echo '%' "$*"          >> "$CONFIG_LOG"
    "${cat}" "$temp.out"        >> "$CONFIG_LOG"
    rm -f "$tempdir/TOPDIR"
    return $status
}



### ------------------------------------------------------------------------
### Process options


# Usage: get_option_value OPTION
#   Parse options like '--with-threads=xxxx' and return value

get_option_value()
{
# This won't work with many types of Bourne Shell.  I need to fix it.
#
    if [ `expr "$*" : '[-A-Za-z_]*='` = 0 ]
    then
      echo yes
    else
      echo "$*" | "${sed}" 's/^[-A-Za-z_]*=//'
    fi
}



# Usage: process_general_option OPTION
#   Process general purpose options
#   Returns 0 exit status if option is recognized

process_general_option()
{
  case "$*" in
        --prefix=* )
            prefix=`get_option_value "$*"`
            ;; 
        --with-debug|-g )
            C_OPT=REPLACE
            C_CXXOPT=REPLACE
            C_CCOPT=REPLACE
            R_CXXOPT='-g'
            R_CCOPT='-g'
            R_OPT=''
            append_defs DEBUG 1
            HAS_DEBUG=true
#            C_DEFS=APPEND
#            A_DEFS="-DDEBUG $A_DEFS"
            ;;
        --with-flag=* )
            C_DEFS=APPEND
            s=`get_option_value "$*"`
            A_DEFS=`escape "$s"`" $A_DEFS"
            ;;
        *)
            # Do not complain for empty option
            [ x$* != x ] && return 1;
            ;;
  esac
  return 0
}


# Usage: list_general_options
#  List general options on stderr

list_general_options()
{
"${cat}" 1>&2 <<EOF
  --prefix=PREFIXDIR                        set install directory.
  --with-debug, -g                          compile with debug enabled.
  --with-flag=FLAG                          pass specific optimization flag
EOF
} 



### ------------------------------------------------------------------------
### Macro definition functions.

add_undefs()
{
  while [ -n "$1" ]
  do
    Dname="$1"
    shift
    echo '#ifdef '"$Dname" >> "$CONFIG_H"
    echo '#undef '"$Dname" >> "$CONFIG_H"
    echo '#endif ' >> "$CONFIG_H"
    echo '#ifdef '"$Dname" >> "$CONFIG_H_CACHE"
    echo '#undef '"$Dname" >> "$CONFIG_H_CACHE"
    echo '#endif ' >> "$CONFIG_H_CACHE"
  done
}

add_defs()
{
  while [ -n "$1" ]
  do
    Dname="$1"
    shift
    Dvalue="$1"
    shift
    add_undefs "$Dname"
    echo '#define '"$Dname $Dvalue" >> "$CONFIG_H"
    echo '#define '"$Dname $Dvalue" >> "$CONFIG_H_CACHE"
  done
}

append_undefs()
{
  while [ -n "$1" ]
  do
    Dname="$1"
    shift
    echo '#ifdef '"$Dname" >> "$CONFIG_H"
    echo '#undef '"$Dname" >> "$CONFIG_H"
    echo '#endif ' >> "$CONFIG_H"
  done
}
  
append_defs()
{
  while [ -n "$1" ]
  do
    Dname="$1"
    shift
    Dvalue="$1"
    shift
    append_undefs "$Dname"
    echo '#define '"$Dname $Dvalue" >> "$CONFIG_H"
  done
}



### ------------------------------------------------------------------------
### Check compiler

# Usage: check_compiler
# Side effect:  $CC              <-- name of a working C compiler
#               $CXX             <-- name of a working C++ compiler

check_compiler()
{
  if [ -z "$CC_SET" -o -z "$CXX_SET" ]
  then
    compilers="$*"
    if [ -z "$compilers" ]
    then
      compilers="cxx cc"
    fi
    LIBS=
    CONFIG_VARS=`echo LIBS $CONFIG_VARS`
    for i in $compilers
    do
      case $i in 
      c|C|cc|CC)
        if [ -z "$CC_SET" ]
        then
          . ${CONFIG_DIR}/cc.sh
        fi
        ;;
      [Cc][Xx+][Xx+])
        if [ -z "$CXX_SET" ]
        then
          . ${CONFIG_DIR}/cxx.sh
        fi
        ;;
      esac
    done
  fi
}

require_compiler()
{
  if [ -z "$CC_SET" -o -z "$CXX_SET" -o -z "$CCPIC_SET" -o -z "$CXXPIC_SET" ]
  then
    need_shared=
    compilers="$*"
    if [ -z "$compilers" ]
    then
      compilers="cc cxx"
    fi
    LIBS=
    CONFIG_VARS=`echo LIBS $CONFIG_VARS`
    compiler_is_gcc=yes
    for i in $compilers
    do
      case $i in 
      c|C|cc|CC)
        if [ -z "$CC_SET" ]
        then
          . ${CONFIG_DIR}/cc.sh
          if [ -z "$cc_is_gcc" ] ; then compiler_is_gcc= ; fi
          if [ -z "$CC" ]
          then
            echo 1>&2 "${PROGRAM_NAME}: C compiler not found."
            exit 1
          fi
        fi
        ;;
      [Cc][Xx+][Xx+])
        if [ -z "$CXX_SET" ]
        then
          . ${CONFIG_DIR}/cxx.sh
          if [ -z "$cxx_is_gcc" ] ; then compiler_is_gcc= ; fi
          if [ -z "$CXX" ]
          then
            echo 1>&2 "${PROGRAM_NAME}: C++ compiler not found."
            exit 1
          fi
        fi
        ;;
      [Ss][Hh][Aa][Rr][Ee][Dd])
        need_shared=true
        ;;
      [Ss][Tt][Aa][Tt][Ii][Cc])
        need_shared=
        ;;
      esac
    done
    if [ -n "$need_shared" ]
    then
      if [ -n "$CC_SET" -a -z "$CCPIC_SET" ]
      then
        . ${CONFIG_DIR}/ccpic.sh
      fi
      if [ -n "$CXX_SET" -a -z "$CXXPIC_SET" ]
      then
        . ${CONFIG_DIR}/cxxpic.sh
      fi
    else
      CCPIC_SET=
      CCPIC=
      CCSYMBOLIC=
      CXXPIC_SET=
      CXXPIC=
      CXXSYMBOLIC=
    fi
  fi
}

### ------------------------------------------------------------------------
### Check debug option

# Usage: check_debug_option DEBUGFLAG
# Side effect:  $OPT <-- replaced with -g


check_debug_option()
{
  if [ -z "$*" ] ;
  then
    append_defs NO_DEBUG 1
    return
  fi
  # We could test that one ...
  C_CCOPT=REPLACE
  C_CXXOPT=REPLACE
  C_OPT=REPLACE
  R_CCOPT="-g"
  R_CXXOPT="-g"
  R_OPT=""
  append_defs DEBUG 1
  HAS_DEBUG=true
}



### ------------------------------------------------------------------------
### Check thread option


# Usage: check_thread_option THREADFLAG
# Side effects: changes C/A_DEFS, C/A_LIBS, C/A_CXXFLAGS, C/A_CCFLAGS
# Also introduces CXXTHREADS, CCTHREADS, DEFSTHREADS and LIBSTHREADS
# because w/o them I (eaf) can't configure djview and nsdejavu.so in on
# configuration script

check_thread_option()
{
  if [ -z "$1" ] ; then return; fi
  case "$1" in
    yes )
       echo 2>&1 "${PROGRAM_NAME}: Autodetermination of thread model is not yet implemented."
       echo 2>&1 "-- Please specify one of nothreads, cothreads, posixthreads."
       exit 1
       ;;
    no* )
       DEFSTHREADS="-DTHREADMODEL=NOTHREADS"
       ;;       
    jri* )
       DEFSTHREADS="-DTHREADMODEL=JRITHREADS"
       ;;       
    co* )
       if [ ! -z "$CXX" ]
       then
         if [ -z "$CXXCOTHREAD_TEST" ]  ; then
           . "${CONFIG_DIR}/cothread.sh"
         fi
         if [ -z "$CXXCOTHREAD" ]
         then
           echo 1>&2 "${PROGRAM_NAME}: Cothreads does not work with ${CXX}."
           echo 1>&2 "You need GNU g++ or a derivative such as pgcc or egcs."
           exit 1
         fi
	 CXXTHREADS="${CXXCOTHREAD}"
       fi
       if [ ! -z "$CC" ]
       then
         if [ -z "$CCCOTHREAD_TEST" ]  ; then
           . "${CONFIG_DIR}/cothread.sh"
         fi
         if [ -z "$CCCOTHREAD" ]
         then
           echo 1>&2 "${PROGRAM_NAME}: Cothreads does not work with ${CC}."
           echo 1>&2 "You need GNU gcc or a derivative such as pgcc or egcs."
           exit 1
         fi
	 CCTHREADS="${CCCOTHREAD}"
       fi
       if [ ! -z "$CXXCOTHREAD_UNSAFE$CCCOTHREAD_UNSAFE" ] 
       then
          echo 1>&2 "${PROGRAM_NAME}: Using COTHREADS without the patch is unsafe."
          echo 1>&2 "-- See documentation for libdjvu++."
       fi
       ;;       
    posix* | dce* )
       if [ ! -z "$CC" ]
       then
         if [ -z "$CCPTHREAD" ]
         then
           . "${CONFIG_DIR}/pthread.sh"
         fi
	 CCTHREADS="$CCPTHREAD"
       fi
       if [ ! -z "$CXX" ]
       then
         if [ -z "$CXXPTHREAD" ]
         then
           . "${CONFIG_DIR}/pthread.sh"
         fi
	 CXXTHREADS="$CXXPTHREAD"
       fi
       if [ ! -z "${CCPTHREAD_LIB}${CXXPTHREAD_LIB}" ]
       then 
	 LIBSTHREADS="${CXXPTHREAD_LIB}"
       fi
       ;;
    *)
       echo 1>&2 "${PROGRAM_NAME}: unrecognized multithreading option."
       exit 1
       ;;
  esac
  CONFIG_VARS="$CONFIG_VARS CCTHREADS CXXTHREADS DEFSTHREADS LIBSTHREADS"
}

check_dlopen()
{
  if [ -z "$DLOPEN_TEST" ]  ; then
    . "${CONFIG_DIR}/dlopen.sh"
  fi
}

check_iconv()
{
  if [ -z "$ICONV_TEST" ]  ; then
    . "${CONFIG_DIR}/iconv.sh"
  fi
}

### ------------------------------------------------------------------------
### Check repository option


# Usage: check_rpo_option RPOFLAG
# Side effects:  $RPO <-- name of rpo program
#                $OPT <-- updated to use -frepo


check_rpo_option()
{
  if [ -z "$1" ] ; then return; fi
  if [ ! -z "$CXX" ]
  then
    if [ -z "$CXXRPO_TEST" ]
    then
      . "${CONFIG_DIR}/rpo.sh"
    fi
    if [ -z "$CXXRPO" ]
    then
      echo 2>&1 "${PROGRAM_NAME}: The ${CXX} option '-frepo' is not supported."
      exit 1
    fi
    C_CXXOPT=APPEND
    A_CXXOPT="$CXXRPO $A_CXXOPT"
  fi
  if [ ! -z "$CC" ]
  then
    if [ -z "$CCRPO_TEST" ]
    then
      . "${CONFIG_DIR}/rpo.sh"
    fi
    if [ -z "$CCRPO" ]
    then
      echo 2>&1 "${PROGRAM_NAME}: The ${CC} option '-frepo' is not supported."
      exit 1
    fi
    C_CCOPT=APPEND
    A_CCOPT="$CCRPO $A_CCOPT"
  fi
}

### ------------------------------------------------------------------------
### Check compile and link flags

# Usage: check_compile_flags NAME tmpfile.c|.cpp ALTERNATIVE_COMPILE_FLAGS
# Side effect: Update variable <NAME> and <NAME>_paths

check_compile_flags()
{
  BBname="$1"
  shift
  BBtmpfile="$1"
  shift
  if [ "x$1" = "x@%%@" ]
  then
    shift
    BBargs="$*"
  else
    BBargs=""
    for s
    do
      s=`unescape "$s"`
      if [ -z "$BBargs" ]
      then
        BBargs="`escape $s`"
      else
        BBargs="$ACargs `escape $s`"
      fi
    done
  fi
  s='echo $'"${BBname}_paths"
  s=`eval $s`
  if [ "x$s" != "x$BBargs" ] ; then
    eval "${BBname}_paths='$BBargs'"
    CONFIG_VARS=`echo ${BBname}_paths ${BBname} $CONFIG_VARS`
    case $BBtmpfile in 
    *.c)
      for BBflags in $BBargs
      do
        flags=`unescape "$BBflags"`
        run "$CC" $CCFLAGS -c $BBtmpfile $CCOPT $CCWARN $DEFS $BBflags
        if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
        then
          eval "${BBname}='"`escape "$BBflags"`"'"
          return 0
        fi
      done
      ;;
    *.cpp)
      for BBflags in $BBargs
      do
        run "$CXX" -c $BBtmpfile $CXXFLAGS $CXXOPT $CXXWARN $DEFS $BBflags
        if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
        then
          eval "${BBname}='"`escape "$BBflags"`"'"
          return 0
        fi
      done
      ;;
    esac
    return 1;
  fi
  return 0;
}

# Usage: check_link_flags NAME tmpfile.c|.cpp ALTERNATIVE_COMPILE_FLAGS
# Side effect: Update variable <NAME> and <NAME>_paths

check_link_flags()
{
  AAname="$1"
  shift
  AAtmpfile="$1"
  shift
  if [ "x$1" = "x@%%@" ]
  then
    shift
    AAargs="$*"
  else
    AAargs=""
    for s
    do
      s=`unescape "$s"`
      if [ -z "$AAargs" ]
      then
        AAargs="`escape $s`"
      else
        AAargs="$AAargs `escape $s`"
      fi
    done
  fi
  s='echo $'"${AAname}_paths"
  s=`eval $s`
  if [ "x$s" != "x$AAargs" ] ; then
    eval "${AAname}_paths"'="$AAargs"'
    CONFIG_VARS=`echo ${AAname}_paths ${AAname} $CONFIG_VARS`
    case $AAtmpfile in 
    *.c)
      for AAflags in $AAargs
      do
        AAflags=`unescape "$AAflags"`
        if ( run "$CC" $AAtmpfile $CCFLAGS $CCOPT $CCWARN $DEFS $AAflags -o $temp ) 
        then

          eval "${AAname}='"`escape "$AAflags"`"'"
          return 0
        fi
      done
      ;;
    *.cpp)
      for AAflags in $AAargs
      do
        AAflags=`unescape "$AAflags"`
        if ( run "$CXX" $AAtmpfile $CXXFLAGS $CXXOPT $CXXWARN $DEFS $AAflags -o $temp ) 
        then
          eval "${AAname}='"`escape "$AAflags"`"'"
          return 0
        fi
      done
      ;;
    esac
    return 1
  fi
  s='echo $'"${AAname}"
  if [ -z "`eval $s`" ] 
  then
    return 1
  else
    return 0
  fi
}

check_shared_link_flags()
{
  AAname="$1"
  shift
  AAtmpfile="$1"
  shift
	AAflags="$*"
	shift

    CONFIG_VARS=`echo ${AAname} $CONFIG_VARS`
    case $AAtmpfile in 
    *.c)
        if ( run "$CC" $CCFLAGS $CCOPT $CCWARN $DEFS $AAflags $AAtmpfile -o $temp ) 
        then

          eval "${AAname}='$AAflags'"
          return 0
        fi
      ;;
    *.cpp)
        if ( run "$CXX" $CXXFLAGS $CXXOPT $CXXWARN $DEFS $AAflags $AAtmpfile -o $temp ) 
        then
          eval "${AAname}='$AAflags'"
          return 0
        fi
      ;;
    esac
  s='echo $'"${AAname}"
  if [ -z "`eval $s`" ] 
  then
    return 1
  else
    return 0
  fi
}

### ------------------------------------------------------------------------
### Check library

# Usage: check_library NAME FUNCTION ...ALTERNATIVE_LINKSPEC...
# Side effect: Update variable lib<NAME>


check_library()
{
  ABname="$1"
  shift
  ABfunc="$1"
  shift
  if [ "x$1" = "x@%%@" ]
  then
    shift
    ABargs="$*"
  else
    ABargs=""
    for s
    do
      s=`unescape "$s"`
      if [ -z "$ABargs" ]
      then
        ABargs="`escape $s`"
      else
        ABargs="$ABargs `escape $s`"
      fi
    done
  fi
  s='echo $'"lib${ABname}_paths"
  s=`eval $s`
  if [ "x$s" != "x$ABargs" ] ; then
    echon "Searching library containing ${ABfunc}() ... "
    if [ -z "$CXX" ] 
    then
      testfile $temp.c <<EOF
int ${ABfunc}(void);
int main(void) { return ${ABfunc}(); }
EOF
      check_link_flags "lib${ABname}" $temp.c @%%@ $ABargs
    else
      testfile $temp.cpp <<EOF
extern "C" int ${ABfunc}(void);
int main(void) { return ${ABfunc}(); }
EOF
      check_link_flags "lib${ABname}" $temp.cpp @%%@ $ABargs
    fi
    if [ $? = 0 ] ; then
      s='echo $lib'"${ABname}"
      eval $s
      return 0
    fi
    echo "not found in $ABargs."
    return 1;
  fi
  s='echo $lib'"${ABname}"
  if [ -z "`eval $s`" ] 
  then
    return 1
  else
    return 0
  fi
}

# Usage: require_library NAME FUNCTION ...ALTERNATIVE_LINKSPEC...
# Side effect: Update variable lib<NAME>

require_library()
{
  ACname="$1"
  shift
  ACfunc="$1"
  shift
  if [ "x$1" = "x@%%@" ]
  then
    shift
    ACargs="$*"
  else
    ACargs=""
    for s
    do
      s=`unescape "$s"`
      if [ -z "$ACargs" ]
      then
        ACargs="`escape $s`"
      else
        ACargs="$ACargs `escape $s`"
      fi
    done
  fi
  check_library "$ACname" "$ACfunc" @%%@ $ACargs
  if [ $? = 0 ] 
  then
    return 0
  fi
  echo 2>&1 "${PROGRAM_NAME}: Function ${ACfunc}() not found."
  exit 1
}

# Usage: search_for_library NAME FUNCTION ...ALTERNATIVE_LIBRARY_FILENAMES...
# Side effect: Update variable lib<NAME>

search_for_library()
{
  ADname="$1"
  shift
  ADfunc="$1"
  shift
  for i
  do
    if [ `"${basename}" "x$i" .a` = `"${basename}" "x$i"` ] 
    then
      if [ `echo x$i|sed 's,^\(x.\).*,\1,'` = 'x-' ]
      then
        check_library "$ADname" "$ADfunc" @%%@ "$i"
        if [ $? = 0 ]
        then
          return 0
        fi
      else
        check_library "$ADname" "$ADfunc" @%%@ `echo "/$i"|sed -e "s,//,/,g"`
        if [ $? = 0 ]
        then
          return 0
        fi
        check_library "$ADname" "$ADfunc" @%%@ `echo $SOPATHS" "|sed -e "s, ,/$i ,g" -e "s,//,/,g"`
        if [ $? = 0 ]
        then
          return 0
        fi
      fi
    else
      check_library "$ADname" "$ADfunc" @%%@ `echo "/$i"|sed -e "s, ,/$i ,g" -e 's,//,/,g' -e 's, $,,'`
      if [ $? = 0 ]
      then
        return 0
      fi
    fi
  done
  return 1
}

### ------------------------------------------------------------------------
### Searching how to make archives


# Usage: check_make_stlib
# Side effect:  make_stlib <-- how to make a static library
#               RANLIB     <-- how to prepare a static library

check_make_stlib()
{
  if [ -z "$make_stlib_test" ]
  then
    make_stlib_test=checked
    if [ -z "$MAKE_STLIB" ]
    then
      make_stlib="${CONFIG_DIR}/make_stlib.sh" 
    else
      make_stlib="$MAKE_STLIB" 
    fi
    if [ ! -z "$CC$CXX" ]
    then
      echon Searching how to build a static library ...
#       if [ -z "$CC" ]
#       then
#         testfile $temp.cpp <<EOF
# int main(void) { return 1; }
# EOF
#         run "$CXX" $CCFLAGS $OPT $DEFS $WARN -c $temp.cpp
#       else
#         testfile $temp.c <<EOF
# int main(void) { return 1; }
# EOF
#         run "$CC" $CCFLAGS $OPT $DEFS $WARN -c $temp.c
#       fi
#       if ( run "${make_stlib}" $temp.a $temp.o ) 
#       then
        echo "${make_stlib}"
#       else
#         make_stlib=""
#       fi
      CONFIG_VARS=`echo make_stlib make_stlib_test $CONFIG_VARS`
    fi
  fi
}

# Usage: check_make_shlib
# Side effect:  make_stlib <-- can a shared library be build

check_make_shlib()
{
  if [ -z "$make_shlib_test" ]
  then
    make_shlib_test=checked
    if [ -z "$CXXSYMBOLIC$CCSYMBOLIC" -o -z "CXXPICCCPIC" ]
    then
     	make_shlib=""
    else
      make_shlib="supported"
    fi
    CONFIG_VARS=`echo make_shlib make_shlib_test $CONFIG_VARS`
  fi
}

require_make_stlib()
{
  check_make_stlib $*
  if [ -z "$make_stlib" ]
  then
    echo unknown.
    echo 1>&2 "${PROGRAM_NAME}: Cannot find how to make a static library."
    echo 1>&2 "-- Please set environment variable make_stlib or AR."
    exit 1
  fi
}

require_make_shlib()
{
  check_make_shlib $*
  if [ -z "$make_shlib" ]
  then
    echo unknown.
    echo 1>&2 "${PROGRAM_NAME}: Cannot find how to make a shared library."
    echo 1>&2 "-- Please set environment variable make_shlib or CXXSYMBOLIC."
    echo 1>&2 "-- or CCSYMBOLIC."
    exit 1
  fi
}



### ------------------------------------------------------------------------
### Function to generate sub makefiles



# Usage: generate_makefile DIRNAME
# Side effect: Generate DIRNAME/Makefile from SRCDIR/DIRNAME/Makefile.in

generate_makefile()
{
  TOPSRCDIR=`unescape $TOPSRCDIR`
  TOPBUILDDIR=`unescape $TOPBUILDDIR`
  ${mkdirp} "$TOPBUILDDIR/bin"
  ${mkdirp} "$TOPBUILDDIR/profiles/modules"
  ${mkdirp} "$TOPBUILDDIR/lib"
  ${mkdirp} "$TOPBUILDDIR/text"
  ${mkdirp} "$TOPBUILDDIR/html"
  ${mkdirp} "$TOPBUILDDIR/man"
  while [ -n "$1" ]
  do
    # compute xtopsrcdir
    # compute xsrcdir
    xsrcdir=$TOPSRCDIR/$1

    # make nice pathnames
    xsrcdir=`pathclean $xsrcdir`

    # substitute
    ${mkdirp} "$TOPBUILDDIR/$1"
    if [ -z "$WROTE_STATUS" ]
    then
      . "${CONFIG_DIR}/write_status.sh"
    fi
    echo "Creating: $TOPBUILDDIR/$1/Makefile"
    echo "	from  $TOPSRCDIR/$1/Makefile.in"
    run "${CONFIG_STATUS}" "$TOPSRCDIR/$1/Makefile.in" "$TOPBUILDDIR/$1/Makefile"
 
    shift
  done
}

### ------------------------------------------------------------------------
### Function to generate main makefiles




# Usage: cat fragement | generate_main_makefile  SUBDIRS

generate_main_makefile()
{
    subdirs=$*

    echo "Writing $TOPBUILDDIR/Makefile"
    # Generate Makefile header
    "${sed}" -e 's,x$,\\,g' > "$TOPBUILDDIR/Makefile" <<EOF
SHELL=${smartshell}
TOPSRCDIR=`unescape $TOPSRCDIR`
TOPBUILDDIR=`unescape $TOPBUILDDIR`
SUBDIRS= x
EOF
    while [ -n "$1" ] 
    do
      if [ -n "$2" ]
      then
        echo "'	`unescape $1`' \\" >> "$TOPBUILDDIR/Makefile"
      else
        echo "'	`unescape $1`'" >> "$TOPBUILDDIR/Makefile"
      fi
      shift
    done

    # Insert Makefile fragment
    "${cat}" >> "${TOPBUILDDIR}/Makefile"

    # Add final rules
    "${cat}" >> "${TOPBUILDDIR}/Makefile" <<\EOF

clean:
	for n in $(SUBDIRS) ; do ( cd $$n ; $(MAKE) clean ) ; done

html:
	for n in $(SUBDIRS) ; do ( cd $$n ; $(MAKE) html ) ; done

update-depend:
	for n in $(SUBDIRS) ; do ( cd $$n ; $(MAKE) update-depend ) ; done

depend:
	for n in $(SUBDIRS) ; do ( cd $$n ; $(MAKE) depend ) ; done

PHONY: all install clean html depend
EOF
}

generate_main_makefile_extra()
{

    # Insert Makefile fragment
    "${cat}" >> "${TOPBUILDDIR}/Makefile"
}

finish_config()
{
  if [ -z "$WROTE_STATUS" ]
  then
    . "${CONFIG_DIR}/write_status.sh"
  fi
  . ${CONFIG_DIR}/write_cache.sh
  . ${CONFIG_DIR}/write_reconfig.sh
}

# We always need to know the program name and directory.
#
if [ -z "$PROGRAM" ] ; then
  PROGRAM="$0"
fi
PROGRAM_NAME=`basename "$PROGRAM"`

# Now set the CONFIG_DIR variable.
#

if [ -z "$CONFIG_DIR" ] ; then
  CONFIG_DIR=`dirname "$PROGRAM"`/config
  CONFIG_DIR=`cd "$CONFIG_DIR" 2>>/dev/null 1>>/dev/null;pwd`
  CONFIG_VARS=`echo CONFIG_DIR ${CONFIG_VARS}`
fi

# Next we can read in the available commands
#

if [ -z "$whence" ] ; then
  . "${CONFIG_DIR}/commands.sh"
fi

# Next we need to set the SYS variable.
#

if [ -z "$SYS_SET" ] ; then
  . "${CONFIG_DIR}"/sys.sh
fi

# Now we are ready to read the cache file.
#
. ${CONFIG_DIR}/read_cache.sh

# Next we can read in the available commands agian, if needed
#

if [ -z "$whence" ] ; then
  . "${CONFIG_DIR}/commands.sh"
fi
CONFIG_VARS=`echo make_shlib $CONFIG_VARS`

### ------------------------------------------------------------------------
### General stuff


# --- Log file
date > "$CONFIG_LOG"
# --- Default prefix directory
if [ -z "$prefix" ] ; then
. ${CONFIG_DIR}/parse_config.sh
fi

if [ -r src/include/DjVuVersions.h ] 
then
  i=src/include/DjVuVersion.h
else
  i="${CONFIG_DIR}/../src/include/DjVuVersion.h"
fi
version=""
# version=`sed -n -e 's,.* DJVU_CVS_NAME[^0-9]*\([1-9][-_0-9A-Za-z]*\).*,\1,p' < "$i"`
if [ -z "$version" ] 
then 
  version=`awk '/DJVU_VERSION/ {split($0,x,"\"")} /DJVU_CVS_REV/ {split($0,y,"\"");split(y[2],z)} END {print x[2] "-" z[2]}' < $i`
fi
if [ -z "$version" ] 
then
  echo "Version not found in TOPDIR/SRCDIR/src/include/DjVuVersion.h" 1>&2
  exit 1
fi
version=`echo $version|sed -e 's, ,-,g' -e 's,-*$,,g'`
v=`echo "$version"|sed -n -e 's,^\([0-9]\).*,\1,p'`
if [ -z "$v" ]
then
  version="alpha"
fi
echo Building version $version
if [ ! -d "$TOPBUILDDIR/src/include" ]
then
  ${mkdirp} "$TOPBUILDDIR/src/include"
fi
CONFIG_VARS=`echo version $CONFIG_VARS`

