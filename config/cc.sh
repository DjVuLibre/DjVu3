# This rule sets the following variables:
#	CC, CCFLAGS, CCSYMBOLIC CCPIC

if [ -z "$CONFIG_DIR" ]
then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

EGCS="egcs"
if [ -z "$CC" ]
then
  CCFLAGS=""
  CCSYMBOLIC=""
  CCPIC=""
  cc_is_gcc=""
  (echo '#include <stdio.h>';echo 'int main(void) {puts("Hello World\n");return 0;}')|testfile $temp.c
  echon "Searching for C compiler ... "
  if ( run $EGCS -c $temp.c ) ; then
    CC="$EGCS"
  elif ( run gcc -c $temp.c ) ; then
    CC=gcc
  elif ( run cc -c $temp.c ) ; then
    CC=cc
  elif ( run CC -c $temp.c ) ; then
    CC=CC
  else
    echo "none available"
    echo "Error: Can't find a C compiler" 1>&2
    exit 1
  fi
  echo "$CC"

  CCVERSION=""
  echon "Checking ${CC} version ... "
  if [ "$CC" != "$EGCS" ]
  then
    CCprefix="`$CC -Vfoo 2>&1|fgrep 'file path prefix'|sed 's,.* .\([^ ]*/\)foo/. never.*,\1,'`"
    if [ ! -z "$CCprefix" ]
    then
      echo "egcs"
    else
      EGCSTEST=`($CC -v 2>&1)|sed -n -e 's,.*/egcs-.*,Is EGCS,p' -e 's,.*/pgcc-.*,Is PGCC,p'`
      if [ ! -z "$EGCSTEST" ]
      then
        echo "egcs"
      elif ( run $CC -V2.7.2.3 -c $temp.c )
      then
        CCVERSION="-V2.7.2.3"
        echo 2.7.2.3
      elif ( run $CC -V2.7.2 -c $temp.c )
      then
        CCVERSION="-V2.7.2"
        echo 2.7.2
      elif ( run $CC -V2.8.1 -c $temp.c )
      then
        CCVERSION="-V2.8.1"
        echo 2.8.1
        echo WARNING: version 2.7.2.3 of gcc is recommended. 1>&2
      else
        echo unknown
        echo WARNING: version 2.7.2.3 of gcc is recommended. 1>&2
      fi
    fi
  else
    echo egcs
  fi
  CCFLAGS=${CCVERSION}
  
  echon "Checking whether ${CC} -pipe works ... "
  if ( run $CC ${CCFLAGS} -pipe -c $temp.c )
  then
    echo yes
    CCPIPE="-pipe"
  else
    CCPIPE=""
    echo no
  fi
  CCFLAGS=`echo "${CCPIPE}" "${CCFLAGS}"`
  
  CCMMX=""
  if [ `uname -m` = i686 ]
  then
    echon "Chesking whether ${CC} -mpentiumpro and -mmx work ... "
    if ( run $CC ${CCFLAGS} -mpentiumpro -c $temp.c) 
    then
      CCMMX="-mpentiumpro"
      if ( run $CC ${CCFLAGS} ${CCMMX} -mmx -c $temp.c )
      then
        CCMMX="$CCMMX -mmx"
        echo "yes, both work"
      else
        echo "just $CCMMX works"
      fi
    else
      echo no
    fi
    CCFLAGS=`echo "${CCMMX}" "${CCFLAGS}"`
  fi

  echon "Checking ${CC} symbolic option ... "
  CCSYMBOLIC=""
  if [ -z "`( cd $tempdir 2>>/dev/null;${CC} ${CCFLAGS} -symbolic -c $temp.c 2>&1)|grep 'unrecognized option'`" ]
  then
    CCSYMBOLIC='-symbolic'
  elif ( run ${CC} ${CCFLAGS} -shared -Wl,-Bsymbolic -o c$$.so $temp.c -lc -lm )
  then
    if [ "$SYS" != "linux-libc6" ]
    then
      CCSYMBOLIC='-Wl,-Bsymbolic'
    fi
  fi
  if [ -z "$CCSYMBOLIC" ]
  then
    echo "none"
  else
    echo "$CCSYMBOLIC"
  fi

  echon "Checking whether ${CC} -fPIC works ... "
  if ( run $CC ${CCFLAGS} -fPIC -c $temp.c )
  then
    CCPIC="-fPIC"
    echo yes
  else
    echo no
  fi

  echon "Checking whether ${CC} is gcc ... "
  echo 'int main(void) { return __GNUC__;}' | testfile $temp.c
  CCOPT=""
  if ( run $CC $CCFLAGS -c $temp.c ) 
  then
    echo yes
    cc_is_gcc=yes
    echon "Checking whether ${CC} -O3 works ... "
    if ( run $CC ${CCFLAGS} -O3 -c $temp.c )
    then
      echo yes
      CCOPT="-O3"
    else
      echo no
    fi
  else
     echo no
  fi
  if [ -z "$CCOPT" ]
  then
    echon "Checking whether ${CC} -O works ... "
    if ( run $CC ${CCFLAGS} -O -c $temp.c )
    then
      echo yes
      CCOPT="-O"
    else
      echo no
      CCOPT=""
    fi
  fi
  rm -rf $temp.c $temp.o $temp.so
  CONFIG_VARS=`echo CC CCFLAGS CCOPT CCSYMBOLIC CCPIC cc_is_gcc $CONFIG_VARS`
fi

