#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
//#include"iperf_defaults.h"
#include "iperf_api.h"

void set_defaults(struct iperf_test * test){
	test->mode = DEFAULT_MODE;
	test->server_port = SERVER_PORT;
	test->protocol = 't';
	test->socket_bufsize = 144*1024;
}

void parse_args(struct iperf_test *test,int argc,char **argv){
	if(argc==1){
		usage();
		exit(1);
	}
	char c;
	while ((c = getopt (argc, argv, "c:sp:ub:")) != -1){
        switch (c){
			case 'c':
				test->mode='c';
				test->server_ip =(char*)malloc(strlen(optarg)+1);
				strcpy(test->server_ip,optarg);
				test->execute=client_tcp;
				break;
			case 's':
				test->mode='s';
				test->execute=server_tcp;
				break;
			case 'p':
				test->server_port=atoi(optarg);
				break;
			case 'u':
				test->protocol = 'u';
				test->bit_rate = 1000000;
				break;
			case 'b':
				test->protocol = 'u';
				test->bit_rate = atoi(optarg);
				break;
			default:
				 usage();
				 exit(0);
				 break;
		}
	}
	if(test->protocol=='u'){
		switch(test->mode){
			case 'c':
				test->execute=client_udp;
				break;
			case 's':
				test->execute=server_udp;
				break;
		}
	}
}
