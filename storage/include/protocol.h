#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <string>
#include <vector>
using namespace std;


// "Ack" == everything's ok; "Stop" == cease communications
enum ClientResponse {Ack, Stop};

// Perform necessary configurations before listening for connections
void initiateProtocol(void);

// Analyze the user's request, perform the required action, and reply
void handleRequest(int connfd, char* buff, unsigned int buffSize);

// Send a message to the client by writing to the relevant buffer
int respondToClient(int connfd, const char* buff, unsigned int buffSize);

// Block until the client replies with a known response (ClientResponse)
ClientResponse waitForConfirmation(int connfd, const char* buff, unsigned int buffSize);

// Before sending, place outputs into a vector to allow sender to parse easily
vector<string> parseClientInput(const string& input);


#endif
