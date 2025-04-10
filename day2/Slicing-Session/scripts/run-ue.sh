#!/bin/bash

create_namespace() {
  local ue_id=$1
  local name="ue${ue_id}"
  echo "creating the network namespace ${name}"
  ip netns add "${name}"
  ip link add "v-eth${ue_id}" type veth peer name "v-ue${ue_id}"
  ip link set "v-ue${ue_id}" netns "${name}"
  local BASE_IP=$((200+ue_id))
  ip addr add "10.${BASE_IP}.1.100/24" dev "v-eth${ue_id}"
  ip link set "v-eth${ue_id}" up
  iptables -t nat -A POSTROUTING -s "10.${BASE_IP}.1.0/255.255.255.0" -o lo -j MASQUERADE
  ip netns exec "${name}" ip link set dev lo up
  ip netns exec "${name}" ip addr add "10.${BASE_IP}.1.${ue_id}/24" dev "v-ue${ue_id}"
  ip netns exec "${name}" ip link set "v-ue${ue_id}" up
}

delete_namespace() {
  local ue_id=$1
  local name="ue${ue_id}"
  echo "deleting namespace ${name}"
  ip link delete "v-eth${ue_id}"
  ip netns delete "${name}"
  local BASE_IP=$((200+ue_id))
  sudo iptables -t nat -D POSTROUTING -s "10.${BASE_IP}.1.0/255.255.255.0" -o lo -j MASQUERADE
}

inside_namespace() {
  ip -br -c a | grep -q "v-ue"
  local outside=$?
  if [[ ${outside} != 1 ]]; then
    echo "Cannot run the command inside the namespace. Please Press Ctrl+D to exit the network namespace and try running the script outside the network namespace again"
    exit 1
  fi
}

if [[ $(id -u) -ne 0 ]]; then 
  echo "Please run as root"; 
  exit 1; 
fi

inside_namespace

read -p "Enter the UE ID: " ue_id
if [[ -z "${ue_id}" ]]; then
  echo "Error: UE ID cannot be empty. Exiting."
  exit 1
elif [[ ! "${ue_id}" =~ ^[1-9]$ ]]; then
  echo "Error: UE ID should be a number from 1 to 9. Exiting."
  exit 1
fi


namespace_name="ue${ue_id}"
if ip netns list | grep -q "${namespace_name}";  then
  echo "namespace ${namespace_name} already exists. skipping creation"
else
  create_namespace "${ue_id}"
fi


nr_ue="../flexric-slicing/openairinterface5g/cmake_targets/ran_build/build/nr-uesoftmodem"
ip netns exec "${namespace_name}" bash -c "exec ${nr_ue} -r 106 --numerology 1 --band 78 -C 3619200000 --rfsim --sa  -O ./configs/ue_${ue_id}.conf --rfsimulator.serveraddr 10.20${ue_id}.1.100 &> ./ue_${ue_id}.log" &
pid=$!

cleanup() {
  sudo kill -SIGINT "${pid}"
  [[ -f "${ue_rc_file}" ]] && rm "${ue_rc_file}"
  exit 0
}

set_prompt_file() {
  ue_prompt="<<ue${ue_id}>> -- ${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ "
  ue_rc_file="./ue_${ue_id}_rcfile.sh"
  echo "PS1='${ue_prompt}'" > "${ue_rc_file}"
}

trap "cleanup" SIGINT SIGTERM EXIT

echo "Setting the UE up..." && sleep 10

if ! sudo ip netns exec "${namespace_name}" ip -br a | grep -q oaitun_ue1; then
  echo "Error: Failed to start UE ${ue_id}. Exiting."
  exit 1
fi

sudo ip netns exec "${namespace_name}" ./set_default_route.sh

set_prompt_file

sudo ip netns exec "${namespace_name}" bash --rcfile "${ue_rc_file}"
