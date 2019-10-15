/* myclient.c */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define BUF 1024

#include <iostream>
#include <cstdlib>
#include <string>
using namespace std;

void clear_buffer(char *buffer){
	char *begin = &buffer[0];
	char *end = begin + sizeof(buffer);
	fill(begin,end,0);
}

int main (int argc, char **argv) {
	int create_socket;    // Integer for socket creation and it's error handling
	char buffer[BUF], input[BUF]; // Character Arrays, buffer is the final datastream that gets send to the server, input is used for input, concatenation and copies
	struct sockaddr_in address; //socket struct - base connection
	int size; // Integer for message receiving and it's error handling
	uint32_t port_short;  // unsigned 32bit integer - socket struct changes this short into hex for port connection
	bool overload = false;  // boolean for input size overload check
	string input_str;

	// check for required amount of command line arguments
	if( argc < 3 ){
	cout << "Usage: " << argv[0] << " ServerAdresse Port" << endl;
		exit(EXIT_FAILURE);
	}

	// Exit with error message
	if ((create_socket = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Socket error");
		return EXIT_FAILURE;
	}

	// After successful socket creation get port number
	port_short = atoi(argv[2]);

	// initialise socket
	memset(&address,0,sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons (port_short); // byte order - host to network short
	inet_aton (argv[1], &address.sin_addr);

	// connection try, success and error messages
	if (connect ( create_socket, (struct sockaddr *) &address, sizeof (address)) == 0){
		printf ("Connection with server (%s) established\n", inet_ntoa (address.sin_addr));
		size=recv(create_socket,buffer,BUF-1, 0);
		if (size>0){
			buffer[size]= '\0';
			cout << buffer << endl;
			clear_buffer(buffer);
		}
	}
	else{
		perror("Connect error - no server available");
		return EXIT_FAILURE;
	}

	// input after successful connection
	do{
		while(input_str != "quit"){
			overload = false;  // set overload to false initially for any case of unexpected operation cancelling
			cout << "Enter your command: "<< endl;
			//fgets (str, BUF, stdin);
			getline(cin, input_str);

			if(input_str == "SEND"){ //two times in a row exceeds char limit - clean input after successful send
				input_str += "\n"; // add newline
				strncpy(buffer,input_str.c_str(),BUF);

				// Sender input
				while(!overload){
					getline(cin,input_str);
					if(input_str.length() > 8){
						cout << "input exceeds char limit. (max. 8 characters)<" << input_str << ">"<<  endl;
					}
					else{
						overload = true; // gets set to true to break out of loop
						input_str += "\n";
						strcat(buffer,input_str.c_str());
					}
				}

				overload = false; // reset overload for receiver

				// receiver input
				while(!overload){
					getline(cin,input_str);
					if(input_str.length() > 8){
						cout << "input exceeds char limit. (max. 8 characters) <" << input_str << ">"<<  endl;
					}
					else{
						overload = true; // gets set to true to break out of loop
						input_str += "\n";
						strcat(buffer,input_str.c_str());
					}
				}

				overload = false;

				//subject input
				while(!overload){
					getline(cin,input_str);
					if(input_str.length() > 80){
						cout << "input exceeds char limit. (max. 80 characters) <" << input_str << ">"<<  endl;
					}
					else{
						overload = true; // gets set to true to break out of loop
						input_str += "\n";
						strcat(buffer,input_str.c_str());
					}
				}

				overload = false;

				getline(cin,input_str);  //get first line of the text message to prevent ending when someone puts "." as subject
				// message input
				while(input_str != "."){
					input_str += "\n";
					strcat(buffer,input_str.c_str());
					getline(cin,input_str);
				}

				// copy ".\n" into buffer too, then send it to server
				input_str += "\n";
				strcat(buffer,input_str.c_str());
				send(create_socket, buffer, strlen (buffer), 0);
				clear_buffer(buffer); // clear buffer after sending message
			}

			// LIST command, lists all subjects of a certain user
			else if(input_str == "LIST"){
				input_str += "\n";
				strncpy(buffer,input_str.c_str(),BUF);

				// name input
				while(!overload){
					getline(cin,input_str);
					if(input_str.length() > 8){
						cout << "input exceeds char limit. (max. 8 characters)<" <<  endl;
					}
					else{
						overload = true; // gets set to true to break out of loop
						input_str += "\n";
						strcat(buffer,input_str.c_str());
					}
				}

				overload = false; // reset overload for next input check

				send(create_socket, buffer, strlen (buffer), 0); // send user request
				clear_buffer(buffer);

				//wait for answer from server
				size=recv(create_socket,buffer,BUF-1, 0);
				if (size>0){
					buffer[size]= '\0';
					cout << buffer << endl;
					clear_buffer(buffer);
				}
			}

			// READ command, read a certain message of a certain user
			else if(input_str == "READ"){
				input_str += "\n";
				strncpy(buffer,input_str.c_str(),BUF);

				// name input
				while(!overload){
					getline(cin,input_str);
					if(input_str.length() > 8){
						cout << "input exceeds char limit. (max. 8 characters)<" <<  endl;
					}
					else{
						overload = true; // gets set to true to break out of loop
						input_str += "\n";
						strcat(buffer,input_str.c_str());
					}
				}

				overload = false; // reset overload for next input check

				// let's trust the user for once and assume he knows the difference between a number and not a number
				getline(cin, input_str); // simply cin >> input_str takes the command line enter as another input
				input_str += "\n";
				strcat(buffer,input_str.c_str());
				send(create_socket, buffer, strlen (buffer), 0); // send user request

				size=recv(create_socket,buffer,BUF-1, 0); //wait for answer from server
				if (size>0){
					buffer[size]= '\0';
					cout << buffer << endl;
					clear_buffer(buffer);
				}
			}

			// DEL command, delete a certain message from a certain user file
			else if(input_str == "DEL"){
				input_str += "\n";
				strncpy(buffer,input_str.c_str(),BUF);
			
				// name input
				while(!overload){
					getline(cin,input_str);
					if(input_str.length() > 8){
						cout << "input exceeds char limit. (max. 8 characters)<" <<  endl;
					}
					else{
						overload = true; // gets set to true to break out of loop
						input_str += "\n";
						strcat(buffer,input_str.c_str());
					}
				}

				overload = false; // reset overload for next input check

				// let's trust the user for once and assume he knows the difference between a number and not a number
				getline(cin, input_str); // cin >> input_str takes the command line enter as another input
				input_str += "\n";
				strcat(buffer,input_str.c_str());
				send(create_socket, buffer, strlen (buffer), 0); // send user request

				size=recv(create_socket,buffer,BUF-1, 0); //wait for answer from server
				if (size>0){
					buffer[size]= '\0';
					cout << buffer << endl;
					clear_buffer(buffer);
				}
			}
			// catch empty commands
			else if(input_str == ""){} 
			else if(input_str == "quit"){
				input_str += "\n";
				strncpy(buffer,input_str.c_str(),BUF);
				break;
			}
			// all other command inputs won't be accepted
			else{
				cout << "Unknown command, please repeat." << endl;
			}
		}
	}
	while (strcmp (buffer, "quit\n") != 0);
	close (create_socket);
	return EXIT_SUCCESS;
}


// TODO
// dyn buffer
// read versucht zu frueh wieder auf command input zu wechseln, bekommt ne leerzeile
// del  programmieren
