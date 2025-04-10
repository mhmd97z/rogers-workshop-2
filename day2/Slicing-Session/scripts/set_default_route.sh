#!/bin/bash

new_address=$(ip -br  a | grep oaitun | awk ' { print  $3 }' | sed -E 's/([0-9]+\.[0-9]+\.[0-9]+\.)[0-9]+\/[0-9]+/\11/'I)
echo "Setting the default route to ${new_address}"
ip route add default via "${new_address}" dev oaitun_ue1
