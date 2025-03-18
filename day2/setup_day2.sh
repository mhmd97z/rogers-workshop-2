#!/bin/bash

git clone https://github.com/niloysh/testbed-automator.git
cd testbed-automator
./uninstall.sh
./install.sh
cd ..

git clone https://github.com/niloysh/open5gs-k8s.git
cd open5gs-k8s
./deploy-core.sh
./add-cots-subscribers.sh
cd ..

sudo ip link set n3br up
sudo ip address add 10.10.3.231/24 dev n3br

sudo sed -i 's/127.0.0.53/8.8.8.8/g' /etc/resolv.conf

sudo docker run --detach  --rm --publish 5201:5201 --name iperf-server-1 networkstatic/iperf3 -s
sudo docker run --detach  --rm --publish 5202:5201 --name iperf-server-2 networkstatic/iperf3 -s
