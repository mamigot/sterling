#include <thread>
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
  //if(status == ServerSignal::Success) singleItem = "success";
  //else if(status == ServerSignal::Error) singleItem = "error";
  //numItems = 1;
}

string Message::getSingleItem() const { return singleItem; }

vector<string> Message::getMultipleItems() const { return multipleItems; }

unsigned int Message::getItemSize() const { return itemSize; }

unsigned int Message::getNumItems() const { return numItems; }

string readConn(const unsigned int connfd) {
  // Read from the file descriptor
  char data[BUFFSIZE];

  if(read(connfd, data, BUFFSIZE) == -1) {
    cerr << "Error reading from connfd" << endl;
    exit(1);
  }

  return string(data);
}

void sendConn(const string& content, const unsigned int connfd) {
  // Send via the file descriptor
  char data[BUFFSIZE];

  if(write(connfd, content.c_str(), BUFFSIZE) == -1){
    cerr << "Write to connection failed." << endl;
  }

  cerr << "Sent: '" << content << "'" << endl;
}

void sendConn(const Message& msg, const unsigned int connfd) {
  // Check if the response fits the criteria to be sent
  unsigned int singleItemSize, numItems, maxItemsPerPacket, numPacketsToExpect;
  string rawMsg;

  char data[BUFFSIZE];

  if((singleItemSize = resp.getItemSize()) > BUFFSIZE){
    rawMsg = "500: Response is too big to fit in the given buffer. Cancelling.";
    sendConn(rawMsg, connfd);
  }

  if(!(numItems = resp.getNumItems())){
    cerr << "Don't have anything to reply... ending connection" << endl;
    return;
  }

  maxItemsPerPacket = BUFFSIZE / singleItemSize;
  numPacketsToExpect = numItems / maxItemsPerPacket + 1;

  // Let the client know how many packets to expect
  rawMsg = "201: Expect packets: " + to_string(numPacketsToExpect);
  sendPacket(connfd, rawMsg);

  // Wait for the client's acknowledgement and quit if not Ack
  if(waitForClientSignal(connfd) != ClientSignal::Ack){
    cerr << "Did not receive an ACK from the user. Quitting..." << endl;
  }

  if(!resp.getMultipleItems().empty()){
    // Send the packets, one at a time
    vector<string> items = resp.getMultipleItems();

    stringstream packetStream;
    for(size_t i = 0; i < numItems; i++){
      packetStream << items[i];

      if(!(i % maxItemsPerPacket) || i == numItems - 1){
        sendConn(packetStream.str(), connfd);

        if(waitForClientSignal(connfd, buff) == ClientSignal::Ack){
          packetStream.str(""); // Reset the string stream and continue
          packetStream.clear();
        }
      }
    }
  }else if(!resp.getSingleItem().empty()){
    // Only one item to send
    sendConn(resp.getSingleItem(), connfd);
    // Wait for confirmation from the client
    waitForClientSignal(connfd);
  }
}

void sendPort(const string& content, const unsigned int destPort) {}

void sendPort(const Message& msg, const unsigned int destPort) {}

ClientSignal waitClientSignal(const unsigned int connfd) {
  return waitClientSignal(connfd, 0);
}

ClientSignal waitClientSignal(const unsigned int connfd, unsigned int maxTimeoutMs) {
  string msg = readConn(connfd);

  if(!msg.compare("ACK")){
    cerr << "Response from user: ACK" << endl;
    return ClientSignal::Ack;

  }else if(!msg.compare("STOP")){
    cerr << "Response from user: STOP" << endl;
    return ClientSignal::Stop;

  }else{
    cerr << "Response from user: [unknown]" << endl; // TODO: HAVE A TIMEOUT HERE AND RETURN THIS AFTER A WHILE
    return ClientSignal::Unknown;
  }
}

void dispatcher(const int listenerPort, const int listenfd) {
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

    thread([connfd, port] { requestHandler(connfd, port); }).detach();
  }
}
