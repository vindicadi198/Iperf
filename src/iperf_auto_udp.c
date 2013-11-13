#include"iperf_api.h"
#include<math.h>
struct packet{
	uint32_t seq_no;
	char data[128*1024];
}__attribute__((packed));

static int connect_server(struct iperf_test *test){
    
	char *servIP=test->server_ip;
	in_port_t servPort=test->server_port;
    
	int sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sockfd <0){
		perror("socket() failed");
		exit(-1);
	}
	int bufsize = test->socket_bufsize,rv;
	rv=setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,&bufsize,sizeof(bufsize));
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));
	rv=setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,&bufsize,sizeof(bufsize));
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));

	bufsize = 0;
	unsigned int len=sizeof(bufsize);
	rv=getsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(void*)&bufsize,&len);
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));

	printf("buffer size is %d\n",bufsize);

    //set the server address
    struct sockaddr_in servAddr;
    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    int err=inet_pton(AF_INET,servIP,&servAddr.sin_addr.s_addr);
    if(err<=0){
        perror("inet_pton() failed");
        return -1;
    }
    servAddr.sin_port=htons(servPort); //h-host,n-network order s-short
    
    //connect to server
    if(connect(sockfd,(struct sockaddr *)&servAddr,sizeof(servAddr))){
        perror("connect failed");
        return -1;
    }
	return sockfd;
}
static int start_tcp_server(struct iperf_test *test){
	in_port_t servPort = test->server_port; //local port 
    
    int servSock;
    if((servSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0){
        perror("sockert() failed");
        exit(-1);
    }
	int bufsize = test->socket_bufsize,rv;
	rv=setsockopt(servSock,SOL_SOCKET,SO_SNDBUF,&bufsize,sizeof(bufsize));
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));
	rv=setsockopt(servSock,SOL_SOCKET,SO_RCVBUF,&bufsize,sizeof(bufsize));
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));
	bufsize = -1;
	unsigned int len=sizeof(bufsize);
	rv=getsockopt(servSock,SOL_SOCKET,SO_RCVBUF,(void*)&bufsize,&len);
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));
	printf("buffer size is %dKB\n",bufsize>>10);


    struct sockaddr_in servAddr;
    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servAddr.sin_port=htons(servPort);
    
    //Bind to local address
    if(bind(servSock,(struct sockaddr*)&servAddr,sizeof(servAddr))<0){
        perror("bind() failed");
        exit(-1);
    }
    
    //Listen to client
    if(listen(servSock,5)<0){
        perror("listen() failed");
        exit(-1);
    }
	return servSock;
}
	
void client_udp_auto(struct iperf_test *test){
	
	char *servIP=test->server_ip;
    
	in_port_t servPort=test->server_port;
	int sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(sockfd <0){
		perror("socket() failed");
		exit(-1);
	}
	int bufsize = test->socket_bufsize,rv;
	rv=setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,&bufsize,sizeof(bufsize));
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));
	rv=setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,&bufsize,sizeof(bufsize));
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));

	bufsize = 0;
	unsigned int len=sizeof(bufsize);
	rv=getsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(void*)&bufsize,&len);
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));

	printf("buffer size is %dKB\n",bufsize>>10);

    //set the server address
    struct sockaddr_in servAddr;
    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    int err=inet_pton(AF_INET,servIP,&servAddr.sin_addr.s_addr);
    if(err<=0){
        perror("inet_pton() failed");
        exit(-1);
    }
    servAddr.sin_port=htons(servPort); 

	bufsize = -1;
	rv=getsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(void*)&bufsize,&len);
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));
	printf("buffer size is %dKB\n",bufsize>>10);

	struct packet send_packet;
	send_packet.seq_no=0;
	srand(time(NULL));
	int packet_len=1024;
	for(int i=0;i<packet_len-4;i++){
		send_packet.data[i]=rand()%256;
	}
		
	double loss_rate=-1.0L,prev_loss_rate=-1.0L;
	int tcp_sock=connect_server(test);
	if(tcp_sock==-1){
		fprintf(stderr,"Unable to connect to server\n");
		exit(1);
	}
	unsigned int totalSent =0;
	struct timeval global_start,global_stop,start,stop;
	double diffTime=0.0L;
	uint64_t rate = 10000,d=(packet_len)*8;
	char someLoss = 0;
	FILE *udp_log=fopen("udp","w");
	do{
   	
		unsigned int  no_sends = (rate/d)+1;
		printf("No sends is %u\n",no_sends);
		double delay = (double)d/rate;
		diffTime=0.0L;totalSent=0;
		send_packet.seq_no=0;	
		err=send(tcp_sock,&no_sends,sizeof(int),0);
		if(err<0)
			perror("send() failed\n");
		else if(err!=sizeof(int))
			perror("send() failed to send full info\n");
		//close(tcp_sock);
		gettimeofday(&global_start,NULL);
		for(unsigned int i=0;i<no_sends;i++){
			gettimeofday(&start,NULL);
			ssize_t sentLen = sendto(sockfd,&send_packet,packet_len,0,(struct sockaddr *)&servAddr,sizeof(struct sockaddr));
			gettimeofday(&stop,NULL);
			diffTime = (stop.tv_usec-start.tv_usec);
			if(sentLen<0){
				perror("send() failed");
			}else if(sentLen != packet_len){
				perror("sent unexpected number of bytes");
			}
			totalSent+=sentLen;
			uint64_t wait_time = (delay*1000000L);
			gettimeofday(&start,NULL);
			while(1){
				gettimeofday(&stop,NULL);
				uint64_t diff = ((stop.tv_sec-start.tv_sec)*1000000)+(stop.tv_usec-start.tv_usec);
				if(diff>wait_time)
					break;
			}
			send_packet.seq_no = send_packet.seq_no+1;

		}
		gettimeofday(&global_stop,NULL);
		diffTime = ((global_stop.tv_sec-global_start.tv_sec)*1000000)+(global_stop.tv_usec-global_start.tv_usec);
		if((recv(tcp_sock,&loss_rate,sizeof(double),0))!=sizeof(double)){
			perror("recv() error in receiving loss rate");
			exit(1);
		}
		if(fabs((loss_rate-0))> 0.0001)
			someLoss=1;
		
		if(loss_rate>=prev_loss_rate&&someLoss&&(loss_rate*100)>test->loss_threshold_percent)
			rate = rate/4;
		else
			rate=rate*2;
		prev_loss_rate=loss_rate;
		double throughput = (totalSent/diffTime)*8000000;
		if(throughput<1000000000&&someLoss==0)
			rate=rate*2;
		printThroughput(throughput);
		printf("Loss ratio is %lf\n",loss_rate);
		fprintf(udp_log,"%lf\n",throughput);
	}while((loss_rate*100)<test->loss_threshold_percent|| someLoss==0);
	//printf("diffTime is %lf\n",diffTime);
	double throughput = (totalSent/diffTime)*8000000;
	//printf("The acheived throughput is %lfbit/sec %u\n",throughput,totalSent);
    printThroughput(throughput);
	printf("Loss rate at server is %lf\n",loss_rate);
	fclose(udp_log);
	close(sockfd);
	close(tcp_sock);

}

void server_udp_auto(struct iperf_test *test){
	in_port_t servPort = test->server_port; //local port 
    
    int servSock;
    if((servSock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))<0){
        perror("sockert() failed");
        exit(-1);
    }
	int bufsize = test->socket_bufsize,rv;
	rv=setsockopt(servSock,SOL_SOCKET,SO_SNDBUF,&bufsize,sizeof(bufsize));
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));
	rv=setsockopt(servSock,SOL_SOCKET,SO_RCVBUF,&bufsize,sizeof(bufsize));
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));

	bufsize = -1;
	unsigned int len=sizeof(bufsize);
	rv=getsockopt(servSock,SOL_SOCKET,SO_RCVBUF,(void*)&bufsize,&len);
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));

	printf("buffer size is %dKB\n",bufsize>>10);


    struct sockaddr_in servAddr;
    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servAddr.sin_port=htons(servPort);
    
    //Bind to local address
    if(bind(servSock,(struct sockaddr*)&servAddr,sizeof(servAddr))<0){
        perror("bind() failed");
        exit(-1);
    }
	int tcp_serv_sock=start_tcp_server(test);
	if(tcp_serv_sock==-1){
		fprintf(stderr,"Unable to open tcp socket\n");
		fprintf(stderr,"%s\n",strerror(errno));
		exit(1);
	}
    
    for(;;){
		struct sockaddr_in clientSock;
		unsigned int len=0;
		memset(&clientSock,0,sizeof(struct sockaddr_in));
		struct sockaddr_in clntAddr;
		socklen_t clntAddrLen = sizeof(clntAddr);
		int clntSock = accept(tcp_serv_sock,(struct sockaddr *)&clntAddr,&clntAddrLen);
		if(clntSock<0){
			perror("accept() failed");
			exit(-1);
		}
		double loss_ratio;
		char someLoss =0;
		do{
			unsigned int number_of_packets=0,number_of_received=0;
			int tcp_recvLen=recv(clntSock,&number_of_packets,sizeof(int),0);
			if(tcp_recvLen<0){
				fprintf(stderr,"recv() error\n");
				exit(1);
			}
			printf("Number of packets expected = %u\n",number_of_packets);
			while(1){
				bufsize=1024;
				char buffer[bufsize];
				memset(buffer,0,bufsize);
				ssize_t recvLen=recvfrom(servSock,buffer,bufsize-1,0,(struct sockaddr*)&clientSock,&len);
				if(recvLen<0){
					perror("recv() failed");
					exit(-1);
				}
				number_of_received++;
				struct packet *recv_packet=(struct packet*)buffer;
				//printf("Got packet sequence number %d\n",recv_packet->seq_no);
				if(recv_packet->seq_no==(number_of_packets-1)){
					printf("Received last sequence number\n");
					loss_ratio = 1.0-((double)number_of_received)/ number_of_packets;
					printf("Loss ratio is %lf\n",loss_ratio);
					if((send(clntSock,&loss_ratio,sizeof(double),0))!=sizeof(double)){
						perror("send() loss rate failed");
					}
					break;
				}
			}
			if(fabs((loss_ratio-0))> 0.0001)
				someLoss=1;
		}
		while(someLoss==0||(loss_ratio*100)<test->loss_threshold_percent);
		close(clntSock);
        printf("end of server program");
    }
    printf("End of program");
	close(tcp_serv_sock);
	close(servSock);

}
