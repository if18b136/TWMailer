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
#define BUF 1024
#define PORT 6543

#include <iostream>
#include <cstdlib>
#include <cstring>
using namespace std;

int main (int argc, char **argv) {

  if( argc < 3 ){
    cout << "Usage: " << argv[0] << " Port Verzeichnispfad" << endl;
     exit(EXIT_FAILURE);
  }

  int create_socket, new_socket;
  socklen_t addrlen;
  char buffer[BUF];
  int size;
  struct sockaddr_in address, cliaddress;
  uint32_t port_short;

  port_short = atoi(argv[1]); //port has to be 4 numbers long

  create_socket = socket (AF_INET, SOCK_STREAM, 0); //get socket file descriptor
  if (create_socket < 0) {  //check if socket was created succesfully
    perror("ERROR opening socket");
    return EXIT_FAILURE;
  }

  memset(&address,0,sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons (port_short);

  cout << address.sin_port;

  if (bind ( create_socket, (struct sockaddr *) &address, sizeof (address)) != 0) { // descriptor und bind adresse designiert, wo ich den socket gerne haben moechte, man erhaelt einen offenen socket
     perror("bind error");
     return EXIT_FAILURE;
  }
  listen (create_socket, 5); // gibt int zurueck, wenn es funktioniert hat, eine fehlermeldung, wenn fehler - noch nicht ganz sauber ausprogrammiert

  addrlen = sizeof (struct sockaddr_in);

  while (1) {
     printf("Waiting for connections...\n");
     new_socket = accept ( create_socket, (struct sockaddr *) &cliaddress, &addrlen ); // blockiert und wartet bis client antwortet.
     // befehl zum Verbindungscheck: netstat anp | grep myserver
     // telnet "Webserveradresse" "Port"
     if (new_socket > 0) // client verbunden
     {
        printf ("Client connected from %s:%d...\n", inet_ntoa (cliaddress.sin_addr),ntohs(cliaddress.sin_port));
        strcpy(buffer,"Welcome to myserver!\n");	// welcome message
        send(new_socket, buffer, strlen(buffer),0);
     }
     do {
        size = recv (new_socket, buffer, BUF-1, 0);
        if( size > 0)
        {
           buffer[size] = '\0';
           printf ("Message received: %s\n", buffer);
        }
        else if (size == 0)
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
