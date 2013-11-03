#ifndef __IPERF_API_H
#define __IPERF_API_H

#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include "iperf_defaults.h"
struct iperf_test{
	char mode;
	char *server_ip;
	int server_port;
	char protocol;
	uint64_t bit_rate;
	int socket_bufsize;
	void (*execute)(struct iperf_test *);
};
void usage();
void set_defaults(struct iperf_test * test);
void parse_args(struct iperf_test *test,int argc,char **argv);
void server_tcp(struct iperf_test *);
void client_tcp(struct iperf_test *);
void server_udp(struct iperf_test *);
void client_udp(struct iperf_test *);
void destroy(struct iperf_test *test);
#endif
