# This rule sets the following variables:
#	CXX, CXXFLAGS, CXXSYMBOLIC, CXXPIC
#
# This requires $SYS be set.
#
# Note: A value of false should be replaced with "".

. `dirname $0`/sys.sh

ECXX="eg++"
if [ -z "$CXX" ] ; then
  CXXFLAGS=""
  CXXSYMBOLIC=""
  CXXPIC=""
  echo Searching for C++ compiler
  echo 'int foo(void) {return 0;}' > /tmp/cpp$$.cpp
  if ( cd /tmp 2>>/dev/null;$EGXX -c cpp$$.cpp 1>>/dev/null 2>>/dev/null ) ; then
    CXX="$EGXX"
  elif ( cd /tmp 2>>/dev/null;g++ -c cpp$$.cpp 1>>/dev/null 2>>/dev/null ) ; then
    CXX=g++
  elif ( cd /tmp 2>>/dev/null;c++ -c cpp$$.cpp 1>>/dev/null 2>>/dev/null ) ; then
    CXX=c++
  elif ( cd /tmp 2>>/dev/null;CC -c cpp$$.cpp 1>>/dev/null 2>>/dev/null ) ; then
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
      echo 'int foo(void) {return 0;}' > /tmp/cpp$$.cpp
      if [ ! -z "$EGCSTEST" ] ; then
        echo "$CXX is egcs"
      elif ( cd /tmp 2>>/dev/null;$CXX -V2.8.1 -c cpp$$.cpp  1>>/dev/null 2>>/dev/null ) ; then
        CXXVERSION="-V2.8.1"
      else
        echo WARNING: DjVu is designed to compile with egcs or g++ version 2.8.1 1>&2
      fi
    fi
  fi
  CXXFLAGS="$CXXVERSION"

  echo 'int foo(void) {return 0;}' > /tmp/cpp$$.cpp
  CXXPIPE=""
  if ( cd /tmp 2>>/dev/null;$CXX ${CXXFLAGS} -pipe -c cpp$$.cpp  1>>/dev/null 2>>/dev/null ) ; then
    echo "Testing ${CXX} pipe option"
    CXXPIPE="-pipe"
  fi
  CXXFLAGS=`echo "${CXXPIPE}" "${CXXFLAGS}"`

  echo 'int foo(void) {return 0;}' > /tmp/cpp$$.cpp
  CXXMMX=""
  echo "Testing ${CXX} MMX option"
  if [ `uname -m` = i686 ]
  then
    if (cd /tmp 2>>/dev/null;$CXX ${CXXFLAGS} -mpentiumpro -c cpp$$.cpp) 
    then
      CXXMMX="-mpentiumpro"
      if ( cd /tmp 2>>/dev/null;$CXX ${CXXFLAGS} ${CXXMMX} -mmx -c cpp$$.cpp  1>>/dev/null 2>>/dev/null ) ; then
        CXXMMX="$CXXMMX -mmx"
      fi
    fi
  fi
  CXXFLAGS=`echo "${CXXMMX}" "${CXXFLAGS}"`

  echo "Testing ${CXX} Symbolic Option"
  CXXSYMBOLIC=""
  echo 'extern "C" {void exit(int);};void foo(void) {exit(0);}' > /tmp/cpp$$.cpp
  if [ -z "`(cd /tmp 2>>/dev/null;${CXX} ${CXXFLAGS} -symbolic -c cpp$$.cpp 2>&1)|grep 'unrecognized option'`" ] ; then
    CXXSYMBOLIC='-symbolic'
    echo "Using CXXSYMBOLIC=$CXXSYMBOLIC"
  elif (cd /tmp 2>>/dev/null;${CXX} ${CXXFLAGS} -shared -Wl,-Bsymbolic -o cpp$$.so cpp$$.cpp -lc -lm 2>>/dev/null 1>>/dev/null) ; then
    if [ "$SYS" != "linux-libc6" ] ; then
      CXXSYMBOLIC='-Wl,-Bsymbolic'
      echo "Using CXXSYMBOLIC=$CXXSYMBOLIC"
    fi
  fi

  echo "Testing ${CXX} PIC option"
  echo 'int foo(void) {return 0;}' > /tmp/cpp$$.cpp
  if ( cd /tmp 2>>/dev/null;$CXX ${CXXFLAGS} -fPIC -c cpp$$.cpp 1>>/dev/null 2>>/dev/null ) ; then
    CXXPIC="-fPIC"
  else
    CXXPIC=""
  fi
  rm -rf /tmp/cpp$$.cpp /tmp/cpp$$.o
  CONFIG_VARS=`echo CXX CXXFLAGS CXXSYMBOLIC CXXPIC "$CONFIG_VARS"`
fi

