# This sets the variable SYS INCS JOBJ and DEFS
#

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z "$SYS_SET" ] ; then
  echon "Checking system type ... "
  if [ -z "$SYS" ] ; then 
    SYS=`"${uname}" -s`
  fi
  if [ -z "$PROC" ] ; then 
    PROC=`"${uname}" -p`
  fi
  DEFS="-DUNIX"
  INCS=" "
  JOBJ=" "
  SECURITYMODE="-D_LOCK_ID_"
  SENTINAL_NEED_LIB=""
  SENTINAL=""
  WHOLEARCHIVESEP=" "
  WHOLEARCHIVE="-Wl,--whole-archive"
  NOWHOLEARCHIVE="-Wl,--no-whole-archive"
  LIBDL=""
  LIBC=""
  if [ "$SYS" = "Linux" ] ; then
    if [ -r /usr/lib/libdl.a ] ; then
      LIBDL=/usr/lib/libdl.a
    elif [ -r /lib/libdl.a ] ; then
      LIBDL=/lib/libdl.a
    fi
    if [ -r /lib/libc.so.6 ] ; then
      LIBC="libc.so.6"
      SYS=linux-libc6
    elif [ -r /usr/lib/libc.so.6 ] ; then
      LIBC="libc.so.5"
      SYS=linux-libc6
    else
      SYS=linux-libc5
    fi
    SENTINAL=src/3rd-party/sentinal_lm_60/linux
  elif [ "$SYS" = "SunOS" ] ; then
    s=`"${uname}" -r|"${sed}" 's,\(5.[4-9]\)[.0-9]*,SOLARIS,'`
    WHOLEARCHIVESEP=","
    if [ "x$s" = xSOLARIS ] ; then
      SYS=Solaris
      if [ x$PROC = xi386 ]
      then
        SYS=Solaris-i386
      elif [ x$PROC = xsparc ]
      then
        SYS=Solaris-sparc
        SENTINAL=src/3rd-party/sentinal_lm_60/sol2sprc
	SENTINAL_NEED_LIB="/usr/lib/libsocket.a /usr/lib/libnsl.a"
        DEFS="$DEFS -DNEED_DJVU_MEMORY"
      fi
    fi
  elif [ "$SYS" = "IRIX64" ] ; then
    WHOLEARCHIVE="-Wl,-all"
    NOWHOLEARCHIVE="-Wl,-notall"
    SENTINAL=src/3rd-party/sentinal_lm_60/irixn32
    strip=""
  fi
  if [ -n "$SENTINAL" ]
  then
    if [ ! -d "${CONFIG_DIR}/../$SENTINAL" ]
    then 
      SENTINAL=""
      SENTINAL_NEED_LIB=""
    fi
  fi
  SYS_SET=true
  echo "$SYS"
  CONFIG_VARS=`echo SYS SYS_SET LIBC LIBDL DEFS INCS JOBJ WHOLEARCHIVE WHOLEARCHIVESEP NOWHOLEARCHIVE SENTINAL SENTINAL_NEED_LIB $CONFIG_VARS`
fi

