# This script sets the variables:
#   COTHREAD,CXXCOTHREAD,CXXCOTHREAD_UNSAFE,CCCOTHREAD,CCCOTHREAD_UNSAFE

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z ${COTHREAD} ]
then
  CXXCOTHREAD=""
  CXXCOTHREAD_UNSAFE=""
  if [ ! -z "$cxx_is_gcc" ] 
  then
    CXXCOTHREAD="-DTHREADMODEL=COTHREADS"
    echon "Testing exception handler patch for ${CC} ... "
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
       CXXCOTHREAD="$CXXCOTHREAD -DNO_LIBGCC_HOOKS"
       CXXCOTHREAD_UNSAFE=true
    fi
  fi
  CCCOTHREAD=""
  CCCOTHREAD_UNSAFE=""
  if [ ! -z "$cc_is_gcc" ] 
  then
    CCCOTHREAD="-DTHREADMODEL=COTHREADS"
    echon "Testing exception handler patch for ${CXX} ... "
    testfile $temp.c <<\EOF
extern "C" void *(*__get_eh_context_ptr)(void);
extern "C" void *__new_eh_context(void);
void main() { __get_eh_context_ptr = &__new_eh_context; }
EOF
    if ( run $CC $CCFLAGS $temp.c -o $temp ) 
    then
       echo yes.
    else
       echo no.
       CCCOTHREAD="$CCCOTHREAD -DNO_LIBGCC_HOOKS"
       CCCOTHREAD_UNSAFE=true
    fi
  fi
  COTHREAD=tested
  CONFIG_VARS=`echo COTHREAD CXXCOTHREAD CXXCOTHREAD_UNSAFE CCCOTHREAD CCCOTHREAD_UNSAFE $CONFIG_VARS`
fi

