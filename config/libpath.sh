# This sets the variable SYS and DEFS
#

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z "$LIBPATHS" ]
then
  LIBPATHS=/usr/X11R6/lib:/usr/openwin/lib:/usr/lib:/lib
  if [ ! -z "$prefix" ] ; then
    LIBPATHS="$prefix/lib:$LIBPATHS"
  fi
  if [ ! -z "$PROJECT_PREFIX" ] ; then
    LIBPATHS="$PROJECT_PREFIX/lib:$LIBPATHS"
  fi
  if [ ! -z "$LD_LIBRARY_PATH" ] ; then
    LIBPATHS="$LD_LIBRARY_PATH:$LIBPATHS"
  fi
  if [ -r /etc/ld.so.conf ] ; then
    for i in `grep -v '^ *$' < /etc/ld.so.conf`; do
      if [ -d "$i" ] ; then
        LIBPATHS="$LIBPATHS:$i"
      fi
    done
  fi
  LIBPATHS=`escape "$LIBPATHS"|sed -e 's,:, ,g'`
  INCPATHS=`echo "$LIBPATHS"|sed -e 's,$,:,g' -e 's,lib:,include:,g' -e 's,:$,,g'`
  SOPATHS=`echo "$LIBPATHS"|sed -e 's,:/usr/openwin/lib:,:,g'`
  CONFIG_VARS=`echo LIBPATHS INCPATHS SOPATHS $CONFIG_VARS`
fi

