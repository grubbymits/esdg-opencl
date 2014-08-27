#!/bin/sh
cd bfs
make clean
make
./run > output
cd ..
cd gaussian
make clean
make
./run > output
cd ..
cd nw
make clean
make
./run > output
cd ..
cd nn
make clean
make
./run > output
cd ..

