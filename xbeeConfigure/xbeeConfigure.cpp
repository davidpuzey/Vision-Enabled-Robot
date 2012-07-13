#include "rs232.h"

#include <stdlib.h>
#include <iostream>
#include <pthread.h>

using namespace std;

void *ReadSerial(void*);

unsigned char buf[4096];
int cport_nr=0;

int main(int argc, const char** argv) {
	int bdrate=9600;
	string configCmd;
	pthread_t thread_id;
	
	if(OpenComport(cport_nr, bdrate)) {
		printf("Can not open comport\n");
		return(0);
	}
	
	if (pthread_create(&thread_id, NULL, ReadSerial, NULL) != 0) {
		printf("Camera thread could not be created.\n");
		exit(1);
	}
	
	SendBuf(cport_nr, (unsigned char*)"+++", 3);
	
	char cr = 13;
	while (1) {
		getline(cin, configCmd);
		configCmd.push_back(cr); // Add a carriage return as it seems the xbee module doesn't like line feeds
		SendBuf(cport_nr, (unsigned char*)configCmd.c_str(), configCmd.size()); // Send the command
	}
}

void *ReadSerial(void *param) {
	int n;
	while (1) {
		n = PollComport(cport_nr, buf, 4096);
		if (n > 0) {
			buf[n] = '\0';
			printf("Received: %s\n", (char *)buf);
		}
		usleep(100000);
	}
	return param;
}
