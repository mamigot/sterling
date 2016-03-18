#include <iostream>
#include <regex>
#include "user.h"
#include "config.h" //take out! just for testing
#include "protocol.h"
using namespace std;


// GET/credentials/username\0
regex existsRe("GET/credentials/([\\w]+)\0");


void handleRequest(int connfd, char* buff, unsigned int buffSize){
  vector<string> output = parseClientInput(string(buff));
  if(!output.size()){
    // Don't know what the user is saying
  }

}

ClientResponse waitForConfirmation(void){

  return ClientResponse::Ack;
}

vector<string> parseClientInput(const string& input){
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

int main(){
  configServer();

  string input = "GET/credentials/bob\0";
  vector<string> output = parseClientInput(input);

  for(string item:output){
    cout << item << endl;
  }
}
