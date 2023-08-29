#!/bin/sh

g++ aes_256_xts.cpp -o aes_256_xts.o -g -c
g++ Options.cpp -o Options.o -g -c
g++ main.cpp -o main.o -g -c
g++ aes_256_xts.o main.o Options.o -lssl -lcrypto -o main
