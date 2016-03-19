#include <iostream>
#include <sstream>
#include <regex>
#include "user.h"
#include "config.h"
#include "protocol.h"
using namespace std;


// The following regex patterns are used to interpret the command that
// the user specifies in its request

/********** GET/... **********/
regex reGet("GET/.*");

// GET/credential/username\0
regex reExists("GET/credential/([\\w]+)\0");

// GET/credential/username:password\0
regex reVerifyCredential("GET/credential/([\\w]+):([\\w]+)\0");

// GET/posts/profile/username:limit\0
regex reGetProfilePosts("GET/posts/profile/([\\w]+):(-?[\\d]+)\0");

// GET/posts/timeline/username:limit\0
regex reGetTimelinePosts("GET/posts/timeline/([\\w]+):(-?[\\d]+)\0");

// GET/relations/username:friendUsername\0
regex reIsFollowing("GET/relations/([\\w]+):([\\w]+)\0");

// GET/relations/followers/username:limit\0
regex reGetFollowers("GET/relations/followers/([\\w]+):(-?[\\d]+)\0");

// GET/relations/friends/username:limit\0
regex reGetFriends("GET/relations/friends/([\\w]+):(-?[\\d]+)\0");


/********** SAVE/... **********/
regex reSave("SAVE/.*");

// SAVE/credential/username:password\0
regex reSaveCredential("SAVE/credential/([\\w]+):([\\w]+)\0");

// SAVE/posts/username:text\0
regex reSavePost("SAVE/posts/([\\w]+):(.*)\0");

// SAVE/relations/username:friendUsername\0
regex reFollow("SAVE/relations/([\\w]+):([\\w]+)\0");


/********** DELETE/... **********/
regex reDelete("DELETE/.*");

// DELETE/credential/username:password\0
regex reDeleteCredential("DELETE/credential/([\\w]+):([\\w]+)\0");

// DELETE/posts/username:timestamp\0
regex reDeletePost("DELETE/posts/([\\w]+):([\\d]{10})\0");

// DELETE/relations/username:friendUsername\0
regex reUnfollow("DELETE/relations/([\\w]+):([\\w]+)\0");


void initiateProtocol(void){
  configServer();
}

void handleRequest(int connfd, char* buff, unsigned int buffSize){
  // This is called when a connection is received

  // Read the user's request
  int n;
  if((n = read(connfd, buff, buffSize)) == -1) {
    perror("recv");
    exit(1);
  }
  cerr << "Requested data: " << buff << ". Length: " << string(buff).length() << endl;

  // Determine contents of response to the user
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
    cerr << "Did not receive the expected \"ACK\"... ending" << endl;
    return;
  }

  // Send packets, one at a time
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

  if(strncmp(buff, "ACK", 4)){
    cerr << "Response from user: ACK" << endl;
    return ClientResponse::Ack;

  }else{
    cerr << "Response from user: STOP" << endl;
    return ClientResponse::Stop;
  }
}

vector<string> parseClientInput(const string& input){
  // Parses user request through regular expressions, extracting the
  // relevant arguments for each command.
  // Returns response data as a vector. When there's more than one element,
  // all are guaranteed to be of the same size.
  vector<string> output;
  smatch matches;

  if(regex_match(input, matches, reGet)){
    // Match against all GET patterns
    if(regex_match(input, matches, reExists)){
      string username = matches[1];

      if(exists(username)) output.push_back("true");
      else output.push_back("false");

    }else if(regex_match(input, matches, reVerifyCredential)){
      string username = matches[1], password = matches[2];

      if(verifyCredential(username, password)) output.push_back("true");
      else output.push_back("false");

    }else if(regex_match(input, matches, reGetProfilePosts)){
      string username = matches[1];
      int limit = stoi(matches[2], NULL, 10);

      output = getProfilePosts(username, limit);

    }else if(regex_match(input, matches, reGetTimelinePosts)){
      string username = matches[1];
      int limit = stoi(matches[2], NULL, 10);

      output = getTimelinePosts(username, limit);

    }else if(regex_match(input, matches, reIsFollowing)){
      string username = matches[1], friendUsername = matches[2];

      if(isFollowing(username, friendUsername)) output.push_back("true");
      else output.push_back("false");

    }else if(regex_match(input, matches, reGetFollowers)){
      string username = matches[1];
      int limit = stoi(matches[2], NULL, 10);

      output = getFollowers(username, limit);

    }else if(regex_match(input, matches, reGetFriends)){
      string username = matches[1];
      int limit = stoi(matches[2], NULL, 10);

      output = getFriends(username, limit);
    }

  }else if(regex_match(input, matches, reSave)){
    // Match against all SAVE patterns
    if(regex_match(input, matches, reSaveCredential)){
      string username = matches[1], password = matches[2];

      if(saveCredential(username, password)) output.push_back("success");
      else output.push_back("error");

    }else if(regex_match(input, matches, reSavePost)){
      string username = matches[1], text = matches[2];

      if(savePost(username, text)) output.push_back("success");
      else output.push_back("error");

    }else if(regex_match(input, matches, reFollow)){
      string username = matches[1], friendUsername = matches[2];

      if(follow(username, friendUsername)) output.push_back("success");
      else output.push_back("error");
    }

  }else if(regex_match(input, matches, reDelete)){
    // Match against all DELETE patterns
    if(regex_match(input, matches, reDeleteCredential)){
      string username = matches[1], password = matches[2];

      if(deleteCredential(username, password)) output.push_back("success");
      else output.push_back("error");

    }else if(regex_match(input, matches, reDeletePost)){
      string username = matches[1], timestamp = matches[2];

      if(deletePost(username, timestamp)) output.push_back("success");
      else output.push_back("error");

    }else if(regex_match(input, matches, reUnfollow)){
      string username = matches[1], friendUsername = matches[2];

      if(unfollow(username, friendUsername)) output.push_back("success");
      else output.push_back("error");
    }
  }

  return output;
}
