# This script reads the variables from the cache file.

if [ -z "${CONFIG_DIR}" ] ; then
  . `dirname $0`/functions.sh
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

