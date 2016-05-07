#include <thread>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include "replication.h"
#include "messaging.h"
using namespace std;

Message::Message(const string& singleItem) : singleItem(singleItem),
  itemSize(singleItem.length()), numItems(1) {}

Message::Message(const vector<string>& multipleItems) :
  multipleItems(multipleItems), numItems(multipleItems.size()) {

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

Message::Message(const bool& conditional) : numItems(1) {
  singleItem = conditional ? "true" : "false";
  itemSize = singleItem.length();
  //numItems = 1;
}

Message::Message(const ServerSignal& status) : numItems(1) {
  singleItem = (status == ServerSignal::Success) ? "success" : "error";
  itemSize = singleItem.length();
}

string Message::getSingleItem() const { return singleItem; }

vector<string> Message::getMultipleItems() const { return multipleItems; }

unsigned int Message::getItemSize() const { return itemSize; }

unsigned int Message::getNumItems() const { return numItems; }

string readConn(const unsigned int connfd) {
  // Read from the file descriptor
  char data[BUFFSIZE];

  if(read(connfd, data, BUFFSIZE) == -1) {
    throw std::runtime_error("Error reading from connfd. Perhaps a timeout.");
  }

  return string(data);
}

void sendConn(const string& content, const unsigned int connfd) {
  // Send via the file descriptor

  if(write(connfd, content.c_str(), BUFFSIZE) == -1){
    cerr << "Write to connection failed." << endl;
  }

  cerr << "Sent: '" << content << "'" << endl;
}

void sendConn(const Message& msg, const unsigned int connfd) {
  // Check if the response fits the criteria to be sent
  unsigned int singleItemSize, numItems, maxItemsPerPacket, numPacketsToExpect;
  string raw;

  if((singleItemSize = msg.getItemSize()) > BUFFSIZE){
    raw = "500: Response is too big to fit in the given buffer. Cancelling.";
    sendConn(raw, connfd);
  }

  if(!(numItems = msg.getNumItems())){
    cerr << "Don't have anything to reply... ending connection" << endl;
    return;
  }

  maxItemsPerPacket = BUFFSIZE / singleItemSize;
  numPacketsToExpect = numItems / maxItemsPerPacket + 1;

  // Let the client know how many packets to expect
  raw = "201: Expect packets: " + to_string(numPacketsToExpect);
  sendConn(raw, connfd);

  // Wait for the client's acknowledgement and quit if not Ack
  if(waitClientSignal(connfd) != ClientSignal::Ack){
    cerr << "Did not receive an ACK from the user. Quitting..." << endl;
    return;
  }

  if(!msg.getMultipleItems().empty()){
    // Send the packets, one at a time
    vector<string> items = msg.getMultipleItems();

    stringstream packetStream;
    for(size_t i = 0; i < numItems; i++){
      packetStream << items[i];

      if(!(i % maxItemsPerPacket) || i == numItems - 1){
        sendConn(packetStream.str(), connfd);

        if(waitClientSignal(connfd) == ClientSignal::Ack){
          packetStream.str(""); // Reset the string stream and continue
          packetStream.clear();
        }
      }
    }
  }else if(!msg.getSingleItem().empty()){
    // Only one item to send
    sendConn(msg.getSingleItem(), connfd);
    // Wait for confirmation from the client
    waitClientSignal(connfd);
  }
}

void sendPort(const string& content, const unsigned int destPort) {}

void sendPort(const Message& msg, const unsigned int destPort) {}

ClientSignal waitClientSignal(const unsigned int connfd) {
  try{
    // Throws error if the timeout is exceeded
    string msg = readConn(connfd);

    if(!msg.compare("ACK")){
      cerr << "Response from user: ACK" << endl;
      return ClientSignal::Ack;

    }else if(!msg.compare("STOP")){
      cerr << "Response from user: STOP" << endl;
      return ClientSignal::Stop;
    }

  }catch (std::runtime_error &e) { cerr << e.what() << endl; }

  cerr << "Response from user: [unknown]" << endl;
  return ClientSignal::Unknown;
}

void dispatcher(const unsigned int listenerPort, const unsigned int listenfd) {
  unsigned int connfd;

  // Block until someone connects.
  for (;;) {
    // We could provide a sockaddr if we wanted to know details of whom we are
    // talking to.
    fprintf(stderr, "Ready to connect.\n");
    if ((connfd = accept(listenfd, (SA *) NULL, NULL)) == -1) {
      perror("accept failed");
      exit(4);
    }
    fprintf(stderr, "Connected.\n");

    // Give each request a maximum of a 1.5 second timeout
    // (http://stackoverflow.com/a/2939145/2708484)
    struct timeval tv;
    tv.tv_sec = 1; // 1s + 500ms = 1.5s
    tv.tv_usec = 500000;
    setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(struct timeval));

    thread([&] { requestHandler(connfd, listenerPort); }).detach();
  }
}
