# Reads an xml description of a machine model and produces an equivalent target
# for LLVM if one does not already exist.
#
# To run manually change the path to point to the llvm source root directory,
# then pass the script the name of the xml file.
#
# LLVM then needs to be rebuilt.

#!/usr/bin/perl

use strict;
use warnings;
use v5.10;

sub read_xml($);
sub write_target_desc($%);

# !!! SET LLVM SOURCE ROOT DIRECTORY
my $source_dir = "/home/sam/src/esdg-opencl/llvm-3.2/";

my $xml_desc = $ARGV[0];
my $le1_dir = "${source_dir}lib/Target/LE1/";
my $model_dir = "${le1_dir}MachineModels/";

# check that an filename has been passed
if($xml_desc eq "") {
  say "Error, please provide filename";
  exit(-1);
}
# if so, check that the file exists
else {
  unless (-e $xml_desc) {
    say "Error, $xml_desc does not exist";
    exit(-1);
  }

  my %mm_desc = read_xml($xml_desc);

  my $target_desc = $mm_desc{issue_width}.'w'.$mm_desc{ialus}.'a'.
                    $mm_desc{imults}.'m'.$mm_desc{lsus}.'ls'.$mm_desc{brus}.'b';
  my $target_filename = "${model_dir}LE1${target_desc}.td";

  # create a new target description if one does not already exist
  unless (-e $target_filename) {
    write_target_desc($target_desc, \%mm_desc);

    # include the new target schedule
    my $main_schedule = "${le1_dir}LE1Schedule.td";
    open(SCHEDULE, ">>" . $main_schedule);# or die $!);
    say SCHEDULE "include \"MachineModels/LE1$target_desc.td\"";
    close(SCHEDULE);

    # define the new processor
    my $main_target = "${le1_dir}LE1.td";
    open(TARGET, ">>" . $main_target); #or die $!);
    say TARGET "def : Processor< \"$target_desc\", LE1${target_desc}Itineraries, []>;";
    close(TARGET);

    say "Target added. Use this configuration with -mcpu=${target_desc}\n";
  }
  else {
    say "Target description already exists, don't need to do anything";
    exit(0);
  }
}

sub read_xml($) {
  my ($xml_desc) = @_;
  say "\nReading $xml_desc";

  my %mm_desc = (
    issue_width => 0,
    ialus => 0,
    imults => 0,
    lsus => 0,
    brus => 0,
  );

  my $got_width = 0;
  my $got_alu = 0;
  my $got_mult = 0;
  my $got_lsu = 0;
  my $got_bru = 0;

  # read through the file a line at a time, looking for the tags that we're
  # interested in for the target description
  open(DESC, $xml_desc);
  while(<DESC>) {
    chop;
    my $line = $_;
    if( $line =~ /ISSUE_WIDTH_MAX/) {
      if( $line =~ />(.*?)</ ) {
        $mm_desc{issue_width} = $1;
        $got_width = 1;
      }
    }
    elsif( $line =~ /IALUS/) {
      if( $line =~ />(.*?)</ ) {
        $mm_desc{ialus} = $1;
        $got_alu = 1;
      }
    }
    elsif( $line =~ /IMULTS/) {
      if( $line =~ />(.*?)</ ) {
        $mm_desc{imults} = $1;
        $got_mult = 1;
      }
    }
    elsif( $line =~ /LSU_CHANNELS/) {
      if( $line =~ />(.*?)</ ) {
        $mm_desc{lsus} = $1;
        $got_lsu = 1;
      }
    }
    elsif( $line =~ /BRUS/) {
      if( $line =~ />(.*?)</ ) {
        ($mm_desc{brus}) = $1;
        $got_bru = 1;
      }
    }
  }

  close(DESC);

  if( $got_width && $got_alu && $got_mult && $got_lsu && $got_bru) {
    say "IssueWidth = $mm_desc{issue_width}";
    say "IALUS = $mm_desc{ialus}";
    say "IMULTS = $mm_desc{imults}";
    say "LSU_CHANNELS = $mm_desc{lsus}";
    say "BRUS = $mm_desc{brus}";
    return %mm_desc;
  }
  else {
    die "Error, something went wrong parsing the model";
  }

}

sub write_target_desc ($%) {
  my $target_desc = $_[0];
  my $target_filename = "${model_dir}LE1${target_desc}.td";
  my (%mm) = %{$_[1]};

  # create functional unit name strings
  my @ISSS;
  my @ALUS;
  my @MULS;
  my @LSUS;
  my @BRUS;
  for(my $ISS = 0; $ISS < $mm{issue_width}; ++$ISS) {
    $ISSS[$ISS] = "ISS_${ISS}_${target_desc}";
  }
  for(my $ALU = 0; $ALU < $mm{ialus}; ++$ALU) {
    $ALUS[$ALU] = "ALU_${ALU}_${target_desc}";
  }
  for(my $MUL = 0; $MUL < $mm{imults}; ++$MUL) {
    $MULS[$MUL] = "MUL_${MUL}_${target_desc}";
  }
  for(my $LSU = 0; $LSU < $mm{lsus}; ++$LSU) {
    $LSUS[$LSU] = "LSU_${LSU}_${target_desc}";
  }
  for(my $BRU = 0; $BRU < $mm{brus}; ++$BRU) {
    $BRUS[$BRU] = "BRU_${BRU}_${target_desc}";
  }

  open(TARGET, ">" . $target_filename);
  say TARGET "//===---------------------------------------------------------------------===//";
  say TARGET "//  Instruction Itinerary Class for a $mm{issue_width} wide LE1 with:";
  say TARGET "//  $mm{ialus} ALU(s), $mm{imults} MUL(s), $mm{lsus} LSU(s) and $mm{brus} BRU(s)";
  say TARGET "//===---------------------------------------------------------------------===//";

  foreach(@ISSS) {
    say TARGET "def $_    : FuncUnit;";
  }
  foreach(@ALUS) {
    say TARGET "def $_    : FuncUnit;";
  }
  foreach(@MULS) {
    say TARGET "def $_    : FuncUnit;";
  }
  foreach(@LSUS) {
    say TARGET "def $_    : FuncUnit;";
  }
  foreach(@BRUS) {
    say TARGET "def $_    : FuncUnit;";
  }

  print TARGET "\n";
  say TARGET "def LE1${target_desc}Itineraries : ProcessorItineraries< [";
  print TARGET "    ";
  foreach(@ISSS) {
    print TARGET "$_, ";
  }
  print TARGET "\n    ";
  foreach(@ALUS) {
    print TARGET "$_, ";
  }
  print TARGET "\n    ";
  foreach(@MULS) {
    print TARGET "$_, ";
  }
  print TARGET "\n    ";
  foreach(@LSUS) {
    print TARGET "$_, ";
  }
  print TARGET "\n    ";
  # TODO only expects one branch unit 
  foreach(@BRUS) {
    print TARGET "$_ ";
  }

  say TARGET "], [/*ByPass*/], [\n";

  # ALU Itineraries
  say TARGET "    InstrItinData<IIAlu,  [InstrStage<1,";
  print TARGET "    [";
  for my $i (0 .. $#ISSS) {
    if($i < $#ALUS) {
      print TARGET "$ISSS[$i], ";
    }
    else {
      print TARGET "$ISSS[$i]";
    }
  }
  say TARGET "], 0>,\n";

  say TARGET "                           InstrStage<1,";
  print TARGET "    [";
  for my $i (0 .. $#ALUS) {
    if($i < $#ALUS) {
      print TARGET "$ALUS[$i], ";
    }
    else {
      print TARGET "$ALUS[$i]";
    }
  }
  say TARGET "]>], [2, 1]>,\n";

  # Multiplier Itineraries
  say TARGET "    InstrItinData<IIMul,  [InstrStage<1,";
  print TARGET "    [";
  for my $i (0 .. $#ISSS) {
    if($i < $#ALUS) {
      print TARGET "$ISSS[$i], ";
    }
    else {
      print TARGET "$ISSS[$i]";
    }
  }
  say TARGET "], 0>,\n";
  say TARGET "                           InstrStage<1,";
  print TARGET "    [";
  for my $i (0 .. $#MULS) {
    if($i < $#MULS) {
      print TARGET "$MULS[$i], ";
    }
    else {
      print TARGET "$MULS[$i]";
    }
  }
  say TARGET "]>], [2, 1]>,\n";

  # LSU Itineraries
  say TARGET "    InstrItinData<IILoadStore,  [InstrStage<1,";
  print TARGET "    [";
  for my $i (0 .. $#ISSS) {
    if($i < $#ALUS) {
      print TARGET "$ISSS[$i], ";
    }
    else {
      print TARGET "$ISSS[$i]";
    }
  }
  say TARGET "], 0>,\n";
  say TARGET "                                 InstrStage<1,";
  print TARGET "    [";
  for my $i (0 .. $#LSUS) {
    if($i < $#LSUS) {
      print TARGET "$LSUS[$i], ";
    }
    else {
      print TARGET "$LSUS[$i]";
    }
  }
  say TARGET "]>], [2, 1]>,\n";

  # Branch Itineraries
  say TARGET "    InstrItinData<IIBranch,  [InstrStage<1,";
  print TARGET "    [";
  for my $i (0 .. $#ISSS) {
    if($i < $#ALUS) {
      print TARGET "$ISSS[$i], ";
    }
    else {
      print TARGET "$ISSS[$i]";
    }
  }
  say TARGET "], 0>,\n";
  say TARGET "                              InstrStage<6,";
  print TARGET "    [";
  for my $i (0 .. $#BRUS) {
    if($i < $#BRUS) {
      print TARGET "$BRUS[$i], ";
    }
    else {
      print TARGET "$BRUS[$i]";
    }
  }
  say TARGET "]>], [6, 1]> ]>;\n";

  say TARGET "def LE1Model${target_desc} : SchedMachineModel {";
  say TARGET "  let IssueWidth = $mm{issue_width};";
  say TARGET "  let Itineraries = LE1${target_desc}Itineraries;";
  say TARGET "}";

  close(TARGET);
}
