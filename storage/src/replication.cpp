/*

If an RM is not the primary, it has a thread that is repeatedly spinning to
consume requests on the requestQueue. Once it becomes the leader, it gets rid
of this request.

*/


// While true, user requests should not be processed.
// Just added to requestLedger/requestQueue.
atomic<bool> TRANSIT_MODE = false;

// Base ports of the setup
const unsigned int MY_PORT = stoi(getenv("MY_PORT"), NULL, 10);
atomic<unsigned int> PRIMARY_RM_PORT;

// Period of time in milliseconds between each heartbeat interval
const unsigned int HEARTBEAT_PERIOD_MS = 1000;

// Used to keep track of the leader in the setup. Refresh periodically.
atomic<unsigned int> lastHeardFromPrimary = 0;

enum RMStatus { Alive, Dead };

// Maps every known RM to a status (enum). Refresh periodically.
map<unsigned int, RMStatus> rmStatuses;
mutex rmStatusesMut;

map<string, vector<int>> requestLedger; // the primary RM keeps track of the
// confirmations its waiting to hear... when it receives a request from the
// user, it knows to add it to this. Likewise, when it receives a confirmation
// from a backup RM, it purges its requirement
mutex requestLedgerMut;


void requestHandler(const unsigned int connfd, const unsigned int recvPort) {

  /*

  I know on what port I received the request on so I know if it came from another
  RM or a user.

  If it's a request from a user and I'm not the leader: bounce!

  If it's a request from a user and I'm the leader:
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
  /*
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

bool iAmPrimaryRM() { return MY_PORT == PRIMARY_RM_PORT };

bool isAnRMPort(const unsigned int port) {
  /* Users will contact the user port. we want the internal port.
  Maybe this should be isInternalPort */



  // true iff the requested port belongs to the setup of RMs
  lock_guard<rmStatusesMut> lck;

  return rmStatuses.find(port) != rmStatuses.end();
};

void addToRequestLedger(const unsigned int port, const string& request) {};

void addToRequestLedger(const string& request) {}; // add to all ports' accounts

void deleteFromRequestLedger(const unsigned int port, const string& request) {};

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

  /*

  (only valid if I'm a leader; if i grant the request,
  i go into TRANSIT_MODE which means that i have to queue my own requests
  in addition to propagating them... i'd make the snapshot and return to
  my duties. note that if i'm not the leader, i'd just deny it)

  */

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

void configReplication() {

  // read topology.txt and set relevant port numbers

  // start heartbeatChecks as a running thread that, after HEARTBEAT_PERIOD_MS,
  // polls everybody

}
