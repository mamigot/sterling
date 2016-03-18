#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <string>
#include <vector>
using namespace std;


//regex verifyCredentialRe("GET/credentials/self.username:self.password\0");

enum ClientResponse {Ack, Stop};

void handleRequest(int connfd, char* buff, unsigned int buffSize);

ClientResponse waitForConfirmation(void);

vector<string> parseClientInput(const string& input);



#endif
