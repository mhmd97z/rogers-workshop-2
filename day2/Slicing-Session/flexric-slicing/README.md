# flexric-slicing


# Install FlexRIC
## Requirments
```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/ppa
sudo apt update
sudo apt install g++-10
sudo apt install cmake
sudo apt-get install autotools-dev
sudo apt-get install automake
sudo apt-get install bison -y
sudo apt-get install byacc -y
sudo apt install libsctp-dev python3.8 cmake-curses-gui libpcre2-dev python3-dev libcjson-dev


## if LD_LIBRARY_PATH is not configured properly
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
```

# Change g++ version
```bash
sudo update-alternatives --remove-all gcc
sudo update-alternatives --remove-all g++
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 \
                         --slave /usr/bin/g++ g++ /usr/bin/g++-10
sudo update-alternatives --config gcc
```
OR 
```bash
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 100
sudo update-alternatives --config g++
g++ --version
```
g++ (Ubuntu 10.5.0-1ubuntu1~20.04) 10.5.0

## Install swig
```bash
git clone https://github.com/swig/swig.git
cd swig
git checkout release-4.1
./autogen.sh
./configure --prefix=/usr/
make -j8
make install
```


```bash
cd flexric && mkdir build && cd build && cmake .. && make -j8 
make install
```
ref: https://gitlab.eurecom.fr/mosaic5g/flexric/-/tree/rc_slice_xapp?ref_type=heads


# Install OAI


The following command should change in future and use submodule to make the deployment stable.
```bash
cp -r ~/flexric-slicing/flexric/ ~/flexric-slicing/openairinterface5g/openair2/E2AP/
```

```bash
cd openairinterface5g/cmake_targets/
./build_oai -I -w SIMU --gNB --nrUE --build-e2 --ninja
./build_oai -w SIMU --gNB --nrUE --build-e2 --ninja
```


## Open5GS
```bash
git clone https://github.com/niloysh/testbed-automator.git
cd testbed-automator
./install.sh
kubectl create namespace open5gs
kubectl apply -k mongodb -n open5gs
kubectl apply -k networks5g -n open5gs

sudo apt-get install python3-pip
sudo pip3 install virtualenv
virtualenv venv
source venv/bin/activate
pip3 install -r requirements.txt

cd mongo-tools/
python3 generate-data.py
kubectl apply -k open5gs -n open5gs
kubectl apply -k open5gs-webui -n open5gs

cd mongo-tools/
python3 add-subscribers.py
python3 add-admin-account.py
```
The Open5GS webui is configured to run on port 30300.


# Run

```bash
cd ~/flexric-slicing/flexric
./build/examples/ric/nearRT-RIC
```
Interface of core
```bash
ip addr add 10.10.3.231/24 dev n3br
ip link set n3br up
```
gNB commands:

```bash

cd ~/flexric-slicing/openairinterface5g/cmake_targets/ran_build/build
sudo ./nr-softmodem -O ../../../targets/PROJECTS/GENERIC-NR-5GC/CONF/gnb.sa.band78.fr1.106prb.usrp210.modified.conf  --gNBs.[0].min_rxtxtime 6 --rfsim --sa
sudo ./nr-softmodem -O ../../../targets/PROJECTS/GENERIC-NR-5GC/CONF/gnb.sa.band78.fr1.106prb.usrp210.modified.conf --gNBs.[0].min_rxtxtime 6 --rfsim --sa --rfsimulator.serveraddr server

sudo ./nr-softmodem -O /home/setare/oai_slicing/RAN_OAI_Slicing/targets/PROJECTS/GENERIC-NR-5GC/CONF/monolithic.x310.106.conf --sa --tune-offset 30720000 --MACRLCs.[0].ul_max_mcs 14 --L1s.[0].max_ldpc_iterations 4 --usrp-tx-thread-config 1 | tee  output.txt
```
UE commands:
```bash
sudo -E ./nr-uesoftmodem -r 106 --numerology 1 --band 78 -C 3619200000 --rfsim --sa -O /root/flexric-slicing/openairinterface5g/targets/PROJECTS/GENERIC-NR-5GC/CONF/ue.conf
sudo RFSIMULATOR=127.0.0.1 ./nr-uesoftmodem -r 106 --numerology 1 --band 78 -C 3619200000  --rfsim --sa -O ../../../targets/PROJECTS/GENERIC-NR-5GC/CONF/ue.conf
sudo ./nr-uesoftmodem -r 106 --numerology 1 --band 78 -C 3619200000  --rfsim --sa -O ../../../targets/PROJECTS/GENERIC-NR-5GC/CONF/ue.conf --rfsimulator.serveraddr 10.201.1.100
```


# Multi UE
```bash
ip netns identify
```
To create a namespace for a UE $1
```bash
./multi-ue.sh -c1 -e
```
To delete a namespace for a UE $1 after first exiting the netns
```bash
./multi-ue.sh -d1
```
To open the shell 
```bash
./multi-ue.sh -o1
```
To list the namespaces
```bash
./multi-ue.sh -l
```

ref : https://gitlab.eurecom.fr/oaiworkshop/summerworkshop2023/-/tree/main/ran

# LOGs

The AMF logs when new gnb and ue connect
 
```bash
09/19 20:27:20.655: [amf] INFO: gNB-N2 accepted[10.10.3.231]:38306 in ng-path module (../src/amf/ngap-sctp.c:113)
09/19 20:27:20.656: [amf] INFO: gNB-N2 accepted[10.10.3.231] in master_sm module (../src/amf/amf-sm.c:741)
09/19 20:27:20.678: [amf] INFO: [Added] Number of gNBs is now 1 (../src/amf/context.c:1231)
09/19 20:27:20.678: [amf] INFO: gNB-N2[10.10.3.231] max_num_of_ostreams : 2 (../src/amf/amf-sm.c:780)
09/19 20:27:35.645: [amf] INFO: InitialUEMessage (../src/amf/ngap-handler.c:401)
09/19 20:27:35.645: [amf] INFO: [Added] Number of gNB-UEs is now 1 (../src/amf/context.c:2550)
09/19 20:27:35.646: [amf] INFO: Unknown UE by 5G-S_TMSI[AMF_ID:0x2004d,M_TMSI:0x56789abc] (../src/amf/ngap-handler.c:479)
09/19 20:27:35.646: [amf] INFO:     RAN_UE_NGAP_ID[1] AMF_UE_NGAP_ID[11] TAC[1] CellID[0xe0000] (../src/amf/ngap-handler.c:562)
09/19 20:27:35.646: [amf] INFO: [suci-0-001-01-0000-0-0-0000060592] Unknown UE by SUCI (../src/amf/context.c:1835)
09/19 20:27:35.647: [amf] INFO: [Added] Number of AMF-UEs is now 1 (../src/amf/context.c:1616)
09/19 20:27:35.647: [gmm] INFO: Registration request (../src/amf/gmm-sm.c:1165)
09/19 20:27:35.647: [gmm] INFO: [suci-0-001-01-0000-0-0-0000060592]    SUCI (../src/amf/gmm-handler.c:166)
09/19 20:27:37.003: [gmm] INFO: [imsi-001010000060592] Registration complete (../src/amf/gmm-sm.c:2146)
09/19 20:27:37.003: [amf] INFO: [imsi-001010000060592] Configuration update command (../src/amf/nas-path.c:612)
09/19 20:27:37.004: [gmm] INFO:     UTC [2024-09-19T20:27:37] Timezone[0]/DST[0] (../src/amf/gmm-build.c:559)
09/19 20:27:37.004: [gmm] INFO:     LOCAL [2024-09-19T20:27:37] Timezone[0]/DST[0] (../src/amf/gmm-build.c:564)
09/19 20:27:37.033: [amf] INFO: [Added] Number of AMF-Sessions is now 1 (../src/amf/context.c:2571)
09/19 20:27:37.034: [gmm] INFO: UE SUPI[imsi-001010000060592] DNN[internet] S_NSSAI[SST:1 SD:0x1] smContextRef [NULL] (../src/amf/gmm-handler.c:1241)
09/19 20:27:37.034: [gmm] INFO: SMF Instance [b155a29c-6eb4-41ef-9802-79ea83d0c910] (../src/amf/gmm-handler.c:1280)
09/19 20:27:37.203: [amf] INFO: [imsi-001010000060592:10:11][0:0:NULL] /nsmf-pdusession/v1/sm-contexts/{smContextRef}/modify (../src/amf/nsmf-handler.c:837)
```



The gnb logs for creating slices and initial assocation of UE to slices:

```bash 
[NR_MAC]   [E2-Agent]: RC CONTROL rx, RIC Style Type 2, Action ID 6
[NR_MAC]   Add default DL slice id 99, label default, sst 0, sd 0, slice sched algo NVS_CAPACITY, pct_reserved 0.05, ue sched algo nr_proportional_fair_wbcqi_dl
[NR_MAC]   configure slice 0, label SST1SD1, Dedicated_PRB_Policy_Ratio 70
[NR_MAC]   add DL slice id 1, label SST1SD1, slice sched algo NVS_CAPACITY, pct_reserved 0.66, ue sched algo nr_proportional_fair_wbcqi_dl
[NR_MAC]   Matched slice, Add UE rnti 0xfda1 to slice idx 0, sst 0, sd 0
[NR_MAC]   Matched slice, Add UE rnti 0xfda1 to slice idx 1, sst 1, sd 1
[NR_MAC]   configure slice 1, label SST1SD5, Dedicated_PRB_Policy_Ratio 30
[NR_MAC]   add DL slice id 2, label SST1SD5, slice sched algo NVS_CAPACITY, pct_reserved 0.28, ue sched algo nr_proportional_fair_wbcqi_dl
[NR_MAC]   Failed matching UE rnti fda1 with current slice (sst 1, sd 5), might lost user plane data
[E2-AGENT]: CONTROL ACKNOWLEDGE tx
```


The gnb logs for inter slice handover:
```bash 
[NR_MAC]   [E2-Agent]: RC CONTROL rx, RIC Style Type 3, Action ID 1
[NR_MAC]   [E2-Agent]: START INTER SLICE HO 
 [NR_MAC]   [MAC]: Find old_idx and new_idx 
 [NR_MAC]   RC_ngap_id: 0, RC_mcc: 1, RC_mnc: 1 || UE_ngap_id: 16, UE_mcc: 1, UE_mnc: 1 
[NR_MAC]   The ngap_id condition is not considered. (FIX ME) 
 
[NR_MAC]   Try triggering HO from slice 1 to slice 2
[NR_MAC]   The UE with ngap_id: 16, mcc: 1, mnc: 1 from slice 1 handed over to slice 2
[E2-AGENT]: CONTROL ACKNOWLEDGE tx
``` 

# Speed test
```bash
sudo apt update
sudo apt install speedtest-cli
speedtest-cli --source $ipOfTheInterface

curl --interface oaitun_ue1 -o /dev/null http://speedtest.tele2.net/100MB.zip

sudo ping -I oaitun_ue1 -s 1400 -i 0.01 8.8.8.8
```

# Monitoring

There has been changes made on the kpm xApp and SM, then after running xApp with the command below the is going to be stored in txt file. After that using flexric/examples/xApp/c/monitor/monitor_manager.py we can store data inn csv file.

```bash
./xapp_kpm_moni >> /root/flexric-slicing/flexric/examples/xApp/c/monitor/xApp_logs.txt
```