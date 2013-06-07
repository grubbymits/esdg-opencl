#!/bin/sh
  testdir="$( cd "$(dirname "$0")" && pwd )"
  cd ~/Dropbox/src/LE1/Assembler
  perl generate.pl $testdir -d -k -syscall -llvm
  cd $testdir
