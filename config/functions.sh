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
    echo $* "" "\c" 
  }
else
  echon() { 
    echo -n $* ""
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


# Make directory and all parents unless they exist already
mkdir -p "$temp/test/test" 2>>/dev/null 1>>/dev/null
if [ -d "$temp/test/test" ] ; then
  rm -rf "$temp"
  mkdirp()
  {
    "${mkdir}" -p "$*"
  }
  MKDIRP="'${mkdir}' -p"
else
  mkdirp()
  {
    if [ -n "$*" -a "$*" != "." -a ! -d "$*" ]
    then
      mkdirp `"${dirname}" "$*"`
      "${mkdir}" "$*"
    fi
  }
  MKDIRP="'${CONFIG_DIR}/mkdirp.sh'"
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
            prefix=`get_option_value $1`
            ;; 
        --with-debug|-g )
            OPT='-g'
            DEFS="$DEFS -DDEBUG"
            ;;
        --with-flag=* )
            DEFS="$DEFS "`get_option_value $1`
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
      if [ -z "$OPT" ] ; then  OPT="-O3 -funroll-loops" ; fi
      if [ -z "$WARN" ] ; then  WARN="-Wall -Wno-unused" ; fi
    else
      compiler_is_gcc=
    fi

    # -----

    if [ -z "$OPT" ]
    then
        echon "Testing optimization option ... "
        echo 'int main(void) {return 0;}' | testfile $temp.cpp
        if ( run $CXX $CXXFLAGS -O -c $temp.cpp -o $temp ) 
        then
            OPT=-O
            echo $OPT
        else
            echo broken.
        fi
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
  OPT="-g"
  DEFS="$DEFS -DDEBUG"
}



### ------------------------------------------------------------------------
### Check thread option


# Usage: check_thread_option THREADFLAG
# Side effects:   $DEFS <-- updated for multithread compilation
#                 $OPT  <-- updated for multithread compilation


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
       DEFS="$DEFS -DTHREADMODEL=NOTHREADS" 
       ;;       
    jri* )
       DEFS="$DEFS -DTHREADMODEL=JRITHREADS" 
       ;;       
    co* )
       DEFS="$DEFS -DTHREADMODEL=COTHREADS" 
       if [ -z "$compiler_is_gcc" ] 
       then
          echo 1>&2 "${PROGRAM_NAME}: Cothreads only work with gcc."
          exit 1
       fi
       echon "Testing exception handler patch for gcc ... "
       testfile $temp.cpp <<\EOF
extern "C" void *(*__get_eh_context_ptr)(void);
extern "C" void *__new_eh_context(void);
void main() { __get_eh_context_ptr = &__new_eh_context; }
EOF
       if ( run $CXX $CXXFLAGS $temp.cpp -o $temp ) 
       then
          echo yes.
       else
          echo no.
          echo 1>&2 "${PROGRAM_NAME}: Using COTHREADS without the patch is unsafe."
          echo 1>&2 "-- See documentation for libdjvu++."
          DEFS="$DEFS -DNO_LIBGCC_HOOKS"
       fi
       ;;       
    posix* | dce* )
       DEFS="$DEFS -DTHREADMODEL=POSIXTHREADS" 
       echo 'int main(void) {return 0;}' | testfile $temp.cpp
       echon "Check option -pthread ... "
       run $CXX $CXXFLAGS -pthread $temp.cpp -o $temp
       if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
       then
         echo yes.
         CC="$CC -pthread"
         CXX="$CXX -pthread"
       else
         echo no.
         echon "Check option -threads ... "
         run $CXX $CXXFLAGS -threads $temp.cpp -o $temp
         if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
         then
           echo yes.
           CC="$CC -threads"
           CXX="$CXX $CXXFLAGS -threads"
         else
           echo no.
           LIBS="$LIBS -lpthread"
           DEFS="$DEFS -D_REENTRANT"
         fi
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

# Usage: check_library FUNCTION ...ALTERNATIVE_LINKSPEC...
# Side effect: Update variable LIBS


check_library()
{
  func="$1"
  shift
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
      LIBS="$LIBS $lib"
      return 0
    fi
  done
  echo "not found."
  echo 2>&1 "${PROGRAM_NAME}: Function ${func}() not found."
  exit 1
}


### ------------------------------------------------------------------------
### Searching how to make archives


# Usage: check_make_stlib
# Side effect:  MAKE_STLIB <-- how to make a static library
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
      MAKE_STLIB="${ar} cq"
    fi
    if ( run "${MAKE_STLIB}" $temp.a $temp.o ) 
    then
        echo "${MAKE_STLIB}"
    else
        echo unknown.
        echo 1>&2 "${PROGRAM_NAME}: Cannot find how to make a static library."
        echo 1>&2 "-- Please set environment variable MAKE_STLIB or AR."
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
  xtopsrcdir=`pathclean $TOPSRCDIR`
  xtopbuilddir=`pathclean $TOPBUILDDIR`

  # substitute
  mkdirp "$TOPBUILDDIR/$1"
  if [ -z "$WROTE_STATUS" ]
  then
    . "${CONFIG_DIR}/write_status.sh"
  fi
  run "${CONFIG_STATUS}" "$TOPSRCDIR/$1/Makefile.in" "$TOPBUILDDIR/$1/Makefile"
  mv "$TOPBUILDDIR/$1/Makefile" "$temp"
  sed < "$temp" > "$TOPBUILDDIR/$1/Makefile" \
    -e 's!@%PROJECT_PREFIX%@!'"${PROJECT_PREFIX}"'!g' \
    -e 's!@%MKDIRP%@!'"$MKDIRP"'!g' \
    -e 's!@%topsrcdir%@!'"$xtopsrcdir"'!g' \
    -e 's!@%topbuilddir%@!'"$xtopbuilddir"'!g' \
    -e 's!@%srcdir%@!'"$xsrcdir"'!g' \
    -e 's!@%cc%@!'"$CC $CCFLAGS"'!g' \
    -e 's!@%cxx%@!'"$CXX $CXXFLAGS"'!g' \
    -e 's!@%defs%@!'"$DEFS"'!g' \
    -e 's!@%opt%@!'"$OPT"'!g' \
    -e 's!@%warn%@!'"$WARN"'!g' \
    -e 's!@%libs%@!'"$LIBS"'!g' \
    -e 's!@%rpo%@!'"$RPO"'!g' \
    -e 's!@%docxx%@!'"doc++"'!g' \
    -e 's!@%make_stlib%@!'"$MAKE_STLIB"'!g' \
    -e 's!@%make_shlib%@!'"$MAKE_SHLIB"'!g'
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
  CONFIG_VARS=`echo CONFIG_DIR "${CONFIG_VARS}"`
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

### ------------------------------------------------------------------------
### General stuff


# --- Log file
date > "$CONFIG_LOG"
# --- Default prefix directory
if [ -z "$prefix" ] ; then
. ${CONFIG_DIR}/parse_config.sh
fi

