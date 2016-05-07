#ifndef MESSAGING_H_
#define MESSAGING_H_

#include <stdio.h>       // perror, snprintf
#include <stdlib.h>      // exit
#include <unistd.h>      // close, write
#include <sys/socket.h>  // socket, AF_INET, SOCK_STREAM, bind, listen, accept
#include <netinet/in.h>  // servaddr, INADDR_ANY, htons
#include <cstring>
#include <string>
#include <vector>
using namespace std;

#define SA struct sockaddr

const unsigned int UREQUEST_PORT = stoi(getenv("UREQUEST_PORT"), NULL, 10);
const unsigned int IREQUEST_PORT = stoi(getenv("IREQUEST_PORT"), NULL, 10);

// Max. size of the buffer used to transport data across servers
const unsigned int BUFFSIZE = stoi(getenv("DATASERVER_BUFFSIZE"), NULL, 10);

enum ClientSignal {Ack, Stop, Unknown}; // client -> RM communication
enum ServerSignal {Success, Error}; // RM -> client or RM -> RM communication

class Message {
public:
  Message(const string& singleItem);
  Message(const vector<string>& multipleItems);
  Message(const bool& conditional);
  Message(const ServerSignal& status);

  vector<string> getMultipleItems() const;
  string getSingleItem() const;
  unsigned int getItemSize() const;
  unsigned int getNumItems() const;

private:
  string singleItem;
  vector<string> multipleItems;
  unsigned int itemSize;
  unsigned int numItems;
};

void dispatcher(const unsigned int listenerPort, const unsigned int listenfd);

void sendPort(const string& content, const unsigned int destPort);

void sendPort(const Message& msg, const unsigned int destPort);

void sendConn(const string& content, const unsigned int connfd);

void sendConn(const Message& msg, const unsigned int connfd);

string readConn(const unsigned int connfd);

ClientSignal waitClientSignal(const unsigned int connfd);

#endif
