#! /usr/bin/env perl
use Net::FTP;
$hostname = "ftp.geo.ku.dk";
$dir = "FULL/LST";
$user = "msguser";
$password = "xxxxx";
$dstdir="/home/ikh/ftp/MSG-SVGIRI/data";
$rexd = qr/^[0-9]+$/;
$rexf = qr/^.*\.h5$/;

print "Connecting to host '$hostname', user '$user', dir '$dir', dstdir '$dstdir', rexd '$rexd', rexf '$rexf' :\n";
$ftp = Net::FTP->new("$hostname", Debug => 0) or die "Cannot connect to '$hostname': $@";
$ftp->login($user,$password) or die "Cannot login to '$hostname' as '$user'", $ftp->message;
$ftp->binary() or die "Cannot switch to binary mode ", $ftp->message;
$ftp->cwd("$dir") or die "Cannot change working directory to '$dir'", $ftp->message;
@xs = $ftp->ls("") or die "Cannot obtain directory listing for '$dir'";
foreach $x (@xs) {
   if ($x =~ $rexd) {
      print "Processing subdirectory '$dir/$x':\n";
      $ftp->cwd($x) or die "Cannot change working directory to '$x'", $ftp->message;
      @ys = $ftp->ls("") or die "Cannot obtain directory listing for '$dir/$x'";
      system("mkdir -p \"$dstdir/$x\"");
      foreach $y (@ys) {
         $fn = "$dir/$x/$y";
         $dfn = "$dstdir/$x/$y";
         $zfn = $dfn; $zfn =~ s/.(gz|Z)$//;

         print "Processing file '$fn' -> '$dfn' ...\n";
         $mdtm = $ftp->mdtm($y);
         $size = $ftp->size($y);

         if (-e $dfn) {
            $zflag = 0;
            $rfn = $dfn;
            @st = stat($dfn) or die "Cannot stat '$dfn'";
         } elsif (-e $zfn) {
            $zflag = 1;
            $rfn = $zfn;
            @st = stat($zfn) or die "Cannot stat '$zfn'";
         } else {
            $zflag = 0;
            $rfn = "";
            @st = (0,0,0,0,0,0,0,0,0,0,0,0,0);
         }
         $dsize = $st[7];
         $dmdtm = $st[9];

         print "sfile '$fn', size '$size', mdtm '$mdtm'\n";
         print "dfile '$rfn', dsize '$dsize', dmdtm '$dmdtm', zflag '$zflag'\n";

         if ($y =~ $rexf) {
            if (!defined($size) || !defined($mdtm) || $mdtm > $dmdtm || $size > $dsize) { # assumption size of uncompressed file >= size of compressed file 
               $ftp->get($y,$dfn) or die "get of '$fn' -> '$dfn' failed ", $ftp->message;
               print "got $fn\n"
            } else {
               print "exists $rfn\n";
            }
         } else {
            print "skipped $y $fn\n";
         }
         print "\n";
      }
      $ftp->cwd("..") or die "Cannot change working directory '..'", $ftp->message;
   } else {
      print "Skipped subdirectory '$dir/$x'\n";
   }
}
$ftp->quit;
print "Done!\n";

