# This rule sets the following variables:
#	CXX, CXXFLAGS, CXXSYMBOLIC, CXXPIC

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

ECXX="eg++"
if [ -z "$CXX" ] ; then
  echo 'extern "C" {void exit(int);};void foo(void) {exit(0);}' |testfile $temp.cpp
  CXXFLAGS=""
  CXXSYMBOLIC=""
  CXXPIC=""
  cxx_is_gcc=
  echo Searching for C++ compiler
  if ( run $EGXX -c $temp.cpp ) ; then
    CXX="$EGXX"
  elif ( run g++ -c $temp.cpp ) ; then
    CXX=c++
  elif ( run CC -c $temp.cpp ) ; then
    CXX=CC
  else 
    echo "Error: Can't find a C++ compiler" 1>&2
    exit 1
  fi

  CXXVERSION=""
  if [ "$CXX" != "$EGXX" ] ; then
    echo "Testing ${CXX} version"
    CXXprefix="`$CXX -Vfoo 2>&1|fgrep 'file path prefix'|sed 's,.* .\([^ ]*/\)foo/. never.*,\1,'`"
    if [ ! -z "$CXXprefix" ] ; then
      echo "$CXX is egcs"
    else
      EGCSTEST=`($CXX -v 2>&1)|sed -n -e 's,.*/egcs-.*,Is EGCS,p' -e 's,.*/pgcc-.*,Is PGCC,p'`
      if [ ! -z "$EGCSTEST" ] ; then
        echo "$CXX is egcs"
      elif ( run $CXX -V2.8.1 -c $temp.cpp ) ; then
        CXXVERSION="-V2.8.1"
      else
        echo WARNING: DjVu is designed to compile with egcs or g++ version 2.8.1 1>&2
      fi
    fi
  fi
  CXXFLAGS="$CXXVERSION"

  CXXPIPE=""
  if ( run $CXX ${CXXFLAGS} -pipe -c $temp.cpp ) ; then
    echo "Testing ${CXX} pipe option"
    CXXPIPE="-pipe"
  fi
  CXXFLAGS=`echo "${CXXPIPE}" "${CXXFLAGS}"`

  CXXMMX=""
  echo "Testing ${CXX} MMX option"
  if [ `uname -m` = i686 ]
  then
    if ( run $CXX ${CXXFLAGS} -mpentiumpro -c $temp.cpp ) 
    then
      CXXMMX="-mpentiumpro"
      if ( run $CXX ${CXXFLAGS} ${CXXMMX} -mmx -c $temp.cpp ) ; then
        CXXMMX="$CXXMMX -mmx"
      fi
    fi
  fi
  CXXFLAGS=`echo "${CXXMMX}" "${CXXFLAGS}"`

  echo "Testing ${CXX} Symbolic Option"
  CXXSYMBOLIC=""
  if [ -z "`(cd $tempdir 2>>/dev/null;${CXX} ${CXXFLAGS} -symbolic -c $temp.cpp 2>&1)|grep 'unrecognized option'`" ] ; then
    CXXSYMBOLIC='-symbolic'
    echo "Using CXXSYMBOLIC=$CXXSYMBOLIC"
  elif ( run ${CXX} ${CXXFLAGS} -shared -Wl,-Bsymbolic -o $temp.so $temp.cpp -lc -lm ) ; then
    if [ "$SYS" != "linux-libc6" ] ; then
      CXXSYMBOLIC='-Wl,-Bsymbolic'
      echo "Using CXXSYMBOLIC=$CXXSYMBOLIC"
    fi
  fi

  echo "Testing ${CXX} PIC option"
  if ( run $CXX ${CXXFLAGS} -fPIC -c $temp.cpp ) ; then
    CXXPIC="-fPIC"
  else
    CXXPIC=""
  fi
  echon "Checking whether ${CXX} is gcc ... "
  echo 'int main(void) { return __GNUG__;}' | testfile $temp.cpp
  if ( run $CXX $CXXFLAGS -c $temp.cpp ) 
  then
      cxx_is_gcc=yes
      echo yes
  else
      echo no
  fi
  rm -rf $temp.cpp $temp.so $temp.o
  CONFIG_VARS=`echo CXX CXXFLAGS CXXSYMBOLIC CXXPIC cxx_is_gcc "$CONFIG_VARS"`
fi

