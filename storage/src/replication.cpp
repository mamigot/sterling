#include <atomic>
#include <mutex>
#include <memory>
#include <thread>
#include <unistd.h>
#include <regex>
#include <string>
#include <map>
#include <queue>
#include <vector>
#include <iostream>
#include "messaging.h"
#include "urequests.h"
#include "replication.h"
using namespace std;


/*
If an RM is not the primary, it has a thread that is repeatedly spinning to
consume requests on the requestQueue. Once it becomes the leader, it gets rid
of this request.
*/

enum RMStatus { Alive, Dead };

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
  bool iAmPrimaryRM() { return myURequestPort == primaryURequestPort; }

  void heartbeats() {
    // Conduct heartbeats. Update rmStatuses, lastHeardFromPrimary and
    // potentially primaryURequestPort.

    // Send heartbeat checks every periodMs
    const unsigned int periodMs = 1000;

    while(true) {
      //(hold the mutex only when we're ready to update rmStatuses...
      // otherwise we'll have it ad infinitum)

      // sleep after making a particular check
    }
  }

  unsigned int getMyURequestPort() { return (unsigned int) myURequestPort; }

  unsigned int getMyIRequestPort() { return (unsigned int) myIRequestPort; }

  vector<unsigned int> getOtherIRequestPorts(const RMStatus& status) {
    vector<unsigned int> RMs;
    lock_guard<mutex> lck(rmStatusesMut);

    for(auto& kv : rmStatuses) {
      if(kv.second == status && kv.first != myIRequestPort){ // Exclude myself
        RMs.push_back(kv.first);
      }
    }

    return RMs;
  }

private:
  const unsigned int myURequestPort;
  const unsigned int myIRequestPort;

  atomic<unsigned int> primaryURequestPort;
  atomic<unsigned int> lastHeardFromPrimary;

  // map IRequestPorts to statuses
  map<unsigned int, RMStatus> rmStatuses;
  mutex rmStatusesMut;

}

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

  unsigned int size() const {
    lock_guard<mutex> lck(ledgerMut);
    return ledger.size();
  }

  void broadcast(const string& request) const {
    // Ports of active backup RMs
    auto iPorts = society.getOtherIRequestPorts(RMStatus::Active);

    // Implements ordering (do not release the lock until all RMs have
    // received the message)
    unique_lock<mutex> lck(broadcastingMut);

    vector<thread> unicasts;
    for(auto& iPort:iPorts){
      unicasts.push_back(
        thread([iPort, request] { sendPort(iPort, request); })
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
    string request = next();

    thread([request] {

      unique_lock<mutex> lck(inTransitMut);
      while(size()){ parseURequest(request); }

    }).detach();
  }

};

URequestLedger ledger;

string modifyRequestAsPrimaryRM(string& request) {
  // The contents of the request may need to change if it's being submitted
  // to the primary RM. This performs those checks and returns an updated one.
  // (It's possible that no modifications will be made; depends on the request.)

  // If we're dealing with a savePost request, insert our time
  smatch matches;
  if(regex_match(request, matches, reSavePostTimeless)){
    request = insertTimestampIntoSavePostRequest(request);
  }

  return request;
}

void bounceRequest(const unsigned int connfd) {
  // Bounce the request via connfd and tell the user to redirect his
  // efforts to PRIMARY_RM_PORT
}

void processURequest(const string& request, const unsigned int connfd) {
  // Request comes from a user

  // Bounce it if I'm a backup or I have other requests to attend to first
  if(!society.iAmPrimaryRM() || ledger.size() > 0) { bounceRequest(connfd); }

  // The primary may want to modify the raw request
  request = modifyRequestAsPrimaryRM(request);

  // Expect all backup RMs to implement it
  if(isUpdateURequest(request)){ ledger.broadcast(request); }

  // Process the request and get back to the user
  sendConn(std::ref(parseURequest(request)), connfd);
}

void processIRequest(const string& request, const unsigned int connfd) {
  // Request comes from another RM (it's internal)

  if(isURequest(request)){
    // Set up to be implemented eventually
    if(isUpdateURequest(request)){ ledger.add(request); }

    // Let the sender know that the request has been noted
    sendConn(std::ref(Message(ServerSignal::Success)), connfd);
  }

  /***************/
  // This has to be an election message or something like that
  /***************/

}

void requestHandler(const unsigned int connfd, const unsigned int recvPort) {
  string request = readConn(connfd);

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
