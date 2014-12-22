#!/bin/bash

cp /rom/etc/config/network /etc/config/network
cp /rom/etc/config/firewall /etc/config/firewall

/etc/init.d/network restart > /dev/null 2>&1
/etc/init.d/firewall restart > /dev/null 2>&1


