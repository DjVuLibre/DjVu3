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
# $Id: pdftodjvu.pl,v 1.8 2001-05-17 00:16:24 debs Exp $
# $Name:  $

# Perl libs to use
use File::Basename;
use Cwd;
use File::Find;
use File::Copy;

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

# max number of djvu files which can be bundled (this is to support a 
# workaround for a # known limitation of DjVu Enterprise for Win32)
if ( $ENV{'PDFTODJVU_DEBUG'} ) {
  $max_bundle = 5;
  $max_files = 10;
} else {
  $max_bundle = 250;
  $max_files = 500;
}
$max_ifiles = $max_files / $max_bundle;
$_ = $max_ifiles;
if ( /\./ ) { $max_ifiles = sprintf("%d", ++$max_ifiles); }

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
        $j="";
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
} else { 
    print STDERR "Too few arguments.\n";
    $usage=1;
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
  $dpi =~ s/^.*([1-6][0-5]0)$/$1/;
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
  open(IFILE, "<tmp.txt");
  if ( -f 'tmp.txt' ) { unlink 'tmp.txt'; }
  if ( $rc eq 0 ) {
    $djvucommand="$documentcommand";
  } else {
    $djvucommand="$bitonalcommand";
  }
}

$tmpdir="${output}.tmp";
($name, $dir, $type) = fileparse($input, '\..*');
$type = lc($type);
if ( -d "$tmpdir" ) 
{
  print STDERR "$tmpdir exists\n";
  exit 1
}
mkdir($tmpdir, 0777);

# if this is a postscript file, convert it to a PDF first.

if ( $type eq ".ps" ) {
  $pdffile = "$tmpdir/$name.pdf";
  $cmdstr="gswin32c -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -r$rdpi " . 
    "-sOutputFile=" . '"' . $pdffile . '" "' .  $input . '" 2>&1';
  if ( $verbose ) { print "$cmdstr\n"; }
  $outputstr = `$cmdstr`;
  $outputstr =~ s/\*+Unknown operator: ri\n//g;
  if ( $verbose ) { print $outputstr; }
  $_ = $outputstr;
  /Error:/ && die "Error creating interim PDF file:\n$outputstr\n";
} else {
  $pdffile = $input;
}

# create a DjVu file for each page of the PDF file being converted
$pg=0;
$do_next=1;
while ( $do_next ) {
  ++$pg;
  $gsname = $tmpdir . "/gs$$.pnm";
  $djname = $tmpdir . "/gs$$" . sprintf("%04d", $pg) . '.djvu';
  $cmdstr="gswin32c -dBATCH -dNOPAUSE -dFirstPage=$pg -dLastPage=$pg -q " .
    "-sDEVICE=$outputdev -r$rdpi -sOutputFile=" . '"' . "$gsname" . '" "' .
    $pdffile . '" 2>&1';
  if ( $verbose ) { print "$cmdstr\n"; }
  $outputstr=`$cmdstr`;
  $outputstr =~ s/\*+Unknown operator: ri\n//g;
  $_ = $outputstr;
  if ( /^Error: \/rangecheck in --get--/ ) { 
    $do_next = 0; 
  } elsif ( /Error:/ ) {
    if ( $verbose ) { print "$outputstr\n"; }
    @tmpdirlist=();
    find(\&wanted2, $tmpdir);
    foreach $file ( @tmpdirlist ) { 
      if ( $file ne $tmpdir ) { unlink $file; }
    }
    rmdir $tmpdir;
    die "Ghostscript command failed for page $pg of $input with the following" .
      " error:\n$outputstr\n";
  } else {
    if ( $verbose ) { print "$outputstr\n"; }
    $cmdstr="$djvucommand $args " . '"' . $gsname . '" "' . $djname . 
      '" 2>&1';
    if ( $verbose ) { print "$cmdstr\n"; }
    $outputstr2=`$cmdstr`;
    $outputstr2 =~ s/Failed to read page 2\n//;
    if ( $verbose ) { print "$outputstr2\n"; }
  }
}

# The following logic gets the names of the DjVu files created for each page
# of the target PDF, and splits up the file names for intermediate bundling, 
# using the maximum allowed number of files per bundle
@filelist = ();
find(\&wanted,$tmpdir);
$lctr = 1;
$fctr = 0;
$dfilelist = "$tmpdir/files$$_" . sprintf("%04d", $lctr) . ".txt";
push @listfiles, $dfilelist;
open(OFILE, ">$dfilelist") or die "Could not open $dfilelist for writing.\n";
foreach $file ( @filelist ) { 
  ++$fctr;
  if ( $fctr <= $max_bundle ) {
    print OFILE "$file\n"; 
  } else {
    close OFILE;
    ++$lctr;
    $fctr = 1;
    $dfilelist = "$tmpdir/files$$_" . sprintf("%04d", $lctr) . ".txt";
    push @listfiles, $dfilelist;
    open(OFILE, ">$dfilelist") or die "Could not open $dfilelist for writing.\n";
    print OFILE "$file\n"; 
  }
}
close OFILE;

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
    $cmdstr="$combine -c $tmpdir/bundled.djvu --filelist=" . '"' . $dfilelist .
      '"';
    if ( $verbose ) { print "$cmdstr\n"; }
    system("$cmdstr");
    $cmdstr="$split -i $tmpdir/bundled.djvu " . '"' . $output . '"';
    if ( $verbose ) { print "$cmdstr\n"; }
    system("$cmdstr");
  } else {
    $cmdstr="$combine -c " . '"' . $output . '" --filelist="' . $dfilelist . '"';
    if ( $verbose ) { print "$cmdstr\n"; }
    system("$cmdstr");
  }
} else {
  # The logic below is the workaround for a known limitation in DjVu Enterprise
  # for Win32 (max open files).  It was done for supporting DjVu Enterprise;
  # similar logic remains to be put in place for OSI (above).
  if ( -d $output )
  {
    $combine="$joincommand";
    $output="$output/index.djvu";
  } else {
    $combine="$bundlecommand";
  }
  unlink $output;
  @ifiles=();
  # create the intermediate bundled DjVu files, and capture the intermediate
  # file names
  foreach $dfilelist (@listfiles) {
    $ifile = "$tmpdir/" . basename($dfilelist) . '.djvu';
    push @ifiles, $ifile;
    $cmdstr="$combine $cargs --filelist=" . '"' . $dfilelist . '" "' . $ifile . '"';
    if ( $verbose ) { print "$cmdstr\n"; }
    system("$cmdstr");
  }
  # if we have more than the maximum number of intermediate bundled files,
  # create multiple output files
  if ( @ifiles > $max_ifiles ) {
    $dfilelist = "";
    $ictr = 0;
    $lctr = 1;
    $ctr = 0;
    @ofiles = ();
    ($obase, $odir, $oext) = fileparse($output, "\\\..*");
    foreach $file ( @ifiles ) {
      ++$ictr;
      ++$ctr;
      if ( $ictr < $max_ifiles ) {
        $dfilelist .= '"' . $file . '" ';
      } elsif ( $ictr == $max_ifiles  ) {
        $dfilelist .= '"' . $file . '" ';
        $ofile = sprintf("%s%s%04d%s", $odir, $obase, $lctr, $oext);
	push @ofiles, $ofile;
	if ( -f $ofile ) { unlink $ofile; }
        $cmdstr="$combine $cargs $dfilelist " . '"' . $ofile . '"';
        if ( $verbose ) { print "$cmdstr\n"; }
        system("$cmdstr");
	$ictr = 0;
	++$lctr;
	$dfilelist = "";
      } 
      if ( $ctr == @ifiles ) {
        $ofile = sprintf("%s%s%04d%s", $odir, $obase, $lctr, $oext);
	push @ofiles, $ofile;
	if ( -f $ofile ) { unlink $ofile; }
        $cmdstr="$combine $cargs $dfilelist " . '"' . $ofile . '"';
        if ( $verbose ) { print "$cmdstr\n"; }
        system("$cmdstr");
      }
    }
    print "Number of pages exceeded maximum number which can be converted " .
      "into a single DjVu file on this operating system.  Multiple files " .
      "were created:\n" . join("\n", @ofiles) . "\n";
  # if we have more than 1 intermediate file, but not more than the maximum
  # allowed number of intermediate files, combine them all into a single
  # output file
  } elsif ( (@ifiles > 1) && (@ifiles <= $max_ifiles)  ) {
    $dfilelist = "";
    foreach $file (@ifiles) { $dfilelist .= '"' . $file . '" '; }
    $cmdstr="$combine $cargs $dfilelist " . '"' . $output . '"';
    if ( $verbose ) { print "$cmdstr\n"; }
    system("$cmdstr");
  # Otherwise, we only have one intermediate file; copy it to the target
  # output file
  } else {
    copy($ifiles[0], $output);
  }
}
@tmpdirlist=();
find(\&wanted2, $tmpdir);
foreach $file ( @tmpdirlist ) { 
  if ( $file ne $tmpdir ) { unlink $file; }
}
rmdir $tmpdir;

sub wanted {
    /^.*\.djvu$/ && { push @filelist, $File::Find::name };
}

sub wanted2 {
  push @tmpdirlist, $File::Find::name;
}
