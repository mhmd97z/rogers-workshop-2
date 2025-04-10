#!/bin/bash

gNB="../flexric-slicing/openairinterface5g/cmake_targets/ran_build/build/nr-softmodem"
config="./configs/gnb.conf"

sudo "${gNB}" -O $config  --gNBs.[0].min_rxtxtime 6 --rfsim --sa --sa --rfsimulator.serveraddr server
