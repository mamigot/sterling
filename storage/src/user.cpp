#include <iostream>
#include <map>
#include "config.h"
#include "filehandler.h"
#include "serializers.h"
#include "utils.h"
#include "user.h"
using namespace std;


bool exists(const string& username){
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

void saveCredential(const string& username, const string& password){
  string credentialPath = getStoredFilePath(StoredFileType::CredentialFile, username);
  string dataType = "CREDENTIAL";

  map<string, string> matchArgs = {
    {"USERNAME", username},
    {"PASSWORD", password}
  };

  int matchLocation = itemMatch(credentialPath, dataType, matchArgs);

  if(matchLocation == -1){
    Credential credential = {Active::Yes, username, password};
    string serialized = serializeCredential(credential);

    appendToFile(credentialPath, serialized);
  }
}

void deleteCredential(const string& username, const string& password){
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

  int numModified = setActiveFlag(false, credentialPath, dataType, matchArgs);
  if(!numModified){
    throw std::runtime_error("Cannot verify credential");
  }
}

void savePost(const string& username, const string& text){
  // Record the user's profile file
  string postTimestamp = getTimeNow();

  string profilePostPath = getStoredFilePath(StoredFileType::ProfilePostFile, username);

  ProfilePost profilePost = {Active::Yes, username, postTimestamp, text};
  string serialized = serializeProfilePost(profilePost);
  appendToFile(profilePostPath, serialized);

  // Record to each follower's timeline file
  string paddedFollowerUsername, followerUsername, timelinePostPath;
  string dataType = "RELATION", fieldType = "SECOND_USERNAME";

  for(string& serializedRelation : getFollowers(username, -1)){
    paddedFollowerUsername = extractField(serializedRelation, dataType, fieldType);

    followerUsername = unpad(paddedFollowerUsername);
    timelinePostPath = getStoredFilePath(StoredFileType::TimelinePostFile, followerUsername);

    TimelinePost timelinePost = {Active::Yes, followerUsername, username, postTimestamp, text};
    serialized = serializeTimelinePost(timelinePost);
    appendToFile(timelinePostPath, serialized);
  }
}

void deletePost(const string& username, const string& timestamp){
  string profilePostPath = getStoredFilePath(StoredFileType::ProfilePostFile, username);
  string dataType = "PROFILE_POST";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username},
    {"TIMESTAMP", timestamp}
  };

  setActiveFlag(false, profilePostPath, dataType, matchArgs);

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
}

vector<string> getTimelinePosts(const string& username, int limit){
  string timelinePostPath = getStoredFilePath(StoredFileType::TimelinePostFile, username);
  string dataType = "TIMELINE_POST";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username}
  };

  vector<string> timelinePosts = itemMatchSweep(timelinePostPath, dataType, matchArgs, -1);
  return timelinePosts;
}

vector<string> getProfilePosts(const string& username, int limit){
  string profilePostPath = getStoredFilePath(StoredFileType::ProfilePostFile, username);
  string dataType = "PROFILE_POST";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username}
  };

  vector<string> profilePosts = itemMatchSweep(profilePostPath, dataType, matchArgs, -1);
  return profilePosts;
}

bool isFollowing(const string& username, const string& friendUsername){
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

void follow(const string& username, const string& friendUsername){
  if(!exists(friendUsername)){
    throw std::runtime_error("Given friend is not real");
  }

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

      appendToFile(relationPath, serialized);
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

      appendToFile(relationPath, serialized);
    }
  }
}

void unfollow(const string& username, const string& friendUsername){
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
}

vector<string> getFollowers(const string& username, int limit){
  string relationPath = getStoredFilePath(StoredFileType::RelationFile, username);
  string dataType = "RELATION";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"FIRST_USERNAME", username},
    {"DIRECTION", "<"},
  };

  vector<string> serializedRelations = itemMatchSweep(relationPath, dataType, matchArgs, -1);
  return serializedRelations;
}

vector<string> getFriends(const string& username, int limit){
  string relationPath = getStoredFilePath(StoredFileType::RelationFile, username);
  string dataType = "RELATION";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"FIRST_USERNAME", username},
    {"DIRECTION", ">"},
  };

  vector<string> serializedRelations = itemMatchSweep(relationPath, dataType, matchArgs, -1);
  return serializedRelations;
}
