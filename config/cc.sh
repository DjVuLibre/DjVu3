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
  echo '#include <stdio.h>' > /tmp/c$$.c
  echo 'int main(void) {puts("Hello World\n");return 0;}' > /tmp/c$$.c
  echo Searching for C compiler
  if ( cd /tmp 2>>/dev/null;$EGCS -c c$$.c 1>>/dev/null 2>>/dev/null ) ; then
    CC="$EGCS"
  elif ( cd /tmp 2>>/dev/null;gcc -c c$$.c 1>>/dev/null 2>>/dev/null ) ; then
    CC=gcc
  elif ( cd /tmp 2>>/dev/null;cc -c c$$.c 1>>/dev/null 2>>/dev/null ) ; then
    CC=cc
  elif ( cd /tmp 2>>/dev/null;CC -c c$$.c 1>>/dev/null 2>>/dev/null ) ; then
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
      echo 'int foo(void) {return 0;}' > /tmp/c$$.c
      if [ ! -z "$EGCSTEST" ] ; then
        echo "$CC is egcs"
      elif ( cd /tmp 2>>/dev/null;$CC -V2.7.2.3 -c c$$.c  1>>/dev/null 2>>/dev/null ) ; then
        CCVERSION="-V2.7.2.3"
      elif ( cd /tmp 2>>/dev/null;$CC -V2.7.2 -c c$$.c  1>>/dev/null 2>>/dev/null ) ; then
        CCVERSION="-V2.7.2"
      elif ( cd /tmp 2>>/dev/null;$CC -V2.8.1 -c c$$.c  1>>/dev/null 2>>/dev/null ) ; then
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
  if ( cd /tmp 2>>/dev/null;$CC ${CCFLAGS} -pipe -c c$$.c 1>>/dev/null 2>>/dev/null ) ; then
    CCPIPE="-pipe"
  fi
  CCFLAGS=`echo "${CCPIPE}" "${CCFLAGS}"`
  
  echo "Testing ${CC} mmx option"
  CCMMX=""
  if [ `uname -m` = i686 ]
  then
    if (cd /tmp 2>>/dev/null;$CC ${CCFLAGS} -mpentiumpro -c c$$.c) 
    then
      CCMMX="-mpentiumpro"
      if ( cd /tmp 2>>/dev/null;$CC ${CCFLAGS} ${CCMMX} -mmx -c c$$.c 1>>/dev/null 2>>/dev/null ) ; then
        CCMMX="$CCMMX -mmx"
      fi
    fi
  fi
  CCFLAGS=`echo "${CCMMX}" "${CCFLAGS}"`

  echo "Testing ${CC} Symbolic Option"
  CCSYMBOLIC=""
  echo 'void foo(void) {exit(0);}' > /tmp/c$$.c
  if [ -z "`(cd /tmp 2>>/dev/null;${CC} ${CCFLAGS} -symbolic -c c$$.c 2>&1)|grep 'unrecognized option'`" ] ; then
    CCSYMBOLIC='-symbolic'
    echo "Using CCSYMBOLIC=$CCSYMBOLIC"
  elif (cd /tmp 2>>/dev/null;${CC} ${CCFLAGS} -shared -Wl,-Bsymbolic -o c$$.so c$$.c -lc -lm 2>>/dev/null 1>>/dev/null) ; then
    if [ "$SYS" != "linux-libc6" ] ; then
      CCSYMBOLIC='-Wl,-Bsymbolic'
      echo "Using CCSYMBOLIC=$CCSYMBOLIC"
    fi
  fi

  echo "Testing ${CC} PIC option"
  if ( cd /tmp 2>>/dev/null;$CC ${CCFLAGS} -fPIC -c c$$.c 1>>/dev/null 2>>/dev/null ) ; then
    CCPIC="-fPIC"
  fi
  rm -rf /tmp/c$$.c /tmp/c$$.o /tmp/c$$.so
  CONFIG_VARS=`echo CC CCFLAGS CCSYMBOLIC CCPIC $CONFIG_VARS`
fi

