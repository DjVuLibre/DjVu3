# This rule sets the following variables:
#	CXX, CXXFLAGS, CXXSYMBOLIC, CXXPIC, CXXUNROLL, CXXWARN

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
  echon "Searching for C++ compiler ... "
  if ( run $EGXX -c $temp.cpp ) ; then
    CXX="$EGXX"
  elif ( run g++ -c $temp.cpp ) ; then
    CXX=c++
  elif ( run CC -c $temp.cpp ) ; then
    CXX=CC
  else 
    echo "none available"
    echo "Error: Can't find a C++ compiler" 1>&2
    exit 1
  fi
  echo "$CXX"

  echon "Checking ${CXX} version ... "
  CXXVERSION=""
  if [ "$CXX" != "$EGXX" ] ; then
    s=`$CXX -Vfoo 2>&1|"${grep}" 'file path prefix'|"${sed}" 's,.* .\([^ ]*/\)foo/. never.*,\1,'`
    if [ ! -z "$s" ]
    then
      echo "egcs"
    else
      EGCSTEST=`($CXX -v 2>&1)|"${sed}" -n -e 's,.*/egcs-.*,Is EGCS,p' -e 's,.*/pgcc-.*,Is PGCC,p'`
      if [ ! -z "$EGCSTEST" ] ; then
        echo "egcs"
      elif ( run $CXX -V2.8.1 -c $temp.cpp ) ; then
        echo "gcc 2.8.1"
        CXXVERSION="-V2.8.1"
      else
        echo "unknown"
        echo WARNING: DjVu is designed to compile with egcs or g++ version 2.8.1 1>&2
      fi
    fi
  else
    echo egcs
  fi
  CXXFLAGS="$CXXVERSION"

  echon "Checking if ${CXX} -pipe works ... "
  CXXPIPE=""
  if ( run $CXX ${CXXFLAGS} -pipe -c $temp.cpp ) ; then
    CXXPIPE="-pipe"
    echo yes
  else
    echo no
  fi
  CXXFLAGS=`echo "${CXXPIPE}" "${CXXFLAGS}"`

  CXXMMX=""
  if [ `uname -m` = i686 ]
  then
    echon "Checking whether ${CXX} -mpentiumpro and -mmx work ... "
    if ( run $CXX ${CXXFLAGS} -mpentiumpro -c $temp.cpp ) 
    then
      CXXMMX="-mpentiumpro"
      if ( run $CXX ${CXXFLAGS} ${CXXMMX} -mmx -c $temp.cpp ) ; then
        CXXMMX="$CXXMMX -mmx"
        echo "yes, both work"
      else
        echo "just $CXXMMX works"
      fi
    else
      echo "no"
    fi
    CXXFLAGS=`echo "${CXXMMX}" "${CXXFLAGS}"`
  fi

  echon "Checking ${CXX} symbolic option ... "
  CXXSYMBOLIC=""
  s=`(cd $tempdir 2>>/dev/null;${CXX} ${CXXFLAGS} -symbolic -c $temp.cpp 2>&1)|"${grep}" 'unrecognized option'`
  if [ -z "$s" ]
  then
    echo " -symbolic"
    CXXSYMBOLIC='-symbolic'
  elif ( run ${CXX} ${CXXFLAGS} -shared -Wl,-Bsymbolic -o $temp.so $temp.cpp -lc -lm ) ; then
    if [ "$SYS" != "linux-libc6" ] ; then
      CXXSYMBOLIC='-Wl,-Bsymbolic'
    fi
  fi
  if [ -z "$CXXSYMBOLIC" ] ; then
    echo "none"
  else
    echo "$CXXSYMBOLIC"
  fi

  echon "Checking whether ${CXX} -fPIC works ... "
  if ( run $CXX ${CXXFLAGS} -fPIC -c $temp.cpp ) ; then
    echo yes
    CXXPIC="-fPIC"
  else
    echo no
    CXXPIC=""
  fi

  echon "Checking whether ${CXX} is gcc ... "
  echo 'int main(void) { return __GNUG__;}' | testfile $temp.cpp
  CXXOPT=""
  CXXUNROLL=""
  CXXWARN=""
  if ( run $CXX $CXXFLAGS -c $temp.cpp ) 
  then
    echo yes
    cxx_is_gcc=yes
    CXXWARN="-Wall"
    echon "Checking whether ${CXX} -O3 works ... "
    if ( run $CXX ${CXXFLAGS} -O3 -c $temp.cpp ) ; then
      echo yes
      CXXOPT="-O3"
    else
      echo no
    fi
    echon "Checking whether ${CXX} -funroll-loops works ... "
    if ( run $CXX ${CXXFLAGS} ${CXXOPT} -funroll-loops -c $temp.cpp )
    then
      echo yes
      CXXUNROLL="-funroll-loops"
    else
      echo no
    fi
  else
    echo no
  fi
  if [ -z "$CXXOPT" ] ; then
    echon "Checking whether ${CXX} -O works ... "
    if ( run $CXX ${CXXFLAGS} -O -c $temp.cpp ) ; then
      echo yes
      CXXOPT="-O"
    else
      echo no
      CXXOPT=""
    fi
  fi
  "${rm}" -rf $temp.cpp $temp.so $temp.o
  CONFIG_VARS=`echo CXX CXXFLAGS CXXOPT CXXUNROLL CXXWARN CXXSYMBOLIC CXXPIC cxx_is_gcc $CONFIG_VARS`
fi

