# This script sets the variables:
#   CXXPTHREAD,CXXPTHREAD_LIB,CCPTHREAD,CCPTHREAD_LIB

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ ! -z "$CXX" ]
then
  if [ -z "${CXXPTHREAD}" ]
  then
    echo 'int main(void) {return 0;}' | testfile $temp.cpp
    echon "Checking if ${CXX} -pthread works ... "
    run $CXX $CXXFLAGS -pthread $temp.cpp -o $temp
    CXXPTHREAD="-DTHREADMODEL=POSIXTHREADS"
    CXXPTHREAD_LIB=""
    if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
    then
      echo yes.
      CXXPTHREAD="$CXXPTHREAD -pthread"
    else
      echo no.
      echon "Checking if ${CXX} -thread works ... "
      run $CXX $CXXFLAGS -threads $temp.cpp -o $temp
      if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
      then
        echo yes.
        CXXPTHREAD="$CXXPTHREAD -threads"
      else
        echo no.
        CXXPTHREAD="$CXXPTHREAD -D_REENTRANT"
        CXXPTHREAD_LIB="-lpthread"
      fi
    fi
    CONFIG_VARS=`echo CXXPTHREAD CXXPTHREAD_LIB $CONFIG_VARS`
  fi
fi

if [ ! -z "$CC" ]
then
  if [ -z "${CCPTHREAD}" ]
  then
    echo 'int main(void) {return 0;}' | testfile $temp.c
    echon "Checking if ${CC} -pthread works ... "
    run $CC $CCFLAGS -pthread $temp.c -o $temp
    CCPTHREAD="-DTHREADMODEL=POSIXTHREADS"
    CCPTHREAD_LIB=""
    if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
    then
      echo yes.
      CCPTHREAD="$CCPTHREAD -pthread"
    else
      echo no.
      echon "Checking if ${CC} -threads works ... "
      run $CC $CCFLAGS -threads $temp.cpp -o $temp
      if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
      then
        echo yes.
        CCPTHREAD="$CCPTHREAD -threads"
      else
        echo no.
        CCPTHREAD="$CCPTHREAD -D_REENTRANT"
        CCPTHREAD_LIB="-lpthread"
      fi
    fi
    CONFIG_VARS=`echo CCPTHREAD CCPTHREAD_LIB $CONFIG_VARS`
  fi
fi

