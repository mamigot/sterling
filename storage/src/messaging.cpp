#include <thread>
#include "urequests.h"
#include "messaging.h"


void yo(const unsigned int connfd){
  fprintf(stderr, "suhhh.\n");
  close(connfd);
}

typedef void (dispatchFunc)(unsigned int);

void dispatcher(const int port, const int listenfd) {
  unsigned int connfd;

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

    dispatchFunc handler;
    
    if(port == UREQUEST_PORT) handler = uRequest;
    else if(port == RMREQUEST_PORT) handler = rmRequest;
    else if(port == STATUS_PORT) handler = statusRequest;
    else continue; // Unknown port, so nothing to do

    thread([connfd, handler] { handler(connfd); }).detach();

    /*
    User request cases:

    - I'm the primary RM and I'm getting a urequest via the user port
    - I'm a backup RM and I'm getting a urequest via the RM port (fine because it means that it's from the primary RM)
    - I'm a backup RM and I'm getting a urequest via the user port (bad –bounce!)

    */

    /*
    if(port == 13002){
      requests.push_back(
        thread([connfd] { interpretURequest(connfd); })
      );
    }
    */

    /*
    // We have a connection. Do whatever our task is.
    //async([connfd] { dispatch(connfd) });

    requests.push_back(
      thread([connfd] { yo(connfd); })
    );
    */
  }
}
