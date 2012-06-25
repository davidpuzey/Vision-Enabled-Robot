#!/bin/sh
g++ -ggdb `pkg-config --cflags opencv` -o overall overall.cpp rs232.c `pkg-config --libs opencv`
