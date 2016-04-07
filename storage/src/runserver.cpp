#include <stdio.h>       // perror, snprintf
#include <stdlib.h>      // exit
#include <unistd.h>      // close, write
#include <sys/socket.h>  // socket, AF_INET, SOCK_STREAM, bind, listen, accept
#include <netinet/in.h>  // servaddr, INADDR_ANY, htons
#include <cstring>
#include <thread>
#include <vector>
#include "config.h"
#include "protocol.h"

#define SA struct sockaddr
#define LISTENQ 1024  // 2nd argument to listen()


void launchDispatcher(const int listenfd);


int main(int argc, char **argv) {
  // Perform necessary configurations before listening for connections
  configServer();

  int listenfd; // Unix file descriptor
  struct sockaddr_in servaddr;

  // 1. Create the socket
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Unable to create a socket");
    exit(1);
  }

  // 2. Set up the sockaddr_in
  memset(&servaddr, 0, sizeof(servaddr)); // Zero it
  servaddr.sin_family = AF_INET; // Specify the family
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // Use any network card present
  servaddr.sin_port = htons(PORT_NUM); // Daytime server

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

  // Prepare for requests and handle them as they come
  launchDispatcher(listenfd);
}

void launchDispatcher(const int listenfd){
  int connfd;
  vector<thread> requests;

  // Block until someone connects.
  for (;;) {
    // We could provide a sockaddr if we wanted to know details of whom we are
    // talking to.
    fprintf(stderr, "Ready to connect.\n");
    if ((connfd = accept(listenfd, (SA *) NULL, NULL)) == -1) {
      perror("accept failed");
      exit(4);
    }
    fprintf(stderr, "Connected.\n");

    // We have a connection. Do whatever our task is.
    requests.push_back(
      thread([connfd] { handleRequest(connfd); })
    );
  }

  for(thread& th:requests){ th.join(); }
}
