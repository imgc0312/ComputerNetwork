#include "B043040044.h"

#define FILENAME "Video.mp4"

int main(int argc, char* argv[]){
	signal(SIGINT,stopHandle);
	srand(time(NULL));

	who = "Client";
	errno = 255;	//set the default error message for unknown
	string ip = "127.0.0.7" ;
	unsigned int port = 0;
	string dIP = "127.0.0.5";
	unsigned short dPort;
	string sfile,dfile;
	
	if(argc < 2){
		DIE("too few argument, please type a interger port number\n");
	}
	if(sscanf(argv[1], "%d", &port) < 1){
		DIE("port number have to be a integer\n");
	}
	pConInfo(ip, port);
	
	while(1){
		cout << "Please Input Server [IP] [Port] you want to connect:" << endl;
		cin >> dIP >> dPort;
	
		tcpConnect con(ip,port);
		if(con.tryCon(dIP, dPort)){
			for(;;){
				//in communicating
				cout << "want to receive file, please key the new file name" << endl;
				cin >> sfile;
				con.reqF(sfile);
				dfile = "";
				dfile = dfile + argv[1] + FILENAME;
				cout << "start receive " << dfile << endl;
				//cin >> file;
				if(con.recvF(dfile)){
				    return 0;   //recv success
                }
			}
			break;
		}
		else{
			cout << "Please try again" << endl;
		}
	}
	return 0;
} 
