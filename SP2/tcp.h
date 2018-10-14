
//this .h is define the TCP segment

#include "std.h"
//B043040044

#ifndef TCP_H
#define TCP_H

//mask for header op flag
#define URG_ 0x0020
#define ACK_ 0x0010
#define PSH_ 0x0008
#define RST_ 0x0004
#define SYN_ 0x0002
#define FIN_ 0x0001
#define OPFLAG_ 0x003F

//data max size
#define DSIZE MSS - sizeof(Head)

#define NAMEL 30	//filename max length

struct Head{	// TCP header
	unsigned short sPort = -1;	//source port number
	unsigned short dPort = -1;	//destination port number
	unsigned int seqNum = 0;	//sequence num
	unsigned int ackNum = 0;	//acknowledge num
	unsigned short op = 0;		/* op {
	0xF000			0~3	header length
	0x0FC0			4~9	option field
	0x0020			10	flag URG
	0x0010			11	flag ACK
	0x0008			12	flag PSH
	0x0004			13	flag RST
	0x0002			14	flag SYN
	0x0001			15	flag FIN
							get or modify by function
				}*/
	unsigned short rwnd = RECV_BUF;	//receive window
	unsigned short checkSum = 0;//check sum
	unsigned short urgPtr = 0;	//urgent data pointer field


	//get or modify value from op
	unsigned int length(){				//get header length
		return (op & 0xF000) >> 12;
	}
	unsigned int length(unsigned int a){//modify header length
		op = (op & ~(0xF000)) | ((a & 0x000F) << 12);
		return a & 0x000F;
	}
	unsigned int option(){				//get option field
		return (op & 0x0FC0) >> 6;
	}	
	unsigned int option(unsigned int a){//modify option field 
		op = (op & ~(0x0FC0)) | ((a & 0x003F) << 6);
		return (a & 0x003F);
	}
	bool URG(){							//get URG
		return op & URG_;
	}
	bool URG(bool a){					//set URG
		if(a)
			return op |= URG_;
		else
			return op &= ~(URG_);
	}
	bool ACK(){							//get ack
		return op & ACK_;
	}
	bool ACK(bool a){					//set ack
		if(a)
			return op |= ACK_;
		else
			return op &= ~(ACK_);
	}
	bool PSH(){							//get PSH
		return op & PSH_;
	}
	bool PSH(bool a){					//set PSH
		if(a)
			return op |= PSH_;
		else
			return op &= ~(PSH_);
	}
	bool RST(){							//get RST
		return op & RST_;
	}
	bool RST(bool a){					//set RST
		if(a)
			return (op |= RST_);
		else
			return op &= ~(RST_);
	}
	bool SYN(){							//get SYN
		return op & SYN_;
	}
	bool SYN(bool a){					//set SYN
		if(a)
			return op |= SYN_;
		else
			return op &= ~(SYN_);
	}
	bool FIN(){							//get FIN
		return op & FIN_;
	}
	bool FIN(bool a){					//set FIN
		if(a)
			return op |= FIN_;
		else
			return op &= ~(FIN_);
	}
	unsigned short flag(){	//get Flag
		return op & OPFLAG_;
	}
	unsigned short flag(unsigned short opFlag){	//set Flag
		opFlag &= OPFLAG_;	// get last 6bits for flag
		op &= ~(OPFLAG_);	// clear flag bits
		op |= opFlag;
		return opFlag;
	}

};

struct TCP{
	Head header;
	unsigned char data[DSIZE];
	TCP(){
		memset(data,0,sizeof(data));
	}
	void setHead(unsigned short sPort, unsigned short dPort,unsigned short opFlag);
};

union DATA{		//format the special data
	struct INFO{	//store some info about what to send
		char name[NAMEL];	//filename
		int size;	//target send size
	}info;
	unsigned char data[DSIZE];	//format size
};

void print(Head a);	// view header test





#endif
