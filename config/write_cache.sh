# This script writes the variables currently set to standard output.

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z "$CONFIG_READONLY" ] ; then

  if [ -z "${CONFIG_CACHE}" ] ; then
    "${CONFIG_DIR}"/dirs.sh
  fi

  if [ ! -d "$TOPBUILDDIR" ] ; then
    mkdirp("$TOPBUILDDIR")
  fi

  echo CONFIG_VARS="'$CONFIG_VARS'" > "${CONFIG_CACHE}"
  for i in $CONFIG_VARS ; do
    echo "${i}='"`eval echo \"\$"$i"\"`"'" >> "${CONFIG_CACHE}"
  done
fi

