#!/usr/bin/perl

use warnings;

open( $com, "+<","/dev/ttyACM0" ) || die "Cannot open serial port : $!\n";
while( 1 )
{   
	
    print $com ".";  # trigger read print for 1 line
    $read = <$com>;  # line of data
    $read =~ s/^\s+//; # remove leading white space (might be better to remove this in arduino code)
    @data = split(/\s/, $read);
    $count = 1;
    open($plot, '>', './plot.dat');
    foreach $datum (@data) {
        if ( $datum eq 'a' ) {
            close($plot); # close out the plot file
            print "$count\n";
            last;
        }
        print $plot "$count $datum\n";
        $count++;
    }
}
