#include "rs232.h"

#include <stdlib.h>
#include <iostream>
#include <pthread.h>

using namespace std;

void *ReadSerial(void*);

unsigned char buf[4096];
int cport_nr=0;

int main(int argc, const char** argv) {
	int bdrate=9600;       /* baud rate */
	pthread_t thread_id;
	
	if(OpenComport(cport_nr, bdrate)) {
		printf("Can not open comport\n");
		return(0);
	}
	
	if (pthread_create(&thread_id, NULL, ReadSerial, NULL) != 0) {
		printf("Camera thread could not be created.\n");
		exit(1);
	}
}

void *ReadSerial(void *param) {
	int i, n;
	while (1) {
		n = PollComport(cport_nr, buf, 4096);
		if (n > 0) {
			for (i=0; i < n; i++) {
				if (buf[i] < 32)
					printf("%i;", buf[i]);
			}
			buf[n] = '\0';
			printf("Received: %s\n", (char *)buf);
		}
		usleep(100000);
	}
	return param;
}
