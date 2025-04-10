#!/bin/bash

cleanup() {
    echo $PID1
    sudo kill $PID1
    # sudo kill $PID2
    exit 0
}

trap cleanup SIGINT

cd /home/user/workshop/oai_ran/cmake_targets/ran_build/build
sudo -E ./nr-uesoftmodem -r 106 --numerology 1 --band 78 -C 3619200000 --rfsim --sa -O ../../../targets/PROJECTS/GENERIC-NR-5GC/CONF/ue.conf &
PID1=$!
sleep 5
ping -I oaitun_ue1 -i 0.01 8.8.8.8 > ping.output
# PID2=$!
# while true; do
#     sleep 1
# done
wait $PID1 
