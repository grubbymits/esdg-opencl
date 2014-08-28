#!/bin/sh
cd rodinia
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
cd ../..
cd AMD
cd FastWalshTransform
make clean
make
./run > output
cd ..
cd RadixSort
make clean
make
./run > output
cd ..
cd NBody
make clean
make
./run > output
cd ../..
