#!/usr/bin/perl

# auto run Makefile to generate binaries
$CFLAGS = "-c -Wall -pedantic -std=c99 -Wextra -m32 -g `xml2-config --cflags` ";
$INSIZZLE_BINARIES_DIR = "../InsizzleBinaries";

@options = ("PRINTOUT", "INSDEBUG", "VTHREAD");

for($i=0;$i<@options;$i++) {
    @ar = @empty;
    push @ar, $options[$i];

    if(($i + 1) < @options) {
	for($j=($i+1);$j<@options;$j++) {
	    &make(@ar);
	    push @ar, $options[$j];
	}
	&make(@ar);
    }
    else {
	&make(@ar);
    }
}

exit(0);

sub make() {
    $opt = '-D';
    for($c=0;$c<@_;$c++) {
	$opt .= $_[$c];
	if($c < $#_) {
	    $opt .= " -D";
	}
    }
    print $CFLAGS, $opt, "\n";

    system("make CFLAGS=\"$CFLAGS $opt\"");

    $opt =~ s/\s*-D/_/g;
    print "\tINSIZZLE", $opt, "\n";
    system("mv INSIZZLE $INSIZZLE_BINARIES_DIR/INSIZZLE_$opt");
    system("make distclean");
}
