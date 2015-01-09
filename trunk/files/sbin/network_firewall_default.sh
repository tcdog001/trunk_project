#!/bin/bash

cp /rom/etc/config/network /etc/config/network
cp /rom/etc/config/firewall /etc/config/firewall

set_evdo_service.sh > /dev/null 2>&1
/etc/init.d/firewall restart > /dev/null 2>&1


