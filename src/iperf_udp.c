#include"iperf_api.h"
#define BUFSIZE	(32*1024)
int connect_server(char *servIP,int port){
    
	in_port_t servPort=port;
    
	int sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sockfd <0){
		perror("socket() failed");
		exit(-1);
	}
	int bufsize = 128*1024,rv;
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
void client_udp(struct iperf_test *test){
	
	char *servIP=test->server_ip;
	char *echoString;
    
	in_port_t servPort=test->server_port;
   	/*int sock_tcp = connect_server(servIP,5001);
	if(sock_tcp==-1){
		perror("could not connect to server\n");
		exit(-1);
	}*/
	int sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(sockfd <0){
		perror("socket() failed");
		exit(-1);
	}
	int bufsize = 128*1024,rv;
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
    servAddr.sin_port=htons(servPort); //h-host,n-network order s-short
    
	bufsize = -1;
	rv=getsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(void*)&bufsize,&len);
	if(rv<0)
		printf("setsockopt error %s\n",strerror(errno));

	printf("buffer size is %dKB\n",bufsize>>10);
	srand(time(NULL));
	int echoStringLen=(1024);
	echoString = (char*)malloc(echoStringLen*sizeof(char));
	for(int i=0;i<echoStringLen;i++){
		echoString[i]=rand()%256;
	}
		

   	int rate = test->bit_rate,d=(echoStringLen)*8;
	int no_sends = ((int) rate/d);
	double delay = (double)d/rate;
	//printf("no of sends is %d\n",no_sends);
	//printf("Delay is %lf\n",delay); 
	unsigned int totalSent =0;
	struct timeval global_start,global_stop,start,stop;
	double diffTime=0.0L;
	gettimeofday(&global_start,NULL);
	for(int i=0;i<no_sends;i++){
		gettimeofday(&start,NULL);
		ssize_t sentLen = sendto(sockfd,echoString,echoStringLen,0,(struct sockaddr *)&servAddr,sizeof(struct sockaddr));
		gettimeofday(&stop,NULL);
		diffTime = (stop.tv_usec-start.tv_usec);
		//printf("difftime is %lf i is %d sentlen %ld\n",diffTime,i,sentLen);
		if(sentLen<0){
			perror("send() failed");
		}else if(sentLen != echoStringLen){
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

	}
	gettimeofday(&global_stop,NULL);
	diffTime = ((global_stop.tv_sec-global_start.tv_sec)*1000000)+(global_stop.tv_usec-global_start.tv_usec);
    
	printf("diffTime is %lf\n",diffTime);
	double throughput = (totalSent/diffTime)*8000000;
	printf("The acheived throughput is %lfbit/sec %u\n",throughput,totalSent);
    close(sockfd);
	free(echoString);

}

void server_udp(struct iperf_test *test){
	in_port_t servPort = test->server_port; //local port 
    
    int servSock;
    if((servSock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))<0){
        perror("sockert() failed");
        exit(-1);
    }
	int bufsize = 128*1024,rv;
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
    
    for(;;){
		struct sockaddr_in clientSock;
		unsigned int len=0;
		memset(&clientSock,0,sizeof(struct sockaddr_in));
		while(1){
			//Receive data
			char buffer[BUFSIZE];
			memset(buffer,0,BUFSIZE);
			ssize_t recvLen=recvfrom(servSock,buffer,BUFSIZE-1,0,(struct sockaddr*)&clientSock,&len);
			if(recvLen<0){
				perror("recv() failed");
				exit(-1);
			}
			//fputs(buffer,stdout);
			//if(recvLen!=0)
				//printf("Received string of size %lu\n",recvLen);
		}
        printf("end of server program");
    }
    printf("End of program");

}
