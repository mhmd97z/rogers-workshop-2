#!/bin/bash
cd /home/user/workshop/oai_ran/cmake_targets/ran_build/build
sudo ./nr-softmodem -O ../../../targets/PROJECTS/GENERIC-NR-5GC/CONF/gnb.sa.band78.fr1.106prb.usrp210.modified.conf  --gNBs.[0].min_rxtxtime 6 --rfsim --sa
