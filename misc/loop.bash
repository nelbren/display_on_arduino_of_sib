#!/bin/bash
# loop.bash
# v0.0.1 - 2018-10-07 - nelbren.com

si=/usr/local/npres/bin/system/si.bash
output=/home/hosting/npr3s/si/si.txt

while true; do
  $si -n | cut -d"|" -f1 > $output
  sleep 120
done
