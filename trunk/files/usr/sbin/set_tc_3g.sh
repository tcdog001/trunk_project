#!/bin/sh
# xiaoh www.linuxbyte.org
 

IDEV="eth0"

#ap 3g download speed
UP="90mbit"
DOWN="90mbit"
#the user 3g download speed
DOWNLOAD="160kbit"
MDOWNLOAD="160kbit"

 
#IP
INET="192.168.0."
 

IPS="2" 
IPE="254"

tc qdisc del dev $IDEV root 2>/dev/null


tc qdisc add dev $IDEV root handle 10: htb default 256


tc class add dev $IDEV parent 10: classid 10:1 htb rate $DOWN ceil $DOWN
 

i=$IPS;
while [ $i -le $IPE ]
do

tc class add dev $IDEV parent 10:1 classid 10:2$i htb rate $DOWNLOAD ceil $MDOWNLOAD prio 1
tc qdisc add dev $IDEV parent 10:2$i handle 100$i: pfifo
tc filter add dev $IDEV parent 10: protocol ip prio 100 handle 2$i fw classid 10:2$i
iptables -t mangle -A PREROUTING -s $INET$i -j MARK --set-mark 2$i
iptables -t mangle -A PREROUTING -s $INET$i -j RETURN
iptables -t mangle -A POSTROUTING -d $INET$i -j MARK --set-mark 2$i
iptables -t mangle -A POSTROUTING -d $INET$i -j RETURN


i=`expr $i + 1`
done
