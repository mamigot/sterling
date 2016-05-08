#include <iostream>
#include <sstream>
#include <regex>
#include <cstring>
#include <unistd.h>
#include "storage/access.h"
#include "storage/config.h"
#include "messaging.h"
#include "utils.h"
#include "urequests.h"
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

string insertTimestampIntoSavePostRequest(string& timelessRequest) {
  // timelessRequest should match reSavePostTimeless... we want to
  // make it match reSavePost
  smatch matches;

  if(regex_match(timelessRequest, matches, reSavePostTimeless)){
    string username = matches[1], text = matches[2];

    // Now, rebuild and return using the current time
    string timestamp = getTimeNow();
    string newRequest = "SAVE/posts/" + username + ":" + text + ":" + timestamp + "\0";

    // Verify that the new format is correct
    if(!regex_match(newRequest, matches, reSavePost)){
      throw std::runtime_error("The new request does not match reSavePost!");
    }

    return newRequest;
  }

  throw std::runtime_error("The provided request does not match reSavePostTimeless");
}

bool isURequest(const string& request) {
  smatch matches;

  return regex_match(request, matches, reGet) || \
    regex_match(request, matches, reSave) || \
    regex_match(request, matches, reDelete);
}

bool isUpdateURequest(const string& request) {
  smatch matches;

  return regex_match(request, matches, reSave) || \
    regex_match(request, matches, reDelete);
}

Message parseURequest(const string& input){
  cerr << "Requested data: " << input << ". Length: " << input.length() << endl;

  // Determine which command the request matches against
  smatch matches;

  if(regex_match(input, matches, reGet)){
    // Match against all GET patterns
    if(regex_match(input, matches, reExists)){
      string username = matches[1];
      return Message(exists(username));

    }else if(regex_match(input, matches, reVerifyCredential)){
      string username = matches[1], password = matches[2];
      return Message(verifyCredential(username, password));

    }else if(regex_match(input, matches, reGetProfilePosts)){
      string username = matches[1];
      int limit = stoi(matches[2], NULL, 10);
      return Message(getProfilePosts(username, limit));

    }else if(regex_match(input, matches, reGetTimelinePosts)){
      string username = matches[1];
      int limit = stoi(matches[2], NULL, 10);
      return Message(getTimelinePosts(username, limit));

    }else if(regex_match(input, matches, reIsFollowing)){
      string username = matches[1], friendUsername = matches[2];
      return Message(isFollowing(username, friendUsername));

    }else if(regex_match(input, matches, reGetFollowers)){
      string username = matches[1];
      int limit = stoi(matches[2], NULL, 10);
      return Message(getFollowers(username, limit));

    }else if(regex_match(input, matches, reGetFriends)){
      string username = matches[1];
      int limit = stoi(matches[2], NULL, 10);
      return Message(getFriends(username, limit));
    }

  }else{
    bool succeeded = false;

    if(regex_match(input, matches, reSave)){
      // Match against all SAVE patterns
      if(regex_match(input, matches, reSaveCredential)){
        string username = matches[1], password = matches[2];
        succeeded = saveCredential(username, password);

      }else if(regex_match(input, matches, reSavePost)){
        string username = matches[1], text = matches[2], timestamp = matches[3];
        succeeded = savePost(username, text, timestamp);

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

    if(succeeded) return Message(ServerSignal::Success);
  }

  // If nothing has matched, return an error
  return Message(ServerSignal::Error);
}
