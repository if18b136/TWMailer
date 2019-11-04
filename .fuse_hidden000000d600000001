# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -Wfatal-errors -pedantic-errors

all:	myclient myserver

make:
	rm -f *.zip myclient myserver
	find . -name "*.o" -type f -delete

	g++ -std=c++11 myclient.cpp -o	myclient
	g++ -std=c++11 myserver.cpp -o	myserver -lpthread -lldap -llber

myclient:
	g++ -std=c++11 myclient.cpp -o	myclient

myserver:
	g++ -std=c++11 myserver.cpp -o	myserver -lpthread -lldap -llber

clean:
	rm -f *.zip myclient myserver 
	find . -name "*.o" -type f -delete