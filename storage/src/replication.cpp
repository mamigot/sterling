/*

If an RM is not the primary, it has a thread that is repeatedly spinning to
consume requests on the requestQueue. Once it becomes the leader, it gets rid
of this request.

*/

const unsigned int MY_PORT = stoi(getenv("MY_PORT"), NULL, 10); // my base port

atomic<unsigned int> PRIMARY_RM_PORT = // base port of the primary RM

atomic<unsigned int> lastHeardFromPrimary =

atomic<bool> TRANSIT_MODE = // when in TRANSIT_MODE, requests cannot be processed normally

map<unsigned int, RMStatus> rmStatuses; // maps every known RM to an enum
// detailing its status (either alive or dead). This is updated periodically
// by heartbeats.
mutex rmStatusesAccess;

map<unsigned int, string> requestLedger; // the primary RM keeps track of the
// confirmations its waiting to hear... when it receives a request from the
// user, it knows to add it to this. Likewise, when it receives a confirmation
// from a backup RM, it purges its requirement
mutex requestLedgerAccess;


bool iAmPrimaryRM() {}; // compare my port to the primary RM's

void addToRequestLedger(const unsigned int port, const string& request) {};

void addToRequestLedger(const string& request) {}; // add to all ports' accounts

void deleteFromRequestLedger(const unsigned int port, const string& request) {};

void enterTransitMode() {}; // set the transit mode variable via the mutex

void leaveTransitMode() {}; // set the transit mode variable via the mutex

void statusRequest(const unsigned int connfd) {

  // respond to a status request (might be a leadership inquiry, an
  // announcement, or just a simple heartbeat)

}

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

void leadershipAnnouncement() {

  // if not in transit mode, the RM can tell everyone else that it's the leader
  // waits for their acknowledgements

}

void heartbeatChecks() {

  // used to refresh the RM's worldview in regards to the PRIMARY_RM_PORT,
  // lastHeardFromPrimary and rmStatuses

}

void config() {

  // read topology.txt and set relevant port numbers

  // start heartbeatChecks as a running thread

}
