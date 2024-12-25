# Authors: Jerry S.
# Date: November 10, 2024

#!/bin/sh

# The following script
# Scans for hosts on a subnet
# Used when no access to internal network

# Display ASCII art with the IP address
echo "    _       __  __            __           "
echo "   (_)___  / / / /_  ______  / /____  _____"
echo "  / / __ \/ /_/ / / / / __ \/ __/ _ \/ ___/"
echo " / / /_/ / __  / /_/ / / / / /_/  __/ /    "
echo "/_/ .___/_/ /_/\__,_/_/ /_/\__/\___/_/     "
echo " /_/                                       "
echo ""

echo "[i] Please enter the first 3 octets of the IP address you would like to ping: "
read -r ip_prefix

echo "\n[+] Happy Hunting! This may take a while..."

# Define output file
output_file="./huntedIps.txt"

# Clear the output file before starting
> "$output_file"

# Trap SIGINT (CTRL+C) to cleanly exit the script 
trap 'echo "[!] Scan interrupted."; exit 1' INT

# Loop through each IP and check for a response
for i in $(seq 1 254); do # CHANGE VALUES
  ip="$ip_prefix.$i"
  # Ping the IP with 1 packet and timeout of 1 second
  if timeout 1 ping -c 1 "$ip" > /dev/null; then
    # Append responsive IP to the output file
    echo "[+] $ip is responsive"
    echo "$ip" >> "$output_file"
  else
    echo "[!] $ip is not responsive"
  fi
done

echo "[+] Scan complete. Responsive IPs saved to $output_file."
