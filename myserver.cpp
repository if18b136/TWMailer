/* myserver.c */
// es fehlen fehlerabfragen und string buffer ueberschreibungsschutz
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
#define BUF 1024
#define PORT 6543

using namespace std;

void clear_buffer(char *buffer){
  char *begin = &buffer[0];
  char *end = begin + sizeof(buffer);
  fill(begin,end,0);
}

int main (int argc, char **argv) {

  if( argc < 3 ){
    cout << "Usage: " << argv[0] << " Port Verzeichnispfad" << endl;
     exit(EXIT_FAILURE);
  }

  int create_socket, new_socket;
  socklen_t addrlen;
  char buffer[BUF];
  int size, num;
  struct sockaddr_in address, cliaddress;
  uint32_t port_short;

  port_short = atoi(argv[1]); //port has to be 4 numbers long

  while (1) {
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
    listen (create_socket, 5); // gibt int zurueck, wenn es funktioniert hat, eine fehlermeldung, wenn fehler - noch nicht ganz sauber ausprogrammiert

    addrlen = sizeof (struct sockaddr_in);
     printf("Waiting for connections...\n");
     new_socket = accept ( create_socket, (struct sockaddr *) &cliaddress, &addrlen ); // blockiert und wartet bis client antwortet.

     if (new_socket > 0){  // client verbunden
        printf ("Client connected from %s:%d...\n", inet_ntoa (cliaddress.sin_addr),ntohs(cliaddress.sin_port));
        strcpy(buffer,"Welcome to myserver!\n");	// welcome message
        send(new_socket, buffer, strlen(buffer),0);
        clear_buffer(buffer);
        close (create_socket);
     }
     do {
        size = recv (new_socket, buffer, BUF-1, 0);
        if( size > 0){
           buffer[size] = '\0';
           string input_str, num_str, token, username, filename, command, line;  //various strings for input/output handling
           token = strtok (buffer,"\n");
           cout << "Command received: "<< token << endl;
           username = strtok (NULL,"\n");
           filename = username + ".txt";
           cout << "Token: " << token << endl;
           // SEND command on server side
           if(token == "SEND"){
             cout << "Message received from: " << username << endl;
             ofstream outfile;
             outfile.open(filename,ios_base::app);
             ifstream file(filename);

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
           else if(token == "LIST"){
            bool new_msg = true;  //starts with true so that first subject gets added in while loop
            ofstream outfile;
            ifstream file(filename);
            cout << "looking for messages from: " << username << endl;
            outfile.open(filename,ios_base::app);

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
              num_str.append(input_str);
              strncpy (buffer,num_str.c_str(), BUF);
              send(new_socket, buffer, strlen(buffer),0);
              clear_buffer(buffer);
              input_str = ""; // clear input string for reuse in READ
             }
           }
           else if (token == "READ"){
             bool found_msg = false, no_msg = true, recv_subj = true;
             ofstream outfile;
             ifstream file(filename);
             cout << "looking for certain message from: " << username << endl;
             outfile.open(filename,ios_base::app);
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
                   cout << "found message" << endl;
                   if(line == "."){
                     strncpy (buffer,input_str.c_str(), BUF);
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
           else if (token == "DEL"){
             ofstream outfile;
             ifstream file(filename);
             cout << "looking for certain message to delete from: " << username << endl;
             outfile.open(filename,ios_base::app);
             token = strtok(NULL,"\n");

             getline(file, line);
             if(line != "1"){
               strncpy (buffer,"ERR\n", BUF);
               send(new_socket, buffer, strlen(buffer),0);
               clear_buffer(buffer);
             }else{
               string filename_temp = "temp_" + filename;
               ofstream of_tempfile;
               ifstream tempfile(filename_temp);
             }


            /*ofstream outfile;
            ifstream file(username);
            int message_count = 1;
            string line;
            string message_nr_str = strtok (NULL,"\n");
            int message_nr = stoi(message_nr_str ,nullptr);
            cout << "message Nr" << message_nr << endl;

            outfile.open(username, ios_base::app);
            cout << "File openeded" << endl;
            getline(file,line);
            if(line!="1"){
              cout << "no 1 here?" << endl;
              char *begin = &buffer[0];
              char *end = begin + sizeof(buffer);
              fill(begin,end,0);
              strcpy(buffer, "ERR");
              send(new_socket, buffer, strlen(buffer),0);
            }
            else{
              while(getline(file, line)){
                cout << "line gotten.." << endl;
                if(line == "."){
                  char *begin = &buffer[0];
                  char *end = begin + sizeof(buffer);
                  fill(begin,end,0);
                  cout << "end of first msg found" << endl;
                  message_count++;
                  cout << "msg_count: " << message_count <<endl << "message_nr: " << message_nr<< endl;
                  if(message_count == message_nr){
                    getline(file, line);
                    cout << "message found" << endl;
                  }

                }
                else if(file.eof()){
                  cout << "EOF.. nothing found"<< endl;
                  strcpy(buffer, "ERR");
                  send(new_socket, buffer, strlen(buffer),0);
                  outfile.close();
                  break;
                }
                if (line == "SEND" && message_count == message_nr){
                  cout << "deleting.."<<endl;
                  char *begin = &buffer[0];
                  char *end = begin + sizeof(buffer);
                  fill(begin,end,0);

                  string ok = "OK\n";
                  strcat(buffer, ok.c_str());
                  send(new_socket, buffer, strlen(buffer),0);
                  outfile.close();
                  message_count = 0;

                  outfile.close();
                  break;


                  /*while(getline(file, line)){
                    if(line != "."){
                       line.append("\n");
                       strcat(buffer, line.c_str());
                    }
                    else{

                      outfile.close();
                      message_count = 0;
                      break;
                    }
                  }
                }
              }
            }
           }*/
           //else{
             //error
           }
        }
        else if (size == 0) // fehlermeldungen checken
        {
           printf("Client closed remote socket\n");
           break;
        }
        else
        {
           perror("recv error");
           return EXIT_FAILURE;
        }
     } while (strncmp (buffer, "quit", 4)  != 0);
     close (new_socket);
  }
  close (create_socket);
  return EXIT_SUCCESS;
}
