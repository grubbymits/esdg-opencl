#!/bin/sh
for device in `seq 0 4`;
do
  ./bfs ../data/bfs/graph4096.txt $device
done
