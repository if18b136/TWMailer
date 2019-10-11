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
#define LEN 5
#define PORT 6543

#include <iostream>
#include <cstdlib>
#include <string>
using namespace std;

int main (int argc, char **argv) {
  int create_socket;
  char buffer[BUF], input[BUF];
  string sender;
  struct sockaddr_in address;
  int size;
  uint32_t port_short;
  bool overload = false;

  if( argc < 3 ){
    cout << "Usage: " << argv[0] << " ServerAdresse Port" << endl;
     exit(EXIT_FAILURE);
  }

  if ((create_socket = socket (AF_INET, SOCK_STREAM, 0)) == -1)
  {
     perror("Socket error");
     return EXIT_FAILURE;
  }

  port_short = atoi(argv[2]);

  memset(&address,0,sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons (port_short); // byte order - host to network
  inet_aton (argv[1], &address.sin_addr);

  if (connect ( create_socket, (struct sockaddr *) &address, sizeof (address)) == 0)
  {
     printf ("Connection with server (%s) established\n", inet_ntoa (address.sin_addr));
     size=recv(create_socket,buffer,BUF-1, 0);
     if (size>0)
     {
        buffer[size]= '\0';
        printf("%s",buffer);
     }
  }
  else
  {
     perror("Connect error - no server available");
     return EXIT_FAILURE;
  }

  do {
     while(strcmp (input, "quit\n") != 0){
       cout << "Enter your command: "<< endl;
       fgets (input, BUF, stdin);
       if(strcmp (input, "SEND\n") == 0){ //two times in a row exceeds char limit - clean input after successful send
         strncpy(buffer,input,BUF);

         // Sender input
         do{
           if(overload){
             cout << "input exceeds char limit. (max. 8 characters)" << endl;
           }
           fgets (input, BUF, stdin);
           overload = true;
         }
         while(input[8] != '\n' && input[8] != '\0');
         strcat(buffer,input);

         // receiver input
         overload = false; // reusing bool for next input exceeding
         do{
           if(overload){
             cout << "input exceeds char limit. (max. 8 characters)" << endl;
           }
           fgets (input, BUF, stdin);
           overload = true;
         }
         while(input[8] != '\n' && input[8] != '\0');
         strcat(buffer,input);

         // subject input
         overload = false; // reusing bool for next input exceeding
         do{
           if(overload){
             cout << "input exceeds char limit. (max. 80 characters)" << endl;
           }
           char *begin = &input[0];
           char *end = begin + sizeof(input);
           fill(begin,end,0);
           fgets (input, BUF, stdin);
           overload = true;
           //cout << " Betreff " << input << "stelle 80 " << input[80]<< endl;
         }
         while(input[80] != '\n' && input[80] != '\0');

         strcat(buffer,input);

         while(strcmp (input, ".\n") != 0){
           //cout << "input message" << input << endl;
           fgets (input, BUF, stdin);
           strcat(buffer,input);
         }
         //cout << "went out of loop" << input << endl;
         send(create_socket, buffer, strlen (buffer), 0);
     }

     else if(strcmp (input, "LIST\n") == 0){
       strncpy(buffer,input,BUF);
       // username input
       overload = false; // reusing bool for next input exceeding
       do{
         if(overload){
           cout << "input exceeds char limit. (max. 8 characters)" << endl;
         }
         char *begin = &input[0];
         char *end = begin + sizeof(input);
         fill(begin,end,0);
         fgets (input, BUF, stdin);
         overload = true;
       }
       while(input[8] != '\n' && input[8] != '\0');
       strcat(buffer,input);
       send(create_socket, buffer, strlen (buffer), 0); // send user request

       size=recv(create_socket,buffer,BUF-1, 0); //wait for buffer from server
       if (size>0)
       {
          buffer[size]= '\0';
          cout << buffer << endl;
       }
     }
     else if(strcmp (input, "READ\n") == 0){
       strncpy(buffer,input,BUF);
       // username input
       overload = false; // reusing bool for next input exceeding
       do{
         if(overload){
           cout << "input exceeds char limit. (max. 8 characters)" << endl;
         }
         char *begin = &input[0];
         char *end = begin + sizeof(input);
         fill(begin,end,0);
         fgets (input, BUF, stdin);
         overload = true;
       }
       while(input[8] != '\n' && input[8] != '\0');
       strcat(buffer,input);
       fgets (input, BUF, stdin);
       strcat(buffer,input);
       cout << buffer << endl;
       send(create_socket, buffer, strlen (buffer), 0); // send user request

       size=recv(create_socket,buffer,BUF-1, 0); //wait for buffer from server
       if (size>0)
       {
          buffer[size]= '\0';
          cout << buffer << endl;
       }
     }
     else if(strcmp (input, "DEL\n") == 0){
       strncpy(buffer,input,BUF);
       // username input
       overload = false; // reusing bool for next input exceeding
       do{
         if(overload){
           cout << "input exceeds char limit. (max. 8 characters)" << endl;
         }
         char *begin = &input[0];
         char *end = begin + sizeof(input);
         fill(begin,end,0);
         fgets (input, BUF, stdin);
         overload = true;
       }
       while(input[8] != '\n' && input[8] != '\0');
       strcat(buffer,input);
       fgets (input, BUF, stdin);
       strcat(buffer,input);
       cout << buffer << endl;
       send(create_socket, buffer, strlen (buffer), 0); // send user request

       size=recv(create_socket,buffer,BUF-1, 0); //wait for buffer from server
       if (size>0)
       {
          buffer[size]= '\0';
          cout << buffer << endl;
       }
     }
     else if(strcmp (input, "quit\n") == 0){
       strncpy(buffer,input,BUF);
       break;
     }
     else{
       cout << "Unknown command, please repeat." << endl;
     }
   }
  }
  while (strcmp (buffer, "quit\n") != 0);
  close (create_socket);
  return EXIT_SUCCESS;
}
// dyn buffer
// fehlerbedingungen bei funktionsabbruch
// fgets kann auch nichts einlesen - fehlermeldung
// return codes anschauen
