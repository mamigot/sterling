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
/*
RMSociety:
- Tells me what my UREQUEST_PORT and IREQUEST_PORT are
- Tells me who the leader is (and reflects changes if needed)
- Has a method to refresh itself by sending heartbeats around
- Tells me the statuses of every RM (refreshed periodically)
- Has a method to tell me if a given RM is alive or dead
*/

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

    /***** For now... TODO: change! *****/
    primaryURequestPort = 13002;
    //lastHeardFromPrimary = 0;
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
    // Conduct heartbeats. Update rmStatuses, lastHeardFromPrimary and
    // potentially primaryURequestPort.

    // Send heartbeat checks every periodMs
    //const unsigned int periodMs = 1000;

    while(true) {
      //(hold the mutex only when we're ready to update rmStatuses...
      // otherwise we'll have it ad infinitum)

      // sleep after making a particular check
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
      std::to_string(lastSequenceID) + "/" + std::to_string(myURequestPort) + "\0";

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
    RMVote leader = RMVote(myURequestPort, lastSequenceID);

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
  atomic<unsigned int> lastSequenceID;
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

class URequestLedger {
public:
  void add(const string& request) {
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

  void broadcast(const string& request) {
    // Ports of active backup RMs
    auto iPorts = society.getIRequestPorts(RMStatus::Alive);

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

private:
  queue<string> ledger;
  mutex ledgerMut;

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

string modifyRequestAsPrimaryRM(string& request) {
  // The contents of the request may need to change if it's being submitted
  // to the primary RM. This performs those checks and returns an updated one.
  // (It's possible that no modifications will be made; depends on the request.)

  // If we're dealing with a savePost request, insert our time
  // (if this doesn't apply, won't change anything)
  request = insertTimestampIntoSavePostRequest(request);

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
  if(isUpdateURequest(request)){ ledger.broadcast(request); }

  // Process the request and get back to the user
  sendConn(parseURequest(request), connfd);
}

void processIRequest(const string& request, const unsigned int connfd) {
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

  /*
  I know on what port I received the request on so I know if it came from another
  RM or a user.

  If it's a request from a user and I'm not the leader: bounce!

  If it's a request from a user and I'm the leader:
    (if it's savePost, modify it to include my timestamp!)
  - Add the request to requestLedger (for each backup)
  - Tell backups to implement it (don't wait for their confirmation)
  - Process the request
  - Get back to the user with confirmation of completion

  If it's a forwarded user request from an RM and I'm not the leader:
  - Add the request to requestQueue
  - Get back to the primary RM with confirmation of receipt (which allows
    the leader to remove it from its ledger)
  - (Another thread processes the requests on the requestQueue.)
    if(requestQueue.length() <= 1 && !TRANSIT_MODE) { // TODO: lock before
      // There'll be at least one because we just put one
      // Create a new thread to process the requestQueue, one-by-one
    }

  If it's another kind of request from an RM and I'm not the leader: process it!

  If it's a request from an RM and I'm the leader: process it!

  // could be:
  - user request (being sent from the primary for replication purposes)
  - leadership inquiry (someone asking if i'm the leader)
  - leadership announcement (someone telling me that he's the leader)
  - a simple heartbeat
  - snapshot request (only valid if I'm a leader; if i grant the request,
    i go into TRANSIT_MODE which means that i have to queue my own requests
    in addition to propagating them... i'd make the snapshot and return to
    my duties. note that if i'm not the leader, i'd just deny it)
  */
}

/*

bool isAnRMPort(const unsigned int port) {
  // Users will contact the user port. we want the internal port.
  // Maybe this should be isInternalPort

  // true iff the requested port belongs to the setup of RMs
  lock_guard<rmStatusesMut> lck;

  return rmStatuses.find(port) != rmStatuses.end();
};

void leadershipInquiry() {

  // called every time the RM resurfaces

  // asks everyone who the leader is and, if after a predetermined number of
  // seconds there hasn't been a response, proclaims itself as the leader and
  // tells everyone (when an RM proclaims itself as the leader, all others
  // accept its claim... they assume that it has enough experience/data to lead)

  // if a leader gets back to it, this asks the leader to replicate its data
  // (the leader will regard this as a backup RM and will start to send user
  // requests for its queue... however, whilst in TRANSIT_MODE, this RM will
  // not process any of those requests)

}

void leadershipResponse() {

  // if an RM receives a leadershipInquiry from another, it checks if it's
  // the standing leader. if so, then it replies "no". if it's not the
  // standing leader, it has no authority and thus doesn't say anything.

}

void leadershipAnnouncement() {

  // if not in transit mode, the RM can tell everyone else that it's the leader.
  // waits for their acknowledgements

}

void handleSnapshotRequest() {

  // (only valid if I'm a leader; if i grant the request,
  // i go into TRANSIT_MODE which means that i have to queue my own requests
  // in addition to propagating them... i'd make the snapshot and return to
  // my duties. note that if i'm not the leader, i'd just deny it)

}

void makeSnapshotRequest() {

  // (in communication with the leader while it's all going on)

  // set TRANSIT_MODE to true (when this happens, user requests should not
  // be processed –just added to the ledger and the backups' queues)

  // start a cp process from the srcPort to the destPort

  // set TRANSIT_MODE to false

}

void heartbeatChecker() {

  // used to refresh the RM's worldview in regards to the PRIMARY_RM_PORT,
  // lastHeardFromPrimary and rmStatuses

  // spin up one thread for each active RM in rmStatuses, send each a message
  // and, if successful (failures because of busy sockets mean success),
  // update their statuses on rmStatuses (also note failures, obviously).
  // when checking the status of the primary RM, also update lastHeardFromPrimary

  // thread_sleep() for HEARTBEAT_PERIOD_MS –this function will be running
  // continuously

}
*/

void configReplication() {
  // read topology.txt and set relevant port numbers

  // start heartbeatChecks as a running thread that, after HEARTBEAT_PERIOD_MS,
  // polls everybody

  // configure society and ledger
  // society = ...
  // ledger = ...




}
