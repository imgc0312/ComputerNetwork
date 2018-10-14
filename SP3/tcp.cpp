#include "tcp.h"
//this .cpp is declare function about the TCP segment

void TCP::setHead(unsigned short sPort, unsigned short dPort,unsigned short opFlag){
								// set port		
	header.sPort = sPort;
	header.dPort = dPort;
	header.flag(opFlag);		//set packet flag	
}

void print(Head a){	// view header test
	cout<<"=====Head=====\n";
	cout<<"sPort = "<< a.sPort << "\n";
	cout<<"dPort = "<< a.dPort <<"\n";
	cout<<"seq num = "<< a.seqNum << "\n";
	cout<<"ack num = "<< a.ackNum << "\n";
	cout<<"header length = "<< a.length() <<"\n";
	cout<<"option field = "<< a.option() <<"\n";
	cout<<"URG = "<< a.URG() <<"\n";
	cout<<"ACK = "<< a.ACK() <<"\n";
	cout<<"PSH = "<< a.PSH() <<"\n";
	cout<<"RST = "<< a.RST() <<"\n";
	cout<<"SYN = "<< a.SYN() <<"\n";
	cout<<"FIN = "<< a.FIN() <<"\n";
	cout<<"rwnd = "<< a.rwnd <<"\n";
	cout<<"check sum = "<< a.checkSum <<"\n";
	cout<<"urgPtr = "<< a.urgPtr <<"\n";
	cout<<"====================\n";

}

