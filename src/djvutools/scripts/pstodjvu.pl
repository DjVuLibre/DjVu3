#!perl
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
# $Id: pstodjvu.pl,v 1.3 2001-03-23 15:58:31 debs Exp $
# $Name:  $

# Perl libs to use
use File::Basename;
use Cwd;
use File::Find;

# slash conversion for Win32
if ( $ENV{'windir'} ) {
  $slash='\\';
} else {
  $slash='/';
}
## $slash='/';

# capture working directory
$curdir=getcwd;
$curdir =~ s,/,$slash,g;

# DjVu Enterprise Commands

$documentcommand="documenttodjvu";
$bitonalcommand="documenttodjvu";
$bundlecommand="djvubundle";
$joincommand="djvujoin";


# DjVu Open Source Commands

$electroniccommand="cpaldjvu";
$mergecommand="djvm";
$splitcommand="djvmcvt";

# Default settings

$defaultprofile="--profile=clean";
$defaultdpi="300";
$outputdev="ppmraw";

# Parse arguments

$args="";
if ( $ARGV[0] )
{
  $j=$ARGV[0];
  while ( $j )
  {
    $_="$j";
    SWITCH: {
     /^--.*$/  && do {
     	SWITCH2: {
	/^--profile=.*$/ && do {
		$free="";
		$profile="$_";
		last SWITCH2;
		}; # END SW2
	(/^--best$/ || /^--best=t.*/) && do {
		$best="true";
		last SWITCH2;
		}; # END SW2
	(/^--free$/ || /^--free=t.*/) && do {
		$free="true";
		last SWITCH2;
		}; # END SW2
	/^--colors=.*$/ && do {
		$free="true";
		$args="$args ${_}";
		last SWITCH2;
		}; # END SW2
	(/^--tobitonal$/ || /^--tobitonal=t.*/) && do {
		$outputdev="pbmraw";
		$defaultprofile="--profile=bitonal";
		$args="$args ${_}";
		last SWITCH2;
		}; # END SW2
	(/^--togray$/ || /^--togray=t.*/) && do {
		$outputdev="pgmraw";
		$args="$args ${_}";
		last SWITCH2;
		}; # END SW2
	/^--dpi=.*$/ && do {
		$dpi="$_";
		$dpi =~ s/^--dpi=//;
		last SWITCH2;
		}; # END SW2
	(/^--subsample=.*$/ || /^--upsample=.*$/ 
		|| /^--resize=.*$/ ) && do {
		$best="true";
		$args="$args ${_}";
		last SWITCH2;
		}; # END SW2
	(/^--verbose$/ || /^--verbose=t.*/ ) && do {
		$verbose='true';
		if ("$cargs" eq "" ) {
		  $cargs="$_";
		} else {
		  $cargs="$cargs $_";
		}
		$args="$args ${_}";
		last SWITCH2;
		}; # END SW2
	(/^--page-range=.*/ || /^--pages-per-dict=.*/) && do {
		print STDERR "Option $_ is not supported by $0";
		$usage=1;
		last SWITCH2;
		}; # END SW2
	/^--help$/ && do {
		$usage=0;
		last SWITCH2;
		}; # END SW2
	/^.*$/ && do {
		$free="";
		$args="$args ${_}";
		last SWITCH2;
		}; # END SW2
	$nothing2=1;
	}; # END SWITCH2
        shift @ARGV;
	if ( ! $usage ) {
	  $old_targ="$_";
	  $_="$ARGV[0]";
          SWITCH3: {
          /^--.*$/  && do {
	    $j="$_";
	    last SWITCH3;
	    }; # END SW3
          /^.*$/  && do {
	    $j="";
	    last SWITCH3;
	    }; # END SW3
	  } # END SWITCH3
	  $_="$old_targ";
	} else {
	  $j="";
	}
	last SWITCH;
        }; #END SW
     /^.*$/  && do {
        $end_loop=1;
	last SWITCH;
     	}; #END SW
    $nothing=1;
    } # END SWITCH
  } # END while ( $_ = shift @ARGV)

  if ( ! $usage )
  {
    $input=shift @ARGV;
    $output=shift @ARGV;
    if ( ! -r "$input" ) {
      print STDERR "Failed to find input file '${input}'.\n";
      $usage=1;
    } elsif ( ! $output ) {
      print STDERR "Failed to specify output.\n";
      $usage=1;
    } elsif ( @ARGV ) {
      print STDERR "Too many arguments.\n";
      $usage=1;
    }
  } else {
    print STDERR "Too few arguments.\n";
    $usage=1;
  } # END if ( "$usage" eq "" )
} # END if ( $ARGV[0] ne "" )
if ( $usage ) {
  $e=basename($electroniccommand);
  $d=basename($documentcommand);
  print "\n"
  . "For usage with the DjVu3 Open Source:\n"
  . "  Bundled output:\n"
  . "    $0 --free [$e options] \\ \n"
  . '        <inputfile> <outputfile>'."\n"
  . "  Indirect output:\n"
  . "    $0 --free [$e options] \\ \n"
  . "        <inputfile> <outputdir> \n"
  . "\n"
  . "For usage with DjVu Enterprise\n"
  . "  Bundled output:\n"
  . "    $0 --profile=[profilename] [$d options] \\ \n"
  . '        [--best] <inputfile> <outputfile>'."\n"
  . "  Indirect output:\n"
  . "    $0 --profile=[profilename] [$d options] \\ \n"
  . '        [--best] <inputfile> <outputdir>'."\n"
  . "\n"
  . "For most PDF and Postscript documents, use '--free' or '--profile=clean300'.\n"
  . "\n";

  exit $usage;
} # END if ( $usage )

if ( ! $electroniccommand ) {
  $free="";
} 
if ( ! $profile ) {
  if ( ! $dpi ) {
    $dpi=$defaultdpi;
  }
  $profile="$defaultprofile$dpi";
} elsif ( ! $dpi ) {
  $dpi="$profile";
  $dpi =~ /.*\([1-6][0-5]0\)$1p/;
  if ( ! $dpi ) {
    $dpi=$defaultdpi;
  }
}
if ( $free ) {
  $rc=system "$electroniccommand -help > tmp.txt 2>&1";
  if ( -f 'tmp.txt' ) { unlink 'tmp.txt'; }
  if ( $rc != 10 ) {
    $free="";
  }
}

if ( $free || ! $best || "$dpi" eq "150" || $dpi gt 400 ) {
  $rdpi=$dpi;
} else {
  $rdpi=$dpi;
  $rdpi += $dpi;
  if ( $args ) {
    $args="$args --subsample=2";
  } else {
    $args="--subsample=2";
  }
}
if ( $free ) {
  $args =~ s/'--togray=*[a-z]*'//g;
  $args =~ s/'--tobitonal=*[a-z]*'//g;
  $args =~ s/=/ /g;
  $args =~ s/--/-/g;
  $args="'-dpi $rdpi' $args";
  $djvucommand="$electroniccommand";
} else {
  $args="--dpi=$rdpi $profile $args";
  $rc=system "$documentcommand -help > tmp.txt 2>&1";
  if ( -f 'tmp.txt' ) { unlink 'tmp.txt'; }
  if ( $rc eq 0 ) {
    $djvucommand="$documentcommand";
  } else {
    $djvucommand="$bitonalcommand";
  }
}

$tmpdir="${output}.tmp";
$name=basename("$input",".ps");
$name=basename("$name",".pdf");
if ( -d "$tmpdir" ) 
{
  print STDERR "$tmpdir exists\n";
  exit 1
}

mkdir($tmpdir, 0777);
$tfile="${tmpdir}/gs$$.pnm";
$tfile2=sprintf("${tmpdir}/${name}-%04d.djvu");
## $script="${tmpdir}${slash}run.pl";
## $script="${curdir}${slash}${tmpdir}${slash}run.pl";
## open(SFILE,">$script");
## $script_str="#!perl\n"
##   . '$verbose="' . $verbose . '";' . "\n"
##   . '$cmdstr="' . "$djvucommand $args " . '$ARGV[0] $ARGV[1]";' . "\n"
##   . 'if ( $verbose ) { print "$cmdstr\n";}' . "\n"
##   . '$stat=system($cmdstr);' . "\n"
##   . 'unlink $tmpfile;' . "\n"
##   . 'exit $stat;' . "\n"
## ;
## print SFILE $script_str;
## close SFILE;
## $cmdstr="gswin32c -dBATCH -dNOPAUSE -q -sDEVICE=$outputdev -r$rdpi -sOutputFile=$tfile $input; $script $tfile $tfile2";
## $cmdstr="gswin32c -dBATCH -dNOPAUSE -q -sDEVICE=$outputdev -r$rdpi -sOutputFile=$tfile $input; $djvucommand $args $tfile $tfile2";
$cmdstr="gswin32c -dBATCH -dNOPAUSE -q -sDEVICE=$outputdev -r$rdpi -sOutputFile=" . '"' . $tfile . '" "' . $input . '"';
if ( $verbose ) { print "$cmdstr\n"; }
system($cmdstr);
$cmdstr="$djvucommand $args " . '"' . $tfile . '" "' . $tfile2 . '"';
if ( $verbose ) { print "$cmdstr\n"; }
system($cmdstr);

find(\&wanted,$tmpdir);

if ( ! "$joincommand$bundlecommand" )
{
  $free="true";
} elsif ( ! $mergecommand ) {
  $free="";
}
if ( $free )
{
  $combine="$mergecommand";
  $split="$splitcommand";
  if ( -d $output )
  {
    $output="$output/index.djvu";
    $cmdstr="$combine -c $tmpdir/bundled.djvu $filelist";
    print "$cmdstr\n";
    system("$cmdstr");
    $cmdstr="$split -i $tmpdir/bundled.djvu " . '"' . $output . '"';
    print "$cmdstr\n";
    system("$cmdstr");
  } else {
    $cmdstr="$combine -c " . '"' . $output . '" ' . $filelist;
    print "$cmdstr\n";
    system("$cmdstr");
  }
} else {
  if ( -d $output )
  {
    $combine="$joincommand";
    $output="$output/index.djvu";
  } else {
    $combine="$bundlecommand";
  }
  unlink $output;
  $cmdstr="$combine $cargs $filelist " . '"' . $output . '"';
  if ( $verbose ) { print "$cmdstr\n"; }
  system("$cmdstr");
}
foreach $file (<$tmpdir/*>) { unlink $file; }
rmdir $tmpdir;

sub wanted {
    /^$name-.*$/ && {$filelist = "$filelist" . '"' . "$tmpdir/$_" . '" '};
}
