#include "con.h"
//this .cpp is declare function about connect & infomation

string who = "Someone";	//default exec name

void pConInfo(string ip,unsigned short port){	//print the connect infomation
	cout<<"=====Parameter=====\n";
	cout<<"The RTT delay = "<< RTT << " ms\n";
	cout<<"The threshold = "<< THRESHOLD << " bytes\n";
	cout<<"The MSS = "<< MSS << " bytes\n";
	cout<<"The buffer size = "<< RECV_BUF << " bytes\n";
	cout<< who << "'s IP is " << ip << "\n";
	cout<< who << " is listening on port "<< port << "\n";
	cout<<"====================\n";
}

tcpConnect::tcpConnect(string addr, unsigned short sPort){
	this->addr = addr;

	fd = socket(AF_INET,SOCK_DGRAM,0);// udp socket
	if(fd<0){
		DIE("creat socket fail\n");
	}
	
	fp = 0;
	cwnd = 1;				//congestion window
	rwnd = RECV_BUF;		//receive window
	memset((char*)&outPort,0,sizeof(outPort));//the output direct
	memset((char*)&inPort,0,sizeof(inPort));// the input direct
		
	// set input direct socket

	//familly is internet	
	inPort.sin_family=AF_INET;
	//set address by addr
	inPort.sin_addr.s_addr = inet_addr(addr.c_str());
	//set send port by port
	inPort.sin_port=htons(this->sPort = sPort);

	//bind socket by port
	if(bind(fd, (struct sockaddr*)&inPort, sizeof(inPort)) < 0){
		DIE("Bind socket fail\n");
	}	

}

tcpConnect::~tcpConnect(){
	if(fp){
		fclose(fp);
		fp = 0;
	}
	close(fd);
}

void tcpConnect::setTimeOut(int sec,int usec){
	//if 0,0 means cancel timeout
	timeOut = {sec,usec};	//time out in 1 sec
	if(setsockopt(fd		//socket
		, SOL_SOCKET		//level : socket
		, SO_RCVTIMEO		//option: receive time out
		, (char *)&timeOut	//time out value
		, sizeof(timeOut))	//option lenth
		< 0 ){
		DIE("setsockopt error\n");
	}
}

bool tcpConnect::tryCon(string dAddr, unsigned short dPort){	//try to make a connect
	// set output direct socket
	outPort.sin_family = AF_INET;

	outPort.sin_port = htons(this->dPort = dPort);

	if((outPort.sin_addr.s_addr= inet_addr(dAddr.c_str())) == INADDR_NONE){
		FAIL("address error\n");
	}

		//3-way-handshake
	TCP rcvPkt;		// store the receive packet
	TCP sndPkt;		// store the packet wait to send 
	socklen_t addrSize = sizeof(outPort);	//size of socket add
	seqNum = rand()%10000+1;	//seq randomly 1~10000
	ackNum = 4000;	//for neer 4096
	printf("=====Start 3-way handshake=====\n");
	while(1){
		//1st step send SYN
		sndPkt.setHead(sPort, dPort, SYN_);
		
		sndPkt.header.seqNum = seqNum;	//seq randomly 1~10000
		sndPkt.header.ackNum = ackNum + 1;

		if(sendto(fd, (char*)&sndPkt, sizeof(sndPkt), 0, (struct sockaddr*)&outPort, addrSize) < 0){
			FAIL("send packet fail\n");
		}
		printf("Send a SYN to %s : %d\n",inet_ntoa(outPort.sin_addr),sndPkt.header.dPort);

		//2nd step recv SYN,ACK
		setTimeOut(1,RTT);//time out for RTT microseconds
		if(recvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
			cout << "receive fail\n" << endl;
			continue;
		}

		if(rcvPkt.header.SYN() && rcvPkt.header.ACK()){
			printf("Receive a SYN,ACK from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
			printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);
			break;	// success connect
		}
		else{
			printf("Receive a unexpected from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
			printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);
			printf("=====3-way handshake be rejected=====\n");
			return 0;
		}
	}
	//3rd step send ACK
	sndPkt.setHead(sPort, dPort, ACK_);
	sndPkt.header.seqNum = seqNum;
	sndPkt.header.ackNum = ackNum + 1;

	if(sendto(fd, (char*)&sndPkt, sizeof(sndPkt), 0, (struct sockaddr*)&outPort, addrSize) < 0){
		FAIL("send packet fail\n");
	}
	printf("Send a ACK to %s : %d\n",inet_ntoa(outPort.sin_addr),sndPkt.header.dPort);
	
	printf("=====Complete the 3-way handshake=====\n");
	return 1;

}

bool tcpConnect::waitCon(){	//wait someone make a connect
		//3-way-handshake
	TCP rcvPkt;		// store the receive packet
	TCP sndPkt;		// store the packet wait to send
	socklen_t addrSize = sizeof(outPort);	//size of socket add
	seqNum = rand()%10000+1;	//seq randomly 1~10000
	printf("=====Start 3-way handshake=====\n");
	
	//listen to accept
	while(1){
		if(recvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
			FAIL("receive fail\n");
		}

		if(rcvPkt.header.SYN()){
			printf("Receive a SYN from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
			printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);
			dPort =  rcvPkt.header.sPort;
			break;
		}

		sleep(1);	// if receive unexpected drop and wait 
	}

	//send SYN,ACK to accept
	sndPkt.setHead(sPort, dPort, SYN_ | ACK_);
	sndPkt.header.ackNum = ackNum+1;
	sndPkt.header.seqNum = seqNum;
	
	if(sendto(fd, (char*)&sndPkt, sizeof(sndPkt), 0, (struct sockaddr*)&outPort, addrSize) < 0){
		FAIL("send packet fail\n");
	}
	printf("Send a SYN,ACK to %s : %d\n",inet_ntoa(outPort.sin_addr),sndPkt.header.dPort);

	//receive ACK
	setTimeOut(0,0);//cancel time out
	if(recvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
		FAIL("receive fail\n");
	}

	if(rcvPkt.header.flag() == ACK_){
		printf("Receive a ACK from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
		printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);
		
	}
	else{
		printf("Receive a unexpected from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
		printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);
		printf("=====3-way handshake has some problem=====\n");
		return 0;
	}
	
	printf("=====Complete the 3-way handshake=====\n");
	return 1;	
}


