#C-
#C- DjVu® Reference Library (v. 3.0)
#C- 
#C- Copyright © 2000-2001 LizardTech, Inc. All Rights Reserved.
#C- The DjVu Reference Library is protected by U.S. Pat. No.
#C- 6,058,214 and patents pending.
#C- 
#C- This software is subject to, and may be distributed under, the
#C- GNU General Public License, Version 2. The license should have
#C- accompanied the software or you may obtain a copy of the license
#C- from the Free Software Foundation at http://www.fsf.org .
#C- 
#C- The computer code originally released by LizardTech under this
#C- license and unmodified by other parties is deemed the "LizardTech
#C- Original Code."
#C- 
#C- With respect to the LizardTech Original Code ONLY, and subject
#C- to any third party intellectual property claims, LizardTech
#C- grants recipient a worldwide, royalty-free, non-exclusive license
#C- under patent claims now or hereafter owned or controlled by
#C- LizardTech that are infringed by making, using, or selling
#C- LizardTech Original Code, but solely to the extent that any such
#C- patent(s) is/are reasonably necessary to enable you to make, have
#C- made, practice, sell, or otherwise dispose of LizardTech Original
#C- Code (or portions thereof) and not to any greater extent that may
#C- be necessary to utilize further modifications or combinations.
#C- 
#C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
#C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
#C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
#C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id: write_cache.sh,v 1.13 2001-01-04 22:04:54 bcr Exp $
# $Name:  $

# This script writes the variables currently set to standard output.

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z "$CONFIG_READONLY" ] ; then
  p=`"${pwdcmd}"`
  echo "Writing the ${CONFIG_CACHE} file"|"${sed}" -e "s! ${p}[/]*! !" 
  if [ ! -d "$TOPBUILDDIR" ] ; then
    ${mkdirp} "$TOPBUILDDIR"
  fi
  CONFIG_VARS=`sortlist $CONFIG_VARS`
  echo CONFIG_VARS="'$CONFIG_VARS'" > "${CONFIG_CACHE}"
  for i in $CONFIG_VARS ; do
    s='escape $'"${i}"
    echo "${i}='`eval $s`'" >> "${CONFIG_CACHE}"
  done
fi


