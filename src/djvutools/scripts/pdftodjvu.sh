#!/bin/sh
#C-
#C- DjVu® Reference Library (v. 3.0)
#C- 
#C- Copyright © 2001 LizardTech, Inc. All Rights Reserved.
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
#C- 
#
# $Id: pdftodjvu.sh,v 1.6 2001-02-06 21:48:09 bcr Exp $
# $Name:  $

# DjVu Enterprise Commands

documentcommand="documenttodjvu"
bitonalcommand="documenttodjvu"
bundlecommand=djvubundle
joincommand=djvujoin


# DjVu Open Source Commands

electroniccommand=cpaldjvu
mergecommand=djvm
splitcommand=djvmcvt

# Default settings

defaultprofile="--profile=clean"
defaultdpi="300"
outputdev="pnmraw"

# Parse arguments

args=""
if [ -n "$1" ]
then
  j="$1"
  case "$j" in
    "--"?*)
      while [ -n "$j" ] ; do
        case "$j" in
          "--profile="*)
             free=""
             profile="$j" ;;
          "--best"|"--best=t"*)
             best=true ;;
          "--free"|"--free=t"*)
             free=true ;;
          "--colors="*)
             free=true 
             args="$args '$j'" ;;
          "--tobitonal"|"--tobitonal=t"*)
             outputdev=pbmraw
             defaultprofile="--profile=bitonal"
             args="$args '$j'" ;;
          "--togray"|"--togray=t"*)
             outputdev=pgmraw
             args="$args '$j'" ;;
          "--dpi="*)
             dpi=`echo $j|sed 's,--dpi=,,'` ;;
          "--subsample="*|"--upsample="*|"--resize="*)
             best=true
             args="$args '$j'" ;;
          "--verbose"|"--verbose=t"*)
             verbose=true
             cargs=`echo $cargs $j`
             args="$args '$j'" ;;
          "--page-range="*|"--pages-per-dict=*")
             echo "Option $j is not supported by %0" 1>&2
             usage=1 ;;
          "--help")
             usage=0 ;;
          *)
             free=""
             args="$args '$j'" ;;
        esac
        shift
        if [ -z "$usage" ]
        then
          case "$1" in
            "--"?*) j="$1" ;;
            *) j="" ;;
          esac
        else
          j="";
        fi
      done ;;
    *) ;;
  esac
  if [ -z "$usage" ]
  then
    input="$1"
    shift
    output="$1"
    shift
    if [ ! -r "$input" ]
    then
      echo "Failed to find input file '$input'." 1>&2
      usage=1
    elif [ -z "$output" ]
    then
      echo "Failed to specify output." 1>&2
      usage=1
    elif [ -n "$1" ]
    then
      echo "Too many arguments." 1>&2
      usage=1
    fi
  else 
    echo "Too few arguments." 1>&2
    usage=1
  fi
fi
if [ -n "$usage" ] 
then
  e=`basename "$electroniccommand"`
  d=`basename "$documentcommand"`
  cat <<+
For usage with the DjVu3 Open Source:
  Bundled output:
    $0 --free [$e options] \\
        <inputfile> <outputfile>
  Indirect output:
    $0 --free [$e options] \\
        <inputfile> <outputdir>

For usage with DjVu Enterprise
  Bundled output:
    $0 --profile=[profilename] [$d options] \\
        [--best] <inputfile> <outputfile>
  Indirect output:
    $0 --profile=[profilename] [$d options] \\
        [--best] <inputfile> <outputdir>

For most PDF and Postscript documents, use '--free' or '--profile=clean300'.
+
  
  exit $usage
fi

if [ -z "$electroniccommand" ]
then
  free=""
fi
if [ -z "$profile" ] 
then
  if [ -z "$dpi" ] 
  then
    dpi=$defaultdpi
  fi
  profile="$defaultprofile$dpi"
elif [ -z "$dpi" ]
then
  dpi=`echo "$profile"|sed -n -e 's,.*\([1-6][0-5]0\)$,\1,p'`
  if [ -z "$dpi" ]
  then
    dpi=$defaultdpi  
  fi
fi
if [ -n "$free" ]
then
  $electroniccommand -help 2>>/dev/null 1>>/dev/null
  if [ $? -ne 10 ]
  then
    free=""
  fi
fi

if [ -n "$free" -o -z "$best" -o "$dpi" = "150" -o "$dpi" -gt 400 ]
then
  rdpi="$dpi"
else
  rdpi="`expr $dpi + $dpi`"
  args=`echo $args "--subsample=2"`
fi
if [ -n "$free" ]
then
  args="-dpi $rdpi "`echo "$args"|sed -e "s,'--togray[='][a-z]*,,g" -e "s,'--tobitonal[='][a-z]*,,g" -e "s,=,' ',g" -e "s,--,-,g"`
  djvucommand=$electroniccommand
else
  args="--dpi=$rdpi $profile --page-range=1 $args"
  if ( $documentcommand --help </dev/null 1>>/dev/null 2>>/dev/null )
  then
    djvucommand=$documentcommand
  else
    djvucommand=$bitonalcommand
  fi
fi

tmpdir="$output.tmp" 
name=`basename "$input" .ps`
name=`basename "$name" .pdf`
if [ -d "$tmpdir" ] 
then
  echo "$tmpdir" exists 1>&2
  exit 1
fi

mkdir "$tmpdir"
script="$tmpdir/run.sh"
sed 's,%-dollar-%,\$,g' <<+ > "$script"
#!/bin/sh
tmpfile=$tmpdir/gs$$.pnm
cat > "%-dollar-%tmpfile"
if [ -n "$verbose" ]
then
  echo "$djvucommand $args %-dollar-%tmpfile %-dollar-%1"
fi
"$djvucommand" $args "%-dollar-%tmpfile" "%-dollar-%1"
stat=%-dollar-%?
rm -f "%-dollar-%tmpfile"
exit %-dollar-%stat
+
chmod ugo+x "$script"
if [ -n "$verbose" ]
then
  echo "gs -dNOPAUSE -q -sDEVICE=$outputdev -r$rdpi -sOutputFile=|'$script' '$tmpdir/$name-%04d.djvu' $input"
fi
gs -dBATCH -dNOPAUSE -q "-sDEVICE=$outputdev" "-r$rdpi" -sOutputFile="|'$script' '$tmpdir/$name-%04d.djvu'" "$input"

if [ -z "$joincommand$bundlecommand" ]
then
  free="true"
elif [ -z "$mergecommand" ]
then
  free=""
fi
if [ -n "$free" ]
then
  combine=$mergecommand 
  split=$splitcommand
  if [ -d "$output" ] 
  then
    output="$output/index.djvu"
    echo "$combine -c $tmpdir/bundled.djvu $tmpdir/$name-*.djvu"
    "$combine" -c "$tmpdir/bundled.djvu" "$tmpdir/$name-"*.djvu
    echo "$split -i $tmpdir/bundled.djvu $output"
    "$split" -i "$tmpdir/bundled.djvu" "$output"
  else
    echo "$combine -c $output $tmpdir/$name-*.djvu"
    "$combine" -c "$output" "$tmpdir/$name-"*.djvu
  fi
else
  if [ -d "$output" ] 
  then
    combine="$joincommand"
    output="$output/index.djvu"
  else
    combine="$bundlecommand"
  fi
  rm -f "$output"
  if [ -n "$verbose" ]
  then
    echo "$combine $cargs $tmpdir/$name-*.djvu $output"
  fi
  "$combine" $cargs "$tmpdir/$name-"*.djvu "$output"
fi
rm -rf "$tmpdir"

