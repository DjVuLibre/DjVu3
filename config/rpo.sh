# This script sets the variables:
#   CXXRPO_TEST,CXXRPO,CCRPO_TEST,CCRPO

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ ! -z "$CXX" ]
then
  if [ -z "$CXXRPO_TEST" ]
  then
    CXXRPO=""
    CXXRPO_TEST=true
    echon "Testing if ${CXX} supports option '-frepo' ... "
    if [ -z "$cxx_is_gcc" ] 
    then
      echo no.
      echo "The option -frepo is only supported with g++"
    else
      echo 'int main(void) {return 0;}' | testfile $temp.cpp
      run "$CXX" $CXXFLAGS -frepo $temp.cpp -o $temp
      if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
      then
        echo yes.
        CXXRPO="-frepo"
      else
        echo no.
      fi
      echon "Searching rpo program ... "
      if [ -z "$rpo" -o "x$rpo" = "xyes" ] ; then rpo=rpo ; fi
      if ( run "$rpo" </dev/null ) 
      then
        echo "$rpo"
      else
        echo not found.
        echo  "-- You cannot use option -frepo without this program."
        CXXRPO=""
      fi
    fi
    CXXRPO_TEST=tested
    CONFIG_VARS=`echo rpo CXXRPO_TEST CXXRPO $CONFIG_VARS`
  fi
fi

if [ ! -z "$CC" ]
then
  if [ -z "$CCRPO_TEST" ]
  then
    CCRPO=""
    CCRPO_TEST=true
    echon "Testing if ${CC} supports option '-frepo' ... "
    if [ -z "$cc_is_gcc" ] 
    then
      echo no.
      echo "The option -frepo is only supported with gcc"
    else
      echo 'int main(void) {return 0;}' | testfile $temp.c
      run "$CC" $CCFLAGS -frepo $temp.c -o $temp
      if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
      then
        echo yes.
        CCRPO="-frepo"
      else
        echo no.
      fi
      if [ -z "$CXXRPO" ]
      then
        echon "Searching rpo program ... "
        if [ -z "$rpo" -o "x$rpo" = "xyes" ] ; then rpo=rpo ; fi
        if ( run $rpo ) 
        then
          echo "$rpo"
        else
          echo not found.
          echo  "-- You cannot use option -frepo without this program."
          CCRPO=""
        fi
      fi
    fi
    CCRPO_TEST=tested
    CONFIG_VARS=`echo rpo CCRPO_TEST CCRPO $CONFIG_VARS`
  fi
fi

