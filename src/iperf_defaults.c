#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
//#include"iperf_defaults.h"
#include "iperf_api.h"

void set_defaults(struct iperf_test * test){
	test->mode = DEFAULT_MODE;
	test->server_port = SERVER_PORT;
}

void parse_args(struct iperf_test *test,int argc,char **argv){
	char c;
	while ((c = getopt (argc, argv, "c:sp:")) != -1){
        switch (c){
			case 'c':
				test->mode='c';
				test->execute=client_tcp;
				break;
			case 's':
				test->mode='s';
				test->execute=server_tcp;
				break;
			case 'p':
				test->server_port=atoi(optarg);
				break;
			default:
				 abort ();
		}
	}
}
