# This rule sets the following variables:
#	CXX, CXXFLAGS, CXXSYMBOLIC, CXXPIC, CXXUNROLL, CXXWARN

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

ECXX="eg++"
if [ -z "$CXX_SET" ] ; then
  echo 'extern "C" {void exit(int);};void foo(void) {exit(0);}' |testfile $temp.cpp
  
  # I added CXX_OVERRIDE to be able to select compiler myself and let
  # you do the rest (flags and options) -eaf
  if [ -z "$CXX_OVERRIDE" ]; then
    CXXFLAGS=""
    CXXSYMBOLIC=""
    CXXPIC=""
    cxx_is_gcc=
    echon "Searching for C++ compiler ... "
    if ( run $EGXX -c $temp.cpp ) ; then
      CXX="$EGXX"
      echo "$CXX"
    elif [ ! -z "$CXX" ] ; then
      if ( run g++ -c $temp.cpp ) ; then
        echo "$CXX"
      else
        CXX=""
      fi
    fi
    if [ -z "$CXX" ] ; then
      if ( run g++ -c $temp.cpp ) ; then
        CXX=c++
      elif ( run CC -c $temp.cpp ) ; then
        CXX=CC
      else 
        echo "none available"
        echo "Error: Can't find a C++ compiler" 1>&2
        exit 1
      fi
      echo "$CXX"
    fi
  fi
  if [ -z "$CC" ] 
  then
    CC="$CXX -x c -fexceptions"
  fi
  echon "Checking ${CXX} version ... "
  CXXVERSION=""
  if [ "$CXX" != "$EGXX" ] ; then
    s=`$CXX -Vfoo 2>&1|"${grep}" 'file path prefix'|"${sed}" 's,.* .\([^ ]*/\)foo/. never.*,\1,'`
    if [ ! -z "$s" ]
    then
      echo "egcs"
    else
      # This test fails for pre-release versions of egcs (-eaf)
      EGCSTEST=`($CXX -v 2>&1)|"${sed}" -n -e 's,.*/egcs-.*,Is EGCS,p' -e 's,.*/pgcc-.*,Is PGCC,p'`
      if [ ! -z "$EGCSTEST" ] ; then
        echo "egcs"
      elif ( run $CXX -V2.8.1 -c $temp.cpp ) ; then
        echo "gcc 2.8.1"
        CXXVERSION="-V2.8.1"
      else
        (echo '#include <stdio.h>';\
	 echo 'int main(void) { printf("%d", __GNUC__); }') | testfile $temp.cpp
	if (run $CXX $temp.cpp -o $temp.exe)
	then
	  MAJOR=`$temp.exe`
	fi
	(echo '#include <stdio.h>';\
	 echo 'int main(void) { printf("%d", __GNUC_MINOR__); }') | testfile $temp.cpp
	if (run $CC $temp.cpp -o $temp.exe)
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
          echo "unknown"
          echo WARNING: DjVu is designed to compile with egcs or g++ version 2.8.1 1>&2
	fi
	rm -f $temp.exe
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
    echon "Checking ${CXX} supports pentium optimizations ... "
    check_compile_flags CXXMMX $temp.cpp "-mpentiumpro -mmx" "-mpentiumpro"
    if [ -z "$CXXMMX" ] ; then
      echo "none"
    else
      echo "$CXXMMX"
      CXXFLAGS="${CXXMMX} ${CXXFLAGS}"
    fi
  fi

  echon "Checking ${CXX} symbolic option ... "
  CXXSYMBOLIC=""
  if [ "$SYS" != "linux-libc6" ] ; then
    check_link_flags CXXSYMBOLIC $temp.cpp "-shared -symbolic" "-shared -Wl,-Bsymbolic" "-shared -Wl,-Bsymbolic -lc" "-shared -Wl,-Bsymbolic -lc -lm"
  fi
  if [ -z "$CXXSYMBOLIC" ] ; then
    echo "none"
  else
    echo "$CXXSYMBOLIC"
  fi

  echon "Checking whether ${CXX} -fPIC works ... "
  check_compile_flags CXXPIC $temp.cpp "-fPIC"
  if [ $? = 0 ]
  then
    echo yes
  else
    echo no
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
  CXX_SET=true
  "${rm}" -rf $temp.cpp $temp.so $temp.o
  CONFIG_VARS=`echo CXX_SET CXX CXXFLAGS CXXOPT CXXUNROLL CXXWARN CXXSYMBOLIC CXXPIC cxx_is_gcc $CONFIG_VARS`
fi

