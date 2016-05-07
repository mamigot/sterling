#include <iostream>
#include <map>
#include "storage/config.h"
#include "utils.h"
#include "storage/filehandler.h"
#include "storage/serializers.h"
#include "storage/access.h"
using namespace std;


bool exists(const string& username){
  // Check if the wanted account exists and is active
  string credentialPath = getStoredFilePath(StoredFileType::CredentialFile, username);
  string dataType = "CREDENTIAL";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username}
  };

  int matchLocation = itemMatch(credentialPath, dataType, matchArgs);
  return matchLocation != -1;
}

bool verifyCredential(const string& username, const string& password){
  // true iff there's an entry marked "active" that matches the user's username and password
  string credentialPath = getStoredFilePath(StoredFileType::CredentialFile, username);
  string dataType = "CREDENTIAL";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username},
    {"PASSWORD", password}
  };

  int matchLocation = itemMatch(credentialPath, dataType, matchArgs);
  return matchLocation != -1;
}

bool saveCredential(const string& username, const string& password){
  // save the username, password combo to the system
  string credentialPath = getStoredFilePath(StoredFileType::CredentialFile, username);
  string dataType = "CREDENTIAL";

  // able to save as long as the username is different
  const map<string, string> matchArgs = {{"USERNAME", username}};
  int matchLocation = itemMatch(credentialPath, dataType, matchArgs);

  if(matchLocation == -1){
    // Means that the credential wasn't found in the system. Save it!
    Credential credential = {Active::Yes, username, password};
    string serialized = serializeCredential(credential);

    appendToDataFile(credentialPath, serialized);
    return true;
  }

  return false; // Credential already existed (none created)
}

bool deleteCredential(const string& username, const string& password){
  // Delete the credential as well as all of the relevant posts
  string timestamp, dataType = "PROFILE_POST", fieldType = "TIMESTAMP";
  for(string& serializedPost : getProfilePosts(username, -1)){
    timestamp = extractField(serializedPost, dataType, fieldType);
    deletePost(username, timestamp);
  }

  string credentialPath = getStoredFilePath(StoredFileType::CredentialFile, username);
  dataType = "CREDENTIAL";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username},
    {"PASSWORD", password}
  };

  setActiveFlag(false, credentialPath, dataType, matchArgs);
  return true; // iff everything has gone well
}

bool savePost(const string& username, const string& text, const string& postTimestamp){
  // Save the post (as seen by the user's profile file and the followers'
  // timeline files).

  // Record to the user's profile file
  string profilePostPath = getStoredFilePath(StoredFileType::ProfilePostFile, username);

  ProfilePost profilePost = {Active::Yes, username, postTimestamp, text};
  string serialized = serializeProfilePost(profilePost);
  appendToDataFile(profilePostPath, serialized);

  // Record to each follower's timeline file
  string paddedFollowerUsername, followerUsername, timelinePostPath;
  string dataType = "RELATION", fieldType = "SECOND_USERNAME";

  for(string& serializedRelation : getFollowers(username, -1)){
    paddedFollowerUsername = extractField(serializedRelation, dataType, fieldType);

    followerUsername = unpad(paddedFollowerUsername);
    timelinePostPath = getStoredFilePath(StoredFileType::TimelinePostFile, followerUsername);

    // Create the timeline post and save it
    TimelinePost timelinePost = {Active::Yes, followerUsername, username, postTimestamp, text};
    serialized = serializeTimelinePost(timelinePost);
    appendToDataFile(timelinePostPath, serialized);
  }

  return true;
}

bool deletePost(const string& username, const string& timestamp){
  // Delete the post (as seen by the user's profile file and the followers'
  // timeline files) --slow

  string profilePostPath = getStoredFilePath(StoredFileType::ProfilePostFile, username);
  string dataType = "PROFILE_POST";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username},
    {"TIMESTAMP", timestamp}
  };

  setActiveFlag(false, profilePostPath, dataType, matchArgs);

  // Delete from the followers' timelines
  string paddedFollowerUsername, followerUsername, timelinePostPath, fieldType;
  for(string& serializedRelation : getFollowers(username, -1)){
    dataType = "RELATION", fieldType = "SECOND_USERNAME";
    paddedFollowerUsername = extractField(serializedRelation, dataType, fieldType);

    followerUsername = unpad(paddedFollowerUsername);
    timelinePostPath = getStoredFilePath(StoredFileType::TimelinePostFile, followerUsername);
    dataType = "TIMELINE_POST";

    matchArgs.clear();
    matchArgs["ACTIVE"] = "1";
    matchArgs["USERNAME"] = followerUsername;
    matchArgs["AUTHOR"] = username;
    matchArgs["TIMESTAMP"] = timestamp;

    setActiveFlag(false, timelinePostPath, dataType, matchArgs);
  }

  return true; // iff everything has gone well
}

vector<string> getTimelinePosts(const string& username, int limit){
  // Retrieve the posts that belong on the user's timeline (read from single file --fast)
  string timelinePostPath = getStoredFilePath(StoredFileType::TimelinePostFile, username);
  string dataType = "TIMELINE_POST";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username}
  };

  vector<string> timelinePosts = itemMatchSweep(timelinePostPath, dataType, matchArgs, limit);
  return timelinePosts;
}

vector<string> getProfilePosts(const string& username, int limit){
  // Retrieve the posts that belong on the user's profile (read from single file --fast)
  string profilePostPath = getStoredFilePath(StoredFileType::ProfilePostFile, username);
  string dataType = "PROFILE_POST";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username}
  };

  vector<string> profilePosts = itemMatchSweep(profilePostPath, dataType, matchArgs, limit);
  return profilePosts;
}

bool isFollowing(const string& username, const string& friendUsername){
  // True if username follows friendUsername, false otherwise --fast
  string relationPath = getStoredFilePath(StoredFileType::RelationFile, username);
  string dataType = "RELATION";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"FIRST_USERNAME", username},
    {"DIRECTION", ">"},
    {"SECOND_USERNAME", friendUsername}
  };

  int matchLocation = itemMatch(relationPath, dataType, matchArgs);
  return matchLocation != -1;
}

bool follow(const string& username, const string& friendUsername){
  // Record the follow as a relation in username's file as well as
  // friendUsername's --slow
  if(!exists(friendUsername)) return false;

  // Record the user's relation (only make a change if not following already)
  string relationPath = getStoredFilePath(StoredFileType::RelationFile, username);
  string dataType = "RELATION";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"FIRST_USERNAME", username},
    {"DIRECTION", ">"},
    {"SECOND_USERNAME", friendUsername}
  };

  int numModified = setActiveFlag(true, relationPath, dataType, matchArgs);

  if(!numModified){
    matchArgs["ACTIVE"] = "0";
    numModified = setActiveFlag(true, relationPath, dataType, matchArgs);

    if(!numModified){
      Relation relation = {Active::Yes, username, '>', friendUsername};
      string serialized = serializeRelation(relation);

      appendToDataFile(relationPath, serialized);
    }
  }

  // Record the friend's relation (only make a change if not following already)
  relationPath = getStoredFilePath(StoredFileType::RelationFile, friendUsername);

  matchArgs.clear();
  matchArgs["ACTIVE"] = "1";
  matchArgs["FIRST_USERNAME"] = friendUsername;
  matchArgs["DIRECTION"] = "<";
  matchArgs["SECOND_USERNAME"] = username;

  numModified = setActiveFlag(true, relationPath, dataType, matchArgs);

  if(!numModified){
    matchArgs["ACTIVE"] = "0";
    numModified = setActiveFlag(true, relationPath, dataType, matchArgs);

    if(!numModified){
      Relation relation = {Active::Yes, friendUsername, '<', username};
      string serialized = serializeRelation(relation);

      appendToDataFile(relationPath, serialized);
    }
  }

  return true;
}

bool unfollow(const string& username, const string& friendUsername){
  // Record the unfollow as a relation in username's file as well as
  // friendUsername's. Also delete friendUsername's posts from username's
  // timeline --slow
  string dataType = "RELATION";

  string relationPath = getStoredFilePath(StoredFileType::RelationFile, username);

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"FIRST_USERNAME", username},
    {"DIRECTION", ">"},
    {"SECOND_USERNAME", friendUsername}
  };

  setActiveFlag(false, relationPath, dataType, matchArgs);

  relationPath = getStoredFilePath(StoredFileType::RelationFile, friendUsername);

  matchArgs.clear();
  matchArgs["ACTIVE"] = "1";
  matchArgs["FIRST_USERNAME"] = friendUsername;
  matchArgs["DIRECTION"] = "<";
  matchArgs["SECOND_USERNAME"] = username;

  setActiveFlag(false, relationPath, dataType, matchArgs);

  dataType = "TIMELINE_POST";
  string timelinePostPath = getStoredFilePath(StoredFileType::TimelinePostFile, username);

  matchArgs.clear();
  matchArgs["ACTIVE"] = "1";
  matchArgs["USERNAME"] = username;
  matchArgs["AUTHOR"] = friendUsername;

  setActiveFlag(false, timelinePostPath, dataType, matchArgs);
  return true; // iff everything has gone well
}

vector<string> getFollowers(const string& username, int limit){
  // Retrieve follower relations (read from single file --fast)
  string relationPath = getStoredFilePath(StoredFileType::RelationFile, username);
  string dataType = "RELATION";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"FIRST_USERNAME", username},
    {"DIRECTION", "<"},
  };

  vector<string> serializedRelations = itemMatchSweep(relationPath, dataType, matchArgs, limit);
  return serializedRelations;
}

vector<string> getFriends(const string& username, int limit){
  // Retrieve friend relations (read from single file --fast)
  string relationPath = getStoredFilePath(StoredFileType::RelationFile, username);
  string dataType = "RELATION";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"FIRST_USERNAME", username},
    {"DIRECTION", ">"},
  };

  vector<string> serializedRelations = itemMatchSweep(relationPath, dataType, matchArgs, limit);
  return serializedRelations;
}
