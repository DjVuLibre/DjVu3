# This sets the variable SYS
#

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z "$SYS" ] ; then
  echo "Detecting system type"
  SYS=`uname -s`
  if [ "$SYS" = "Linux" ] ; then
    if [ -r /lib/libc.so.6 ] ; then
      SYS=linux-libc6
    elif [ -r /usr/lib/libc.so.6 ] ; then
      SYS=linux-libc6
    else
      SYS=linux-libc5
    fi
  elif [ "$SYS" = "SunOS" ] ; then
    if [ "`uname -r|sed 's,\(5.[4-9]\)[.0-9]*,SOLARIS,'`" = SOLARIS ] ; then
      SYS=Solaris
    fi
  fi
  echo "Configuring for $SYS"
  CONFIG_VARS=`echo SYS "$CONFIG_VARS"`
fi

