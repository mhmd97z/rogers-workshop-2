#!/bin/bash
rm /home/user/workshop/collected_metrics/exporter_stats_rlc.json /home/user/workshop/collected_metrics/exporter_stats_mac.json
cd /home/user/workshop/flexric/build/examples/xApp/python3
python3 monitoring_exporter.py