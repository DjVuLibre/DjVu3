# This rule sets the following variables:
#	CC, CCFLAGS, CCSYMBOLIC CCPIC

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

EGCS="egcs"
if [ -z "$CC" ] ; then
  CCFLAGS=""
  CCSYMBOLIC=""
  CCPIC=""
  cc_is_gcc=""
  (echo '#include <stdio.h>';echo 'int main(void) {puts("Hello World\n");return 0;}')|testfile $temp.c
  echo Searching for C compiler
  if ( run $EGCS -c $temp.c ) ; then
    CC="$EGCS"
  elif ( run gcc -c $temp.c ) ; then
    CC=gcc
  elif ( run cc -c $temp.c ) ; then
    CC=cc
  elif ( run CC -c $temp.c ) ; then
    CC=CC
  else
    echo "Error: Can't find a C compiler" 1>&2
    exit 1
  fi
  CCVERSION=""
  if [ "$CC" != "$EGCS" ] ; then
    echo "Testing ${CC} version"
    CCprefix="`$CC -Vfoo 2>&1|fgrep 'file path prefix'|sed 's,.* .\([^ ]*/\)foo/. never.*,\1,'`"
    if [ ! -z "$CCprefix" ] ; then
      echo "$CC is egcs"
    else
      EGCSTEST=`($CC -v 2>&1)|sed -n -e 's,.*/egcs-.*,Is EGCS,p' -e 's,.*/pgcc-.*,Is PGCC,p'`
      if [ ! -z "$EGCSTEST" ] ; then
        echo "$CC is egcs"
      elif ( run $CC -V2.7.2.3 -c $temp.c ) ; then
        CCVERSION="-V2.7.2.3"
      elif ( run $CC -V2.7.2 -c $temp.c ) ; then
        CCVERSION="-V2.7.2"
      elif ( run $CC -V2.8.1 -c $temp.c ) ; then
        CCVERSION="-V2.8.1"
        echo WARNING: version 2.7.2.3 of gcc is recommended. 1>&2
      else
        echo WARNING: version 2.7.2.3 of gcc is recommended. 1>&2
      fi
    fi
  fi
  CCFLAGS=${CCVERSION}
  
  echo "Testing ${CC} pipe option"
  CCPIPE=""
  if ( run $CC ${CCFLAGS} -pipe -c $temp.c ) ; then
    CCPIPE="-pipe"
  fi
  CCFLAGS=`echo "${CCPIPE}" "${CCFLAGS}"`
  
  echo "Testing ${CC} mmx option"
  CCMMX=""
  if [ `uname -m` = i686 ]
  then
    if ( run $CC ${CCFLAGS} -mpentiumpro -c $temp.c) 
    then
      CCMMX="-mpentiumpro"
      if ( run $CC ${CCFLAGS} ${CCMMX} -mmx -c $temp.c ) ; then
        CCMMX="$CCMMX -mmx"
      fi
    fi
  fi
  CCFLAGS=`echo "${CCMMX}" "${CCFLAGS}"`

  echo "Testing ${CC} Symbolic Option"
  CCSYMBOLIC=""
  if [ -z "`( cd $tempdir 2>>/dev/null;${CC} ${CCFLAGS} -symbolic -c $temp.c 2>&1)|grep 'unrecognized option'`" ] ; then
    CCSYMBOLIC='-symbolic'
    echo "Using CCSYMBOLIC=$CCSYMBOLIC"
  elif ( run ${CC} ${CCFLAGS} -shared -Wl,-Bsymbolic -o c$$.so $temp.c -lc -lm ) ; then
    if [ "$SYS" != "linux-libc6" ] ; then
      CCSYMBOLIC='-Wl,-Bsymbolic'
      echo "Using CCSYMBOLIC=$CCSYMBOLIC"
    fi
  fi

  echo "Testing ${CC} PIC option"
  if ( run $CC ${CCFLAGS} -fPIC -c $temp.c ) ; then
    CCPIC="-fPIC"
  fi
  echon "Checking whether ${CC} is gcc ... "
  echo 'int main(void) { return __GNUC__;}' | testfile $temp.c
  if ( run $CC $CCFLAGS -c $temp.c ) 
  then
      echo yes
      cc_is_gcc=yes
  else
      echo no
  fi
  rm -rf $temp.c $temp.o $temp.so
  CONFIG_VARS=`echo CC CCFLAGS CCSYMBOLIC CCPIC cc_is_gcc $CONFIG_VARS`
fi

