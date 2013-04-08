#!/usr/bin/perl
#finds lines with non-ascii characters 
my $file = $ARGV[0];
open(FILE, "<$file") || die "Unable to open $file <$!>\n";
my($content, $length, $t, $line);
$line = 0;
while( chomp($content = <FILE>) ) {
    $t = 0;
    $line ++;
    $length = length($content);
    for( $i = 0; $i < $length; $i++ ) {
        my $c = ord(substr($content, $i, 1));

        if($c >= 127) {
            #print "$content\n";
            $t = 1;
            last;
        }        
    }
    if( $t ) {
       #print $file, ":", $line, " [", $content, "]", "\n";
       print $file, ":", $line, "\n";
    }
}
close(FILE);
exit 0

