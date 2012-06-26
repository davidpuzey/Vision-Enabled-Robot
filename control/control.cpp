#include <stdlib.h>
#include <stdio.h>
#include "rs232.h"

//char port* = "/dev/ttyACM0";

int main(int argc, const char** argv) {
	int err, bsize=0,
		cport_nr=0,        /* 0 is ACM0 and 1 is ACM1*/
		bdrate=9600;       /* 9600 baud */
	unsigned char buf[4096];
	char command = argv[1][0];
	
	// TODO Put error checking stuff in ... this is very poor coding >:(
	switch (command) {
		case 'p': // change platform angle
			bsize = 3;
			buf[0] = 'p';
			buf[1] = atoi(argv[2]);
			buf[2] = atoi(argv[3]);
			break;
		case 'u': // get ultrasonic reading
			bsize = 1;
			buf[0] = 'u';
			break;
		case 'i': // get irpd reading
			bsize = 1;
			buf[0] = 'i';
			break;
		case 'm': // change speed
			bsize = 3;
			buf[0] = 'm';
			buf[1] = atoi(argv[2]);
			buf[2] = 0;
			break;
		case 't': // turn
			bsize = 3;
			buf[0] = 't';
			buf[1] = atoi(argv[2]);
			buf[2] = 0;
			break;
		case 'w': // set wheel speed
			bsize = 4;
			buf[0] = 'w';
			buf[1] = atoi(argv[2]);
			buf[2] = atoi(argv[3]);
			buf[3] = 0;
			break;
		default:
			return 0;
	}
	
	if(OpenComport(cport_nr, bdrate)) {
		printf("Can not open comport\n");
		return(0);
	}
	
	err = SendBuf(cport_nr, buf, bsize);
	if(err == -1) {
		printf("Could not send data");
	}
}
