# This rule sets the following variables:
#	CCSYMBOLIC, CCPIC

if [ -z "$CC_SET" ]
then
  echo "You must source cc.sh" 1>&2
  exit 1
fi

EGCS="egcs"
if [ -z "$CCPIC_SET" ]
then
  CCSYMBOLIC=""
  CCPIC=""
  (echo '#include <stdio.h>';echo 'int main(void) {puts("Hello World\n");return 0;}')|testfile $temp.c

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
        TESTCCSYMBOLIC="-shared"
        TESTCCPIC="-fPIC"
        ;;
      solaris-*-yes)
        if [ -z "$CROSSCOMPILER" ] ; then 
          TESTCCSYMBOLIC="-shared"
        else
          TESTCCSYMBOLIC="-shared"
        fi
        TESTCCPIC="-fPIC"
        ;;
      solaris-*)
        if [ -z "$CROSSCOMPILER" ] ; then 
          TESTCCSYMBOLIC="-shared"
        else
          TESTCCSYMBOLIC="-shared"
        fi
        TESTCCPIC="-fPIC"
        ;;
      irix*-*) 
        TESTCCSYMBOLIC="-shared"
        TESTCCPIC=""
        ;;
      aix*-*) 
        TESTCCSYMBOLIC="-r"
        TESTCCPIC="-bM\:SRE"
	;;
     esac

    check_shared_link_flags CCSYMBOLIC $temp.c "$TESTCCSYMBOLIC"
  fi
  if [ -z "$CCSYMBOLIC" ]
  then
    echo "none"
  else
    echo "$CCSYMBOLIC"
  fi

  echon "Checking whether ${CC} -fPIC works ... "
  if ( run $CC ${CCFLAGS} $TESTCCPIC -c $temp.c )
  then
    CCPIC=$TESTCCPIC
    echo yes
  else
    echo no
  fi

  "${rm}" -rf $temp.c $temp.o $temp.so
  CCPIC_SET=true
  CONFIG_VARS=`echo CCPIC_SET CCSYMBOLIC CCPIC $CONFIG_VARS`
fi

