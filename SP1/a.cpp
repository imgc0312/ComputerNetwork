#include "B043040044.h"

int main(){
	Head x;
	cout << sizeof(x) << endl;
	print(x);
	x.sPort = 1;
	x.dPort = 2;
	x.seqNum = 10;
	x.ackNum = 15;
	x.op = 0xFFFF;
	x.rwnd = 1024;
	x.checkSum = 3;
	x.urgPtr = 9;
	x.length(10);
	x.flag(ACK_ | SYN_);
	print(x);
	
	return 0;
}
