#include "con.h"
//this .cpp is declare function about connect & infomation
//for sp1

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

	//send file description
	while(1){
		//send info by SYN flag
		sndPkt.setHead(sPort,dPort, SYN_);
		sndPkt.header.ackNum = ackNum+1;
		sndPkt.header.seqNum = seqNum;
		memcpy(sndPkt.data, fDes.data, sizeof(sndPkt.data));
		if(sendto(fd, (char*)&sndPkt, sizeof(sndPkt), 0,( struct sockaddr*)&outPort, addrSize) < 0){
			FAIL("send file fail\n");
		}
		cout << "send info" << endl;

		setTimeOut(0,RTT);//time out for RTT microseconds
		if(recvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
			continue;
		}

		if(ackNum >= rcvPkt.header.seqNum){
			continue; // receive old packet ,drop
		}

		// receive SYNACK to check receiver know the info
		if(rcvPkt.header.SYN() && rcvPkt.header.ACK()){
			printf("Receive a SYNACK from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
			printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",ackNum = rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);
			break; //info send accessfully
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
		sndPkt.setHead(sPort,dPort, ACK_);
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
		if(recvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
			continue;
		}

		if(ackNum >= rcvPkt.header.seqNum){
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
		//usleep(100);
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
	
	cout << "Receive a file from " << inet_ntoa(outPort.sin_addr) << " : " << dPort << endl;
	for(int cur = 0; cur < fSize; ){
		if((cur + dSize) > fSize){
			dSize = fSize - cur;
		}

		setTimeOut(0,RTT);//time out for RTT microseconds
		if(recvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
			continue;
		}
		if(ackNum >= rcvPkt.header.seqNum){
			continue; // receive old packet ,drop
		}

		sndPkt.header.ackNum = ackNum+1;
		sndPkt.header.seqNum = seqNum;
		memset(sndPkt.data,0,sizeof(sndPkt.data));

		switch(rcvPkt.header.flag()){
			case SYN_:	//receive info
				cur = 0;				
				memcpy(fDes.data, rcvPkt.data, sizeof(fDes.data));
				fSize = fDes.info.size;
				sndPkt.setHead(sPort,dPort, SYN_ | ACK_);
				break;
			case ACK_:	//receice file
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
