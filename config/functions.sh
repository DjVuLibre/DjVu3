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
# check_make_stlib
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
  (while [ $# != 0 ] ; do
    echo "$1"
    shift
  done)|${sort} -u
}

# Escape characters.
escape()
{
  echo "$*"|${sed} -e 's,@%%@,@%%@p,g' -e 's,!,@%%@e,g' -e 's, ,@%%@s,g' -e 's,	,@%%@t,g' -e 's,\$,@%%@d,g' -e 's,",@%%@q,g' -e 's,'"'"',@%%@a,g'
}

unescape()
{
  (while [ $# != 0 ] ; do
    echo "$1"
    shift
  done)|sed  -e 's,@%%@a,'"'"',g' -e 's,@%%@q,",g' -e 's,@%%@d,\$,g' -e 's,@%%@t,	,g' -e 's,@%%@s, ,g' -e 's,@%%@e,!,g' -e 's,@%%@p,@%%@,g'
}

# Make directory and all parents unless they exist already
mkdir -p "$temp/test/test" 2>>/dev/null 1>>/dev/null
if [ -d "$temp/test/test" ] ; then
  rm -rf "$temp"
  mkdirp()
  {
    "${mkdir}" -p "$*"
  }
  SUPPORTS_MKDIRP=true
else
  mkdirp()
  {
    if [ -n "$*" -a "$*" != "." -a ! -d "$*" ]
    then
      mkdirp `"${dirname}" "$*"`
      "${mkdir}" "$*"
    fi
  }
fi

# Make a test file with echo on CONFIG_LOG
testfile()
{
    echo "------- ($*)" >> "$CONFIG_LOG"
    "${tee}" "$*"            >> "$CONFIG_LOG"
}


# Run command with echo on CONFIG_LOG
run()
{
    ( cd "$tempdir" 2>>/dev/null 1>>/dev/null; $* ) > $temp.out 2>&1 
    status=$?
    echo "------- (cmd)"   >> "$CONFIG_LOG"
    echo '%' "$*"          >> "$CONFIG_LOG"
    "${cat}" "$temp.out"        >> "$CONFIG_LOG"
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
            C_DEFS=APPEND
            A_DEFS="-DDEBUG $A_DEFS"
            ;;
        --with-flag=* )
            C_DEFS=APPEND
            s=`get option_value "$*"`
            A_DEFS=`escape "$s"`" $A_DEFS"
            ;;
        *)
            return 1;
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
### Check compiler

# Usage: check_compiler
# Side effect:  $CC              <-- name of a working C compiler
#               $CXX             <-- name of a working C++ compiler
#               $OPT             <-- initial guess for optimization options
#               $WARN            <-- initial guess for warning options
#               $LIBS            <-- empty
#               $compiler_is_gcc <-- 'yes' if this compiler is g++


check_compiler()
{
    . ${CONFIG_DIR}/cc.sh
    . ${CONFIG_DIR}/cxx.sh
    OPT=${OPT}
    WARN=${WARN}
    LIBS=

    # -----

    if [ -n "$cc_is_gcc" -a -n "$cxx_is_gcc" ]
    then
      compiler_is_gcc=yes
      if [ -z "$OPT" ] ; then  OPT="-funroll-loops" ; fi
      if [ -z "$WARN" ] ; then  WARN="-Wall -Wno-unused" ; fi
    else
      compiler_is_gcc=
    fi

}


### ------------------------------------------------------------------------
### Check debug option

# Usage: check_debug_option DEBUGFLAG
# Side effect:  $OPT <-- replaced with -g


check_debug_option()
{
  if [ -z "$*" ] ; then return; fi
  # We could test that one ...
  C_CCOPT=REPLACE
  C_CXXOPT=REPLACE
  C_OPT=REPLACE
  R_CCOPT="-g"
  R_CXXOPT="-g"
  R_OPT=""
  C_DEFS=APPEND
  A_DEFS="-DDEBUG $A_DEFS"
}



### ------------------------------------------------------------------------
### Check thread option


# Usage: check_thread_option THREADFLAG
# Side effects:   $A_DEFS <-- updated for multithread compilation
#                 $A_OPT  <-- updated for multithread compilation


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
       C_DEFS=APPEND
       A_DEFS="-DTHREADMODEL=NOTHREADS $A_DEFS" 
       ;;       
    jri* )
       C_DEFS=APPEND
       A_DEFS="-DTHREADMODEL=JRITHREADS $A_DEFS" 
       ;;       
    co* )
       if [ -z "$COTHREAD" ]  ; then
         . "${CONFIG_DIR}/cothread.sh"
       fi
       if [ -z "$CXXCOTHREAD" ]
       then
         echo 1>&2 "${PROGRAM_NAME}: Cothreads does not work with ${CXX}."
         echo 1>&2 "You need GNU g++ or a dirivative such as pgcc or egcs."
         exit 1
       fi
       if [ -z "$CCCOTHREAD" ]
       then
         echo 1>&2 "${PROGRAM_NAME}: Cothreads does not work with ${CC}."
         echo 1>&2 "You need GNU gcc or a dirivative such as pgcc or egcs."
         exit 1
       fi
       C_CXXFLAGS=APPEND
       A_CXXFLAGS="${CXXCOTHREAD} $A_CXXFLAGS" 
       C_CCFLAGS=APPEND
       A_CCFLAGS="${CCCOTHREAD} $A_CCFLAGS" 
       if [ ! -z "$CXXCOTHREAD_UNSAFE$CCCOTHREAD" ] 
       then
          echo 1>&2 "${PROGRAM_NAME}: Using COTHREADS without the patch is unsafe."
          echo 1>&2 "-- See documentation for libdjvu++."
       fi
       ;;       
    posix* | dce* )
       if [ -z "$CXXPTHREAD" ]  ; then
         . "${CONFIG_DIR}/pthread.sh"
       fi
       C_CCFLAGS=APPEND
       A_CCFLAGS="$CCPTHREAD $A_CCFLAGS"
       C_CXXFLAGS=APPEND
       A_CXXFLAGS="$CXXPTHREAD $A_CXXFLAGS"
       if [ ! -z "${CCPTHREAD_LIB}${CXXPTHREAD_LIB}" ]
       then 
         C_LIBS=APPEND
         A_LIBS="${CXXPTHREAD_LIB} ${A_LIBS}"
       fi
       ;;
    *)
       echo 1>&2 "${PROGRAM_NAME}: unrecognized multithreading option."
       exit 1
       ;;
  esac
}



### ------------------------------------------------------------------------
### Check repository option


# Usage: check_rpo_option RPOFLAG
# Side effects:  $RPO <-- name of rpo program
#                $OPT <-- updated to use -frepo


check_rpo_option()
{
  if [ -z "$1" ] ; then return; fi
  echon "Testing if compiler supports option '-frepo' ... "
  if [ -z "$compiler_is_gcc" ] 
  then
     echo no.
     echo 2>&1 "${PROGRAM_NAME}: Option '-frepo' only work with gcc."
     exit 1
  fi
  echo 'int main(void) {return 0;}' | testfile $temp.cpp
  run $CXX $CXXFLAGS -frepo $temp.cpp -o $temp
  if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
  then
    echo yes.
    OPT="$OPT -frepo"
  else
    echo no.
    echo 1>&2 "${PROGRAM_NAME}: Compiler does not support option '-frepo'."
    exit 1
  fi
  echon "Searching rpo program ... "
  if [ -z "$RPO" ] ; then RPO=rpo ; fi
  if ( run $RPO ) 
  then
    echo $RPO
  else
    echo not found.

    #
    # We could build it instead of complaining
    #
    echo 1>&2 "${PROGRAM_NAME}: Cannot find program RPO."
    echo 1>&2 "-- You cannot use option -frepo without this program."
    exit 1
  fi
}


### ------------------------------------------------------------------------
### Check library

# Usage: check_library NAME FUNCTION ...ALTERNATIVE_LINKSPEC...
# Side effect: Update variable lib<NAME>


check_library()
{
  name="$1"
  shift
  func="$1"
  shift
  s='echo $'"lib${name}_paths"
  s=`eval $s`
  t=`escape "$*"`
  if [ "x$s" != "x$t" ] ; then
    eval "lib${name}_paths='$t'"
    CONFIG_VARS=`lib${name}_paths lib${name} $CONFIG_VARS`
    echon "Searching library containing ${func}() ... "
    testfile $temp.cpp <<EOF
extern "C" int ${func}(void);
int main(void) { return ${func}(); }
EOF
    for lib
    do
      if ( run $CXX $CXXFLAGS $OPT $DEFS $WARN $temp.cpp $lib -o $temp ) 
      then
        echo $lib
        eval "lib${name}='"`escape "$lib"`"'"
        return 0
      fi
    done
    echo "not found."
    echo 2>&1 "${PROGRAM_NAME}: Function ${func}() not found."
    exit 1
  fi
}


### ------------------------------------------------------------------------
### Searching how to make archives


# Usage: check_make_stlib
# Side effect:  make_stlib <-- how to make a static library
#               RANLIB     <-- how to prepare a static library

check_make_stlib()
{
    echon Searching how to build a static library ...
    testfile $temp.c <<EOF
int main(void) { return 1; }
EOF
    run $CC $CCFLAGS $OPT $DEFS $WARN -c $temp.c
    if [ -z "$MAKE_STDLIB" ]
    then
      make_stlib="${ar} cq"
    fi
    if ( run "${make_stlib}" $temp.a $temp.o ) 
    then
        echo "${make_stlib}"
    else
        echo unknown.
        echo 1>&2 "${PROGRAM_NAME}: Cannot find how to make a static library."
        echo 1>&2 "-- Please set environment variable make_stlib or AR."
        exit 1
    fi
}








### ------------------------------------------------------------------------
### Function to generate sub makefiles



# Usage: generate_makefile DIRNAME
# Side effect: Generate DIRNAME/Makefile from SRCDIR/DIRNAME/Makefile.in

generate_makefile()
{
  # compute xtopsrcdir
  # compute xsrcdir
  xsrcdir=$TOPSRCDIR/$1

  # make nice pathnames
  xsrcdir=`pathclean $xsrcdir`

  # substitute
  mkdirp "$TOPBUILDDIR/$1"
  if [ -z "$WROTE_STATUS" ]
  then
    . "${CONFIG_DIR}/write_status.sh"
  fi
  run "${CONFIG_STATUS}" "$TOPSRCDIR/$1/Makefile.in" "$TOPBUILDDIR/$1/Makefile"
}




### ------------------------------------------------------------------------
### Function to generate main makefiles




# Usage: cat fragement | generate_main_makefile  SUBDIRS

generate_main_makefile()
{
    subdirs=$*

    # Generate Makefile header
    "${cat}" > "$TOPBUILDDIR/Makefile" <<EOF
SHELL=/bin/sh
TOPSRCDIR= $TOPSRCDIR
TOPBUILDDIR= $TOPBUILDDIR
CC= $CC $CCFLAGS
CXX= $CXX $CXXFLAGS
OPT= $OPT
WARN= $WARN
DEFS= $DEFS
SUBDIRS= $subdirs
EOF

    # Insert Makefile fragment
    "${cat}" >> "${TOPBUILDDIR}/Makefile"

    # Add final rules
    "${cat}" >> "${TOPBUILDDIR}/Makefile" <<\EOF

clean:
	for n in $(SUBDIRS) ; do ( cd $$n ; $(MAKE) clean ) ; done

html:
	for n in $(SUBDIRS) ; do ( cd $$n ; $(MAKE) html ) ; done

update-depend:
	for n in $(SUBDIRS) ; do ( cd $$n ; $(MAKE) depend ) ; done

depend:
	for n in $(SUBDIRS) ; do ( cd $$n ; $(MAKE) depend ) ; done

PHONY: all install clean html depend
EOF
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
  CONFIG_DIR=`dirname "$0"`/config
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

if [ -z "$SYS" ] ; then
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
CONFIG_VARS=`echo DEFS OPT WARN LIBS RPO make_stlib make_shlib $CONFIG_VARS`

### ------------------------------------------------------------------------
### General stuff


# --- Log file
date > "$CONFIG_LOG"
# --- Default prefix directory
if [ -z "$prefix" ] ; then
. ${CONFIG_DIR}/parse_config.sh
fi

