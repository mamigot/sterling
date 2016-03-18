#include <iostream>
#include <sstream>
#include <regex>
#include "user.h"
#include "config.h" //take out! just for testing (calling configServer())
#include "protocol.h"
using namespace std;


// GET/credentials/username\0
regex existsRe("GET/credentials/([\\w]+)\0");


void initiateProtocol(void){
    configServer();
}

void handleRequest(int connfd, char* buff, unsigned int buffSize){
  int n;
  if((n = read(connfd, buff, buffSize)) == -1) {
    perror("recv");
    exit(1);
  }
  cerr << "Requested data: " << buff << ". Length: " << string(buff).length() << endl;

  vector<string> output = parseClientInput(string(buff));
  unsigned int numItems = (unsigned int) output.size();
  if(!numItems){
    cerr << "Don't have anything to reply... ending" << endl;
    return;
  }

  // All elements (strings) are guaranteed to be of the same size
  unsigned int singleItemSize = output[0].length();
  cerr << "Single item size: " << singleItemSize << endl;

  if(singleItemSize > buffSize){
    /*** Tell the client ***/
    string msg = "500: Response is too big to fit in the given buffer. Cancelling.";
    cerr << msg << endl;
    respondToClient(connfd, msg.c_str(), buffSize);
    /*** Done telling the client ***/
  }

  unsigned int maxItemsPerPacket = buffSize / singleItemSize;
  unsigned int numPacketsToExpect = numItems / maxItemsPerPacket + 1;

  /*** Tell the client ***/
  string msg = "201: Expect packets: " + std::to_string(numPacketsToExpect);
  cerr << msg << endl;
  respondToClient(connfd, msg.c_str(), buffSize);
  /*** Done telling the client ***/

  cerr << "Waiting for client to acknowledge" << endl;
  ClientResponse resp = waitForConfirmation(connfd, buff, buffSize);
  if(resp != ClientResponse::Ack){
    cerr << "Did not receive the expected \"Ack\"... ending" << endl;
    return;
  }

  stringstream packet;
  for(size_t i = 0; i < numItems; i++){
    packet << output[i];

    if(!(i % maxItemsPerPacket) || i == numItems - 1){
      /*** Send the packet to the client ***/
      string packetContent = packet.str();
      cerr << "Sending: " << packetContent << endl;
      respondToClient(connfd, packetContent.c_str(), buffSize);
      /*** Done sending the packet to the client ***/

      resp = waitForConfirmation(connfd, buff, buffSize);
      if(resp == ClientResponse::Ack){
        // Reset the string stream and continue
        packet.str("");
        packet.clear();
      }else{
        // Finished
        break;
      }
    }
  }
}

int respondToClient(int connfd, const char* buff, unsigned int buffSize){
  int len = strlen(buff);
  if (len != write(connfd, buff, strlen(buff))) {
    cerr << "Write to connection failed" << endl;
  }

  return len;
}

ClientResponse waitForConfirmation(int connfd, const char* buff, unsigned int buffSize){
  int n;
  if((n = read(connfd, (void *) buff, buffSize)) == -1) {
    cerr << "Receive failed" << endl;
  }

  cerr << "Received from user: " << buff << endl;

  if(strcmp(buff, "ACK"))
    return ClientResponse::Ack;
  else
    return ClientResponse::Stop;
}

vector<string> parseClientInput(const string& input){
  // Returns response data as a vector. When there's more than one element,
  // all are guaranteed to be of the same size.
  vector<string> output;
  smatch matches;

  if(regex_match(input, matches, existsRe)) {
    string username = matches[1]; // first group

    if(exists(username))
      output.push_back("exists");
    else
      output.push_back("does not exist");
  }

  return output;
}

/*
int main(){
  configServer();

  string input = "GET/credentials/bob\0";
  vector<string> output = parseClientInput(input);

  for(string item:output){
    cout << item << endl;
  }
}
*/
