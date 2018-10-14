PROGS = Server Client
O = sp2.o con.o tcp.o std.o
C = g++
option = -g -Wall -std=c++11

all: $(PROGS) clean 

Server: Server.o $(O)
	$(C) $^ $(option) -o $@

Client: Client.o $(O)
	$(C) $^ $(option) -o $@

%.o:%.cpp
	$(C) $(option) -c $<

clean :
	rm -f *.o 

remove: clean
	rm $(PROGS) 
