#include "con.h"
//this .cpp is declare function about connect & infomation
//for sp1
//redef recvfrom
int tcpConnect::myRecvfrom(int sockfd, TCP *buf, int len, unsigned int flag, struct sockaddr *from, socklen_t *fromlen){
	
	while(1){
		if(recvfrom(sockfd, buf, len, MSG_PEEK, from, fromlen) < 0){
			// for peek recv
			return -1;
		}
		if( (buf->header.sPort) == dPort){
			return recvfrom(sockfd, buf, len, flag, from, fromlen);
		}
		usleep(100);
	}
	return 0;
}


bool tcpConnect::sendF(string file){
	// send the file
	TCP rcvPkt;		// store the receive packet
	TCP sndPkt;		// store the packet wait to send 
	socklen_t addrSize = sizeof(outPort);	//size of socket add
	
	int fSize;	// file size
	int dSize = sizeof(sndPkt.data);	// data size
	DATA fDes;	//descript the file size

	fp = fopen(file.c_str(),"rb");
	if(!fp){
		FAIL("cannot open the file\n");
	}
	//get file size
	if(fseek(fp, 0, SEEK_END)){
		FAIL("fseek() fail\n");
	}
	if((fSize=ftell(fp)) < 0){
		FAIL("ftell() fail\n");
	}

	fDes.info.size = fSize;
	fseek(fp, 0, SEEK_SET);
	cout << "Start to send file 1 to " << inet_ntoa(outPort.sin_addr) << " : " << dPort << ", the file size is " << fSize << " bytes" << endl;
	//outPort.sin_port =  htons(dPort);
	//send file description
	while(1){
		//send info by RST_ flag
		sndPkt.setHead(sPort, dPort, RST_);
		sndPkt.header.ackNum = ackNum+1;
		sndPkt.header.seqNum = seqNum;
		memcpy(sndPkt.data, fDes.data, sizeof(sndPkt.data));
		if(sendto(fd, (char*)&sndPkt, sizeof(sndPkt), 0,( struct sockaddr*)&outPort, addrSize) < 0){
			FAIL("send file fail\n");
		}
		printf("send info to %s : %d\n", inet_ntoa(outPort.sin_addr), ntohs(outPort.sin_port));

		setTimeOut(0,RTT);//time out for RTT microseconds
		if(myRecvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
			continue;
		}

		if(ackNum > rcvPkt.header.seqNum){
			continue; // receive old packet ,drop
		}

		// receive ACK to check receiver know the info
		if(rcvPkt.header.ACK()){
			printf("Receive a ACK from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
			printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);
			cout << "send info succcess" << endl;
			break; //info send successfully
		}
		else{
			cout << "unexpected flag " << rcvPkt.header.flag() << endl;
			printf("Receive a unexpected from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
			printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);
			//info send fail, resend
		}
		usleep(500);
	}

	//send file
	for(int cur = 0; cur < fSize; ){
		sndPkt.setHead(sPort,dPort, 0);
		sndPkt.header.ackNum = ackNum+1;
		sndPkt.header.seqNum = seqNum;
		memset(sndPkt.data,0,sizeof(sndPkt.data));
		fseek(fp, cur, SEEK_SET);
		fread( sndPkt.data, sizeof(char), dSize , fp);

		if(sendto(fd, (char*)&sndPkt, sizeof(sndPkt), 0,( struct sockaddr*)&outPort, addrSize) < 0){
			FAIL("send file fail\n");
		}
		printf("\tSend a packet at : %d byte\n",cur);

		//wait ACK
		setTimeOut(0,RTT);//time out for RTT microseconds
		if(myRecvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
			continue;
		}

		if(ackNum >= rcvPkt.header.seqNum){
			cout << "duplicate ACK seq:" << rcvPkt.header.seqNum << endl;
			continue; // receive old packet ,drop
		}

		if(rcvPkt.header.ACK()){
			printf("Receive a ACK from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
			printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);
			cur += dSize; //data send accessfully
		}
		else{
			cout << "unexpected flag " << rcvPkt.header.flag() << endl;
			printf("Receive a unexpected from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
			printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);\
			cur += 0; //data send non accessfully
		}
		usleep((rand()%15+5) * 100000);	//avoid send too fast
	}
	cout << "===== File Send Complete =====" << endl << endl;
	fclose(fp);
	fp = 0;
	return 1;
}

bool tcpConnect::recvF(string file){
	// receive the file
	TCP rcvPkt;		// store the receive packet
	TCP sndPkt;		// store the packet wait to send 
	socklen_t addrSize = sizeof(outPort);	//size of socket add
	
	int fSize = 0x7FFFFFFF;	// file size
	int dSize = sizeof(sndPkt.data);	// data size
	DATA fDes;	//descript the file size

	fp = fopen(file.c_str(),"wb");
	if(!fp){
		FAIL("cannot write the file\n");
	}
	//get file size
	
	cout << "Receive a file from " << inet_ntoa(outPort.sin_addr) << " : " << ntohs(outPort.sin_port) << endl;
	for(int cur = 0; cur < fSize; ){
		if((cur + dSize) > fSize){
			dSize = fSize - cur;
		}

		setTimeOut(0,RTT);//time out for RTT microseconds
		if(recvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
			continue;
		}
		if(ackNum >= rcvPkt.header.seqNum){
				// receive old packet ,drop
			cout << "duplicate ACK seq:" << rcvPkt.header.seqNum << endl;
			//continue;
		}
		else{	// receive new packet

			switch(rcvPkt.header.flag()){	
				case RST_:	//receive info
					cur = 0;				
					memcpy(fDes.data, rcvPkt.data, sizeof(fDes.data));
					fSize = fDes.info.size;
					sndPkt.setHead(sPort, dPort, ACK_);
					cout << "receive info" << endl;
					break;
				case 0:	//receice file
					fseek(fp, cur, SEEK_SET);
					fwrite(rcvPkt.data, sizeof(char), dSize, fp);
					cur += dSize; //data receive accessfully
					sndPkt.setHead(sPort,dPort, ACK_);
					break;
				default:	//receive unexpected
					cout << "unexpected flag " << rcvPkt.header.flag() << endl;
					sndPkt.setHead(sPort,dPort, 0);				
			}
			printf("Receive a packet from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
			printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);

		
			sndPkt.header.ackNum = ackNum+1;
			sndPkt.header.seqNum = seqNum;
			memset(sndPkt.data,0,sizeof(sndPkt.data));
		}
		printf("Send a ACK to %s : %d\n",inet_ntoa(outPort.sin_addr),sndPkt.header.dPort);
		if(sendto(fd, (char*)&sndPkt, sizeof(sndPkt), 0,( struct sockaddr*)&outPort, addrSize) < 0){
			FAIL("send file fail\n");
		}
		//usleep(100);
	}
	cout << "===== File Receive Complete =====" << endl << endl;
	fclose(fp);
	fp = 0;
	return 1;
}


bool tcpConnect::reqF(string file){//request file
	// receive the file
	TCP rcvPkt;		// store the receive packet
	TCP sndPkt;		// store the packet wait to send 
	socklen_t addrSize = sizeof(outPort);	//size of socket add
	
	int dSize = sizeof(sndPkt.data);	// data size
	DATA fDes;	//descript the file name
	strcpy(fDes.info.name, file.c_str());
	
	seqNum++;// becasue with 3way handshack make duplicate ack 

	while(1){
		//send info by RST_ flag
		sndPkt.setHead(sPort, dPort, RST_);
		sndPkt.header.ackNum = ackNum+1;
		sndPkt.header.seqNum = seqNum;
		memcpy(sndPkt.data, fDes.data, dSize);
		if(sendto(fd, (char*)&sndPkt, sizeof(sndPkt), 0,( struct sockaddr*)&outPort, addrSize) < 0){
			FAIL("send file request fail\n");
		}
		printf("send request to %s : %d\n", inet_ntoa(outPort.sin_addr), ntohs(outPort.sin_port));

		setTimeOut(0,RTT);//time out for RTT microseconds
		if(recvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
			continue;
		}

		if(ackNum > rcvPkt.header.seqNum){
			continue; // receive old packet ,drop
		}

		// receive ACK to check receiver know the info
		if(rcvPkt.header.ACK()){
			printf("Receive a ACK from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
			printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);
			cout << "send request succcess" << endl;
			break; //req send successfully
		}
		else{
			cout << "unexpected flag " << rcvPkt.header.flag() << endl;
			printf("Receive a unexpected from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
			printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);
			//info send fail, resend
		}
		usleep(500);
	}
	return 1;
}


bool tcpConnect::wReqF(string& file){// wait request file

	TCP rcvPkt;		// store the receive packet
	TCP sndPkt;		// store the packet wait to send 
	socklen_t addrSize = sizeof(outPort);	//size of socket add
	
	int dSize = sizeof(sndPkt.data);	// data size
	DATA fDes;	//descript the file name

	cout << "wait request from " << inet_ntoa(outPort.sin_addr) << " : " << ntohs(outPort.sin_port) << endl;
	while(1){
		setTimeOut(0,RTT);//time out for RTT microseconds
		if(myRecvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
			continue;
		}
		if(ackNum >= rcvPkt.header.seqNum){
				// receive old packet ,drop
			cout << "duplicate packet seq:" << rcvPkt.header.seqNum << endl;
			//continue;
		}
		else{	// receive new packet

			if(rcvPkt.header.flag() == RST_){	
				//receive info			
				memcpy(fDes.data, rcvPkt.data, dSize);
				file = fDes.info.name;
				sndPkt.setHead(sPort, dPort, ACK_);
				cout << "receive request" << endl;
				ackNum = rcvPkt.header.seqNum;
				seqNum = rcvPkt.header.ackNum;
				printf("Receive a packet from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
				printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n", rcvPkt.header.seqNum, rcvPkt.header.ackNum);
				break;
			}
			else{	//receive unexpected
				cout << "unexpected flag " << rcvPkt.header.flag() << endl;	
				printf("Receive a packet from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
				printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n", rcvPkt.header.seqNum, rcvPkt.header.ackNum);
				return 0;
			}

			
			sndPkt.header.ackNum = ackNum+1;
			sndPkt.header.seqNum = seqNum;
			memset(sndPkt.data,0,sizeof(sndPkt.data));
		}
	}

	printf("Send a ACK to %s : %d\n",inet_ntoa(outPort.sin_addr),sndPkt.header.dPort);
	if(sendto(fd, (char*)&sndPkt, sizeof(sndPkt), 0,( struct sockaddr*)&outPort, addrSize) < 0){
		FAIL("send file fail\n");
	}
	usleep(100);
	
	cout << "===== File Request Receive =====" << endl << endl;
	return 1;
}
