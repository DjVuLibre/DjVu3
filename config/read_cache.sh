# This script reads the variables from the cache file.

if [ -z "${CONFIG_DIR}" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi
if [ -z "${CONFIG_CACHE}" ] ; then
  . ${CONFIG_DIR}/dirs.sh
fi

if [ -r "${CONFIG_CACHE}" ] ; then
  THISSYS="$SYS"
  SYS=""
  . "${CONFIG_CACHE}"
  if [ "x${SYS}" != "x${THISSYS}" ] ; then
    for i in $CONFIG_VARS ; do
      eval `$i=''`
    done
  fi
  SYS="$THISSYS"
fi

