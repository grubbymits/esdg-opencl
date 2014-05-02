#!/usr/bin/perl

if(!$ARGV[0] || !$ARGV[1]) {
    print "You haven't specified any input!\n";
    exit(0);
}

$inFile = $ARGV[0];
if(!-e($inFile)) {
    print "Input file does not exist: $!\n";
    exit(0);
}
$outFile = $ARGV[1];

# string to find in Binary
my $revString = 'Insizzle_Revision';
# revision number of current git repo
my $revNumber = readpipe('git rev-parse --short HEAD');
chomp($revNumber);
# pad with '\0' to make same length
for($i=length($revNumber);$i<length($revString);$i++) {
    $revNumber .= chr('\0');
}

# get Binary into buffer
open FILE, $inFile or die $!;
binmode FILE;
my ($buf, $data, $n);
while (($n = read FILE, $data, 1) != 0) {
    $buf .= $data;
}
close(FILE);

# replace $revString in binary with new $revNumber
$buf =~ s/$revString/$revNumber/;
open FILE, "> $outFile" or die $!;
print FILE $buf;
close FILE;

# chmod output file
chmod 0755, "$outFile";
