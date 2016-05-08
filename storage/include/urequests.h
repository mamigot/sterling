#ifndef UREQUESTS_H_
#define UREQUESTS_H_

// Parse the user's request and possibly reflect changes on the filesystem
Message parseURequest(const string& input);

// True if the request matches the high-level patterns of a user request
bool isURequest(const string& request);

// True if the request is a save/delete user request
bool isUpdateURequest(const string& request);

// Utility function to insert the current time into the request
string insertTimestampIntoSavePostRequest(string& timelessRequest);

#endif
