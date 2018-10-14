#include "con.h"
//this .cpp is declare function about connect & infomation
//for sp1

bool tcpConnect::sendF(string file){
	// send the file
	TCP rcvPkt;		// store the receive packet
	TCP sndPkt;		// store the packet wait to send 
	socklen_t addrSize = sizeof(outPort);	//size of socket add
	
	int fSize;			// file size
	int dSize = sizeof(sndPkt.data);	// data size
	DATA fDes;			//descript the file size
	int nCur;			//for packet 2 cur

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
			cout << "duplicate ACK " << rcvPkt.header.seqNum << endl;
			continue; // receive old packet ,drop
		}

		// receive SYNACK to check receiver know the info
		if(rcvPkt.header.SYN() && rcvPkt.header.ACK()){
			printf("Receive a ACK from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
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

	//****//
	//send file
	
	state = CONV;	// for initial can be change to SLOW
	cwnd = 0;
	rwnd = RECV_BUF;
	TOSig = TRUE;
	for(int cur = 0; cur < fSize; ){
	
		switch(state){//state chage
			case SLOW:
				if(cwnd >= THRESHOLD){
					state = CONV;	// => congestion avoid
					cout << "****Congestion avoidance****" << endl;
				}
				break;
			case CONV:
				if(TOSig){
					state = SLOW;	// => slow start
					cout << "*****Slow start*****" << endl;
					TOSig = FALSE;
				}
				break;
			default:
				cout << "unknown transfor state :" << state << endl;
				return 0;
		}

		switch(state){	//cwnd change
			case SLOW:
				cwnd *= 2;
				break;
			case CONV:
				cwnd += DSIZE;
				break;
			default:
				cout << "unknown transfor state :" << state << endl;
				return 0;
		}

		if(cwnd <= 0){
			cwnd = 1;
		}

		//try send
		printf("cwnd = %d, rwnd = %d, threshold = %d\n",cwnd,rwnd,THRESHOLD);
		nCur = cur;
		sndPkt.header.ackNum = ackNum;
		sndPkt.header.seqNum = seqNum - 1;
		for(int i = cwnd;i>0;){	//i=unsend in cwnd
			
			if(nCur >= fSize){	// all send
				break;
			}

			dSize = DSIZE;
			// send 1
			if((nCur + dSize) > fSize){
				dSize = fSize - nCur;
			}

			if(dSize > i){
				dSize = i;
			}
			i -= dSize;
			sndPkt.setHead(sPort,dPort, 0);
			sndPkt.header.ackNum += dSize;
			sndPkt.header.seqNum += 1;
			sndPkt.header.length(dSize);	//let client know size
			memset(sndPkt.data,0,sizeof(sndPkt.data));
			fseek(fp, nCur, SEEK_SET);
			fread( sndPkt.data, sizeof(char), dSize , fp);

			if(sendto(fd, (char*)&sndPkt, sizeof(sndPkt), 0,( struct sockaddr*)&outPort, addrSize) < 0){
				FAIL("send file fail\n");
			}
			printf("\tSend a packet at : %d byte\n",nCur);
			
		//try to send 2
			nCur += dSize;
			if((nCur < fSize) && (i > 0)){
				if((nCur + dSize) > fSize){
					dSize = fSize - nCur;
				}
				if(dSize > i){
					dSize = i;
					
				}
				i -= dSize;
				sndPkt.setHead(sPort,dPort, 0);
				sndPkt.header.ackNum += dSize;
				sndPkt.header.seqNum += 1;
				sndPkt.header.length(dSize);
				memset(sndPkt.data,0,sizeof(sndPkt.data));
				fseek(fp,nCur, SEEK_SET);
				fread( sndPkt.data, sizeof(char), dSize , fp);

				if(sendto(fd, (char*)&sndPkt, sizeof(sndPkt), 0, (struct sockaddr*)&outPort, addrSize) < 0){
					FAIL("send file fail\n");
				}
				printf("\tSend a packet at : %d byte\n",nCur);
				nCur += dSize;
				
			}
		}
		//wait ACK
		while(1){
			setTimeOut(0, RTT);//time out for RTT microseconds
			if(recvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
				//usleep(10000);
				break;
			}

			if(ackNum >= rcvPkt.header.seqNum){
				cout << "duplicate ACK " << rcvPkt.header.seqNum << endl;
				break; // receive old packet ,drop
			}

			if(rcvPkt.header.ACK()){
				//printf("Receive a ACK from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
				printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n",rcvPkt.header.seqNum,seqNum = rcvPkt.header.ackNum);
				//data send accessfully
				cur += (rcvPkt.header.seqNum - ackNum);
				ackNum = rcvPkt.header.seqNum;
				rwnd = rcvPkt.header.rwnd;
			}
			else{
				cout << "unexpected flag " << rcvPkt.header.flag() << endl;
				printf("Receive a unexpected from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
				printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n", rcvPkt.header.seqNum, rcvPkt.header.ackNum);\
				cur += 0; //data send non accessfully
			}
			//usleep(100);
		}
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
	int dSize = DSIZE;	// data size
	DATA fDes;	//descript the file size

	fp = fopen(file.c_str(),"wb");
	if(!fp){
		FAIL("cannot write the file\n");
	}
	//get file size
	
	cout << "Receive a file from " << inet_ntoa(outPort.sin_addr) << " : " << dPort << endl;
	for(int cur = 0; cur < fSize; ){
		dSize = DSIZE;
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
			case 0:	//receice file
				dSize = rcvPkt.header.ackNum - seqNum;

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
		

		rwnd -= rcvPkt.header.length();
		if(rwnd < 0){
			rwnd = RECV_BUF;	//release
		}

		//****//
		//delay ACK
		if(cur < fSize){	//have not get full file
		//try to receive packt 2
			if((cur + dSize) > fSize){
				dSize = fSize - cur;
			}
			setTimeOut(0,DELAY);//time out for DELAY microseconds		
			if(recvfrom(fd, &rcvPkt, sizeof(rcvPkt), 0, (struct sockaddr*)&outPort, &addrSize) < 0){
				;	//not receive packet 2
			}
			else if(ackNum >= rcvPkt.header.seqNum){
				 // receive old packet ,drop
			}
			else{	// receive packet 2
				printf("Receive a packet from %s : %d\n",inet_ntoa(outPort.sin_addr),rcvPkt.header.sPort);
				printf("\tReceive a packet (seq_num = %d, ack_num = %d)\n", rcvPkt.header.seqNum, rcvPkt.header.ackNum);
	
				if(rcvPkt.header.flag() == 0){
					dSize = rcvPkt.header.ackNum - seqNum;

					fseek(fp, cur, SEEK_SET);
					fwrite(rcvPkt.data, sizeof(char), dSize, fp);
					cur += dSize; //data receive accessfully
					sndPkt.setHead(sPort,dPort, ACK_);
					ackNum = rcvPkt.header.seqNum;
					seqNum = rcvPkt.header.ackNum;

					rwnd -= rcvPkt.header.length();
					if(rwnd < 0){
						rwnd = RECV_BUF;	//release
					}
				}
			}
		}
		//****//

		sndPkt.header.ackNum = ackNum+1;
		sndPkt.header.seqNum = seqNum;
		sndPkt.header.rwnd = rwnd;
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
