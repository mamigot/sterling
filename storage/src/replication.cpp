#include <atomic>
#include <mutex>
#include <memory>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <regex>
#include <cstring>
#include <string>
#include <map>
#include <queue>
#include <vector>
#include <iostream>
#include "messaging.h"
#include "urequests.h"
#include "replication.h"
using namespace std;

// The following regex patterns are used to interpret the command that
// RMs use to communicate internally

// Leader requests (RMs should only send these internally to the leader)
regex rePrimaryRMRequest("PRIMARY/.*");

regex reSequenceTaggedRequest("([0-9]+)/(.*)");

// BOUNCE/PrimaryURequestPort\0
regex reBounce("BOUNCE/([0-9]+)\0");

// SenderIRequestPort/ElectionInquiry\0
regex reElectionInquiry("([0-9]+)/ElectionInquiry\0");

// SenderIRequestPort/ElectionResponse/lastSequenceID/SenderURequestPort\0
regex reElectionResponse("([0-9]+)/ElectionResponse/([0-9]+)/([0-9]+)\0");

regex reSnapRequest("PRIMARY/"); // TODO

regex reSnapResponseStart(""); // TODO

regex reSnapResponseEnd(""); // TODO

enum RMStatus { Alive, Dead };

class URequestLedger {
public:
  URequestLedger() : lastSequenceID(0) {}

  void add(string& request) {
    // If we're adding the request, we must have received it from the leader
    // so it'll be preceeded by the leader's lastSequenceID... extract it
    smatch matches;

    if(regex_match(request, matches, reSequenceTaggedRequest)){
      request = matches[2];
      unsigned int newSequenceID = std::atoi(matches[1].str().c_str());

      if(newSequenceID > lastSequenceID) lastSequenceID = newSequenceID;

    }else{
      string msg = "Request being processed by a backup is not tagged with the \
        primary's lastSequenceID. Seen request: " + request;
      throw std::runtime_error(msg);
    }

    unsigned int ledgerSize;
    {
      lock_guard<mutex> lck(ledgerMut);
      ledger.push(request);

      ledgerSize = ledger.size();
    }

    if(ledgerSize <= 1){
      // If there's more than one element, another thread will be working to
      // complete the requests. However, if this is the only element, then we
      // should spin up another thread to process the request (and potentially
      // future ones).
      ledgerProcessor();
    }
  }

  string next() {
    lock_guard<mutex> lck(ledgerMut);

    string next = ledger.front();
    ledger.pop();

    return next;
  }

  unsigned int size() {
    lock_guard<mutex> lck(ledgerMut);
    return ledger.size();
  }

  void broadcast(const string& request, vector<unsigned int>& iPorts) {
    // Implements ordering (do not release the lock until all RMs have
    // received the message)
    unique_lock<mutex> lck(broadcastingMut);

    vector<thread> unicasts;
    for(auto& iPort:iPorts){
      unicasts.push_back(
        thread([iPort, request] { sendPort(request, iPort); })
      );
    }

    // Wait until all sends have been completed to release the lock
    for(thread& th:unicasts){ th.join(); }
  }

  unsigned int getLastSequenceID() const {
    return (unsigned int) lastSequenceID;
  }

private:
  queue<string> ledger;
  mutex ledgerMut;

  atomic<unsigned int> lastSequenceID;

  // Use when broadcasting a message to other RMs
  mutex broadcastingMut;

  // Use while exchanging data with another RM (e.g. when resurrecting)
  mutex inTransitMut;

  void ledgerProcessor() {

    thread([&] {

      unique_lock<mutex> lck(inTransitMut);
      while(size()){ parseURequest(next()); }

    }).detach();
  }

};

URequestLedger ledger;

class RMVote {
public:
  RMVote(const string& vote) : voteAsStr(vote) {
    // A vote will be a concatenation of the highest known sequence ID (every
    // time I receive a request, I update my sequence ID) with a machine's
    // port number. For example, if my highest known sequence is 10 and my
    // port is 13001, my vote will be 1013001 –this allows to go off the
    // sequence ID first and, in the event of a tie, break it off with the
    // RM's port number.

    // No RM's port number could be greater than this
    unsigned int maxPort = 100000;

    // Ex.: voteInt = 1013001
    unsigned int voteAsInt = std::stoi(vote);

    // Ex.: port = 13001
    port = voteAsInt % maxPort;

    // Ex.: lastSequenceID = (voteInt - port) / maxPort = 10
    lastSequenceID = (voteAsInt - port) / maxPort;
  }

  RMVote(unsigned int port, unsigned int lastSequenceID) : port(port),
    lastSequenceID(lastSequenceID) {

    voteAsStr = std::to_string(lastSequenceID) + std::to_string(port);
  }

  unsigned int getPort() const { return port; }

  unsigned int getLastSequenceID() const { return lastSequenceID; }

  unsigned int getValue() const { return voteAsInt; }

  string getValueAsStr() const { return voteAsStr; }

private:
  unsigned int port;
  unsigned int lastSequenceID;
  unsigned int voteAsInt;
  string voteAsStr;
};

class RMSociety {
public:
  RMSociety() {}

  void config(const unsigned int URequestPort, const unsigned int IRequestPort,
    vector<unsigned int>& iPorts) {

    myURequestPort = URequestPort;
    myIRequestPort = IRequestPort;

    for(auto iPort:iPorts) {
      rmStatuses.insert(
        // Assume that all RMs are dead originally (if it works, assume Alive)
        std::pair<unsigned int, RMStatus>(iPort, RMStatus::Alive)
      );
    }

    // Check up on other RMs in the vicinity
    heartbeats();

    // Pick a leader!
    startElection();
  }

  bool iAmPrimaryRM() { return myURequestPort == primaryURequestPort; }

  unsigned int getMyURequestPort() {
    return (unsigned int) myURequestPort;
  }

  unsigned int getMyIRequestPort() {
    return (unsigned int) myIRequestPort;
  }

  unsigned int getPrimaryURequestPort() {
    return (unsigned int) primaryURequestPort;
  };

  vector<unsigned int> getIRequestPorts(const RMStatus& status) {
    vector<unsigned int> RMs;
    lock_guard<mutex> lck(rmStatusesMut);

    for(auto& kv : rmStatuses) {
      // Exclude myself
      if(kv.second == status && kv.first != myIRequestPort){
        RMs.push_back(kv.first);
      }
    }

    return RMs;
  }

  void act(const string& request, const unsigned int connfd) {
    // Received an internal message from an RM. Parse it and act accordingly.
    smatch matches;

    if(regex_match(request, matches, rePrimaryRMRequest) && !iAmPrimaryRM()) {
      bounceRequest(connfd, primaryURequestPort);

    }else if(regex_match(request, matches, reElectionInquiry)) {
      unsigned int IRequestPort = std::atoi(matches[1].str().c_str());
      vote(IRequestPort);

    }else if(regex_match(request, matches, reElectionResponse)) {
      unsigned int lastSequenceID = std::atoi(matches[2].str().c_str()),
        URequestPort = std::atoi(matches[3].str().c_str());

      RMVote vote = RMVote(URequestPort, lastSequenceID);
      registerVote(vote);

    }else if(regex_match(request, matches, reSnapRequest)) {

    }else if(regex_match(request, matches, reSnapResponseStart)) {

    }else if(regex_match(request, matches, reSnapResponseEnd)) {

    }

    cerr << "Unknown internal message request: " << request << endl;
  }

  void heartbeats() {
    // Conduct heartbeats
    const unsigned int periodMs = 1500;

    // Get all neighboring RMs' ports
    vector<unsigned int> iPorts;
    {
      lock_guard<mutex> lck(rmStatusesMut);
      for(auto& kv:rmStatuses){ iPorts.push_back(kv.first); };
    }

    while(true) {
      for(auto& iPort:iPorts){
        int returnedBytes = sendPort("HEARTBEAT", iPort);

        {
          lock_guard<mutex> lck(rmStatusesMut);
          rmStatuses[iPort] = returnedBytes > 0 ? RMStatus::Alive : RMStatus::Dead;
        }

        cerr << "Heartbeat'd " << std::to_string(iPort) << ": ";
        if(returnedBytes > 0) cerr << "alive.\n";
        else cerr << "dead.\n";
      }

      // Relax for a bit before asking everyone again
      std::this_thread::sleep_for(std::chrono::milliseconds(periodMs));
    }
  }

  void startElection() {
    // I'd call this if I don't know who the leader is, either because I'm
    // starting up or the leader I knew died.

    // Disregard results from the previous election and start another
    votes.clear();
    inElection = true;

    // Ask every alive RM to participate in the election
    for(auto& iPort:getIRequestPorts(RMStatus::Alive)) {
      thread([&] {

        string msg = std::to_string(myIRequestPort) + "/ElectionInquiry\0";
        sendPort(msg, iPort);

      }).detach();
    }
  }

  void vote(unsigned int electionIPort) {
    // If I'm asked to participate in somebody's election
    string fullVote = std::to_string(myIRequestPort) + "/ElectionResponse/" + \
      std::to_string(ledger.getLastSequenceID()) + "/" + \
      std::to_string(myURequestPort) + "\0";

    sendPort(fullVote, electionIPort);
    cerr << "Voted on " << electionIPort << "'s election: " << fullVote << endl;
  }

  void registerVote(RMVote& vote) {
    // If I'm conducting an election and logging other RMs' votes
    cerr << "Received a vote: " << vote.getValueAsStr() << endl;

    if(inElection) {
      // Add the vote to the queue
      {
        unique_lock<mutex> lck(votesMut);
        votes.push_back(vote);
      }

      if(!aliveVoter) {
        // Launch a voter thread if one hasn't been called already
        aliveVoter = true;
        thread(chooseLeader);
      }
    }
  }

  void chooseLeader() {
    // Wait some time for the candidates to fill up before deciding
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    // All votes are in (don't allow more)
    cerr << "All votes are in!" << endl;
    inElection = false;
    unique_lock<mutex> lck(votesMut);

    // Choose a new leader (we're in the running too)
    RMVote leader = RMVote(myURequestPort, ledger.getLastSequenceID());

    // Get the highest vote
    for(auto& vote:votes) {
      if(vote.getValue() > leader.getValue()) {
        leader = vote;
      }
    }

    // We have a winner
    primaryURequestPort = leader.getPort();
    aliveVoter = false;

    cerr << "Elected a new leader: " + std::to_string(primaryURequestPort) << endl;

    /*** TODO: Request a snap from the leader ***/
  }

  void snapRequest() {
    // wait for confirmation (i.e. not a BOUNCE)
  }

  void snapResponse() {
    // handle different depending on whether I'm the leader or the requester
  }

private:
  atomic<unsigned int> myURequestPort;
  atomic<unsigned int> myIRequestPort;

  atomic<unsigned int> primaryURequestPort;
  //atomic<unsigned int> lastHeardFromPrimary;

  // Used to elect a new leader / primaryRM
  atomic<bool> inElection;
  atomic<bool> aliveVoter;
  vector<RMVote> votes;
  mutex votesMut;

  // Map IRequestPorts to statuses
  map<unsigned int, RMStatus> rmStatuses;
  mutex rmStatusesMut;

};

RMSociety society;

string modifyRequestAsPrimaryRM(string& request) {
  // The contents of the request may need to change if it's being submitted
  // to the primary RM. This performs those checks and returns an updated one.
  // (It's possible that no modifications will be made; depends on the request.)

  // If we're dealing with a savePost request, insert our time
  // (if this doesn't apply, won't change anything)
  request = insertTimestampIntoSavePostRequest(request);

  // Add our sequence number to the request
  request += std::to_string(ledger.getLastSequenceID());

  return request;
}

void processURequest(string& request, const unsigned int connfd) {
  // Request comes from a user

  // Bounce it if I'm a backup or I have other requests to attend to first
  if(!society.iAmPrimaryRM() || ledger.size() > 0) {
    bounceRequest(connfd, society.getPrimaryURequestPort());
  }

  // The primary may want to modify the raw request
  request = modifyRequestAsPrimaryRM(request);

  // Expect all backup RMs to implement it
  if(isUpdateURequest(request)){
    // Ports of active backup RMs
    auto iPorts = society.getIRequestPorts(RMStatus::Alive);
    ledger.broadcast(request, iPorts);
  }

  // Process the request and get back to the user
  sendConn(parseURequest(request), connfd);
}

void processIRequest(string& request, const unsigned int connfd) {
  // Request comes from another RM (it's internal)

  if(isURequest(request)){
    // Set up to be implemented eventually
    if(isUpdateURequest(request)){ ledger.add(request); }

    // Let the sender know that the request has been noted
    sendConn(Message(ServerSignal::Success), connfd);
  }

  // Has to be an internal message sent from one RM to another.
  // Pass it to the society manager, who'll decide what to do.
  society.act(request, connfd);
}

void requestHandler(const unsigned int connfd, const unsigned int recvPort) {
  string request = readConn(connfd);
  cerr << "Received request: " << request << endl;

  if(recvPort == UREQUEST_PORT){ // Request comes from the user
    cerr << "Responding to the user request." << endl;

    if(isURequest(request)) {
      processURequest(request, connfd);

    }else{
      throw std::runtime_error("Non-user request received via UREQUEST_PORT");
    }

  }else if(recvPort == IREQUEST_PORT){ // Request comes from another RM
    cerr << "Responding to the internal request." << endl;

    processIRequest(request, connfd);
  }

  close(connfd);
  cerr << "Finished." << endl;
}

void configReplication() {
  // read topology.txt and set relevant port numbers

  // start heartbeatChecks as a running thread that, after HEARTBEAT_PERIOD_MS,
  // polls everybody

  // configure society and ledger
  // society = ...
  // ledger = ...


}
