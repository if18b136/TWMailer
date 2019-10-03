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
#define PORT 6543

#include  <iostream>
#include <cstdlib>
#include <cstring>
using namespace std;

int main (int argc, char **argv) {
  int create_socket;
  char buffer[BUF];
  struct sockaddr_in address;
  int size;
  uint32_t port_short;

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
     printf ("Send message: ");
     fgets (buffer, BUF, stdin);
     send(create_socket, buffer, strlen (buffer), 0);
  }
  while (strcmp (buffer, "quit\n") != 0);
  close (create_socket);
  return EXIT_SUCCESS;
}
