#include <iostream>
#include <sstream>
#include <regex>
#include <cstring>
#include <unistd.h>
#include "user.h"
#include "config.h"
#include "protocol.h"
using namespace std;


// The following regex patterns are used to interpret the command that
// the user specifies in its request

/********** GET/... **********/
regex reGet("GET/.*");

// GET/credential/username\0
regex reExists("GET/credential/([a-z]+)\0");

// GET/credential/username:password\0
regex reVerifyCredential("GET/credential/([a-z]+):([a-z]+)\0");

// GET/posts/profile/username:limit\0
regex reGetProfilePosts("GET/posts/profile/([a-z]+):(-?[0-9]+)\0");

// GET/posts/timeline/username:limit\0
regex reGetTimelinePosts("GET/posts/timeline/([a-z]+):(-?[0-9]+)\0");

// GET/relations/username:friendUsername\0
regex reIsFollowing("GET/relations/([a-z]+):([a-z]+)\0");

// GET/relations/followers/username:limit\0
regex reGetFollowers("GET/relations/followers/([a-z]+):(-?[0-9]+)\0");

// GET/relations/friends/username:limit\0
regex reGetFriends("GET/relations/friends/([a-z]+):(-?[0-9]+)\0");

/********** SAVE/... **********/
regex reSave("SAVE/.*");

// SAVE/credential/username:password\0
regex reSaveCredential("SAVE/credential/([a-z]+):([a-z]+)\0");

// SAVE/posts/username:text\0
regex reSavePost("SAVE/posts/([a-z]+):(.*)\0");

// SAVE/relations/username:friendUsername\0
regex reFollow("SAVE/relations/([a-z]+):([a-z]+)\0");

/********** DELETE/... **********/
regex reDelete("DELETE/.*");

// DELETE/credential/username:password\0
regex reDeleteCredential("DELETE/credential/([a-z]+):([a-z]+)\0");

// DELETE/posts/username:timestamp\0
regex reDeletePost("DELETE/posts/([a-z]+):([0-9]{10})\0");

// DELETE/relations/username:friendUsername\0
regex reUnfollow("DELETE/relations/([a-z]+):([a-z]+)\0");

enum ClientSignal {Ack, Stop, Unknown};

enum ServerSignal {Success, Error};

typedef struct {
  char* data;
  unsigned int length;
} TransportBuffer;

class ServerResponse {
public:
  ServerResponse(const string& singleItem) : singleItem(singleItem), itemSize(singleItem.length()), numItems(1) {}

  ServerResponse(const vector<string>& multipleItems) : multipleItems(multipleItems), numItems(multipleItems.size()) {
    if(!numItems) itemSize = 0;
    else{
      unsigned int tempItemSize = multipleItems[0].length();

      for(auto& item:multipleItems)
        if(item.length() != tempItemSize)
          throw std::runtime_error("Not all items in the response are of the same length.");

      // Now that we know that all elements are of the same length, set it
      itemSize = tempItemSize;
    }
  }

  ServerResponse(const bool& conditional) {
    if(conditional) singleItem = "true";
    else singleItem = "false";

    itemSize = singleItem.length();
    numItems = 1;
  }

  ServerResponse(const ServerSignal& status) {
    if(status == ServerSignal::Success) singleItem = "success";
    else if(status == ServerSignal::Error) singleItem = "error";

    itemSize = singleItem.length();
    numItems = 1;
  }

  string getSingleItem() const { return singleItem; }

  vector<string> getMultipleItems() const { return multipleItems; }

  unsigned int getItemSize() const { return itemSize; }

  unsigned int getNumItems() const { return numItems; }

private:
  string singleItem;
  vector<string> multipleItems;
  unsigned int itemSize;
  unsigned int numItems;
};


ClientSignal waitForClientSignal(const int connfd, const TransportBuffer& buff);

ServerResponse parseRequest(const int connfd, TransportBuffer& buff);

void respond(const int connfd, const ServerResponse& resp, TransportBuffer& buff);

void sendPacket(const int connfd, const string& content, const int buffSize);


void handleRequest(const int connfd){
  cerr << "Responding to request." << endl;

  char data[BUFFSIZE];
  TransportBuffer buff = {data, BUFFSIZE};

  ServerResponse resp = parseRequest(connfd, std::ref(buff));
  respond(connfd, std::ref(resp), std::ref(buff));

  close(connfd);
  cerr << "Finished." << endl;
}

ClientSignal waitForClientSignal(const int connfd, const TransportBuffer& buff){
  if(read(connfd, buff.data, buff.length) == -1){
    cerr << "Receive failed" << endl;
  }

  if(strncmp(buff.data, "ACK", 4)){
    cerr << "Response from user: ACK" << endl;
    return ClientSignal::Ack;

  }else if(strncmp(buff.data, "STOP", 5)){
    cerr << "Response from user: STOP" << endl;
    return ClientSignal::Stop;

  }else{
    cerr << "Response from user: [unknown]" << endl; // TODO: HAVE A TIMEOUT HERE AND RETURN THIS AFTER A WHILE
    return ClientSignal::Unknown;
  }
}

ServerResponse parseRequest(const int connfd, TransportBuffer& buff){
  // Read from the file descriptor
  if(read(connfd, buff.data, buff.length) == -1) {
    cerr << "Error reading from connfd" << endl;
    exit(1);
  }

  string input(buff.data);
  cerr << "Requested data: " << buff.data << ". Length: " << input.length() << endl;

  // Determine which command the request matches against
  smatch matches;

  if(regex_match(input, matches, reGet)){
    // Match against all GET patterns
    if(regex_match(input, matches, reExists)){
      string username = matches[1];
      return ServerResponse(exists(username));

    }else if(regex_match(input, matches, reVerifyCredential)){
      string username = matches[1], password = matches[2];
      return ServerResponse(verifyCredential(username, password));

    }else if(regex_match(input, matches, reGetProfilePosts)){
      string username = matches[1];
      int limit = stoi(matches[2], NULL, 10);
      return ServerResponse(getProfilePosts(username, limit));

    }else if(regex_match(input, matches, reGetTimelinePosts)){
      string username = matches[1];
      int limit = stoi(matches[2], NULL, 10);
      return ServerResponse(getTimelinePosts(username, limit));

    }else if(regex_match(input, matches, reIsFollowing)){
      string username = matches[1], friendUsername = matches[2];
      return ServerResponse(isFollowing(username, friendUsername));

    }else if(regex_match(input, matches, reGetFollowers)){
      string username = matches[1];
      int limit = stoi(matches[2], NULL, 10);
      return ServerResponse(getFollowers(username, limit));

    }else if(regex_match(input, matches, reGetFriends)){
      string username = matches[1];
      int limit = stoi(matches[2], NULL, 10);
      return ServerResponse(getFriends(username, limit));
    }

  }else{
    bool succeeded = false;

    if(regex_match(input, matches, reSave)){
      // Match against all SAVE patterns
      if(regex_match(input, matches, reSaveCredential)){
        string username = matches[1], password = matches[2];
        succeeded = saveCredential(username, password);

      }else if(regex_match(input, matches, reSavePost)){
        string username = matches[1], text = matches[2];
        succeeded = savePost(username, text);

      }else if(regex_match(input, matches, reFollow)){
        string username = matches[1], friendUsername = matches[2];
        succeeded = follow(username, friendUsername);
      }

    }else if(regex_match(input, matches, reDelete)){
      // Match against all DELETE patterns
      if(regex_match(input, matches, reDeleteCredential)){
        string username = matches[1], password = matches[2];
        succeeded = deleteCredential(username, password);

      }else if(regex_match(input, matches, reDeletePost)){
        string username = matches[1], timestamp = matches[2];
        succeeded = deletePost(username, timestamp);

      }else if(regex_match(input, matches, reUnfollow)){
        string username = matches[1], friendUsername = matches[2];
        succeeded = unfollow(username, friendUsername);
      }
    }

    if(succeeded) return ServerResponse(ServerSignal::Success);
  }

  // If nothing has matched, return an error
  return ServerResponse(ServerSignal::Error);
}

void respond(const int connfd, const ServerResponse& resp, TransportBuffer& buff){
  // Check if the response fits the criteria to be sent
  unsigned int singleItemSize, numItems, maxItemsPerPacket, numPacketsToExpect;
  string msg;

  if((singleItemSize = resp.getItemSize()) > buff.length){
    msg = "500: Response is too big to fit in the given buffer. Cancelling.";
    sendPacket(connfd, msg, buff.length);
  }

  if(!(numItems = resp.getNumItems())){
    cerr << "Don't have anything to reply... ending connection" << endl;
    return;
  }

  maxItemsPerPacket = buff.length / singleItemSize;
  numPacketsToExpect = numItems / maxItemsPerPacket + 1;

  // Let the client know how many packets to expect
  msg = "201: Expect packets: " + to_string(numPacketsToExpect);
  sendPacket(connfd, msg, buff.length);

  // Wait for the client's acknowledgement and quit if not Ack
  if(waitForClientSignal(connfd, buff) != ClientSignal::Ack){
    cerr << "Did not receive an ACK from the user. Quitting..." << endl;
  }

  if(!resp.getMultipleItems().empty()){
    // Send the packets, one at a time
    vector<string> items = resp.getMultipleItems();

    stringstream packetStream;
    for(size_t i = 0; i < numItems; i++){
      packetStream << items[i];

      if(!(i % maxItemsPerPacket) || i == numItems - 1){
        sendPacket(connfd, packetStream.str(), buff.length);

        if(waitForClientSignal(connfd, buff) == ClientSignal::Ack){
          packetStream.str(""); // Reset the string stream and continue
          packetStream.clear();
        }
      }
    }
  }else if(!resp.getSingleItem().empty()){
    // Only one item to send
    sendPacket(connfd, resp.getSingleItem(), buff.length);
    // Wait for confirmation from the client
    waitForClientSignal(connfd, buff);
  }
}

void sendPacket(const int connfd, const string& content, const int buffSize){
  if(write(connfd, content.c_str(), buffSize) == -1){
    cerr << "Write to connection failed." << endl;
  }

  cerr << "Sent: '" << content << "'" << endl;
}
