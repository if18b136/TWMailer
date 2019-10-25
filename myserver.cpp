#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <string>

#include <pthread.h>
#include <ldap.h>

#define LDAP_URI "ldap://ldap.technikum-wien.at:389"
#define SEARCHBASE "dc=technikum-wien,dc=at"
#define SCOPE LDAP_SCOPE_SUBTREE
#define FILTER "(uid=if18b*)"
#define BIND_USER ""	/* anonymous bind with user and pw empty */
#define BIND_PW ""

#define BUF 1024
int THREAD_NUM = 0;
char *path_global;

// pthread_join();
using namespace std;

void clear_buffer(char *buffer){
	char *begin = &buffer[0];
	char *end = begin + sizeof(buffer);
	fill(begin,end,0);
}

void *test_thread(void *arg) { //needs the socket connection parameters as argunments


	int size, *socket_p, new_socket;
	char buffer[BUF];
	string path = path_global;

	socket_p = reinterpret_cast<int*>(arg);
	new_socket = * socket_p;

	cout << "Thread number: " << THREAD_NUM << endl;

	do {
			size = recv (new_socket, buffer, BUF-1, 0);
			if( size > 0){
				buffer[size] = '\0';
				string input_str, num_str, token, username, filename, command, line;  //various strings for input/output handling
				token = strtok (buffer,"\n");
				cout << "Command received: "<< token << endl;
				username = strtok (NULL,"\n");
				filename = username + ".txt";
				
				//LOGIN command on server
				if(token == "LOGIN"){
					string password = strtok(NULL,"\n");

					LDAP *ld;			/* LDAP resource handle */
					LDAPMessage *result, *e;	/* LDAP result handle */
					BerElement *ber;		/* array of attributes */
					char *attribute;
					BerValue **vals;

					BerValue *servercredp;
					BerValue cred;
					cred.bv_val = (char *)BIND_PW;
					cred.bv_len=strlen(BIND_PW);
					int i,rc=0;

					const char *attribs[] = { "uid", "cn", NULL };		/* attribute array for search */

					int ldapversion = LDAP_VERSION3;

					/* setup LDAP connection */
					if (ldap_initialize(&ld,LDAP_URI) != LDAP_SUCCESS){
						fprintf(stderr,"ldap_init failed");
						return 0; // former EXIT_FAILURE
					}

					printf("connected to LDAP server %s\n",LDAP_URI);

					if ((rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldapversion)) != LDAP_SUCCESS){
						fprintf(stderr, "ldap_set_option(PROTOCOL_VERSION): %s\n", ldap_err2string(rc));
						ldap_unbind_ext_s(ld, NULL, NULL);
						return 0; // former EXIT_FAILURE
					}

					if ((rc = ldap_start_tls_s(ld, NULL, NULL)) != LDAP_SUCCESS){
						fprintf(stderr, "ldap_start_tls_s(): %s\n", ldap_err2string(rc));
						ldap_unbind_ext_s(ld, NULL, NULL);
						return 0; // former EXIT_FAILURE
					}

					/* anonymous bind */
					rc = ldap_sasl_bind_s(ld,BIND_USER,LDAP_SASL_SIMPLE,&cred,NULL,NULL,&servercredp);

					if (rc != LDAP_SUCCESS){
						fprintf(stderr,"LDAP bind error: %s\n",ldap_err2string(rc));
						ldap_unbind_ext_s(ld, NULL, NULL);
						return 0; // former EXIT_FAILURE
					}
					else{
						printf("bind successful\n");
					}

					/* perform ldap search */
					rc = ldap_search_ext_s(ld, SEARCHBASE, SCOPE, FILTER, (char **)attribs, 0, NULL, NULL, NULL, 500, &result);

					if (rc != LDAP_SUCCESS){
						fprintf(stderr,"LDAP search error: %s\n",ldap_err2string(rc));
						ldap_unbind_ext_s(ld, NULL, NULL);
						return 0; // former EXIT_FAILURE
					}

					printf("Total results: %d\n", ldap_count_entries(ld, result));



					// // OUTPUT for all user details
					// for (e = ldap_first_entry(ld, result); e != NULL; e = ldap_next_entry(ld,e))
					// {
					//    printf("DN: %s\n", ldap_get_dn(ld,e));

					//    /* Now print the attributes and values of each found entry */

					//    for (attribute = ldap_first_attribute(ld,e,&ber); attribute!=NULL; attribute = ldap_next_attribute(ld,e,ber))
					//    {
					//       if ((vals=ldap_get_values_len(ld,e,attribute)) != NULL)
					//       {
					//          for (i=0;i < ldap_count_values_len(vals);i++)
					//          {
					//             printf("\t%s: %s\n",attribute,vals[i]->bv_val);
					//          }
					//          ldap_value_free_len(vals);
					//       }
					//       /* free memory used to store the attribute */
					//       ldap_memfree(attribute);
					//    }

					//    /* free memory used to store the value structure */
					//    if (ber != NULL) ber_free(ber,0);

					//    printf("\n");
					// }


					//##########################################
					//User ID compared to LDAP Database (LOGIN)
					//##########################################

					bool overload = false;	//flag for the input

					bool user_found = false;
					string input_uid = username;	//user input
					string cmp_uid = "";	//"buffer" for cutting/comparing database entries
					string dn_uid = "";


					//iterates ldap entries until next entry is NULL (last entry)
					for(e = ldap_first_entry(ld, result); e != NULL; e = ldap_next_entry(ld,e)){

						// long DN sausage
						cmp_uid = ldap_get_dn(ld,e);

						// DN sausage is being cut into pieces -> "if18b***"
						cmp_uid = cmp_uid.substr(4,8);

						// cout << "database uid" << cmp_uid << endl;		//debug
						// cout << "input uid" << input_uid << endl;		//debug

						//compares entered UID to IDs in the database
						if(strncmp(input_uid.c_str(), cmp_uid.c_str(), 8)==0){

							cout << "bingo" << endl;	//username was found!
							dn_uid = ldap_get_dn(ld,e); //full user id sausage

							cout << dn_uid << endl;

							strncpy (buffer,"OK\n", BUF);
							send(new_socket, buffer, strlen(buffer),0);
							clear_buffer(buffer);

							user_found = true;


							// User gets logged into LDAP

							break;

						}
					}

					//checks if user was found or not
					if(user_found == false){

						cout << "user was not found!" << endl;
						strncpy (buffer,"ERR\n", BUF);
						send(new_socket, buffer, strlen(buffer),0);
						clear_buffer(buffer);

					}

					/* free memory used for result */
					ldap_msgfree(result);
					printf("LDAP search suceeded\n");

					ldap_unbind_ext_s(ld, NULL, NULL);

					
				}
				// SEND command on server side
				else if(token == "SEND"){
					cout << "Message received from: " << username << endl;
					ofstream outfile;
					string path_file = path + "/" + filename;
					outfile.open(path_file,ios_base::app);
					ifstream file(path_file);

					int msg_count = 1;

					// check if file is emty by looking for a "1" in the first line
					getline(file, line);
					if(line != "1"){
						outfile << "1" << endl;
						cout << "new file created" << endl;
					}
					else{
						// Count total number of messages in a file
						while(getline(file, line)){
							if(line == "."){
								msg_count++; // increase for every message end => "."
							}
						}
						outfile << msg_count << endl;  //include new message number in file
					}
					while(token != "."){  // tokenize buffer and put into file until end of message
						token = strtok(NULL,"\n");
						outfile << token << endl;
					}
					outfile.close();
				}
				// LIST command on Server
				else if(token == "LIST"){
					bool new_msg = true;  //starts with true so that first subject gets added in while loop
					ofstream outfile;
					cout << "looking for messages from: " << username << endl;
					string path_file = path + "/" + filename;
					outfile.open(path_file,ios_base::app);
					ifstream file(path_file);

					getline(file, line);
					if(line != "1"){
						clear_buffer(buffer);
						strncpy (buffer,"0\n", BUF);
						send(new_socket, buffer, strlen(buffer),0);
						clear_buffer(buffer);
					}
					else{
						int msg_count = 1; // starts at one so that first message subject gets added in while loop
						num_str = line;// first message is done extra
						num_str.append("\n");

						while(getline(file,line)){
							if(new_msg){
								//msg_count = stoi(line);
								if(msg_count % 2 == 0){ // get number
									num_str = line + "\n";
								}
								else{
									getline(file,line);
									input_str.append(line);
									input_str.append("\n");
									new_msg = false;  // after adding the subject we go hunting for new messages again
								}
								msg_count++;
							}
							if(line == "."){
								new_msg = true; //prepare for adding num and subject in the next iterations;
							}
						}
						outfile.close();
						num_str.append(input_str);
						strncpy (buffer,num_str.c_str(), BUF);
						send(new_socket, buffer, strlen(buffer),0);
						clear_buffer(buffer);
						input_str = ""; // clear input string for reuse in READ
					}
				}
				// READ command on Server
				else if (token == "READ"){
					bool found_msg = false, no_msg = true, recv_subj = true;
					cout << "looking for certain message from: " << username << endl;
					ofstream outfile;
					string path_file = path + "/" + filename;
					outfile.open(path_file,ios_base::app);
					ifstream file(path_file);
					string read = strtok(NULL,"\n");

					getline(file, line);
					if(line != "1"){
						strncpy (buffer,"ERR\n", BUF);
						send(new_socket, buffer, strlen(buffer),0);
						clear_buffer(buffer);
					}
					else{
						do{
							if(found_msg){
								if(line == "."){
									strncpy (buffer,input_str.c_str(), BUF);
									cout << "buffer: <" << buffer << ">" << endl;
									send(new_socket, buffer, strlen(buffer),0);
									clear_buffer(buffer);
									found_msg = false;
									no_msg = false;
								}
								if(recv_subj){ //lazy way to skip receiver and subject once
									getline(file,line);
									getline(file,line);
									recv_subj = false;
								}
								input_str.append(line);
								input_str.append("\n");
							}
							if(line == read){
								found_msg = true;
							}
						}while(getline(file,line));
						if(no_msg){
							strncpy (buffer,"ERR", BUF);
							send(new_socket, buffer, strlen(buffer),0);
							clear_buffer(buffer);
						}
					}
				}
				//DEL command on Server
				else if (token == "DEL"){
					bool found_msg = false, del_msg = false;
					input_str = "";
					ofstream outfile;
					cout << "looking for certain message to delete from: " << username << endl;
					string path_file = path + "/" + filename;
					outfile.open(path_file,ios_base::app);
					ifstream file(path_file);
					string del = strtok(NULL,"\n");

					getline(file, line);
					if(line != "1"){
						strncpy (buffer,"ERR\n", BUF);
						send(new_socket, buffer, strlen(buffer),0);
						clear_buffer(buffer);
					}
					else{
						string filename_temp = path + "/temp_" + filename;
						ifstream temp_file(filename_temp);
						ofstream outfile_temp;
						outfile_temp.open(filename_temp,ios_base::app);
						if(line == del){
							found_msg = true;
						}
						else{
						outfile_temp << line << endl;	//get first line into temp file 
						}
						while(getline(file,line)){
							if(line == del){
								found_msg = true;
							}
							if(!found_msg){	// copy lines as long as we haven't reached our message
								outfile_temp << line << endl;
							}
							if(line == "." && found_msg){
								found_msg = false;
								del_msg = true;
							}
						}
						outfile.close();
						remove(path_file.c_str());
						rename(filename_temp.c_str(),path_file.c_str());
						outfile_temp.close();
						// delete old file and rename new one
						if(!del_msg){
							strncpy (buffer,"ERR\n", BUF);
							send(new_socket, buffer, strlen(buffer),0);
							clear_buffer(buffer);
						}
						else{
							strncpy (buffer,"OK\n", BUF);
							send(new_socket, buffer, strlen(buffer),0);
							clear_buffer(buffer);
						}
					}
				}
			}
			else if (size == 0){	// fehlermeldungen checken
				printf("Client closed remote socket\n");
				break;
			}
			else{
				perror("recv error");
				return 0; // former EXIT_FAILURE
			}
		} while (strncmp (buffer, "quit", 4)  != 0);
		close(new_socket);
		pthread_exit(NULL);
}



int main (int argc, char **argv) {

	if( argc < 3 ){
		cout << "Usage: " << argv[0] << " Port Verzeichnispfad" << endl;
		exit(EXIT_FAILURE);
	}

	int create_socket, new_socket, status;
	socklen_t addrlen;
	char buffer[BUF];
	int num;
	struct sockaddr_in address, cliaddress;
	uint32_t port_short;

	// thread initialize
	pthread_t thread;

	port_short = atoi(argv[1]); //port has to be 4 numbers long
	path_global = argv[2];
	
	create_socket = socket (AF_INET, SOCK_STREAM, 0); //get socket file descriptor
	if (create_socket < 0) {  //check if socket was created succesfully
		perror("ERROR opening socket");
		return EXIT_FAILURE;
	}


	memset(&address,0,sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons (port_short);

	if (bind ( create_socket, (struct sockaddr *) &address, sizeof (address)) != 0) { // descriptor und bind adresse designiert, wo ich den socket gerne haben moechte, man erhaelt einen offenen socket
		perror("bind error");
		return EXIT_FAILURE;
	}
	listen (create_socket, 5); // gibt int zurueck, wenn es funktioniert hat, eine fehlermeldung, wenn fehler

	addrlen = sizeof (struct sockaddr_in);

	while (1) {
		printf("Waiting for connections...\n");
		
		new_socket = accept ( create_socket, (struct sockaddr *) &cliaddress, &addrlen ); // blockiert und wartet bis client antwortet.
	
		if (new_socket > 0){  // client verbunden
			printf ("Client connected from %s:%d...\n", inet_ntoa (cliaddress.sin_addr),ntohs(cliaddress.sin_port));
			strcpy(buffer,"Welcome to myserver!\n");	// welcome message
			send(new_socket, buffer, strlen(buffer),0);
			clear_buffer(buffer);

			status = pthread_create(&thread, NULL, test_thread, (void *) &new_socket); // after client connects successfully, we need to open a thread for the client
			THREAD_NUM++;
		}
		//close (new_socket);
	}
	close (create_socket);
	pthread_join;
	return EXIT_SUCCESS;
}