# This sets the variable SYS INCS JOBJ and DEFS
#

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z "$SYS" ] ; then
  echon "Checking system type ... "
  SYS=`"${uname}" -s`
  DEFS="-DUNIX"
  INCS=" "
  JOBJ=" "
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
    fi
    DEFS="$DEFS -DNEED_DJVU_MEMORY"
  fi
  echo "$SYS"
  CONFIG_VARS=`echo SYS DEFS INCS JOBJ $CONFIG_VARS`
fi

