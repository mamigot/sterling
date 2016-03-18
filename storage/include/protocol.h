#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <string>
#include <vector>
using namespace std;


enum ClientResponse {Ack, Stop};

void initiateProtocol(void);

void handleRequest(int connfd, char* buff, unsigned int buffSize);

int respondToClient(int connfd, const char* buff, unsigned int buffSize);

ClientResponse waitForConfirmation(int connfd, const char* buff, unsigned int buffSize);

vector<string> parseClientInput(const string& input);


#endif
