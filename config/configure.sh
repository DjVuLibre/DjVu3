#
# Support functions for configuration scripts
# Configuration scripts should source this file 
# and call the relevant functions.
#


### ------------------------------------------------------------------------
### General stuff

# --- Prefix for temporary files
temp=/tmp/c$$

# --- Log file
log=$temp.log

# --- Make sure that all temp files are removed
trap "rm 2>/dev/null $temp $temp.*" 0

# --- Default prefix directory
prefix=/usr/local



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
mkdirp()
{
    if [ -n "$1" ] && [ "$1" != "." ] && [ ! -d "$1" ]
    then
      mkdirp `dirname $1`
      mkdir $1
    fi
}



### ------------------------------------------------------------------------
### Process options


# Usage: get_option_value OPTION
#   Parse options like '--with-threads=xxxx' and return value

get_option_value()
{
    if [ `expr "$1" : '^[-A-Za-z_]*='` = 0 ]
    then
      echo yes
    else
      echo $1 | sed 's/^[-A-Za-z_]*=//'
    fi
}



# Usage: process_general_option OPTION
#   Process general purpose options
#   Returns 0 exit status if option is recognized

process_general_option()
{
  case "$1" in
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
cat 1>&2 <<EOF
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
    CC=${CC-gcc}
    CXX=${CXX-g++}
    OPT=${OPT}
    WARN=${WARN}
    LIBS=

    # -----

    echo 'int main(void) {return 0;}' > $temp.c
    echon "Testing C compiler ... "
    if ( ( $CC -c $temp.c -o $temp.o ) >$log 2>&1 ) 
    then
        echo $CC.
    elif ( ( gcc -c $temp.c -o $temp.o ) >$log 2>&1 ) 
    then
        CC=gcc
        echo $CC.
    elif ( ( cc -c $temp.c -o $temp.o ) >$log 2>&1 ) 
    then
        CC=cc
        echo $CC.
    else
        echo 1>&2 "$conf: Cannot find C compiler"
        echo 1>&2 "-- Use environment variable CC to specify a C compiler."
        echo 1>&2 "-- Use environment variable OPT and WARN to specify compiler options."
        exit 1
    fi

    # -----

    echo 'int main(void) {return 0;}' > $temp.cpp
    echon "Testing C++ compiler ... "
    if ( ( $CXX -c $temp.cpp -o $temp.o ) >$log 2>&1 ) 
    then
        echo $CXX.
    elif ( ( g++ -c $temp.cpp -o $temp.o ) >$log 2>&1 ) 
    then
        CXX=g++
        echo $CXX.
    elif ( ( CC -c $temp.cpp -o $temp.o ) >$log 2>&1 ) 
    then
        CXX=CC
        echo $CXX.
    else
        echo 1>&2 "$conf: Cannot find C++ compiler"
        echo 1>&2 "-- Use environment variable CXX to specify a C++ compiler."
        echo 1>&2 "-- Use environment variable OPT and WARN to specify compiler options."
        exit 1
    fi

    # -----

    cxx_is_gcc=
    cc_is_gcc=
    echon "Checking whether both compilers are gcc ... "
    echo 'int main(void) { return __GNUG__;}' > $temp.cpp
    if ( $CXX -O -c $temp.cpp -o $temp >$log 2>&1 ) 
    then
        cxx_is_gcc=yes
    fi
    echo 'int main(void) { return __GNUC__;}' > $temp.c
    if ( $CC -c $temp.c -o $temp >$log 2>&1 ) 
    then
        cc_is_gcc=yes
    fi
    if [ -n "$cc_is_gcc" ] && [ -n "$cxx_is_gcc" ]
    then
      echo yes.
      compiler_is_gcc=yes
      test -z "$OPT" && OPT="-O3 -funroll-loops"
      test -z "$WARN" && WARN="-Wall -Wno-unused"
    else
      echo no.
      compiler_is_gcc=
    fi

    # -----

    if [ -n "$compiler_is_gcc" ]
    then
        echon "Testing pipe option ... "
        echo 'int main(void) {return 0;}' > $temp.cpp
        if ( $CXX -pipe -c $temp.cpp -o $temp >$log 2>&1 ) 
        then
            echo yes.
            CC="$CC -pipe"
            CXX="$CXX -pipe"
        else
            echo no.
        fi
    fi

    # -----

    if [ -z "$OPT" ]
    then
        echon "Testing optimization option ... "
        echo 'int main(void) {return 0;}' > $temp.cpp
        if ( $CXX -O -c $temp.cpp -o $temp >$log 2>&1 ) 
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
  if [ -z "$1" ] ; then return; fi
  # We could test that one ...
  OPT=-g
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
       echo 2>&1 "$conf: Autodetermination of thread model is not yet implemented."
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
          echo 1>&2 "$conf: Cothreads only work with gcc."
          exit 1
       fi
       echon "Testing exception handler patch for gcc ... "
       cat > $temp.cpp <<\EOF
extern "C" { extern void *(*__get_eh_context_ptr)(void), *__new_eh_context(void); }
void main() { __get_eh_context_ptr = &__new_eh_context; }
EOF
       if ( $CXX $temp.cpp -o $temp 1>$log 2>&1 ) 
       then
          echo yes.
       else
          echo no.
          echo 1>&2 "$conf: Using COTHREADS without the patch is unsafe."
          echo 1>&2 "-- See documentation for libdjvu++."
          DEFS="$DEFS -DNO_LIBGCC_HOOKS"
       fi
       ;;       
    posix* | dce* )
       DEFS="$DEFS -DTHREADMODEL=POSIXTHREADS" 
       echo 'int main(void) {return 0;}' > $temp.cpp
       echon "Check option -pthread ... "
       if ( ( $CXX -pthread $temp.cpp -o $temp >$log 2>&1 ) \
            && [ -z "`grep -i unrecognized $log`" ] )
       then
         echo yes.
         CC="$CC -pthread"
         CXX="$CXX -pthread"
       else
         echo no.
         echon "Check option -threads ... "
         if ( ( $CXX -threads $temp.cpp -o $temp >$log 2>&1 ) \
              && [ -z "`grep -i unrecognized $log`" ] )
         then
           echo yes.
           CC="$CC -threads"
           CXX="$CXX -threads"
         else
           echo no.
           LIBS="$LIBS -lpthread"
           DEFS="$DEFS -D_REENTRANT"
         fi
       fi
       ;;
    *)
       echo 1>&2 "$conf: unrecognized multithreading option."
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
     echo 2>&1 "$conf: Option '-frepo' only work with gcc."
     exit 1
  fi
  echo 'int main(void) {return 0;}' > $temp.cpp
  if ( ( $CXX -frepo $temp.cpp -o $temp >$log 2>&1 ) \
         && [ -z "`grep -i unrecognized $log`" ] )
  then
    echo yes.
    OPT="$OPT -frepo"
  else
    echo no.
    echo 1>&2 "$conf: Compiler does not support option '-frepo'."
    exit 1
  fi
  echon "Searching rpo program ... "
  test -z "$RPO" && RPO=rpo
  if ( $RPO 1>$log 2>&1 ) 
  then
    echo $RPO
  else
    echo not found.

    #
    # We could build it instead of complaining
    #
    echo 1>&2 "$conf: Cannot find program RPO."
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
  func=$1
  shift
  echon "Searching library containing $func() ... "
  cat > $temp.cpp <<EOF
extern "C" int $func(void);
int main(void) { return $func(); }
EOF
  for lib
  do
    if ( $CXX $OPT $DEFS $WARN $temp.cpp $lib -o $temp 1>$log 2>&1 ) 
    then
      echo $lib
      LIBS="$LIBS $lib"
      return 0
    fi
  done
  echo "not found."
  echo 2>&1 "$conf: Function $func() not found."
  exit 1
}


### ------------------------------------------------------------------------
### Searching how to make archives


# Usage: check_make_stlib
# Side effect:  MAKE_STLIB <-- how to make a static library
#               RANLIB     <-- how to prepare a static library

check_make_stlib()
{
    AR=${AR-ar}
    MAKE_STLIB=${MAKE_STLIB-$AR cq}
    
    echon Searching how to build a static library ...
    if ( $MAKE_STLIB /tmp/$$.a /tmp/$$.o >$log 2>&1 ) 
    then
        echo $MAKE_STLIB
    else
        echo unknown.
        echo 1>&2 "$conf: Cannot find how to make a static library."
        echo 1>&2 "-- Please set environment variable MAKE_STLIB or AR."
        exit 1
    fi

    echon Searching RANLIB program ...
    if ( $RANLIB /tmp/$$.a >$log 2>&1 )
    then
        echo $RANLIB
    else
        RANLIB=true
        echo "none."
    fi
}








### ------------------------------------------------------------------------
### Function to generate sub makefiles



# Usage: generate_makefile DIRNAME
# Side effect: Generate DIRNAME/Makefile from SRCDIR/DIRNAME/Makefile.in

generate_makefile()
{
  # compute xtopsrcdir
  xtopsrcdir=$topsrcdir
  case $xtopsrcdir in
      /*) # absolute path
        ;;
      *)  # relative path
        temp=$1
        while [ "$temp" != "." ] 
        do
          xtopsrcdir=../$xtopsrcdir
          temp=`dirname $temp`
        done
        ;;
  esac 

  #compute topbuilddir
  temp=$1
  xtopbuilddir=.
  while [ "$temp" != "." ] 
  do
    xtopbuilddir=../$xtopbuilddir
    temp=`dirname $temp`
  done

  # compute xsrcdir
  xsrcdir=$xtopsrcdir/$1

  # make nice pathnames
  xsrcdir=`pathclean $xsrcdir`
  xtopsrcdir=`pathclean $xtopsrcdir`
  xtopbuilddir=`pathclean $xtopbuilddir`

  # substitute
  mkdirp $1
  sed < $topsrcdir/$1/Makefile.in > $1/Makefile \
    -e 's!@prefix@!'"$prefix"'!g' \
    -e 's!@topsrcdir@!'"$xtopsrcdir"'!g' \
    -e 's!@topbuilddir@!'"$xtopbuilddir"'!g' \
    -e 's!@srcdir@!'"$xsrcdir"'!g' \
    -e 's!@cc@!'"$CC"'!g' \
    -e 's!@cxx@!'"$CXX"'!g' \
    -e 's!@defs@!'"$DEFS"'!g' \
    -e 's!@opt@!'"$OPT"'!g' \
    -e 's!@warn@!'"$WARN"'!g' \
    -e 's!@libs@!'"$LIBS"'!g' \
    -e 's!@ranlib@!'"$RANLIB"'!g' \
    -e 's!@rpo@!'"$RPO"'!g' \
    -e 's!@docxx@!'"doc++"'!g' \
    -e 's!@make_stlib@!'"$MAKE_STLIB"'!g' \
    -e 's!@make_shlib@!'"$MAKE_SHLIB"'!g'

  # dependencies
  if [ -r $topsrcdir/$1/Makefile.depend ]
  then
    cat $topsrcdir/$1/Makefile.depend >> $1/Makefile
  fi
}




### ------------------------------------------------------------------------
### Function to generate main makefiles

# Usage: generate_main_makefile  SUBDIRS

generate_main_makefile()
{
    subdirs=$*
    targets="all clean depend html tex install"
    cat > Makefile <<EOF
SHELL=/bin/sh
TOPSRCDIR= $topsrcdir
TOPBUILDDIR= $topbuilddir
CC= $CC
CXX= $CXX
OPT= $OPT
WARN= $WARN
DEFS= $DEFS
SUBDIRS=$subdirs

EOF
     for target in $targets
     do
        cat >>Makefile <<EOF 
$target:
	@for n in \${SUBDIRS} ; do ( cd \$\$n ; \${MAKE} $target ) ; done
	@echo Done.

EOF
     done

        cat >>Makefile <<EOF 
.PHONY: $targets
EOF
}
