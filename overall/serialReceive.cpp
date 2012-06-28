#include "rs232.h"

#include <iostream>
#include <stdio.h>
#include <cmath>

using namespace std;

int main(int argc, const char** argv) {
	int cport_nr=0,        /* 0 is Arduino com 0 and 1 is Arduino com 1 */
		bdrate=9600,       /* 9600 baud */
		n;
	unsigned char buf[4096];
	
	if(OpenComport(cport_nr, bdrate)) {
		printf("Can not open comport\n");
		return(0);
	}
	
	while (1) {
		n = PollComport(cport_nr, buf, 4096);
		if (n > 0) {
			buf[n] = 0;
			printf("%s\n", buf);
		}
		usleep(100000);
	}
}
