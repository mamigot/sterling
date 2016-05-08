#ifndef UREQUESTS_H_
#define UREQUESTS_H_

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
regex reSavePostTimeless("SAVE/posts/([a-z]+):(.*)\0");

// SAVE/posts/username:text:timestamp\0
regex reSavePost("SAVE/posts/([a-z]+):(.*):(-?[0-9]+)\0");

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

// Parse the user's request and possibly reflect changes on the filesystem
Message parseURequest(const string& input);

// True if the request matches the high-level patterns of a user request
bool isURequest(const string& request);

// True if the request is a save/delete user request
bool isUpdateURequest(const string& request);

// Utility function to insert the current time into the request
string insertTimestampIntoSavePostRequest(string& timelessRequest);

#endif
