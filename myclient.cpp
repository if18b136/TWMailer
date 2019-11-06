#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
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

string uppercase(string str){
	for(unsigned int i = 0; i < str.length(); ++i) {
    	str[i] = toupper(str[i]);
	}
	return str;
}

int getch() {
    int ch;
    struct termios t_old, t_new;

    tcgetattr(STDIN_FILENO, &t_old);
    t_new = t_old;
    t_new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
    return ch;
}


string getpass(const char *prompt, bool show_asterisk=true)
{
  const char BACKSPACE=127;
  const char RETURN=10;

  string password;
  unsigned char ch=0;

  cout <<prompt<<endl;

  while((ch=getch())!=RETURN)
    {
       if(ch==BACKSPACE)
         {
            if(password.length()!=0)
              {
                 if(show_asterisk)
                 cout <<"\b \b";
                 password.resize(password.length()-1);
              }
         }
       else
         {
             password+=ch;
             if(show_asterisk)
                 cout <<'*';
         }
    }
  cout <<endl;
  return password;
}

int main (int argc, char **argv) {
	int create_socket;    // Integer for socket creation and it's error handling
	char buffer[BUF], input[BUF]; // Character Arrays, buffer is the final datastream that gets send to the server, input is used for input, concatenation and copies
	struct sockaddr_in address; //socket struct - base connection
	int size; // Integer for message receiving and it's error handling
	uint32_t port_short;  // unsigned 32bit integer - socket struct changes this short into hex for port connection
	bool overload = false, logged_in = false;  // boolean for input size overload check
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
		size=recv(create_socket,buffer,BUF-1, 0);
		if (size>0){
			buffer[size]= '\0';

			cout << buffer << endl;
			// end connection here early if user gets blocked

			//cout << "buff: "<< buffer[0] << endl;
			if(buffer[0] == 'I'){
				close (create_socket);
				return EXIT_SUCCESS;
			}
			clear_buffer(buffer);
		}
		printf ("Connection with server (%s) established\n", inet_ntoa (address.sin_addr));
	}
	else{
		perror("Connect error - no server available");
		return EXIT_FAILURE;
	}

	// input after successful connection
	do{
		while(input_str != "QUIT"){
			overload = false;  // set overload to false initially for any case of unexpected operation cancelling
			cout << endl;
			cout << "------------------------------------" << endl;
			cout << "| LOGIN | SEND | LIST | READ | DEL |" << endl;
			cout << "------------------------------------" << endl;
			cout << endl;
			cout << "Enter your command: "<< endl;

			getline(cin, input_str);	// get input from user
			input_str = uppercase(input_str);	// parse input to uppercase
			
			if(input_str == "LOGIN"){
				input_str += "\n"; // add newline
				strncpy(buffer,input_str.c_str(),BUF);

				// username input
				while(!overload){
					cout << "UID: ";
					getline(cin,input_str);
					if(input_str.length() > 8){
						cout << "input exceeds char limit. (max. 8 characters)" << endl;
					}
					else if(input_str.length() == 0){
						cout << "empty input not allowed." << endl;
					}
					else{
						overload = true; // gets set to true to break out of loop
						input_str += "\n";
						strcat(buffer,input_str.c_str());
					}
				}

				overload = false; // reset overload for password

				// password input
				while(!overload){
					input_str=getpass("Password: ",true);
					if(input_str.length() == 0){
						cout << "empty input not allowed." << endl;
					}
					else{
						overload = true; // gets set to true to break out of loop
						input_str += "\n";
						strcat(buffer,input_str.c_str());
					}
				}

				overload = false;

				send(create_socket, buffer, strlen (buffer), 0);
				clear_buffer(buffer); // clear buffer after sending message

				size=recv(create_socket,buffer,BUF-1, 0); //wait for answer from server
				if (size>0){
					buffer[size]= '\0';
					cout << buffer << endl;

					string token = strtok (buffer,"\n");
					if(token == "OK"){
						// set logged in boolean to true so other commands can be used
						logged_in = true;
					}
					else if(token == "ERR"){

						// let's check if we are already blocked
						size=recv(create_socket,buffer,BUF-1, 0);
						if (size > 0){

							buffer[size]= '\0';
							string exit = strtok (buffer,"\n");
							if (exit == "3"){
								cout << "You failed 3 times to log in - blocked for 2 Minutes." << endl;
								close (create_socket);
								return EXIT_SUCCESS;
							}
						}
						else{

							close (create_socket);
							return EXIT_SUCCESS;
						}
					}
					else{
						// should not occur but cathing it removes unwanted behaviour
					}
				}
			}

			//Command: SEND
			else if(input_str == "SEND"){
				if(!logged_in){
					cout << "Please log in first." << endl;
				}
				else{
					input_str += "\n"; // add newline
					strncpy(buffer,input_str.c_str(),BUF);

					// Sender input
					while(!overload){
						cout << "From: ";
						getline(cin,input_str);
						if(input_str.length() > 8){
							cout << "input exceeds char limit. (max. 8 characters)" <<  endl;
						}
						else if(input_str.length() == 0){
							cout << "empty input not allowed." << endl;
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
						cout << "To: ";
						getline(cin,input_str);
						if(input_str.length() > 8){
							cout << "input exceeds char limit. (max. 8 characters)" <<  endl;
						}
						else if(input_str.length() == 0){
							cout << "empty input not allowed." << endl;
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
						cout << "Subject: ";
						getline(cin,input_str);
						if(input_str.length() > 80){
							cout << "input exceeds char limit. (max. 80 characters)" <<  endl;
						}
						else if(input_str.length() == 0){
							cout << "empty input not allowed." << endl;
						}
						else{
							overload = true; // gets set to true to break out of loop
							input_str += "\n";
							strcat(buffer,input_str.c_str());
						}
					}

					overload = false;
					cout << "Message (a single '.' as input marks the end of the message): " << endl;
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
			}

			// LIST command, lists all subjects of a certain user
			else if(input_str == "LIST"){
				if(!logged_in){
					cout << "Please log in first." << endl;
				}
				else{
					input_str += "\n";
					strncpy(buffer,input_str.c_str(),BUF);

					// name input
					while(!overload){
						cout << "UID (From): ";
						getline(cin,input_str);
						if(input_str.length() > 8){
							cout << "input exceeds char limit. (max. 8 characters)" <<  endl;
						}
						else if(input_str.length() == 0){
							cout << "empty input not allowed." << endl;
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
			}

			// READ command, read a certain message of a certain user
			else if(input_str == "READ"){
				if(!logged_in){
					cout << "Please log in first." << endl;
				}
				else{
					input_str += "\n";
					strncpy(buffer,input_str.c_str(),BUF);

					// name input
					while(!overload){
						cout << "UID (From): ";
						getline(cin,input_str);
						if(input_str.length() > 8){
							cout << "input exceeds char limit. (max. 8 characters)<" <<  endl;
						}
						else if(input_str.length() == 0){
							cout << "empty input not allowed." << endl;
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
			}

			// DEL command, delete a certain message from a certain user file
			else if(input_str == "DEL"){
				if(!logged_in){
					cout << "Please log in first." << endl;
				}
				else{
					input_str += "\n";
					strncpy(buffer,input_str.c_str(),BUF);
				
					// name input
					while(!overload){
						cout << "UID (From): ";
						getline(cin,input_str);
						if(input_str.length() > 8){
							cout << "input exceeds char limit. (max. 8 characters)" <<  endl;
						}
						else if(input_str.length() == 0){
							cout << "empty input not allowed." << endl;
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
					clear_buffer(buffer);

					size=recv(create_socket,buffer,BUF-1, 0); //wait for answer from server
					if (size>0){
						buffer[size]= '\0';
						cout << buffer << endl;
						clear_buffer(buffer);
					}
				}
			}
			// catch empty commands
			else if(input_str == ""){} 
			else if(input_str == "QUIT"){
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
	while (strcmp (buffer, "QUIT\n") != 0);
	close (create_socket);
	return EXIT_SUCCESS;
}