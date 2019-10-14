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
     // befehl zum Verbindungscheck: netstat anp | grep myserver
     // telnet "Webserveradresse" "Port"
     if (new_socket > 0) // client verbunden
     {
        printf ("Client connected from %s:%d...\n", inet_ntoa (cliaddress.sin_addr),ntohs(cliaddress.sin_port));
        strcpy(buffer,"Welcome to myserver!\n");	// welcome message
        send(new_socket, buffer, strlen(buffer),0);
        close (create_socket);
     }
     do {
        size = recv (new_socket, buffer, BUF-1, 0);
        if( size > 0)
        {
           buffer[size] = '\0';
           string str = buffer;
           char *tokenized, *username;
           tokenized = strtok (buffer,"\n");
           cout << tokenized << endl;
           username = strtok (NULL,"\n");


           if(tokenized[0] == 'S'){

             
             cout << "Message received from: " << username << endl;
             ofstream outfile;

             outfile.open(username,ios_base::app);

             ifstream file(username);
              string line;
              int linecount = 0;
              getline(file, line);
              cout << line << endl;
              if(line != "1"){
                  outfile << "1" << endl;
                  cout << "new file created" << endl;
              }
              while(getline(file, line)){
                  /*if(line == "."){

                  }
                  else */if(file.eof()){
                    cout << "end of file" << endl;
                    break;
                  }
                  else{
                    continue;
                  }
              }

             while(tokenized != NULL){
               outfile << tokenized << endl;
               tokenized = strtok(NULL,"\n");
             }
             outfile.close();
           }
           else if(tokenized[0] == 'L'){

            ofstream outfile;
            ifstream file(username);
            string line;

            cout << "looking for messages from: " << username << endl;
            outfile.open(username,ios_base::app);
            getline(file, line);
            if(line != "1"){
              char *begin = &buffer[0];
              char *end = begin + sizeof(buffer);
              fill(begin,end,0);      
              char return_zero[BUF];
              strncpy (return_zero,"0\n", BUF);
              send(new_socket, return_zero, strlen(return_zero),0);
               
            }
            else{
              int msg_count = 0, line_tracker = 0;
              string return_subj;
              string first_subj;
              string all_subj;
              while(getline(file, line)){
                if(line_tracker == 2){
                  all_subj.append(line);
                  all_subj.append("\n");
                }
                if(line == "."){
                  line_tracker = 1;
                  msg_count++;
                  do{
                    getline(file, line);

                    line_tracker++;
                  }
                  while(line_tracker <= 3);
                  if(file.eof()){
                    break;
                    }
                  else{
                    all_subj.append(line);
                    all_subj.append("\n");
                  
                  }
                }
                line_tracker++;
              }
              stringstream dumbuffer(stringstream::out);
              dumbuffer << msg_count;
              dumbuffer.str();
              strcpy(buffer, dumbuffer.str().c_str());
              strcat(buffer, "\n");
              strcat(buffer, all_subj.c_str());
              send(new_socket, buffer, strlen(buffer),0);
            }
            
           }
           else if (tokenized[0] == 'R'){
            ofstream outfile;
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
                  cout << "parsing.."<<endl;
                  char *begin = &buffer[0];
                  char *end = begin + sizeof(buffer);
                  fill(begin,end,0);
                  string ok = "OK\n";
                  strcat(buffer, ok.c_str());
                  while(getline(file, line)){
                    if(line != "."){
                       line.append("\n");
                       strcat(buffer, line.c_str());
                    }
                    else{
                      send(new_socket, buffer, strlen(buffer),0);
                      outfile.close();
                      message_count = 0;
                      break;
                    }
                  } 
                }
              }
            }
           }
           else if (tokenized[0] == 'D'){


            
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
