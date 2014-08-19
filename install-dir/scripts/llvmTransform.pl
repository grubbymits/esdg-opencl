#!/usr/bin/perl

# transform the LLVM assembly into somthing I want

use strict;

########
# Operations - x
# Imports (should be none)
# Data Labels
# Data Section - x - Data_align=32
# BSS Labels - x - Bss_align=32

my $inFile = '';
my %supportedSYSCALLS;

foreach my $arg (@ARGV) {
    if($arg =~ /-syscall=(.+)/) {
	&populateSyscalls($1);
    }
    else {
	$inFile = $arg;
    }
}

open INFILE, "< $inFile" or die;
my @inFile = <INFILE>;
close INFILE;

$inFile =~ /(\w+)\.(\w+)/;
my $filename = $1;
my $instCount = 0;
my (@oper, %instLabels);
my $i;

$instLabels{'exit'} = 'FUNC_exit'; # not the best way to get round this :S

for($i=0;$i<=$#inFile;$i++) {
    if($inFile[$i] =~ /\@object/) {
	last;
    }
    else {
	chomp($inFile[$i]);

	my ($inst, $comment) = split(/#/, $inFile[$i]);

	# remove leading and trailing whitespace
	$inst =~ s/^\s+//g;
	$inst =~ s/\s+$//g;

	if($inst =~ /^\./) { } # ignore any . instructions
        elsif ($inst =~ /^[^\$].+:$/) { # function labels
	    #push @oper, '-- FUNC_' . $1;
            #$instLabels{$1} = 'FUNC_' . $1;
            chop($inst);
            #print "pushing " . $inst . "\n";
            my $formatted_label = $inst;
            $formatted_label =~ s/\./_/g;
	    push @oper, '-- FUNC_' . $formatted_label;
	    $instLabels{$inst} = 'FUNC_' . $formatted_label; # keep track of functions
	}
	elsif($inst =~ /^\$BB(\d+)_(\d+):/) { # basic block labels
	    push @oper, '-- ' . $filename . '_L' . $1 . '?' . $2;
	    $instLabels{'\$BB' . $1 . '_' . $2} = $filename . '_L' . $1 . '?' . $2;
	}
	elsif($inst =~ /^;/) { # clock
	    push @oper, ';';
	}
	elsif($inst =~ /^\w+\./) { # operations
	    push @oper, $inst;
	    $instCount++;
	}
	elsif($inst =~ /^$/) { } # empty lines
	elsif($inst =~ /^\$tmp/) { } # unused instruction labels
	else {
	    print "UNKNOWN: $inst\n";
	}
    }
}

# perform some modifications
for(my $j=0;$j<=$#oper;$j++) {

    # replace labels in instructions
    if($oper[$j] !~ /^--/) {
      #print "Replace labels\n";
	# TODO: the section below is not fully tested
	# the transform now take 1.6% of the time as using the code in the if(0) section
	if(0) {
	    while(my ($key, $value) = each(%instLabels)) {
		# TODO: this may need to be altered
		$oper[$j] =~ s/(\W+)\(?$key\)?\s*$/$1$value\)/g;
	    }
	}
	else {
	    $oper[$j] =~ s/\$BB(\d+)_(\d+)/$filename\_L$1\?$2/g;
	    my @o = split(/\s+/, $oper[$j]);
	    for(my $i=0;$i<@o;$i++) {
		if ($o[$i] =~ /^([a-zA-Z_].+)$/) {
                #if ($o[$i] =~ /^[^\$].+:$/)
		    # check if it is in %instLabels
                    #print "instLables = " . $1 . " = " . $instLabels{$1} . "\n";
		    if(defined($instLabels{$1})) {
			$o[$i] = $instLabels{$1};
		    }
		}
	    }
	    $oper[$j] = join(' ', @o);
	}
    }
    # rename multiply instructions
    $oper[$j] =~ s/^mul/mpy/;
    # insert some extra exit information
    if($oper[$j] =~ /call\.0\s+l\d+\.\d+,\s+FUNC_exit/) {
	$oper[$j] = '.call exit, caller, arg($r0.3:s32), ret($r0.3:s32)' . "\n" . $oper[$j];
    }
    # replace syscall operations
    if($oper[$j] =~ /call\.(\d+)\s+l\d+\.\d+,\s+(.+)/) {
	if(defined($supportedSYSCALLS{$2})) {
	    $oper[$j] = 'syscall.' . $1 . ' ' . $supportedSYSCALLS{$2};
	}
    }
}

# check if main is the first function
# if not, move to the top
my $mainStart = -1;
my $mainEnd = 0;
for(my $j=0;$j<=$#oper;$j++) {
    if($mainStart != -1) {
	# in main
	do {
	    $mainEnd = ++$j;
	    if($oper[$j] =~ /-- FUNC/) {
		$j--;
		last;
	    }
	} while($j < $#oper);
    }
    if($oper[$j] =~ /^-- FUNC_main$/) {
	$mainStart = $j;
    }
}

# need to move main to the top of the file
if($mainStart != 0) {
    # remove main function
    my @mainFunc = splice(@oper, $mainStart, ($mainEnd - $mainStart));
    # put in at top of operations array
    splice(@oper, 0, 0, @mainFunc);
}

my $comm = 0;
my @data;
my $currentLine = 0;
my $cleanLine = 1;
my $currentAddr = 0;
my %lookup;

for($i=$i-1;$i<=$#inFile;$i++) {
    if($inFile[$i] =~ /\.type\s+((\$?\.?\w+)+),\@object/) {
	my $varname = $1;
	&alignMem();
	$lookup{$varname} = $currentAddr;
	my $found = 0;
	my $size = 0;
	my $width = 0;

	# escape $ in varname
	$varname =~ s/\$/\\\$/g;

	# figure out size and width of data item
	my $i_copy = $i;
	my $break = 0;
	my $i_start = 0;
	do {
	    $i++;
	    if($inFile[$i] =~ /$varname:/) {
		$i_start = $i;
	    }
	    if($inFile[$i] =~ /\.size\s+($varname),\s*(\d+)/) {
		$size = $2;
		$width = $size / ($i - ($i_start + 1));
		$break = 1;
	    }
	} while(($inFile[$i] !~ /^\n/) && ($i <= $#inFile) && (!$break));
	$i = $i_copy;

	do {
	    $i++;
	    if($inFile[$i] =~ /\.comm\s+($varname),(\d+),(\d+)/) {
		&pushData($varname, 0x0, $2, "COMM");
		$found = 1;
	    }
	    elsif($inFile[$i] =~ /\.word\s+(\d+|.?\w+)/) {
		&pushData($varname, $1, 0x0, "WORD");
	    }
	    elsif($inFile[$i] =~ /\.half\s+(\d+)/) {
		&pushData($varname, $1, 0x0, "HALF");
	    }
	    elsif($inFile[$i] =~ /\.byte\s+(\d+)/) {
		&pushData($varname, $1, 0x0, "BYTE");
	    }
	    elsif($inFile[$i] =~ /\.2byte\s+(\d+)/) {
		&pushData($varname, $1, 0x0, "HALF");
	    }
	    elsif($inFile[$i] =~ /\.4byte\s+(\d+)/) {
		&pushData($varname, $1, 0x0, "WORD");
	    }
	    elsif($inFile[$i] =~ /\.4byte\s+(\w+)/) {
		&pushData($varname, $1, 0x0, "WORD");
	    }
	    elsif($inFile[$i] =~ /\.asci[iz]\s+\"(\w+)\"/) {
		&pushData($varname, $1, 0x0, "STRING");
	    }
	    elsif($inFile[$i] =~ /\.asci[iz]\s+\"(.+)\"/) {
		my @data = ascii_to_dec($1);
		my $c;
		for($c=0;$c<@data;$c++) {
		    &pushData($varname, $data[$c], 0x0, "BYTE");
		}
		while($c < $width) {
		    &pushData($varname, 0x0, 0x0, "BYTE");
		    $c++;
		}
	    }
	    elsif($inFile[$i] =~ /\.space\s+(\d+),?(\d+)?/) {
		#$space = $1;
		# space can have a value allocated to it
		my $fill = (!$2) ? 0x0 : $2;
		for(my $s=0;$s<$1;$s++) {
		    &pushData($varname, $fill, 0x0, "BYTE");
		}
		# check its pushed
		#if(!($currentAddr % 4) && $currentAddr && !$cleanLine) {
		#    push @data, $currentLine;
		#    $currentLine = 0;
		#    $cleanLine = 1;
		#}
		#$found = 1;
	    }
	    elsif($inFile[$i] =~ /\.size\s+($varname),\s*(\d+)/) {
		# should be the end of data item
		$found = 1;
	    }
	} while(($inFile[$i] !~ /^\n/) && ($i <= $#inFile) && ($found == 0));

	#&alignMem();
    }
}

# Operations
print "## Operations - $instCount\n";
for(my $i=0;$i<=$#oper;$i++) {
    print $oper[$i] . "\n";
}
print "\n";

exit(0);

sub pushData {
    my $name = $_[0];
    my $data = $_[1];
    my $size = $_[2];
    my $type = $_[3];

    if($type eq "COMM") {
	# pad to word boundary
	if($currentAddr % 4) {
	    while($currentAddr % 4) {
		$currentLine |= 0x00 << ((~($currentAddr % 4) & 0x3) * 8);
		$cleanLine = 0;
		$currentAddr++;
	    }
	    push @data, $currentLine;
	    $currentLine = 0;
	    $cleanLine = 1;
	}

	#$lookup{$name} = $currentAddr;

	for(my $j=0;$j<$size;$j++) {
	    $currentLine |= 0x00 << ((~($currentAddr % 4) & 0x3) * 8);
	    $cleanLine = 0;
	    $currentAddr++;
	    if(!($currentAddr % 4)) {
		push @data, $currentLine;
		$currentLine = 0;
		$cleanLine = 1;
	    }
	}
    }
    elsif($type eq "WORD") {
	# pad to word boundary
	if($currentAddr % 4) {
	    while($currentAddr % 4) {
		$currentLine |= 0x00 << ((~($currentAddr % 4) & 0x3) * 8);
		$cleanLine = 0;
		$currentAddr++;
	    }
	    push @data, $currentLine;
	    $currentLine = 0;
	    $cleanLine = 1;
	}
	#if(!defined($lookup{$name})) {
	#    $lookup{$name} = $currentAddr;
	#}

	if($data !~ /^\d+$/) {
	    push @data, "LABEL: FUNC_" . $1;
	}
	else {
	    push @data, $data;
	}
	$currentAddr += 4;
    }
    elsif($type eq "HALF") {
	# need to push any full words to data
	if(!($currentAddr % 4) && $currentAddr) {
	    push @data, $currentLine;
	    $currentLine = 0;
	    $cleanLine = 1;
	}

	# possibility of padding
	if(($currentAddr % 2) && $currentAddr) {
	    $currentLine |= 0x00 << ((~($currentAddr % 4) & 0x3) * 8);
	    $cleanLine = 0;
	    $currentAddr++;
	}

	#if(!defined($lookup{$name})) {
	#    $lookup{$name} = $currentAddr;
	#}

	$currentLine |= $data << (((~$currentAddr-1) % 4) * 8);
	$cleanLine = 0;
	$currentAddr += 2;
    }
    elsif($type eq "BYTE") {
	# need to push any full words to data
	if(!($currentAddr % 4) && $currentAddr && !$cleanLine) {
	    push @data, $currentLine;
	    $currentLine = 0;
	    $cleanLine = 1;
	}

	#if(!defined($lookup{$name})) {
	#    $lookup{$name} = $currentAddr;
	#}

	$currentLine |= $data << ((~($currentAddr % 4) & 0x3) * 8);
	$cleanLine = 0;
	$currentAddr++;
    }
    elsif($type eq "SPACE") {
	# need to push any full words to data
	if(!($currentAddr % 4) && $currentAddr) {
	    push @data, $currentLine;
	    $currentLine = 0;
	    $cleanLine = 1;
	}

	for(my $k=0;$k<$size;$k++) {
	    $currentLine |= 0x00 << ((~($currentAddr % 4) & 0x3) * 8);
	    $cleanLine = 0;
	    $currentAddr++;

	    if(!($currentAddr % 4) && $currentAddr) {
		push @data, $currentLine;
		$currentLine = 0;
		$cleanLine = 1;
	    }
	}
    }
    elsif($type eq "STRING") {
	#if(!defined($lookup{$name})) {
	#    $lookup{$name} = $currentAddr;
	#}
	# need to replace \x with char equivalents
	$data =~ s/\\n/\n/g;

	my @string = split(//, $data);
	for(my $k=0;$k<=$#string;$k++) {
	    $currentLine |= ord($string[$k]) << ((~($k % 4) & 0x3) * 8);
	    $cleanLine = 0;
	    $currentAddr++;
	    if(!($currentAddr % 4) && $currentAddr) {
		push @data, $currentLine;
		$currentLine = 0;
		$cleanLine = 1;
	    }
	}
    }
}

sub alignMem {
# make sure all data is output
    if($currentAddr % 4) {
	while($currentAddr % 4) {
	    $currentLine |= 0x00 << ((~($currentAddr % 4) & 0x3) * 8);
	    $cleanLine = 0;
	    $currentAddr++;
	}
	push @data, $currentLine;
	$currentLine = 0;
	$cleanLine = 1;
    }
    else {
	if(!$cleanLine) {
	    push @data, $currentLine;
	    $currentLine = 0;
	    $cleanLine = 1;
	}
    }
}

sub populateSyscalls {
    open SYSCALL, "< $_[0]" or die "Could not open file: $_[0] ($!)\n";
    while(<SYSCALL>) {
	chomp();
	if(/#define\s+(\w+)\s+(\d+)/) {
	    $supportedSYSCALLS{lc($1)} = $2;
	}
    }
    close SYSCALL;
}

sub sortLookUp() {
    $lookup{$a} <=> $lookup{$b};
}

sub ascii_to_dec {
    my @data;
    my @tmp = split(//, $_[0]);
    for(my $i=0;$i<@tmp;$i++){
        if($tmp[$i] eq "\\") {
            $i++;
            if($tmp[$i] =~ /\d/) {
                my $spec =  $tmp[$i] . $tmp[$i+1] . $tmp[$i+2];
                push (@data, oct($spec));
                $i += 2;
            }
            elsif($tmp[$i] =~ /\w/) {
                my $spec = "\\" . $tmp[$i];
                $spec =~ s/\\a/\a/g;
                $spec =~ s/\\b/\b/g;
                $spec =~ s/\\n/\n/g;
                $spec =~ s/\\t/\t/g;
                $spec =~ s/\\r/\r/g;
                $spec =~ s/\\f/\f/g;
                push (@data, ord($spec));
            }
            elsif($tmp[$i] eq "\\") {
                push (@data, ord("\\"));
            }
	    elsif($tmp[$i] eq "\"") {
                push (@data, ord("\""));
	    }
            else {
		print $tmp[$i] . "\n";
                print "ERROR\n";
            }
        }
        else {
            push (@data, ord($tmp[$i]));
        }
    }
    return @data;
}
