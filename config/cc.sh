# This rule sets the following variables:
#	CC, CCFLAGS, CCSYMBOLIC, CCPIC, CCWARN, CCUNROLL

if [ -z "$CONFIG_DIR" ]
then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

EGCS="egcs"
if [ -z "$CC_SET" ]
then
  (echo '#include <stdio.h>';echo 'int main(void) {puts("Hello World\n");return 0;}')|testfile $temp.c
  # I added CC_OVERRIDE to be able to select compiler myself and let
  # you do the rest (flags and options) -eaf
  if [ -z "$CC_OVERRIDE" ]; then
    CCFLAGS=""
    CCSYMBOLIC=""
    CCPIC=""
    cc_is_gcc=""
    echon "Searching for C compiler ... "
    if [ -n "$CC" ] ; then
      if ( run "$CC" -c $temp.c ) ; then
        echo "$CC"
      else
        CC=""
      fi
    fi
    if [ -z "$CC" ]
    then
      if ( run "$EGCS" -c $temp.c ) ; then
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
    fi
  fi
#     The -x option doesn't work correctly when linking files.
#  if [ -z "$CXX" ] 
#  then
#    CXX="$CC -x c++"
#  fi

  CCVERSION=""
  echon "Checking ${CC} version ... "
  if [ "$CC" != "$EGCS" ]
  then
    s=`$CC -Vfoo 2>&1|"${grep}" 'file path prefix'|"${sed}" 's,.* .\([^ ]*/\)foo/. never.*,\1,'`
    if [ ! -z "$s" ]
    then
      echo "egcs"
    else
      # This test fails for pre-release versions of egcs (-eaf)
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
	(echo '#include <stdio.h>';\
	 echo 'int main(void) { printf("%d", __GNUC__); }') | testfile $temp.c
	if (run $CC $temp.c -o $temp.exe)
	then
	  MAJOR=`$temp.exe`
	fi
	(echo '#include <stdio.h>';\
	 echo 'int main(void) { printf("%d", __GNUC_MINOR__); }') | testfile $temp.c
	if (run $CC $temp.c -o $temp.exe)
	then
	  MINOR=`$temp.exe`
	fi
	if [ -n "$MAJOR" -a -n "$MINOR" ]
	then
	  echon "$MAJOR.$MINOR"
	  if [ $MAJOR -eq 2 -a $MINOR -ge 9 -o $MAJOR -ge 3 ]
	  then
	    echo " (egcs)"
	  else
	    echo
	    echo WARNING: version 2.7.2.3 of gcc is recommended. 1>&2
	  fi
	else
          echo unknown
          echo WARNING: version 2.7.2.3 of gcc is recommended. 1>&2
	fi
	rm -f $temp.exe
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
  m=`${uname} -m`
  if [ "${m}" = i686 ]
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

  echon "Checking whether ${CC} is gcc ... "
  echo 'int main(void) { return __GNUC__;}' | testfile $temp.c
  CCOPT=""
  CCUNROLL=""
  CCWARN=""
  if ( run $CC $CCFLAGS -c $temp.c ) 
  then
    echo yes
    cc_is_gcc=yes
    CCWARN="-Wall"
    echon "Checking whether ${CC} -O3 works ... "
    if ( run $CC ${CCFLAGS} -O3 -c $temp.c )
    then
      echo yes
      CCOPT="-O3"
    else
      echo no
    fi
    echon "Checking whether ${CC} -funroll-loops works ... "
    if ( run $CC ${CCFLAGS} ${CCOPT} -funroll-loops -c $temp.c )
    then
      echo yes
      CCUNROLL="-funroll-loops"
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

  echon "Checking ${CC} symbolic option ... "
  CCSYMBOLIC=""
  s=`( cd "$tempdir" 2>>/dev/null;${CC} ${CCFLAGS} -symbolic -c $temp.c 2>&1)|"${grep}" 'unrecognized option'`
  if [ -z "$s" ]
  then
    CCSYMBOLIC='-symbolic'
  else 
	  SYSTEMGCC=`echo $SYS | tr A-Z a-z `-$cc_is_gcc
		case $SYSTEMGCC in
		  linux-*) 
			  TESTCCSYMBOLIC="-Wl,-Bstatic,-lstdc++ -shared"
				TESTCCPIC="-fPIC"
				;;
		  solaris-yes) 
			  TESTCCSYMBOLIC="-Wl,-Bstatic,-lstdc++ -G"
				TESTCCPIC="-fpic"
				;;
		  solaris-*) 
			  TESTCCSYMBOLIC="-Wl,-Bstatic,-lstdc++ -G"
				TESTCCPIC="-K PIC"
				;;
		  irix*-*) 
			  TESTCCSYMBOLIC="-Wl,-Bstatic,-lstdc++ -shared"
				TESTCCPIC=""
				;;
		  aix*-*) 
			  TESTCCSYMBOLIC="-Wl,-Bstatic,-lstdc++ -r"
				TESTCCPIC="-bM\:SRE"
				;;
		esac

	check_shared_link_flags CCSYMBOLIC $temp.c "$TESTCCSYMBOLIC"
#	check_link_flags CCSYMBOLIC $temp.c "-shared -symbolic" "-shared -Wl,-Bsymbolic" "-shared -Wl,-Bsymbolic -lc" "-shared -Wl,-Bsymbolic -lc -lm"
#  elif ( run ${CC} ${CCFLAGS} -shared -Wl,-Bsymbolic -o c$$.so $temp.c -lc -lm )
#  then
#    if [ "$SYS" != "linux-libc6" ]; then
#      CCSYMBOLIC='-Wl,-Bsymbolic'
#    fi
  fi
  if [ -z "$CCSYMBOLIC" ]
  then
    echo "none"
  else
    echo "$CCSYMBOLIC"
  fi

  echon "Checking whether ${CC} -fPIC works ... "
#  if ( run $CC ${CCFLAGS} -fPIC -c $temp.c )
  if ( run $CC ${CCFLAGS} $TESTCCPIC -c $temp.c )
  then
    CCPIC=$TESTCCPIC
    echo yes
  else
    echo no
  fi

  "${rm}" -rf $temp.c $temp.o $temp.so
  CC_SET=true
  CONFIG_VARS=`echo CC CC_SET CCFLAGS CCOPT CCWARN CCUNROLL CCSYMBOLIC CCPIC cc_is_gcc $CONFIG_VARS`
fi

