# This rule sets the following variables:
#	CXXSYMBOLIC, CXXPIC
# $Id: cxxpic.sh,v 1.1 2000-02-06 22:24:27 bcr Exp $

if [ -z "$CXX_SET" ] ; then
  echo "You must source cxx.sh" 1>&2
  exit 1
fi

if [ -z "$CXXPIC_SET" ] ; then
  echo 'extern "C" {void exit(int);};void foo(void) {exit(0);}' |testfile $temp.cpp
  
  echon "Checking ${CXX} symbolic option ... "
  CXXSYMBOLIC=""
  SYSTEMGXX=`echo $SYS | tr A-Z a-z `-$cxx_is_gcc
  case $SYSTEMGXX in
    linux-*)
      TESTCXXSYMBOLIC="-shared "
      TESTCXXPIC="-fPIC"
      ;;
    solaris-yes)
      if [ -z "$CROSSCOMPILER" ] ; then 
        TESTCXXSYMBOLIC="-shared -L/usr/lib -R/usr/lib "
      else
        TESTCXXSYMBOLIC="-shared -Wl,-rpath,/usr/lib:/usr/ccs/lib:/usr/openwin/lib "
      fi
      TESTCXXPIC="-fPIC"
      ;;
    solaris-*)
      if [ -z "$CROSSCOMPILER" ] ; then 
        TESTCXXSYMBOLIC="-G -L/usr/lib -R/usr/lib "
      else
        TESTCXXSYMBOLIC="-shared -Wl,-rpath,/usr/lib:/usr/ccs/lib:/usr/openwin/lib "
      fi
      TESTCXXPIC="-K PIC"
      ;;
    irix*-*)
      TESTCXXSYMBOLIC="-shared "
      TESTCXXPIC=""
      ;;
    aix*-*)
      TESTCXXSYMBOLIC="-r "
      TESTCXXPIC="-bM\:SRE"
      ;;
  esac

  check_shared_link_flags CXXSYMBOLIC $temp.cpp "$TESTCXXSYMBOLIC"
  if [ -z "$CXXSYMBOLIC" ] ; then
    echo "none"
  else
    echo "$CXXSYMBOLIC"
  fi
	
  echon "Checking whether ${CXX} $TESTCXXPIC works ... "
  check_compile_flags CXXPIC $temp.cpp $TESTCXXPIC
  if [ $? = 0 ]
  then
    echo yes
  else
    echo no
  fi

  CXXPIC_SET=true
  "${rm}" -rf $temp.cpp $temp.so $temp.o
  CONFIG_VARS=`echo CXXPIC_SET CXXSYMBOLIC CXXPIC $CONFIG_VARS`
fi

