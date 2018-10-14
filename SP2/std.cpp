#include "std.h"

int pid = -1; //to check is parent process

void stopHandle(int sig){
	cout << "\nSHUT DOWN\n";
	exit(0);
}

void stopHandleA(int sig){
	//kill(0, sig);
	if(pid!= 0){
		kill(0, sig);
		cout << "\nSHUT DOWN\n";
	}
	exit(0);
}
