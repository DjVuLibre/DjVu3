# This script reads the relevant profile in /etc/DjVu to set the
# default prefix.
#

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z "$prefix" ] ; then
  INSTALL_CONFIG="/etc/DjVu/${PROJECT}.conf"
  INSTALL_CONFIG_DIR=`dirname "$INSTALL_CONFIG"`
  if [ -d "$INSTALL_CONFIG_DIR" ] ; then
    if [ -r "$INSTALL_CONFIG" ] ; then
      . "$INSTALL_CONFIG"
    else
      eval `"${grep}" '^[ 	]*prefix[ 	]*=' "$INSTALL_CONFIG_DIR"/*.conf` 2>>/dev/null
    fi
  fi
  if [ -z "$prefix" ] ; then
    if [ -d "/opt" ] ; then
      prefix="/opt"
    elif [ -d "/usr/local" ] ; then
      prefix="/usr/local"
    else
      prefix="/usr"
    fi
  fi
  PROJECT_PREFIX="$prefix/ATT-DjVu/$PROJECT_FULLNAME"
  CONFIG_VARS=`echo prefix INSTALL_CONFIG INSTALL_CONFIG_DIR PROJECT_PREFIX "$CONFIG_VARS"`
fi

