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
  WLWHOLEARCHIVE="-Wl,--whole-archive"
  WLNOWHOLEARCHIVE="-Wl,--no-whole-archive"
  if [ "$SYS" = "Linux" ] ; then
    if [ -r /lib/libc.so.6 ] ; then
      SYS=linux-libc6
    elif [ -r /usr/lib/libc.so.6 ] ; then
      SYS=linux-libc6
    else
      SYS=linux-libc5
    fi
  elif [ "$SYS" = "SunOS" ] ; then
    s=`"${uname}" -r|"${sed}" 's,\(5.[4-9]\)[.0-9]*,SOLARIS,'`
    if [ "x$s" = xSOLARIS ] ; then
      SYS=Solaris
      if [ x$PROC = xi386 ]
      then
        SYS=Solaris-i386
      elif [ x$PROC = xsparc ]
      then
        SYS=Solaris-sparc
        DEFS="$DEFS -DNEED_DJVU_MEMORY"
      fi
    fi
  elif [ "$SYS" = "IRIX64" ] ; then
    WLWHOLEARCHIVE="-Wl,-all"
    WLNOWHOLEARCHIVE="-Wl,-notall"
  fi
  SYS_SET=true
  echo "$SYS"
  CONFIG_VARS=`echo SYS SYS_SET DEFS INCS JOBJ WLWHOLEARCHIVE WLNOWHOLEARCHIVE $CONFIG_VARS`
fi

