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
  }
  if($last =~ /<IMG ALT="o" BORDER=0 SRC=icon1[.]gif><A NAME=".*"><.A>/i 
    && $line =~ /(<A HREF=)(.*[.]html)(><B>)(.*)(<.B><.A>)(<DD>.*)/i)
  {
    ($current1,$current2,$current3,$current4,$current5,$current6)=($1,$2,$3,$4,$5,$6);
    if(open(FILE,"<$2"))
    {
      while( <FILE> )
      {
        if($_ =~ /(<H2>[ ]*)([^(]*)(<A HREF="[^"]+">)([^(]+)(<\/A>)(.*)(<\/H2>.*)$/i)
        {
          $line="<B>$2</B>$current1$current2$current3$current4$current5<BR>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<B>$6</B>$current6\n";
          break;
        }
      }
      close FILE;
    }
  }else
  {
    $line =~ s,^<body>,<BODY BGCOLOR=\#fefefe BACKGROUND=back.jpg>,i;
  }
  print OUT $line;
  $last=$line;
}
close OUT;
unlink("$filename.save");

