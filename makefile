all: myclient myserver

client:
	g++ -o myclient myclient.cpp

server:
	g++ -o myserver myserver.cpp

clean:
	rm -f myclient myserver
