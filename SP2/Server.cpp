#include "B043040044.h"

#define FILENAME "Video1.mp4"

int main(int argc, char* argv[]){
	signal(SIGINT, stopHandleA);
	srand(time(NULL));

	who = "Server";
	errno = 255;	//set the default error message for unknown
	string ip = "127.0.0.5" ;
	unsigned int port = 0;
	string file;
	int process = 1;	//process number

	if(argc < 2){
		DIE("too few argument, please type a interger port number\n");
	}
	if(sscanf(argv[1], "%d", &port) < 1){
		DIE("port number have to be a integer\n");
	}
		
	pConInfo(ip, port);
	tcpConnect con(ip, port);

	while(1){
		cout << "Process " << process << " : " << endl;
		if(con.waitCon()){
			if((pid = fork()) > 0){	//parent process
				process ++;
				con.freeCon();	//release connect
				continue;
			}
			for(;;){
				// in communicating
				cout << "Process " << process << " want to send file to " << con.getDAddr() << " : " << con.getDPort() << endl;
				//cin >> file;
				if(con.wReqF(file) < 0){
					DIE("wait request fail\n");
				}
				//file = FILENAME;
				cout << "Process " << process << " try to send the " << file << endl;
				if(con.sendF(file)){
					return 0;	// send success
				}
			}
		}
		else{
			DIE("waitCon() Error\n");
		}
	}
	return 0;
} 
