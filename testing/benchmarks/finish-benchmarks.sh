#!/bin/sh
cd AMD
cd BinarySearch
make
./run > output
cd ..
cd BitonicSort
make
./run > output
cd ..
cd RadixSort
make
./run > output
cd ..
cd Reduction
make
./run > output
cd ..
cd MatrixTranspose
make
./run > output
cd ..
cd NBody
make
./run > output
cd ..
cd FloydWarshall
make
./run > output
cd ..
cd FastWalshTransform
cd RadixSort
make
./run > output
cd ..
cd NBody
make clean
make
./run > output
cd ../..

