#!/bin/sh

iptables -I FORWARD  -m connlimit --connlimit-above 50 -j REJECT
