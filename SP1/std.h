
//this .h is include what standard lib to use and some common value
#ifndef STD
#define STD

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
using namespace std;

// the connect environment value
#define RTT 200			//200 ms
#define MSS 1024		//1024 B
#define THRESHOLD 65535
#define RECV_BUF ( 32 * 1024 ) //buffer size 32KB

#define FAIL(x) perror(x);return 0
#define DIE(x) perror(x),exit(0)
void stopHandle(int sig);
#endif
