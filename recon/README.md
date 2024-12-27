```
    _       __  __            __           
   (_)___  / / / /_  ______  / /____  _____
  / / __ \/ /_/ / / / / __ \/ __/ _ \/ ___/
 / / /_/ / __  / /_/ / / / / /_/  __/ /    
/_/ .___/_/ /_/\__,_/_/ /_/\__/\___/_/     
 /_/                                       
```

# What is it?
---
IP hunter is a script that executes a ping scan to enumerate hosts. 

We decided to create this for very specific scenarios. Sure, there's metasploit modules, nmap, linpeas, etc to perform this task, but what if you couldn't use/upload those tools? That would be the specific scenario you would use these scripts. Additionally, winpeas doesn't have a host discovery function like linpeas, so we basically created our own version. 

It's about as fast as linpeas too which is nice.

Hopefully port scan or specifying number of threads will be implemented in the near future. 

# How to use it
---
Compile the binaries (should connect to internet just in case):
./setup.sh

```
./iphunter.so

Enter IP address and subnet range (ex. 192.168.1.0/24): 172.16.1.0/24
```

Currently there is no Ctrl + C option to stop the program, unless it's fixed, run at your own risk. 