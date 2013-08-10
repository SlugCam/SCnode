ifconfig wlan0 up;
iwconfig wlan0 essid $(grep wifi /home/pi/build5/config | cut -d = -f 2);
sleep 2;
dhcpcd wlan0;
