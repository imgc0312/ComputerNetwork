#include "B043040044.h"

int main(int argc, char* argv[]){
	signal(SIGINT,stopHandle);
	srand(time(NULL));

	who = "Server";
	errno = 255;	//set the default error message for unknown
	string ip = "127.0.0.5" ;
	unsigned int port = 0;
	string file;
	
	if(argc < 2){
		DIE("too few argument, please type a interger port number\n");
	}
	if(sscanf(argv[1], "%d", &port) < 1){
		DIE("port number have to be a integer\n");
	}
		
	pConInfo(ip, port);
	tcpConnect con(ip, port);
	if(con.waitCon()){
		for(;;){
			// in communicating
			cout << "want to send file, please key the file name" << endl;
			cin >> file;
			//file = "t.test";
			cout << "try to send the " << file << endl;
			con.sendF(file);

		}
	}
	else{
		DIE("waitCon() Error\n");
	}
	
	return 0;
} 
