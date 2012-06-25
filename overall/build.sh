#!/bin/sh
g++ -ggdb `pkg-config --cflags opencv` -o overall overall.cpp rs232.c `pkg-config --libs opencv`
g++ -ggdb `pkg-config --cflags opencv` -o test test.cpp rs232.c `pkg-config --libs opencv`
