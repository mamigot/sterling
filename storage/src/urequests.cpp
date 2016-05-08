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
