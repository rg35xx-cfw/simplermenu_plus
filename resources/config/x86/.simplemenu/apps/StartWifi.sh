#!/bin/sh

ifconfig wlan0 down
killall wpa_supplicant
ifconfig wlan0 up
wpa_supplicant -Dnl80211 -iwlan0 -c/userdata/system/wpa_supplicant.conf &


