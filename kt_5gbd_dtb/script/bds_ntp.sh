#!/bin/bash

ntp_config_file=/etc/ntp.conf
gpsd_config_file=/etc/default/gpsd

function ntp_config()
{
    if [ ! -f "$ntp_config_file" ];then
        echo "ntp config file not exit."
        return 1    
    fi

    back_file=$ntp_config_file.bak
    
    if [ -z "$(grep "server 127.127.28.0 minpoll 4 maxpoll 4" $ntp_config_file)" ]; then
        cp $ntp_config_file $back_file
        echo "server 127.127.28.0 minpoll 4 maxpoll 4" > $ntp_config_file
        echo "fudge 127.127.28.0 refid BDS stratum 1" >> $ntp_config_file
    fi

    return 0
}

function gpsd_config()
{
    if [ ! -f "$gpsd_config_file" ]; then
        echo "gpsd config file not exit."
        return 1
    fi

    devices="/dev/ttyS4"
    stty -F $devices 115200
    gpsd_options="-n -b -s 115200 -G -S 2947 -F /var/run/gpsd.sock"
    if [ -z "$(grep "$devices" $gpsd_config_file)" ]; then
        sed -i "s#^DEVICES=\"\"#DEVICES=\"$devices\"#" $gpsd_config_file
        sed -i "s#^GPSD_OPTIONS=\"\"#GPSD_OPTIONS=\"$gpsd_options\"#" $gpsd_config_file
    fi

    return 0
}

function enable_server()
{
    ntp_config
    gpsd_config
    systemctl restart ntp gpsd
}

function disable_server()
{
    systemctl stop ntp
    systemctl stop gpsd
}

function ntp_server_check()
{

    if [ -n "$(ntpq -pn | grep "BDS")" ]; then
        echo "NTP service has taken effect."
        return 0
    fi

    enable_server

    sleep 5
}


ntp_server_check


