#!/bin/sh
echo "Building overall"
g++ -ggdb `pkg-config --cflags opencv` -o overall overall.cpp rs232.c `pkg-config --libs opencv`
#echo "Building test"
#g++ -ggdb `pkg-config --cflags opencv` -o test test.cpp rs232.c `pkg-config --libs opencv`
