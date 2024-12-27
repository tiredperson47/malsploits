#!/bin/bash

gcc -o iphunter.so linux_iphunter.c -lp -pthread
x86_64-w64-mingw32-gcc -o iphunter.exe win_iphunter.c -static -lm -pthread -lws2_32
