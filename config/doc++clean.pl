#!/usr/bin/perl

$last="";
$filename="";
open(OUT,">>/dev/null");
while ( <> )
{
  $line=$_;
  if($ARGV ne $filename)
  {
    unlink("$filename.save");
    close OUT;
    $filename=$ARGV;
    unlink("$filename");
    rename("$filename","$filename.save");
    if(!open(OUT,">$filename"))
    {
      print STDERR "Failed to open $filename\n";
      exit 1;
    }
    unshift @filelist,$filename;
  }
  if($last =~ /^(<IMG ALT="o" BORDER=0 SRC=icon1[.]gif><A NAME=")(.*)("><.A>)/i) 
  {
    $anchor=$2;
    if($line =~ /(<A HREF=)(.*[.]html)(><B>)(.*)(<.B><.A>)(<DD>.*)/i)
    {
      ($current1,$current2,$current3,$current4,$current5,$current6)=($1,$2,$3,$4,$5,$6);
      $oldlink="$current2";
      $newlink="$filename#$anchor";
      $newlink=~s,^.*[/],,g;
      $file="$filename/$current2";
      $file=~s,[^/]*/$current2,$current2,g;
      if(open(FILE,"<$file"))
      {
        $block="";
        $newblock="";
        @file = <FILE>;
        $dobreak=0;
        while ( @file && !$dobreak)
        {
          $in=shift @file;
          if ($in =~ /(<H2>[ ]*)([^<]*)(<A HREF="[^"]+">)([^<]+)(<\/A>)([^\(]*[^\)]*)(<\/H2>.*)$/i)
          {
            $line="<B>$2</B>$current1$current2$current3$current4$current5<B>$6</B>$current6\n";
            $altline="<B>$2</B><B><EM><FONT COLOR=#000088>$current4</FONT></EM></B><B>$6</B>$current6\n";
            $dobreak=1;
          }elsif($in =~ /(<H2>[ ]*)([^(]*)(<A HREF="[^"]+">)([^(]+)(<\/A>)(.*)(<\/H2>.*)$/i)
          {
            $line="<B>$2</B>$current1$current2$current3$current4$current5<BR>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<B>$6</B>$current6\n";
            $altline="<B>$2</B><B><EM><FONT COLOR=#000088>$current4</FONT></EM></B><BR>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<B>$6</B>$current6\n";
            $dobreak=1;
          }elsif ($in =~ /^<H2>/)
          {
            $dobreak=1;
          }
        }
        $dobreak=0;
        while ( @file && ! $dobreak)
        {
          $in=shift @file;
          if($in =~ /^(<BLOCKQUOTE>)([+][+][ ]+.*)(<\/BLOCKQUOTE>)/)
          {
            $block=$2;
            $dobreak=1;
          }
        }
        if($block ne "")
        {
          $dobreak=0;
          while ( @file && ! $dobreak)
          {
            $in=shift @file;
            if($in =~ /^(<BLOCKQUOTE>[+][+][ ]+)(.*)(<\/BLOCKQUOTE>)/)
            {
              $newblock=$2;
              $dobreak=1;
            }elsif($in =~ /^(<BLOCKQUOTE>[+][+][ ]+)(.*)$/)
            {
              $newblock=$2."\n";
              while (@file &&($newblock !~ /<\/BLOCKQUOTE>/))
              {
                $newblock.=shift @file;
              }
              $newblock =~ s,<\/BLOCKQUOTE>$,,;
              $dobreak=1;
            }
          }
        }
        close FILE;
        if($newblock ne "" )
        {
          $altline =~ s,\Q$block\E,$newblock,;
          $line=$altline;
          $linklist{$oldlink}=$newlink;
          unshift @removelist,$file;
        }
      }
    }else
    {
      $line =~ s,<BODY BGCOLOR=".ffffff">,<BODY BGCOLOR=\#fefefe BACKGROUND=back.jpg>,i;
      $line =~ s,^<body>,<BODY BGCOLOR=\#fefefe BACKGROUND=back.jpg>,i;
    }
  }else
  {
    $line =~ s,<BODY BGCOLOR=".ffffff">,<BODY BGCOLOR=\#fefefe BACKGROUND=back.jpg>,i;
    $line =~ s,^<body>,<BODY BGCOLOR=\#fefefe BACKGROUND=back.jpg>,i;
  }
  print OUT $line;
  $last=$line;
}
close OUT;
unlink("$filename.save");
if(@removelist || @filelist)
{
  if(@removelist)
  {
    foreach $file ( @removelist )
    {
      unlink $file;
    }
  }
  foreach $file ( @filelist )
  {
    if(rename("$file","$file.save"))
    {
      if(open(OUT,">$file")&&open(FILE,"<$file.save"))
      {
        while(<FILE>)
        {
          $line=$_;
          foreach $oldlink ( keys %linklist )
          {
            $line=~s,HREF=(["]*)\Q$oldlink\E(["]*),HREF="$linklist{$oldlink}",g;
          }
# We only detect one broken link per line...
          if($line =~ /^(.*)(HREF=")([^"]*)(#[^"]*)(".*)$/i)
          {
            ($c1,$c2,$c3,$c4,$c5)=($1,$2,$3,$4,$5);
            if( $c3 ne "" && $c3 !~ /\//)
            {
              $in="$file/$c3";
              $in=~ s,[^/]+/([^/]*)$,\1,g;
              if(open(TEST,"<$in"))
              {
                close TEST;
              }else
              {
                 print STDERR "Removing broken link in $file line $line";
                $line = "$c1$c5";
              }
            }
          } elsif($line =~ /^(.*)(HREF=")([^"]+)(".*)$/i)
          {
            ($c1,$c2,$c3,$c4)=($1,$2,$3,$4);
            if( $c3 !~ /\//)
            {
              $in="$file/$c3";
              $in=~ s,[^/]+/([^/]*)$,\1,g;
              if(open(TEST,"<$in"))
              {
                close TEST;
              }else
              {
                print STDERR "Broken link in $file line $line";
                $line = "$c1$c4";
              }
            }
          }
          print OUT $line;
        }
        close FILE;
        close OUT;
        unlink("$file.save");
      }else
      {
        rename("$file.save","$file");
      }
    }
  }
}

