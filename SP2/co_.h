
//this .h is define how to connect & infomation
//fuction declare in con.cpp


#include "std.h"
#include "tcp.h"
//B043040044

#ifndef CON
#define CON

extern string who;	//the exec name

void pConInfo(string ip,unsigned short port); //print the connect infomation

class tcpConnect{
	private:
		struct timeval timeOut;	//time out range 
		string addr;	//ip address
		unsigned short sPort,dPort;	// port info
		int fd;			//socket file description
		FILE* fp;		// file pointer		
		struct sockaddr_in outPort;
		struct sockaddr_in inPort;
		unsigned int seqNum,ackNum;
		int cwnd;		//congestion window
		int rwnd;		//receive window
	public:
		tcpConnect(string addr, unsigned short sPort);
		~tcpConnect();
		void setTimeOut(int sec,int usec);
		bool tryCon(string dAddr, unsigned short dPort);
		bool waitCon();

		void freeCon();		//free connect to any connent
		bool sendF(string file);
		bool recvF(string file);

		int myRecvfrom(int sockfd, TCP *buf, int len, unsigned int flag, struct sockaddr *from, socklen_t *fromlen);  
		char* getDAddr();
		unsigned short getDPort();

		bool reqF(string file);	//request file
		bool wReqF(string& file);// wait request file
};

#endif
