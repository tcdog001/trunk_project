#!/bin/bash

cp /etc/config/network-test  /etc/config/network 2>/dev/null
cp /etc/config/wireless-test-5g /etc/config/wireless 2>/dev/null
/etc/init.d/network restart 2>/dev/null
sleep 1
wifi down
sleep 1
wifi
sleep 5
