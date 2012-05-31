#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "rs232.h"

//char port* = "/dev/ttyACM0";

int main(int argc, const char** argv) {
	int err bsize=0,
		cport_nr=0,        /* /dev/ttyS0 (COM1 on windows) */
		bdrate=9600;       /* 9600 baud */
	unsigned char buf[4096];
	
	
	
	//wheel = atoi(argv[1]);
	//speed = atoi(argv[2]);
	// TODO case statement here for commands
	buf[0] = 'w';
	buf[1] = wheel;
	buf[2] = speed;
	buf[3] = 97;
	
	if(OpenComport(cport_nr, bdrate)) {
		printf("Can not open comport\n");
		return(0);
	}
	
	err = SendBuf(cport_nr, buf, bsize);
	if(err == -1) {
		printf("Could not send data");
	}
}
