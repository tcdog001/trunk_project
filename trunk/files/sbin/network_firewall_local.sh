#!/bin/sh

cp /rom/etc/config/network.dev /etc/config/network
cp /rom/etc/config/firewall.dev /etc/config/firewall

/etc/init.d/network reload > /dev/null 2>&1
/etc/init.d/firewall restart > /dev/null 2>&1


