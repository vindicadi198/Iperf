#include "iperf_api.h"
#include<time.h>
#ifdef __linux
#include<linux/tcp.h>
#endif
static const int MAXPENDING =5;
#define BUFSIZE (128*1024)

void usage(){
	printf("IITH iperf options:\n");
	printf("iperf [OPTIONS] \n");
	printf("Options:\n");
	printf("\t-c SERVERIP : run iperf in client and test with server at SERVERIP\n");
	printf("\t-s : run iperf in server mode \n");
	printf("\t-b : bit rate for UDP tests (implicitly UDP mode)\n");
	printf("\t-p PORT : If server listen on port PORT, in client port to connect to server(Default:5001) \n");
	printf("\t-u : Run iperf in UDP mode (Default bit rate:1 Mbps)\n");
}
#ifdef __linux
void output_tcpinfo(FILE *of,int sock){
	if(of==NULL)
		return;
	struct tcp_info tcpInfo;
	unsigned int len=-1;
	getsockopt(sock,SOL_SOCKET, TCP_INFO, &tcpInfo, &len);
	fprintf(of,"%d %u %u %u %u %u %u %u %u %u %u %u %u\n",
			tcpInfo.tcpi_state,
			tcpInfo.tcpi_last_data_sent,
			tcpInfo.tcpi_last_data_recv,
			tcpInfo.tcpi_snd_cwnd,
			tcpInfo.tcpi_snd_ssthresh,
			tcpInfo.tcpi_rcv_ssthresh,
			tcpInfo.tcpi_rtt,
			tcpInfo.tcpi_rttvar,
			tcpInfo.tcpi_unacked,
			tcpInfo.tcpi_sacked,
			tcpInfo.tcpi_lost,
			tcpInfo.tcpi_retrans,
			tcpInfo.tcpi_fackets
		   );
	fflush(of);
}
#endif
	
void client_tcp(struct iperf_test * test){
	char *servIP=test->server_ip;
	char *echoString;
    
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
    
    //connect to server
    if(connect(sockfd,(struct sockaddr *)&servAddr,sizeof(servAddr))){
        perror("connect failed");
        exit(-1);
    }
	bufsize = -1;
	rv=getsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(void*)&bufsize,&len);
		if(rv<0)
			printf("setsockopt error %s\n",strerror(errno));

	printf("buffer size is %dKB\n",bufsize>>10);
	int send_handshake = IPERF_TEST_START;
	if(send(sockfd,&send_handshake,sizeof(int),0)!=sizeof(int)){
		perror("Error starting test handshake");
		close(sockfd);
		exit(1);
	}
	
	srand(time(NULL));
	int echoStringLen=test->socket_bufsize;
	echoString = (char*)malloc(echoStringLen*sizeof(char));
	for(int i=0;i<echoStringLen;i++){
		echoString[i]=rand()%256;
	}
		
    //send string to server
	unsigned int totalSent =0;
	struct timeval start,stop;
	uint64_t diffTime=0.0L;
#ifdef __linux
	FILE *of = fopen("log","w");
	if(of==NULL){
		perror("Unable to open log file");
		exit(-1);
	}
	fprintf(of,"State LastDataSent LastDataRecv SNDCWND SNDSTHRESH RCVSTHRESH RTT RTTVAR UNACK SACKED LOST RETRANS FACKS\n");
#endif
	for(int i=0;i<800;i++){
		gettimeofday(&start,NULL);
		ssize_t sentLen = send(sockfd,echoString,echoStringLen,0);
		gettimeofday(&stop,NULL);
#ifdef __linux
		output_tcpinfo(of,sockfd);
#endif
		diffTime += ((stop.tv_sec-start.tv_sec)*1000000)+(stop.tv_usec-start.tv_usec);
		if(sentLen<0){
			perror("send() failed");
			exit(-1);
		}else if(sentLen != echoStringLen){
			perror("sent unexpected number of bytes");
			//exit(-1);
		}
		totalSent+=sentLen;
		if(diffTime>=(10000000))
			break;
	}
	send_handshake = IPERF_TEST_STOP;
	if(send(sockfd,&send_handshake,sizeof(int),0)!=sizeof(int)){
		perror("Error starting test handshake");
		close(sockfd);
		exit(1);
	}else{
		printf("Stopping test client side\n");
	}
	gettimeofday(&stop,NULL);
	printf("diffTime is %llu\n",diffTime);
	double throughput = (totalSent/diffTime)*8000000;
	printf("The acheived throughput is %lfbit/sec %u\n",throughput,totalSent);
#ifdef __linux
	fclose(of);
#endif
    close(sockfd);
	free(echoString);
}

void server_tcp(struct iperf_test * test){
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
    if(listen(servSock,MAXPENDING)<0){
        perror("listen() failed");
        exit(-1);
    }
    
    for(;;){
        struct sockaddr_in clntAddr;
        socklen_t clntAddrLen = sizeof(clntAddr);
        //Wait for a client to connect
        int clntSock = accept(servSock,(struct sockaddr *)&clntAddr,&clntAddrLen);
        if(clntSock<0){
            perror("accept() failed");
            exit(-1);
        }
		int recv_handshake;
       	if(recv(clntSock,&recv_handshake,sizeof(int),0)!=sizeof(int)){
			perror("Error receiving handshake from client");
			close(clntSock);
			continue;
		}else if(recv_handshake!=IPERF_TEST_START){
			perror("Wrong Handshake message\n");
			close(clntSock);
			continue;
		}
        char clntIpAddr[INET_ADDRSTRLEN];
        if(inet_ntop(AF_INET,&clntAddr.sin_addr.s_addr,clntIpAddr,sizeof(clntIpAddr))!=NULL){
            printf("Handling client %s %d\n",clntIpAddr,ntohs(clntAddr.sin_port));
        }else{
            puts("unable to get client IP address");
        }
		
		while(1){
			//Receive data
			char buffer[bufsize];
			memset(buffer,0,bufsize);
			ssize_t recvLen=recv(clntSock,buffer,bufsize-1,0);
			if(recvLen<0){
				perror("recv() failed");
				exit(-1);
			}else if(recvLen==0){
				printf("Iperf stop testing\n");
				close(clntSock);
				break;
			}else if(recvLen==sizeof(int)){
				int *p=(int*)buffer;
				if((*p)==IPERF_TEST_STOP){
					printf("Iperf test stop received\nStopping Test!!\n");
					break;
				}
			}
		}
		close(clntSock);
        printf("end of server program");
    }
    printf("End of program");
	close(servSock);
}
void destroy(struct iperf_test * test){
	free(test->server_ip);
}
