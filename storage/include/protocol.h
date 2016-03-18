#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <string>
#include <vector>
using namespace std;


enum ClientResponse {Ack, Stop};

void handleRequest(int connfd, char* buff, unsigned int buffSize);

ClientResponse waitForConfirmation(void);

vector<string> parseClientInput(const string& input);


#endif
