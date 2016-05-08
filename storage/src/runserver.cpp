#include <vector>
#include <thread>
#include "messaging.h"
#include "storage/config.h"
#include "replication.h"


int getListenFD(const int port) {
  // start listening on the given port and return a Unix file descriptor
  int listenfd;
  struct sockaddr_in servaddr;

  // Create the socket
  if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Unable to create a socket");
    exit(1);
  }

  // Set up the sockaddr_in
  memset(&servaddr, 0, sizeof(servaddr)); // Zero it
  servaddr.sin_family = AF_INET; // Specify the family
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // Use any network card present
  servaddr.sin_port = htons(port);

  // "Bind" that address object to our listening file descriptor
  if(::bind(listenfd, (SA *) &servaddr, sizeof(servaddr)) == -1) {
    perror("Unable to bind port");
    exit(2);
  }

  // Tell the system that we are going to use this socket for
  // listening and request a queue length
  if(listen(listenfd, 1024) == -1) {
    perror("Unable to listen");
    exit(3);
  }

  return listenfd;
}

int main(int argc, char **argv) {
  configStorage();
  configReplication();

  unsigned int activePorts[] = {UREQUEST_PORT, IREQUEST_PORT};
  vector<thread> listeners;

  for(auto port:activePorts){
    const unsigned int listenfd = getListenFD(port);

    listeners.push_back(
      thread([port, listenfd] { dispatcher(port, listenfd); })
    );
  }

  for(thread& th:listeners){ th.join(); }
}
