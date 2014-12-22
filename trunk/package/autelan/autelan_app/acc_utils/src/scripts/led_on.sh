#!/bin/bash

echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:lan4/brightness
echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:lan3/brightness
echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:lan2/brightness
echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:lan1/brightness
echo 0 > /sys/devices/platform/leds-gpio/leds/db120:green:power/brightness
echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:wlan-2g/brightness
echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:wlan-5g/brightness
echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:alarm/brightness
echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:gps/brightness

