#ifndef MESSAGING_H_
#define MESSAGING_H_

#include <stdio.h>       // perror, snprintf
#include <stdlib.h>      // exit
#include <unistd.h>      // close, write
#include <sys/socket.h>  // socket, AF_INET, SOCK_STREAM, bind, listen, accept
#include <netinet/in.h>  // servaddr, INADDR_ANY, htons
#include <cstring>
#include <string>
using namespace std;

#define SA struct sockaddr


const unsigned int UREQUEST_PORT = stoi(getenv("UREQUEST_PORT"), NULL, 10);
const unsigned int LEADERSHIP_PORT = stoi(getenv("LEADERSHIP_PORT"), NULL, 10);

/*
// Server connection parameters (get from environment variables)
const unsigned int PORT_NUM = stoi(getenv("DATASERVER_PORT"), NULL, 10);
// Max. size of the buffer used to transport data between the server and client
const unsigned int BUFFSIZE = stoi(getenv("DATASERVER_BUFFSIZE"), NULL, 10);
*/

typedef void (dispatchFunc) (const int connfd);

void dispatcher(const int port, const int listenfd);


#endif
