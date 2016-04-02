/*
   From Stevens Unix Network Programming, vol 1.
   Minor modifications by John Sterling

   Application to the assignment by Miguel Amigot
 */

#include <stdio.h>       // perror, snprintf
#include <stdlib.h>      // exit
#include <unistd.h>      // close, write
#include <sys/socket.h>  // socket, AF_INET, SOCK_STREAM, bind, listen, accept
#include <netinet/in.h>  // servaddr, INADDR_ANY, htons
#include <cstring>
#include "config.h"
#include "protocol.h"

#define SA struct sockaddr
#define LISTENQ 1024  // 2nd argument to listen()

// Read socket parameters from env. variables (converted from strings)
unsigned int PORT_NUM = stoi(getenv("DATASERVER_PORT"), NULL, 10);
unsigned int BUFFSIZE = stoi(getenv("DATASERVER_BUFFSIZE"), NULL, 10);


int main(int argc, char **argv) {
  // Perform necessary configurations before listening for connections
  configServer();

  int listenfd, connfd;  // Unix file descriptors
  struct sockaddr_in servaddr;  // Note C use of struct
  char buff[BUFFSIZE];

  // 1. Create the socket
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Unable to create a socket");
    exit(1);
  }

  // 2. Set up the sockaddr_in
  // zero it.
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;// Specify the family
  // use any network card present
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT_NUM);// daytime server

  // 3. "Bind" that address object to our listening file descriptor
  if (::bind(listenfd, (SA *) &servaddr, sizeof(servaddr)) == -1) {
    perror("Unable to bind port");
    exit(2);
  }

  // 4. Tell the system that we are going to use this socket for
  // listening and request a queue length
  if (listen(listenfd, LISTENQ) == -1) {
    perror("Unable to listen");
    exit(3);
  }

  for (;;) {
    // 5. Block until someone connects.
    //    We could provide a sockaddr if we wanted to know details of whom
    //    we are talking to.
    //    Last arg is where to put the size of the sockaddr if we asked for one
    fprintf(stderr, "Ready to connect.\n");
    if ((connfd = accept(listenfd, (SA *) NULL, NULL)) == -1) {
      perror("accept failed");
      exit(4);
    }
    fprintf(stderr, "Connected.\n");

    // We have a connection. Do whatever our task is.
    handleRequest(connfd, buff, BUFFSIZE);

    // 6. Close the connection with the current client and go back for another.
    close(connfd);
  }
}
